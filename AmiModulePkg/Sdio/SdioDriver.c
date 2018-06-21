//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file SdioDriver.c
    SDIO controller detection and configuration.
**/
#define SDIO_WRITE_PROTECT_ERR          0x003   // Write protect error

//----------------------------------------------------------------------

#include "SdioDriver.h"
#include <Protocol/SdioInt13Protocol.h>
#include <Protocol/SmmControl2.h>
#include <Protocol/SmmCommunication.h>
#include <SdioElink.h>

//----------------------------------------------------------------------

#if ( defined(BOOT_SECTOR_WRITE_PROTECT) && (BOOT_SECTOR_WRITE_PROTECT != 0) )
#include <Protocol/AmiBlockIoWriteProtection.h>
AMI_BLOCKIO_WRITE_PROTECTION_PROTOCOL *AmiBlkWriteProtection = NULL;
#endif


UINT8   *gSdioReadData = NULL;
UINT8   *SDDevnameForPNMZero  = (UINT8 *)"SD Memory Card - Device ";
UINT8   *MMCDevnameForPNMZero = (UINT8 *)"MMC Memory Card - Device ";
CHAR16  *gSdioBusDriverName = L"AMI SDIO Driver";
SDIO_DEV_CONFIGURATION gSdioConfiguration;

// EFI Driver Binding Protocol Instance
EFI_DRIVER_BINDING_PROTOCOL gSdioDriverBinding = {
    SdioDriverSupported,
    SdioDriverStart,
    SdioDriverStop,
    0x10,
    NULL,
    NULL
};

EFI_COMPONENT_NAME2_PROTOCOL gSdioCtlDriverName = {
    SdioCtlDriverName,
    SdioCtlGetControllerName,
    LANGUAGE_CODE_ENGLISH
};

EFI_SMM_CONTROL2_PROTOCOL   *gSmmCtl=NULL;
EFI_SDIO_PROTOCOL           *gAmiSdio=NULL;
SDIO_GLOBAL_DATA            *gSdioData=NULL;
EFI_GUID                    gAmiSdioCommunicationGuid = SDIO_PARAMETER_BASE_ADDRESS_GUID;

/**
    Installs gSdioDriverBinding protocol

        
    @param ImageHandle 
    @param SystemTable 

    @retval EFI_STATUS

    @note  
  Here is the control flow of this function:
  1. Initialize Ami Lib.
  2. Install Driver Binding Protocol
  3. Return EFI_SUCCESS.

**/

EFI_STATUS
EFIAPI
SdioDriverEntryPoint (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
)

{
    EFI_STATUS  Status;
    gSdioDriverBinding.DriverBindingHandle=ImageHandle;
    gSdioDriverBinding.ImageHandle=ImageHandle;
        
    InitAmiLib(ImageHandle, SystemTable);

    Status = pBS->InstallMultipleProtocolInterfaces(
                        &gSdioDriverBinding.DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid, &gSdioDriverBinding,
                        &gEfiComponentName2ProtocolGuid, &gSdioCtlDriverName,
                        NULL
                        );
    ASSERT_EFI_ERROR(Status);

    return Status;
}

/**
    It returns the Driver name of this SDIO Driver(AMI SDIO Driver)

        
    @param  This 
    @param  Language 
    @param  DriverName 

         
    @retval EFI_SUCCESS returns driver name address
    @retval EFI_INVALID_PARAMETER If input parameter is not valid.
    @retval EFI_UNSUPPORTED If Language requested is not English.

**/
EFI_STATUS
EFIAPI
SdioCtlDriverName (
    IN  EFI_COMPONENT_NAME2_PROTOCOL    *This,
    IN  CHAR8                           *Language,
    OUT CHAR16                          **DriverName
)
{
    if(!Language || !DriverName) {
        return EFI_INVALID_PARAMETER;
    }
    //
    //Supports only English
    //
    if (!LanguageCodesEqual( Language, LANGUAGE_CODE_ENGLISH)) {
        return EFI_UNSUPPORTED;
    }

    *DriverName = gSdioBusDriverName;
    return EFI_SUCCESS;
}

/**
    It returns the Device name of the SD/MMC card.

        
    @param  This 
    @param  Controller 
    @param  ChildHandle OPTIONAL,
    @param  Language 
    @param  ControllerName 

         
    @retval EFI_SUCCESS returns Device name address
    @retval EFI_INVALID_PARAMETER If input parameter is not valid.
    @retval EFI_UNSUPPORTED If Language requested is not English.

**/
EFI_STATUS
EFIAPI
SdioCtlGetControllerName (
    IN  EFI_COMPONENT_NAME2_PROTOCOL    *This,
    IN  EFI_HANDLE                      Controller,
    IN  EFI_HANDLE                      ChildHandle        OPTIONAL,
    IN  CHAR8                           *Language,
    OUT CHAR16                          **ControllerName
)
{

    EFI_STATUS              Status;
    AMI_SDIO_BUS_PROTOCOL   *SdioBusInterface = NULL;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;
    DLINK                   *dlink = NULL;
    
    // Check if gSdioBusInitProtocolGuid is installed on the Controller Device.
    Status =  pBS->OpenProtocol( Controller,
                                 &gSdioBusInitProtocolGuid,
                                 (VOID**)&SdioBusInterface,
                                 gSdioDriverBinding.DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                                 );
                                
    if (Status != EFI_SUCCESS && Status != EFI_ALREADY_STARTED) {
        return EFI_UNSUPPORTED;
    }
    //
    //Supports only "eng"
    //
    if(!Language || !ControllerName) {
        return EFI_INVALID_PARAMETER;
    }

    if (!LanguageCodesEqual( Language, LANGUAGE_CODE_ENGLISH)) {
        return EFI_UNSUPPORTED;
    }

    if(ChildHandle == NULL) {
        *ControllerName = gSdioBusDriverName;
    } else {
         dlink = SdioBusInterface->SdioDeviceList.pHead;
         do {
             SdioDevInterface = OUTTER(dlink, SdioDeviceLink, SDIO_DEVICE_INTERFACE);
             if (SdioDevInterface->SdioDeviceHandle == ChildHandle) break; 
             dlink = dlink-> pNext;
             SdioDevInterface = NULL;
         } while (dlink);
   
         if (SdioDevInterface && (SdioDevInterface->DeviceState == DEVICE_CONFIGURED_SUCCESSFULLY)) {
             *ControllerName   = SdioDevInterface->UnicodePNMString;
              return EFI_SUCCESS;
         }
    }
    return EFI_UNSUPPORTED;
}


/**
    Checks whether the given controller is Block I/O or not

        
    @param This 
    @param Controller 
    @param RemainingDevicePath 

    @note  
  Here is the control flow of this function:
  1. Check whether PCI and Device path protocol has been installed on this controller
  2. Check if the controller is Block I/O controller
  3. If Block I/O Controller protocol already installed, return EFI_ALREADY_STARTED
  4. Return EFI_SUCCESS

**/

EFI_STATUS
EFIAPI
SdioDriverSupported (
    IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN  EFI_HANDLE                   Controller,
    IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
)
{
    EFI_STATUS                  Status;
    EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
    EFI_PCI_IO_PROTOCOL         *PciIo;
    PCI_STD_DEVICE              Pci;

    //
    // Check whether DevicePath Protocol has been installed on this 
    // controller
    //    
    Status = pBS->OpenProtocol( Controller,
                    &gEfiDevicePathProtocolGuid,
                    (VOID **)&ParentDevicePath,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER );

    if (EFI_ERROR (Status)) {
        return Status;
    }

    pBS->CloseProtocol (
                    Controller,
                    &gEfiDevicePathProtocolGuid,
                    This->DriverBindingHandle,
                    Controller
                    );

    //
    //Check whether PCI Protocol has been installed on this controller
    //
    Status = pBS->OpenProtocol( Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER );

    if  ( EFI_ERROR(Status) ) {
        return Status;
    }

    //
    // See if this is a PCI Mass Storage Controller by looking at the Class 
    // Code Register
    //
    Status = PciIo->Pci.Read (PciIo,
                    EfiPciIoWidthUint32,
                    0,
                    sizeof (Pci) / sizeof (UINT32),
                    &Pci);

    if  ( EFI_ERROR(Status) ) {
            goto ErrorPath;
    }

    if ( Pci.Header.ClassCode[1] != PCI_CL_SYSTEM_PERIPHERALS_SCL_SD || \
         Pci.Header.ClassCode[2] != PCI_CL_SYSTEM_PERIPHERALS ) {
        Status = EFI_UNSUPPORTED;
    }

    ErrorPath:
    pBS->CloseProtocol( Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller );

    return Status;
}

