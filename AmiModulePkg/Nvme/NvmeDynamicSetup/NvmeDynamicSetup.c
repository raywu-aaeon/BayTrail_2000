//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**           5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093      **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file NvmeDynamicSetup.c
    Updates Nvme setup page dynamically

**/

#include <Token.h>

//Library used
#include <Library/HiiLib.h>
#include <Library/AmiHiiUpdateLib.h>
#include <Guid/MdeModuleHii.h>
#include <Protocol/HiiString.h>
#include "NvmeDynamicSetup.h"
#include "Nvme/NvmeIncludes.h"
#include "Nvme/NvmeBus.h"
#include <AmiDxeLib.h>

//-------------------------------------------------------------------------
// Global Variable Definitions
//-------------------------------------------------------------------------

static EFI_HII_STRING_PROTOCOL        *HiiString = NULL;
static CHAR8                          *SupportedLanguages = NULL;

static EFI_HII_HANDLE gNvmeHiiHandle = NULL;

static EFI_GUID gNvmeFormSetGuid = NVME_FORM_SET_GUID;

EFI_STRING_ID gNvmeControllerLocationStrIds[MAXIMUM_NVME_CONTROLLER_DISPLAY] = {0};
EFI_STRING_ID gNvmeControllerNameStrIds[MAXIMUM_NVME_CONTROLLER_DISPLAY] = {0};
EFI_STRING_ID gNvmeControllerSizeStrIds[MAXIMUM_NVME_CONTROLLER_DISPLAY] = {0};
EFI_STRING_ID gNvmeControllerNotPresent = 0;
EFI_STRING_ID gNvmeControllerNvmeSize = 0;

