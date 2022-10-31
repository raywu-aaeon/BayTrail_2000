//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SMIFlash.c
//
// Description: SMIFlash Driver.
//
//<AMI_FHDR_END>
//**********************************************************************
#include <AmiDxeLib.h>
//#include <Library\Debuglib.h>
#include <Token.h>
#include <Protocol\SmiFlash.h>
#include <SMIFlashELinks.h>
#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol\SmmCpu.h>
#include <Protocol\SmmBase2.h>
#include <Protocol\SmmSwDispatch2.h>
#define RETURN(status) {return status;}
#else
#include <Protocol\SmmBase.h>
#include <Protocol\SmmSwDispatch.h>
#define RETURN(status) {return ;}
#endif
#include <Protocol\DevicePath.h>
#include <Protocol\LoadedImage.h>
#include <Protocol\FlashProtocol.h>
#include <Flash.h>
#include <AmiSmm.h>
#include <Setup.h>
#include <RomLayout.h>
#include <Protocol\FirmwareVolume2.h>
#include <Protocol\SmmAccess2.h>
#define ROM_LAYOUT_FFS_GUID \
    { 0x0DCA793A, 0xEA96, 0x42d8, 0xBD, 0x7B, 0xDC, 0x7F, 0x68, 0x4E, 0x38, 0xC1 }

//----------------------------------------------------------------------
// component MACROs
#define FLASH_EMPTY_BYTE (UINT8)(-FLASH_ERASE_POLARITY)
#define STR(a) CONVERT_TO_STRING(a)
//----------------------------------------------------------------------
// Module defined global variables

//----------------------------------------------------------------------
// Module specific global variable
EFI_GUID gSwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;
#if PI_SPECIFICATION_VERSION >= 0x1000A
//EFI_GUID gEfiSmmSwDispatchProtocolGuid = EFI_SMM_SW_DISPATCH2_PROTOCOL_GUID;
//EFI_GUID gEfiSmmBase2ProtocolGuid = EFI_SMM_BASE2_PROTOCOL_GUID;
EFI_GUID gEfiSmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
EFI_SMM_CPU_PROTOCOL            *gSmmCpu;
#else
EFI_GUID gEfiSmmSwDispatchProtocolGuid = EFI_SMM_SW_DISPATCH_PROTOCOL_GUID;
#endif
EFI_GUID gEfiSmiFlashProtocolGuid = EFI_SMI_FLASH_GUID;


// oem flash write enable/disable list creation code must be in this order
typedef VOID (SMIFLASH_UPDATE) (VOID);
extern SMIFLASH_UPDATE SMIFLASH_PRE_UPDATE_LIST EndOfSMIFlashList;
extern SMIFLASH_UPDATE SMIFLASH_END_UPDATE_LIST EndOfSMIFlashList;
SMIFLASH_UPDATE* SMIFlashPreUpdate[] = {SMIFLASH_PRE_UPDATE_LIST NULL};
SMIFLASH_UPDATE* SMIFlashEndUpdate[] = {SMIFLASH_END_UPDATE_LIST NULL};

typedef VOID (SMIFLASH_HANDLER_HOOK) (UINT8 Date, UINT64 pInfoBlock);
extern SMIFLASH_HANDLER_HOOK SMIFLASH_PRE_HANDLER_LIST EndOfPreHandlerList;
extern SMIFLASH_HANDLER_HOOK SMIFLASH_END_HANDLER_LIST EndOfEndHandlerList;
SMIFLASH_HANDLER_HOOK* SMIFlashPreHandler[] = {SMIFLASH_PRE_HANDLER_LIST NULL};
SMIFLASH_HANDLER_HOOK* SMIFlashEndHandler[] = {SMIFLASH_END_HANDLER_LIST NULL};

typedef VOID (SMIFLASH_INIT) (VOID);
extern SMIFLASH_INIT SMIFLASH_IN_SMM_LIST EndOfInSmmList;
extern SMIFLASH_INIT SMIFLASH_NOT_IN_SMM_LIST EndOfNotInSmmList;
SMIFLASH_INIT* SMIFlashInSmm[] = {SMIFLASH_IN_SMM_LIST NULL};

