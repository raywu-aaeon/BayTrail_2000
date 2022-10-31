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
// $Header: /Alaska/SOURCE/Modules/SMBIOS/SmbiosDMIEditSupport/SmbiosDMIEditFunc.c 55    8/02/12 12:45p Davidd $
//
// $Revision: 55 $
//
// $Date: 8/02/12 12:45p $
//**********************************************************************//
//**********************************************************************//

#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Protocol\AmiSmbios.h>
#include "SmbiosDmiEdit.h"

#define SMBIOS_SIG (UINT32)'_MS_'
#define WRITE_ONCE_ENTRIES  0x10    // Maximum number of WRITE_ONCE_STATUS entries

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
extern UINT16 UpdateSmbiosTable (
    IN TABLE_INFO  *TableInfo,
    IN UINT8       *Data
);

VOID                        *gFlashData;
UINT32                      gFlashDataSize;
#else
CHAR16                      *DmiArrayVar = L"DmiArray";
DMI_VAR                     DmiArray[DMI_ARRAY_COUNT] = {0};
UINTN                       DmiArraySize = DMI_ARRAY_COUNT * sizeof(DMI_VAR);
UINT8                       *DmiData;
UINTN                       DmiDataSize;
CHAR16                      *Var = L"                ";
UINT8                       Index;
#endif                                  // SMBIOS_DMIEDIT_DATA_LOC

SMBIOS_TABLE_ENTRY_POINT    *SmbiosTableEntryPoint = NULL;
UINT8                       *ScratchBufferPtr = NULL;
UINT16						MaximumBufferSize;

//
// String type tables
//
STRING_TABLE    StringType_0[] =   {{0x04, 1, 1},
                                    {0x05, 2, 2},
                                    {0x08, 3, 3},
                                    {0xff, 0, 0},
                                   };

STRING_TABLE    StringType_1[] =   {{0x04, 1, 1},
                                    {0x05, 2, 2},
                                    {0x06, 3, 3},
                                    {0x07, 4, 4},
                                    {0x19, 5, 5},
                                    {0x1a, 6, 6},
                                    {0xff, 0, 0},
                                   };

STRING_TABLE    StringType_2[] =   {{0x04, 1, 1},
                                    {0x05, 2, 2},
                                    {0x06, 3, 3},
                                    {0x07, 4, 4},
                                    {0x08, 5, 5},
                                    {0x0a, 6, 6},
                                    {0xff, 0, 0},
                                   };

STRING_TABLE    StringType_3[NUMBER_OF_SYSTEM_CHASSIS][6] =
                                  {{{0x04, 1, 1},
                                    {0x06, 2, 2},
                                    {0x07, 3, 3},
                                    {0x08, 4, 4},
                                    {0x15 + (ELEMENT_COUNT_1 * ELEMENT_LEN_1), 5, 5},
                                    {0xff, 0, 0},
                                   },
#if NUMBER_OF_SYSTEM_CHASSIS > 1
                                   {
                                    {0x04, 1, 1},
                                    {0x06, 2, 2},
                                    {0x07, 3, 3},
                                    {0x08, 4, 4},
                                    {0x15 + (ELEMENT_COUNT_2 * ELEMENT_LEN_2), 5, 5},
                                    {0xff, 0, 0},
                                   },
#endif
#if NUMBER_OF_SYSTEM_CHASSIS > 2
                                   {
                                    {0x04, 1, 1},
                                    {0x06, 2, 2},
                                    {0x07, 3, 3},
                                    {0x08, 4, 4},
                                    {0x15 + (ELEMENT_COUNT_3 * ELEMENT_LEN_3), 5, 5},
                                    {0xff, 0, 0},
                                   },
#endif
#if NUMBER_OF_SYSTEM_CHASSIS > 3
                                   {
                                    {0x04, 1, 1},
                                    {0x06, 2, 2},
                                    {0x07, 3, 3},
                                    {0x08, 4, 4},
                                    {0x15 + (ELEMENT_COUNT_4 * ELEMENT_LEN_4), 5, 5},
                                    {0xff, 0, 0},
                                   },
#endif
#if NUMBER_OF_SYSTEM_CHASSIS > 4
                                   {
                                    {0x04, 1, 1},
                                    {0x06, 2, 2},
                                    {0x07, 3, 3},
                                    {0x08, 4, 4},
                                    {0x15 + (ELEMENT_COUNT_5 * ELEMENT_LEN_5), 5, 5},
                                    {0xff, 0, 0},
                                   },
#endif
                                  };                    // StringType_3

STRING_TABLE    StringType_4[] =   {{0x04, 1, 1},
                                    {0x07, 2, 2},
                                    {0x10, 3, 3},
                                    {0x20, 4, 4},
                                    {0x21, 5, 5},
                                    {0x22, 6, 6},
                                    {0xff, 0, 0},
                                   };

STRING_TABLE    StringType_22[] =  {{0x04, 1, 1},
                                    {0x05, 2, 2},
                                    {0x06, 3, 3},
                                    {0x07, 4, 4},
                                    {0x08, 5, 5},
                                    {0x0e, 6, 6},
                                    {0x14, 7, 7},
                                    {0xff, 0, 0},
                                   };

STRING_TABLE    StringType_39[] =  {{0x05, 1, 1},
                                    {0x06, 2, 2},
                                    {0x07, 3, 3},
                                    {0x08, 4, 4},
                                    {0x09, 5, 5},
                                    {0x0a, 6, 6},
                                    {0x0b, 7, 7},
                                    {0xff, 0, 0},
                                   };

