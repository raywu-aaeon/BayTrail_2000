//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

//**********************************************************************//
// $Header: /Alaska/SOURCE/Modules/SMBIOS/SmbiosGetFlashData/SmbiosGetFlashData.c 9     8/30/11 4:15p Davidd $
//
// $Revision: 9 $
//
// $Date: 8/30/11 4:15p $
//**********************************************************************//

#include <AmiDxeLib.h>
#include <Token.h>
#include <Protocol/SmbiosGetFlashDataProtocol.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Library/BaseMemoryLib.h>

#define Align4(Value) (((Value)+3) & ~3)
#define Align8(Value) (((Value)+7) & ~7)

#pragma pack(1)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  TABLE_INFO
//
// Description: DMI data record
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

#define DMIEDIT_WRITE_ONCE      0x01
#define DMIEDIT_DELETE_STRUC    0x02
#define DMIEDIT_ADD_STRUC       0x04
#define DMIEDIT_EXTENDED_HDR    0x80

typedef struct {
    UINT8   Type;
    UINT8   Offset;     // Structure field offset, or string number for Type 11 and 12
    UINT8   Reserved;   // Size of string including \0 or UUID (16)
    UINT8   Flags;      // Bit0 = Write Once
                        // Bit1 = Delete Structure
                        // Bit2 = Add structure
                        // Bit7 = Extended Header
    UINT8   HdrLength;
    UINT16  Size;
    UINT16  Handle;
} TABLE_INFO;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  ROM_INFO
//
// Description: DMI Data Table Header
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
	UINT32	Signature;
	UINT32 	Size;
} ROM_INFO;

#pragma pack()

EFI_GUID FlashDataFile  =  {0xfd44820b, 0xf1ab, 0x41c0, 0xae, 0x4e, 0x0c, 0x55, 0x55, 0x6e, 0xb9, 0xbd};