FLASH_PROTOCOL *Flash = NULL;
#ifdef FLASH_PART_STRING_LENGTH
VOID GetFlashPartInfomation(UINT8 *pBlockAddress, UINT8 *Buffer);
#endif
static EFI_GUID RomLayoutFfsGuid = ROM_LAYOUT_FFS_GUID;
ROM_AREA *RomLayout;
UINTN    NumberOfRomLayout = 0;
UINTN    NumberOfNcb = 0;
static EFI_SMRAM_DESCRIPTOR *mSmramRanges;
static UINTN                mSmramRangeCount;
//----------------------------------------------------------------------
// externally defined variables

//----------------------------------------------------------------------
// Function definitions
EFI_STATUS
GetFlashInfo(
    IN OUT INFO_BLOCK   *pInfoBlock
);
EFI_STATUS EnableFlashWrite(
    IN OUT FUNC_BLOCK   *pFuncBlock
);
EFI_STATUS DisableFlashWrite(
    IN OUT FUNC_BLOCK   *pFuncBlock
);
EFI_STATUS ReadFlash(
    IN OUT FUNC_BLOCK   *pFuncBlock
);
EFI_STATUS WriteFlash(
    IN OUT FUNC_BLOCK   *pFuncBlock
);
EFI_STATUS EraseFlash(
    IN OUT FUNC_BLOCK   *pFuncBlock
);
EFI_SMI_FLASH_PROTOCOL  SmiFlash = {
    GetFlashInfo,
    EnableFlashWrite,
    DisableFlashWrite,
    ReadFlash,
    WriteFlash,
    EraseFlash,
    FLASH_SIZE
};
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: CheckAddressRange
//
// Description: Check address range to avoid TSEG area.
//
// Input: 
//  Address - starting address
//  Range   - length of the area
//
// Output: 
//  EFI_SUCCESS         - Access granted
//  EFI_ACCESS_DENIED   - Access denied!
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CheckAddressRange(
    IN UINT8 *Address,
    IN UINTN Range
)
{
    UINT8   Count = 0;
    EFI_PHYSICAL_ADDRESS    TsegStart;
    EFI_PHYSICAL_ADDRESS    TsegEnd;
    // Check the size and range
    for( Count = 0; Count < mSmramRangeCount; Count++ )
    {
        TsegStart = mSmramRanges[Count].CpuStart;
        TsegEnd = mSmramRanges[Count].CpuStart + mSmramRanges[Count].PhysicalSize;
        if( ((EFI_PHYSICAL_ADDRESS)Address >= TsegStart) &&
            ((EFI_PHYSICAL_ADDRESS)Address < TsegEnd) )
            return EFI_ACCESS_DENIED;
    
        if( ((EFI_PHYSICAL_ADDRESS)Address < TsegStart) &&
            (((EFI_PHYSICAL_ADDRESS)Address + Range) > TsegStart) )
            return EFI_ACCESS_DENIED;
    }
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   GetFlashInfo
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetFlashInfo(
    IN OUT INFO_BLOCK   *pInfoBlock
)
{
    EFI_STATUS      Status = EFI_SUCCESS;
    UINT32          BuffLen = 0;
    UINT64          BlkOff;
    BLOCK_DESC      *pBlock;
    UINT16          Index;
    UINT32          Add;
    UINT32          Flash4GBMapStart;
    UINT16          NumBlocks;
    UINT16          LastNcb = 0xFFFF;
    UINT8           NcbType = NC_BLOCK;
    UINT8           i;
	UINT8           InRomLayout;
	UINT16          *StartBlock, *EndBlock;

    IoWrite8 (0x80,0x25);
    BuffLen = pInfoBlock->Length;       // Total buffer length
    pInfoBlock->Version = SMI_FLASH_INTERFACE_VERSION;
    pInfoBlock->Implemented = 0;
#if SMI_FLASH_INTERFACE_VERSION > 10
#if EC_FIRMWARE_UPDATE_INTERFACE_SUPPORT
    pInfoBlock->ECVersionOffset = EC_VERSION_OFFSET;
    pInfoBlock->ECVersionMask = EC_VERSION_MASK;
#else
    pInfoBlock->ECVersionOffset = 0;
    pInfoBlock->ECVersionMask = 0;
#endif
#endif
    
    // Reduce number of bytes used for header information
    BuffLen -= EFI_FIELD_OFFSET( INFO_BLOCK, Blocks);
    
    // Get the total block count
    pInfoBlock->TotalBlocks = NumBlocks = NUMBER_OF_BLOCKS;
    
    // Calculate the flash mapping start address. This is calculated
    // as follows:
    //  1. Find the total size of the flash (FLASH_BLOCK_SIZE *
    //      pInfoBlock->TotalBlocks)
    //  2. Subtract the total flash size from 4GB
    Flash4GBMapStart = 0xFFFFFFFF - (FLASH_BLOCK_SIZE * NumBlocks);
    Flash4GBMapStart ++;
    
    BlkOff = (UINT64)&pInfoBlock->Blocks;
    
    // Chek whether we have enough buffer space
    if (BuffLen < (sizeof (BLOCK_DESC) * NumBlocks)) {
        pInfoBlock->Implemented = 1;
        return Status;
    }
    
	//Calculate RomLayout's start ant end block
	Status = pSmst->SmmAllocatePool (
						EfiRuntimeServicesData,
						NumberOfRomLayout * sizeof(UINT16),
						(VOID**)&StartBlock );
	if (EFI_ERROR(Status)) return Status;
	Status = pSmst->SmmAllocatePool (
						EfiRuntimeServicesData,
						NumberOfRomLayout * sizeof(UINT16),
						(VOID**)&EndBlock );
	if (EFI_ERROR(Status)) return Status;
	for( i = 0; i < NumberOfRomLayout; i++ )
	{
		StartBlock[i] = RomLayout[i].Offset / FLASH_BLOCK_SIZE;
		EndBlock[i] = StartBlock[i] + (RomLayout[i].Size / FLASH_BLOCK_SIZE) - 1;
	}
	
	//Assign types
	NcbType =  NcbType + (UINT8)NumberOfNcb - 1;
    for (Index = 0; Index < NumBlocks; Index++) {
    
        pBlock = (BLOCK_DESC *)BlkOff;
        Add = Index * FLASH_BLOCK_SIZE;
        
        pBlock->StartAddress = Add;
        Add |= Flash4GBMapStart;
        pBlock->BlockSize = FLASH_BLOCK_SIZE;
		
		InRomLayout = 0;
		for( i = 0; i < NumberOfRomLayout; i++ )
		{
			if( Index >= StartBlock[i] && Index <= EndBlock[i] )
			{
				pBlock->Type = RomLayout[i].Type;
				InRomLayout = 1;
				break;
			}
		}
			
		if ( InRomLayout == 0 )
		{
			if (LastNcb+1 != Index) NcbType++;
			pBlock->Type = NcbType;
			LastNcb=Index;
		}
        BlkOff += sizeof (BLOCK_DESC);
    }
	
	pSmst->SmmFreePool(StartBlock);
	pSmst->SmmFreePool(EndBlock);

    // Info AFU Project Tag length.
    if (((UINT64)pInfoBlock + pInfoBlock->Length) > (BlkOff + 5))
    {
        CHAR8*    ProjectTag = STR(PROJECT_TAG);
        UINTN     TagLength;
        UINT8     ProjectTagSign[4] = {'$','B','P','T'};

        TagLength = Strlen(ProjectTag);

        MemCpy ( (UINT8*)BlkOff, ProjectTagSign, 4 );
        BlkOff += sizeof (UINT32);
        *(UINT8*)BlkOff = (UINT8)TagLength;
        BlkOff += 1;
        // Added for AFU to identify the Core Version.
        *(UINT8*)BlkOff = 'V';
        BlkOff += 1;
    }

#ifdef FLASH_PART_STRING_LENGTH
    if (((UINT64)pInfoBlock + pInfoBlock->Length) > \
            (BlkOff + FLASH_PART_STRING_LENGTH + 8))
        GetFlashPartInfomation ( (UINT8*)Flash4GBMapStart, (UINT8*)BlkOff );
#endif
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   EnableFlashWrite
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EnableFlashWrite(
    IN OUT FUNC_BLOCK    *pFuncBlock
)
{
    return Flash->DeviceWriteEnable();
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   DisableFlashWrite
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DisableFlashWrite(
    IN OUT FUNC_BLOCK    *pFuncBlock
)
{
    return Flash->DeviceWriteDisable();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   EraseFlash
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EraseFlash(
    IN OUT FUNC_BLOCK   *pFuncBlock
)
{
    EFI_STATUS    Status;
    UINT8         *BlockAddress;
    BlockAddress =
        (UINT8*)(UINTN)(pFuncBlock->BlockAddr +
                        (0xFFFFFFFF - SmiFlash.FlashCapacity + 1));
    (UINT32)BlockAddress &= (0xFFFFFFFF - FLASH_BLOCK_SIZE + 1);

    // Checking access rang for avoiding the privilege escalation into SMM 
    // via Smiflash driver.
    Status = CheckAddressRange( BlockAddress, FlashBlockSize );
    if(EFI_ERROR(Status)) return Status;

    Status = Flash->Erase(BlockAddress, FlashBlockSize);
    pFuncBlock->ErrorCode = EFI_ERROR(Status) != 0;
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   ReadFlash
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadFlash(
    IN OUT FUNC_BLOCK   *pFuncBlock
)
{
    UINT32        Flash4GBMapStart;
    EFI_STATUS    Status;
    
    Flash4GBMapStart = 0xFFFFFFFF - (FLASH_BLOCK_SIZE * NUMBER_OF_BLOCKS);
    Flash4GBMapStart ++;
    
    // Checking access rang for avoiding the privilege escalation into SMM 
    // via Smiflash driver.
    Status = CheckAddressRange( (UINT8*)(Flash4GBMapStart + pFuncBlock->BlockAddr),
                                pFuncBlock->BlockSize );
    if(EFI_ERROR(Status)) return Status;

    Status = Flash->Read(
                 (UINT8*)(Flash4GBMapStart + pFuncBlock->BlockAddr),
                 pFuncBlock->BlockSize,
                 (UINT8*)pFuncBlock->BufAddr
             );
    pFuncBlock->ErrorCode = EFI_ERROR(Status) != 0;
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   WriteFlash
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS WriteFlash(
    IN OUT FUNC_BLOCK   *pFuncBlock
)
{
    EFI_STATUS      Status;
    UINT8           *FlashAddress;
    
    FlashAddress =
        (UINT8*)(UINTN)(pFuncBlock->BlockAddr +
                        (0xFFFFFFFF - SmiFlash.FlashCapacity + 1));

    // Checking access rang for avoiding the privilege escalation into SMM 
    // via Smiflash driver.
    Status = CheckAddressRange( FlashAddress, pFuncBlock->BlockSize );
    if(EFI_ERROR(Status)) return Status;

    Status = Flash->Write(
                 FlashAddress, pFuncBlock->BlockSize, (UINT8*)pFuncBlock->BufAddr
             );
    pFuncBlock->ErrorCode = EFI_ERROR(Status) != 0;
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SMIFlashSMIHandler
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS 
SMIFlashSMIHandler (
    IN  EFI_HANDLE                  DispatchHandle,
		IN CONST VOID                   *Context OPTIONAL,
		IN OUT VOID                     *CommBuffer OPTIONAL,
		IN OUT UINTN                    *CommBufferSize OPTIONAL
)
#else
VOID SMIFlashSMIHandler (
    IN  EFI_HANDLE                  DispatchHandle,
    IN  EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext
)
#endif
{

    EFI_SMM_CPU_SAVE_STATE  *pCpuSaveState = NULL;
    EFI_STATUS  Status = EFI_SUCCESS;
    UINT8       Data;
    UINT64      pCommBuff;
    UINT32      HighBufferAddress = 0;
    UINT32      LowBufferAddress = 0;
    UINTN       Cpu = (UINTN)-1, i = 0;
#if PI_SPECIFICATION_VERSION < 0x1000A    
    SW_SMI_CPU_TRIGGER      *SwSmiCpuTrigger;
#endif
		
#if PI_SPECIFICATION_VERSION >= 0x1000A

    if (CommBuffer != NULL && CommBufferSize != NULL) {
        Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
        Data = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->CommandPort;
    }
    //
    // Found Invalid CPU number, return 
    //
    if(Cpu == (UINTN)-1) RETURN(Status);

    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RBX, \
                                      Cpu, \
                                      &LowBufferAddress );
    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RCX, \
                                      Cpu, \
                                      &HighBufferAddress );
    	
#else
    for (i = 0; i < pSmst->NumberOfTableEntries; ++i) {
        if (guidcmp(&pSmst->SmmConfigurationTable[i].VendorGuid,
                                    &gSwSmiCpuTriggerGuid) == 0) {
            break;
        }
    }
    
    //If found table, check for the CPU that caused the software Smi.
    if (i != pSmst->NumberOfTableEntries) {
        SwSmiCpuTrigger = pSmst->SmmConfigurationTable[i].VendorTable;
        Cpu = SwSmiCpuTrigger->Cpu;
    }

    //
    // Found Invalid CPU number, return 
    //
    if(Cpu == (UINTN) -1) RETURN(Status);

    Data    = (UINT8)DispatchContext->SwSmiInputValue;
    
    pCpuSaveState       = pSmst->CpuSaveState;
    HighBufferAddress   = pCpuSaveState[Cpu].Ia32SaveState.ECX;
    LowBufferAddress    = pCpuSaveState[Cpu].Ia32SaveState.EBX;
#endif    

    pCommBuff           = HighBufferAddress;
    pCommBuff           = Shl64(pCommBuff, 32);
    pCommBuff           += LowBufferAddress;

    // Checking access rang for avoiding the privilege escalation into SMM 
    // via Smiflash driver.
    if(Data == SMIFLASH_GET_FLASH_INFO)
        Status = CheckAddressRange( (UINT8*)pCommBuff, sizeof(INFO_BLOCK) );
    else
        Status = CheckAddressRange( (UINT8*)pCommBuff, sizeof(FUNC_BLOCK) );
    if(EFI_ERROR(Status)) RETURN(Status);
   
    // Initial Error code for blocking Flash Update from PreHandler eLink.
    if (Data != SMIFLASH_GET_FLASH_INFO) ((FUNC_BLOCK*)pCommBuff)->ErrorCode = 0;
        
    for (i = 0; SMIFlashPreHandler[i] != NULL; i++) 
        SMIFlashPreHandler[i](Data, pCommBuff);

    // Process SmiFlash functions if GetFlashInfo call or No Error.
    if ((Data == SMIFLASH_GET_FLASH_INFO) || \
        (((FUNC_BLOCK*)pCommBuff)->ErrorCode == 0)) {

        switch (Data) {
            case 0x20:      // Enable Flash
                for (i = 0; SMIFlashPreUpdate[i] != NULL; i++) 
                    SMIFlashPreUpdate[i]();
                EnableFlashWrite((FUNC_BLOCK *)pCommBuff);
                break;
                
            case 0x24:      // Disable Flash
                for (i = 0; SMIFlashEndUpdate[i] != NULL; i++) 
                    SMIFlashEndUpdate[i]();
                DisableFlashWrite((FUNC_BLOCK *)pCommBuff);
                break;
                
            case 0x22:      // Erase Flash
                EraseFlash((FUNC_BLOCK *)pCommBuff);
                break;
                
            case 0x21:      // Read Flash
                ReadFlash((FUNC_BLOCK *)pCommBuff);
                break;
                
            case 0x23:      // Write Flash
                WriteFlash((FUNC_BLOCK *)pCommBuff);
                break;
                
            case 0x25:      // Get Flash Info
                GetFlashInfo((INFO_BLOCK *)pCommBuff);
        }
    }
    for (i = 0; SMIFlashEndHandler[i] != NULL; i++)
        SMIFlashEndHandler[i](Data, pCommBuff);

    RETURN(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   PrepareRomLayout
//
// Description: Preparing RomLayout table for GetFlashInfo()
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS PrepareRomLayout(VOID)
{
	EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
	EFI_STATUS  Status;
	UINT32      Authentication;
	UINTN       NumHandles;
	EFI_HANDLE  *HandleBuffer = NULL;
	UINT8       *Buffer = NULL;
	UINTN       BufferSize = 0;
	UINT8       *RomAreaBuffer = NULL;
	UINTN       RomAreaBufferSize;
	UINT8       i = 0, j = 0;
	
	Status = pBS->LocateHandleBuffer(
					ByProtocol,
					&gEfiFirmwareVolume2ProtocolGuid,
					NULL,
					&NumHandles,
					&HandleBuffer );
	if (EFI_ERROR(Status)) return Status;

	for ( i = 0; i < NumHandles; ++i )
	{
		Status = pBS->HandleProtocol(
						HandleBuffer[i],
						&gEfiFirmwareVolume2ProtocolGuid,
						&Fv );
		if (EFI_ERROR(Status)) continue;

        Status = Fv->ReadSection(Fv,
						&RomLayoutFfsGuid,
						EFI_SECTION_FREEFORM_SUBTYPE_GUID,
						0,
						(VOID**)&Buffer,
						&BufferSize,
						&Authentication );
		if ( Status == EFI_SUCCESS ) break;
	}

	RomAreaBuffer = Buffer + 0x10;
	RomAreaBufferSize = BufferSize - 0x10;
	NumberOfRomLayout = (RomAreaBufferSize / sizeof(ROM_AREA)) - 1;

	Status = pSmst->SmmAllocatePool (
						EfiRuntimeServicesData,
						NumberOfRomLayout * sizeof(ROM_AREA),
						(VOID**)&RomLayout );
	if (EFI_ERROR(Status)) return Status;
	MemCpy( RomLayout, RomAreaBuffer,
			NumberOfRomLayout * sizeof(ROM_AREA) );

	//Sorting
	for( i = 0; i < NumberOfRomLayout; i++ )
	{
		switch( RomLayout[i].Address  )
		{
//EIP166866 >>		
			case FV_PRE_BB_BASE: 
#if VERIFY_BOOT_SUPPORT				
			case (FV_PRE_BB_BASE - 0x1000): 
#endif
//EIP166866 <<			
			case FV_BB_BASE:
			case FV_MAIN_BASE:
			case NVRAM_ADDRESS:
#ifdef FAULT_TOLERANT_NVRAM_UPDATE
#if FAULT_TOLERANT_NVRAM_UPDATE
			case NVRAM_BACKUP_ADDRESS:
#endif
#endif
#if EC_FIRMWARE_UPDATE_INTERFACE_SUPPORT && SMI_FLASH_INTERFACE_VERSION > 10
			case AMI_EC_BASE:
#endif
				continue;
			default:
				break;
		}
		for( j = i + 1; j < NumberOfRomLayout; j++ )
		{
			if( RomLayout[j].Address < RomLayout[i].Address )
			{
				*(ROM_AREA*)RomAreaBuffer = RomLayout[j];
				RomLayout[j] = RomLayout[i];
				RomLayout[i] = *(ROM_AREA*)RomAreaBuffer;
			}
		}
	}
	
	//Prepare the FLASH_BLOCK_TYPE
	for( i = 0; i < NumberOfRomLayout; i++ )
	{
		switch( RomLayout[i].Address  )
		{
			case FV_PRE_BB_BASE: 
//EIP166866 >>			
#if VERIFY_BOOT_SUPPORT				
		    case (FV_PRE_BB_BASE - 0x1000): 
#endif
//EIP166866 <<			
			case FV_BB_BASE:
				RomLayout[i].Type = BOOT_BLOCK;
				break;
			case FV_MAIN_BASE:
		    case MICROCODE_ADDRESS: 
				RomLayout[i].Type = MAIN_BLOCK;
				break;
			case NVRAM_ADDRESS:
#ifdef FAULT_TOLERANT_NVRAM_UPDATE
#if FAULT_TOLERANT_NVRAM_UPDATE
			case NVRAM_BACKUP_ADDRESS:
#endif
#endif
				RomLayout[i].Type = NV_BLOCK;
				break;
#if EC_FIRMWARE_UPDATE_INTERFACE_SUPPORT && SMI_FLASH_INTERFACE_VERSION > 10
			case AMI_EC_BASE:
				RomLayout[i].Type = EC_BLOCK;
				break;
#endif
			default:
				RomLayout[i].Type = NC_BLOCK + NumberOfNcb;
				NumberOfNcb++;
		}
	}

	pBS->FreePool(Buffer);
	pBS->FreePool(HandleBuffer);

	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   InSmmFunction
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InSmmFunction(
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{  
    EFI_HANDLE  Handle;
    UINTN       Index;
    EFI_HANDLE  DummyHandle = NULL;
    EFI_STATUS  Status;
#if PI_SPECIFICATION_VERSION >= 0x1000A
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext;

#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext;
#endif
    EFI_SMM_ACCESS2_PROTOCOL        *SmmAccess2;
    UINTN                           Size;

#if PI_SPECIFICATION_VERSION >= 0x1000A

    Status = InitAmiSmmLib( ImageHandle, SystemTable );
    
    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &gSmmBase2);
    
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    Status = pSmmBase->GetSmstLocation (gSmmBase2, &pSmst);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;


    Status = pSmst->SmmLocateProtocol( \
                        &gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;


    Status = pSmst->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, &gSmmCpu);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

#else
    VERIFY_EFI_ERROR(pBS->LocateProtocol(
                          &gEfiSmmSwDispatchProtocolGuid, NULL, &pSwDispatch));
#endif

    Status = pBS->LocateProtocol( &gEfiSmmAccess2ProtocolGuid, NULL,
                                  (VOID**)&SmmAccess2 );
    if(EFI_ERROR(Status)) return EFI_SUCCESS;
    Size = 0;
    Status = SmmAccess2->GetCapabilities( SmmAccess2, &Size, mSmramRanges );
    if( Size == 0 ) return EFI_SUCCESS;
    Status = pSmst->SmmAllocatePool( EfiRuntimeServicesData, Size,
                                     (VOID**)&mSmramRanges );
    if(EFI_ERROR(Status)) return EFI_SUCCESS;
    Status = SmmAccess2->GetCapabilities( SmmAccess2, &Size, mSmramRanges );
    if(EFI_ERROR(Status)) return EFI_SUCCESS;
    mSmramRangeCount = Size / sizeof(EFI_SMRAM_DESCRIPTOR);

    VERIFY_EFI_ERROR(pBS->LocateProtocol(&gFlashSmmProtocolGuid,
                                                         NULL, &Flash));
    TRACE((TRACE_ALWAYS,"SmiFlash: Flash Protocol %X\n",Flash));
    
    for (Index = 0x20; Index < 0x26; Index++) {

        SwContext.SwSmiInputValue = Index;
        Status  = pSwDispatch->Register(pSwDispatch, SMIFlashSMIHandler,
                                                      &SwContext, &Handle);
             
        if (EFI_ERROR(Status)) return EFI_SUCCESS;
        
        //TODO: If any errors, unregister any registered SwSMI by this driver.
        //If error, and driver is unloaded, then a serious problem would exist.
    }
                                                      
#if PI_SPECIFICATION_VERSION >= 0x1000A
    Status = pSmst->SmmInstallProtocolInterface(
                 &DummyHandle,
                 &gEfiSmiFlashProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &SmiFlash
             );
#else
    Status = pBS->InstallMultipleProtocolInterfaces(
                 &DummyHandle,
                 &gEfiSmiFlashProtocolGuid,&SmiFlash,
                 NULL
             );
#endif
    ASSERT_EFI_ERROR(Status);

	Status = PrepareRomLayout();
	ASSERT_EFI_ERROR(Status);
	
    for (Index = 0; SMIFlashInSmm[Index] != NULL; SMIFlashInSmm[Index++]());
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SMIFlashDriverEntryPoint
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SMIFlashDriverEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    InitAmiLib(ImageHandle, SystemTable);
    return InitSmmHandler (ImageHandle, \
                            SystemTable, InSmmFunction, NULL);
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