/**
    Installs BlockIoProtocol

    @param  This 
    @param  Controller 
    @param  RemainingDevicePath 

    @note  
  Here is the control flow of this function:
  1. Open PCI and Device path protocol
  2. Enable the device
  3. Post the option rom
  4. If first time, allocate buffer for real mode thunk code
  5. For each disk...
     a. Allocate and initialize a private structure
     b. Install block I/O protocol on a new child device
     c. Open the child device
  6. Increment user counter

**/

EFI_STATUS
EFIAPI
SdioDriverStart (
    IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN  EFI_HANDLE                   Controller,
    IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
)

{
    EFI_STATUS                Status;
    EFI_PCI_IO_PROTOCOL       *PciIo=NULL;
    EFI_DEVICE_PATH_PROTOCOL  *PciDevPath;
    UINTN                     Pages=0;
    EFI_PHYSICAL_ADDRESS      AdmaDescriptorAdd=0;
    AMI_SDIO_BUS_PROTOCOL     *SdioBusInterface;
    UINT8                     SlotNo=0;
    UINT64                    Attributes;
    BOOLEAN                   DeviceConfigured = FALSE;
    UINTN                     Seg, Bus, Dev, Func;
    EFI_HANDLE                Handle=NULL;
    EFI_GUID                  SdioRuntimeDataGuid = SDIO_RUNTIME_DATA_GUID;

    //Open PCI I/O Protocol
    Status = pBS->OpenProtocol( Controller,
                &gEfiPciIoProtocolGuid,
                (VOID **)&PciIo,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_BY_DRIVER );

    if  ( EFI_ERROR(Status) ) {
        return Status;
    }

    //  Open Device Path Protocol
    Status = pBS->OpenProtocol( Controller,
                &gEfiDevicePathProtocolGuid,
                (VOID **)&PciDevPath,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL );

    if (EFI_ERROR (Status)) {
        pBS->CloseProtocol( Controller,
                &gEfiPciIoProtocolGuid,
                This->DriverBindingHandle,
                Controller );

        return Status;
    }

    Status = pBS->OpenProtocol( Controller,
                                &gSdioBusInitProtocolGuid,
                                (VOID **)&SdioBusInterface,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER);


    if  ( !(Status == EFI_SUCCESS || Status == EFI_ALREADY_STARTED)) {

        Status = pBS->AllocatePool (EfiBootServicesData,
                                    sizeof(AMI_SDIO_BUS_PROTOCOL),
                                    (VOID**)&SdioBusInterface
                                    );
        if (EFI_ERROR(Status)){ 
            return EFI_OUT_OF_RESOURCES;
        }

        Status = InstallSdioBusProtocol (Controller, SdioBusInterface, PciIo);
        
        if (EFI_ERROR(Status)) {
            pBS->CloseProtocol( Controller,
                                &gEfiPciIoProtocolGuid,
                                This->DriverBindingHandle,
                                Controller 
                                );
            return Status;
        }
    }
    
    // Enable the Device
    Status = PciIo->Attributes( PciIo,
                                EfiPciIoAttributeOperationSupported,
                                0,
                                &Attributes );
    if(EFI_ERROR(Status)) {
        goto ErrorPath;
    }
    
    Status = PciIo->Attributes( PciIo,
                                EfiPciIoAttributeOperationEnable,
                                Attributes & EFI_PCI_DEVICE_ENABLE,
                                NULL );
    if(EFI_ERROR(Status)) {
        goto ErrorPath;
    }
    
    Status = PciIo->GetLocation (
                            PciIo,
                            &Seg,
                            &Bus,
                            &Dev,
                            &Func
                            );
    if (EFI_ERROR (Status)) {
        goto ErrorPath;
    }
   
#if SDMMC_VERBOSE_PRINT    
        DEBUG((EFI_D_VERBOSE,"SdMmc: Controller Bus=%x Dev=%x Func=%x\n",Bus,Dev,Func));
#endif   
    
    //
    // Allocating Page (4KB of Memory) for ADMA descriptor
    //
    Pages = EFI_SIZE_TO_PAGES (SDIO_SIZE);
    Status = PciIo->AllocateBuffer (PciIo,
                                AllocateMaxAddress,
                                EfiRuntimeServicesData,
                                Pages,
                                (VOID*)&AdmaDescriptorAdd,
                                EFI_PCI_ATTRIBUTE_MEMORY_WRITE_COMBINE);
    ASSERT_EFI_ERROR (Status);
    if(EFI_ERROR(Status)) {
       goto ErrorPath;
    }

    SdioBusInterface->AdmaDescriptorAddress = (UINT64)AdmaDescriptorAdd;
    if (gSdioData == NULL ) {
        Status = pBS->AllocatePool (EfiBootServicesData,
                                       sizeof(SDIO_GLOBAL_DATA),
                                       (VOID**)&gSdioData
                                       );
        if (EFI_ERROR(Status)) {
            goto ErrorPath;
        }
        //
        //  Clear the Buffer
        //
        pBS->SetMem((VOID*)gSdioData, sizeof(SDIO_GLOBAL_DATA),0);
        pBS->InstallProtocolInterface (
                                  &Handle,
                                  &SdioRuntimeDataGuid,
                                  EFI_NATIVE_INTERFACE,
                                  gSdioData
                                  );

        //
        // Get the SDIO access mode from Setup.
        //
        InitilizeSdioSetupConfiguration();
        gSdioData->AdmaDescriptorAddress=SdioBusInterface->AdmaDescriptorAddress;
    }
    
    for(SlotNo=0; SlotNo < SdioBusInterface->NumberOfSlotsImplemented;SlotNo++)
    {
        Status = DetectAndConfigureDevice(This, Controller, RemainingDevicePath, SdioBusInterface,SlotNo);
        if(!EFI_ERROR(Status)) {
            DeviceConfigured  = TRUE;
        }
    }
    
#if ( defined(BOOT_SECTOR_WRITE_PROTECT) && (BOOT_SECTOR_WRITE_PROTECT != 0) )
    if (!EFI_ERROR(Status) && AmiBlkWriteProtection == NULL) {
        Status = pBS->LocateProtocol( &gAmiBlockIoWriteProtectionProtocolGuid, 
                                      NULL, 
                                      &AmiBlkWriteProtection ); 
        if(EFI_ERROR(Status)) {
            AmiBlkWriteProtection = NULL;
        }
    }
#endif    
    
    return EFI_SUCCESS;
    
ErrorPath:
    
    if(AdmaDescriptorAdd) {
        PciIo->FreeBuffer(PciIo,
                          Pages,
                          (VOID *)AdmaDescriptorAdd);
    }
    
    pBS->CloseProtocol( Controller,
                        &gEfiPciIoProtocolGuid,
                        This->DriverBindingHandle,
                        Controller );

    return Status;
}

/**
    Disconnect the Controller

    @param  This 
    @param  Controller 
    @param  NumberOfChildren 
    @param  ChildHandleBuffer 

    @note  
  Here is the control flow of this function:
  1. Decrement user counter
  2. Free global buffer
  3. Release PCI I/O protocol and Block I/O protocol for each child handle.
  4. Shut down the hardware for each child handle.

**/
EFI_STATUS
EFIAPI
SdioDriverStop (
    IN  EFI_DRIVER_BINDING_PROTOCOL *This,
    IN  EFI_HANDLE                  Controller,
    IN  UINTN                       NumberOfChildren,
    IN  EFI_HANDLE                  *ChildHandleBuffer
)