//
// String table
//
VOID*    StringTable[] = {&StringType_0,        // 0
                          &StringType_1,        // 1
                          &StringType_2,        // 2
                          &StringType_3,        // 3
                          &StringType_4,        // 4
                          &StringType_22,       // 5
                          &StringType_39,       // 6
                         };

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetSmbiosTableF000
//
// Description: Searches 0x0F0000 for the SMBIOS signature and fills in the
//              SmbiosTableEntryPoint once it has been found
//
// Input:       None
//
// Output:      None
//
// Modified: SmbiosTableEntryPoint -is modified if table entry point found
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
GetSmbiosTableF000 (VOID)
{
    static BOOLEAN      NotDone = TRUE;

    if (NotDone) {
        UINT32              *ptr32;
        UINT16              i;

        ptr32 = (UINT32*)0xF0000;
        for (i = 0; i < (0x10000 / 16); i++, ptr32 += 4) {
            if (*ptr32 == SMBIOS_SIG) {
                SmbiosTableEntryPoint = (SMBIOS_TABLE_ENTRY_POINT*)ptr32;
                NotDone = FALSE;
                break;
            }
        }
    }
}

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:        WriteOnceTable
//
// Description: Table indicating which structure and offset can be written
//              only once or multiple times
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_THDR_END>
WRITE_ONCE_TABLE WriteOnceTable[] = {
                                      {1, 4, TRUE},
                                      {2, 4, TRUE},
                                    };

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WriteOnceStatusInit
//
// Description: Initialize NVRAM variable holding WriteOnce statuses
//
// Input:       None
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
WriteOnceStatusInit(VOID)
{
    EFI_STATUS          Status;
    WRITE_ONCE_STATUS   *Buffer;
    UINTN               BufferSize;

    BufferSize = WRITE_ONCE_ENTRIES * sizeof(WRITE_ONCE_STATUS);
    pBS->AllocatePool(EfiBootServicesData, BufferSize, &Buffer);

    // Create "WriteOnceStatus" variable if it does not exist
    Status = pRS->GetVariable(L"WriteOnceStatus",
                                &gAmiSmbiosNvramGuid,
                                NULL,
                                &BufferSize,
                                Buffer);

    if (Status == EFI_NOT_FOUND) {
        // WriteOnceStatus variable does not exist
        // Create one with default value of Type 127
        MemSet(Buffer, BufferSize, 127);

    	pRS->SetVariable(L"WriteOnceStatus",
                        &gAmiSmbiosNvramGuid,
                        EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                        BufferSize,
                        Buffer);
    }

    pBS->FreePool(Buffer);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   isWriteOnce
//
// Description: Determines if a given structure type and offset can only
//              be written once or multiple times.
//
// Input:       IN UINT8   Type
//              IN UINT8   Offset
//
// Output:      BOOLEAN - TRUE/FALSE for Write Once/Multiple
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
isWriteOnce(
    IN UINT8    Type,
    IN UINT8    Offset,
    IN UINT16   Handle
)
{
    EFI_STATUS          Status;
    BOOLEAN             WriteOnce = FALSE;
    UINT8               TableEntries = sizeof(WriteOnceTable)/sizeof(WRITE_ONCE_TABLE);
    WRITE_ONCE_STATUS   Buffer[WRITE_ONCE_ENTRIES];
    UINTN               BufferSize;
    UINT8               i;
    UINT8               j;

    BufferSize = WRITE_ONCE_ENTRIES * sizeof(WRITE_ONCE_STATUS);
    Status = pRS->GetVariable(L"WriteOnceStatus",
                                &gAmiSmbiosNvramGuid,
                                NULL,
                                &BufferSize,
                                Buffer);

    for (i = 0; i < TableEntries; ++i) {
        // Check for WriteOnce condition in WriteOnce table
        if (WriteOnceTable[i].Type == Type \
            && WriteOnceTable[i].Offset == Offset \
            && WriteOnceTable[i].WriteOnce) {
            // WriteOnce is set for input Type and Offset,
            // Check if WriteOnce was set already in WriteOnceStatus table
            // If "WriteOnceStatus" variable was not found then assume
            // WriteOnce was not set for this data field
            if (Status == EFI_SUCCESS) {
                for (j = 0; j < WRITE_ONCE_ENTRIES; ++j) {
                    if (Buffer[j].Type == 127) {
                        break;
                    }
                    if (Buffer[j].Type == Type && Buffer[j].Offset == Offset && Buffer[j].Handle == Handle) {
                        // WriteOnce was already set for input Type and Offset
                        WriteOnce = TRUE;
                        break;
                    }
                }
            }

            if (j < WRITE_ONCE_ENTRIES) {           // Make sure we are still within the WRITE_ONCE_ENTRIES
                // Create new WriteOnce entry if it did not exist for input Type and Offset
                if (WriteOnce == FALSE) {
                    Buffer[j].Type = Type;
                    Buffer[j].Offset = Offset;
                    Buffer[j].Handle = Handle;

                	pRS->SetVariable(L"WriteOnceStatus",
                                    &gAmiSmbiosNvramGuid,
                                    EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                    BufferSize,
                                    Buffer);
                }
            }
        }
    }

    return WriteOnce;
}

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetFlashDataInfo
//
// Description: Searches the Flash Data Table for a record of Type and
//              Offset. If found, returns the location found, the data size,
//              and the end of data.
//
// Input:       IN TABLE_INFO   RecordInfo
//
// Output:      FLASH_DATA_INFO
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
FLASH_DATA_INFO
GetFlashDataInfo(
    IN TABLE_INFO   *RecordInfo
)
{
    TABLE_INFO        *FlashDataPtr = gFlashData;
    FLASH_DATA_INFO   FlashDataInfo = {0, 0, 0};

    while (FlashDataPtr->Handle != 0xffff) {
        if (FlashDataPtr->Type == RecordInfo->Type &&
		    FlashDataPtr->Handle == RecordInfo->Handle &&
		    FlashDataPtr->Offset == RecordInfo->Offset &&
			FlashDataPtr->Flags == RecordInfo->Flags) {
			FlashDataInfo.Location = (UINT8*)FlashDataPtr;
			FlashDataInfo.Size = FlashDataPtr->Size;
		}

        FlashDataPtr = (TABLE_INFO*)((UINT8*)(FlashDataPtr + 1) + FlashDataPtr->Size);
	}
	FlashDataInfo.EndOfData = (UINT8*)FlashDataPtr;
	return FlashDataInfo;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetSmbiosInfo
//
// Description: Returns the SMBIOS information
//
// Input:       IN OUT  GET_SMBIOS_INFO   *p
//
// Output:      Returns 0
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetSmbiosInfo(
    IN OUT  GET_SMBIOS_INFO   *p
)
{
    if (!SmbiosTableEntryPoint) return DMI_FUNCTION_NOT_SUPPORTED;
    p->DmiBiosRevision = SmbiosTableEntryPoint->SmbiosBCDRevision;
    p->NumStructures = SmbiosTableEntryPoint->NumberOfSmbiosStructures;
    p->StructureSize = SmbiosTableEntryPoint->MaxStructureSize;
    p->pDmiStorageBase = SmbiosTableEntryPoint->TableAddress;
    p->DmiStorageSize = MaximumBufferSize;

    return 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetStructureByHandle
//
// Description: Searches the structure table for a record with its handle
//              equal to the input Handle.
//              Returns the pointer to the structure if found.
//              Returns NULL if not found
//
// Input:       IN UINT16    *Handle
//
// Output:      UINT8* - Pointer to structure found
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8*
GetStructureByHandle(
    IN UINT16    *Handle
)
{
    UINT8   *SmbiosTable = (UINT8*)((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableAddress;
    UINT8   *SmbiosTableEnd = SmbiosTable + ((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableLength;
    UINT8   *SmbiosTableNext;

    while(SmbiosTable < SmbiosTableEnd && ((SMBIOS_STRUCTURE_HEADER*)SmbiosTable)->Type != 127) {
        SmbiosTableNext = SmbiosTable + GetStructureLength(SmbiosTable);
        if (((SMBIOS_STRUCTURE_HEADER*)SmbiosTable)->Handle == *Handle) {
            return SmbiosTable;
        }
        SmbiosTable = SmbiosTableNext;
    }
    return NULL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetStructureByHandleThenUpdateHandle
//
// Description: Searches the structure table for a record with its handle
//              equal to the input Handle.
//              Returns the pointer to the structure if found.
//              Returns NULL if not found
//
// Input:       IN UINT16    *Handle
//
// Output:      UINT8* - Pointer to structure found
//              Sets Handle to the handle of the next structure
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8*
GetStructureByHandleThenUpdateHandle(
    IN UINT16    *Handle
)
{
    UINT8   *SmbiosTable = (UINT8*)((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableAddress;
    UINT8   *SmbiosTableEnd = SmbiosTable + ((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableLength;
    UINT8   *SmbiosTableNext;

    if (*Handle == 0) {
        SmbiosTableNext = SmbiosTable + GetStructureLength(SmbiosTable);
        if (SmbiosTableNext >= SmbiosTableEnd) *Handle = 0xffff;  //Last handle?
        else *Handle = ((DMI_STRUC*)SmbiosTableNext)->Handle;     //Return next handle
        return SmbiosTable;
    }

    while(SmbiosTable < SmbiosTableEnd) {
        SmbiosTableNext = SmbiosTable + GetStructureLength(SmbiosTable);
        if (((DMI_STRUC*)SmbiosTable)->Handle == *Handle) {
            if (SmbiosTableNext >= SmbiosTableEnd || ((DMI_STRUC*)SmbiosTable)->Type == 127 ) *Handle = 0xffff;  //Last handle?
            else *Handle = ((DMI_STRUC*)SmbiosTableNext)->Handle;     //Return next handle
            return SmbiosTable;
        }

        SmbiosTable = SmbiosTableNext;
    }

  return NULL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetSmbiosStructure
//
// Description: Searches the structure table for a record with its handle
//              equal to the input Handle and copies its content into
//              the provided buffer.
//
// Input:       IN OUT  GET_SMBIOS_STRUCTURE    *p
//
// Output:      GET_SMBIOS_STRUCTURE* - Input pointer "p" is loaded with
//                                      structure data.
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetSmbiosStructure(
    IN OUT  GET_SMBIOS_STRUCTURE    *p
)
{
    UINT8     *SmbStructurePtr;
    UINT32    TableSize;
    UINT8     *src, *dest;

    if (!SmbiosTableEntryPoint) return DMI_FUNCTION_NOT_SUPPORTED;

    SmbStructurePtr = GetStructureByHandleThenUpdateHandle((UINT16*)p->Handle32BitAddr);
    if (!SmbStructurePtr) return DMI_INVALID_HANDLE;

    TableSize = GetStructureLength(SmbStructurePtr);

    src = SmbStructurePtr;
    dest = (UINT8*)p->Buffer32BitAddr;
    while(TableSize--) *dest++ = *src++;  //Copy Table

    return 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetStructureLength
//
// Description: Returns the length of the structure pointed by BufferStart
//              in bytes
//
// Input:       IN UINT8     *BufferStart
//
// Output:      UINT16 - Size of the structure
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetStructureLength(
    IN UINT8     *BufferStart
)
{
    UINT8       *BufferEnd = BufferStart;

    BufferEnd += ((SMBIOS_STRUCTURE_HEADER*)BufferStart)->Length;

    while (*(UINT16*)BufferEnd != 0) {
        BufferEnd++;
    }

    return (UINT16)(BufferEnd + 2 - BufferStart);   // +2 for double zero terminator
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetInstanceByTypeHandle
//
// Description: Returns the instance of the input structure type and its handle
//
// Input:       IN UINT8    Type
//              IN UINT16   Handle
//
// Output:      Instance number (1-based) if found, or 0 if not found
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
GetInstanceByTypeHandle(
    IN UINT8    Type,
    IN UINT16   Handle
)
{
    UINT8   *Table = (UINT8*)((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableAddress;
    UINT8   *TableEnd = Table + ((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableLength;
    UINT8   Instance = 0;		// 1-based

    while ((Table < TableEnd) && ((SMBIOS_STRUCTURE_HEADER*)Table)->Type != 127) {
        if (((SMBIOS_STRUCTURE_HEADER*)Table)->Type == Type) {
        	Instance ++;
        }

        if (((SMBIOS_STRUCTURE_HEADER*)Table)->Handle == Handle) {
            return Instance;
        }

        Table = Table + GetStructureLength(Table);
    }

    return 0;					// Not found
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindStructureType
//
// Description: Find structure type starting from memory location pointed by
//              Buffer
//
// Input:       IN OUT  UINT8   **Buffer
//              IN OUT  UINT8   **StructureFoundPtr
//              IN      UINT8   SearchType
//              IN      UINT8   Instance
//
// Output:
//              BOOLEAN
//                  TRUE  - Structure found
//                  FALSE - Structure not found
//
//              If SearchType is found:
//                UINT8   **Buffer - Points to the next structure
//                UINT8   **StructureFoundPtr - Points to the structure
//                                              that was found
//              If SearchType is not found:
//                UINT8   **Buffer - No change
//                UINT8   **StructureFoundPtr = NULL
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
FindStructureType(
    IN OUT UINT8    **Buffer,
    IN OUT UINT8    **StructureFoundPtr,
	IN     UINT8    SearchType,
    IN     UINT8    Instance        // 1-based
)
{
    UINT8       *BufferPtr = *Buffer;
    BOOLEAN     FindStatus = FALSE;

    *StructureFoundPtr = NULL;
    while (((SMBIOS_STRUCTURE_HEADER*)BufferPtr)->Type != 127) {
        if (((SMBIOS_STRUCTURE_HEADER*)BufferPtr)->Type == SearchType) {
            // If this instance, set the find status flag and update the Buffer pointer
            if (--Instance == 0) {
                FindStatus = TRUE;
                *StructureFoundPtr = BufferPtr;
                *Buffer = BufferPtr + GetStructureLength(BufferPtr);
                break;
            }
        }
        BufferPtr += GetStructureLength(BufferPtr);
    }
    if ((FindStatus == FALSE) & (SearchType == 127)) {
        FindStatus = TRUE;
        *StructureFoundPtr = BufferPtr;
        *Buffer = BufferPtr + GetStructureLength(BufferPtr);
    }
    return FindStatus;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetStringTableIndex
//
// Description: Returns the string array index for a given Offset in
//              structure pointed by input StringTablePtr
//
// Input:       STRING_TABLE    *StringTablePtr
//              IN UINT8        Offset
//
// Output:      UINT8 - String array index
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
GetStringTableIndex(
    STRING_TABLE    *StringTablePtr,
    IN UINT8        Offset
)
{
    UINT8       i;

    for (i = 0; StringTablePtr->Offset != 0xff; i++) {
        if (StringTablePtr->Offset == Offset) break;
        StringTablePtr++;
    }

    return i;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetRemainingStructuresSize
//
// Description: Return the size from the Pointer Buffer to the last
//              structure 127.
//
// Input:       IN UINT8    *Buffer
//
// Output:      Size of remaining structure
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetRemainingStructuresSize(
    IN UINT8  *Buffer
)
{
    UINT16  Length = 0;
    UINT16  BlockSize;

    while (((SMBIOS_STRUCTURE_HEADER*)Buffer)->Type != 127) {
        BlockSize = GetStructureLength(Buffer);
        Length += BlockSize;
        Buffer += BlockSize;
    }
    Length += GetStructureLength(Buffer);
    return Length;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmbiosCheckSum
//
// Description: Returns the checksum of "length" bytes starting from the
//              "*ChecksumSrc"
//
// Input:       IN UINT8    *ChecksumSrc
//              IN UINT8    length
//
// Output:      UINT8 - Checksum value
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
SmbiosCheckSum(
    IN UINT8    *ChecksumSrc,
    IN UINT8    length
)
{
    UINT8     Checksum = 0;
    UINT8     i;

    for (i= 0; i < length; i++) {
        Checksum += *ChecksumSrc++;
    }
    return (0 - Checksum);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetLargestStructureSize
//
// Description: Returns the largest structure size
//
// Input:       IN UINT8    *Buffer
//
// Output:      UINT16 - Largest structure size
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetLargestStructureSize(
    IN UINT8     *Buffer
)
{
    UINT8     *BufferPtr = Buffer;
    UINT16    LargestStructureSize = 0;
    UINT16    CurrentStructureSize;

    while (((SMBIOS_STRUCTURE_HEADER*)BufferPtr)->Type != 127) {
        UINT8     *LastBufferPtr;

        LastBufferPtr = BufferPtr;
        BufferPtr += ((SMBIOS_STRUCTURE_HEADER*)BufferPtr)->Length;
        while(TRUE) {
            if ((*(UINT16*)BufferPtr) == 0) {
                BufferPtr += 2;
                break;
            }
            BufferPtr++;
        }
        CurrentStructureSize = (UINT16)(BufferPtr - LastBufferPtr);
        if (CurrentStructureSize > LargestStructureSize) {
            LargestStructureSize = CurrentStructureSize;
        }
    }
    return LargestStructureSize;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateHeaderInfo
//
// Description: Updates SMBIOS Entry Point Header
//
// Input:       None
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
UpdateHeaderInfo(VOID)
{
    SmbiosTableEntryPoint->TableLength = GetRemainingStructuresSize((UINT8*)SmbiosTableEntryPoint->TableAddress);
    SmbiosTableEntryPoint->IntermediateChecksum = 0;
    SmbiosTableEntryPoint->IntermediateChecksum = SmbiosCheckSum((UINT8*)SmbiosTableEntryPoint + 0x10, 15);
    SmbiosTableEntryPoint->MaxStructureSize = GetLargestStructureSize((UINT8*)SmbiosTableEntryPoint->TableAddress);
    SmbiosTableEntryPoint->EntryPointStructureChecksum = 0;
    SmbiosTableEntryPoint->EntryPointStructureChecksum = SmbiosCheckSum((UINT8*)SmbiosTableEntryPoint,
                                                                          SmbiosTableEntryPoint->EntryPointLength);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetTypeTable
//
// Description: Return pointer to the input type string table
//
// Input:       IN UINT8      Structure Type
//
// Output:      Pointer to the input type string table
//              (or NULL if not found)
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID*
GetTypeTable(
    IN UINT8    StructType
)
{
    UINT8       Index;

    switch (StructType) {
        case    0:
        case    1:
        case    2:
        case    3:
        case    4:  Index = StructType;
                    break;
        case   22:  Index = 5;
                    break;
        case   39:  Index = 6;
                    break;
        default:    Index = 0xff;
    }

    if (Index != 0xff) {
        return StringTable[Index];
    }
    else {
        return NULL;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetStringOffset
//
// Description: Returns the string offset for StringNumber from input string
//              buffer BufferStart
//
// Input:       IN  UINT8   *BufferStart
//              IN  UINT8   StringNumber (1-based)
//
// Output:      UINT16 - Offset from BufferStart
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetStringOffset(
    IN UINT8    *BufferStart,
    IN UINT8    StringNumber          // 1-based
)
{
    UINT8       *BufferEnd = BufferStart;

    while (--StringNumber) {
        while(*BufferEnd != 0) {
            BufferEnd++;
        }
        BufferEnd++;
    }

    return (UINT16)(BufferEnd - BufferStart);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindString
//
// Description: Returns pointer to the string number in structure BufferPtr
//
// Input:       IN OUT  UINT8    **BufferPtr
//              IN      UINT8    StringNumber
//
// Output:      UINT8   *BufferPtr = Pointer to the #StringNumber string
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
FindString(
    IN OUT UINT8    **BufferPtr,
    IN     UINT8    StringNumber          // 1-based
)
{
    *BufferPtr += ((SMBIOS_STRUCTURE_HEADER*)*BufferPtr)->Length;
    *BufferPtr += GetStringOffset(*BufferPtr, StringNumber);
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindLargestStrNumber
//
// Description: Return the largest string number in a structure
//
// Input:       IN UINT8    *StructPtr
//              IN UINT8    *StrTablePtr
//
// Output:      String number (1-based)
//              (or 0 if not found)
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
FindLargestStrNumber (
    IN UINT8            *StructPtr,
    IN STRING_TABLE     *StrTablePtr
)
{
    UINT8       Number;
    UINT8       StrNumber = 0;

    // Find largest string number from structure
    while (StrTablePtr->Offset != 0xff) {
        Number = *(StructPtr + StrTablePtr->Offset);
        if (Number > StrNumber) {
            StrNumber = Number;
        }
        StrTablePtr++;
    }

    return StrNumber;       // 1-based
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetStrNumber
//
// Description: Return the string number for a structure "Type" at "Offset"
//
// Input:       IN UINT8    Pointer to structure
//              IN UINT8    Type
//              IN UINT8    Offset
//
// Output:      String number (1-based)
//              (or 0xff if not found)
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
GetStrNumber(
    IN  UINT8       *StructPtr,
    IN  UINT8       Type,
    UINT8           Offset
)
{
    UINT8       *NextStructPtr = StructPtr;
    UINT8       *TempPtr;

    if (FindStructureType(&NextStructPtr, &TempPtr, Type, 1)) {
        return *(TempPtr + Offset);
    }
    else {
        return 0xff;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DeleteStringNumber
//
// Description: Zero out the string number in StructPtr
//
// Input:       IN  UINT8   *StructurePtr
//              IN UINT8    StrNumber
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
DeleteStringNumber (
    IN UINT8    *StructPtr,
    IN UINT8    StrNumber
)
{
    UINT8           Number;
    STRING_TABLE    *StrTablePtr;

    StrTablePtr = GetTypeTable(((SMBIOS_STRUCTURE_HEADER*)StructPtr)->Type);

    while (StrTablePtr->Offset != 0xff) {
        Number = *(StructPtr + StrTablePtr->Offset);
        if (Number > StrNumber) {
            *(StructPtr + StrTablePtr->Offset) = Number - 1;
        }
        if (Number == StrNumber) {
            *(StructPtr + StrTablePtr->Offset) = 0;
        }
        StrTablePtr++;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DeleteString
//
// Description: Delete string at Offset
//
// Input:       IN  UINT8   *StructPtr
//              IN UINT8    Offset
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
DeleteString (
    IN UINT8            *StructPtr,
    IN UINT8            Offset
)
{
    UINT8       StrNumber;
    UINT8       *TempPtr;
    UINT8       *StructEndPtr;
    UINTN       RemainingSize;

    StrNumber = GetStrNumber(StructPtr, ((SMBIOS_STRUCTURE_HEADER*)StructPtr)->Type, Offset);

    // Delete string number
    DeleteStringNumber(StructPtr, StrNumber);

    FindString(&StructPtr, StrNumber);              // StructPtr = StrNumber string
    TempPtr = StructPtr + Strlen(StructPtr) + 1;    // Move pointer to next string

    // Find end of structure
    StructEndPtr = TempPtr;
    while(*(UINT16*)StructEndPtr != 0) {
        StructEndPtr++;
    }

    // Copy remaining strings
    RemainingSize = StructEndPtr + 2 - TempPtr;     // Including double NULL characters
    MemCpy(StructPtr, TempPtr, RemainingSize);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReplaceString
//
// Description: Replace the #StringNum in the input buffer *DestStructPtr
//              with StringData
//
// Input:       IN UINT8       *DestStructPtr  Pointer to structure to be updated
//              IN UINT8       StringNum       String number (1 based)
//              IN UINT8       *StringData     String with NULL terminated character
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
ReplaceString(
    IN UINT8    *DestStructPtr,
    IN UINT8    StringNum,
    IN UINT8    *StringData
)
{
    UINT8       StringSize = 0;
    UINT8       *TempPtr;
    UINT8       *NextStrPtr;
    UINT8       *StructEndPtr;
    UINTN       RemainingSize;

    FindString(&DestStructPtr, StringNum);
    NextStrPtr = DestStructPtr;
    StructEndPtr = DestStructPtr;

    while(*NextStrPtr != 0) {
        NextStrPtr++;
    }

    // NextStrPtr = Pointer to the next string
    NextStrPtr++;

    while(*(UINT16*)StructEndPtr != 0) {
        StructEndPtr++;
    }

    RemainingSize = StructEndPtr + 2 - NextStrPtr;  // Including double NULL characters

    TempPtr = StringData;
    while (*(TempPtr++) != 0) {
        StringSize++;
    }
    StringSize++;                   // Including NULL character

    // Copy remaining strings
    MemCpy(DestStructPtr + StringSize, NextStrPtr, RemainingSize);

    // Copy the string
    MemCpy(DestStructPtr, StringData, StringSize);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AddStringNumber
//
// Description: Add new string number for a structure "Type" at "Offset".
//              Return the string index, assuming all strings exist in the
//              structure according to the Smbios specification
//
// Input:       IN UINT8    Pointer to SmbiosTable or Structure
//              IN UINT8    Type
//              IN UINT8    Offset
//
// Output:      String index (0-based)
//              (0xff if not found)
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
AddStringNumber(
    IN  UINT8       *SmbiosTable,
    IN  UINT8       Type,
    UINT8           Offset
)
{
    STRING_TABLE    *StrTablePtr;
    UINT8           *NextStructPtr = SmbiosTable;
    UINT8           *TempPtr;
    UINT8           Index = 0xff;
    UINT8           StrNumber = 0;
    UINT8           Number;

    if (FindStructureType(&NextStructPtr, &TempPtr, Type, 1)) {
        StrTablePtr = GetTypeTable(Type);
        if (StrTablePtr != NULL) {
            // Find largest string number from structure
            while (StrTablePtr->Offset != 0xff) {
                if (StrTablePtr->Offset == Offset) {
                    // String index in Smbios spec
                    Index = StrTablePtr->SpecStrNum - 1;        // 0-based
                }

                Number = *(TempPtr + StrTablePtr->Offset);
                if (Number > StrNumber) {
                    StrNumber = Number;
                }
                StrTablePtr++;
            }

            // Assign next string number to structure at input Offset
            *(TempPtr + Offset) = ++StrNumber;

            return Index;           // 0-based
        }
    }

    return 0xff;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AddNullTerminator
//
// Description: Add NULL terminator to the end of the structure
//
// Input:       IN UINT8   *StructPtr
//              IN UINT8   *StrTablePtr
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
AddNullTerminator (
    IN UINT8            *StructPtr,
    IN STRING_TABLE     *StrTablePtr
)
{
    UINT8       StrNumber;
    UINT8       i;

    // Find largest string number
    StrNumber = FindLargestStrNumber(StructPtr, StrTablePtr);

    // Skip to string section
    StructPtr += ((SMBIOS_STRUCTURE_HEADER*)StructPtr)->Length;

    // Move pointer to end of last string
    for (i = 0; i < StrNumber; i++) {
        while (*StructPtr != 0) StructPtr++;
        StructPtr++;
    }

    // Add NULL terminator
    *StructPtr = 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateStrings
//
// Description: Updates strings in SMBIOS Structure with input Handle
//              in Runtime with DMI data
//
// Input:       IN UINT16      Handle,
//              IN TABLE_INFO  TableInfo,
//              IN UINT8       *Data
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
UpdateStrings(
    IN UINT16           Handle,
    IN TABLE_INFO       TableInfo,
    IN UINT8            *Data
)
{
    UINT8               *TablePtr;
    UINT8               *TempBuffer;
    UINT8               *StructPtr;
    UINT8               i;
    UINT16              BlockSize;
    UINT16              AvailableBlkSize;
    STRING_TABLE        *StrTablePtr;
    UINT8               StrNumber;
    UINT8               Instance;

    // Check if enough space
    AvailableBlkSize = MaximumBufferSize - SmbiosTableEntryPoint->TableLength;
    if (AvailableBlkSize < (Strlen(Data) + 1)) {
        return DMI_BAD_PARAMETER;               // Not enough space
    }

    // Get pointer to structure to be updated
    StructPtr = GetStructureByHandle(&Handle);
    if (StructPtr == NULL) {
        return DMI_INVALID_HANDLE;
    }

    // Get pointer to the StringTable
    StrTablePtr = GetTypeTable(((SMBIOS_STRUCTURE_HEADER*)StructPtr)->Type);
    if (((SMBIOS_STRUCTURE_HEADER*)StructPtr)->Type == 3) {
        Instance = GetInstanceByTypeHandle(3, Handle);
        StrTablePtr += 6 * (Instance - 1);
    }

    if (StrTablePtr == NULL) return DMI_BAD_PARAMETER;

    // Copy structure data
    TempBuffer = ScratchBufferPtr;
    BlockSize = GetStructureLength(StructPtr);
    MemCpy(TempBuffer, StructPtr, BlockSize);

    // Set TablePtr to next structure
    TablePtr = StructPtr + BlockSize;


    // Update String fields
    for (i = 0; StrTablePtr[i].Offset != 0xff; i++) {
        // Update string at input Offset
        if (StrTablePtr[i].Offset == TableInfo.Offset) {
            // Update string if input data not empty, else delete it
            if (Strlen(Data)) {
                BlockSize = (UINT16)Strlen(Data) + 1;
                // Add string if does not exist, else replace it
                StrNumber = GetStrNumber(TempBuffer, TableInfo.Type, TableInfo.Offset);
                if (StrNumber == 0) {
                    AddStringNumber(TempBuffer, TableInfo.Type, TableInfo.Offset);
                    StrNumber = GetStrNumber(TempBuffer, TableInfo.Type, TableInfo.Offset);
                }
                ReplaceString(TempBuffer, StrNumber, Data);
            }
            else {
                DeleteString(TempBuffer, TableInfo.Offset);
            }
        }
    }

    // Add structure terminator Null byte
    AddNullTerminator(TempBuffer, StrTablePtr);

    BlockSize = GetRemainingStructuresSize(TablePtr);
    MemCpy(TempBuffer + GetStructureLength(TempBuffer), TablePtr, BlockSize);

    // Replace all DMI data with TempBuffer
    TempBuffer = ScratchBufferPtr;
    BlockSize = GetRemainingStructuresSize(TempBuffer);
    MemCpy(StructPtr, TempBuffer, BlockSize);

    // Update SMBIOS Structure Table Entry Point - Structure Table Length, Intermediate checksum
    UpdateHeaderInfo();

    return DMI_SUCCESS;
}

#if OEM_STRING_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DynamicUpdateType11
//
// Description: Updates SMBIOS Type 11 Structure in Runtime with DMI data
//
// Input:       IN UINT16      Handle,
//              IN TABLE_INFO  TableInfo,
//              IN UINT8       *Data
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
DynamicUpdateType11(
    IN UINT16      Handle,
    IN TABLE_INFO  TableInfo,
    IN UINT8       *Data
)
{
    UINT8               *TablePtr;
    UINT8               *TempBuffer;
    UINT8               *StructPtr;
    UINT16              BlockSize;
    UINT16              StringSize;
    UINT8               i;
    UINT16              AvailableBlkSize;
    UINT8               Count;

    StructPtr = GetStructureByHandle(&Handle);
    if (StructPtr == NULL) {
        return DMI_INVALID_HANDLE;
    }

    TablePtr = StructPtr;
    TempBuffer = ScratchBufferPtr;

    AvailableBlkSize = MaximumBufferSize - SmbiosTableEntryPoint->TableLength;
    if (AvailableBlkSize < (Strlen(Data) + 1)) {
        return DMI_BAD_PARAMETER;               // Not enough space
    }

    // Copy structure data (without string data)
    BlockSize = ((SMBIOS_STRUCTURE_HEADER*)TablePtr)->Length;
    MemCpy(TempBuffer, TablePtr, BlockSize);
    Count = ((SMBIOS_OEM_STRINGS_INFO*)TempBuffer)->Count;

    TablePtr += BlockSize;
    TempBuffer += BlockSize;

    // string fields
    for (i = 1; i < (Count + 1); i++) {
        StringSize = (UINT16)Strlen(TablePtr) + 1;       // Size including string NULL terminator
        if (TableInfo.Offset == i) {
            BlockSize = (UINT16)Strlen(Data) + 1;
            MemCpy(TempBuffer, Data, BlockSize);
            TempBuffer += BlockSize;
        }
        else {
            MemCpy(TempBuffer, TablePtr, StringSize);
            TempBuffer += StringSize;
        }
        TablePtr += StringSize;
    }

    // Add NULL byte for end of string-set
    *TempBuffer = 0;
    TempBuffer++;
    TablePtr++;

    BlockSize = GetRemainingStructuresSize(TablePtr);
    MemCpy(TempBuffer, TablePtr, BlockSize);

    // Replace all DMI data with TempBuffer
    TempBuffer = ScratchBufferPtr;
    BlockSize = GetRemainingStructuresSize(TempBuffer);
    MemCpy(StructPtr, TempBuffer, BlockSize);

    // Update SMBIOS Structure Table Entry Point - Structure Table Length, Intermediate checksum
    UpdateHeaderInfo();

    return DMI_SUCCESS;
}
#endif

#if SYSTEM_CONFIG_OPTION_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DynamicUpdateType12
//
// Description: Updates SMBIOS Type 12 Structure in Runtime with DMI data
//
// Input:       IN UINT16      Handle,
//              IN TABLE_INFO  TableInfo,
//              IN UINT8       *Data
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
DynamicUpdateType12(
    IN UINT16      Handle,
    IN TABLE_INFO  TableInfo,
    IN UINT8       *Data
)
{
    UINT8               *TablePtr;
    UINT8               *TempBuffer;
    UINT8               *StructPtr;
    UINT16              BlockSize;
    UINT16              StringSize;
    UINT8               i;
    UINT16              AvailableBlkSize;
    UINT8               Count;

    StructPtr = GetStructureByHandle(&Handle);
    if (StructPtr == NULL) {
        return DMI_INVALID_HANDLE;
    }

    TablePtr = StructPtr;
    TempBuffer = ScratchBufferPtr;

    AvailableBlkSize = MaximumBufferSize - SmbiosTableEntryPoint->TableLength;
    if (AvailableBlkSize < (Strlen(Data) + 1)) {
        return DMI_BAD_PARAMETER;               // Not enough space
    }

    // Copy structure data (without string data)
    BlockSize = ((SMBIOS_STRUCTURE_HEADER*)TablePtr)->Length;
    MemCpy(TempBuffer, TablePtr, BlockSize);
    Count = ((SMBIOS_SYSTEM_CONFIG_INFO*)TempBuffer)->Count;

    TablePtr += BlockSize;
    TempBuffer += BlockSize;

    // string fields
    for (i = 1; i < (Count + 1); i++) {
        StringSize = (UINT16)Strlen(TablePtr) + 1;       // Size including string NULL terminator
        if (TableInfo.Offset == i) {
            BlockSize = (UINT16)Strlen(Data) + 1;
            MemCpy(TempBuffer, Data, BlockSize);
            TempBuffer += BlockSize;
        }
        else {
            MemCpy(TempBuffer, TablePtr, StringSize);
            TempBuffer += StringSize;
        }
        TablePtr += StringSize;
    }

    // Add NULL byte for end of string-set
    *TempBuffer = 0;
    TempBuffer++;
    TablePtr++;

    BlockSize = GetRemainingStructuresSize(TablePtr);
    MemCpy(TempBuffer, TablePtr, BlockSize);

    // Replace all DMI data with TempBuffer
    TempBuffer = ScratchBufferPtr;
    BlockSize = GetRemainingStructuresSize(TempBuffer);
    MemCpy(StructPtr, TempBuffer, BlockSize);

    // Update SMBIOS Structure Table Entry Point - Structure Table Length, Intermediate checksum
    UpdateHeaderInfo();

    return DMI_SUCCESS;
}
#endif

/////////////////////////////////////////////
// Worker function for setting structures. //
/////////////////////////////////////////////

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC == 2
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   StoreNvramData
//
// Description: Store DMIEdit data into input variable
//
// Input:       IN  CHAR16  *Var
//              IN  VOID    *Data
//              IN  UINTN   DataSize
//
//              Global variable "DmiArray", "DmiArraySize",
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
StoreNvramData(
    IN  CHAR16  *Var,
    IN  VOID    *Data,
    IN  UINTN   DataSize
)
{
    EFI_STATUS  Status;
    UINTN       Size;
    UINT8       *Buffer;

	// Check if variable already exists
    //
    // Size of zero is used to detect if the variable exists.
    // If the variable exists, an error code of EFI_BUFFER_TOO_SMALL
    // would be returned
    Size = 0;
    Status = pRS->GetVariable(
                        Var,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &Size,
                        &Buffer);

    if (Status == EFI_NOT_FOUND) {
		// Record not present, increment record count
        DmiArray[0].Type += 1;

        Status = pRS->SetVariable(
                            DmiArrayVar,
                            &gAmiSmbiosNvramGuid,
                            EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                            DmiArraySize,
                            &DmiArray);
        ASSERT_EFI_ERROR(Status);
    }

	// Update DMI data record if already exists,
	// or store new record if total record count in DmiArray was successfully
	// updated
    if (Status == EFI_BUFFER_TOO_SMALL || Status == EFI_SUCCESS) {
	    Status = pRS->SetVariable(
	                        Var,
	                        &gAmiSmbiosNvramGuid,
	                        EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                            DataSize,
	                        Data);
	    ASSERT_EFI_ERROR(Status);
	}

    return Status;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetDmiDataSize
//
// Description: Returns the data size for DMI Function 0x52
//
// Input:       IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//
// Output:      UINT16 - Data Size
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
GetDmiDataSize(
    IN SET_SMBIOS_STRUCTURE_DATA   *Data
)
{
    switch(Data->Command) {
        case 0:
                return 1;
        case 1:
                return 2;
        case 2:
                return 4;
        case 4:
                return 0;                   // Delete command, size does not matter
        default:
                return Data->DataLength;    // Add, String, or Block change command
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetInputDataInfo
//
// Description: Fills "TableInfo" with data from DMI Function 0x52
//
// Input:       IN      UINT16                      Handle,
//              IN      SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN OUT  TABLE_INFO                  *TableInfo
//
// Output:      UINT16 - Data Size
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
GetInputDataInfo(
    IN      UINT16                      Handle,
    IN      SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN OUT  TABLE_INFO                  *TableInfo
)
{
    TableInfo->Type = Data->StructureHeader.Type;
    TableInfo->Offset = Data->FieldOffset;
    TableInfo->Reserved = 0;
    TableInfo->Flags = DMIEDIT_EXTENDED_HDR;
    TableInfo->HdrLength = sizeof(TABLE_INFO);
    TableInfo->Size = GetDmiDataSize(Data);
    TableInfo->Handle = Handle;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType0
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 0 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:       Type 0 Offset 8 (Release Date) is a fixed form string. This
//              function only checks for proper length. It is up to the DMI
//              editing utility to check for the proper format.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType0(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS  Status;
    TABLE_INFO  TableInfo;

    if (Data->Command != 5) return DMI_BAD_PARAMETER;

    if ( Data->FieldOffset != 4
      && Data->FieldOffset != 5
      && Data->FieldOffset != 8
    ) return DMI_BAD_PARAMETER;

    if ((Data->FieldOffset == 8) && (Data->DataLength != 11)) {
        return DMI_BAD_PARAMETER; // Date string is fixed size
    }

    if (Set == FALSE) return DMI_SUCCESS;

    if (isWriteOnce(0, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2

    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);

#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 0;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    if (Set == FALSE) return DMI_SUCCESS;

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }

    // Dynamically update strings in Smbios table
    return UpdateStrings(Handle, TableInfo, Data->StructureData);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType1
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 1 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType1(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;

    if (isWriteOnce(1, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
    switch (Data->FieldOffset) {
        case 0x04 :
        case 0x05 :
        case 0x06 :
        case 0x07 :
        case 0x19 :
        case 0x1a : if (Data->Command != 5) return DMI_BAD_PARAMETER;
                    if (Set == FALSE) return DMI_SUCCESS;

                    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);
                    break;

        default:    if ((Data->FieldOffset > 0x07) && (Data->FieldOffset < 0x18)) {
                        UINT8       *Ptr;
                        UINT8       *UuidPtr;
                        UINT8       i;

                        Ptr = GetStructureByHandle(&Handle);
                        UuidPtr = (UINT8*)&((SMBIOS_SYSTEM_INFO*)Ptr)->Uuid;
                        Ptr = UuidPtr + Data->FieldOffset - 8;

                        if (Data->Command < 3) {
                            if (Data->Command == 0) {
                                *Ptr &= (UINT8)Data->ChangeMask;
                                *Ptr |= (UINT8)Data->ChangeValue;
                            }
                            if (Data->Command == 1) {
                                *(UINT16*)Ptr &= (UINT16)Data->ChangeMask;
                                *(UINT16*)Ptr |= (UINT16)Data->ChangeValue;
                            }
                            if (Data->Command == 2) {
                                *(UINT32*)Ptr &= Data->ChangeMask;
                                *(UINT32*)Ptr |= Data->ChangeValue;
                            }
                        }
                        else if (Data->Command == 6) {
                            for (i = 0; i < (UINT8)TableInfo.Size; i++) {
                                Ptr[i] = Data->StructureData[i];
                            }
                        }
                        else {
                            return DMI_BAD_PARAMETER;
                        }

                        if (Set == FALSE) return DMI_SUCCESS;

                        Status = UpdateSmbiosTable(&TableInfo, UuidPtr);
                    }
                    else {
                        return DMI_BAD_PARAMETER;
                    }
    }
#else
{
    VOID    *NvramData;

    NvramData = &Data->StructureData;

	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 1;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    switch (Data->FieldOffset) {
        case 0x04 :
        case 0x05 :
        case 0x06 :
        case 0x07 :
        case 0x19 :
        case 0x1a : if (Data->Command != 5) return DMI_BAD_PARAMETER;
                    break;
        default:    if ((Data->FieldOffset > 0x07) && (Data->FieldOffset < 0x18)) {
                        UINT8       *Ptr;
                        UINT8       *UuidPtr;
                        UINT8       i;

                        Ptr = GetStructureByHandle(&Handle);
                        UuidPtr = (UINT8*)&((SMBIOS_SYSTEM_INFO*)Ptr)->Uuid;
                        Ptr = UuidPtr + Data->FieldOffset - 8;

                        if (Data->Command < 3) {
                            if (Data->Command == 0) {
                                *Ptr &= (UINT8)Data->ChangeMask;
                                *Ptr |= (UINT8)Data->ChangeValue;
                            }
                            if (Data->Command == 1) {
                                *(UINT16*)Ptr &= (UINT16)Data->ChangeMask;
                                *(UINT16*)Ptr |= (UINT16)Data->ChangeValue;
                            }
                            if (Data->Command == 2) {
                                *(UINT32*)Ptr &= Data->ChangeMask;
                                *(UINT32*)Ptr |= Data->ChangeValue;
                            }
                        }
                        else if (Data->Command == 6) {
                            for (i = 0; i < (UINT8)TableInfo.Size; i++) {
                                Ptr[i] = Data->StructureData[i];
                            }
                        }
                        else {
                            return DMI_BAD_PARAMETER;
                        }

                        DmiArray[Index].Offset = 0x08;
                        NvramData = UuidPtr;
                    }
                    else {
                        return DMI_BAD_PARAMETER;
                    }
    }

    if (Set == FALSE) return DMI_SUCCESS;

    Status = StoreNvramData(Var, NvramData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }

    if ((Data->FieldOffset > 0x07) && (Data->FieldOffset < 0x18)) {
        return DMI_SUCCESS;
    }
    else {
        // Dynamically update strings in Smbios table
        return UpdateStrings(Handle, TableInfo, Data->StructureData);
    }
}

#if BASE_BOARD_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType2
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 2 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType2(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;

    if (Data->Command != 5) return DMI_BAD_PARAMETER;
    if ( Data->FieldOffset != 4
      && Data->FieldOffset != 5
      && Data->FieldOffset != 6
      && Data->FieldOffset != 7
      && Data->FieldOffset != 8
      && Data->FieldOffset != 0x0a
    ) return DMI_BAD_PARAMETER;

    if (Set == FALSE) return DMI_SUCCESS;

    if (isWriteOnce(2, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2

    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);

#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 2;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }

    // Dynamically update strings in Smbios table
    return UpdateStrings(Handle, TableInfo, Data->StructureData);
}
#endif

#if SYS_CHASSIS_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType3
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 3 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType3(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;
    UINT8               *StructPtr;
    UINT8               Instance;
    STRING_TABLE        *StringTablePtr;

    switch (Data->FieldOffset) {
        case 4:
        case 6:
        case 7:
        case 8:     if (Data->Command != 5) return DMI_BAD_PARAMETER;
                    break;
        case 5:     if (Data->Command != 0) return DMI_BAD_PARAMETER;
                    break;
        case 0x0d:  if (Data->Command != 2) return DMI_BAD_PARAMETER;
                    break;
        default:    {
                        // Get instance number
                        Instance = GetInstanceByTypeHandle(3, Handle);
                        StringTablePtr = &StringType_3[0][0];
                        StringTablePtr += 6 * (Instance - 1);

                        while (StringTablePtr->Offset != 0xff) {
                            if (StringTablePtr->Offset == Data->FieldOffset) {
                                break;
                            }

                            StringTablePtr++;
                        };

                        if (StringTablePtr->Offset != 0xff) {
                            if (Data->Command != 0x5) {
                                return DMI_BAD_PARAMETER;
                            }
                        }
                        else {
                            return DMI_BAD_PARAMETER;
                        }
                    }
    }

    if (Set == FALSE) return DMI_SUCCESS;

    if (isWriteOnce(3, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
    if (Data->Command == 0 || Data->Command == 0x2)
        *(UINT32*)Data->StructureData = Data->ChangeValue;

    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);

#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 3;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    if (Data->Command == 0 || Data->Command == 0x2)
        *(UINT32*)Data->StructureData = Data->ChangeValue;

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }

    // Dynamically update the structure in the Smbios table
    StructPtr = GetStructureByHandle(&Handle);
    if (StructPtr != NULL) {
        switch (Data->FieldOffset) {
            case 0x05:  ((SMBIOS_SYSTEM_ENCLOSURE_INFO*)StructPtr)->Type &= (UINT8)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_ENCLOSURE_INFO*)StructPtr)->Type |= (UINT8)Data->ChangeValue;
                        break;
            case 0x0d:  ((SMBIOS_SYSTEM_ENCLOSURE_INFO*)StructPtr)->OemDefined &= (UINT32)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_ENCLOSURE_INFO*)StructPtr)->OemDefined |= (UINT32)Data->ChangeValue;
        }
    }

    return UpdateStrings(Handle, TableInfo, Data->StructureData);
}
#endif

#if PROCESSOR_DMIEDIT_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType4
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 4 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType4(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;

    switch (Data->FieldOffset) {
        case 0x20:
        case 0x21:
        case 0x22:  if (Data->Command != 0x5) return DMI_BAD_PARAMETER;
                    break;
        default:    return DMI_BAD_PARAMETER;
    }

    if (Set == FALSE) return DMI_SUCCESS;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);
#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 4;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }

    // Dynamically update the structure in the Smbios table
    return UpdateStrings(Handle, TableInfo, Data->StructureData);
}
#endif

#if OEM_STRING_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType11
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 11 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType11(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;
    static UINT8        StringNumber = 0;

    if (isWriteOnce(11, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

    if (Data->Command == 0) {
        if (Data->FieldOffset != 4) return DMI_BAD_PARAMETER;
        if (Set == FALSE) return DMI_SUCCESS;

        StringNumber = (UINT8) Data->ChangeValue;
        return DMI_SUCCESS;
    }

    if (Data->Command != 5) return DMI_BAD_PARAMETER;
    if (Data->FieldOffset != 4) return DMI_BAD_PARAMETER;
    if (!StringNumber)  return DMI_BAD_PARAMETER;
    if (Set == FALSE) return DMI_SUCCESS;

    TableInfo.Offset = StringNumber;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2

    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);

#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 11;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = StringNumber - 1;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }
    return DynamicUpdateType11(Handle, TableInfo, Data->StructureData);
}
#endif

#if SYSTEM_CONFIG_OPTION_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType12
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 12 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType12(
    UINT16                      Handle,
    SET_SMBIOS_STRUCTURE_DATA   *Data,
    BOOLEAN                     Set
)
{
    EFI_STATUS  	Status;
    TABLE_INFO  	TableInfo;
    static UINT8	StringNumber = 0;

    if (Data->Command == 0) {
        if (Data->FieldOffset != 4) return DMI_BAD_PARAMETER;
        if (Set == FALSE) return DMI_SUCCESS;

        StringNumber = (UINT8) Data->ChangeValue;
        return DMI_SUCCESS;
    }

    if (Data->Command != 5) return DMI_BAD_PARAMETER;
    if (Data->FieldOffset != 4) return DMI_BAD_PARAMETER;
    if (!StringNumber)  return DMI_BAD_PARAMETER;
    if (Set == FALSE) return DMI_SUCCESS;

    if (isWriteOnce(12, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

    TableInfo.Offset = StringNumber;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);
    if (Status != 0) {
        return (UINT16)Status;
    }
#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 12;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = StringNumber - 1;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }
    return DynamicUpdateType12(Handle, TableInfo, Data->StructureData);
}
#endif

#if PORTABLE_BATTERY_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType22
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 22 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType22(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;
    UINT8               *StructPtr;

    switch (Data->FieldOffset) {
        case 0x09:
        case 0x0f:
        case 0x15:  if (Data->Command != 0) return DMI_BAD_PARAMETER;
                    break;
        case 0x0a:
        case 0x0c:
        case 0x10:
        case 0x12:  if (Data->Command != 1) return DMI_BAD_PARAMETER;
                    break;
        case 0x16:  if (Data->Command != 2) return DMI_BAD_PARAMETER;
                    break;
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x0e:
        case 0x14:  if (Data->Command != 5) return DMI_BAD_PARAMETER;
                    break;
        default:    return DMI_BAD_PARAMETER;
    }

    if (Set == FALSE) return DMI_SUCCESS;

    if (isWriteOnce(22, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

    if (Data->Command == 0 || Data->Command == 0x1 || Data->Command == 0x2)
        *(UINT32*)Data->StructureData = Data->ChangeValue;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2

    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);

#else
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 22;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
#endif

    if (Status) {
        return (UINT16)Status;
    }

    // Dynamically update the structure in the Smbios table
    StructPtr = GetStructureByHandle(&Handle);
    if (StructPtr != NULL) {
        switch (Data->FieldOffset) {
            case 0x09:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DeviceChemistry &= (UINT8)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DeviceChemistry |= (UINT8)Data->ChangeValue;
                        break;
            case 0x0a:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DesignCapacity &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DesignCapacity |= (UINT16)Data->ChangeValue;
                        break;
            case 0x0c:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DesignVoltage &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DesignVoltage |= (UINT16)Data->ChangeValue;
                        break;
            case 0x0f:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->MaxErrorInBatteryData &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->MaxErrorInBatteryData |= (UINT16)Data->ChangeValue;
                        break;
            case 0x10:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->SBDSSerialNumber &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->SBDSSerialNumber |= (UINT16)Data->ChangeValue;
                        break;
            case 0x12:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->SBDSManufacturerDate &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->SBDSManufacturerDate |= (UINT16)Data->ChangeValue;
                        break;
            case 0x15:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DesignCapabilityMult &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->DesignCapabilityMult |= (UINT16)Data->ChangeValue;
                        break;
            case 0x16:  ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->OEMSpecific &= (UINT32)Data->ChangeMask;
                        ((SMBIOS_PORTABLE_BATTERY_INFO*)StructPtr)->OEMSpecific |= (UINT32)Data->ChangeValue;
                        break;
        }
    }

    return UpdateStrings(Handle, TableInfo, Data->StructureData);
}
#endif

#if SYSTEM_POWER_SUPPLY_INFO
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetType39
//
// Description: Updates Flash Data record with input DMI data
//              Updates SMBIOS Type 39 Structure in Runtime with DMI data
//
// Input:       IN UINT16                      Handle,
//              IN SET_SMBIOS_STRUCTURE_DATA   *Data,
//              IN BOOLEAN                     Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetType39(
    IN UINT16                      Handle,
    IN SET_SMBIOS_STRUCTURE_DATA   *Data,
    IN BOOLEAN                     Set
)
{
    EFI_STATUS          Status;
    TABLE_INFO          TableInfo;
    UINT8               *StructPtr;

    switch (Data->FieldOffset) {
        case 0x04:  if (Data->Command != 0) return DMI_BAD_PARAMETER;
                    break;
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0a:
        case 0x0b:  if (Data->Command != 5) return DMI_BAD_PARAMETER;
                    break;
        case 0x0c:
        case 0x0e:
        case 0x10:
        case 0x12:
        case 0x14:  if (Data->Command != 1) return DMI_BAD_PARAMETER;
                    break;
        default:    return DMI_BAD_PARAMETER;
    }

    if (Set == FALSE) return DMI_SUCCESS;

    if (isWriteOnce(39, Data->FieldOffset, Handle)) return DMI_READ_ONLY;

    // Fill TableInfo with input data
    GetInputDataInfo(Handle, Data, &TableInfo);

    if (Data->Command == 0 || Data->Command == 0x1)
        *(UINT32*)Data->StructureData = Data->ChangeValue;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2

    Status = UpdateSmbiosTable(&TableInfo, Data->StructureData);

#else
{
	//
	// Get number of DMI data records in NVRAM
	//
	// Note: DMI data record actually starts with record #1,
	//		 first record #0 holds total number of DMI data records
	//       instead of TABLE_INFO
	//       ===> DmiArray[0].Type = count
	//
    Status = pRS->GetVariable(
                        DmiArrayVar,
                        &gAmiSmbiosNvramGuid,
                        NULL,
                        &DmiArraySize,
                        &DmiArray);

	if (Status == EFI_SUCCESS) {
	    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
	    ++Index;
	}
	else {
		Index = 1;
	}

    DmiArray[Index].Type = 39;
    DmiArray[Index].Handle = Handle;
    DmiArray[Index].Offset = Data->FieldOffset;
    DmiArray[Index].Flags = 0;

    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
			   DmiArray[Index].Type,
			   DmiArray[Index].Handle,
			   DmiArray[Index].Offset,
			   DmiArray[Index].Flags);

    Status = StoreNvramData(Var, &Data->StructureData, (UINTN)TableInfo.Size);
}
#endif

    if (Status) {
        return (UINT16)Status;
    }

    // Dynamically update the structure in the Smbios table
    StructPtr = GetStructureByHandle(&Handle);
    if (StructPtr != NULL) {
        switch (Data->FieldOffset) {
            case 0x04:  ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->PwrUnitGroup &= (UINT8)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->PwrUnitGroup |= (UINT8)Data->ChangeValue;
                        break;
            case 0x0c:  ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->MaxPwrCapacity &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->MaxPwrCapacity |= (UINT16)Data->ChangeValue;
                        break;
            case 0x0e:  ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->PwrSupplyChar &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->PwrSupplyChar |= (UINT16)Data->ChangeValue;
                        break;
            case 0x10:  ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->InputVoltProbeHandle &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->InputVoltProbeHandle |= (UINT16)Data->ChangeValue;
                        break;
            case 0x12:  ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->CoolingDevHandle &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->CoolingDevHandle |= (UINT16)Data->ChangeValue;
                        break;
            case 0x14:  ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->InputCurrentProbeHandle &= (UINT16)Data->ChangeMask;
                        ((SMBIOS_SYSTEM_PWR_SUPPY_INFO*)StructPtr)->InputCurrentProbeHandle |= (UINT16)Data->ChangeValue;
                        break;
        }
    }

    return UpdateStrings(Handle, TableInfo, Data->StructureData);
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PnPFn52AddStructure
//
// Description: PnP function 52 Command 03: Add structure
//
// Input:       IN SET_SMBIOS_STRUCTURE_DATA    *dmiDataBuffer
//              IN BOOLEAN                      Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
PnPFn52AddStructure (
    IN SET_SMBIOS_STRUCTURE    *p
)
{
    UINT16  Status;
    UINT8   *SmbiosTable = (UINT8*)((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableAddress;
    UINT8   *SmbiosTableEnd = SmbiosTable + ((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableLength;
    UINT8   *SrcPtr;
    UINT8   *DestPtr;
    UINT8   Type127Buffer[4];
    SET_SMBIOS_STRUCTURE_DATA    *dmiDataBuffer;
    TABLE_INFO  TableInfo;

    dmiDataBuffer = (SET_SMBIOS_STRUCTURE_DATA*)p->Buffer32BitAddr;
    DestPtr = GetStructureByHandle(&dmiDataBuffer->StructureHeader.Handle);

    if (DestPtr) {
        Status = DMI_INVALID_HANDLE;
    }
    else {
        SrcPtr = SmbiosTable;
        if (FindStructureType(&SrcPtr, &DestPtr, 127, 1)) {
            if ((MaximumBufferSize - ((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableLength) >= dmiDataBuffer->DataLength) {
                if (p->Control & 1) {
                    TableInfo.Type = dmiDataBuffer->StructureHeader.Type;
                    TableInfo.Offset = dmiDataBuffer->FieldOffset;
                    TableInfo.Reserved = 0;
                    TableInfo.Flags = DMIEDIT_ADD_STRUC | DMIEDIT_EXTENDED_HDR;
                    TableInfo.HdrLength = sizeof(TABLE_INFO);
                    TableInfo.Size = dmiDataBuffer->DataLength;
                    TableInfo.Handle = dmiDataBuffer->StructureHeader.Handle;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
                    Status = UpdateSmbiosTable(&TableInfo, (UINT8*)&dmiDataBuffer->StructureHeader);
                    if (Status != 0) {
                        return Status;
                    }
#else
{
					EFI_STATUS	Status;

                    Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
							   TableInfo.Type,
							   TableInfo.Handle,
							   TableInfo.Offset,
							   TableInfo.Flags);

					//
					// Get number of DMI data records in NVRAM
					//
					// Note: DMI data record actually starts with record #1,
					//		 first record #0 holds total number of DMI data records
					//       instead of TABLE_INFO
					//       ===> DmiArray[0].Type = count
					//
				    Status = pRS->GetVariable(
				                        DmiArrayVar,
				                        &gAmiSmbiosNvramGuid,
				                        NULL,
				                        &DmiArraySize,
				                        &DmiArray);

					if (Status == EFI_SUCCESS) {
					    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
					    ++Index;
					}
					else {
						Index = 1;
					}

					// Check if record already exists
                    //
                    // DmiDataSize can be anything since the purpose of this GetVariable
                    // call is to detect if the variable already exists or not. Its
                    // content is not used.
                    DmiDataSize = 0;                        // Dummy value
				    Status = pRS->GetVariable(
				                        Var,
				                        &gAmiSmbiosNvramGuid,
				                        NULL,
				                        &DmiDataSize,
				                        &DmiData);

				    if (Status == EFI_NOT_FOUND) {
						// Record not present, increment record count
				        DmiArray[Index].Type = TableInfo.Type;
				        DmiArray[Index].Handle = TableInfo.Handle;
				        DmiArray[Index].Offset = TableInfo.Offset;
				        DmiArray[Index].Flags = TableInfo.Flags;

				        DmiArray[0].Type += 1;          // Increment # variable counter

				        Status = pRS->SetVariable(
				                            DmiArrayVar,
				                            &gAmiSmbiosNvramGuid,
				                            EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
				                            DmiArraySize,
				                            &DmiArray);
				        ASSERT_EFI_ERROR(Status);
                        if (Status != 0) {
                            return (UINT16)Status;
                        }
				    }

					// Update DMI data record if already exists,
					// or store new record if total record count in DmiArray was successfully
					// updated
                    if (Status == EFI_BUFFER_TOO_SMALL || Status == EFI_SUCCESS) {
                        DmiDataSize = (UINTN)dmiDataBuffer->DataLength;
					    Status = pRS->SetVariable(
					                        Var,
					                        &gAmiSmbiosNvramGuid,
					                        EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                            DmiDataSize,
					                        (UINT8*)&dmiDataBuffer->StructureHeader);
					    ASSERT_EFI_ERROR(Status);
					}

                    if (Status != 0) {
                        return (UINT16)Status;
                    }
}
#endif

                    // Copy Type 127
                    MemCpy(&Type127Buffer, DestPtr, 4);
                    MemCpy(DestPtr, (UINT8*)&dmiDataBuffer->StructureHeader, dmiDataBuffer->DataLength);
                    DestPtr = DestPtr + GetStructureLength(DestPtr);
                    MemCpy(DestPtr, &Type127Buffer, 4);

                    // Update SMBIOS Structure Table Entry Point - Structure Table Length, Intermediate checksum
                    UpdateHeaderInfo();
                }

                Status = DMI_SUCCESS;
            }
            else {
                Status = DMI_ADD_STRUCTURE_FAILED;
            }
        }
        else {
            Status = DMI_ADD_STRUCTURE_FAILED;
        }
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PnPFn52DeleteStructure
//
// Description: PnP function 52 Command 04: Delete structure
//
// Input:       IN SET_SMBIOS_STRUCTURE_DATA    *dmiDataBuffer
//              IN BOOLEAN                      Set
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
PnPFn52DeleteStructure (
    IN SET_SMBIOS_STRUCTURE    *p
)
{
    UINT16  Status;
    UINT8   *SmbiosTable = (UINT8*)((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableAddress;
    UINT8   *SmbiosTableEnd = SmbiosTable + ((SMBIOS_TABLE_ENTRY_POINT*)SmbiosTableEntryPoint)->TableLength;
    UINT8   *DestPtr;
    UINT16  i;
    UINT16  RemainingSize;
    SET_SMBIOS_STRUCTURE_DATA    *dmiDataBuffer;
    TABLE_INFO  TableInfo;

    dmiDataBuffer = (SET_SMBIOS_STRUCTURE_DATA*)p->Buffer32BitAddr;
    DestPtr = GetStructureByHandle(&((SET_SMBIOS_STRUCTURE_DATA*)dmiDataBuffer)->StructureHeader.Handle);
    if (DestPtr) {
        if (p->Control & 1) {
            UINT8   *SrcPtr;

            TableInfo.Type = dmiDataBuffer->StructureHeader.Type;
            TableInfo.Offset = 0xff;
            TableInfo.Reserved = 0;
            TableInfo.Flags = DMIEDIT_DELETE_STRUC | DMIEDIT_EXTENDED_HDR;
            TableInfo.HdrLength = sizeof(TABLE_INFO);
            TableInfo.Size = 0;
            TableInfo.Handle = dmiDataBuffer->StructureHeader.Handle;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
            Status = UpdateSmbiosTable(&TableInfo, (UINT8*)&dmiDataBuffer->StructureHeader);
            if (Status != 0) {
                return Status;
            }
#else
{
			EFI_STATUS	Status;

            Swprintf(Var, L"DmiVar%02x%04x%02x%02x",
					   TableInfo.Type,
					   TableInfo.Handle,
					   TableInfo.Offset,
					   TableInfo.Flags);

			//
			// Get number of DMI data records in NVRAM
			//
			// Note: DMI data record actually starts with record #1,
			//		 first record #0 holds total number of DMI data records
			//       instead of TABLE_INFO
			//       ===> DmiArray[0].Type = count
			//
		    Status = pRS->GetVariable(
		                        DmiArrayVar,
		                        &gAmiSmbiosNvramGuid,
		                        NULL,
		                        &DmiArraySize,
		                        &DmiArray);

			if (Status == EFI_SUCCESS) {
			    Index = DmiArray[0].Type;	// Note: DmiArray[0] has count # instead of Type/Offset
			    ++Index;
			}
			else {
				Index = 1;
			}

			// Check if record already exists
            //
            // DmiDataSize can be anything since the purpose of this GetVariable
            // call is to detect if the variable already exists or not. Its
            // content is not used.
            DmiDataSize = 0;                        // Dummy value
		    Status = pRS->GetVariable(
		                        Var,
		                        &gAmiSmbiosNvramGuid,
		                        NULL,
		                        &DmiDataSize,
		                        &DmiData);

		    if (Status == EFI_NOT_FOUND) {
				// Record not present, increment record count
		        DmiArray[Index].Type = TableInfo.Type;
		        DmiArray[Index].Handle = TableInfo.Handle;
		        DmiArray[Index].Offset = TableInfo.Offset;
		        DmiArray[Index].Flags = TableInfo.Flags;

		        DmiArray[0].Type += 1;          // Increment # variable counter

		        Status = pRS->SetVariable(
		                            DmiArrayVar,
		                            &gAmiSmbiosNvramGuid,
		                            EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
		                            DmiArraySize,
		                            &DmiArray);
		        ASSERT_EFI_ERROR(Status);
                if (Status != 0) {
                    return (UINT16)Status;
                }
		    }

			// Update DMI data record if already exists,
			// or store new record if total record count in DmiArray was successfully
			// updated
            if (Status == EFI_BUFFER_TOO_SMALL || Status == EFI_SUCCESS) {
                DmiDataSize = (UINTN)sizeof(TABLE_INFO);
			    Status = pRS->SetVariable(
			                        Var,
			                        &gAmiSmbiosNvramGuid,
			                        EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                    DmiDataSize,
			                        &TableInfo);
			    ASSERT_EFI_ERROR(Status);
			}

            if (Status != 0) {
                return (UINT16)Status;
            }
}
#endif

			// Copy / update the remaining structures in the Smbios Table
            SrcPtr = DestPtr + GetStructureLength(DestPtr);
            RemainingSize = (UINT16)(SmbiosTableEnd - SrcPtr);

            for (i = 0; i < RemainingSize; i++) {
                *DestPtr = *SrcPtr;
                SrcPtr++;
                DestPtr++;
            }

            // Update SMBIOS Structure Table Entry Point - Structure Table Length, Intermediate checksum
            UpdateHeaderInfo();
        }

        Status = DMI_SUCCESS;
    }
    else {
        Status = DMI_INVALID_HANDLE;
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetSmbiosStructure
//
// Description: DMIEdit function to update the structures and saves the
//              DMI data in the Flash Part for subsequent boot.
//
// Input:       IN SET_SMBIOS_STRUCTURE    *p
//
// Output:      UINT8 - Status
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
SetSmbiosStructure(
    IN SET_SMBIOS_STRUCTURE    *p
)
{
    SET_SMBIOS_STRUCTURE_DATA *Data = (SET_SMBIOS_STRUCTURE_DATA *)p->Buffer32BitAddr;
    UINT8                     *SmbTable;
    UINT16                    Handle = Data->StructureHeader.Handle;

    if (!SmbiosTableEntryPoint) return DMI_FUNCTION_NOT_SUPPORTED;

    if (Data->Command == 3) {           // Add structure
        return PnPFn52AddStructure(p);
    }

    if (Data->Command == 4) {           // Delete structure
        return PnPFn52DeleteStructure(p);
    }

    SmbTable = GetStructureByHandle(&Handle);
    if (!SmbTable) return DMI_INVALID_HANDLE;

    // Verify header
    if (*(UINT16*)&Data->StructureHeader != *(UINT16*)SmbTable) return DMI_BAD_PARAMETER;

    // Currently only accept certain table types;
    switch (Data->StructureHeader.Type) {
        case 0:
                return SetType0(Handle, Data, p->Control&1);
        case 1:
                return SetType1(Handle, Data, p->Control&1);
#if BASE_BOARD_INFO
        case 2:
                return SetType2(Handle, Data, p->Control&1);
#endif
#if SYS_CHASSIS_INFO
        case 3:
                return SetType3(Handle, Data, p->Control&1);
#endif
#if PROCESSOR_DMIEDIT_SUPPORT
        case 4:
                return SetType4(Handle, Data, p->Control&1);
#endif
#if OEM_STRING_INFO
        case 11:
                return SetType11(Handle, Data, p->Control&1);
#endif
#if SYSTEM_CONFIG_OPTION_INFO
        case 12:
                return SetType12(Handle, Data, p->Control&1);
#endif
#if PORTABLE_BATTERY_INFO
        case 22:
                return SetType22(Handle, Data, p->Control&1);
#endif
#if SYSTEM_POWER_SUPPLY_INFO
        case 39:
                return SetType39(Handle, Data, p->Control&1);
#endif
    }
    return DMI_BAD_PARAMETER;
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