ROM_INFO    gRomInfo;
void        *gRomData;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetFlashTableInfo
//
// Description: Get Flash Data location and size.
//
// Input:       IN      EFI_SMBIOS_FLASH_DATA_PROTOCOL  *This
//              IN OUT  VOID                            **Location
//              IN OUT  UINT32                          *Size
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetFlashTableInfo (
	IN      EFI_SMBIOS_FLASH_DATA_PROTOCOL  *This,
	IN OUT  VOID 	                        **Location,
	IN OUT  UINT32 	                        *Size
)
{
	*Location = gRomData;
	*Size = gRomInfo.Size;
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetField
//
// Description: Get Flash Data Field
//
// Input:       IN EFI_SMBIOS_FLASH_DATA_PROTOCOL   *This
//              IN UINT8                            Table
//              IN UINT8                            Offset
//              IN VOID                             **Field
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetField (
	IN EFI_SMBIOS_FLASH_DATA_PROTOCOL  *This,
	IN UINT8                           Table,
	IN UINT8                           Offset,
	IN VOID                            **Field
)
{
    TABLE_INFO *p = gRomData;

    while ( p->Offset != 0xff && (p->Type != Table || p->Offset != Offset))	{
		p = (TABLE_INFO*)  ((UINT8*)(p+1) + p->Size);
	}

	if (p->Offset != 0xff) {
		*Field = p + 1;
		return EFI_SUCCESS;
	}

	*Field = 0;
	return EFI_NOT_FOUND;
}


EFI_SMBIOS_FLASH_DATA_PROTOCOL gSmbiosFlashDataProtocol = {
	GetFlashTableInfo,
	GetField
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindRawSection
//
// Description: Find the RAW section
//
// Input:       IN VOID  *Section
//              IN VOID  *End
//
// Output:      VOID* - Pointer to Raw Section if found
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID*
FindRawSection (
    IN VOID *Section,
    IN VOID *End
)
{
	EFI_COMMON_SECTION_HEADER	*p = Section;
	while((UINTN)p < (UINTN)End)	//Use signed because 0 = 0x100000000
	{
		if (p->Type == EFI_SECTION_RAW) return (p+1);

		if (Align4(SECTION_SIZE(p)) == 0x00) {
			return 0;               // Section size = 0 indicates data is corrupted
		}

		p = (EFI_COMMON_SECTION_HEADER*)((UINT8*)p+Align4(SECTION_SIZE(p)));
	}
	return 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindFile
//
// Description: Find file in the FV with the input GUID
//
// Input:       IN EFI_GUID  *Guid
//              IN VOID      *File
//              IN VOID      *EndOfFiles
//
// Output:      VOID* - Pointer to File if found
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID*
FindFile (
    IN EFI_GUID *Guid,
    IN VOID     *File,
    IN VOID     *EndOfFiles
)
{
	EFI_FFS_FILE_HEADER *p = File;
	while((UINTN)p < (UINTN)EndOfFiles)	// Use signed because 0 = 0x100000000
	{
		if (CompareGuid(Guid,&p->Name)) {
			// Found File.
			return FindRawSection(
				        p+1,
				        (UINT8*)p + (*(UINT32*)p->Size & 0xffffff) - sizeof(*p)
				        );
		}

		p = (EFI_FFS_FILE_HEADER*)((UINT8*)p + Align8((*(UINT32*)p->Size & 0xffffff)));
	}

	return NULL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetDataLocation
//
// Description: Find the Flash Data file in the FV.
//
// Input:       None
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetDataLocation (VOID)
{
	EFI_GUID							gEfiFirmwareVolumeBlockProtocolGuid = EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL_GUID;
    EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *FvBlock;
    EFI_PHYSICAL_ADDRESS                Address;
	EFI_HANDLE							*HandleBuffer;
	EFI_STATUS							Status;
    UINTN                               i;
	UINTN								NumHandles;
    void                                *FirstFile;
    void                                *EndOfFiles;
    void                                *File;

    Status = pBS->LocateHandleBuffer(ByProtocol,
                                    &gEfiFirmwareVolumeBlockProtocolGuid,
                                    NULL,
                                    &NumHandles,
                                    &HandleBuffer);
	if (EFI_ERROR(Status)) return Status;

    for (i = 0; i < NumHandles; ++i) {
        Status = pBS->HandleProtocol(HandleBuffer[i],
                                    &gEfiFirmwareVolumeBlockProtocolGuid,
                                    (void **)&FvBlock);
        if (EFI_ERROR(Status)) continue;

        Status = FvBlock->GetPhysicalAddress(FvBlock, &Address);

        if (Status == EFI_SUCCESS) {
            FirstFile = (UINT8*)Address
                        + ((EFI_FIRMWARE_VOLUME_HEADER*)Address)->HeaderLength;

            EndOfFiles = (UINT8*)Address
                        + ((EFI_FIRMWARE_VOLUME_HEADER*)Address)->FvLength;

#if (SMBIOS_X64_BUILD == 0)
            //BayTrail Build x32 hang at 0x70            
            //if (EndOfFiles == 0) EndOfFiles = (UINT8*)0xffffffff;   // For x32, 0 = 0x100000000  
            //
#endif

            File = FindFile(&FlashDataFile, FirstFile, EndOfFiles);
            if (!File) {
                Status = EFI_NOT_FOUND;
                continue;
            }

            gRomInfo = *(ROM_INFO*)File;
            gRomData = (ROM_INFO*)File + 1;
            break;
        }
    }

	pBS->FreePool(HandleBuffer);
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmbiosGetFlashDataInstall
//
// Description: Driver entry point for SmbiosGetFlashData
//
// Input:       IN EFI_HANDLE           ImageHandle
//              IN EFI_SYSTEM_TABLE     *SystemTable
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SmbiosGetFlashDataInstall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
)
{
	EFI_STATUS  Status;

    InitAmiLib(ImageHandle, SystemTable);

    Status = GetDataLocation();
    if (EFI_ERROR(Status)) return Status;

	return pBS->InstallMultipleProtocolInterfaces(
		            &ImageHandle,
		            &gAmiSmbiosFlashDataProtocolGuid,&gSmbiosFlashDataProtocol,
		            NULL);
}

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