{
    AMI_SDIO_BUS_PROTOCOL       *SdioBusInterface;
    SDIO_DEVICE_INTERFACE       *SdioDeviceInterface;
    EFI_STATUS                  Status;
    UINT8                       Index;
    UINT8                       Slot=0;
    EFI_PCI_IO_PROTOCOL         *PciIo;
    EFI_DEVICE_PATH_PROTOCOL    *PciDevPath;
    UINT64                      Attributes=0;

    // Check if SDIO_BUS_PROTOCOL is installed on the Controller.
    Status = pBS->OpenProtocol( Controller,
                                &gSdioBusInitProtocolGuid,
                                (VOID **)&SdioBusInterface,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }
   
    //
    // Check if ChildHandleBuffer is valid
    //
    if(NumberOfChildren) {
        for (Index=0; Index<NumberOfChildren; Index++) {
          
            Status = pBS->CloseProtocol ( Controller,
                                         &gEfiPciIoProtocolGuid,
                                         This->DriverBindingHandle,
                                         ChildHandleBuffer[Index]
                                       );
            if (EFI_ERROR(Status)) {
                ASSERT_EFI_ERROR(Status);
                continue;
            }
            Status = pBS->OpenProtocol(
                                   ChildHandleBuffer[Index],
                                   &gEfiDevicePathProtocolGuid,
                                   (VOID **)&PciDevPath,
                                   This->DriverBindingHandle,     
                                   Controller,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL
                                   );
            ASSERT_EFI_ERROR(Status);
           
            //
            // Retrieve the Slot number from the device path.
            //
            do {
               if ((PciDevPath->Type == MESSAGING_DEVICE_PATH) && (PciDevPath->SubType == MSG_SDIO_DP)) {
                    Slot = ((SDIO_DEVICE_PATH *)PciDevPath)->SlotNumber;
                    break;
                } else {
                    PciDevPath = NEXT_NODE(PciDevPath);
                }
            } while (PciDevPath->Type != END_DEVICE_PATH);
            
            if(PciDevPath->Type == END_DEVICE_PATH) {

                // Unable to find the Messaging device path node.
                ASSERT(FALSE);  
                return EFI_DEVICE_ERROR;
            }
        
            SdioDeviceInterface = GetSdioDevInterface(SdioBusInterface, Slot);
            if (!SdioDeviceInterface) return EFI_DEVICE_ERROR;
        
            //
            // Before uninstalling BLOCKIO check whether it is installed or not
            //
            Status = pBS->OpenProtocol( ChildHandleBuffer[Index],
                                    &gEfiBlockIoProtocolGuid,
                                    NULL,
                                    This->DriverBindingHandle,
                                    ChildHandleBuffer[Index],
                                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
            
            if(!EFI_ERROR(Status)) {
                
                Status = pBS->UninstallProtocolInterface (
                            ChildHandleBuffer[Index],
                            &gEfiBlockIoProtocolGuid, 
                            (EFI_BLOCK_IO_PROTOCOL *)(SdioDeviceInterface->SdioBlkIo)
                            );
                ASSERT_EFI_ERROR(Status);
                pBS->FreePool(SdioDeviceInterface->SdioBlkIo->BlkIo.Media);
                pBS->FreePool(SdioDeviceInterface->SdioBlkIo);
            
            }
        
            Status = pBS->UninstallProtocolInterface (
                                                      ChildHandleBuffer[Index],
                                                      &gEfiDevicePathProtocolGuid,
                                                      SdioDeviceInterface->DevicePathProtocol
                                                      );
            ASSERT_EFI_ERROR(Status);
            pBS->FreePool (SdioDeviceInterface->DevicePathProtocol);
            DListDelete(&(SdioBusInterface->SdioDeviceList), &(SdioDeviceInterface->SdioDeviceLink));
            pBS->FreePool (SdioDeviceInterface);
        
        } // for loop    
    } else {   
        
        //
        //Close all the protocols opened in Start Function
        //

        Status = SdioBusInterface->PciIO->Attributes( 
                                         SdioBusInterface->PciIO,
                                         EfiPciIoAttributeOperationSupported,
                                         0,
                                         &Attributes 
                                         );
        ASSERT_EFI_ERROR(Status);
        
        Status = SdioBusInterface->PciIO->Attributes (
                                           SdioBusInterface->PciIO,
                                           EfiPciIoAttributeOperationDisable,
                                           Attributes & EFI_PCI_DEVICE_ENABLE,
                                           NULL
                                           );
        ASSERT_EFI_ERROR(Status);

        Status = pBS->CloseProtocol( Controller,
                                &gEfiPciIoProtocolGuid,
                                This->DriverBindingHandle,
                                Controller);
    
        ASSERT_EFI_ERROR(Status);
      
        Status = pBS->CloseProtocol( Controller,
                                   &gEfiDevicePathProtocolGuid,
                                   This->DriverBindingHandle,
                                   Controller);
         
        ASSERT_EFI_ERROR(Status);

        Status = pBS->CloseProtocol( Controller,
                                &gSdioBusInitProtocolGuid,
                                This->DriverBindingHandle,
                                Controller);
        ASSERT_EFI_ERROR(Status);
    
        Status = pBS->UninstallProtocolInterface ( Controller,
                                        &gSdioBusInitProtocolGuid,
                                        SdioBusInterface
                                        );
        if (EFI_ERROR(Status))  {
            
            //
            // Open PCI I/O Protocol
            //
            Status = pBS->OpenProtocol( Controller,
                                &gEfiPciIoProtocolGuid,
                                (VOID **)&PciIo,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER 
                                );
            
            ASSERT_EFI_ERROR(Status);
        
            // Open Device Path Protocol
            Status = pBS->OpenProtocol( Controller,
                                &gEfiDevicePathProtocolGuid,
                                (VOID**)&PciDevPath,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER );
        
            ASSERT_EFI_ERROR(Status);
        
            Status = pBS->OpenProtocol( Controller,
                                &gSdioBusInitProtocolGuid,
                                (VOID **)&SdioBusInterface,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER
                                );
            ASSERT_EFI_ERROR(Status);
            return EFI_DEVICE_ERROR;
        }
        pBS->FreePool(gSdioData);
        // Free the Pages allocated for AdmaDescriptorAddress 
        if(SdioBusInterface->AdmaDescriptorAddress) {
            pBS->FreePages(SdioBusInterface->AdmaDescriptorAddress,
                         EFI_SIZE_TO_PAGES (SDIO_SIZE));
          
        }
        
    }
    return EFI_SUCCESS;
}

/**
    Installs BUS Init Protocol on the IDE controller Handle

        
    @param  Controller 
    @param  IdeBusInitInterface,
    @param  IdeControllerInterface,
    @param  PciIO

    @retval EFI_STATUS

    @note  
   1. Call SdioInitController
   2. Install gSdioBusInitProtocolGuid protocol

**/

EFI_STATUS
InstallSdioBusProtocol (
    IN  EFI_HANDLE                  Controller,
    IN  OUT AMI_SDIO_BUS_PROTOCOL   *SdioBusInterface,
    IN  EFI_PCI_IO_PROTOCOL         *PciIO
)
{
    EFI_STATUS  Status;

    // Initialize the default Values
    ZeroMemory (SdioBusInterface, sizeof(AMI_SDIO_BUS_PROTOCOL));

    SdioBusInterface->ControllerHandle = Controller;
    SdioBusInterface->PciIO = PciIO;

    // Init Sdio Controller
    Status = SdioInitController(SdioBusInterface);
    if (EFI_ERROR(Status)) { 
        return Status;
    }

    Status = pBS->InstallProtocolInterface(
                    &Controller,
                    &gSdioBusInitProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    SdioBusInterface);

    return Status;

}

/**
    Initializes Sdio Controller

    @param  SdioBusInterface

    @retval EFI_STATUS

    @note  
  1. Update internal Data area about the Sdio controller Capabilities.
  2. Allocate memory for FIS and CommandList
  3. Enable Sdio mode
  3. Disable all the ports

**/

EFI_STATUS
SdioInitController (
    IN  OUT AMI_SDIO_BUS_PROTOCOL   *SdioBusInterface
)
{
    EFI_STATUS  Status;
    UINT8       PciConfig[41];
    UINT8       BarOffset;
    UINT8       BarType;
    UINT8       BarSize=4;
    UINT64      BarAnd;
    UINT8       SlotNo;

    // Make sure SDIO Base address is programmed Properly
    Status = SdioBusInterface->PciIO->Pci.Read (
                                      SdioBusInterface->PciIO,
                                      EfiPciIoWidthUint8,
                                      0,
                                      sizeof (PciConfig),
                                      PciConfig
                                      );

    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    //
    // Calculate the Base address for the first port asper Section C.3.3 
    // Slot Information Register (Offset 40h). The PCI BAR may not always 
    // start at 10h First Base Address Register Number bits will provide 
    // the BAR for firt slot.
    //
    BarOffset= PCI_BAR0 + (PciConfig[40] & 0x07) *4;
    
    //
    // Slot Information Register's Bit 6-4 will provide the number of slots 
    // present in the controller. 
    //
    SdioBusInterface->NumberOfSlotsImplemented = ((PciConfig[40] & 0x70) >> 4)+1;
    BarType = (*(UINT8 *)(PciConfig + BarOffset))&BAR_64_32_SUPPORT_BIT;
    
    //
    // The lower nible of the Bar address is cleared. If the Bar support 
    // 64 Bit then the Bar address is 8 bytes else 4 bytes
    //
    if (BarType == BAR_64_BIT){
        BarAnd = 0xFFFFFFFFFFFFFFF0;
        BarSize = 8;
    }else {
        BarAnd = 0xFFFFFFF0;
        BarSize = 4;
    }

    
    for( SlotNo = 0;SlotNo < (SdioBusInterface->NumberOfSlotsImplemented); SlotNo++ )
    {
        SdioBusInterface->SdioBaseAddress[SlotNo] =((*(UINT64 *)(PciConfig + BarOffset+(SlotNo*BarSize))) & BarAnd);
        if (!SdioBusInterface->SdioBaseAddress[SlotNo]){ 
            ASSERT_EFI_ERROR(Status);
            return EFI_DEVICE_ERROR;
        }
    }
    return EFI_SUCCESS;
}

/**

        
    @param  SdioBusInterface 
    @param  Port 
    @param  PMPort 

    @retval Sdio_DEVICE_INTERFACE*

    @note  
  1. Return the Pointer to the Sdio_DEVICE_INTERFACE for the given Port and PM Port

**/
SDIO_DEVICE_INTERFACE *
GetSdioDevInterface (
    IN  AMI_SDIO_BUS_PROTOCOL   *SdioBusInterface,
    IN  UINT8                   Slot
)
{

    DLINK                   *dlink = SdioBusInterface->SdioDeviceList.pHead;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;

    if (!dlink) return NULL;
    do {
        SdioDevInterface = OUTTER(dlink, SdioDeviceLink, SDIO_DEVICE_INTERFACE);
        if (SdioDevInterface->SlotNumber == Slot) break;
        dlink = dlink-> pNext;
        SdioDevInterface = NULL;
    }while (dlink);

    return SdioDevInterface;
}

/**
    Detects and Configures Sdio Device

    @param  This 
    @param  Controller 
    @param  RemainingDevicePath 
    @param  SdioBusInterface,
    @param  Slot

    @retval EFI_STATUS

    @note  
 1. Detect whether device is connected to the port. If no device exit.
 2. Install SdioDevInterface. 
 3. Configure the Sdio device and the controller.
 4. Install DevicePath, BlockIO and DiskInfo protocol.

**/
EFI_STATUS
DetectAndConfigureDevice (
    IN  EFI_DRIVER_BINDING_PROTOCOL *This,
    IN  EFI_HANDLE                  Controller,
    IN  EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath,
    IN  AMI_SDIO_BUS_PROTOCOL       *SdioBusInterface,
    IN  UINT8                       Slot
)
{

    EFI_STATUS              Status;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;
    UINT8                   Index;
    EFI_PCI_IO_PROTOCOL     *PciIo=NULL;
    
    // Check the Slot already detected 
    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, Slot);
    if (SdioDevInterface && ((SdioDevInterface->DeviceState == DEVICE_DETECTION_FAILED)||
        (SdioDevInterface->DeviceState == DEVICE_CONFIGURED_SUCCESSFULLY))){
        return EFI_SUCCESS;
    }

    Status = SdioDetectDeviceAndConfigure(SdioBusInterface,Slot);
    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, Slot);
    if (EFI_ERROR(Status)) {
        if (SdioDevInterface) { 
            SdioDevInterface->DeviceState = DEVICE_DETECTION_FAILED;
        }
        return EFI_DEVICE_ERROR;
    }
    SdioDevInterface->DeviceState = DEVICE_CONFIGURED_SUCCESSFULLY;
    
    // Create the Device path
    Status = CreateSdioDevicePath (This, Controller, SdioDevInterface, RemainingDevicePath);
    if (EFI_ERROR(Status)) { 
        ASSERT_EFI_ERROR(Status);
        return EFI_DEVICE_ERROR;
    }

    // Install Device path on the new handle
    Status = pBS->InstallMultipleProtocolInterfaces (
                &(SdioDevInterface->SdioDeviceHandle),
                &gEfiDevicePathProtocolGuid,
                SdioDevInterface->DevicePathProtocol,
                NULL);
    if(EFI_ERROR(Status)) {
        SdioDevInterface->DeviceState = DEVICE_DETECTION_FAILED;
        return EFI_DEVICE_ERROR;
    }
    
    //
    // Open the Pci EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER attribute.
    Status = pBS->OpenProtocol (Controller,
                                &gEfiPciIoProtocolGuid,
                                (VOID**)&PciIo,
                                This->DriverBindingHandle,
                                SdioDevInterface->SdioDeviceHandle,
                                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER );
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    

    if(!SdioDevInterface->MassStorageDevice) {
        //
        // The detected Device is not a Mass Storage device.
        //
        return EFI_SUCCESS;
    }

    //
    // Convert the PNM string into Unicode String and store it. 
    //
    for(Index=0;Index<27;Index++) {
       SdioDevInterface->UnicodePNMString[Index] = (CHAR16)SdioDevInterface->SdioDevInfo->PNM[Index];
    }
    
    //
    // The detected device is Mass storage device. So initialize the Block IO and Legacy Int13
    // Functions for the SDIO device.
    //
    Status = InitSdioBlockIO (SdioDevInterface);
    if (EFI_ERROR(Status)) { 
        return EFI_DEVICE_ERROR; 
    }

    Status = pBS->InstallMultipleProtocolInterfaces (
                    &(SdioDevInterface->SdioDeviceHandle),
                    &gEfiBlockIoProtocolGuid, (EFI_BLOCK_IO_PROTOCOL *)(SdioDevInterface->SdioBlkIo),
                    NULL);
    if (EFI_ERROR(Status)) { 
        return EFI_DEVICE_ERROR; 
    }

    //
    // Hardware specific tuning if required.
    //
    Status = SdMmcTunning(SdioDevInterface,Slot);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) { 
        return Status; 
    }
    
    // Tranfer the SDIO data to SMM driver for Legacy operation
    Status = TransferSdioDataToSmram();
    if (EFI_ERROR(Status)) { 
        // In case if it's error don't Install the legacy interface 
        return EFI_SUCCESS; 
    }    
    
    //
    //Install Legacy boot Support
    //
    if(gAmiSdio == NULL) {
        pBS->LocateProtocol(&gEfiSdioProtocolGuid, NULL, (VOID **) &gAmiSdio);
    }

    if(gAmiSdio != NULL) {
        SDIO_DEV_INFO   SdioDevInfo;
        SDIO_MASS_DEV   SdioMassDevInfo;
        UINTN       PciSeg, PciBus, PciDev, PciFunc;

        SdioDevInfo.wBlockSize=SdioDevInterface->SdioDevInfo->wBlockSize;
        SdioDevInfo.dMaxLba=SdioDevInterface->SdioDevInfo->dMaxLBA;
        SdioDevInfo.bHeads=SdioDevInterface->SdioDevInfo->NumHeads;
        SdioDevInfo.bSectors=SdioDevInterface->SdioDevInfo->NumSectors;
        SdioDevInfo.wCylinders=SdioDevInterface->SdioDevInfo->NumCylinders;
        SdioDevInfo.bNonLBAHeads=SdioDevInterface->SdioDevInfo->LBANumHeads;
        SdioDevInfo.bNonLBASectors=SdioDevInterface->SdioDevInfo->LBANumSectors;
        SdioDevInfo.wNonLBACylinders=SdioDevInterface->SdioDevInfo->LBANumCyls;

        //
        // Get the SDIO controller Bus,Dev and Fun
        //
        SdioBusInterface->PciIO->GetLocation (SdioBusInterface->PciIO, &PciSeg, &PciBus, &PciDev, &PciFunc);

        SdioMassDevInfo.PciBDF=(UINT16)((PciBus << 8)+(PciDev << 3) + PciFunc);
        SdioMassDevInfo.DevString=&SdioDevInterface->SdioDevInfo->PNM[0];
        SdioMassDevInfo.DevInfo=(VOID*)&SdioDevInfo;
        SdioMassDevInfo.LogicalAddress=SdioDevInterface->SdioDevInfo->DeviceAddress;
        SdioMassDevInfo.StorageType = SdioDevInterface->SdioDevInfo->bStorageType;
        SdioMassDevInfo.Handle=SdioDevInterface->SdioDeviceHandle;
        gAmiSdio->SdioInstallLegacyDevice(&SdioMassDevInfo);
    }

    return EFI_SUCCESS;
}

