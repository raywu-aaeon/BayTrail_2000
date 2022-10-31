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
// $Header: /Alaska/SOURCE/Modules/SMBIOS/SmbiosDMIEditSupport/SmbiosDMIEdit.c 21    6/06/12 3:07p Davidd $
//
// $Revision: 21 $
//
// $Date: 6/06/12 3:07p $
//**********************************************************************//
//**********************************************************************//

#include <AmiDxeLib.h>
#include <Token.h>
#include <AmiHobs.h>
#include <Smm.h>
#include <AmiSmm.h>
#include <Protocol\SmmSwDispatch2.h>
#include <Protocol\LoadedImage.h>
#include <Protocol\SmmBase2.h>
#include <Protocol\SmmCpu.h>
#include <Protocol\AmiSmbios.h>
#include "SmbiosDmiEdit.h"

#define FLASH_DEVICE_BASE (0xFFFFFFFF - FLASH_SIZE + 1)

EFI_GUID						gSwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;

EFI_SMM_SYSTEM_TABLE2			*mSmst;
EFI_PHYSICAL_ADDRESS            TsegStart = 0;
EFI_PHYSICAL_ADDRESS            TsegEnd = 0;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || (SMBIOS_DMIEDIT_DATA_LOC != 2)
#include <Protocol\SmbiosGetFlashDataProtocol.h>
#include <Protocol\SmiFlash.h>

extern EFI_GUID gEfiSmiFlashProtocolGuid;

EFI_SMBIOS_FLASH_DATA_PROTOCOL  *mSmbiosFlashDataProtocol;
EFI_SMI_FLASH_PROTOCOL          *mSmiFlash;
#endif                                  // SMBIOS_DMIEDIT_DATA_LOC

//----------------------------------------------------------------------------
//  External Variables
//----------------------------------------------------------------------------
extern  SMBIOS_TABLE_ENTRY_POINT    *SmbiosTableEntryPoint;
extern  UINT8                       *ScratchBufferPtr;
extern  UINT16                      MaximumBufferSize;

//----------------------------------------------------------------------------
//  External Function Declaration
//----------------------------------------------------------------------------
extern VOID EnableShadowWrite();
extern VOID DisableShadowWrite();
extern VOID GetSmbiosTableF000 (VOID);
extern VOID WriteOnceStatusInit(VOID);

extern UINT16 GetSmbiosInfo(
    IN OUT  GET_SMBIOS_INFO   *p
);

extern UINT16 GetSmbiosStructure(
    IN OUT  GET_SMBIOS_STRUCTURE    *p
);

extern UINT16 SetSmbiosStructure(
    IN SET_SMBIOS_STRUCTURE    *p
);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || (SMBIOS_DMIEDIT_DATA_LOC != 2)
extern FLASH_DATA_INFO GetFlashDataInfo(
    IN TABLE_INFO   *RecordInfo
);
#endif

//----------------------------------------------------------------------------
//  Function Declaration
//----------------------------------------------------------------------------
EFI_STATUS
InSmmFunction(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
);