/**
    Add/Set the String to HII Database using HiiString Protocol

    @param HiiHandle
    @param String
    @param StringId

    @retval VOID

**/
EFI_STRING_ID
NvmeHiiAddStringInternal (
    IN  EFI_HII_HANDLE  HiiHandle,
    IN  CHAR16 *        String,
    IN  EFI_STRING_ID   StringId
)
{
    EFI_STATUS      Status;
    CHAR8*          Languages = NULL;
    UINTN           LangSize = 0;
    CHAR8*          CurrentLanguage;
    BOOLEAN         LastLanguage = FALSE;

    if(HiiString == NULL) {
        Status = pBS->LocateProtocol(&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
        if(EFI_ERROR(Status)) {
            return 0;
        }
    }

    if(SupportedLanguages == NULL) {
        Status = HiiString->GetLanguages(HiiString, HiiHandle, Languages, &LangSize);
        if(Status == EFI_BUFFER_TOO_SMALL) {
            Status = pBS->AllocatePool(EfiBootServicesData, LangSize, (VOID**)&Languages);
            if(EFI_ERROR(Status)) {
                //
                //not enough resources to allocate string
                //
                return 0;
            }
            Status = HiiString->GetLanguages(HiiString, HiiHandle, Languages, &LangSize);
            if(EFI_ERROR(Status)) {
                return 0;
            }
        }
        SupportedLanguages=Languages;
    } else {
        Languages=SupportedLanguages;
    }

    while(!LastLanguage) {
        //
        //point CurrentLanguage to start of new language
        //
        CurrentLanguage = Languages;
        while(*Languages != ';' && *Languages != 0)
            Languages++;

        //
        //Last language in language list
        //
        if(*Languages == 0) {
            LastLanguage = TRUE;
            if(StringId == 0) {
                Status = HiiString->NewString(HiiString, HiiHandle, &StringId, CurrentLanguage, NULL, String, NULL);
            } else {
                Status = HiiString->SetString(HiiString, HiiHandle, StringId, CurrentLanguage, String, NULL);
            }
            if(EFI_ERROR(Status)) {
                return 0;
            }
        } else {
            //
            //put null-terminator
            //
            *Languages = 0;
            if(StringId == 0) {
                Status = HiiString->NewString(HiiString, HiiHandle, &StringId, CurrentLanguage, NULL, String, NULL);
            } else {
                Status = HiiString->SetString(HiiString, HiiHandle, StringId, CurrentLanguage, String, NULL);
            }
            *Languages = ';';       //restore original character
            Languages++;
            if(EFI_ERROR(Status)) {
                return 0;
            }
        }
    }
    return StringId;
}

/**
    Add the String to HII Database using HiiString Protocol

    @param HiiHandle
    @param String

    @retval VOID

**/
EFI_STRING_ID
NvmeHiiAddString (
    IN  EFI_HII_HANDLE  HiiHandle,
    IN  CHAR16 *        String
)
{
    return NvmeHiiAddStringInternal(HiiHandle, String, 0);
}

/**
    Set the String to HII Database using HiiString Protocol

    @param HiiHandle
    @param String
    @param StringId

    @retval VOID

**/
EFI_STRING_ID
NvmeHiiSetString (
    IN  EFI_HII_HANDLE  HiiHandle,
    IN  CHAR16 *        String,
    IN  EFI_STRING_ID   StringId
)
{
    return NvmeHiiAddStringInternal(HiiHandle, String, StringId);
}

/**
    This function will dynamically add VFR contents to Nvme BIOS setup page
    using HII library functions.

    @param VOID

    @retval VOID

**/
VOID
NvmeInitDynamicMainFormContents(
    VOID
)
{
    VOID                                *StartOpCodeHandle;
    VOID                                *EndOpCodeHandle;
    EFI_IFR_GUID_LABEL                  *StartLabel;
    EFI_IFR_GUID_LABEL                  *EndLabel;
    UINTN                               Index;
    CHAR16                              *String = NULL;
    EFI_STATUS                          Status;
    UINTN                               HandleCount;
    EFI_HANDLE                          *HandleBuffer;
    UINTN                               HandleCountPciIo;
    EFI_HANDLE                          *HandleBufferPciIo;
    EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
    EFI_DEVICE_PATH_PROTOCOL            *DevicePathNode;
    EFI_PCI_IO_PROTOCOL                 *PciIO;
    AMI_NVME_CONTROLLER_PROTOCOL        *NvmeController = NULL;
    EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
    UINTN                               OpenInfoCount;
    UINTN                               IndexPciIo;
    UINTN                               IndexOpenInfo;
    EFI_HANDLE                          DeviceHandle;
    EFI_HANDLE                          ControllerHandle;
    EFI_HANDLE                          DriverBindingHandle;
    EFI_COMPONENT_NAME2_PROTOCOL        *ComponentName2Protocol;
    CHAR16                              *DriveNameUni;
    UINTN                               SegmentNumber;
    UINTN                               BusNumber;
    UINTN                               DeviceNumber;
    UINTN                               FunctionNumber;
    UINT8                               DriveIndex = 0;
    ACTIVE_NAMESPACE_DATA               *ActiveNameSpace;
    LIST_ENTRY                          *LinkData;
    UINT64                              NameSpaceSizeInBytes = 0;
    UINTN                               RemainderInBytes = 0;
    UINT32                              NameSpaceSizeInGB = 0;
    UINTN                               NumTenthsOfGB = 0;
    
    DEBUG((EFI_D_ERROR,"\nNvmeInitDynamicMainFormContents Entry \n"));

    StartOpCodeHandle = HiiAllocateOpCodeHandle ();
    EndOpCodeHandle = HiiAllocateOpCodeHandle ();
    
    if(StartOpCodeHandle == NULL || EndOpCodeHandle == NULL) {
        return;
    }
    
    // Create Hii Extended Label OpCode as the start and end opcode
    StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
            StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));

    EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
            EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    
    if(StartLabel == NULL || EndLabel == NULL) {
        return;
    }
    
    StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
    EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    StartLabel->Number = NVME_MAIN_FORM_LABEL_START;
    EndLabel->Number = NVME_MAIN_FORM_LABEL_END;
    
    // Collect all DevicePath protocol and check for NVMe Controller
    Status = pBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                 );
    
    if (EFI_ERROR(Status)) {
        return;
    }
    
    for (Index = 0; (Index < HandleCount)&&(DriveIndex < MAXIMUM_NVME_CONTROLLER_DISPLAY); Index++) {
        
        Status = pBS->HandleProtocol(
                      HandleBuffer[Index],
                      &gEfiDevicePathProtocolGuid,
                      (VOID *)&DevicePath
                      );
        
         ASSERT_EFI_ERROR(Status);
         if(EFI_ERROR(Status)) {
             continue;
         }
         
         DevicePathNode = DevicePath;
         while (!isEndNode (DevicePathNode)) {
             if ((DevicePathNode->Type == MESSAGING_DEVICE_PATH) &&
                     (DevicePathNode->SubType == MSG_NVME_DP)) {
                 break;
             }

             DevicePathNode = NEXT_NODE(DevicePathNode);
        }
         
        if (DevicePathNode == NULL || isEndNode (DevicePathNode)) {
            continue;
        }
        
        // NVMe Device Handle is found.
        DeviceHandle = HandleBuffer[Index];
        
        // NVMe device is found. Now get the CONTROLLER. Check all the PCIio handles.
        Status = pBS->LocateHandleBuffer(
                     ByProtocol,
                     &gEfiPciIoProtocolGuid,
                     NULL,
                     &HandleCountPciIo,
                     &HandleBufferPciIo
                     );
        
        for (IndexPciIo = 0; IndexPciIo < HandleCountPciIo; IndexPciIo++) {
            Status = pBS->HandleProtocol(
                          HandleBufferPciIo[IndexPciIo],
                          &gEfiPciIoProtocolGuid,
                          (VOID *)&PciIO
                          );
                    
            ASSERT_EFI_ERROR(Status);
            if(EFI_ERROR(Status)) {
                continue;
            }
            
            Status = pBS->HandleProtocol(
                          HandleBufferPciIo[IndexPciIo],
                          &gAmiNvmeControllerProtocolGuid,
                          (VOID *)&NvmeController
                          );
            
            // If Ami Nvme Controller Protocol not found on the Controller handle ( PciIo handle)
            // do not process the Pci Io Handle
            if(EFI_ERROR(Status)) {
                continue;
            }
            
            OpenInfoCount = 0;
            Status = pBS->OpenProtocolInformation(
                          HandleBufferPciIo[IndexPciIo],
                          &gEfiPciIoProtocolGuid,
                          &OpenInfo,
                          &OpenInfoCount
                          );
            
            ASSERT_EFI_ERROR(Status);
            if(EFI_ERROR(Status)) {
                continue;
            }                
            
            for (IndexOpenInfo = 0;( IndexOpenInfo < OpenInfoCount)&&(DriveIndex < MAXIMUM_NVME_CONTROLLER_DISPLAY); IndexOpenInfo++) {
                
                //Check if the handle is opened BY_CHILD and also compare the device handle.
                if ((OpenInfo[IndexOpenInfo].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) &&
                        (OpenInfo[IndexOpenInfo].ControllerHandle == DeviceHandle)){
                
                    DriverBindingHandle = OpenInfo[IndexOpenInfo].AgentHandle;
                    // Get the handle for the Controller
                    ControllerHandle = HandleBufferPciIo[IndexPciIo]; 
                    
                    // Now PCI controller and DriverBinding handle is found. Get the Component2Protocol now.
                    Status = pBS->HandleProtocol(
                                  DriverBindingHandle,
                                  &gEfiComponentName2ProtocolGuid,
                                  (VOID *)&ComponentName2Protocol
                                  );
                    
                    ASSERT_EFI_ERROR(Status);
                    if(EFI_ERROR(Status)) {
                        continue;
                    }     
                    
                    Status = ComponentName2Protocol->GetControllerName (
                                                     ComponentName2Protocol,
                                                     ControllerHandle,
                                                     DeviceHandle,
                                                     LANGUAGE_CODE_ENGLISH,
                                                     &DriveNameUni
                                                     );
                    
                    ASSERT_EFI_ERROR(Status);
                    if(EFI_ERROR(Status)) {
                        continue;
                    }     
                    
                    Status = pBS->AllocatePool( EfiBootServicesData,
                                       (UINTN)50,
                                       (VOID**)&String );
                    ASSERT_EFI_ERROR(Status);
                    
                    gNvmeControllerNameStrIds[DriveIndex] = NvmeHiiAddString(gNvmeHiiHandle, DriveNameUni);
                    
                    pBS->SetMem(String, (UINTN)50, 0);
                                          
                    Status = PciIO->GetLocation (
                                     PciIO,
                                     &SegmentNumber,
                                     &BusNumber,
                                     &DeviceNumber,
                                     &FunctionNumber
                                     );
                    ASSERT_EFI_ERROR(Status);
      
                    Swprintf(String, L"Bus:%X Dev:%X Func:%X", BusNumber, DeviceNumber, FunctionNumber);
                    gNvmeControllerLocationStrIds[DriveIndex] = NvmeHiiAddString(gNvmeHiiHandle, String);
                    
                    pBS->SetMem(String, (UINTN)50, 0);
                    NumTenthsOfGB=0;
                    NameSpaceSizeInGB=0;
                    for (LinkData = NvmeController->ActiveNameSpaceList.ForwardLink; 
                                LinkData != &NvmeController->ActiveNameSpaceList; 
                                LinkData = LinkData->ForwardLink) {
                            
                        ActiveNameSpace = BASE_CR(LinkData ,ACTIVE_NAMESPACE_DATA, Link);
                    
                        // NameSpaceSize In Bytes
                        NameSpaceSizeInBytes = Mul64((UINT64)ActiveNameSpace->IdentifyNamespaceData.NSIZE,\
                                                   ActiveNameSpace->NvmeBlockIO.Media->BlockSize);
                        
                        // NameSpaceSizeInGB is NameSpaceSizeInBytes / 1 GB (1 Decimal GB = 10^9 bytes)
                        NameSpaceSizeInGB += (UINT32) Div64(NameSpaceSizeInBytes, 1000000000, &RemainderInBytes);
                     
                        // Convert the Remainder, which is in bytes, to number of tenths of a Decimal GB.
                        NumTenthsOfGB += RemainderInBytes / 100000000;
                       
                     }

                     // Display Total Size of Nvme (All ActiveNameSpace Size) in GB
                    Swprintf(String, L"%d.%dGB", NameSpaceSizeInGB, NumTenthsOfGB);
                    gNvmeControllerSizeStrIds[DriveIndex] = NvmeHiiAddString(gNvmeHiiHandle, String);
                    
                    DriveIndex++;
                    
                    if (String!= NULL) {
                        pBS->FreePool(String);
                    }
                }
            }
            pBS->FreePool(OpenInfo);              
        }
        pBS->FreePool(HandleBufferPciIo);
    }

    pBS->FreePool(HandleBuffer);
    
    HiiCreateSubTitleOpCode(StartOpCodeHandle,
            STRING_TOKEN(STR_NVME_SUBTITLE_STRING),
            STRING_TOKEN(STR_EMPTY),
            0,0);
    
    HiiCreateSubTitleOpCode(StartOpCodeHandle,
            STRING_TOKEN(STR_EMPTY),
            STRING_TOKEN(STR_EMPTY),
            0,0);
    
    if (DriveIndex == 0) {
        Status = pBS->AllocatePool( EfiBootServicesData,
                           (UINTN)50,
                           (VOID**)&String );
        ASSERT_EFI_ERROR(Status);
        
        Swprintf(String, L"No NVME Device Found");
        gNvmeControllerNotPresent = NvmeHiiAddString(gNvmeHiiHandle, String);
        
        HiiCreateSubTitleOpCode(StartOpCodeHandle,
                gNvmeControllerNotPresent,
                STRING_TOKEN(STR_EMPTY),
                0,0);
        
        if (String!= NULL) {
            pBS->FreePool(String);
        }
    } else {
        
        Status = pBS->AllocatePool( EfiBootServicesData,
                                   (UINTN)50,
                                   (VOID**)&String );
        ASSERT_EFI_ERROR(Status);
        
        Swprintf(String, L"Nvme Size");
        gNvmeControllerNvmeSize = NvmeHiiAddString(gNvmeHiiHandle, String);
        
        for(Index = 0; Index < DriveIndex; Index++) {
            // Updating NVME controller information dynamically
            HiiCreateTextOpCode(StartOpCodeHandle,
                    gNvmeControllerLocationStrIds[Index],
                    STRING_TOKEN(STR_EMPTY),
                    gNvmeControllerNameStrIds[Index]);
            
            HiiCreateTextOpCode(StartOpCodeHandle,
                    gNvmeControllerNvmeSize,
                    STRING_TOKEN(STR_EMPTY),
                    gNvmeControllerSizeStrIds[Index]);
            
            HiiCreateSubTitleOpCode(StartOpCodeHandle,
                    STRING_TOKEN(STR_EMPTY),
                    STRING_TOKEN(STR_EMPTY),
                    0,0);
        }
        if (String!= NULL) {
            pBS->FreePool(String);
        }
    }
    
    // We are done!! Updating form
    HiiUpdateForm (
      gNvmeHiiHandle,
      &gNvmeFormSetGuid,
      NVME_MAIN_FORM_ID,
      StartOpCodeHandle,
      EndOpCodeHandle
    );
    
    if (StartOpCodeHandle != NULL) {
        HiiFreeOpCodeHandle (StartOpCodeHandle);
    }
    
    if (EndOpCodeHandle != NULL) {
        HiiFreeOpCodeHandle (EndOpCodeHandle);
    }
    
    DEBUG((EFI_D_ERROR,"\nNvmeInitDynamicMainFormContents Exit \n"));
}

