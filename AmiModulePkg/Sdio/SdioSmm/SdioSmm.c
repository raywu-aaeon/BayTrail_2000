//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioSmm.c 8     3/07/12 4:09a Rajeshms $
//
// $Revision: 8 $
//
// $Date: 3/07/12 4:09a $
//**********************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SdioSmm.c
//
// Description: Smm driver to handle the SIO MMIO operations
//
//<AMI_FHDR_END>
//**********************************************************************

#include "SdioSmm.h"
#include "SdioController.h"
#include <AmiDxeLib.h>
#include <Setup.h>

#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
#else
EFI_GUID    gEfiSmmBaseProtocolGuid    = EFI_SMM_BASE_PROTOCOL_GUID;
EFI_GUID    gSwDispatchProtocolGuid      = EFI_SMM_SW_DISPATCH_PROTOCOL_GUID;
#endif
EFI_GUID    gSdioSmmNonSmmProtocolGuid      = SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL_GUID;
typedef VOID        (*API_FUNC)(SDIO_STRUC*);
SDIO_GLOBAL_DATA    *gSdioData;
UINT8               SDIO_Access_Mode;
SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL *gSdioSmmNonSmmInterface = NULL;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:        SdioAPITable - SDIO API Function Dispatch Table
//
// Type:        Function Dispatch Table
//
// Description: This is the table of functions used by SDIO Storage API
//
//----------------------------------------------------------------------------
//<AMI_THDR_END>

API_FUNC SdioApiTable[] = {
    SdioAPICheckDevicePresence,
    SdioApiConfigureDevice,
    SdioAPIGetMassDeviceInformation,
    SdioAPIRead,
    SdioAPIWrite,
    SdioApiDeviceGeometry,
    SioApiReset
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioSmmDriverEntryPoint
//
// Description: This is the entrypoint of the SDIO Smm driver.
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SdioSmmDriverEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
    InitAmiLib(ImageHandle, SystemTable);

    //
    // Perform SMM registartion when SDIO_SMM_SUPPORT is enabled
    //
#if SDIO_SMM_SUPPORT
    return InitSmmHandler(ImageHandle, SystemTable, SdioInSmmFunction, NotInSmmFunction);
#endif

    return EFI_SUCCESS;


}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetSetSdioAccessMode
//
// Description: Get the SDIO Access mode from Setup and Set the SDIO_Access_Mode 
//              Global variable
//
// Input:
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID GetSetSdioAccessMode(
 )
{

    EFI_STATUS  Status;
    EFI_GUID    gSetupGuid = SETUP_GUID;
    SDIO_DEV_CONFIGURATION  SdioConfiguration;
    UINTN       VariableSize=sizeof(SDIO_DEV_CONFIGURATION);
    UINT8       Index;

    Status = pRS->GetVariable(L"SdioDevConfiguration",&gSetupGuid,
                              NULL,&VariableSize, &SdioConfiguration);

    if(EFI_ERROR(Status)) {
        //
        // Error in Getting variable. Set the mode to Auto
        //
        SDIO_Access_Mode = 0;    
    } else {
        //
        // Initialize the Sdio Access mode and emulation type from Setup.
        //
        SDIO_Access_Mode = SdioConfiguration.SdioMode;

        for (Index = 0; Index < MAX_SDIO_DEVICES; Index++) {
            gSdioData->SDIOMassEmulationOptionTable[Index] =
                            *((UINT8*)&SdioConfiguration.SdioEmu1+Index);
        }        
    }        

    return;
}