/**
    Detects a Sdio device connected to given Port and PMPort

    @param  SdioBusInterface 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
SdioDetectDeviceAndConfigure (
    IN  AMI_SDIO_BUS_PROTOCOL   *SdioBusInterface,
    IN  UINT8                   Slot
)
{

    EFI_STATUS              Status;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;


    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, Slot);

    if (!SdioDevInterface){
        //
        // Entry not Present, Create New entry for the Slot 
        //
        Status = pBS->AllocatePool (EfiBootServicesData,
                    sizeof(SDIO_DEVICE_INTERFACE),
                    (VOID**)&SdioDevInterface
                    );
        if (EFI_ERROR(Status)) return Status;

        ZeroMemory (SdioDevInterface, sizeof(SDIO_DEVICE_INTERFACE));

        SdioDevInterface->SlotNumber = Slot;
        SdioDevInterface->SdioBusInterface = SdioBusInterface;
        SdioDevInterface->DeviceState = DEVICE_IN_RESET_STATE;

        //
        // Add to the SdioBusInterface
        //
        DListAdd(&(SdioBusInterface->SdioDeviceList), &(SdioDevInterface->SdioDeviceLink));

    }

    //
    // Check the device Presence in the slot
    //
    Status = SdMmcCheckDevicePresence (SdioDevInterface, Slot);

    if (EFI_ERROR(Status)) {
        DListDelete(&(SdioBusInterface->SdioDeviceList), &(SdioDevInterface->SdioDeviceLink));
        pBS->FreePool(SdioDevInterface);
        return EFI_DEVICE_ERROR;
    }

    //
    // Cofiure the SD and MMC device
    //
    Status = SdMmcConfigureDevice (SdioDevInterface, Slot);
    if(EFI_ERROR(Status)) {
        DListDelete(&(SdioBusInterface->SdioDeviceList), &(SdioDevInterface->SdioDeviceLink));
        pBS->FreePool(SdioDevInterface);
        return EFI_DEVICE_ERROR;
    }

    if(SdioDevInterface->MassStorageDevice == FALSE) {
        return EFI_SUCCESS;
    }

    //
    //Get the SD/MMC card device name
    //
    Status= SDMmcMassGetPNMWithDeviceClass(SdioDevInterface->SdioDevInfo,Slot);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Based on the device and controller select the device access mode
    //
    Status= SDMMCCardSelectAccessMode(SdioDevInterface->SdioDevInfo,Slot);
    if(EFI_ERROR(Status)) {
    	return Status;
    }
    
    //
    // Identify the device type ( HDD, Floppy ) based on the setup option
    //
    SdMmcGetEmulationType(SdioDevInterface->SdioDevInfo, Slot);
    
    return Status;
}


//**********************************************************************
/**
    Creates a SDIO device device path and adds it to SdioDevInterface

    @param  This 
    @param  Controller 
    @param  SdioDevInterface
    @param  RemainingDevicePath 

    @retval EFI_STATUS

    @note  
  Here is the control flow of this function:
  1.  Get the controller device path and append the SD\MMC device path
  2.  But currently we are using SATA device path SDIO device path is not 
      mentioned in the UEFI specification.

**/
EFI_STATUS
CreateSdioDevicePath (
    IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
    IN  EFI_HANDLE                      Controller,
    IN  SDIO_DEVICE_INTERFACE           *SdioDevInterface,
    IN  OUT EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
 )
{

    EFI_STATUS                  Status;
    SDIO_DEVICE_PATH            NewDevicePath;
    EFI_DEVICE_PATH_PROTOCOL    *TempDevicePath;
    
    // Refer UEFI2.5spec. 9.3.5.25 SD Device Path section
    NewDevicePath.Header.Type = MESSAGING_DEVICE_PATH;
    NewDevicePath.Header.SubType = MSG_SDIO_DP;
    SET_NODE_LENGTH(&NewDevicePath.Header,sizeof(SDIO_DEVICE_PATH));
    NewDevicePath.SlotNumber = SdioDevInterface->SlotNumber;
    
    // Append the Device Path
    Status = pBS->OpenProtocol( Controller,
                        &gEfiDevicePathProtocolGuid,
                        (VOID **)&TempDevicePath,
                        This->DriverBindingHandle,
                        Controller,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    SdioDevInterface->DevicePathProtocol = DPAddNode(TempDevicePath, &NewDevicePath.Header);
    
    return Status;
}



/**
    Get the SDIO Access mode from Setup and Set the SDIO_Access_Mode 
    Global variable

    @param 

    @retval EFI_STATUS

**/
VOID
InitilizeSdioSetupConfiguration (
 )
{

    EFI_STATUS             Status;
    EFI_GUID               gSetupGuid = SETUP_GUID;
    UINTN                  VariableSize=sizeof(SDIO_DEV_CONFIGURATION);


    Status = pRS->GetVariable(L"SdioDevConfiguration",&gSetupGuid,
                              NULL,&VariableSize, &gSdioConfiguration);

    if(EFI_ERROR(Status)) {
        //
        // Error in Getting variable. Set the mode to Auto
        //
        gSdioData->SDIO_Access_Mode = 0;
    } else {
        //
        // Initialize the Sdio Access mode and emulation type from Setup.
        //
        gSdioData->SDIO_Access_Mode = gSdioConfiguration.SdioMode;
    }
    return;
}

/**
    Check the device presence and return the status
        
    @param  SataDevInterface 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
SdMmcCheckDevicePresence (
    IN  SDIO_DEVICE_INTERFACE   *SdioDevInterface,
    IN  UINT8                   Slot
)
{
    EFI_STATUS              Status;
    UINT8                   DevIndex;
    UINT8                   Index;
    AMI_SDIO_BUS_PROTOCOL   *SdioBusInterface = SdioDevInterface->SdioBusInterface;

    DevIndex=GetFreeDevEntry();
    if(DevIndex == 0xFF) {
        return EFI_DEVICE_ERROR;
    }
    
    gSdioData->SdioDev[DevIndex].SdioBaseAddress=SdioBusInterface->SdioBaseAddress[Slot];
    SdioDevInterface->SdioDevInfo = &gSdioData->SdioDev[DevIndex];
    
    Index=GetDevEntry(gSdioData->SdioDev[DevIndex].SdioBaseAddress);
    if(Index!=0xff) {
        SdioDevInterface->SdioDevInfo->wEmulationOption = gSdioConfiguration.SdioEmu[Index].SdEmul;
    }

    //
    // Check the Device Presence
    //
    Status = CheckDevicePresenceInController (SdioDevInterface->SdioDevInfo, Slot);

    if(EFI_ERROR(Status)) {
        return EFI_NOT_FOUND;
    }

    
    //
    // Device is present in the SD Port
    //
    SdioDevInterface->SdioDevInfo->DevEntryUsed=TRUE;
    SdioDevInterface->SdioDevInfo->DeviceAddress=DevIndex+1;
    SdioDevInterface->SdioDevInfo->SlotNumber = SdioDevInterface->SlotNumber;

    return EFI_SUCCESS;
}

/**
    Configure the Device 

    @param  SataDevInterface 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
SdMmcConfigureDevice (
    IN  SDIO_DEVICE_INTERFACE   *SdioDevInterface,
    IN  UINT8                   Slot
)
{

    SDIO_DEVICE_INFO  *SdioDevInfo = SdioDevInterface->SdioDevInfo;
    EFI_STATUS        Status;
    
    //
    // Check the device in SD slot is I/O device. If IO only device 
    // return success and if memory present then initialize the memory.
    //
#if CHECK_FOR_SD_IO_DEVICE
    Status= SdioCheckDevice(SdioDevInfo,Slot);
    if(!EFI_ERROR(Status)) {
        if(SdioDevInfo->IODevice == TRUE) {
            SdioDevInterface->MassStorageDevice=FALSE;
            return EFI_SUCCESS;
        }
    }
#endif
    
    //
    // Configure the Memory type Device
    //
    Status = ConfigureSdMmcMemoryDevice(SdioDevInfo, Slot);
    if(EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }
    SdioDevInterface->MassStorageDevice=TRUE;
    return EFI_SUCCESS;
}

/**
    Transfer the Nvme Controller information to SMRAM area that would be used
    by the SDIO SMM driver 

    @param  

    @retval None

**/
EFI_STATUS
TransferSdioDataToSmram ()
{

    EFI_STATUS                      SmmStatus;
    EFI_SMM_COMMUNICATION_PROTOCOL  *SmmCommunication;
    EFI_SMM_COMMUNICATE_HEADER      *CommHeader;
    UINT8                           CommBuffer[sizeof(EFI_GUID) + sizeof(UINTN) + sizeof(gSdioData)];
    UINTN                           CommSize;
    
    // Get needed resource
    SmmStatus = pBS->LocateProtocol (
                    &gEfiSmmCommunicationProtocolGuid,
                    NULL,
                    (VOID **)&SmmCommunication
                    );
    if (EFI_ERROR (SmmStatus)) {
        return SmmStatus;
    }

    CommHeader = (EFI_SMM_COMMUNICATE_HEADER *)&CommBuffer[0];
    pBS->CopyMem (&CommHeader->HeaderGuid, &gAmiSdioCommunicationGuid, sizeof(gAmiSdioCommunicationGuid));
    CommHeader->MessageLength = sizeof(gSdioData);
    pBS->CopyMem (&(CommHeader->Data[0]), &gSdioData, sizeof(gSdioData));
    
    // Send command
    CommSize = sizeof(CommBuffer);
    SmmStatus = SmmCommunication->Communicate (
                                 SmmCommunication,
                                 &CommBuffer[0],
                                 &CommSize
                                 );
    return SmmStatus;

}

/**
    Get the Free Entry Index from SDIO Device Buffer

    @param 

    @retval Entry No

**/
UINT8
GetFreeDevEntry (
)
{
    UINT8  i;

    for(i=0;i<MAX_SDIO_DEVICES;i++) {
        if(gSdioData->SdioDev[i].DevEntryUsed == FALSE) {
            return i;
        }
    }

    return 0xFF;

}

/**
    This function identifies the device type and fill the 
    geometry parameter into SDIO_DEVICE_INFO

    @param  SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
    @param  Port        - SDIO Port number

    @retval VOID

**/

VOID
SdMmcGetEmulationType (
    IN  SDIO_DEVICE_INFO *SdioDevInfo,
    IN  UINT8            Port
)
{
    EFI_STATUS      Status;
    UINT16          wEmulationType;
    UINT16          wForceEmulationType = 0;
    static UINT16   SDIOMassEmulationTypeTable[4] = {
        0,                                                  // Auto
        (SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD,   // Floppy
        (SDIO_EMU_FORCED_FDD << 8) + SDIO_MASS_DEV_ARMD,    // Forced floppy
        (SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD };     // HDD

    if ( SdioDevInfo->wEmulationOption ) {                        // non Auto
        wEmulationType = SDIOMassEmulationTypeTable[SdioDevInfo->wEmulationOption];
        wForceEmulationType = SDIOMassEmulationTypeTable[SdioDevInfo->wEmulationOption];
    }

    if (gSdioReadData == NULL) {
        Status = pBS->AllocatePool(EfiBootServicesData, 0x1000, (VOID**)&gSdioReadData);
        ASSERT_EFI_ERROR(Status);
    }

    //
    // Assume floppy
    //
    wEmulationType = (UINT16)(SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD;

    if (SDIOGetFormatType( \
           SdioDevInfo, Port, gSdioReadData, &wEmulationType) == SDIO_ERROR) {
        //
        // Find the device type by size
        //
        if ((SdioDevInfo->dMaxLBA >> 11) > MAXIMUM_SIZE_FOR_FLOPPY_EMULATION ) {
            //
            // Assume HDD
            //
            wEmulationType = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
        }
    }

    if (wForceEmulationType) wEmulationType = wForceEmulationType;
    SdioDevInfo->bStorageType = (UINT8)wEmulationType;
    SdioDevInfo->bEmuType = (UINT8)(wEmulationType >> 8);

    if (gSdioReadData != NULL) {
        pBS->FreePool(gSdioReadData);
        gSdioReadData = NULL;
    }

    return;

}

/**
    This function reads the first sector from the mass storage
    device and identifies the formatted type, Sets the Emulation
    type accordingly.

    @param  SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
    @param  Port        - SDIO Port number
    @param  SdioReadData - Pointer to Buffer
    @param  EmuType - Pointer to location that contains Emulation type of the device

    @retval SDIO_ERROR   If could not identify the format type
    @retval SDIO_SUCCESS If formatted type is identified and EmuType is updated


**/

UINT8
SDIOGetFormatType (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port,
    IN  UINT8               *SdioReadData,
    OUT UINT16              *EmuType
)
{
    EFI_STATUS  Status;
    UINT16      Emu_Type;
    UINT8       *ActPartAddr;
    UINT32      dHS;

    //
    // Read the first sector of the device
    //
    Status = SdMmcRead(SdioDevInfo, Port, 0, 1, SdioReadData);

    if ( EFI_ERROR(Status) ) {
            return SDIO_ERROR;
    }

    SdioDevInfo->bHiddenSectors = 0;

    //
    // Check for validity of Boot Record
    //
    if ( *(UINT16*)(SdioReadData + 0x1FE) != 0xAA55 ) {
        SDIOSetDefaultGeometry(SdioDevInfo, Port);
        return SDIO_ERROR;
    }

    //
    // Check for validity of the partition table
    //
    if ( SDIOValidatePartitionTable( \
            SdioReadData, SdioDevInfo->dMaxLBA, &ActPartAddr) == SDIO_SUCCESS ) {
        //
        // Only one partition present, check the device size, if the device size
        // is less than 530 MB assume FDD or else assume the emulation as HDD
        //
        if ( SdioDevInfo->dMaxLBA < MAX_LBA_FOR_FLOPPY_EMULATION ) {
            Emu_Type = (UINT16)(SDIO_EMU_FORCED_FDD << 8) + SDIO_MASS_DEV_ARMD;
        } else {
            Emu_Type = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
        }

        //
        // Read boot sector, set the LBA number to boot record LBA number
        //
        dHS = *((UINT32*)(ActPartAddr + 8));
        SdioDevInfo->bHiddenSectors = (UINT8)dHS; // Save hidden sector value

        Status = SdMmcRead(SdioDevInfo, Port, dHS, 1, SdioReadData);

        if ( EFI_ERROR(Status) ) return SDIO_ERROR;

        if ( SDIOUpdateCHSFromBootRecord( \
                SdioDevInfo, Port, SdioReadData) == SDIO_SUCCESS) {

            *EmuType = Emu_Type;
             return SDIO_SUCCESS;
        } else {

            SDIOSetDefaultGeometry(SdioDevInfo, Port);
            *EmuType = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
            SdioDevInfo->bHiddenSectors = 0;      // Reset hidden sector value
            return SDIO_SUCCESS;
        }
    }

    if ( SDIOUpdateCHSFromBootRecord(
            SdioDevInfo, Port, SdioReadData) == SDIO_SUCCESS) {

        // Assume the emulation as floppy
        // If boot record is a valid FAT/NTFS file system
        *EmuType = (UINT16)(SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD;
        return SDIO_SUCCESS;
    }
    
    SDIOSetDefaultGeometry(SdioDevInfo, Port);  
    Emu_Type = (UINT16)(SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD;

    if ( SdioDevInfo->dMaxLBA >= MAX_LBA_FOR_FLOPPY_EMULATION ) {

        // Assume the emulation as HDD
        // If the device size greater than 530MB
        SdioDevInfo->bHiddenSectors = 0;
        Emu_Type = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
    }

    *EmuType = Emu_Type;
    return SDIO_SUCCESS;

}

/**
    This procedure updates cylinder parameter for device geometry.
    head and sector parameters are required before invoking this
    function.

    @param  SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
    @param  Port        - SDIO Port number

    @retval VOID

**/

VOID
SdioMassUpdateCylinderInfo (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port 
)
{
    UINT32 data = SdioDevInfo->dMaxLBA /(SdioDevInfo->NumSectors * SdioDevInfo->NumHeads);

    if (data <= 1) data++;
    if (data > 0xFFFF) data = 0xFFFF;   // DOS workaround

    SdioDevInfo->NumCylinders = (UINT16)data;
    SdioDevInfo->LBANumCyls = (UINT16)data;
    return;
}

/**
    This procedure sets the  default geometry for mass 
    storage devices.

    @param  SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
    @param  Port         - SDIO Port number

    @retval SDIO_SUCCESS

**/

UINT8 
SDIOSetDefaultGeometry ( 
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    SdioDevInfo->NumHeads = 0xFF;
    SdioDevInfo->LBANumHeads = 0xFF;
    SdioDevInfo->NumSectors = 0x3F;
    SdioDevInfo->LBANumSectors = 0x3F;

    SdioMassUpdateCylinderInfo(SdioDevInfo, Port);
    return SDIO_SUCCESS;
}

/**
    This procedure checks whether the partition table is valid.

    @param  Buffer - Pointer to the Boot Record of the device
    @param  dMaxLBA - Max LBA of the device
    @param  ActPartAddr - Pointer to a pointer that contains Active Partition offset

    @retval SDIO_SUCCESS partition table is valid:
    @retval ValidEntryCount Possible valid entry count ( 1-based )
    @retval ActPartAddr Active entry offset ( Absolute offset )
    @retval SDIO_ERROR Invalid partition table

**/

UINT8
SDIOValidatePartitionTable (
    IN  UINT8   *Buffer,
    IN  UINT32  dMaxLBA,
    OUT UINT8   **ActPartAddr
)
{

    UINT8  *PartPtr;
    UINT8  PartNo = 0;
    UINT8  *ActPart = NULL;
    UINT8  ValidEntryCount = 0;

    //
    // Drive has a partition table, start from 1st bootable partition
    //
    PartPtr = Buffer + 0x1BE;

    for (; PartNo<4; PartNo++, PartPtr+=0x10 ) {

        if (*PartPtr & 0x7F ) return SDIO_ERROR; //BootFlag should be 0x0 or 0x80
        //
        // Check whether beginning LBA is reasonable
        //
        if (*(UINT32*)(PartPtr + 8) > dMaxLBA) return SDIO_ERROR;
        
        ValidEntryCount++;                      // Update valid entry count
        //
        // Update active entry offset
        //
        if (!(*PartPtr & 0x80)) continue;
        if (ActPart) continue;
        ActPart = PartPtr;
    }

    if (ValidEntryCount < 1) return SDIO_ERROR; // Atleast one valid partition is found
    //
    // If no active partition table entry found use first entry
    //
    if (ActPart == NULL) ActPart = Buffer + 0x1BE;

    *ActPartAddr = ActPart;

    return SDIO_SUCCESS;
}

/**
    This function parses the boot record and extract the CHS
    information of the formatted media from the boot record.
    This routine checks for DOS & NTFS formats only

    @param  SdioDevInfo  - Pointer to SDIO_DEVICE_INFO structure
    @param  Port         - SDIO Port number
    @param  SdBootRecord - Pointer to Boot record of the device

    @retval SDIO_ERROR If the boot record is un-recognizable and CHS info is not extracted
    @retval SDIO_SUCCESS If the boot record is recognizable and CHS info is extracted. CHS information is updated in the SDIO_DEVICE_INFO structure
**/

UINT8
SDIOUpdateCHSFromBootRecord (
    IN  SDIO_DEVICE_INFO*   SdioDevInfo,
    IN  UINT8               Port,
    IN  UINT8*              SdBootRecord
)
{
    UINT32  OemName;
    UINT8   Heads;
    UINT8   SecPerTrack;
    UINT16  SecTimesHeads;
    UINT16  TotalSect;

    if (*((UINT16*)(SdBootRecord + 0x1FE)) != 0xAA55) return SDIO_ERROR;

    //
    // Read succeeded so the drive is formatted
    // Check for valid MSDOS/MSWIN/NTFS boot record
    //
    OemName = *(UINT32*)(SdBootRecord + 3);
    
    if ((OemName != 0x4F44534D) &&   // 'ODSM' for MSDO
        (OemName != 0x4957534D) &&   // 'IWSM' for MSWI
        (OemName != 0x5346544E)) {   // 'SFTN' for NTFS
        //
        // Check for valid FAT,FAT16,FAT32 boot records
        //
        *(SdBootRecord + 0x36 + 3) = 0x20;              // Ignore the 4th byte and fill it with space
        if ((*(UINT32*)(SdBootRecord + 0x36) != 0x20544146) &&      // " TAF" for FATx
            (*(UINT32*)(SdBootRecord + 0x52) != 0x33544146)) {      // "3TAF" for FAT3
            //
            // Boot Record is invalid. Return with error
            //
            return SDIO_ERROR;
        }
    }

    Heads = *(SdBootRecord + 0x1A);         // Number of heads
    SecPerTrack = *(SdBootRecord + 0x18);   // Sectors/track
    SecTimesHeads = Heads * SecPerTrack;

    // Zero check added to prevent invalid sector/head information in BPB
    if (SecTimesHeads == 0) {
        return SDIO_ERROR;
    }

    TotalSect = *(UINT16*)(SdBootRecord + 0x13);
    if ( TotalSect ) {
        SdioDevInfo->dMaxLBA = TotalSect;
    }

    SdioDevInfo->NumHeads = Heads;
    SdioDevInfo->LBANumHeads = Heads;
    SdioDevInfo->NumSectors = SecPerTrack;
    SdioDevInfo->LBANumSectors = SecPerTrack;

    SdioMassUpdateCylinderInfo(SdioDevInfo, Port);

    return  SDIO_SUCCESS;
}

/**
    This function will select the Access mode for the data transfer.

    @param  SdioDevInfo  - Pointer to SDIO_DEVICE_INFO structure
    @param  Port         - SDIO Port number

    @retval EFI_SUCCESS

**/
EFI_STATUS
SDMMCCardSelectAccessMode (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    UINT32  Capability=0;

    Capability=SDIO_REG32(SdioDevInfo->SdioBaseAddress,CAP);

    
    if(gSdioData->SDIO_Access_Mode == 0 ) {

        //
        // If the SDIO access mode is Auto, based on capabilities select PIO,SDMA 
        // or ADMA
        //
        if((Capability & ADMA2_SUPPORT) && (SdioDevInfo->bHostControllerVersion>=1)) {
            SdioDevInfo->SDIO_Access=ADMA;
        }else if(Capability & SDMA_SUPPORT) {
            SdioDevInfo->SDIO_Access=SDMA;
        }else {
            SdioDevInfo->SDIO_Access=PIO;
        }
    } else if(gSdioData->SDIO_Access_Mode == 1) {

        //
        // If the SDIO access mode is ADMA and controller support it then ADMA 
        // is select else SDMA or PIO
        //
        if((Capability & ADMA2_SUPPORT)&&(SdioDevInfo->bHostControllerVersion>=1)) {
            SdioDevInfo->SDIO_Access=ADMA;
        }else if(Capability & SDMA_SUPPORT) {
            SdioDevInfo->SDIO_Access=SDMA;
        }else {
            SdioDevInfo->SDIO_Access=PIO;
        }
    } else if(gSdioData->SDIO_Access_Mode == 2) {
        //
        // If the SDIO access mode is SDMA and controller support it then SDMA 
        // is select else ADMA or PIO
        //
        if(Capability & SDMA_SUPPORT) {
            SdioDevInfo->SDIO_Access=SDMA;
        }else if((Capability & ADMA2_SUPPORT)&&(SdioDevInfo->bHostControllerVersion>=1)) {
            SdioDevInfo->SDIO_Access=ADMA;
        }else {
            SdioDevInfo->SDIO_Access=PIO;
        }
    }else if(gSdioData->SDIO_Access_Mode == 3) {
        //
        // If the SDIO access mode is PIO and then PIO is selected
        //
        SdioDevInfo->SDIO_Access=PIO;
    }

    return EFI_SUCCESS;
}
/**
    Get the Sd/MMC card Device Name and concatenate it with the
    string formed based on the size of SD card if PNM is not Zero.

    @param  SdioDevInfo
    @param  Port

    @retval EFI_STATUS

    @note
    If PNM is zero then SD Memory Card - Device (Index) is displayed for SD.
    MMC Memory Card - Device (Index) is displayed for MMC device. Index is
    varied for the no. of devices detected without PNM.

**/
EFI_STATUS
SDMmcMassGetPNMWithDeviceClass (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    UINT8   *TempUint8;
    UINT8   i;
    UINT8   j;
    UINT64  CardSize;
    BOOLEAN IsPNMZero = FALSE;
    static  UINT8 SDDevIndex='0';
    static  UINT8 MMCDevIndex='0';
    BOOLEAN SDCardWithOutPNMDetected=FALSE;

    TempUint8=(UINT8*)&SdioDevInfo->d4CID[3];

    //
    // Check if PNM is Zero.
    //
    for(j=0 ; j<5 ; j++,TempUint8--) {
        if( *TempUint8 != '0' ) {
            break;
        }
    }

    if( j == 5 ) {
       IsPNMZero = TRUE;
    }

    if ( (SdioDevInfo->bMode == MMC_STAND_CAP) || \
            (SdioDevInfo->bMode == MMC_HIGH_CAP) ) {
        //
        // If PNM is Zero display String MMCDevnameForPNMZero
        //
        if(IsPNMZero) {
            TempUint8 = (UINT8*)MMCDevnameForPNMZero;
        } else {
            TempUint8 = (UINT8*)"MMC - ";
        }
    } else {
        //
        // If PNM is Zero display String SDDevnameForPNMZero
        //
        if(IsPNMZero) {
            TempUint8 = SDDevnameForPNMZero;
            SDCardWithOutPNMDetected = TRUE;
        } else {
            //
            //Calculate the SD Card Size.
            //
            CardSize = ((UINT64)(SdioDevInfo->dMaxLBA) * 512);
            //
            //Assign name to display in setup based on the Size.
            //
            if( CardSize <= TWO_GB ) {
                TempUint8 = (UINT8*)"SDSC - ";
            } else if( CardSize <= THIRTY_TWO_GB ) {
                TempUint8 = (UINT8*)"SDHC - ";
            } else {
                TempUint8 = (UINT8*)"SDXC - ";
            }
        }
    }

    if(!IsPNMZero) {

        for(i=0; *TempUint8 != 0; i++,TempUint8++) {
            SdioDevInfo->PNM[i]=*TempUint8;
        }

        TempUint8=(UINT8*)&SdioDevInfo->d4CID[3];
        //
        // Concatenate product name with above assigned name.
        //
        for(j=0;j<5;j++,TempUint8--) {
            // if there is non printable character, fill with space
            if(*TempUint8 > 0x1F) {
                SdioDevInfo->PNM[i+j]= *TempUint8;
            } else {
                SdioDevInfo->PNM[i+j]= ' ';
            }
        }

        if ( (SdioDevInfo->bMode == MMC_STAND_CAP) || \
            (SdioDevInfo->bMode == MMC_HIGH_CAP)) {
            if(*TempUint8 > 0x1F) {
                SdioDevInfo->PNM[i+j]= *TempUint8;
            } else {
                SdioDevInfo->PNM[i+j]= ' ';
            }
            j++;
        }
        SdioDevInfo->PNM[i+j]=0;

    } else {

        for(i=0; *TempUint8 != 0; i++,TempUint8++) {
            SdioDevInfo->PNM[i]=*TempUint8;
        }

        //
        // Append the Index to the string to differentiate between two
        // Cards with PNM equal to Zero.
        //
        if(SDCardWithOutPNMDetected) {
            SdioDevInfo->PNM[i++] = SDDevIndex++;
        } else {
            if ( SdioDevInfo->MmcBusWidth == MMC_1_BIT_BUS_WIDTH ) {
                SdioDevInfo->PNM[i++] = MMCDevIndex - 1;
            } else {
                SdioDevInfo->PNM[i++] = MMCDevIndex++;
            }
        }

        SdioDevInfo->PNM[i]=0;
    }

    return EFI_SUCCESS;
}
/**
    Get the SDIO device information. It return if memory is present and 
    how many IO functions are present in the device.

    @param  SdioDevInfo 
    @param  Port 
    @param  NumberOfIo 
    @param  MemoryPresent 

    @retval EFI_STATUS
    Note: This function is following section 3.2 The IO_SEND_OP_COND Command
    (CMD5) and 3.3 The IO_SEND_OP_COND Response (R4) of Spec SDIO Simplified 
    Specification Version 3.00

**/
EFI_STATUS
SDCard_CheckIO (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port,
    IN  UINT8               *NumberOfIo,
    IN  UINT8               *MemoryPresent
)
{

    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    UINT32      ResponceData=0;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    EFI_STATUS  Status;

    //
    //Issue CMD5
    //
    CommandIndex=(IO_SEND_OP_COND_CMD5 << 8) | (RESP4_TYPE);
    CommandArgument= 0x000000;
    Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // According to section 3.3 The IO_SEND_OP_COND Response (R4) of SDIO 
    // Simplified  Specification Version 3.00. Bit27 is Memory Present Bit
    // and Bit28-Bit30 is Number of IO bits
    //
    ResponceData=SDIO_REG32(SdioBaseAddr,RESP0);
    *NumberOfIo= ((ResponceData >> 28) & 07);
    *MemoryPresent= ((ResponceData >> 27) & 01);

    return EFI_SUCCESS;
}

#if CHECK_FOR_SD_IO_DEVICE
/**
    Check the Connected SD device is I/O device or not 

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: This function is implemented as per sec 3.1.2 
    Initialization by I/O Aware Host of SDIO Simplified 
    Specification Version 3.00
    If the connected device is IO then return success 
    else return EFI_NOT_FOUND.
**/
EFI_STATUS
SdioCheckDevice (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{

    EFI_STATUS  Status;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    UINT8       MemoryPresent;
    UINT8       NumberOfIO;

    Status= ControllerInitEnvironment(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Issue CMD0
    //
    CommandIndex=GO_IDLE_STATE_CMD0;
    CommandArgument=0;
    Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // For SDHC and SDXC CMD8 is mandatory 
    //
    CommandIndex=(SEND_IF_COND_CMD8 << 8) | (RESP7_TYPE);
    
    // 
    // The CMD8's lower 8 bit is check patern and BIT8-BIT is VHS
    // (Voltage Suplied). VHS=0001 is 2.7-3.6V.
    //
    CommandArgument=0x1aa;
    SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);

    //
    // Check , SD device has only IO function.
    //
    Status= SDCard_CheckIO(SdioDevInfo,Port,&NumberOfIO,&MemoryPresent);
    if((EFI_ERROR(Status)) || MemoryPresent != 0 || NumberOfIO == 0) {
        return EFI_NOT_FOUND;
    } 

    //
    // It's an SD I/O Device and only read the Manufacture name and display it.
    //      
    SdioDevInfo->IODevice=TRUE;
    return EFI_SUCCESS;
}
#endif

/**
    Get the Index no for the SdioBaseAddress

    @param  SdioBaseAddress

    @retval DeviceIndex

**/
UINT8
GetDevEntry (
    IN  UINT64   SdioBaseAddress
)
{
    UINT8   i;

    for(i=0;i<MAX_SDIO_DEVICES;i++) {
        if(gSdioConfiguration.SdioEmu[i].SdMmcAddress == SdioBaseAddress) {
            return i;
        }
    }

    return 0xFF;

}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