/**
    This function initializes the Nvme IFR contents.

    @param EFI_EVENT Event - Event to signal
    @param VOID* Context - Event specific context

    @retval VOID

**/

VOID
EFIAPI
InitNvmeDynamicIfrContents (
        IN EFI_EVENT Event,
        IN VOID *Context
)
{
    NvmeInitDynamicMainFormContents();
}

/**
    This function is the entry point. Registers callback on TSE event to update IFR data.

    @param  EFI_HANDLE          ImageHandle
    @param  EFI_SYSTEM_TABLE    *SystemTable

    @retval Status

**/
EFI_STATUS
EFIAPI
NvmeDynamicSetupEntryPoint (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS      Status;
    VOID            *SetupRegistration;
    EFI_GUID        SetupEnterGuid = AMITSE_SETUP_ENTER_GUID;
    EFI_EVENT       SetupEnterEvent;
    
    InitAmiLib(ImageHandle, SystemTable);
    
    RegisterProtocolCallback(
        &SetupEnterGuid, InitNvmeDynamicIfrContents,
        NULL, &SetupEnterEvent, &SetupRegistration
    );
    
    // Loads ImageHandle related strings
    Status = LoadStrings( ImageHandle, &gNvmeHiiHandle );
    ASSERT_EFI_ERROR(Status);

    return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**           5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093      **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
