//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioDriver.c 15    3/07/12 4:19a Rajeshms $
//
// $Revision: 15 $
//
// $Date: 3/07/12 4:19a $
//**********************************************************************

//<AMI_FHDR_START>
//**********************************************************************
//
// Name:    SdioDriver.c
//
// Description: SDIO controller detection and configuration.
//**********************************************************************
//<AMI_FHDR_END>
#define SDIO_WRITE_PROTECT_ERR          0x003   // Write protect error

#include "SdioDriver.h"
#include <Protocol\SdioInt13Protocol.h>
#if !SDIO_SMM_SUPPORT
#include "SdioSmm.h"
#endif
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
#include <Protocol\SmmControl2.h>
#else
#include <Protocol\SmmControl.h>
#endif

//
// 4KB Memory for Transfer Buffer.
// For DMA transfer it should be 4KB alligned. 
//   
#define SDIO_SIZE  4096

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

CHAR16 *gSdioBusDriverName = L"AMI SDIO Driver";


#if SDIO_SMM_SUPPORT
    EFI_GUID gSdioSmmNonSmmProtocolGuid     = SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL_GUID;
    SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL *gSdioSmmNonSmmInterface = NULL;
#else
    extern EFI_GUID gSdioSmmNonSmmProtocolGuid;
    extern SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL *gSdioSmmNonSmmInterface;
#endif

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
EFI_SMM_CONTROL2_PROTOCOL        *gSmmCtl=NULL;
#else
EFI_SMM_CONTROL_PROTOCOL         *gSmmCtl=NULL;
#endif
EFI_SDIO_PROTOCOL               *gAmiSdio=NULL;

#if !SDIO_SMM_SUPPORT
extern VOID
SdioAPIHandler (
	    SDIO_STRUC   *SdioURP );
#endif

// EFI standard functions
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SdioDriverEntryPoint
//
// Description: Installs gSdioDriverBinding protocol
//
// Input:
//  IN EFI_HANDLE        ImageHandle,
//  IN EFI_SYSTEM_TABLE  *SystemTable
//
// Output:
//  EFI_STATUS
//
// Notes:
//  Here is the control flow of this function:
//  1. Initialize Ami Lib.
//  2. Install Driver Binding Protocol
//  3. Return EFI_SUCCESS.
//
//<AMI_PHDR_END>
//**********************************************************************
// EFI_DRIVER_ENTRY_POINT(SdioDriverEntryPoint)

EFI_STATUS
SdioDriverEntryPoint (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )

