//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioInt13.c 2     4/19/11 6:58a Lavanyap $
//
// $Revision: 2 $
//
// $Date: 4/19/11 6:58a $
//**********************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SdioDriver.c
//
// Description: Sdio driver for Legacy mode
//<AMI_FHDR_END>
//**********************************************************************

#include "AmiDxeLib.h"
#include <Protocol\SdioInt13Protocol.h>
#include <Protocol\ComponentName.h>
#include <Protocol\LegacyBiosExt.h>
#include <Protocol\LegacyBios.h>
#include <Protocol\PciIo.h>
#include <protocol\BlockIo.h>
#include <Protocol\PDiskInfo.h>
#include <Protocol\SdioBus.h>
#include "SdioInt13.h"
#include "token.h"
#include "pci.h"

EFI_LEGACY_BIOS_EXT_PROTOCOL        *gBiosExtensions = NULL;
UINT13_DATA                         *gI13BinData = NULL;
EFI_SDIO_PROTOCOL                   gAmiSdio;
UINT8                               gBootOverrideDeviceIndx = 0;


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
// Procedure:   SdioInt13EntryPoint
//
// Description: SDIO INT13 driver entry point. Installs callback notification
//              on gEfiSdioProtocolGuid installation.
//
// Input:       IN EFI_HANDLE        ImageHandle,
//              IN EFI_SYSTEM_TABLE  *SystemTable
//
// Output:      EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
SdioInt13EntryPoint(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS  Status;
    EFI_HANDLE  SdioHandle=NULL;

    InitAmiLib(ImageHandle, SystemTable);

    Status = InitInt13RuntimeImage();
    if (EFI_ERROR(Status)) return Status;

    gAmiSdio.SdioInstallLegacyDevice = SdioInstallLegacyDevice;

    Status = pBS->InstallProtocolInterface (
                                    &SdioHandle,
                                    &gEfiSdioProtocolGuid,
                                    EFI_NATIVE_INTERFACE,
                                    &gAmiSdio
                                    );

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
//  Name:           InitInt13RuntimeImage
//
//  Description:    Initialization of data structures and placement of runtime
//                  code of SDIO INT13
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InitInt13RuntimeImage()
{
    EFI_STATUS  Status;
    VOID        *Image;
    UINTN       ImageSize = 0;


    //
    // Get the SDIO INT13 runtime image
    //
    Status = pBS->LocateProtocol(
        &gEfiLegacyBiosExtProtocolGuid, NULL, &gBiosExtensions);
    if (EFI_ERROR(Status)) return Status;

    Status = gBiosExtensions->GetEmbeddedRom(
        CSM16_MODULEID, CSM16_VENDORID, CSM16_SDIO_RT_DID, &Image, &ImageSize);
    if (EFI_ERROR(Status)) return Status;

    //
    // Do the necessary RT data initialization here using Image before it is shadowed
    //..............................
    {
#pragma pack(push, 1)
        // Update SDIO SMI information
        typedef struct _SDIO_SMM_RTS {
            UINT8   MiscInfo;
            UINT16  SmmAttr;
            UINT32  SmmPort;
            UINT32  SmmData;
        } SDIO_SMM_RTS;

        static SDIO_SMM_RTS SdioSmmRt = {1, 0, SW_SMI_IO_ADDRESS, SDIO_SWSMI};

        *(SDIO_SMM_RTS*)((UINTN)Image + ((UINT13_DATA*)Image)->SdioSmmDataOffset) = SdioSmmRt;
#pragma pack(pop)
    }

    // Copy image to shadow E000/F000 area
    (UINTN)gI13BinData = gBiosExtensions->CopyLegacyTable(Image, (UINT16)ImageSize, 0x10, 2);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        CreateDeviceName
//
// Description: This function retrieves SDIO device name, copies it into
//              lower memory and returns a pointer to the string.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CreateDeviceName(
    UINT8   DevIndex,
    UINT8   *DevNameStringSrc,
    UINT16  *StringDestinationSegment,
    UINT16  *StringDestinationOffset,
    UINT16  *MfgStringDestinationSegment,
    UINT16  *MfgStringDestinationOffset
)
{
    UINT8 *DevName = (gI13BinData->SdioMassI13Dev)[DevIndex].DeviceNameString;
    UINT8 i;

    //
    // Copy the string, compact it on the way (no more that one ' ' in a row)
    //
    for (i=0; i<31, *DevNameStringSrc != 0; i++, DevNameStringSrc++)
    {
        if ((*DevNameStringSrc == 0x20) && (*(DevNameStringSrc-1) == 0x20)) continue;
        *DevName++ = *DevNameStringSrc;  // DevNameStringSrc incremented unconditionally
    }
    *DevName = 0;   // string terminator

    DevName = (gI13BinData->SdioMassI13Dev)[DevIndex].DeviceNameString;

    *StringDestinationSegment = (UINT16)(((UINTN)DevName & 0xf0000) >> 4);
    *StringDestinationOffset = (UINT16)((UINTN)DevName & 0xffff);

    *MfgStringDestinationSegment = (UINT16)(((UINTN)gI13BinData->MfgGenericName & 0xf0000) >> 4);
    *MfgStringDestinationOffset = (UINT16)((UINTN)gI13BinData->MfgGenericName & 0xffff);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        CreateBbsEntry
//
// Description: This function takes the device index within SDIOMASS_INT13_DEV
//              list and prepares BBS entry for this device.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CreateBbsEntry(
    UINT8           DevIndex,
    IN SDIO_MASS_DEV *SdioMassDevice,
    OUT BBS_TABLE   *BbsEntry
)
{
    EFI_STATUS  Status;
    UINT8       Handle;
    UINT8       DevAndSysType;
    UINT8       BaidDeviceType;
    BBS_STATUS_FLAGS    StatusFlags;

    ASSERT(DevIndex < SDIODEVS_MAX_ENTRIES);

    if (gBiosExtensions == NULL) return EFI_NOT_FOUND;

    pBS->SetMem(BbsEntry, sizeof(BBS_TABLE), 0);

    //
    // Get the HC PCI location
    //
    BbsEntry->Bus = (UINT32)(SdioMassDevice->PciBDF >> 8);
    BbsEntry->Device = (UINT32)((SdioMassDevice->PciBDF & 0xFF) >> 3);
    BbsEntry->Function = (UINT32)(SdioMassDevice->PciBDF & 7);

    //
    // Update class/subclass information
    //
    BbsEntry->Class = PCI_CL_SYSTEM_PERIPHERALS;
    BbsEntry->SubClass = PCI_CL_SYSTEM_PERIPHERALS_SCL_SD;

    StatusFlags.Enabled = 1; StatusFlags.MediaPresent = 1;
    BbsEntry->StatusFlags = StatusFlags;  // Enabled, Unknown media

    //
    // Copy the device name string into low memory at gLegacyMemoryAddress, and
    // update the string pointer in BBS table entry
    //
    Status = CreateDeviceName(
                DevIndex,
                SdioMassDevice->DevString,
                &BbsEntry->DescStringSegment,
                &BbsEntry->DescStringOffset,
                &BbsEntry->MfgStringSegment,
                &BbsEntry->MfgStringOffset
    );
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return Status;

    DevAndSysType = (SYSTYPE_ATA << 4)+DEVTYPE_SYS;
    Handle = (UINT8)SdioMassDevice->LogicalAddress;

    switch (SdioMassDevice->StorageType) {

        case SDIO_MASS_DEV_HDD:
            BbsEntry->DeviceType = BBS_HARDDISK;
            BaidDeviceType = BAID_TYPE_RMD_HDD;
            Handle |= 0x80;
            BbsEntry->BootHandlerSegment = (UINT16)((UINTN)gI13BinData >> 4);
            BbsEntry->BootHandlerOffset = gI13BinData->BcvOffset + DevIndex*4;
            break;
        case SDIO_MASS_DEV_ARMD:
            BbsEntry->DeviceType = BBS_FLOPPY;
            BaidDeviceType = BAID_TYPE_RMD_FDD;
            BbsEntry->BootHandlerSegment = (UINT16)((UINTN)gI13BinData >> 4);
            BbsEntry->BootHandlerOffset = gI13BinData->BcvOffset + DevIndex*4;
            break;
        default:
            BbsEntry->DeviceType = BBS_UNKNOWN;
    }

    BbsEntry->InitPerReserved = ((UINT32)BaidDeviceType<<24)
                        +((UINT32)Handle<<8)
                        +(UINT32)DevAndSysType;

    *(UINTN*)(&BbsEntry->IBV1) = (UINTN)SdioMassDevice->Handle;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  SdioInstallLegacyDevice
//
// Description: This function installs SDIO INT13 device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
SdioInstallLegacyDevice (
    SDIO_MASS_DEV    *SdioMassDevice
)
{
    BBS_TABLE   BbsEntry;
    EFI_STATUS  Status;
    UINT8       EntryNumber = 0xff;
    UINT8       Index;
    SDIO_DEV_INFO    *Device;

    TRACE((-1, "Installing SDIO INT13 device %x\n", SdioMassDevice));

    //
    // See if device is already in the list, if yes - return error.
    //
    for (Index=0; Index<SDIODEVS_MAX_ENTRIES; Index++) {
        if ((gI13BinData->SdioMassI13Dev)[Index].Handle == (UINT8)SdioMassDevice->LogicalAddress) {
            ASSERT(FALSE);  // ERROR: Device already exists
            return EFI_INVALID_PARAMETER;
        }
    }
    //
    // Look for an empty slot in BcvLookupTable
    //
    for (Index=0; Index<SDIODEVS_MAX_ENTRIES; Index++) {
        if  ((gI13BinData->SdioMassI13Dev)[Index].Handle == 0) break;
    }
    ASSERT(Index<SDIODEVS_MAX_ENTRIES);

    if (Index==SDIODEVS_MAX_ENTRIES) {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = gBiosExtensions->UnlockShadow(0, 0, 0, 0);
    ASSERT_EFI_ERROR(Status);

    Status = CreateBbsEntry(Index, SdioMassDevice, &BbsEntry);
    ASSERT_EFI_ERROR(Status);

    Status = gBiosExtensions->InsertBbsEntryAt(gBiosExtensions,
                                                &BbsEntry,
                                                &EntryNumber);
    ASSERT_EFI_ERROR(Status);

    //
    // Entry has been successfully added, update the lookup table
    //
    (gI13BinData->SdioMassI13Dev)[Index].Handle = (UINT8)SdioMassDevice->LogicalAddress;
    (gI13BinData->SdioMassI13Dev)[Index].BbsEntryNo = EntryNumber;
    (gI13BinData->SdioMassI13Dev)[Index].DevBaidType = (UINT8)(BbsEntry.InitPerReserved>>24);

    //
    // Update device geometry related information
    //
    Device = (SDIO_DEV_INFO*)SdioMassDevice->DevInfo;
    (gI13BinData->SdioMassI13Dev)[Index].NumHeads = Device->bNonLBAHeads;
    (gI13BinData->SdioMassI13Dev)[Index].LBANumHeads = Device->bHeads;
    (gI13BinData->SdioMassI13Dev)[Index].NumCylinders = Device->wNonLBACylinders;
    (gI13BinData->SdioMassI13Dev)[Index].LBANumCyls = Device->wCylinders;
    (gI13BinData->SdioMassI13Dev)[Index].NumSectors = Device->bNonLBASectors;
    (gI13BinData->SdioMassI13Dev)[Index].LBANumSectors = Device->bSectors;
    (gI13BinData->SdioMassI13Dev)[Index].BytesPerSector = Device->wBlockSize;
//    (gI13BinData->SdioMassI13Dev)[Index].MediaType = Device->bMediaType;
    (gI13BinData->SdioMassI13Dev)[Index].LastLBA = Device->dMaxLba;

    Status = gBiosExtensions->LockShadow(0, 0);
    ASSERT_EFI_ERROR(Status);

    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