EFI_STATUS
SmiHandler (
    IN  EFI_HANDLE  DispatchHandle,
    IN  CONST VOID	*DispatchContext,
    IN  OUT	VOID	*CommBuffer,
    IN  OUT	UINTN	*CommBufferSize
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmbiosDmiEditSupportInstall
//
// Description: DMIEdit support driver entry point
//
// Input:       IN EFI_HANDLE           ImageHandle,
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
SmbiosDmiEditSupportInstall(
    IN EFI_HANDLE           ImageHandle,
	IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS              Status;
    CPUINFO_HOB             *CpuInfoHob = NULL;
    EFI_GUID                CpuInfoHobGuid = AMI_CPUINFO_HOB_GUID;
    EFI_GUID                HobListGuid = HOB_LIST_GUID;

	InitAmiLib(ImageHandle, SystemTable);

    CpuInfoHob = (CPUINFO_HOB*)GetEfiConfigurationTable(pST, &HobListGuid);
    if (CpuInfoHob != NULL) {
        Status = FindNextHobByGuid(&CpuInfoHobGuid,(VOID**)&CpuInfoHob);
        if (Status == EFI_SUCCESS) {
            TsegStart = CpuInfoHob->TsegAddress;
            TsegEnd = CpuInfoHob->TsegAddress + CpuInfoHob->TsegSize;
        }
    }

    return InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InSmmFunction
//
// Description: Initialize pointers and register SW SMI handlers for
//              DMIEdit support.
//
// Input:       IN EFI_HANDLE           ImageHandle,
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
InSmmFunction(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS                      Status;
    UINTN                           Index;
    EFI_HANDLE                      SwHandle  = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT     SwContext;
    EFI_SMM_SW_DISPATCH2_PROTOCOL   *SwDispatch;
    AMI_SMBIOS_PROTOCOL             *SmbiosProtocol;
    EFI_SMM_BASE2_PROTOCOL          *SmmBase;

    Status = pBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, &SmmBase);
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    SmmBase->GetSmstLocation (SmmBase, &mSmst);   // Save the system table pointer

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
    Status = pBS->LocateProtocol (&gAmiSmbiosFlashDataProtocolGuid, NULL, &mSmbiosFlashDataProtocol);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    mSmbiosFlashDataProtocol->GetFlashTableInfo(mSmbiosFlashDataProtocol, &gFlashData, &gFlashDataSize);

    Status = mSmst->SmmLocateProtocol (&gEfiSmiFlashProtocolGuid, NULL, &mSmiFlash);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) {
        return Status;
    }
#endif

    WriteOnceStatusInit();

    Status = pBS->LocateProtocol (&gAmiSmbiosProtocolGuid, NULL, &SmbiosProtocol);
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    SmbiosTableEntryPoint = SmbiosProtocol->SmbiosGetTableEntryPoint();
    ScratchBufferPtr = SmbiosProtocol->SmbiosGetScratchBufferPtr();
	MaximumBufferSize = SmbiosProtocol->SmbiosGetBufferMaxSize();

    // Register the SW SMI handler
    Status = mSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, &SwDispatch);
    ASSERT_EFI_ERROR(Status);

    for(Index = 0x50; Index <= 0x52; Index++) {
        SwContext.SwSmiInputValue = Index;
        Status = SwDispatch->Register (SwDispatch, SmiHandler, &SwContext, &SwHandle);

        if (EFI_ERROR (Status)) {
            return Status;
        }
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckAddress
//
// Description: Check address to avoid TSEG area.
//
// Input:
//  Address     - starting address
//  Function    - DMIEdit function
//
// Output:
//  EFI_SUCCESS         - Access granted
//  DMI_BAD_PARAMETER   - Access denied!
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
CheckAddress (
    IN UINT8 *Address,
    IN UINT8 Function
)
{
    UINT16  Status;

    Status = EFI_SUCCESS;

    if (TsegStart != 0) {
        //
        // Check address against TSEG
        //
        if ((EFI_PHYSICAL_ADDRESS)Address >= TsegStart &&
            (EFI_PHYSICAL_ADDRESS)Address <= TsegEnd) {
            Status = DMI_BAD_PARAMETER;
        }
        else {
            switch(Function) {
                case 0x50:  {
                                if ((EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->DmiBiosRevision32BitAddr) >= TsegStart &&
                                    (EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->DmiBiosRevision32BitAddr) <= TsegEnd) {
                                    Status = DMI_BAD_PARAMETER;
                                }
                                if ((EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->NumStructures32BitAddr) >= TsegStart &&
                                    (EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->NumStructures32BitAddr) <= TsegEnd) {
                                    Status = DMI_BAD_PARAMETER;
                                }
                                if ((EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->StructureSize32BitAddr) >= TsegStart &&
                                    (EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->StructureSize32BitAddr) <= TsegEnd) {
                                    Status = DMI_BAD_PARAMETER;
                                }
                                if ((EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->DmiStorageBase32BitAddr) >= TsegStart &&
                                    (EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->DmiStorageBase32BitAddr) <= TsegEnd) {
                                    Status = DMI_BAD_PARAMETER;
                                }
                                if ((EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->DmiStorageSize32BitAddr) >= TsegStart &&
                                    (EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_INFO*)Address)->DmiStorageSize32BitAddr) <= TsegEnd) {
                                    Status = DMI_BAD_PARAMETER;
                                }
                                break;
                            }
                case 0x51:
                case 0x52:  {
                                if ((EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_STRUCTURE*)Address)->Buffer32BitAddr) >= TsegStart &&
                                    (EFI_PHYSICAL_ADDRESS)(((GET_SMBIOS_STRUCTURE*)Address)->Buffer32BitAddr) <= TsegEnd) {
                                    Status = DMI_BAD_PARAMETER;
                                }
                            }
            }
        }
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmiHandler
//
// Description: Handles the SMI
//
// Input:       IN EFI_HANDLE                    DispatchHandle
//              IN EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
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
EFI_STATUS
SmiHandler (
    IN  EFI_HANDLE  DispatchHandle,
    IN  CONST VOID	*DispatchContext,
    IN  OUT	VOID	*CommBuffer,
    IN  OUT	UINTN	*CommBufferSize
)
{
    UINT8                   Data;
    VOID                    *pInterface;
    UINT16                  RetStatus;
    UINTN                   Cpu;
    EFI_GUID                SmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
    EFI_SMM_CPU_PROTOCOL    *SmmCpuProtocol;
    EFI_SMM_SW_CONTEXT      *SwContext = (EFI_SMM_SW_CONTEXT*)CommBuffer;

    GetSmbiosTableF000();

 	Cpu = SwContext->SwSmiCpuIndex;

    mSmst->SmmLocateProtocol(
                        &SmmCpuProtocolGuid,
                        NULL,
                        &SmmCpuProtocol
                        );

    pInterface = 0;         // Clear upper 64-bits.
    SmmCpuProtocol->ReadSaveState(
                            SmmCpuProtocol,
                            4,
                            EFI_SMM_SAVE_STATE_REGISTER_RBX,
                            Cpu,
                            &pInterface
                            );

    Data = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->CommandPort;

    RetStatus = CheckAddress(pInterface, Data);

    if (RetStatus == EFI_SUCCESS) {
        switch(Data) {
            case 0x50:
                        RetStatus = GetSmbiosInfo(pInterface);
                        break;
            case 0x51:
                        RetStatus = GetSmbiosStructure(pInterface);
                        break;
            case 0x52:
                        EnableShadowWrite();
                        RetStatus = SetSmbiosStructure(pInterface);
                        DisableShadowWrite();
        }
    }

    SmmCpuProtocol->WriteSaveState(
                            SmmCpuProtocol,
                            2,
                            EFI_SMM_SAVE_STATE_REGISTER_RAX,
                            Cpu,
                            &RetStatus
                            );

    return EFI_SUCCESS;
}

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WriteToFlash
//
// Description: Write to the flash part starting at "Address" for a length
//              of "Size".
//
// Input:       IN VOID    *Address,
//              IN VOID    *Data,
//              IN UINTN   Size
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
WriteToFlash(
    IN VOID    *Address,
    IN VOID    *Data,
    IN UINTN   Size
)
{
	EFI_STATUS	Status;
	FUNC_BLOCK	FuncBlock;

	Status = mSmiFlash->EnableFlashWrite(&FuncBlock);
	if (EFI_ERROR(Status)) return Status;

    FuncBlock.BlockAddr = (UINT32)Address - (0xffffffff - FLASH_SIZE + 1);
	FuncBlock.BlockSize = (UINT32)Size;
	FuncBlock.BufAddr = (UINT64)Data;
	Status = mSmiFlash->WriteFlash(&FuncBlock);
	mSmiFlash->DisableFlashWrite(&FuncBlock);
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateSmbiosTable
//
// Description: Searches the Flash Data Table for a record of Type and
//              Offset. If found, the existing data will be replaced with
//              the new data, else the data will be added as a new record.
//
// Input:       IN TABLE_INFO  TableInfo,
//              IN UINT8       *Data
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
UINT16
UpdateSmbiosTable(
    IN TABLE_INFO  *TableInfo,
    IN UINT8       *Data
)
{
    UINT16              	i;
    UINT8                   *BufferPtr = NULL;
    UINT16               	BufferSize = 0;
    UINT16               	Count = 0;
    UINT32              	SpaceAvailable;
    EFI_STATUS          	Status;
    FLASH_DATA_INFO     	FlashDataInfo;
    UINT8               	*FlashDataPtr;
    FUNC_BLOCK          	FuncBlock[2];
    EFI_PHYSICAL_ADDRESS    SmmBuffer;
    EFI_PHYSICAL_ADDRESS    Buffer;

    FlashDataInfo = GetFlashDataInfo(TableInfo);

    // Check Size
    SpaceAvailable = (UINT32)((UINT8*)gFlashData + gFlashDataSize - FlashDataInfo.EndOfData);
    if (FlashDataInfo.Location) SpaceAvailable += FlashDataInfo.Size + sizeof(TABLE_INFO);

    if (sizeof(TABLE_INFO) + TableInfo->Size > SpaceAvailable) {
        return DMI_ADD_STRUCTURE_FAILED;
    }

    // Initialize FuncBlock
    for (i = 0; i < 2; i++) {
        FuncBlock[i].BufAddr = 0;
        FuncBlock[i].BlockAddr = 0;
        FuncBlock[i].BlockSize = 0;
        FuncBlock[i].ErrorCode = 0;
    }

    // Allocate 4K working buffer in SMM.
    Status = mSmst->SmmAllocatePages ( AllocateAnyPages, EfiRuntimeServicesData, 1, &Buffer);
    if (EFI_ERROR(Status)) return DMI_ADD_STRUCTURE_FAILED;
    BufferPtr = (UINT8*)Buffer;

    // Update String;
    *(TABLE_INFO *)BufferPtr = *TableInfo;
    BufferPtr += sizeof(TABLE_INFO);

    for(i = 0; i < TableInfo->Size; ++i) {
        *BufferPtr++ = Data[i];
    }

    if (FlashDataInfo.Location) {
        UINT32              	FlashDataOffset;

        // Allocate 64K GetFlashInfo buffer in SMM.
        Status = mSmst->SmmAllocatePages ( AllocateAnyPages, \
                                           EfiRuntimeServicesData, \
                                           16, \
                                           &SmmBuffer);
        if (EFI_ERROR(Status)) {
            // Free buffer and return error.
            mSmst->SmmFreePages (Buffer, 1);
            return DMI_ADD_STRUCTURE_FAILED;
        }

        ((INFO_BLOCK*)SmmBuffer)->Length = 0x10000;

        Status = mSmiFlash->GetFlashInfo((INFO_BLOCK*)SmmBuffer);

        if (Status) {
            // Free buffers and return error.
            mSmst->SmmFreePages (Buffer, 1);
            mSmst->SmmFreePages (SmmBuffer, 16);
            return DMI_ADD_STRUCTURE_FAILED;
        }

        // Initialize FUNC_BLOCK structure for SMIFlash used.
        for (i = 0, Count = 1; i < ((INFO_BLOCK*)SmmBuffer)->TotalBlocks; i++) {
            if (((UINT32)FlashDataInfo.Location - FLASH_DEVICE_BASE) > \
                    (((INFO_BLOCK*)SmmBuffer)->Blocks[i].StartAddress + \
                     ((INFO_BLOCK*)SmmBuffer)->Blocks[i].BlockSize)) continue;
            FuncBlock[0].BlockSize = \
                            ((INFO_BLOCK*)SmmBuffer)->Blocks[i].BlockSize;
            FuncBlock[0].BlockAddr = \
                            ((INFO_BLOCK*)SmmBuffer)->Blocks[i].StartAddress;

            // Check whether SmbiosFlashData exceeds the block boundary.
            if (((UINT32)gFlashData + (UINT32)FLASHDATA_SIZE - FLASH_DEVICE_BASE) > \
                    (((INFO_BLOCK*)SmmBuffer)->Blocks[i+1].StartAddress)) {
                Count = 2;
                FuncBlock[1].BlockSize = \
                            ((INFO_BLOCK*)SmmBuffer)->Blocks[i+1].BlockSize;
                FuncBlock[1].BlockAddr = \
                            ((INFO_BLOCK*)SmmBuffer)->Blocks[i+1].StartAddress;
            }
            break;
        }

        // Free the GetFlashInfo buffer.
        Status = mSmst->SmmFreePages (SmmBuffer, 16);
        ASSERT_EFI_ERROR(Status);

        // Allocate the blocks buffer.
        Status = mSmst->SmmAllocatePages ( \
                            AllocateAnyPages, \
                            EfiRuntimeServicesData, \
                            (FuncBlock[0].BlockSize * Count) / 0x1000, \
                            &SmmBuffer);
        if (EFI_ERROR(Status)) {
            // Free buffer and return error.
            mSmst->SmmFreePages (Buffer, 1);
            return DMI_ADD_STRUCTURE_FAILED;
        }
        FuncBlock[0].BufAddr = SmmBuffer;
            FuncBlock[1].BufAddr = SmmBuffer + FuncBlock[0].BlockSize;

        // Read the whole SmbiosFlashData Blocks.
        for (i = 0; i < Count; i++) {
            Status = mSmiFlash->ReadFlash(&FuncBlock[i]);
            if (Status) {
                // Free buffer and return error.
                mSmst->SmmFreePages (Buffer, 1);

                mSmst->SmmFreePages ( \
                            SmmBuffer, \
                            (FuncBlock[0].BlockSize * Count) / 0x1000);
                return DMI_ADD_STRUCTURE_FAILED;
            }
        }

        // Initialize SmbiosFlashData buffer.
        for (i = 0; i < FLASHDATA_SIZE; i++, *((UINT8*)Buffer + i) = 0xff);

        // Re-collect the Smbios structures to SmbiosFlashData buffer.
        FlashDataPtr = gFlashData;
        BufferPtr = (UINT8*)Buffer;

        while((((TABLE_INFO*)FlashDataPtr)->Size != 0xffff) &&
              (((TABLE_INFO*)FlashDataPtr)->Size != 0)) {
            if ((((TABLE_INFO*)FlashDataPtr)->Type == TableInfo->Type) && \
                (((TABLE_INFO*)FlashDataPtr)->Handle == TableInfo->Handle) && \
                (((TABLE_INFO*)FlashDataPtr)->Offset == TableInfo->Offset)) {
                // Replace the structure with updated data.
                MemCpy(BufferPtr, (UINT8*)TableInfo, sizeof(TABLE_INFO));
                BufferSize = TableInfo->Size;
                MemCpy (BufferPtr + sizeof(TABLE_INFO), Data, BufferSize);
                BufferSize += sizeof(TABLE_INFO);
            } else {
                // Copy the structure.
                BufferSize = (((TABLE_INFO*)FlashDataPtr)->Size + sizeof(TABLE_INFO));
                MemCpy (BufferPtr, FlashDataPtr, BufferSize);
            }

            BufferPtr += BufferSize;
            FlashDataPtr += (((TABLE_INFO*)FlashDataPtr)->Size + sizeof(TABLE_INFO));
        }

        // Copy the new SmbiosFlashData to read buffer.
        FlashDataOffset = ((UINT32)FlashDataInfo.Location - \
                                    FLASH_DEVICE_BASE - FuncBlock[0].BlockAddr);
        BufferPtr = (UINT8*)Buffer + (UINT32)FlashDataInfo.Location - (UINT32)gFlashData;
        MemCpy((UINT8*)(FuncBlock[0].BufAddr + FlashDataOffset),
                       (UINT8*)BufferPtr,
                       (UINT32)gFlashData + (UINT32)FLASHDATA_SIZE - (UINT32)FlashDataInfo.Location);

        // Write the block buffer with updated SmbiosFlashData back.
        Status = mSmiFlash->EnableFlashWrite(&FuncBlock[0]);

        if (!EFI_ERROR(Status)) {
            for (i = 0; i < Count; i++) {
                // Erase Flash
                Status = mSmiFlash->EraseFlash(&FuncBlock[i]);
                if (Status) break;

                // Write Flash
                Status = mSmiFlash->WriteFlash(&FuncBlock[i]);
                if (Status) break;
            }

			mSmiFlash->DisableFlashWrite(&FuncBlock[0]);
		}

        // Free the Block Buffer in SMM.
		mSmst->SmmFreePages ( SmmBuffer, \
							  (FuncBlock[0].BlockSize * Count) / 0x1000);
    }
    else {
        UINT32 EndOfData;

        EndOfData = (UINT32)FlashDataInfo.EndOfData & 0x0ffff;

        if ((EndOfData + (UINT32)(BufferPtr - (UINT8*)Buffer)) > 0x10000) {
            UINT32 NewOffestOfData;
            UINT32 ExtraSize;
            UINT32 DataLength;

            NewOffestOfData = (UINT32)(((UINT32)FlashDataInfo.EndOfData & 0xffff0000) + 0x10000);
            ExtraSize = EndOfData + (UINT32)(BufferPtr - (UINT8*)Buffer) - 0x10000;
            DataLength = (UINT32)(BufferPtr - (UINT8*)Buffer);

            Status = WriteToFlash(FlashDataInfo.EndOfData,
                                (UINT8*)Buffer,
                                DataLength - ExtraSize);
            ASSERT_EFI_ERROR(Status);

            Status = WriteToFlash( (VOID *)NewOffestOfData,
                                (UINT8*)(Buffer + DataLength - ExtraSize),
                                ExtraSize);
        }
        else {
            Status = WriteToFlash(FlashDataInfo.EndOfData,
                                (UINT8*)Buffer,
                                BufferPtr - (UINT8*)Buffer);
        }

        mSmst->SmmFreePages (Buffer, 1);

        if (Status) return DMI_ADD_STRUCTURE_FAILED;
    }

    return 0;
}
#endif							// SMBIOS_DMIEDIT_DATA_LOC

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