{
    EFI_STATUS  Status;
    SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL *SdioSmmNonSmmInterface = NULL;
    gSdioDriverBinding.DriverBindingHandle=ImageHandle;
    gSdioDriverBinding.ImageHandle=ImageHandle;

    InitAmiLib(ImageHandle, SystemTable);

    Status = pBS->AllocatePool (EfiRuntimeServicesData,
                                sizeof(SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL),
                                &SdioSmmNonSmmInterface);
    ASSERT_EFI_ERROR(Status);

    Status = pBS->InstallMultipleProtocolInterfaces(
                        &gSdioDriverBinding.DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid, &gSdioDriverBinding,
                        &gSdioSmmNonSmmProtocolGuid, SdioSmmNonSmmInterface,
                        &gEfiComponentName2ProtocolGuid, &gSdioCtlDriverName,
                        NULL
                        );
    ASSERT_EFI_ERROR(Status);
    //
    // Locating Protocol
    //
    Status=pBS->LocateProtocol(&gSdioSmmNonSmmProtocolGuid,NULL,&gSdioSmmNonSmmInterface);
    ASSERT_EFI_ERROR(Status);
    gSdioSmmNonSmmInterface->Address=0;

    return Status;
}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: SdioCtlDriverName
//
// Description:
//  It returns the Driver name of this SDIO Driver(AMI SDIO Driver)
//
// Input:
//  IN EFI_COMPONENT_NAME2_PROTOCOL  *This,
//  IN CHAR8                        *Language,
//  OUT CHAR16                      **DriverName
//
// Output:
//  EFI_SUCCESS - returns driver name address
//  EFI_INVALID_PARAMETER - If input parameter is not valid.
//  EFI_UNSUPPORTED  - If Language requested is not english.
//
// Modified:
//
// Referrals:
//  EFI_COMPONENT_NAME2_PROTOCOL
//
// Notes:
//---------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS
SdioCtlDriverName (
    IN EFI_COMPONENT_NAME2_PROTOCOL  *This,
    IN CHAR8                        *Language,
    OUT CHAR16                      **DriverName
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

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: SdioCtlGetControllerName
//
// Description:
//  It returns the Device name of the SD/MMC card.
//
// Input:
//  IN EFI_COMPONENT_NAME2_PROTOCOL  *This,
//  IN EFI_HANDLE                   Controller,
//  IN EFI_HANDLE                   ChildHandle        OPTIONAL,
//  IN CHAR8                        *Language,
//  OUT CHAR16                      **ControllerName
//
// Output:
//  EFI_SUCCESS - returns Device name address
//  EFI_INVALID_PARAMETER - If input parameter is not valid.
//  EFI_UNSUPPORTED  - If Language requested is not english.
//
// Modified:
//
// Referrals:
//  EFI_COMPONENT_NAME2_PROTOCOL
//
// Notes:
//---------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS
SdioCtlGetControllerName(
    IN EFI_COMPONENT_NAME2_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_HANDLE                   ChildHandle        OPTIONAL,
    IN CHAR8                        *Language,
    OUT CHAR16                      **ControllerName
)
{

    UINT8       PortNo=0;
    SDIO_BUS_PROTOCOL       *SdioBusInterface = NULL;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;

    //
    //Supports only "eng"
    //
    if(!Language || !ControllerName) {
        return EFI_INVALID_PARAMETER;
    }

    if (!LanguageCodesEqual( Language, LANGUAGE_CODE_ENGLISH)) {
        return EFI_UNSUPPORTED;
    }

    //
    // We are not handling any Child Device, we open by EFI_OPEN_PROTOCOL_BY_DRIVER only.
    //
    if (ChildHandle != NULL) {
        return EFI_UNSUPPORTED;
    }

    //
    // Locate the Sdio Bus Interface and get the Sdio Device interface and get the string
    // of the Device.
    //
    if (EFI_ERROR(pBS->HandleProtocol(Controller, &gSdioBusInitProtocolGuid, &SdioBusInterface)))  {
        return EFI_UNSUPPORTED;
    }

    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, PortNo);

    if (SdioDevInterface && (SdioDevInterface->DeviceState == DEVICE_CONFIGURED_SUCCESSFULLY) ){
        *ControllerName   = SdioDevInterface->UnicodePNMString;
        return EFI_SUCCESS;
    }

    return EFI_UNSUPPORTED;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SdioDriverSupported
//
// Description: Checks whether the given controller is Blck I/O or not
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
//  IN EFI_HANDLE                     Controller,
//  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
//
// Notes:
//  Here is the control flow of this function:
//  1. Check whether PCI and Devicepath protocol has been installed on this controller
//  2. Check if the controller is Block I/O controller
//  3. If Block I/O Controller protocol already installed, return EFI_ALREADY_STARTED
//  4. Return EFI_SUCCESS
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
SdioDriverSupported (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
    )
{
    EFI_STATUS                  Status;
    EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
    EFI_PCI_IO_PROTOCOL         *PciIo;
    PCI_STD_DEVICE              Pci;

    //
    //Check whether DevicePath Protocol has been installed on this controller
    //    
    Status = pBS->OpenProtocol( Controller,
                    &gEfiDevicePathProtocolGuid,
                    (void **)&ParentDevicePath,
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
                    (void **)&PciIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER );

    if  ( EFI_ERROR(Status) ) {
        return Status;
    }

    //
    //See if this is a PCI Mass Storage Controller by looking at the Class Code Register
    //
    Status = PciIo->Pci.Read (PciIo,
                    EfiPciIoWidthUint32,
                    0,
                    sizeof (Pci) / sizeof (UINT32),
                    &Pci);

    if  ( EFI_ERROR(Status) ) {
            goto Done;
    }

    if ( Pci.Header.ClassCode[1] != PCI_CL_SYSTEM_PERIPHERALS_SCL_SD || \
         Pci.Header.ClassCode[2] != PCI_CL_SYSTEM_PERIPHERALS ) {
            Status = EFI_UNSUPPORTED;
            goto Done;
    }

    //  Check if BlockIoProtocol already installed
    Status = pBS->OpenProtocol( Controller,
                &gEfiBlockIoProtocolGuid,
                NULL,                   // Optional
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_TEST_PROTOCOL );

    if  ( Status == EFI_SUCCESS ) {
        Status = EFI_ALREADY_STARTED;
    } else {
        Status = EFI_SUCCESS;
    }

Done:
    pBS->CloseProtocol( Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller );

    return Status;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SdioDriverStart
//
// Description: Installs BlockIoProtocol
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
//  IN EFI_HANDLE                     Controller,
//  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
//
// Notes:
//  Here is the control flow of this function:
//  1. Open PCI and Devicepath protocol
//  2. Enable the device
//  3. Post the option rom
//  4. If first time, allocate buffer for real mode thunk code
//  5. For each disk...
//     a. Allocate and initialize a private structure
//     b. Install block I/O protocol on a new child device
//     c. Open the child device
//  6. Increment user counter
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
SdioDriverStart (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
)

{
    EFI_STATUS                Status;
    EFI_PCI_IO_PROTOCOL       *PciIo;
    EFI_DEVICE_PATH_PROTOCOL  *PciDevPath;
    UINTN                      Pages=0;
    EFI_PHYSICAL_ADDRESS       TransferBuffer=0;
    SDIO_BUS_PROTOCOL         *SdioBusInterface;
    UINT8               	   PortNo=0;
    UINT64                     Attributes;

    PciIo = NULL;

    //
    //Open PCI I/O Protocol
    //
    Status = pBS->OpenProtocol( Controller,
                &gEfiPciIoProtocolGuid,
                (void **)&PciIo,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_BY_DRIVER );

    if  ( EFI_ERROR(Status) ) {
        return Status;
    }

    //  Open Device Path Protocol
    Status = pBS->OpenProtocol( Controller,
                &gEfiDevicePathProtocolGuid,
                (void**)&PciDevPath,
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
                                    sizeof(SDIO_BUS_PROTOCOL),
                                    (VOID**)&SdioBusInterface
                                    );
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;

        Status = InstallSdioBusProtocol (Controller, SdioBusInterface, PciIo);
        if (EFI_ERROR(Status)) return Status;
    }

    Status = PciIo->Attributes( PciIo,
                                EfiPciIoAttributeOperationGet,
                                0,
                                &Attributes );

    //
    // PCI device Attributes can not be 0. With OLD PCI bus driver we are getting 0.
    // to Workaround this problem enabled all the resource.
    //
//    if(Attributes == 0) {
        //
        // With the EFI_PCI_DEVICE_ENABLE Old PCI driver returns UNSUPPORTED if the device
        // does not support IO. So,setting the atributes separately.
        //
        Attributes = EFI_PCI_IO_ATTRIBUTE_MEMORY;
        PciIo->Attributes( PciIo,
                           EfiPciIoAttributeOperationEnable,
                           Attributes,
                           NULL );

        Attributes = EFI_PCI_IO_ATTRIBUTE_BUS_MASTER;
        PciIo->Attributes( PciIo,
                           EfiPciIoAttributeOperationEnable,
                           Attributes,
                           NULL );

        Attributes = EFI_PCI_IO_ATTRIBUTE_IO;
        PciIo->Attributes( PciIo,
                           EfiPciIoAttributeOperationEnable,
                           Attributes,
                           NULL );
//    } else {

//        Status = PciIo->Attributes( PciIo,
//                                    EfiPciIoAttributeOperationEnable,
//                                    Attributes,
//                                    NULL );
//        if(EFI_ERROR(Status)) {
//            goto Done;
//        }
//    }

    //
    // Allocating Page (4KB of Memory) for Transfer Buffer
    //
    Pages = EFI_SIZE_TO_PAGES (SDIO_SIZE);
    Status = pBS->AllocatePages (AllocateAnyPages,
                                 EfiRuntimeServicesData,
                                 Pages,
                                 &TransferBuffer);
    ASSERT_EFI_ERROR (Status);

    SdioBusInterface->TransferBufferAddress = (UINT32)TransferBuffer;

    Status = DetectAndConfigureDevice(This, Controller, RemainingDevicePath, SdioBusInterface,PortNo);
    
    return Status;

//Done:
    pBS->CloseProtocol( Controller,
                        &gEfiPciIoProtocolGuid,
                        This->DriverBindingHandle,
                        Controller );
    pBS->CloseProtocol( Controller,
                        &gEfiDevicePathProtocolGuid,
                        This->DriverBindingHandle,
                        Controller );
    return Status;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SdioDriverStop
//
// Description: Installs IdeControllerProtocol
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
//  IN EFI_HANDLE                     Controller,
//  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
//
// Notes:
//  Here is the control flow of this function:
//  1. Decrement user counter
//  2. Free global buffer
//  3. Release PCI I/O protocol and Block I/O protocol for each child handle.
//  4. Shut down the hardware for each child handle.
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
SdioDriverStop (
    IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
    IN  EFI_HANDLE                      Controller,
    IN  UINTN                           NumberOfChildren,
    IN  EFI_HANDLE                      *ChildHandleBuffer
)

{
    SDIO_BUS_PROTOCOL           *SdioBusInterface;
    SDIO_DEVICE_INTERFACE       *SdioDeviceInterface;
    EFI_STATUS                  Status;
    EFI_BLOCK_IO_PROTOCOL       *BlockIo;
    UINT8                       i;
    EFI_PCI_IO_PROTOCOL         *PciIo;
    EFI_DEVICE_PATH_PROTOCOL    *PciDevPath;


    //  Check if SDIO_BUS_PROTOCOL is installed on the Controller.
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
    //Check if ChildHandleBuffer is valid
    //
    for (i=0; i<NumberOfChildren; i++) {

        //
       //Before uninstalling BLOCKIO check whether it is installed or not
       //
        Status = pBS->OpenProtocol( ChildHandleBuffer[i],
                                &gEfiBlockIoProtocolGuid,
                                (void **)&BlockIo,
                                This->DriverBindingHandle,
                                ChildHandleBuffer[i],
                                EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        SdioDeviceInterface=((SDIO_BLOCK_IO *)BlockIo)->SdioDevInterface;
        Status = pBS->UninstallMultipleProtocolInterfaces (
                        ChildHandleBuffer[i],
                        &gEfiBlockIoProtocolGuid, (EFI_BLOCK_IO_PROTOCOL *)(SdioDeviceInterface->SdioBlkIo),
                        NULL);
		ASSERT_EFI_ERROR(Status);
		
       //
       //Shutdown the hardware
       //
       SdioBusInterface->PciIO->Attributes (
                               SdioBusInterface->PciIO,
                               EfiPciIoAttributeOperationDisable,
                               EFI_PCI_DEVICE_ENABLE,
                               NULL);

       pBS->CloseProtocol( Controller,
                           &gEfiPciIoProtocolGuid,
                           This->DriverBindingHandle,
                           ChildHandleBuffer[i]);

       Status = pBS->CloseProtocol( Controller,
                       &gEfiDevicePathProtocolGuid,
                       This->DriverBindingHandle,
                       Controller );
    }

    //
    //Close all the protocols opened in Start Function
    //

    Status = pBS->CloseProtocol( Controller,
                                &gEfiPciIoProtocolGuid,
                                This->DriverBindingHandle,
                                Controller);

    Status = pBS->CloseProtocol( Controller,
                                &gEfiDevicePathProtocolGuid,
                                This->DriverBindingHandle,
                                Controller);

    Status = pBS->CloseProtocol( Controller,
                                &gSdioBusInitProtocolGuid,
                                This->DriverBindingHandle,
                                Controller);
	ASSERT_EFI_ERROR(Status);
    Status = pBS->UninstallProtocolInterface ( Controller,
                                        &gSdioBusInitProtocolGuid,
                                        SdioBusInterface);

    if (EFI_ERROR(Status))  {

    //
    //Open PCI I/O Protocol
    //
    Status = pBS->OpenProtocol( Controller,
                &gEfiPciIoProtocolGuid,
                (void **)&PciIo,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_BY_DRIVER );

    //  Open Device Path Protocol
    Status = pBS->OpenProtocol( Controller,
                &gEfiDevicePathProtocolGuid,
                (void**)&PciDevPath,
                This->DriverBindingHandle,
                Controller,
                EFI_OPEN_PROTOCOL_BY_DRIVER );



        Status = pBS->OpenProtocol( Controller,
                            &gSdioBusInitProtocolGuid,
                            (VOID **)&SdioBusInterface,
                            This->DriverBindingHandle,
                            Controller,
                            EFI_OPEN_PROTOCOL_BY_DRIVER);
		ASSERT_EFI_ERROR(Status);
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InstallSdioBusProtocol
//
// Description: Installs BUS Init Protocol on the IDE controller Handle
//
// Input:
//  IN EFI_HANDLE                   Controller,
//  IDE_BUS_INIT_PROTOCOL           *IdeBusInitInterface,
//  IDE_CONTROLLER_PROTOCOL         *IdeControllerInterface,
//  EFI_PCI_IO_PROTOCOL             *PciIO
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals: AllocatePool, InstallProtocolInterface, SdioInitController
//
// Notes:
//   1. Call SdioInitController
//   2. Install gSdioBusInitProtocolGuid protocol
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
InstallSdioBusProtocol (
    IN EFI_HANDLE                       Controller,
    IN OUT SDIO_BUS_PROTOCOL            *SdioBusInterface,
    IN EFI_PCI_IO_PROTOCOL              *PciIO
 )
{

    EFI_STATUS                  Status;

    // Initialize the default Values
    ZeroMemory (SdioBusInterface, sizeof(SDIO_BUS_PROTOCOL));

    SdioBusInterface->ControllerHandle = Controller;
    SdioBusInterface->PciIO = PciIO;

    // Init Sdio Controller
    Status = SdioInitController(SdioBusInterface);
    if (EFI_ERROR(Status)) return Status;

    Status = pBS->InstallProtocolInterface(
                    &Controller,
                    &gSdioBusInitProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    SdioBusInterface);

    return Status;

}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SdioInitController
//
// Description: Initializes Sdio Controller
//
// Input:
// IN OUT   Sdio_BUS_PROTOCOL       *SdioBusInterface
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//  1. Update internal Data area about the Sdio controller Capabilities.
//  2. Allocate memory for FIS and CommandList
//  3. Enable Sdio mode
//  3. Disable all the ports
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
SdioInitController (
    IN OUT SDIO_BUS_PROTOCOL        *SdioBusInterface
)
{
    EFI_STATUS      Status;
    UINT8           PciConfig[40];

    //  Make sure Sdio Base address is programmed Properly
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

    SdioBusInterface->SdioBaseAddress =((*(UINT32 *)(PciConfig + PCI_BAR0)) & 0xFFFFFFF0);
    if (!SdioBusInterface->SdioBaseAddress) return EFI_DEVICE_ERROR;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetSdioDevInterface
//
// Description:
//
// Input:
//  IN SDIO_BUS_PROTOCOL                   *SdioBusInterface,
//  IN UINT8                               Port,
//  IN UINT8                               PMPort,
//
// Output:
//      Sdio_DEVICE_INTERFACE*
//
// Modified:
//
// Referrals:
//
// Notes:
//  1. Return the Pointer to the Sdio_DEVICE_INTERFACE for the given Port and PM Port
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
SDIO_DEVICE_INTERFACE *
GetSdioDevInterface(
    IN SDIO_BUS_PROTOCOL                   *SdioBusInterface,
    IN UINT8                               Port
)
{

    DLINK                   *dlink = SdioBusInterface->SdioDeviceList.pHead;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;

    if (!dlink) return NULL;
    do {
        SdioDevInterface = OUTTER(dlink, SdioDeviceLink, SDIO_DEVICE_INTERFACE);
        if (SdioDevInterface->PortNumber == Port) break;
        dlink = dlink-> pNext;
        SdioDevInterface = NULL;
    }while (dlink);

    return SdioDevInterface;

}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   DetectAndConfigureDevice
//
// Description: Detects and Configures Sdio Device
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
//  IN EFI_HANDLE                     Controller
//  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath,
//  Sdio_BUS_PROTOCOL                 *SdioBusInterface,
//  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *IdeControllerInterface,
//  UINT8                             Port,
//  UINT8                             PMPort
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals: SdioDetectDevice ConfigureDevice InitSdioBlockIO
//
// Notes:
// 1. Detect whether device is connected to the port. If no device exit.
// 2. Install SdioDevInterface. If PMPort, Configure PMPort and Exit.
// 3. Configure the Sdio device and the controller.
// 4. Install DevicePath, BlockIO and DiskInfo protocol.
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
DetectAndConfigureDevice (
    IN EFI_DRIVER_BINDING_PROTOCOL      *This,
    IN EFI_HANDLE                       Controller,
    IN EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath,
    SDIO_BUS_PROTOCOL                   *SdioBusInterface,
    UINT8                               Port
)
{

    EFI_STATUS                  Status;
    SDIO_DEVICE_INTERFACE       *SdioDevInterface = NULL;
    UINT8                       Index;

    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, Port);

    if (SdioDevInterface && ((SdioDevInterface->DeviceState == DEVICE_DETECTION_FAILED)||
        (SdioDevInterface->DeviceState == DEVICE_CONFIGURED_SUCCESSFULLY))){
        return EFI_SUCCESS;
    }

    Status = SdioDetectDevice(SdioBusInterface,Port);
    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, Port);
    if (EFI_ERROR(Status)) {
        if (SdioDevInterface) SdioDevInterface->DeviceState = DEVICE_DETECTION_FAILED;
        return EFI_DEVICE_ERROR;
    }

    SdioDevInterface->SdioDeviceHandle=Controller;
    SdioDevInterface->DeviceState = DEVICE_CONFIGURED_SUCCESSFULLY;

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
       SdioDevInterface->UnicodePNMString[Index] = (CHAR16)SdioDevInterface->PNM[Index];
    }
    
    //
    // The detected device is Mass storage device. So initilize the Block IO and Legacy Int13
    // Functions for the SDIO device.
    //

    // Initialize Block_IO Protocol
    Status = InitSdioBlockIO (SdioDevInterface);
    if (EFI_ERROR(Status)) { return EFI_DEVICE_ERROR; }


    Status = pBS->InstallMultipleProtocolInterfaces (
                    &(SdioDevInterface->SdioDeviceHandle),
                    &gEfiBlockIoProtocolGuid, (EFI_BLOCK_IO_PROTOCOL *)(SdioDevInterface->SdioBlkIo),
                    NULL);

    //
    //Install Legacy boot Support
    //

    if(gAmiSdio == NULL) {
        Status = pBS->LocateProtocol(&gEfiSdioProtocolGuid, NULL, &gAmiSdio);
		ASSERT_EFI_ERROR(Status);
    }

    if(gAmiSdio != NULL) {
        SDIO_DEV_INFO   SdioDevInfo;
        SDIO_MASS_DEV   SdioMassDevInfo;
        UINTN       PciSeg, PciBus, PciDev, PciFunc;


        SdioDevInfo.wBlockSize=SdioDevInterface->wBlockSize;
        SdioDevInfo.dMaxLba=SdioDevInterface->dMaxLBA;
        SdioDevInfo.bHeads=SdioDevInterface->NumHeads;
        SdioDevInfo.bSectors=SdioDevInterface->NumSectors;
        SdioDevInfo.wCylinders=SdioDevInterface->NumCylinders;
        SdioDevInfo.bNonLBAHeads=SdioDevInterface->LBANumHeads;
        SdioDevInfo.bNonLBASectors=SdioDevInterface->LBANumSectors;
        SdioDevInfo.wNonLBACylinders=SdioDevInterface->LBANumCyls;

        //
        // Get the SDIO controller Bus,Dev and Fun
        //
        SdioBusInterface->PciIO->GetLocation (SdioBusInterface->PciIO, &PciSeg, &PciBus, &PciDev, &PciFunc);

        SdioMassDevInfo.PciBDF=(UINT16)((PciBus << 8)+(PciDev << 3) + PciFunc);
        SdioMassDevInfo.DevString=&SdioDevInterface->PNM[0];
        SdioMassDevInfo.DevInfo=(VOID*)&SdioDevInfo;
        SdioMassDevInfo.LogicalAddress=SdioDevInterface->DeviceAddress;
        SdioMassDevInfo.StorageType = SdioDevInterface->StorageType;
        SdioMassDevInfo.Handle=SdioDevInterface->SdioDeviceHandle;
        gAmiSdio->SdioInstallLegacyDevice(&SdioMassDevInfo);
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SdioDetectDevice
//
// Description:  Detects a Sdio device connected to given Port and PMPort
//
// Input:
//  IN SDIO_BUS_PROTOCOL                   *SdioBusInterface,
//  IN UINT8                               Port,
//
// Output:
//   EFI_STATUS
//
// Modified:
//
// Referrals:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdioDetectDevice (
    IN SDIO_BUS_PROTOCOL                   *SdioBusInterface,
    IN UINT8                               Port
)
{


    EFI_STATUS              Status;
    SDIO_DEVICE_INTERFACE   *SdioDevInterface = NULL;


    SdioDevInterface = GetSdioDevInterface(SdioBusInterface, Port);

    if (!SdioDevInterface){
        // A device is present.
        Status = pBS->AllocatePool (EfiBootServicesData,
                    sizeof(SDIO_DEVICE_INTERFACE),
                    (VOID**)&SdioDevInterface);
        if (EFI_ERROR(Status)) return Status;

        ZeroMemory (SdioDevInterface, sizeof(SDIO_DEVICE_INTERFACE));

        SdioDevInterface->PortNumber = Port;
        SdioDevInterface->SdioBusInterface = SdioBusInterface;
        SdioDevInterface->DeviceState = DEVICE_IN_RESET_STATE;

        //
        // Add to the SdioBusInterface
        //
        DListAdd(&(SdioBusInterface->SdioDeviceList), &(SdioDevInterface->SdioDeviceLink));

    }

    Status = CheckDevicePresence (SdioDevInterface, Port);

    if (EFI_ERROR(Status)) {
        if (SdioDevInterface->DeviceState == DEVICE_IN_RESET_STATE) {
            DListDelete(&(SdioBusInterface->SdioDeviceList), &(SdioDevInterface->SdioDeviceLink));
            pBS->FreePool(SdioDevInterface);
        }
        return EFI_DEVICE_ERROR;
    }


    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckDevicePresence
//
// Description: Check if any device is connected to the port
//
// Input:
//  IN SATA_DEVICE_INTERFACE            *SataDevInterface,
//  IN UINT8                               Port,
//  IN UINT8                               PMPort
//
// Output:
//   EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CheckDevicePresence (
    IN SDIO_DEVICE_INTERFACE                *SdioDevInterface,
    IN UINT8                                Port
)
{

    SDIO_BUS_PROTOCOL           *SdioBusInterface = SdioDevInterface->SdioBusInterface;
    UINT32                      SdioBaseAddr = (UINT32)(SdioBusInterface->SdioBaseAddress);
    SDIO_STRUC                  Parameters;
    UINT8                       i;

    //
    // Check the Device Presence
    //
    Parameters.bFuncNumber = SDIO_API_DEVICE_DETECT;
    Parameters.ApiData.ControllerInfo.SdioBaseAddress = SdioBaseAddr;
    Parameters.ApiData.ControllerInfo.TransferBufferAddress=SdioBusInterface->TransferBufferAddress;

    SdioHandler (&Parameters);

    if(Parameters.bRetValue != SDIO_SUCCESS) {
        return EFI_DEVICE_ERROR;
    }

    if(Parameters.ApiData.ControllerInfo.DeviceDetected == FALSE) {
        return EFI_NOT_FOUND;
    }

    //
    // Device Present, Save the Device Address
    //
    SdioDevInterface->DeviceAddress=Parameters.ApiData.ControllerInfo.DeviceAddress;


    //
    // Configure the Device
    //
    Parameters.bFuncNumber = SDIO_API_DEVICE_CONFIGURE;
    Parameters.ApiData.ControllerInfo.DeviceAddress=SdioDevInterface->DeviceAddress;
    Parameters.ApiData.ControllerInfo.Port = Port;

    SdioHandler (&Parameters);

    if(Parameters.bRetValue != SDIO_SUCCESS) {
        return EFI_DEVICE_ERROR;
    }

    if(Parameters.ApiData.ControllerInfo.SdIODevice == TRUE) {
        //
        // It's an IO device. Just get the Manufacture Id 
        //
        for(i=0;i<SDIO_MANUFACTUREID_LENGTH;i++) {
            SdioDevInterface->SdIOManufactureId[i]= Parameters.ApiData.ControllerInfo.SdIOManufactureId[i];
        }

        //
        // The SDIO device present in the port is not a MASS storage device
        //
        SdioDevInterface->MassStorageDevice=FALSE;

        return EFI_SUCCESS;
    }

    //
    // Get the SDIO MASS storage Device Details.
    //
    Parameters.bFuncNumber = SDIO_API_GET_MASS_DEVICE_DETAILS;
    Parameters.ApiData.ControllerInfo.DeviceAddress=SdioDevInterface->DeviceAddress;
    Parameters.ApiData.ControllerInfo.Port = Port;

    SdioHandler (&Parameters);

    if(Parameters.bRetValue != SDIO_SUCCESS) {
        return EFI_NOT_FOUND;
    }

    //
    // Valid Mass Storage Device Found
    //
    SdioDevInterface->MassStorageDevice=TRUE;
    SdioDevInterface->dMaxLBA=Parameters.ApiData.ControllerInfo.MaxLBA;
    SdioDevInterface->wBlockSize=Parameters.ApiData.ControllerInfo.BlockSize;
    SdioDevInterface->NumHeads=Parameters.ApiData.ControllerInfo.NumHeads;
    SdioDevInterface->NumSectors=Parameters.ApiData.ControllerInfo.NumSectors;
    SdioDevInterface->NumCylinders=Parameters.ApiData.ControllerInfo.NumCylinders;
    SdioDevInterface->LBANumHeads=Parameters.ApiData.ControllerInfo.LBANumHeads;
    SdioDevInterface->LBANumSectors=Parameters.ApiData.ControllerInfo.LBANumSectors;
    SdioDevInterface->LBANumCyls=Parameters.ApiData.ControllerInfo.LBANumCyls;
    SdioDevInterface->StorageType = Parameters.ApiData.ControllerInfo.StorageType;

    //
    //Save the PNM using added Device Class.
    //
    for(i=0;i<27;i++) {
       SdioDevInterface->PNM[i]= Parameters.ApiData.ControllerInfo.PNM[i];
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOAPI_ReadCard
//
// Description: Read the Data from Sd card
//
// Input:
//    IN SDIO_DEVICE_INTERFACE               *SdioDevInterface
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOAPI_ReadCard (
    IN SDIO_DEVICE_INTERFACE               *SdioDevInterface,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
)
{

    SDIO_STRUC                  Parameters;

    Parameters.bFuncNumber = SDIO_API_READ;

    Parameters.ApiData.Read.DeviceAddress=SdioDevInterface->DeviceAddress;
    Parameters.ApiData.Read.Port = Port;
    Parameters.ApiData.Read.LBA =(UINT32) LBA;
    Parameters.ApiData.Read.BufferAddress =(UINT32*)BufferAddress;
    Parameters.ApiData.Read.NumBlks=(UINT16)NumBlks;

    SdioHandler (&Parameters);

    if(Parameters.bRetValue != SDIO_SUCCESS) {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOAPI_WriteCard
//
// Description: Read the Data from Sd card
//
// Input:
//    IN SDIO_DEVICE_INTERFACE               *SdioDevInterface
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOAPI_WriteCard (
    IN SDIO_DEVICE_INTERFACE               *SdioDevInterface,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
)
{

    SDIO_STRUC                  Parameters;

    Parameters.bFuncNumber = SDIO_API_WRITE;

    Parameters.ApiData.Read.DeviceAddress=SdioDevInterface->DeviceAddress;
    Parameters.ApiData.Read.Port = Port;
    Parameters.ApiData.Read.LBA =(UINT32) LBA;
    Parameters.ApiData.Read.BufferAddress =(UINT32*)BufferAddress;
    Parameters.ApiData.Read.NumBlks=(UINT16)NumBlks;

    SdioHandler (&Parameters);

    #if SDIO_WriteProtect      
    if( Parameters.bRetValue == SDIO_WRITE_PROTECT_ERR ) {        
        return EFI_WRITE_PROTECTED;
    }    
    #endif

    if(Parameters.bRetValue != SDIO_SUCCESS) {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;

}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InitSdioBlockIO
//
// Description: Initializes Sdio Block IO interface
//
// Input:
//  IN Sdio_DEVICE_INTERFACE            *SdioDevInterface
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals: AllocatePool, OpenProtocol, DetectAtapiMedia, AtapiInquiryData
//
// Notes:
//  Here is the control flow of this function:
//  1. Initialize EFI_BLOCK_IO_PROTOCOL Protocol.
//  2. In case of Removable devices, detect Media presence.
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
InitSdioBlockIO (
    IN SDIO_DEVICE_INTERFACE            *SdioDevInterface
 )
{
    EFI_STATUS                  Status;
    EFI_BLOCK_IO_PROTOCOL       *BlkIo;
    SDIO_BLOCK_IO               *SdioBlkIo;
    EFI_BLOCK_IO_MEDIA          *BlkMedia;

    Status = pBS->AllocatePool (EfiBootServicesData,
                sizeof(SDIO_BLOCK_IO),
                (VOID**)&SdioBlkIo);

    if (EFI_ERROR(Status)) return Status;

    BlkMedia = MallocZ(sizeof(EFI_BLOCK_IO_MEDIA));
    if (!BlkMedia) {
        pBS->FreePool (SdioBlkIo);
        return EFI_OUT_OF_RESOURCES;
    }
    SdioDevInterface->SdioBlkIo = SdioBlkIo;

    // Initialize the fields in IdeBlkIo (Sdio_BLOCK_IO)
    SdioBlkIo->SdioDevInterface = SdioDevInterface;

    BlkIo = &(SdioBlkIo->BlkIo);
    BlkIo->Media = BlkMedia;
    BlkIo->Reset = SdioReset;
    BlkIo->ReadBlocks = SdioBlkRead;
    BlkIo->WriteBlocks = SdioBlkWrite;
    BlkIo->FlushBlocks = SdioBlkFlush;

    BlkMedia->MediaId = 0;
    BlkMedia->RemovableMedia = TRUE;

    BlkMedia->MediaPresent = TRUE;
    BlkMedia->LogicalPartition = FALSE;
    BlkMedia->ReadOnly = FALSE;
    BlkMedia->WriteCaching = FALSE;
    BlkMedia->BlockSize = 512;
    BlkMedia->IoAlign = 0;

    //
    // Check for Core Version > 4.6.5.0
    //
#if defined CORE_COMBINED_VERSION && CORE_COMBINED_VERSION > 0x4028a

    if(pST->Hdr.Revision >= 0x0002001F) {
        //
        // Default value set to 1 logical blocks per PhysicalBlock
        //
        BlkMedia->LogicalBlocksPerPhysicalBlock=1;

        //
        // Default value set to 0 for Lowest Aligned LBA
        //
        BlkMedia->LowestAlignedLba=0;
        BlkMedia->OptimalTransferLengthGranularity=BlkMedia->BlockSize;
        
        BlkIo->Revision = EFI_BLOCK_IO_PROTOCOL_REVISION3;
    } else {
        BlkIo->Revision = BLKIO_REVISION;
    }
#else

    BlkIo->Revision = BLKIO_REVISION;
#endif

    BlkMedia->LastBlock = SdioDevInterface->dMaxLBA;

    return EFI_SUCCESS;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   CreateSdioDevicePath
//
// Description: Creates a Sdio device devicepath and adds it to SdioDevInterface
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
//  IN EFI_HANDLE                     Controller,
//  IN Sdio_DEVICE_INTERFACE          *SdioDevInterface
//  IN OUT EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals: AllocatePool, OpenProtocol, IdeBusStart
//
// Notes:
//  Here is the control flow of this function:
//  1.  If Remaining Devicepath is not NULL, we have already verified that it is a
//          valid Atapi device path in IdeBusStart. So nothing to do. Just exit.
//  2.  Build a Atapi devicepath and a End devce path.
//  3.  Get the Devicepath for the IDE controller.
//  3.  Append Atapi devicepath to  IDE controller devicepath.
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
CreateSdioDevicePath (
    IN EFI_DRIVER_BINDING_PROTOCOL      *This,
    IN EFI_HANDLE                       Controller,
    IN SDIO_DEVICE_INTERFACE            *SdioDevInterface,
    IN OUT EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
 )
{

    EFI_STATUS                  Status = EFI_SUCCESS;
/*
    SDIO_DEVICE_PATH            NewDevicePath;
    EFI_DEVICE_PATH_PROTOCOL    *TempDevicePath;

    NewDevicePath.Header.Type = MESSAGING_DEVICE_PATH;
    NewDevicePath.Header.SubType = MSG_USB_SATA_DP;
//  SET_NODE_LENGTH(&NewDevicePath.Header,SDIO_DEVICE_PATH_LENGTH);
    SET_NODE_LENGTH(&NewDevicePath.Header,8);
    NewDevicePath.PortNumber = SdioDevInterface->PortNumber;

    // Append the Device Path
    Status = pBS->OpenProtocol( Controller,
                        &gEfiDevicePathProtocolGuid,
                        (VOID **)&TempDevicePath,
                        This->DriverBindingHandle,
                        Controller,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    SdioDevInterface->DevicePathProtocol = DPAddNode(TempDevicePath, &NewDevicePath.Header);
*/
    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        SDIOGenerateSWSMI
//
// Description: Generates SW SMI using global SmmCtl pointer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SDIOGenerateSWSMI (
    UINT8   Data,
    SDIO_STRUC *Parameters
)
{
    EFI_STATUS  Status;
    UINT8       SwSmiValue = Data;
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    UINT8       DataSize = 1;
#else
    UINTN       DataSize = 1;
#endif

    if(gSmmCtl == NULL) {
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
        Status = pBS->LocateProtocol(&gEfiSmmControl2ProtocolGuid, NULL, &gSmmCtl);
#else
        Status = pBS->LocateProtocol(&gEfiSmmControlProtocolGuid, NULL, &gSmmCtl);
#endif
        if(EFI_ERROR(Status)){
            return ;
        }
    }

    //
    // Storing the SDIO_STRUC Base Address in SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL
    //
    gSdioSmmNonSmmInterface->Address=(UINTN)Parameters;
    gSmmCtl->Trigger(gSmmCtl, &SwSmiValue, &DataSize, 0, 0);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SdioHandler
//
// Description:
//  This function chooses handler [SMIHandler/APIHandler] for SD device
//  communication
//
// Input:
//   IN SDIO_STRUC          *sURP - pointer to SDIO_STRUC
//
// Output:
//   VOID
//
// Modified: sURP
//
// Referrals: SDIOGenerateSWSMI, SdioAPIHandler
//
// Notes:
//     1. if SDIO_SMM_SUPPORT is 1 - then SMIHandler will be called.
//     2. else APIHandler will be called.
//     3. SMI/API handlers are defined in SdioSmm module.
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

VOID
SdioHandler (
    IN SDIO_STRUC          *sURP )
{
#if SDIO_SMM_SUPPORT
	SDIOGenerateSWSMI(SDIO_SWSMI,sURP);
#else
	SdioAPIHandler(sURP);
#endif
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
