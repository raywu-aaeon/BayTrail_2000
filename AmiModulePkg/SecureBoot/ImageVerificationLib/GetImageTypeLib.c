//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
/** @file

  Auxiliary function for image verification services for secure boot
  service in UEFI 2.3.1+
**/

#include <Token.h>
#include "DxeImageVerificationLib.h"
#include <AmiDxeLib.h>
#include <Protocol/PciIo.h>

#include <Protocol/FirmwareVolume.h>
#include <Protocol/DevicePath.h>
#include <Protocol/BlockIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/SecurityManagementLib.h>

#define AMI_MEDIA_DEVICE_PATH_GUID \
    { 0x5023b95c, 0xdb26, 0x429b, 0xa6, 0x48, 0xbd, 0x47, 0x66, 0x4c, 0x80, 0x12 }

static EFI_GUID AmiMediaDevicePathGuid = AMI_MEDIA_DEVICE_PATH_GUID;

static UINTN                               mFvHandlesCount = 0;
static EFI_HANDLE                          *mFvHandles     = NULL;
static BOOLEAN                             gImageLoadingAfterBDS = FALSE;


/**
  Get the image type.

  @param[in]  File         This is a pointer to the device path of the file that is
                           being dispatched. This will optionally be used for logging.
  @param[in]  FileBuffer   A pointer to the buffer with the UEFI file image
  @param[in]  FileSize     The size of File buffer.
  @param[in]  BootPolicy   A boot policy that was used to call LoadImage() UEFI service.

  @return UINT32           Image Type

**/
UINT32
AmiGetImageType (
  IN  CONST EFI_DEVICE_PATH_PROTOCOL   *File,
  IN  VOID                             *FileBuffer,
  IN  UINTN                            FileSize,
  IN BOOLEAN                           BootPolicy
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        DeviceHandle; 
  EFI_DEVICE_PATH_PROTOCOL          *TempDevicePath;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  UINT8                             nDevicePathSubType;
  EFI_PCI_IO_PROTOCOL               *PciIoInterface = NULL; 
  UINT64                            AttributesResult;
  UINTN                             Index;
  BOOLEAN                           IsFv = FALSE;
  
  // Unknown device path: image security policy is applied to the image with the least trusted origin.
  if (File == NULL) {
   return IMAGE_UNKNOWN;
  }

  //
  // Check if File is just a Firmware Volume.
  //
  TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)File;
  while (!IsDevicePathEndType(TempDevicePath)){
      nDevicePathSubType = DevicePathSubType (TempDevicePath);
      TRACE((TRACE_ALWAYS,"Device PathType %x, SubType %x\n", DevicePathType(TempDevicePath), nDevicePathSubType ));
    //
    // EFI Specification extension on Media Device Path. MEDIA_FW_VOL_FILEPATH_DEVICE_PATH is adopted by UEFI later and added in UEFI2.10. 
    // In EdkCompatibility Package, we only support MEDIA_FW_VOL_FILEPATH_DEVICE_PATH that complies with EFI 1.10 and UEFI 2.10.
    //
    if( (DevicePathType(TempDevicePath) == MEDIA_DEVICE_PATH && 
         ((nDevicePathSubType == MEDIA_FV_FILEPATH_DP || nDevicePathSubType == MEDIA_FV_DP)
         // Case for embedded FV application such as Shell(check GUID. BootOptions.h)
//*dbg */          ||
//*dbg */          (nDevicePathSubType == MEDIA_VENDOR_DP && 
//*dbg */             !guidcmp(&((VENDOR_DEVICE_PATH*)TempDevicePath)->Guid, &AmiMediaDevicePathGuid)) 
         )) ||
        (DevicePathType(TempDevicePath) == HARDWARE_DEVICE_PATH && 
          nDevicePathSubType == HW_MEMMAP_DP)
    ){
        IsFv = TRUE;
    } else {
        IsFv = FALSE;
        break;
    }
    TempDevicePath = NextDevicePathNode(TempDevicePath);
  }
  //
  // Check to see if a File or a FV is from internal Firmware Volume.
  //
  if(IsFv) {

    DeviceHandle   = NULL;
    TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)File;
    Status = pBS->LocateDevicePath (
                  &gEfiFirmwareVolume2ProtocolGuid,
                  &TempDevicePath,
                  &DeviceHandle
                  );

    if (!EFI_ERROR (Status)) {
        if(!gImageLoadingAfterBDS)
          return IMAGE_FROM_FV;

        Status = pBS->OpenProtocol (
                        DeviceHandle,
                        &gEfiFirmwareVolume2ProtocolGuid,
                        NULL,
                        NULL,
                        NULL,
                        EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                        );
        if (!EFI_ERROR (Status)) {
            for(Index=0; Index < mFvHandlesCount; Index++){
                if(DeviceHandle == mFvHandles[Index] )
                    return IMAGE_FROM_FV;
            }
        }
    }
    return IMAGE_UNKNOWN;
  }
  //
  // Next check to see if File is from a Block I/O device
  // Must be a Block I/O device since we reached here after Int FV path check
  //
  DeviceHandle   = NULL;
  TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)File;
  Status = pBS->LocateDevicePath (
                  &gEfiBlockIoProtocolGuid,
                  &TempDevicePath,
                  &DeviceHandle
                  );
  if (!EFI_ERROR (Status)) {
    BlockIo = NULL;
    Status = pBS->OpenProtocol (
                    DeviceHandle,
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlockIo,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status) && BlockIo != NULL) {
      if (BlockIo->Media != NULL) {
        if (BlockIo->Media->RemovableMedia) {
          //
          // Block I/O is present and specifies the media is removable
          //
          return IMAGE_FROM_REMOVABLE_MEDIA;
        } else {
          //
          // Block I/O is present and specifies the media is not removable
          //
          return IMAGE_FROM_FIXED_MEDIA;
        }
      }
    }
  }
  //
  // File is not in a Firmware Volume or on a Block I/O device, so check to see if 
  // the device path supports the Simple File System Protocol.
  //
  DeviceHandle   = NULL;
  TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)File;
  Status = pBS->LocateDevicePath (
                  &gEfiSimpleFileSystemProtocolGuid,
                  &TempDevicePath,
                  &DeviceHandle
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Simple File System is present without Block I/O, so assume media is fixed.
    //
    return IMAGE_FROM_FIXED_MEDIA;
  }

  // Return IMAGE_FROM_FV if the EFI_PCI_IO_PROTOCOL installed
  //on the same handle as the Device Path has the attribute EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM
  DeviceHandle = NULL;
  //
  // Check if an instance of the EFI_PCI_IO_PROTOCOL is installed on the same handle
  // as the Device Path.  If an instance is found WorkAroundDevHandle contains the
  // handle for the Device Path and EFI_PCI_IO_PROTOCOL instance.
  //
  Status = pBS->LocateDevicePath(
                    &gEfiPciIoProtocolGuid,
                    &File,
                    &DeviceHandle
  );
  if(!(EFI_ERROR(Status)))
  {   
      Status = pBS->HandleProtocol(
                        DeviceHandle,
                        &gEfiPciIoProtocolGuid,
                        &PciIoInterface
      );
      
      if(!(EFI_ERROR(Status)))
      {
          //
          // Using the EFI_PCI_IO_PROTOCOL get the value of the PCI controller's
          // Embedded Rom attribute (EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM)
          //
          Status = PciIoInterface->Attributes(
                                    PciIoInterface,
                                    EfiPciIoAttributeOperationGet,
                                    0, //this parameter is ignored during Get operation
                                    &AttributesResult
          );
          //
          // Check if the PCI controller's EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM is set
          //          

//          if(!EFI_ERROR(Status) && 
//            (AttributesResult & (EFI_PCI_IO_ATTRIBUTE_EMBEDDED_DEVICE | EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM)))
          if(!EFI_ERROR(Status) && (AttributesResult & EFI_PCI_IO_ATTRIBUTE_EMBEDDED_ROM))
              return IMAGE_FROM_FV;
      }
  }
  //
  // File is not from an FV, Block I/O or Simple File System, so the only options
  // left are a PCI Option ROM and a Load File Protocol such as a PXE Boot from a NIC.  
  //
  TempDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)File;
  while (!IsDevicePathEndType (TempDevicePath)) {
    nDevicePathSubType = DevicePathSubType (TempDevicePath);
    
    switch (DevicePathType (TempDevicePath)) {

    case MEDIA_DEVICE_PATH:

      if (nDevicePathSubType == MEDIA_RELATIVE_OFFSET_RANGE_DP) {
        return IMAGE_FROM_OPTION_ROM;
      }
#if LOAD_UNSIGNED_EMBEDDED_SHELL == 1
      // Case for embedded FV application such as Shell(check GUID. BootOptions.h)
      if (nDevicePathSubType == MEDIA_VENDOR_DP && 
          !guidcmp(&((VENDOR_DEVICE_PATH*)TempDevicePath)->Guid, &AmiMediaDevicePathGuid)) {
            return IMAGE_FROM_FV;
      }
#endif
      break;

    case MESSAGING_DEVICE_PATH:

      if (nDevicePathSubType == MSG_MAC_ADDR_DP) {
        return IMAGE_FROM_REMOVABLE_MEDIA;
      } 
      break;
    }
    TempDevicePath = NextDevicePathNode (TempDevicePath);
  }
  
  return IMAGE_UNKNOWN; 
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetImageTypeLibConstructor
//
// Description: Library constructor. Called in Driver's entry point
//
// Input:
//  EFI_HANDLE          ImageHandle     Image handle.
//  EFI_SYSTEM_TABLE    *SystemTable    Pointer to the EFI system table.
//
// Output:
//  EFI_SUCCESS  
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
EFIAPI GetImageTypeLibConstructor(
    IN  EFI_HANDLE              ImageHandle,
    IN  EFI_SYSTEM_TABLE        *SystemTable
)
{
    EFI_STATUS Status;
    
    Status = pBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiFirmwareVolume2ProtocolGuid,
        NULL,
        &mFvHandlesCount,
        &mFvHandles
    );
    if(!EFI_ERROR(Status)){
        gImageLoadingAfterBDS = TRUE;
    }

    return EFI_SUCCESS;
}

#pragma warning (default : 4090)
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
