//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/efiusbmass.c 40    9/04/12 8:04a Wilsonlee $
//
// $Revision: 40 $
//
// $Date: 9/04/12 8:04a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           EFIUSBMASS.C
//
//  Description:    EFI USB Mass Storage Driver
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"
#include "ComponentName.h"
#include "UsbBus.h"
#include <Setup.h>
#include "UsbMass.h"

#define USBMASS_DRIVER_VERSION 1
#define READ 1
#define WRITE 0

extern  USB_GLOBAL_DATA     *gUsbData;
extern  EFI_USB_PROTOCOL    *gAmiUsbController;


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InstallUSBMass
//
// Description: Installs BlkIo protocol on a USB Mass Storage device
//
// Input:       DevInfo - pointer to a USB device structure to install the protocol.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
InstallUSBMass(
    EFI_HANDLE Controller,
    DEV_INFO* DevInfo
)
{
    USB_MASS_DEV    *MassDev;
    EFI_STATUS      Status;
    UINT8           LogicalAddress;

    //applying check to media not present device only
    if(!(DevInfo->bLastStatus & USB_MASS_MEDIA_PRESENT)) {
        if(gUsbData->dUSBStateFlag & USB_FLAG_MASS_MEDIA_CHECK) {
            if(gUsbData->dUSBStateFlag & USB_FLAG_MASS_SKIP_FDD_MEDIA_CHECK) {
                if(!(DevInfo->bSubClass == SUB_CLASS_UFI)) return;
            }
            else return;
        }
    }

    gBS->AllocatePool(EfiBootServicesData, sizeof(USB_MASS_DEV), &MassDev);

    //
    // Handshaking...
    //
    MassDev->DevInfo    = DevInfo;
    DevInfo->MassDev    = (VOID*)&MassDev->BlockIoProtocol;
    MassDev->Handle     = Controller;
    MassDev->DevString  = (UINT8*)&DevInfo->DevNameString;
    MassDev->StorageType= DevInfo->bStorageType;

    for (LogicalAddress=1; LogicalAddress < MAX_DEVICES; LogicalAddress++) {
        if (&gUsbData->aDevInfoTable[LogicalAddress] == DevInfo) break;
    }
    ASSERT(LogicalAddress<MAX_DEVICES);
    if (LogicalAddress >= MAX_DEVICES) return;

    //
    // Install BLOCK_IO protocol interface
    //
    gBS->AllocatePool(EfiBootServicesData, sizeof(EFI_BLOCK_IO_MEDIA), &MassDev->Media);

    MassDev->Media->MediaId             = 0;        // Media change indicator
    MassDev->Media->RemovableMedia      = TRUE;
    MassDev->Media->MediaPresent        = TRUE;
    MassDev->Media->LogicalPartition    = FALSE;
    MassDev->Media->ReadOnly            = FALSE;
    MassDev->Media->WriteCaching        = FALSE;
    MassDev->Media->BlockSize           = DevInfo->wBlockSize;
    MassDev->Media->IoAlign             = 0;
    MassDev->Media->LastBlock           = DevInfo->dMaxLba-1;   // LastBlock is 0-based


    if (pST->Hdr.Revision >= 0x0002001F) {
        MassDev->BlockIoProtocol.Revision    = EFI_BLOCK_IO_PROTOCOL_REVISION3;
        //
        // Default value set to 1 logical blocks per PhysicalBlock
        //
        MassDev->Media->LogicalBlocksPerPhysicalBlock=1;

        //
        // Default value set to 0 for Lowest Aligned LBA
        //
        MassDev->Media->LowestAlignedLba=0;

        MassDev->Media->OptimalTransferLengthGranularity=MassDev->Media->BlockSize;
    } else {
        MassDev->BlockIoProtocol.Revision    = 1;
    }

    MassDev->BlockIoProtocol.Media        = MassDev->Media;
    MassDev->BlockIoProtocol.Reset        = AmiUsbBlkIoReset;
    MassDev->BlockIoProtocol.ReadBlocks   = AmiUsbBlkIoReadBlocks;
    MassDev->BlockIoProtocol.WriteBlocks  = AmiUsbBlkIoWriteBlocks;
    MassDev->BlockIoProtocol.FlushBlocks  = AmiUsbBlkIoFlushBlocks;

    MassDev->LogicalAddress = LogicalAddress;

    MassDev->PciBDF = gUsbData->HcTable[DevInfo->bHCNumber - 1]->wBusDevFuncNum;

    // Update NVRAM variable for Setup
    UpdateMassDevicesForSetup();

    USB_DEBUG(DEBUG_LEVEL_3, "InstallUSBMass(%x): BS %d, MaxLBA %x, LA: %x %s\n",
        DevInfo, DevInfo->wBlockSize, DevInfo->dMaxLba,
        MassDev->LogicalAddress, &DevInfo->DevNameString);

    Status = gBS->InstallProtocolInterface(
                    &MassDev->Handle,
                    &gEfiBlockIoProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &MassDev->BlockIoProtocol
                    );
    USB_DEBUG(DEBUG_LEVEL_3, "Install BlockIO on %x status = %r\n", Controller, Status);
    ASSERT_EFI_ERROR(Status);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   UninstallUSBMass
//
// Description: Removes BlkIo protocol from USB Mass Storage device
//
// Input:       DevInfo - pointer to a USB device structure
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UninstallUSBMass(USB_MASS_DEV *MassDev)
{
    EFI_STATUS      Status;
	DEV_INFO		*DevInfo = MassDev->DevInfo;
    HC_STRUC*       HcData;
    UINT8           UsbStatus;

    HcData = gUsbData->HcTable[DevInfo->bHCNumber - 1];
 	UsbStatus = UsbDevDriverDisconnect(HcData, DevInfo);
	ASSERT(UsbStatus == USB_SUCCESS);

    USB_DEBUG(DEBUG_LEVEL_3, "Uninstall mass storage device  %x: ", MassDev->Handle);
    Status  = gBS->UninstallMultipleProtocolInterfaces(
                MassDev->Handle,
                &gEfiBlockIoProtocolGuid,
                &MassDev->BlockIoProtocol,
                NULL);
    ASSERT_EFI_ERROR(Status);

    USB_DEBUG(DEBUG_LEVEL_3, "%r\n", Status);
    if(!EFI_ERROR(Status)){
        gBS->FreePool(MassDev->Media);
        gBS->FreePool(MassDev);
        DevInfo->MassDev = NULL;
    }

    // Update NVRAM variable for Setup
    UpdateMassDevicesForSetup();

    return Status;
}



/************ BlockIO Protocol implementation routines******************/
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AmiUsbBlkIoReset
//
// Description: Reset the USB Logic Drive
//
// Input:       This: A pointer to the Block I/O protocol interface
//
//              ExtendedVerification: Indicate that the driver may perform
//              an exhaustive verification operation of the device during
//              reset
//
// Output:      EFI_SUCCESS: The USB Logic Drive is reset
//              EFI_DEVICE_ERROR: The Floppy Logic Drive is not functioning
//              correctly and can not be reset
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbBlkIoReset (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                ExtendedVerification
  )

{
    return  EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AmiUsbBlkIoFlushBlocks
//
// Description: Flush USB Mass Storage Device
//
// Input:       This: A pointer to the Block I/O protocol interface
//
// Output:      EFI_SUCCESS: The USB Logic Drive successfully flushed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbBlkIoFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
  )
{
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AmiUsbBlkIoReadWrite
//
// Description: This routine is invoked from AmiUsbBlkIoReadBlocks and
//              AmiUsbBlkIoWriteBlocks. See these for parameters reference.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbBlkIoReadWrite (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSize,
  IN OUT VOID                *Buffer,
  UINT8                     ReadWrite
  )
{
    USB_MASS_DEV            *MassDev;
    URP_STRUC               Parameters;
    EFI_STATUS              Status = EFI_SUCCESS;
    UINT32                  Buf;
    UINT32                  BytesToTransfer, BytesRemaining;
    UINT32                  CurrentLba;
    UINT16                  BlockSize;
    UINTN                   BufferAddress;
	UINT8					*DataBuffer;
	UINTN					Pages;

    MassDev   = (USB_MASS_DEV*)This;
    BlockSize = ((DEV_INFO*)(MassDev->DevInfo))->wBlockSize;

    //
    // Check if media id matches
    //
    if ( This->Media->MediaId != MediaId ) {
        return EFI_MEDIA_CHANGED;
    }

    if (BufferSize == 0) return EFI_SUCCESS;
    if (Buffer == NULL) return EFI_INVALID_PARAMETER;

    //
    // If IoAlign values is 0 or 1, means that the buffer can be placed 
    // anywhere in memory or else IoAlign value should be power of 2. To be
    // properly aligned the buffer address should be divisible by IoAlign  
    // with no remainder. 
    // 
    BufferAddress = (UINTN)Buffer;
    if((This->Media->IoAlign > 1 ) && (BufferAddress % This->Media->IoAlign)) {
        return EFI_INVALID_PARAMETER;
    }
    

    //
    // Get media status
    //
    This->Media->MediaPresent = TRUE;   // Initialize, to be updated if no media

    Parameters.bFuncNumber = USB_API_MASS_DEVICE_REQUEST;
    Parameters.bSubFunc = USB_MASSAPI_GET_MEDIA_STATUS;
    Parameters.ApiData.MassGetDevSts.bDevAddr = (UINT8)MassDev->LogicalAddress;
    Parameters.ApiData.MassGetDevSts.bDeviceStatus = 0;

 	InvokeUsbApi(&Parameters);

    if (!(Parameters.ApiData.MassGetDevSts.bDeviceStatus & USB_MASS_MEDIA_PRESENT)) {
        This->Media->MediaPresent = FALSE;
        return EFI_NO_MEDIA;
    }

    if (Parameters.ApiData.MassGetDevSts.bDeviceStatus & USB_MASS_MEDIA_CHANGED) {
        This->Media->MediaId++;
        This->Media->ReadOnly = FALSE;
    }

    if (MediaId != This->Media->MediaId) return EFI_MEDIA_CHANGED;
    //
    // Check Parameter to comply with EFI 1.1 Spec
    //
    if (Lba > This->Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if ((Lba + (BufferSize / BlockSize) - 1) > This->Media->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize % BlockSize != 0) {
        return EFI_BAD_BUFFER_SIZE;
    }

	DataBuffer = (UINT8*)(UINTN)Buffer;
	if (Shr64((UINTN)Buffer, 32)) {
		Pages = EFI_SIZE_TO_PAGES(BufferSize);
		DataBuffer = (UINT8*)0xFFFFFFFF;
		Status = gBS->AllocatePages(AllocateMaxAddress, EfiBootServicesData,
                		Pages, (EFI_PHYSICAL_ADDRESS*)&DataBuffer);
		ASSERT_EFI_ERROR(Status);
		if (EFI_ERROR(Status)) {
			return Status;
		}

		if (ReadWrite == WRITE) {
			gBS->CopyMem(DataBuffer, Buffer, BufferSize);
		}
	}

    BytesRemaining = (UINT32)BufferSize;
    CurrentLba = (UINT32)Lba;
    Buf = (UINT32)(UINTN)DataBuffer;
    while (BytesRemaining) {
        BytesToTransfer = (BytesRemaining > 0x10000)? 0x10000 : BytesRemaining;
        //
        // Prepare URP_STRUC with USB_MassRead attributes
        //
        Parameters.bFuncNumber = USB_API_MASS_DEVICE_REQUEST;
        Parameters.bSubFunc = (ReadWrite == READ)? USB_MASSAPI_READ_DEVICE : USB_MASSAPI_WRITE_DEVICE;
        Parameters.ApiData.MassRead.bDevAddr = (UINT8)MassDev->LogicalAddress;//MassDev->DevInfo->bDeviceAddress;
        Parameters.ApiData.MassRead.dStartLBA = CurrentLba;
        Parameters.ApiData.MassRead.wNumBlks = (UINT16)(BytesToTransfer/((DEV_INFO*)MassDev->DevInfo)->wBlockSize);
        Parameters.ApiData.MassRead.wPreSkipSize = 0;
        Parameters.ApiData.MassRead.wPostSkipSize = 0;
        Parameters.ApiData.MassRead.fpBufferPtr = Buf;
        /*
        if (ReadWrite == READ) {
            USB_DEBUG(DEBUG_LEVEL_3, "Reading...%x bytes, Lba %x ", BytesToTransfer, CurrentLba);
        } else {
            USB_DEBUG(DEBUG_LEVEL_3, "Writng...%x bytes, Lba %x ", BytesToTransfer, CurrentLba);
        }
        */
		InvokeUsbApi(&Parameters);
		
        switch (Parameters.bRetValue) {
            case USB_ATA_NO_MEDIA_ERR:
                    Status = EFI_NO_MEDIA;  // No media in drive
                    This->Media->MediaPresent = FALSE;
                    break;
            case USB_ATA_WRITE_PROTECT_ERR:
                    Status = (ReadWrite == READ)? EFI_SUCCESS : EFI_WRITE_PROTECTED;
                    if (Status == EFI_WRITE_PROTECTED)  
                        This->Media->ReadOnly = TRUE;
                    break;
            case USB_ATA_TIME_OUT_ERR:          // 0x080 Command timed out error
            case USB_ATA_DRIVE_NOT_READY_ERR:   // 0x0AA Drive not ready error
            case USB_ATA_DATA_CORRECTED_ERR:    // 0x011 Data corrected error
            case USB_ATA_PARAMETER_FAILED:      // 0x007 Bad parameter error
            case USB_ATA_MARK_NOT_FOUND_ERR:    // 0x002 Address mark not found error
            case USB_ATA_READ_ERR:              // 0x004 Read error
            case USB_ATA_UNCORRECTABLE_ERR:     // 0x010 Uncorrectable data error
            case USB_ATA_BAD_SECTOR_ERR:        // 0x00A Bad sector error
            case USB_ATA_GENERAL_FAILURE:       // 0x020 Controller general failure
                    Status = EFI_DEVICE_ERROR;
                    break;
            default:
                    Status = EFI_SUCCESS;
        }
        //  USB_DEBUG(DEBUG_LEVEL_3, "Status= %r\n", Status);
        if (EFI_ERROR(Status)) break;
        BytesRemaining = BytesRemaining - BytesToTransfer;
        Buf = Buf + BytesToTransfer;
        CurrentLba = CurrentLba + (UINT32)BytesToTransfer/((DEV_INFO*)(MassDev->DevInfo))->wBlockSize;
    }

	if (DataBuffer != Buffer) {
		if (ReadWrite == READ) {
			gBS->CopyMem(Buffer, DataBuffer, BufferSize - BytesRemaining);
		}
		gBS->FreePages((EFI_PHYSICAL_ADDRESS)DataBuffer, Pages);
	}

    return  Status;
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AmiUsbBlkIoReadBlocks
//
// Description: Read the requested number of blocks from the device
//
// Input:       This EFI_BLOCK_IO *: A pointer to the Block I/O protocol
//                                   interface
//              MediaId UINT32: The media id that the read request is for
//              LBA EFI_LBA:    The starting logic block address to read from
//                              on the device
//              BufferSize UINTN:   The size of the Buffer in bytes
//              Buffer VOID *:  A pointer to the destination buffer for the data
//
//
// Output:      EFI_SUCCESS:     The data was read correctly from the device
//              EFI_DEVICE_ERROR:The device reported an error while attempting
//                                  to perform the read operation
//              EFI_NO_MEDIA:    There is no media in the device
//              EFI_MEDIA_CHANGED:   The MediaId is not for the current media
//              EFI_BAD_BUFFER_SIZE: The BufferSize parameter is not a multiple
//                              of the intrinsic block size of the device
//              EFI_INVALID_PARAMETER:The read request contains LBAs that are
//                          not valid, or the buffer is not on proper alignment
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbBlkIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSize,
  OUT VOID                   *Buffer
)

{
    return AmiUsbBlkIoReadWrite(This, MediaId, Lba, BufferSize, Buffer, READ);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AmiUsbBlkIoWriteBlocks
//
// Description: Write a specified number of blocks to the device
//
// Input:       This EFI_BLOCK_IO *: A pointer to the Block I/O protocol
//              interface
//              MediaId UINT32: The media id that the write request is for
//              LBA EFI_LBA:    The starting logic block address to written
//              BufferSize UINTN:   The size of the Buffer in bytes
//              Buffer VOID *:  A pointer to the destination buffer for the data
//
//
// Output:      EFI_SUCCESS:     The data were written correctly to the device
//              EFI_WRITE_PROTECTED: The device can not be written to
//              EFI_NO_MEDIA:    There is no media in the device
//              EFI_MEDIA_CHANGED:   The MediaId is not for the current media
//              EFI_DEVICE_ERROR:  The device reported an error while attempting
//                                  to perform the write operation
//              EFI_BAD_BUFFER_SIZE: The BufferSize parameter is not a multiple
//                                  of the intrinsic block size of the device
//              EFI_INVALID_PARAMETER:The read request contains LBAs that are
//                          not valid, or the buffer is not on proper alignment
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbBlkIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  )
{
    return AmiUsbBlkIoReadWrite(This, MediaId, Lba, BufferSize, Buffer, WRITE);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbMassSupported
//
// Description: Verifies if usb mouse support can be installed on a device
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

static
EFI_STATUS
UsbMassSupported (
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    EFI_DEVICE_PATH_PROTOCOL    *Dp)
{
    EFI_USB_INTERFACE_DESCRIPTOR Desc;
    EFI_STATUS Status;
    EFI_USB_IO_PROTOCOL *UsbIo;
										//(EIP99882+)>
    if (!gUsbData->UsbSetupData.UsbMassDriverSupport) {
        return EFI_UNSUPPORTED;
    }
										//<(EIP99882+)
    Status = gBS->OpenProtocol ( Controller,  &gEfiUsbIoProtocolGuid,
        &UsbIo, This->DriverBindingHandle,
        Controller, EFI_OPEN_PROTOCOL_BY_DRIVER );
    if( EFI_ERROR(Status))
        return Status;

    VERIFY_EFI_ERROR(
        gBS->CloseProtocol (
        Controller, &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle, Controller));

    Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &Desc  );
    if(EFI_ERROR(Status))
        return EFI_UNSUPPORTED;

    if ( Desc.InterfaceClass == BASE_CLASS_MASS_STORAGE &&
        (
        Desc.InterfaceProtocol == PROTOCOL_CBI ||
        Desc.InterfaceProtocol == PROTOCOL_CBI_NO_INT   ||
        Desc.InterfaceProtocol == PROTOCOL_BOT ))
    {
        return EFI_SUCCESS;
    } else {
        return EFI_UNSUPPORTED;
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbMassStart
//
// Description: Starts USB mass storage device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

static
EFI_STATUS
UsbMassStart(
    EFI_DRIVER_BINDING_PROTOCOL  *This,
    EFI_HANDLE Controller,
    EFI_DEVICE_PATH_PROTOCOL *Dp
)
{
    EFI_STATUS              Status;
    EFI_USB_IO_PROTOCOL     *UsbIo;
    DEV_INFO                *DevInfo;

    USB_DEBUG(DEBUG_LEVEL_3,
        "USB: UsbMassStart: starting...\n");
    //
    // Open Protocols
    //
    Status = gBS->OpenProtocol ( Controller,  &gEfiUsbIoProtocolGuid,
        &UsbIo, This->DriverBindingHandle,
        Controller, EFI_OPEN_PROTOCOL_BY_DRIVER );
    if( EFI_ERROR(Status))
        return Status;

    {
        USBDEV_T* Dev = UsbIo2Dev(UsbIo);
        HC_STRUC* HcData;
        UINT8 UsbStatus;

        ASSERT(Dev);
        if (Dev == NULL) return EFI_DEVICE_ERROR;

        DevInfo = Dev->dev_info;
        if (DevInfo->bLUN) {
            USB_DEBUG(DEBUG_LEVEL_3, "USB: Skiping LUN %d\n", DevInfo->bLUN);
        } else  {
            HcData = gUsbData->HcTable[Dev->dev_info->bHCNumber - 1];
            UsbStatus = UsbSmiReConfigDevice(HcData, Dev->dev_info);
            if ((UsbStatus != USB_SUCCESS) || !(DevInfo->bFlag & DEV_INFO_DEV_PRESENT)) {
                USB_DEBUG(DEBUG_LEVEL_3, 
                    "USB: UsbMassStart: failed to Reconfigure: %d\n", UsbStatus );
                gBS->CloseProtocol (
                    Controller, &gEfiUsbIoProtocolGuid,
                    This->DriverBindingHandle, Controller);
                return EFI_DEVICE_ERROR;
            }
        } //End Reconfigure
    }

    InstallUSBMass(Controller, DevInfo);

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbMassStop
//
// Description: Stops USB mass storage device and removes BlkIo
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbMassStop(
    EFI_DRIVER_BINDING_PROTOCOL *Binding,
    EFI_HANDLE Controller,
    UINTN NumberOfChildren,
    EFI_HANDLE *Children
)
{
    EFI_STATUS Status;
    EFI_BLOCK_IO_PROTOCOL   *BlockIo;

    VERIFY_EFI_ERROR(
        Status = gBS->OpenProtocol ( Controller,  &gEfiBlockIoProtocolGuid,
        &BlockIo, Binding->DriverBindingHandle,
        Controller, EFI_OPEN_PROTOCOL_GET_PROTOCOL ));
    if (EFI_ERROR(Status))
        return Status;

    Status = UninstallUSBMass((USB_MASS_DEV*)BlockIo);

    VERIFY_EFI_ERROR(
        gBS->CloseProtocol (
        Controller, &gEfiUsbIoProtocolGuid,
        Binding->DriverBindingHandle, Controller));

    return Status;
}


CHAR16*
UsbMassGetControllerName(
    EFI_HANDLE Controller,
    EFI_HANDLE Child
)
{
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbMassInit
//
// Description: USB Mass storage driver entry point
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbMassInit(
    EFI_HANDLE  ImageHandle,
    EFI_HANDLE  ServiceHandle
)
{
                                        //(EIP59272)>
    static NAME_SERVICE_T Names;
    static EFI_DRIVER_BINDING_PROTOCOL Binding = {
        UsbMassSupported,
        UsbMassStart,
        UsbMassStop,
        USBMASS_DRIVER_VERSION,
        NULL,
        NULL };

    Binding.DriverBindingHandle = ServiceHandle;
    Binding.ImageHandle = ImageHandle;

    return gBS->InstallMultipleProtocolInterfaces(
        &Binding.DriverBindingHandle,
        &gEfiDriverBindingProtocolGuid, &Binding,
        &gEfiComponentName2ProtocolGuid, InitNamesProtocol(&Names,
                L"USB Mass Storage driver", UsbMassGetControllerName),
        NULL);
                                        //<(EIP59272)
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