#if SDIO_SMM_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioInSmmFunction
//
// Description: This function is called from SMM during SMM registration.
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SdioInSmmFunction(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS  Status;
    EFI_HANDLE                      SwHandle = NULL;
#if PI_SPECIFICATION_VERSION >= 0x1000A
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch;

    EFI_SMM_SW_REGISTER_CONTEXT     SwContext;
#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext;
#endif


#if PI_SPECIFICATION_VERSION >= 0x1000A

    Status = InitAmiSmmLib( ImageHandle, SystemTable );
    
    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &gSmmBase2);
    
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    Status = pSmmBase->GetSmstLocation (gSmmBase2, &pSmst);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;


    Status = pSmst->SmmLocateProtocol( \
                        &gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

#else
    //
    // Register the SDIO SW SMI handler
    //
    Status = pBS->LocateProtocol (&gSwDispatchProtocolGuid, NULL, &pSwDispatch);

    if (EFI_ERROR (Status)) {
        return Status;
    }
#endif
    SwContext.SwSmiInputValue = SDIO_SWSMI;
    Status = pSwDispatch->Register (pSwDispatch, SdioSWSMIHandler, &SwContext, &SwHandle);


    if (EFI_ERROR (Status)) {
        return Status;
    }
    //
    //Allocate Memory for SDIO global Data.
    //
    Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData,sizeof(SDIO_GLOBAL_DATA), &gSdioData);
	ASSERT_EFI_ERROR(Status);
    //
    //  Clear the Buffer
    //
    ZeroMemorySmm((VOID*)gSdioData, sizeof(SDIO_GLOBAL_DATA));

    //
    // Get the SDIO access mode from Setup.
    //
    GetSetSdioAccessMode();

    //
    // Locating Protocol
    //
    Status=pBS->LocateProtocol(&gSdioSmmNonSmmProtocolGuid,NULL,&gSdioSmmNonSmmInterface);
    ASSERT_EFI_ERROR(Status);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NotInSmmFunction
//
// Description: This function is called from outside of SMM during SMM registration.
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NotInSmmFunction(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SdioSWSMIHandler
//
// Description: Invoked on reads from SW SMI port with value SDIO_SWSMI. This
//              function dispatches the SDIO Request Packets (URP) to the
//              appropriate functions.
//
// Input:       EBDA:SDIO_DATA_EBDA_OFFSET - Pointer to the URP (SDIO Request
//              Packet structure)
//              DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
//
// Output:      bRetValue   Zero on successfull completion
//              Non-zero on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS
SdioSWSMIHandler (
    EFI_HANDLE                  DispatchHandle,
    EFI_SMM_SW_REGISTER_CONTEXT *DispatchContext,
    IN OUT EFI_SMM_SW_CONTEXT   *SwContext,
    IN OUT UINTN                *CommBufferSize
)
#else
VOID
SdioSWSMIHandler (
    EFI_HANDLE                  DispatchHandle,
    EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext
)
#endif
{
#if PI_SPECIFICATION_VERSION >= 0x1000A
	EFI_STATUS   Status =EFI_SUCCESS;
#endif	
    SDIO_STRUC  *SdioURP=NULL;
    UINT8       bFuncIndex;
    UINT8       bNumberOfFunctions;
    UINT16      EbdaSeg;

    //
    // Checking whether the SMI is generated from EFI or Legacy.
    // If the SMI is generated from EFI, the SDIO_STRUC address is taken \
    // from the interface. If the SMI is from Legacy, the SDIO_STRUC \
    // address is taken from EBDA.
    //
    if(gSdioSmmNonSmmInterface->Address != 0) {
        //
        // Obtaining the SDIO_STRUC Address
        //
        SdioURP = (SDIO_STRUC*)(gSdioSmmNonSmmInterface->Address);
        gSdioSmmNonSmmInterface->Address = 0;
    } else {
        //
        // Get the fpURP pointer from EBDA
        //
        EbdaSeg = *((UINT16*)0x40E);
        SdioURP = *(SDIO_STRUC**)(UINTN)(((UINT32)EbdaSeg << 4) + SDIO_DATA_EBDA_OFFSET);
        SdioURP = (SDIO_STRUC*)((UINTN)SdioURP & 0xFFFFFFFF);
    }

    bFuncIndex = SdioURP->bFuncNumber;
    bNumberOfFunctions = sizeof SdioApiTable / sizeof (API_FUNC *);

    //
    // Make sure function number is valid; if function number is not zero
    // check for valid extended SDIO API function
    //
    if (bFuncIndex >= bNumberOfFunctions ) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
        RETURN(Status);
    }

    //
    // Call the appropriate function
    //

    SdioApiTable[bFuncIndex](SdioURP);

    RETURN(Status);
}
#endif //#if SDIO_SMM_SUPPORT

