//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioRecovery.c 1     7/18/12 4:49a Rajeshms $
//
// $Revision: 1 $
//
// $Date: 7/18/12 4:49a $
//
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SdioDriver/SdioRecovery.c $
// 
// 1     7/18/12 4:49a Rajeshms
// [TAG]  		EIP93345
// [Category]  	New Feature
// [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
// devices
// [Files]  		SdioRecovery.cif
// SdioRecovery.sdl
// SdioRecovery.mak
// SdioRecovery.c
// SdioFindRecoveryDevice.c
// SdioRecovery.h
// 
// 1     7/18/12 4:30a Rajeshms
// [TAG]  		EIP93345 
// [Category]  	New Feature
// [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
// devices
// [Files]  		Board\EM\SdioRecovery\SdioRecovery.cif
// Board\EM\SdioRecovery\SdioRecovery.sdl
// Board\EM\SdioRecovery\SdioRecovery.mak
// Board\EM\SdioRecovery\SdioRecovery.c
// Board\EM\SdioRecovery\SdioFindRecoveryDevice.c
// Board\EM\SdioRecovery\SdioRecovery.h
// 
//*************************************************************************
//<AMI_FHDR_START>
//
//  Name:           SdioRecoveryDevice.c
//
//  Description:    Installs EFI_PEI_RECOVERY_BLOCK_IO_PPI for SD/MMC
//                  device and this file contains the function to Read
//                  from the SD/MMC Device.
//
//<AMI_FHDR_END>
//*************************************************************************

//---------------------------------------------------------------------------
#include "SdioRecovery.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
EFI_PEI_SERVICES    **gPeiServices = NULL;
EFI_PEI_PCI_CFG2_PPI *gPciCfg = NULL;
EFI_PEI_STALL_PPI   *gStallPpi = NULL;

EFI_PEI_PPI_DESCRIPTOR   gSd_PpiDescriptor = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiVirtualBlockIoPpiGuid,
    NULL
};
//---------------------------------------------------------------------------

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: Sd_GetNumberOfBlockDevices
//
// Description:
//  It enumerates to find the presence of SD/MMC devices and returns the 
//  no. of devices found. 
//
// Input:
//  IN EFI_PEI_SERVICES              **PeiServices,
//  IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
//  OUT UINTN                        *NumberBlockDevices 
//
// Output:  
//  Status
//  number of SD/MMC devices found in *NumberBlockDevices.
//      
// Modified:
//
// Referrals:
//  EnumerateSdDevices
//
// Notes:
//  The enumeration will be done to find SD/MMC devices if not enumerated
//  already.
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS Sd_GetNumberOfBlockDevices(
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    OUT UINTN                        *NumberBlockDevices )
{
    SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev = NULL;
    EFI_STATUS                 Status;

    if ( This == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    Sd_BlkIoDev = (SDIO_RECOVERY_BLOCK_IO_DEV *)This;

    //
    // Enumerate to find the SD/MMC Devices.
    //
    if ( !Sd_BlkIoDev->HaveEnumeratedDevices ) {
        Status = EnumerateSdDevices( Sd_BlkIoDev );
        if ( EFI_ERROR( Status )) {
            return Status;
        }

        Sd_BlkIoDev->HaveEnumeratedDevices = TRUE;
    }

    //
    // Return the number of SD/MMC Devices found after enumeration.
    //
    *NumberBlockDevices = Sd_BlkIoDev->DeviceCount;

    return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: Sd_GetBlockDeviceMediaInfo
//
// Description:
//  It gets the Media Info(EFI_PEI_BLOCK_IO_MEDIA) data for the requested
//  (through DeviceIndex) SD/MMC Device.
//
// Input:
//  IN EFI_PEI_SERVICES              **PeiServices,
//  IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
//  IN UINTN                         DeviceIndex,
//  OUT EFI_PEI_BLOCK_IO_MEDIA       *MediaInfo
//
// Output:
//  Status
//  Media Info of the requested SD/MMC device in *MediaInfo.
//
// Modified:
//
// Referrals:
//  EnumerateSdDevices
//
// Notes:
//  The enumeration will be done to find SD/MMC devices if not enumerated
//  already.
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS Sd_GetBlockDeviceMediaInfo(
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    IN UINTN                         DeviceIndex,
    OUT EFI_PEI_BLOCK_IO_MEDIA       *MediaInfo )
{
    SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev = NULL;
    EFI_STATUS              Status;

    if ((This == NULL) || (MediaInfo == NULL)) {
        return EFI_INVALID_PARAMETER;
    }

    Sd_BlkIoDev = (SDIO_RECOVERY_BLOCK_IO_DEV *)This;

    //
    // Enumerate to find the SD/MMC Devices if not enumerated.
    //
    if ( !Sd_BlkIoDev->HaveEnumeratedDevices ) {
        Status = EnumerateSdDevices( Sd_BlkIoDev );
        if ( EFI_ERROR( Status )) {
            return Status;
        }

        Sd_BlkIoDev->HaveEnumeratedDevices = TRUE;
    }

    if ( DeviceIndex > (Sd_BlkIoDev->DeviceCount - 1)) {
        return EFI_INVALID_PARAMETER;
    }

    if ( !Sd_BlkIoDev->DeviceInfo[DeviceIndex]->LookedForMedia ) {
        Sd_BlkIoDev->DeviceInfo[DeviceIndex]->LookedForMedia = TRUE;
    }

    //
    // Return the Media info of the requested SD/MMC Device.
    //
    *MediaInfo = Sd_BlkIoDev->DeviceInfo[DeviceIndex]->MediaInfo;

    return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: Sd_ReadBlocks
//
// Description:
//  Reads from the requested(through DeviceIndex) SD/MMC Device. No. of 
//  blocks to be read is calculated using BufferSize and BlockSize.
//
// Input:
//  IN EFI_PEI_SERVICES              **PeiServices,
//  IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
//  IN UINTN                         DeviceIndex,
//  IN EFI_PEI_LBA                   StartLba,
//  IN UINTN                         BufferSize,
//  OUT VOID                         *Buffer
//
// Output:
//  Data filled in *Buffer
//  Status
//
// Modified:
//
// Referrals:
//  EnumerateSdDevices
//
// Notes:
//  The enumeration will be done to find SD/MMC devices if not enumerated
//  already.
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS Sd_ReadBlocks(
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    IN UINTN                         DeviceIndex,
    IN EFI_PEI_LBA                   StartLba,
    IN UINTN                         BufferSize,
    OUT VOID                         *Buffer )
{
    EFI_PEI_BLOCK_IO_MEDIA  MediaInfo;
    EFI_STATUS              Status;
    UINTN                   NumberOfBlocks;
    UINT8                   Port = 0;
    SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev  = NULL;

    if ( This == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    Sd_BlkIoDev = (SDIO_RECOVERY_BLOCK_IO_DEV *)This;

    if ( Buffer == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( BufferSize == 0 ) {
        return EFI_SUCCESS;
    }

    if ( !Sd_BlkIoDev->HaveEnumeratedDevices ) {
        Status = EnumerateSdDevices( Sd_BlkIoDev );
        if ( EFI_ERROR( Status )) {
            return Status;
        }
        Sd_BlkIoDev->HaveEnumeratedDevices = TRUE;
    }

    if ( !Sd_BlkIoDev->DeviceInfo[DeviceIndex]->LookedForMedia ) {
        Status = Sd_GetBlockDeviceMediaInfo(
                                    PeiServices,
                                    This,
                                    DeviceIndex,
                                    &MediaInfo
                                    );

        if ( Status != EFI_SUCCESS ) {
            return Status;
        }
    }  else {
        MediaInfo = Sd_BlkIoDev->DeviceInfo[DeviceIndex]->MediaInfo;
    }

    if ( !MediaInfo.MediaPresent ) {
        return EFI_NO_MEDIA;
    } 
    
    //
    // Check whether the block size is multiple of MediaInfo.BlockSize
    //
    NumberOfBlocks = BufferSize % MediaInfo.BlockSize;
    if (NumberOfBlocks) {
        return EFI_BAD_BUFFER_SIZE;
    }

    //
    // Check whether we are accessing the within the LastBlock of Device.  
    //
    if ( StartLba > MediaInfo.LastBlock ) {
        return EFI_INVALID_PARAMETER;
    }

    if ((StartLba + NumberOfBlocks) > (MediaInfo.LastBlock + 1)) {
        return EFI_INVALID_PARAMETER;
    }

    NumberOfBlocks = BufferSize / MediaInfo.BlockSize;

    //
    // Read from SD/MMC Card.
    //
    Status = SDIOAPI_ReadCard_Controller(
                        &Sd_BlkIoDev->DeviceInfo[DeviceIndex]->SdioDeviceInfo,
                        Port,
                        StartLba,
                        NumberOfBlocks,
                        Buffer
                        );
    return Status;
}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: SdioRecoveryEntryPoint
//
// Description:
//  Entry point for SDIO Recovery.Installs EFI_PEI_RECOVERY_BLOCK_IO_PPI
//
// Input:
//  IN EFI_FFS_FILE_HEADER       *FfsHeader - Pointer to FfsHeader 
//  IN EFI_PEI_SERVICES          **PeiServices - pointer to PEI services
//
// Output:
//  EFI_STATUS   
//      Status
//
// Modified:
//  gStallPpi, gPeiServices, gPciCfg
//
// Referrals:
//  gSd_PpiDescriptor, gPeiServices, gPciCfg
//
// Notes:
//
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>
EFI_STATUS
SdioRecoveryEntryPoint (
    IN EFI_FFS_FILE_HEADER       *FfsHeader,
    IN EFI_PEI_SERVICES          **PeiServices
)
{

    EFI_STATUS  Status;
    SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev = NULL;

    gPeiServices = PeiServices;

    gPciCfg = (*PeiServices)->PciCfg;

    Status = (**PeiServices).LocatePpi(
        PeiServices,
        &gEfiPeiStallPpiGuid,
        0,
        NULL,
        &gStallPpi
        );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    //
    // Allocate resource for private Structure SDIO_RECOVERY_BLOCK_IO_DEV.
    //
    Status = (**PeiServices).AllocatePool( PeiServices,
                                           sizeof(SDIO_RECOVERY_BLOCK_IO_DEV),
                                           &Sd_BlkIoDev );
    if ( EFI_ERROR( Status )) {
        return EFI_OUT_OF_RESOURCES;
    }

    Sd_BlkIoDev->HaveEnumeratedDevices = FALSE;
    Sd_BlkIoDev->DeviceCount           = 0;

    Sd_BlkIoDev->RecoveryBlkIo.GetNumberOfBlockDevices = Sd_GetNumberOfBlockDevices;
    Sd_BlkIoDev->RecoveryBlkIo.GetBlockDeviceMediaInfo = Sd_GetBlockDeviceMediaInfo;
    Sd_BlkIoDev->RecoveryBlkIo.ReadBlocks              = Sd_ReadBlocks;
    gSd_PpiDescriptor.Ppi = &Sd_BlkIoDev->RecoveryBlkIo;

    //
    // Install EFI_PEI_RECOVERY_BLOCK_IO_PPI.
    //
    Status = (**PeiServices).InstallPpi( PeiServices, &gSd_PpiDescriptor );

    return Status;

}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