#if !SDIO_SMM_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SdioAPIHandler
//
// Description: 
//  This function dispatches the SDIO Request Packets (URP) to the
//   appropriate functions when SDIO_SMM_SUPPORT is disabled.
//
// Input:       
//   IN    SDIO_STRUC   *SdioURP - Pointer to the URP (SDIO Request
//              Packet structure)
//
// Output:      SdioURP->bRetValue   Zero on successfull completion
//              Non-zero on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SdioAPIHandler (
    IN    SDIO_STRUC   *SdioURP
)
{
    EFI_STATUS  Status;
	UINT8       bFuncIndex;
    UINT8       bNumberOfFunctions;
    static BOOLEAN InitSdioAPI = FALSE;
    
    
    //
    // Initialize SDIO global buffers
    //
    if (!InitSdioAPI) {

        Status = pBS->AllocatePool (EfiBootServicesData,
                                    sizeof(SDIO_GLOBAL_DATA),
                                    (VOID**)&gSdioData
                                    );
        if (EFI_ERROR(Status)) {
            return;
        }
        //
        //  Clear the Buffer
        //
        pBS->SetMem((VOID*)gSdioData, sizeof(SDIO_GLOBAL_DATA),0);

        //
        // Get the SDIO access mode from Setup.
        //
        GetSetSdioAccessMode();
        InitSdioAPI = TRUE;
    }
    bFuncIndex = SdioURP->bFuncNumber;
    bNumberOfFunctions = sizeof SdioApiTable / sizeof (API_FUNC *);

    //
    // Make sure function number is valid; if function number is not zero
    // check for valid extended SDIO API function
    //
    if (bFuncIndex >= bNumberOfFunctions ) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
        return;
    }

    //
    // Call the appropriate function
    //
    SdioApiTable[bFuncIndex](SdioURP);

    return;
}
#endif 

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ZeromemorySmm
//
// Description: Clears the buffer
//
// Input:       void    *Buffer,
//              UINTN   Size
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
void
ZeroMemorySmm (
    void                            *Buffer,
    UINTN                           Size
 )
{
    UINT8   *Ptr;
    Ptr = Buffer;
    while (Size--) {
        *(Ptr++) = 0;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetFreeDevEntry
//
// Description: Get the Free Entry Index from SDIO Device Buffer
//
// Input:
//
// Output:      Entry No
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 GetFreeDevEntry (
)
{
    UINT8   i;

    for(i=0;i<MAX_SDIO_DEVICES;i++) {
        if(gSdioData->SdioDev[i].DevEntryUsed == FALSE) {
            return i;
        }
    }

    return 0xFF;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetDevEntry
//
// Description: Get the Index no for the DeviceAddress
//
// Input:       UINT8   DeviceAddress
//
// Output:      DeviceIndex
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 GetDevEntry (
    IN  UINT8   DeviceAddress
)
{
    UINT8   i;

    for(i=0;i<MAX_SDIO_DEVICES;i++) {
        if(gSdioData->SdioDev[i].DeviceAddress == DeviceAddress) {
            return i;
        }
    }

    return 0xFF;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioAPICheckDevicePresence
//
// Description: Check the device presence and return the status
//
// Input:
//              IN SDIO_STRUC       Input details from caller
//
// Output:      Output information stored in SDIO_STRUC
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SdioAPICheckDevicePresence(
    SDIO_STRUC *SdioURP
)
{
    SDIO_DEVICE_INFO    *SdioDevInfo;
    EFI_STATUS          Status;
    UINT8               Port;
    UINT8               DevIndex;

    DevIndex=GetFreeDevEntry();
    if(DevIndex == 0xFF) {
        SdioURP->ApiData.ControllerInfo.DeviceDetected=FALSE;
        return;
    }

    gSdioData->TransferBufferAddress=SdioURP->ApiData.ControllerInfo.TransferBufferAddress;
    gSdioData->SdioDev[DevIndex].SdioBaseAddress=SdioURP->ApiData.ControllerInfo.SdioBaseAddress;
    SdioDevInfo=&gSdioData->SdioDev[DevIndex];
    Port=SdioURP->ApiData.ControllerInfo.Port;

    //
    // Check the Device Presence
    //
    Status = CheckDevicePresence_Controller (SdioDevInfo, Port);

    if(EFI_ERROR(Status)) {
        SdioURP->ApiData.ControllerInfo.DeviceDetected=FALSE;
        return;
    }

    //
    // Device is present in the SDIO Port
    //
    SdioDevInfo->DevEntryUsed=TRUE;
    SdioDevInfo->DeviceAddress=DevIndex+1;
    SdioURP->ApiData.ControllerInfo.DeviceAddress=SdioDevInfo->DeviceAddress;
    SdioURP->ApiData.ControllerInfo.DeviceDetected=TRUE;
    SdioURP->bRetValue = SDIO_SUCCESS;

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioApiConfigureDevice
//
// Description: Configure the Device 
//
// Input:
//              IN SDIO_STRUC       Input details from caller
//
// Output:      Output information stored in SDIO_STRUC
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SdioApiConfigureDevice(
   SDIO_STRUC *SdioURP
)
{

    SDIO_DEVICE_INFO    *SdioDevInfo;
    UINT8               DevEntry;
    UINT8               Port;
    EFI_STATUS          Status;
//    UINT8               i;

    //
    // Get the Device Entry
    //    
    DevEntry=GetDevEntry(SdioURP->ApiData.ControllerInfo.DeviceAddress);
    if(DevEntry == 0xFF) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
    }

    SdioDevInfo=&gSdioData->SdioDev[DevEntry];
    Port=SdioURP->ApiData.ControllerInfo.Port;

    //
    // Check the device in SD slot is I/O device
    //
#if CHECK_FOR_SD_IO_DEVICE
    Status= SdCard_CheckSdIoDevice(SdioDevInfo,Port);
    if(!EFI_ERROR(Status)) {
        if(SdioDevInfo->IODevice == TRUE) {
            SdioURP->ApiData.ControllerInfo.SdIODevice=TRUE;
            for(i=0;i<SDIO_MANUFACTUREID_LENGTH;i++) {
                SdioURP->ApiData.ControllerInfo.SdIOManufactureId[i] =SdioDevInfo->SdIOManufactureId[i];
            }
            //
            //Put back the device into Reset state. So give Reset all Command 
            //
            SDIO_ResetAll(SdioDevInfo, Port);
            SdioURP->bRetValue = SDIO_SUCCESS;
            return;
        }
    }
#endif

    //
    // Configure the Memory type Device
    //
    Status = ConfigureMassDevice_Controller (SdioDevInfo, Port);

    if(EFI_ERROR(Status)) {
        //
        // Device Configuration Failed.
        //    
        SdioURP->bRetValue = SDIO_ERROR;
        SdioURP->ApiData.ControllerInfo.DeviceDetected=FALSE;
        //
        //Put back the device into Reset state. So give Reset all Command 
        //
        SDIO_ResetAll(SdioDevInfo, Port);
        return;
    }

    SdioURP->bRetValue = SDIO_SUCCESS;

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioAPIGetMassDeviceInformation
//
// Description: Initilize the Device and Returns the Device Information
//
// Input:
//              IN SDIO_STRUC       Input details from caller
//
// Output:      Output information stored in SDIO_STRUC
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SdioAPIGetMassDeviceInformation (
    SDIO_STRUC *SdioURP
)
{
    SDIO_DEVICE_INFO    *SdioDevInfo;
    EFI_STATUS          Status;
    UINT8               Port;
    UINT8               i;
    UINT8               DevEntry;

    DevEntry=GetDevEntry(SdioURP->ApiData.ControllerInfo.DeviceAddress);
    if(DevEntry == 0xFF) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
    }
    SdioDevInfo=&gSdioData->SdioDev[DevEntry];
    Port=SdioURP->ApiData.ControllerInfo.Port;

    //
    // Set Emulation type from Setup Option
    //
    SdioDevInfo->wEmulationOption = gSdioData->SDIOMassEmulationOptionTable[DevEntry];

    //
    // Get the SDIO mass device Information.
    //
    Status = GetDeviceInformation (SdioDevInfo, Port);

    if(EFI_ERROR(Status)) {
        SdioURP->bRetValue = SDIO_ERROR;
        SdioURP->ApiData.ControllerInfo.DeviceDetected=FALSE;
        //
        //Put back the device into Reset state. So give Reset all Command 
        //
        SDIO_ResetAll(SdioDevInfo, Port);
        return;
    }


    //
    // Return the Mass Storage Device Information.
    //
    SdioURP->ApiData.ControllerInfo.MaxLBA=SdioDevInfo->dMaxLBA;
    SdioURP->ApiData.ControllerInfo.BlockSize=SdioDevInfo->wBlockSize;

    SdioURP->ApiData.ControllerInfo.NumHeads=SdioDevInfo->NumHeads;
    SdioURP->ApiData.ControllerInfo.NumSectors=SdioDevInfo->NumSectors;
    SdioURP->ApiData.ControllerInfo.NumCylinders=SdioDevInfo->NumCylinders;
    SdioURP->ApiData.ControllerInfo.LBANumHeads=SdioDevInfo->LBANumHeads;
    SdioURP->ApiData.ControllerInfo.LBANumSectors=SdioDevInfo->LBANumSectors;
    SdioURP->ApiData.ControllerInfo.LBANumCyls=SdioDevInfo->LBANumCyls;
    SdioURP->ApiData.ControllerInfo.StorageType=SdioDevInfo->bStorageType;

    for(i=0;i<27;i++) {
        SdioURP->ApiData.ControllerInfo.PNM[i] =SdioDevInfo->PNM[i];
    }
    SdioURP->bRetValue = SDIO_SUCCESS;

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioAPIRead
//
// Description: Read the Data from the Device
//
// Input:
//              IN SDIO_STRUC       Input details from caller
//
// Output:      Output information stored in SDIO_STRUC
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SdioAPIRead (
    SDIO_STRUC *SdioURP
)
{
    SDIO_DEVICE_INFO    *SdioDevInfo;
    EFI_STATUS          Status;
    UINT8               Port;
    UINT32              Lba;
    UINT16              NumBlks;
    UINT32              BufferAddress;
    UINT8               DevEntry;

    DevEntry=GetDevEntry(SdioURP->ApiData.Read.DeviceAddress);
    if(DevEntry == 0xFF) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
    }
    SdioDevInfo=&gSdioData->SdioDev[DevEntry];

    Port=SdioURP->ApiData.Read.Port;
    Lba=SdioURP->ApiData.Read.LBA;
    NumBlks=SdioURP->ApiData.Read.NumBlks;
    BufferAddress=(UINT32)SdioURP->ApiData.Read.BufferAddress;

    //
    // If "Forced FDD" option is selected that means the device has
    // to be emulated as a floppy drive even though it has a HDD emulated
    // image.  This is accomplished by hiding the first cylinder totally.
    // The partition table is in the first cylinder.  LBA value for all
    // the requests to the device will be offset with the number of sectors
    // in the cylinder.
    //

    //
    // Check for forced floppy emulated device and change LBA accordingly
    //
    if (SdioDevInfo->bEmuType == SDIO_EMU_FORCED_FDD) {
        //
        // Skip first track in case of floppy emulation
        //
        Lba += SdioDevInfo->bHiddenSectors;
    }

    Status = SDIOAPI_ReadCard_Controller(SdioDevInfo,Port,(EFI_LBA)Lba, (UINT32)NumBlks,(VOID*)BufferAddress);

    if(EFI_ERROR(Status)) {
        SdioURP->bRetValue = SDIO_GENERAL_FAILURE;
    } else {
        //
        // Check for forced floppy emulated device
        //
        if ((SdioDevInfo->bEmuType == SDIO_EMU_FORCED_FDD) &&
            (SdioURP->ApiData.Read.LBA == 0)) {
            //
            // This is a floppy emulated ZIP drive, with read to
            // first sector. Update the boot record so that floppy
            // emulation is okay.
            //
            // Force #of hidden sectors to 0
            //
            *(UINT32*)(BufferAddress + 0xB + 0x11) = 0;

            //
            // FreeDOS workaround
            //
            if ((*(UINT32*)((UINTN)BufferAddress + 3) == 0x65657246) && // 'eerF'
                (*(UINT32*)((UINTN)BufferAddress + 7) == 0x20534F44) && // ' SOD'
                (*(UINT32*)((UINTN)BufferAddress + 0x3A) == 0x20202032)) {
                    *(UINT16*)((UINTN)BufferAddress + 0x42) =
                        *(UINT16*)((UINTN)BufferAddress + 0x42)-(UINT16)SdioDevInfo->bHiddenSectors;
                    *(UINT16*)((UINTN)BufferAddress + 0x46) =
                        *(UINT16*)((UINTN)BufferAddress + 0x46)-(UINT16)SdioDevInfo->bHiddenSectors;
                    *(UINT16*)((UINTN)BufferAddress + 0x4A) =
                        *(UINT16*)((UINTN)BufferAddress + 0x4A)-(UINT16)SdioDevInfo->bHiddenSectors;
            }
            //
            // Force physical drive# to 0
            // For FAT32, physical drive number is present in offset 40h
            //
            if ((*(UINT32*)((UINTN)BufferAddress + 0x52)) == 0x33544146) { // '3TAF' for FAT3
                *(UINT8*)((UINTN)BufferAddress + 0x40) = 0;
            } else {
                *(UINT8*)((UINTN)BufferAddress + 0x24) = 0;
            }
        }

        SdioURP->bRetValue = SDIO_SUCCESS;
    }

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioAPIWrite
//
// Description: Write the Data to the Device
//
// Input:
//              IN SDIO_STRUC       Input details from caller
//
// Output:      Output information stored in SDIO_STRUC
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SdioAPIWrite (
    SDIO_STRUC *SdioURP
)
{
    SDIO_DEVICE_INFO    *SdioDevInfo;
    EFI_STATUS          Status;
    UINT8               Port;
    UINT32              Lba;
    UINT16              NumBlks;
    UINT32              BufferAddress;
    UINT8               DevEntry;
    #if SDIO_WriteProtect
    UINT32              Temp32;
    #endif

    DevEntry=GetDevEntry(SdioURP->ApiData.Read.DeviceAddress);
    if(DevEntry == 0xFF) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
    }

    SdioDevInfo=&gSdioData->SdioDev[DevEntry];

    Port=SdioURP->ApiData.Read.Port;
    Lba=SdioURP->ApiData.Read.LBA;
    NumBlks=SdioURP->ApiData.Read.NumBlks;
    BufferAddress=(UINT32)SdioURP->ApiData.Read.BufferAddress;

    #if SDIO_WriteProtect        
    Temp32=SDIO_REG32(SdioDevInfo->SdioBaseAddress,PSTATE);
    if(!(Temp32 & PSTATE_WP)) {
        SdioURP->bRetValue = SDIO_WRITE_PROTECT_ERR;        
        return;
    }
    #endif

    //
    // If "Forced FDD" option is selected that means the device has
    // to be emulated as a floppy drive even though it has a HDD emulated
    // image.  This is accomplished by hiding the first cylinder totally.
    // The partition table is in the first cylinder.  LBA value for all
    // the requests to the device will be offset with the number of sectors
    // in the cylinder.
    //

    //
    // Check for forced floppy emulated device and change LBA accordingly
    //
    if (SdioDevInfo->bEmuType == SDIO_EMU_FORCED_FDD) {
        //
        // Skip first track in case of floppy emulation
        //
        Lba += SdioDevInfo->bHiddenSectors;
    }

    Status = SDIOAPI_WriteCard_Controller(SdioDevInfo,Port,(EFI_LBA)Lba, (UINT32)NumBlks,(VOID*)BufferAddress);

    if(EFI_ERROR(Status)) {
        SdioURP->bRetValue = SDIO_GENERAL_FAILURE;
    } else {
        SdioURP->bRetValue = SDIO_SUCCESS;
    }

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioApiDeviceGeometry
//
// Description: Get the Device Geometry
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SdioApiDeviceGeometry (
    SDIO_STRUC *SdioURP
)
{

    SDIO_DEVICE_INFO    *SdioDevInfo;
    UINT8               DevEntry;

    DevEntry=GetDevEntry(SdioURP->ApiData.Read.DeviceAddress);
    if(DevEntry == 0xFF) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
    }
    SdioDevInfo=&gSdioData->SdioDev[DevEntry];

    SdioURP->ApiData.DeviceGeo.NumHeads=SdioDevInfo->NumHeads;
    SdioURP->ApiData.DeviceGeo.NumCylinders=SdioDevInfo->NumCylinders;
    SdioURP->ApiData.DeviceGeo.NumSectors=SdioDevInfo->NumSectors;
    SdioURP->ApiData.DeviceGeo.LBANumHeads=SdioDevInfo->LBANumHeads;
    SdioURP->ApiData.DeviceGeo.LBANumCyls=SdioDevInfo->LBANumCyls;
    SdioURP->ApiData.DeviceGeo.LBANumSectors=SdioDevInfo->LBANumSectors;
    SdioURP->ApiData.DeviceGeo.BlockSize=SdioDevInfo->wBlockSize;
    SdioURP->ApiData.DeviceGeo.MaxLBA=SdioDevInfo->dMaxLBA;

    SdioURP->bRetValue = SDIO_SUCCESS;

    return;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SioApiReset
//
// Description: Reset the Device
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SioApiReset (
    SDIO_STRUC *SdioURP
)
{

    SDIO_DEVICE_INFO    *SdioDevInfo;
    EFI_STATUS          Status;
    UINT8               DevEntry;

    DevEntry=GetDevEntry(SdioURP->ApiData.Read.DeviceAddress);
    if(DevEntry == 0xFF) {
        SdioURP->bRetValue = SDIO_PARAMETER_FAILED;
    }

    SdioDevInfo=&gSdioData->SdioDev[DevEntry];

    Status=SDIOAPI_ResetCard(SdioDevInfo,SdioDevInfo->PortNumber);

    if(EFI_ERROR(Status)) {
        SdioURP->bRetValue = SDIO_GENERAL_FAILURE;
    } else {
        SdioURP->bRetValue = SDIO_SUCCESS;
    }

    return;

}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
