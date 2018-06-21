//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/BIN/Modules/ACPI/Template/S3Support/S3Save/BootScriptSave.c 1     2/03/11 4:08p Oleksiyy $
//
// $Revision: 1 $
//
// $Date: 2/03/11 4:08p $
//**********************************************************************


/** @file BootScriptSave.c
    Save Mem/Io/Pci/Smbus/etc into a table. The script will
    be executed on a S3 resume.

**/
#if PI_SPECIFICATION_VERSION>=0x0001000A
#include <Protocol/S3SaveState.h>
#include <Protocol/S3SmmSaveState.h>
#include <Protocol/SmmBase2.h>
#endif
#include <BootScriptCommon.h>
#include <Protocol/BootScriptSave.h>
#include <Protocol/smbus.h>
#include "BootScriptPrivate.h"
#include <AmiDxeLib.h>
#include <AmiAcpiS3.h>
//---For Compatibility Reasons---------------------------------------------------
#ifndef EFI_BOOT_SCRIPT_IO_POLL_OPCODE_OEM
#define EFI_BOOT_SCRIPT_IO_POLL_OPCODE_OEM EFI_BOOT_SCRIPT_IO_POLL_OPCODE
#define EFI_BOOT_SCRIPT_MEM_POLL_OPCODE_OEM EFI_BOOT_SCRIPT_MEM_POLL_OPCODE
#define EFI_BOOT_SCRIPT_PCI_POLL_OPCODE_OEM EFI_BOOT_SCRIPT_PCI_CONFIG_POLL_OPCODE
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// Currently reentrant code could fail if reentered when new table
// is being created.
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#if PI_SPECIFICATION_VERSION>=0x0001000A
EFI_SMM_SYSTEM_TABLE2       *gSmst2             = NULL;
EFI_SMM_BASE2_PROTOCOL      *mInternalSmmBase2  = NULL;
EFI_HANDLE                  mSmmS3SaveHandle    = NULL;
#endif

EFI_EVENT	                gEvtReadyToBoot;

typedef EFI_STATUS (*GENERIC_OPCODE_FUNC) (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL	*This,
	va_list								List
);


#define INIT_TABLE_SIZE		1024
#define MAX_TABLE_ENTRIES	2048
#define TBL_VAR_NAME	L"S3SS"



//static EFI_GUID gEfiBootScriptSaveGuid = EFI_BOOT_SCRIPT_SAVE_GUID;
static EFI_GUID gEfiBootScriptSaveGuid = EFI_BOOT_SCRIPT_SAVE_PROTOCOL_GUID;
static EFI_GUID gTableDataVarGuid = { 0x4bafc2b4, 0x2dc, 0x4104, 0xb2, 0x36, 0xd6, 0xf1, 0xb9, 0x8d, 0x9e, 0x84};

/**
    Internal Structure with information about boot script collected
 
**/
typedef struct {
	VOID *TableBottom;///< Pointer to the bottom (beginning) of the BootScript table
	VOID *TablePtr;///< Pointer to the next free position
	UINTN TableSize;///< Size of the table
    UINTN NumTableEntries;///< Number of BootScropt entries in the table
    BOOLEAN WasClosed;///< Signals that Close table was already closed
} TABLE_INFO;

#if PI_SPECIFICATION_VERSION>=0x0001000A
/**
    Structure with information on where to put next boot script
 
**/
typedef struct {
   BOOLEAN  IsItInsert;///< Signals, that BootScript should be inserted, and not just added to the end of the table.
   BOOLEAN  BeforeOrAfter;/// Insert before or after Position.
   UINTN    Position;///< Position to insert.
} BOOT_SCRIPT_INSERT_INFO;

BOOT_SCRIPT_INSERT_INFO gInsertInfo = {FALSE, FALSE, 0};
#define LABEL_MAX_SIZE 0x200
#endif

TABLE_INFO *gTableInfo = NULL;

/**
    Initialize first table.

        
    @param Table 

         
    @retval TRUE if new table was created.

    @note  
  Return True if successful table creation, otherwise return false.
    Need valid table info even if table fails, because functions may
    attempt to write to the boot script without a table being created.

**/
BOOLEAN NewTable()
{
    EFI_STATUS Status;
//--------------------------------
    Status = pBS->AllocatePool(EfiBootServicesData, INIT_TABLE_SIZE*2, &(gTableInfo->TableBottom));

    if (EFI_ERROR(Status)) 
    {
        gTableInfo->TableBottom = 0;
        return FALSE;
    }
    gTableInfo->TablePtr = gTableInfo->TableBottom;
    gTableInfo->TableSize = INIT_TABLE_SIZE*2;
    gTableInfo->NumTableEntries = 0;
    gTableInfo->WasClosed = FALSE;
    return TRUE;

}


/**
    Inits Boot Script Structure.

        
    @param IsItSmm 

         
    @retval EFI_SATUS Status of the operation.


**/

EFI_STATUS InitBootScriptStructure(BOOLEAN IsItSmm)
{
#if PI_SPECIFICATION_VERSION>=0x0001000A
    UINTN   sz=sizeof(VOID*);
#endif
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_PHYSICAL_ADDRESS	TableInfo=0xFFFFFFFF;
     
//--------------------------------
    if(gTableInfo == NULL)
    {
#if PI_SPECIFICATION_VERSION>=0x0001000A
        Status = pRS->GetVariable(L"S3SS", &gTableDataVarGuid, NULL, &sz, &gTableInfo);
        if(EFI_ERROR(Status) && Status!=EFI_NOT_FOUND && IsItSmm) return EFI_OUT_OF_RESOURCES;
        if(Status==EFI_NOT_FOUND)
#endif        
        {
            Status = pBS->AllocatePages(AllocateMaxAddress, EfiReservedMemoryType, EFI_SIZE_TO_PAGES(sizeof(TABLE_INFO)),
                            &TableInfo);

            gTableInfo = (TABLE_INFO *)(UINTN)TableInfo;

            ASSERT_EFI_ERROR(Status);
            if(EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
            if (!NewTable()) return EFI_OUT_OF_RESOURCES;
#if PI_SPECIFICATION_VERSION>=0x0001000A
            Status = pRS->SetVariable(L"S3SS",&gTableDataVarGuid,
                     (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
                     sz, &gTableInfo);
            ASSERT_EFI_ERROR(Status);
            if(EFI_ERROR(Status) && IsItSmm) return EFI_OUT_OF_RESOURCES;
#endif
        }            

        
    }
    return Status;
}
#if PI_SPECIFICATION_VERSION>=0x0001000A
/**
    Founds position and, based on Size, allocates space for Entry, 
    that has to be incerted, in table based on parameters in 
    gInsertInfo structure.

        
    @param Size size of entry to allocate in table.

         
    @retval Pointer to allocated entry in table.


**/
VOID* InsertPositionInBootScript (UINTN Size)
{
    TABLE_INFO  *Info = gTableInfo;
    UINT8       *CopyFrom = Info->TableBottom, *CopyTo;
    UINTN       i = 0, CopySize = (UINT8*)Info->TablePtr - (UINT8*)Info->TableBottom;
    BOOLEAN     Found = FALSE;

    if (gInsertInfo.Position != -1)
    {
        for (i=0; i<Info->NumTableEntries; i++)
        {   //Found position with supplied 
            if (((BOOT_SCRIPT_INFO_STRUCTURE*) CopyFrom)->UniqueIndex == gInsertInfo.Position) 
            {
                Found = TRUE;
                if (gInsertInfo.BeforeOrAfter == FALSE)
                    CopyFrom += ((BOOT_SCRIPT_INFO_STRUCTURE*) CopyFrom)->Length;
                break; 
            }
            CopyFrom += ((BOOT_SCRIPT_INFO_STRUCTURE*) CopyFrom)->Length;
        }  
        if (!Found) return NULL; 
    }
    if ((Size != 0) && (i != Info->NumTableEntries))
    {
        CopyTo = CopyFrom + Size;
        CopySize = (UINT8*)Info->TablePtr - CopyFrom;
        MemCpy(CopyTo, CopyFrom, CopySize);
    }
    return CopyFrom;
    
}
#endif
/**
    Allocate Entry from table. If not enough space, create
    larger table and return entry.

        
    @param Size size of entry to allocate in table.

         
    @retval Pointer to allocated entry in table.

    @note  
  Here is the control flow of this function:
	1. Does the BootScript table exist. If no return 0.
	2. Does table contain max entries? If so assert.
  3. Enough space in the table for the boot script call? If yes, go to step 8.
  ---Create table---
  4. Get Size of table needed for entry. (A multiple of INIT_TABLE_SIZE.)
  5. Allocate new table.
  6. Copy old table 
  7. Free old table.
  8. Update gTableInfo data.
  ------------------
  9. Adjust allocation pointer to allocate new space.
  10. Return newly allocated space.

**/

VOID * AllocTableEntry(IN UINTN Size)
{
	TABLE_INFO *Info = gTableInfo;
	VOID	    *TempBottom;
	VOID	    *TempPtr;
	UINTN	    TempSize, NewSize;
    BOOT_SCRIPT_TABLE_END	TableEnd;
#if PI_SPECIFICATION_VERSION>=0x0001000A
    //BOOT_SCRIPT_INFO_STRUCTURE BsHeader;
    Size += sizeof(BOOT_SCRIPT_INFO_STRUCTURE);
#endif    
	if (Info->TableBottom == NULL) return NULL;

	//Should not fill table. If table becomes full, then an error is causing too many table entries.
	ASSERT(Info->NumTableEntries < MAX_TABLE_ENTRIES);

	if ((UINTN)Info->TableBottom + Info->TableSize < (UINTN) Info->TablePtr + Size 
                                 + sizeof(BOOT_SCRIPT_TABLE_END)
#if PI_SPECIFICATION_VERSION>=0x0001000A
                                 + sizeof(BOOT_SCRIPT_INFO_STRUCTURE)
#endif
                                                                )
	{
#if PI_SPECIFICATION_VERSION>=0x0001000A
        if (gSmst2 != NULL) return NULL; // we can't allocate memory in SMM
#endif
		TempSize = Info->TableSize + INIT_TABLE_SIZE * (Size / INIT_TABLE_SIZE + 1);
		
		if (Info->WasClosed == FALSE) // Use AllocatePool if table was not closed - later it will be copyed under 4 Gb.
		{
			if (pBS->AllocatePool(EfiBootServicesData, TempSize, &TempBottom) != EFI_SUCCESS)
				return NULL;

			NewSize = TempSize;
		}
		else // Use AllocatePages if table was closed. We need it under 4 Gb.
		{
			EFI_PHYSICAL_ADDRESS			MaxAddress = 0xFFFFFFFF;

			if (pBS->AllocatePages(AllocateMaxAddress, EfiReservedMemoryType, 
		                        EFI_SIZE_TO_PAGES(TempSize) + 1, &MaxAddress) != EFI_SUCCESS)
				return NULL;
			TempBottom = (VOID*)(UINTN)MaxAddress;
			NewSize = EFI_PAGES_TO_SIZE(EFI_SIZE_TO_PAGES(TempSize) + 1); // Calculate the sieze of new structure from pages
			TempSize = EFI_SIZE_TO_PAGES(Info->TableSize); // Save old size in pages to use in FreePages.
		}

		MemCpy(TempBottom, Info->TableBottom, Info->TableSize);
		
		Info->TablePtr = (UINT8*)TempBottom + (UINTN)Info->TablePtr - (UINTN)Info->TableBottom;
		TempPtr = Info->TableBottom; // Save old TableBottom to freememory
		Info->TableBottom = TempBottom;
		Info->TableSize = NewSize;
        if (Info->WasClosed == TRUE) 
        {
            EFI_STATUS				Status;
	        ACPI_VARIABLE_SET		*AcpiVariableSet = NULL;
            CHAR16		AcpiGlobalVariable[] = ACPI_GLOBAL_VARIABLE;
            EFI_GUID	EfiAcpiVariableGuid	= EFI_ACPI_VARIABLE_GUID;
            UINTN		VariableSize = sizeof(ACPI_VARIABLE_SET*);
            Status = pRS->GetVariable(
		            AcpiGlobalVariable,
		            &EfiAcpiVariableGuid,
		            NULL,
		            &VariableSize,
		            &AcpiVariableSet
	                );
	        ASSERT_EFI_ERROR(Status);
            AcpiVariableSet->AcpiBootScriptTable = (EFI_PHYSICAL_ADDRESS) (UINTN) TempBottom;
            pBS->FreePages ((EFI_PHYSICAL_ADDRESS) (UINTN) TempPtr, TempSize); // If WasClosed == TRUE use FreePages
        }
        else pBS->FreePool(TempPtr); // If WasClosed != TRUE use FreePool
            
	}
#if PI_SPECIFICATION_VERSION>=0x0001000A
	if (gInsertInfo.IsItInsert == TRUE)
    {
        TempPtr = InsertPositionInBootScript (Size);
        if (TempPtr == NULL) return TempPtr;
    }
    else
#endif
    TempPtr = (UINT8*)Info->TablePtr;
	Info->TablePtr = (UINT8*)Info->TablePtr + Size;
#if PI_SPECIFICATION_VERSION>=0x0001000A
    ((BOOT_SCRIPT_INFO_STRUCTURE*)TempPtr)->UniqueIndex = (UINT32) Info->NumTableEntries;
    ((BOOT_SCRIPT_INFO_STRUCTURE*)TempPtr)->Length = (UINT32) Size;
    TempPtr = (UINT8*) TempPtr + sizeof(BOOT_SCRIPT_INFO_STRUCTURE);
#endif   
	TableEnd.Type = TABLE_END_OP_CODE;
	MemCpy((VOID*)((UINTN)Info->TablePtr 
#if PI_SPECIFICATION_VERSION>=0x0001000A
        + sizeof(BOOT_SCRIPT_INFO_STRUCTURE)
#endif 
        ), &TableEnd, sizeof(BOOT_SCRIPT_TABLE_END));
    ++(Info->NumTableEntries); 
	return TempPtr;
}

/**
    Write to boot script table an IoWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
  5. Add data to end of fixed table.
	6. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptIoWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL	*This,
	IN va_list							arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);

	UINT64 Address =  va_arg(arglist, UINT64);
	UINTN  Count = va_arg(arglist, UINTN);
	VOID   *Buffer = va_arg(arglist, VOID*);

	BOOT_SCRIPT_WRITE_IO WriteIo;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINTN	AddressCount;

	AddressCount = ValueSize * (  ((Width & ~3) != EfiBootScriptWidthFifoUint8 ) ? Count : 1);
	
	//---Validate Inputs---
	if (!Count) return EFI_INVALID_PARAMETER;
	if (Address + AddressCount > 0xffff) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry((UINT32)sizeof(BOOT_SCRIPT_WRITE_IO) + AddressCount);
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;

	WriteIo.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,Width);
	WriteIo.Port = (UINT16) Address;
	WriteIo.Count = Count;

	MemCpy(Ptr, &WriteIo, sizeof(BOOT_SCRIPT_WRITE_IO));	//Use MemCpy to avoid alignment problems.
	Ptr = (UINT8*) Ptr + sizeof(BOOT_SCRIPT_WRITE_IO);

	MemCpy(Ptr, Buffer, AddressCount);
	
	return EFI_SUCCESS;
}

/**
    Write to boot script table a IoReadWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result


    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptIoReadWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL	*This,
	va_list								arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address = va_arg(arglist, UINT64);
	VOID *DataValue = va_arg(arglist, VOID *);
	VOID *DataMask = va_arg(arglist, VOID *);

	BOOT_SCRIPT_READ_WRITE_IO ReadWriteIo;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT64	Data, Mask;

	Mask = *(UINT64*)DataMask;
	Data = *(UINT64*)DataValue;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

	//---Validate Inputs---
	if (Address + ValueSize > 0xffff) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_READ_WRITE_IO));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	ReadWriteIo.Type  = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_IO_READ_WRITE_OPCODE, Width);
	ReadWriteIo.Port  = (UINT16) Address;
	ReadWriteIo.Value = Data & BitMask;
	ReadWriteIo.Mask  = Mask & BitMask;

	MemCpy(Ptr, &ReadWriteIo, sizeof(ReadWriteIo));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table an MemWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
  5. Add data to end of fixed table.
	6. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptMemWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL	*This,
	va_list								arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);

	UINT64 Address =  va_arg(arglist, UINT64);
	UINTN  Count = va_arg(arglist, UINTN);
	VOID   *Buffer = va_arg(arglist, VOID*);

	BOOT_SCRIPT_WRITE_MEM WriteMem;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINTN	AddressCount;

	AddressCount = ValueSize * (  ((Width & ~3) != EfiBootScriptWidthFifoUint8 ) ? Count : 1);
	
	//---Validate Inputs---
	if (!Count) return EFI_INVALID_PARAMETER;
	
#if defined(EFI64) || defined(EFIx64)
	if (Address + AddressCount < Address)  return EFI_INVALID_PARAMETER;	//Overflow
#else
	Address &= 0xffffffff;		//When UINT32 -> UINT64, compiler sometimes sign extends.
	if (Address + AddressCount > 0xffffffff) return EFI_INVALID_PARAMETER;

#endif

	//---Fill and Copy Table---
	Ptr = AllocTableEntry((UINT32)sizeof(BOOT_SCRIPT_WRITE_MEM) + AddressCount);
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;

	WriteMem.Type = (UINT16)TABLE_TYPE2(EFI_BOOT_SCRIPT_MEM_WRITE_OPCODE,Width);
	WriteMem.Address = (UINT64)Address;
	WriteMem.Count = Count;

	MemCpy(Ptr, &WriteMem, sizeof(BOOT_SCRIPT_WRITE_MEM));	//Use MemCpy to avoid alignment problems.
	Ptr = (UINT8*) Ptr + sizeof(BOOT_SCRIPT_WRITE_MEM);

	MemCpy(Ptr, Buffer, AddressCount);
	
	return EFI_SUCCESS;
}

/**
    Write to boot script table an MemReadWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptMemReadWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address = va_arg(arglist, UINT64);
	VOID *DataValue = va_arg(arglist, VOID *);
	VOID *DataMask = va_arg(arglist, VOID *);

	BOOT_SCRIPT_READ_WRITE_MEM ReadWriteMem;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT64	Mask, Data;

	Mask = *(UINT64*)DataMask;
	Data = *(UINT64*)DataValue;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

	//---Validate Inputs---
#if defined(EFI64) || defined(EFIx64)
	if (Address + ValueSize < Address) return EFI_INVALID_PARAMETER;	//Overflow
#else
	Address &= 0xffffffff;		//When UINT32 -> UINT64, compiler sometimes sign extends.
	if (Address + ValueSize > 0xffffffff) return EFI_INVALID_PARAMETER;

#endif

	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_READ_WRITE_MEM));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	ReadWriteMem.Type = (UINT16)TABLE_TYPE2(EFI_BOOT_SCRIPT_MEM_READ_WRITE_OPCODE, Width);
	ReadWriteMem.Address = (UINT64)Address;
	ReadWriteMem.Value = Data & BitMask;
	ReadWriteMem.Mask  = Mask & BitMask;

	MemCpy(Ptr, &ReadWriteMem, sizeof(BOOT_SCRIPT_READ_WRITE_MEM));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
  5. Add data to end of fixed table.
	6. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptPciWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);

	UINT64 Address =  va_arg(arglist, UINT64);
	UINTN  Count = va_arg(arglist, UINTN);
	VOID   *Buffer = va_arg(arglist, VOID*);

	BOOT_SCRIPT_WRITE_PCI WritePci;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINTN	AddressCount;
	UINT16	Register;
	UINT16  RegisterMax;
    if (((Width & 3) == EfiBootScriptWidthUint16) && (Address & 0x01))
        return EFI_INVALID_PARAMETER;    
    if (((Width & 3) >= EfiBootScriptWidthUint32) && (Address & 0x03))
        return EFI_INVALID_PARAMETER;    

	AddressCount = ValueSize * (  ((Width & ~3) != EfiBootScriptWidthFifoUint8 ) ? Count : 1);

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif
	
	//---Validate Inputs---
	if (!Count) return EFI_INVALID_PARAMETER;
	
	if (Register + AddressCount > RegisterMax) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry((UINT32)sizeof(BOOT_SCRIPT_WRITE_PCI) + AddressCount);
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;

	WritePci.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_CONFIG_WRITE_OPCODE,Width);
	WritePci.Address = Address;
	WritePci.Count = Count;

	MemCpy(Ptr, &WritePci, sizeof(BOOT_SCRIPT_WRITE_PCI));	//Use MemCpy to avoid alignment problems.
	Ptr = (UINT8*) Ptr + sizeof(BOOT_SCRIPT_WRITE_PCI);

	MemCpy(Ptr, Buffer, AddressCount);
	
	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciReadWrite.

        
    @param This 
        IN va_list							arglist

    @retval 
        EFI_STATUS

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptPciReadWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address	= va_arg(arglist, UINT64);
	VOID *Data		= va_arg(arglist, VOID *);
	VOID *DataMask	= va_arg(arglist, VOID *);

	BOOT_SCRIPT_READ_WRITE_PCI ReadWritePci;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT16	Register;
	UINT16  RegisterMax;
    if (((Width & 3) == EfiBootScriptWidthUint16) && (Address & 0x01))
        return EFI_INVALID_PARAMETER;    
    if (((Width & 3) >= EfiBootScriptWidthUint32) && (Address & 0x03))
        return EFI_INVALID_PARAMETER;    

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif

	//---Validate Inputs---
	if (Register + ValueSize > RegisterMax) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_READ_WRITE_PCI));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	ReadWritePci.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_CONFIG_READ_WRITE_OPCODE, Width);
	ReadWritePci.Address = Address;
	ReadWritePci.Value = *(UINT64*) Data & BitMask;
	ReadWritePci.Mask  = *(UINT64*) DataMask & BitMask;

	MemCpy(Ptr, &ReadWritePci, sizeof(BOOT_SCRIPT_READ_WRITE_PCI));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table an SmbusExecute.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS BootScriptSmbusExecute (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	VOID	*Ptr;
	VOID	*Buffer;

	BOOT_SCRIPT_SMBUS_EXECUTE_32	Smbus;
	
	Smbus.Type			= TABLE_TYPE1(EFI_BOOT_SCRIPT_SMBUS_EXECUTE_OPCODE);
	Smbus.SlaveAddress	= (UINT32)va_arg(arglist, UINTN);
	Smbus.Command		= (UINT32)va_arg(arglist, UINTN);
	Smbus.Operation		= va_arg(arglist, EFI_SMBUS_OPERATION);
	Smbus.PecCheck		= va_arg(arglist, BOOLEAN);
	Smbus.Length		= *(va_arg(arglist, UINTN*));

	Buffer				= va_arg(arglist, VOID*);

	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_SMBUS_EXECUTE_32) + (UINT32)Smbus.Length);
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;

	MemCpy(Ptr, &Smbus, sizeof(BOOT_SCRIPT_SMBUS_EXECUTE_32));	//Use MemCpy to avoid alignment problems.
	Ptr = (UINT8*) Ptr + sizeof(BOOT_SCRIPT_SMBUS_EXECUTE_32);

	MemCpy(Ptr, Buffer, (UINT32)Smbus.Length);

	return EFI_SUCCESS;
}

/**
    Write to boot script table an Stall.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS BootScriptStall (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	VOID	*Ptr;
	BOOT_SCRIPT_STALL Stall;

	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_STALL));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;

	Stall.Type = TABLE_TYPE1(EFI_BOOT_SCRIPT_STALL_OPCODE);
	Stall.Duration = va_arg(arglist, UINTN);

	MemCpy(Ptr, &Stall, sizeof(BOOT_SCRIPT_STALL));

	return EFI_SUCCESS;
}

/**
    Write to boot script table a IoPoll.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS BootScriptIoPoll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address = va_arg(arglist, UINT64);
	VOID *DataMask = va_arg(arglist, VOID *);
	VOID *DataResult = va_arg(arglist, VOID *);

    BOOT_SCRIPT_POLL_IO PollIo;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT64	Mask, Result;

	Mask = *(UINT64*)DataMask;
	Result = *(UINT64*)DataResult;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

	//---Validate Inputs---
	if (Address + ValueSize > 0xffff) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_POLL_IO));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	PollIo.Type  = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_IO_POLL_OPCODE_OEM, Width);
	PollIo.Port  = (UINT16) Address;
	PollIo.Mask = Mask & BitMask;
	PollIo.Result  = Result & BitMask;

	MemCpy(Ptr, &PollIo, sizeof(BOOT_SCRIPT_POLL_IO));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table a MemPoll.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS BootScriptMemPoll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address = va_arg(arglist, UINT64);
	VOID *DataMask = va_arg(arglist, VOID *);
	VOID *DataResult = va_arg(arglist, VOID *);

	BOOT_SCRIPT_POLL_MEM PollMem;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT64	Mask, Result;

	Mask = *(UINT64*)DataMask;
	Result = *(UINT64*)DataResult;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

	//---Validate Inputs---
#if defined(EFI64) || defined(EFIx64)
	if (Address + ValueSize < Address) return EFI_INVALID_PARAMETER;	//Overflow
#else
	Address &= 0xffffffff;		//When UINT32 -> UINT64, compiler sometimes sign extends.
	if (Address + ValueSize > 0xffffffff) return EFI_INVALID_PARAMETER;
#endif

	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_POLL_MEM));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	PollMem.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_MEM_POLL_OPCODE_OEM, Width);
	PollMem.Address = (UINT64)Address;
	PollMem.Mask = Mask & BitMask;
	PollMem.Result  = Result & BitMask;

	MemCpy(Ptr, &PollMem, sizeof(BOOT_SCRIPT_POLL_MEM));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciPoll.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS BootScriptPciPoll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address	= va_arg(arglist, UINT64);
	VOID *DataMask	= va_arg(arglist, VOID *);
	VOID *DataResult= va_arg(arglist, VOID *);

	BOOT_SCRIPT_POLL_PCI PollPci;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT16	Register;
	UINT16  RegisterMax;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif

	//---Validate Inputs---
	if (Register + ValueSize > RegisterMax) return EFI_INVALID_PARAMETER;

	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_POLL_PCI));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	PollPci.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_POLL_OPCODE_OEM, Width);//change from EFI_BOOT_SCRIPT_PCI_POLL_OPCODE to EFI_BOOT_SCRIPT_PCI_POLL_OPCODE_OEM
	PollPci.Address = Address;
	PollPci.Mask = *(UINT64*) DataMask & BitMask;
	PollPci.Result  = *(UINT64*) DataResult & BitMask;

	MemCpy(Ptr, &PollPci, sizeof(BOOT_SCRIPT_POLL_PCI));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table an Dispatch.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS BootScriptDispatch (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
//---------Unsupported by now------------------------

	VOID	*Ptr;
	BOOT_SCRIPT_DISPATCH Dispatch;
  
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_DISPATCH));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	Dispatch.Type = TABLE_TYPE1(EFI_BOOT_SCRIPT_DISPATCH_OPCODE);
	Dispatch.EntryPoint = va_arg(arglist, EFI_PHYSICAL_ADDRESS);

	MemCpy(Ptr, &Dispatch, sizeof(BOOT_SCRIPT_DISPATCH));

	return EFI_SUCCESS;

//    return EFI_UNSUPPORTED;
}
#if PI_SPECIFICATION_VERSION>=0x0001000A
/**
    Write to boot script table an Dispatch2.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS SaveStateDispatch2 (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	VOID	*Ptr;
	BOOT_SCRIPT_DISPATCH2 Dispatch2;
  
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_DISPATCH2));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	Dispatch2.Type = TABLE_TYPE1(EFI_BOOT_SCRIPT_DISPATCH_2_OPCODE);
	Dispatch2.EntryPoint = va_arg(arglist, EFI_PHYSICAL_ADDRESS);
    Dispatch2.Context = va_arg(arglist, EFI_PHYSICAL_ADDRESS);

	MemCpy(Ptr, &Dispatch2, sizeof(BOOT_SCRIPT_DISPATCH2));

	return EFI_SUCCESS;
}

/**
    Write to boot script table an Information entry.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS SaveStateInformation (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	UINT32 InfoSize = va_arg(arglist, UINT32);
	EFI_PHYSICAL_ADDRESS InfoBuffer	= va_arg(arglist, EFI_PHYSICAL_ADDRESS);
	BOOT_SCRIPT_INFORMATION Information;
    VOID	*Ptr;


	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_INFORMATION));
	if (Ptr == NULL) return EFI_OUT_OF_RESOURCES;

	Information.Type = TABLE_TYPE1(EFI_BOOT_SCRIPT_INFORMATION_OPCODE);
	Information.Size = InfoSize;
    Information.MessagePtr = InfoBuffer;

	MemCpy(Ptr, &Information, sizeof(BOOT_SCRIPT_INFORMATION));
	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
  5. Add data to end of fixed table.
	6. Return EFI_SUCCESS.

**/
EFI_STATUS SaveStatePciCfg2Write (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
    UINT16 Segment = va_arg(arglist, UINT16);
	UINT64 Address =  va_arg(arglist, UINT64);
	UINTN  Count = va_arg(arglist, UINTN);
	VOID   *Buffer = va_arg(arglist, VOID*);

	BOOT_SCRIPT_PCI_CFG2_WRITE WritePci;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINTN	AddressCount;
	UINT16	Register;
	UINT16  RegisterMax;
    if (((Width & 3) == EfiBootScriptWidthUint16) && (Address & 0x01))
        return EFI_INVALID_PARAMETER;    
    if (((Width & 3) >= EfiBootScriptWidthUint32) && (Address & 0x03))
        return EFI_INVALID_PARAMETER;    

	AddressCount = ValueSize * (  ((Width & ~3) != EfiBootScriptWidthFifoUint8 ) ? Count : 1);

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif
	
	//---Validate Inputs---
	if (!Count) return EFI_INVALID_PARAMETER;
	
	if (Register + AddressCount > RegisterMax) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry((UINT32)sizeof(BOOT_SCRIPT_PCI_CFG2_WRITE) + AddressCount);
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;

	WritePci.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_CONFIG2_WRITE_OPCODE,Width);
    WritePci.Segment = Segment;
	WritePci.Address = Address;
	WritePci.Count = Count;

	MemCpy(Ptr, &WritePci, sizeof(BOOT_SCRIPT_PCI_CFG2_WRITE));	//Use MemCpy to avoid alignment problems.
	Ptr = (UINT8*) Ptr + sizeof(BOOT_SCRIPT_PCI_CFG2_WRITE);

	MemCpy(Ptr, Buffer, AddressCount);
	
	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciReadWrite.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS SaveStatePciCfg2ReadWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
    UINT16 Segment  = va_arg(arglist, UINT16);
	UINT64 Address	= va_arg(arglist, UINT64);
	VOID *Data		= va_arg(arglist, VOID *);
	VOID *DataMask	= va_arg(arglist, VOID *);

	BOOT_SCRIPT_PCI_CFG2_READ_WRITE ReadWritePci;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT16	Register;
	UINT16  RegisterMax;
    if (((Width & 3) == EfiBootScriptWidthUint16) && (Address & 0x01))
        return EFI_INVALID_PARAMETER;    
    if (((Width & 3) >= EfiBootScriptWidthUint32) && (Address & 0x03))
        return EFI_INVALID_PARAMETER;    

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif

	//---Validate Inputs---
	if (Register + ValueSize > RegisterMax) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_PCI_CFG2_READ_WRITE));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	ReadWritePci.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_CONFIG2_READ_WRITE_OPCODE, Width);
    ReadWritePci.Segment = Segment;
	ReadWritePci.Address = Address;
	ReadWritePci.Value = *(UINT64*) Data & BitMask;
	ReadWritePci.Mask  = *(UINT64*) DataMask & BitMask;

	MemCpy(Ptr, &ReadWritePci, sizeof(BOOT_SCRIPT_PCI_CFG2_READ_WRITE));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table a PI 1.1 IoPoll.

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS SaveStateIoPoll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address = va_arg(arglist, UINT64);
	VOID *DataResult = va_arg(arglist, VOID *);
	VOID *DataMask = va_arg(arglist, VOID *);
    UINT64 Delay = va_arg(arglist, UINT64);

    BOOT_SCRIPT_IO_POLL IoPoll;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT64	Mask, Result;

	Mask = *(UINT64*)DataMask;
	Result = *(UINT64*)DataResult;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

	//---Validate Inputs---
	if (Address + ValueSize > 0xffff) return EFI_INVALID_PARAMETER;


	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_IO_POLL));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	IoPoll.Type  = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_IO_POLL_OPCODE, Width);
	IoPoll.Port  = (UINT16) Address;
	IoPoll.Mask = Mask & BitMask;
	IoPoll.Result  = Result & BitMask;
    IoPoll.Delay  = Delay;

	MemCpy(Ptr, &IoPoll, sizeof(BOOT_SCRIPT_IO_POLL));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table a MemPoll (PI 1.1).

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS SaveStateMemPoll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address = va_arg(arglist, UINT64);
	VOID *DataMask = va_arg(arglist, VOID *); //CSP20131119
    VOID *DataResult = va_arg(arglist, VOID *); //CSP20131119
	UINT64 Delay = va_arg(arglist, UINT64);
    UINT64 LoopTimes = va_arg(arglist, UINT64); //CSP20131119

	BOOT_SCRIPT_MEM_POLL    MemPoll;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT64	Mask, Result;

	Mask = *(UINT64*)DataMask;
	Result = *(UINT64*)DataResult;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

	//---Validate Inputs---
#if defined(EFI64) || defined(EFIx64)
	if (Address + ValueSize < Address) return EFI_INVALID_PARAMETER;	//Overflow
#else
	Address &= 0xffffffff;		//When UINT32 -> UINT64, compiler sometimes sign extends.
	if (Address + ValueSize > 0xffffffff) return EFI_INVALID_PARAMETER;
#endif

	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_MEM_POLL));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	MemPoll.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_MEM_POLL_OPCODE, Width);
	MemPoll.Address = (UINT64)Address;
	MemPoll.Mask = Mask & BitMask;
	MemPoll.Result  = Result & BitMask;
    MemPoll.Delay  = Delay;
    MemPoll.LoopTimes= LoopTimes; //CSP20131119
	MemCpy(Ptr, &MemPoll, sizeof(BOOT_SCRIPT_MEM_POLL));	//Use MemCpy to avoid alignment problems.
    TRACE((-1,"SaveState Mem poll Mask %lx\n", MemPoll.Mask));
    TRACE((-1,"SaveState Mem poll Result %lx\n", MemPoll.Result));
	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciPoll (PI 1.1).

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS SaveStatePciCfgPoll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
	UINT64 Address	= va_arg(arglist, UINT64);
    VOID *DataResult= va_arg(arglist, VOID *);
	VOID *DataMask	= va_arg(arglist, VOID *);
	UINT64 Delay	= va_arg(arglist, UINT64);

	BOOT_SCRIPT_PCI_CFG_POLL        PciPoll;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT16	Register;
	UINT16  RegisterMax;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif

	//---Validate Inputs---
	if (Register + ValueSize > RegisterMax) return EFI_INVALID_PARAMETER;

	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_PCI_CFG_POLL));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	PciPoll.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_CONFIG_POLL_OPCODE, Width);
	PciPoll.Address = Address;
	PciPoll.Mask = *(UINT64*) DataMask & BitMask;
	PciPoll.Result  = *(UINT64*) DataResult & BitMask;
    PciPoll.Delay = Delay;

	MemCpy(Ptr, &PciPoll, sizeof(BOOT_SCRIPT_PCI_CFG_POLL));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}

/**
    Write to boot script table an PciPoll Cfg2 (PI 1.1).

        
    @param This - Pointer to this protocol
    @param va_list	variable arglist

    @retval 
        EFI_STATUS, based on result

    @note  
  Here is the control flow of this function:
	1. Get variable argument list from stack.
  2. Validate argument list. If invalid return EFI_INVALID_PARAMETER;
  3. Allocate entry from table.  If no room in table, return EFI_OUT_OF_RESOURCES.
  4. Fill fixed table entries.
	5. Return EFI_SUCCESS.

**/
EFI_STATUS SaveStatePciCfg2Poll (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL *This,
	va_list arglist
	)
{
	EFI_BOOT_SCRIPT_WIDTH Width = va_arg(arglist, EFI_BOOT_SCRIPT_WIDTH);
    UINT16 Segment  = va_arg(arglist, UINT16);
	UINT64 Address	= va_arg(arglist, UINT64);
    VOID *DataResult= va_arg(arglist, VOID *);
	VOID *DataMask	= va_arg(arglist, VOID *);
	UINT64 Delay	= va_arg(arglist, UINT64);

	BOOT_SCRIPT_PCI_CFG2_POLL       PciPoll2;
	VOID	*Ptr;
	UINT8	ValueSize = (UINT8) (1 << (Width & 3));
	UINT64	BitMask;
	UINT16	Register;
	UINT16  RegisterMax;

	switch(Width&3)
	{
	case EfiBootScriptWidthUint8:	BitMask = 0x00000000000000ff; break;
	case EfiBootScriptWidthUint16:	BitMask = 0x000000000000ffff; break;
	case EfiBootScriptWidthUint32:	BitMask = 0x00000000ffffffff; break;
	default:						BitMask = 0xffffffffffffffff;
	}

#if 0	//<-----For PCI-Express
	if (Address & 0xffffffff00000000)		//Extended register?
	{
		if (Address & 0xfffff00000000000) return EFI_INVALID_PARAMETER; //Limit 4k
		Register = (UINT16) (Address >> 16);
		RegisterMax = 0x1000;
	} else {
#else
		Register = (UINT8) Address;
		RegisterMax = 0x100;
#endif
#if 0
	}
#endif

	//---Validate Inputs---
	if (Register + ValueSize > RegisterMax) return EFI_INVALID_PARAMETER;

	//---Fill and Copy Table---
	Ptr = AllocTableEntry(sizeof(BOOT_SCRIPT_PCI_CFG2_POLL));
	if (Ptr == 0) return EFI_OUT_OF_RESOURCES;
	
	PciPoll2.Type = (UINT16) TABLE_TYPE2(EFI_BOOT_SCRIPT_PCI_CONFIG2_POLL_OPCODE , Width);
    PciPoll2.Segment = Segment;
	PciPoll2.Address = Address;
	PciPoll2.Mask = *(UINT64*) DataMask & BitMask;
	PciPoll2.Result  = *(UINT64*) DataResult & BitMask;
    PciPoll2.Delay = Delay;

	MemCpy(Ptr, &PciPoll2, sizeof(BOOT_SCRIPT_PCI_CFG2_POLL));	//Use MemCpy to avoid alignment problems.

	return EFI_SUCCESS;
}
#endif //#if PI_SPECIFICATION_VERSION>=0x0001000A
GENERIC_OPCODE_FUNC OpCodeFuncs[] = 
{
    BootScriptIoWrite,          //0
    BootScriptIoReadWrite,      //1
    BootScriptMemWrite,         //2
    BootScriptMemReadWrite,     //3
	BootScriptPciWrite,         //4
	BootScriptPciReadWrite,     //5
	BootScriptSmbusExecute,     //6
	BootScriptStall,            //7
	BootScriptDispatch,         //8
#if PI_SPECIFICATION_VERSION>=0x0001000A
	SaveStateDispatch2,         //9
    SaveStateInformation,       //a   ->0xA 
    SaveStatePciCfg2Write,      //b   ->0xB
    SaveStatePciCfg2ReadWrite,  //c   ->0xC
    SaveStateIoPoll,            //d
    SaveStateMemPoll,           //e
    SaveStatePciCfgPoll,        //f
    SaveStatePciCfg2Poll,       //10
#endif
//oem
    BootScriptIoPoll,           //0x80 -> 
    BootScriptMemPoll,          //0x81 -> 
    BootScriptPciPoll,          //0x82 -> 
};

/**
    All boot script functions (except close table) call this function.
    This function passes argument list to appropriate function.

        
    @param This Pointer to this protocol
    @param OpCode Boot Script opcode.
        ...

    @retval 
        EFI_STATUS, based on result.

**/
EFI_STATUS EfiBootScriptWrite (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL	*This,
	IN UINT16							TableName,
	IN UINT16							OpCode,
	...
	)
{
	EFI_STATUS Status;
	va_list arglist;
	va_start(arglist, OpCode);

    if (TableName != EFI_ACPI_S3_RESUME_SCRIPT_TABLE) 
        return EFI_INVALID_PARAMETER;

    if (OpCode > 0x82) {
        Status = EFI_INVALID_PARAMETER; goto EXIT_BOOT_SCRIPT_WRITE;
    }

//CSP20131119 >>
#if PI_SPECIFICATION_VERSION>=0x0001000A
    if (OpCode > 16 && OpCode < 0x80)
#else
    if (OpCode > 8 && OpCode < 0x80)
#endif
    {
        Status = EFI_INVALID_PARAMETER;
        goto EXIT_BOOT_SCRIPT_WRITE;
    }

    if (OpCode >= 0x80)
    {
        OpCode -= (0x80 - 9);
#if PI_SPECIFICATION_VERSION>=0x0001000A
        OpCode += 8;
#endif
    }
//CSP20131119 <<

    Status =  OpCodeFuncs[OpCode](This, arglist);

EXIT_BOOT_SCRIPT_WRITE:
	va_end(arglist);
	return Status;
}

/**
    Save table in runtime closing table and return pointer to table.

        
    @param This Pointer to this protocol.
	@param TableName Table to close.
    @param Address Pointer to a table on return.

    @retval 
        EFI_STATUS

    @note  
  Here is the control flow of this function:
	1. Allocate table entry for BOOT_SCRIPT_TABLE_END. If no entry, return EFI_OUT_OF_RESOURCES.
  2. Fill table.
  3. Allocate space for new table in Runtime.
  4. Copy table to new table.
  5. Set *Address to Runtime Table.
  6. Free old table.
  7. Create a initial table to start over if more Scripts are added.
  8. Return EFI_SUCCESS.

**/
EFI_STATUS EfiBootScriptCloseTable (
	IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL	*This,
	IN UINT16							TableName,
	OUT EFI_PHYSICAL_ADDRESS			*Address
	)
{
 	TABLE_INFO *Info = gTableInfo;
	UINTN	TableLength;
//---------------------------------------
    if (TableName != EFI_ACPI_S3_RESUME_SCRIPT_TABLE) 
        return EFI_INVALID_PARAMETER;
	TableLength = (UINTN)Info->TablePtr - (UINTN)Info->TableBottom 
#if PI_SPECIFICATION_VERSION>=0x0001000A
        + sizeof(BOOT_SCRIPT_INFO_STRUCTURE)
#endif 
		+ (UINTN) sizeof(BOOT_SCRIPT_TABLE_END);

	//Copy to new table.
    *Address = 0xFFFFFFFF;
	if (pBS->AllocatePages(AllocateMaxAddress, EfiReservedMemoryType, 
                        EFI_SIZE_TO_PAGES(TableLength + INIT_TABLE_SIZE) + 1, Address) != EFI_SUCCESS)
		return EFI_OUT_OF_RESOURCES;

	MemCpy((VOID*)(UINTN)*Address,Info->TableBottom, TableLength);
	pBS->FreePool(Info->TableBottom);

	//Reasign Table Address;
    Info->TableBottom = (VOID*)(UINTN)*Address;
    Info->TablePtr = (VOID*)((UINTN) Info->TableBottom + TableLength
        - sizeof(BOOT_SCRIPT_INFO_STRUCTURE)
        - (UINTN) sizeof(BOOT_SCRIPT_TABLE_END));
    Info->TableSize = EFI_PAGES_TO_SIZE(EFI_SIZE_TO_PAGES(TableLength + INIT_TABLE_SIZE) + 1);
    Info->WasClosed = TRUE;

	return EFI_SUCCESS;
}

EFI_BOOT_SCRIPT_SAVE_PROTOCOL gBootScriptSaveProtocol = 
{
	EfiBootScriptWrite,
	EfiBootScriptCloseTable
};


//PI 1.1 ++
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)

VOID ClearInsertInfo()
{
    gInsertInfo.IsItInsert = FALSE;
    gInsertInfo.BeforeOrAfter = FALSE;
    gInsertInfo.Position = -1;
    return;
}
/**
    This function is used to store an OpCode to be replayed as part 
    of the S3 resume boot path. 

        
    @param This Pointer to the S3 Save State Protocol
    @param OpCode Opcode to save
        ...

    @retval 
        EFI_STATUS, based on result.

**/
EFI_STATUS SaveStateWrite(
   IN CONST EFI_S3_SAVE_STATE_PROTOCOL  *This,
   IN       UINT16                      OpCode,
   ...)
{
	EFI_STATUS  Status=EFI_SUCCESS;
    UINTN       OpIndex;
	va_list arglist;
	va_start(arglist, OpCode);
//----------------------------------

    //map SaveState opcodes to the op code table.
    switch(OpCode){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: 
        case 0x9:
        case 0xA:
        case 0xB:
        case 0xC: 
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:         
            OpIndex=OpCode;
            break;
        case 0x80:
        case 0x81:
        case 0x82:        
            OpIndex= OpCode - (0x6F);    
            break;
        default:
            Status = EFI_UNSUPPORTED; 
            goto EXIT_BOOT_SCRIPT_WRITE;
    }
    Status =  OpCodeFuncs[OpIndex](&gBootScriptSaveProtocol, arglist);
    //TRACE((-1,"SaveState UniqueIndex %x\n", gTableInfo->NumTableEntries -1));

EXIT_BOOT_SCRIPT_WRITE:
	va_end(arglist);
	return Status;
}
/**
    This function is used to store an OpCode to be replayed as part 
    of the S3 resume boot path. The opcode is stored before (TRUE) 
    or after (FALSE) the position in the boot script table specified 
    by Position. If Position is NULL or points to NULL then the new 
    opcode is inserted at the beginning of the table (if TRUE) or end 
    of the table (if FALSE).

        
    @param This Pointer to the S3 Save State Protocol
    @param BeforeOrAfter where to insert
    @param Position optional position 
    @param OpCode Boot Script Opcode
        ...

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS SaveStateInsert(
   IN CONST EFI_S3_SAVE_STATE_PROTOCOL  *This,
   IN       BOOLEAN                     BeforeOrAfter,
   IN OUT   EFI_S3_BOOT_SCRIPT_POSITION *Position       OPTIONAL,
   IN       UINT16                      OpCode,
   ...
){

   	EFI_STATUS  Status=EFI_SUCCESS;
    UINTN       OpIndex;
    
	va_list arglist;
	va_start(arglist, OpCode);
//----------------------------------

    //map SaveState opcodes to the op code table.
    switch(OpCode){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: 
        case 0x9:
        case 0xA:
        case 0xB:
        case 0xC: 
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:         
            OpIndex=OpCode;
            break;
        case 0x80:
        case 0x81:
        case 0x82:        
            OpIndex= OpCode - (0x6F);    
            break;
        default:
            Status = EFI_INVALID_PARAMETER; 
            goto EXIT_BOOT_SCRIPT_WRITE;
    }

    gInsertInfo.IsItInsert = TRUE;
    gInsertInfo.BeforeOrAfter = BeforeOrAfter;
    if (!Position || !*Position)
        if (BeforeOrAfter) gInsertInfo.Position = -1;
        else gInsertInfo.IsItInsert = FALSE;
    else
    {           
        if ((UINTN) *Position >= gTableInfo->NumTableEntries) 
        {
            ClearInsertInfo();
            return EFI_INVALID_PARAMETER;
        }
        gInsertInfo.Position = (UINTN)*Position;
    }    
    Status = OpCodeFuncs[OpIndex](&gBootScriptSaveProtocol, arglist);
    if (Position) *Position = (VOID*)(gTableInfo->NumTableEntries - 1);
    ClearInsertInfo();
EXIT_BOOT_SCRIPT_WRITE:
	va_end(arglist);
	return Status;
}

/**
    Find a label within the boot script table and, if not present, 
    optionally create it.

        
    @param This Pointer to the S3 Save State Protocol
    @param BeforeOrAfter where to insert
    @param CreateIfNotFound create a new label if not found
    @param Position optional position 
    @param Label pointer to label


    @retval 
        EFI_STATUS

**/
EFI_STATUS SaveStateLabel(
   IN CONST  EFI_S3_SAVE_STATE_PROTOCOL      *This,
   IN        BOOLEAN                         BeforeOrAfter,
   IN        BOOLEAN                         CreateIfNotFound,
   IN OUT    EFI_S3_BOOT_SCRIPT_POSITION     *Position OPTIONAL,
   IN CONST  CHAR8                           *Label)
{
    UINT32  LabelSize, i;
    UINT8   *TableHdr = gTableInfo->TableBottom, *TableInfo;
    UINT16  Type;
    BOOT_SCRIPT_LABEL LabelStr;
    
    if ((Label == NULL) || (*Label == 0))
    {
        TRACE((-1,"First Exit\n"));
        return EFI_INVALID_PARAMETER;
    }
    for(LabelSize=0; LabelSize<= LABEL_MAX_SIZE ; LabelSize++)
    {
        if (Label[LabelSize] == 0) break; // We assume label is 0 terminated.

    }
    if (LabelSize >= LABEL_MAX_SIZE) 
    {
        TRACE((-1,"2-d Exit\n"));
        return EFI_INVALID_PARAMETER; // Label is too long or wrong/corrupted pointer
    }
    for (i=0; i<=gTableInfo->NumTableEntries; i++)
    {
        TableInfo = TableHdr + sizeof(BOOT_SCRIPT_INFO_STRUCTURE);
        Type = *(UINT8*) TableInfo + (*((UINT8*) TableInfo + 1) << 8);	//In case not aligned if alignment required.
        if (Type == EFI_BOOT_SCRIPT_LABEL_OPCODE_OEM)
        {
            if (((BOOT_SCRIPT_LABEL*) TableInfo)->Size == LabelSize && 
                    !MemCmp((VOID*)Label, TableInfo + sizeof(BOOT_SCRIPT_LABEL), LabelSize))
            {
                if (Position) *Position = ((VOID*)((BOOT_SCRIPT_INFO_STRUCTURE*) TableHdr)->UniqueIndex);
                else return EFI_INVALID_PARAMETER; //We found label, but can't return its position
                return EFI_SUCCESS; // Label found and position returned
            }
        }
        TableHdr += ((BOOT_SCRIPT_INFO_STRUCTURE*) TableHdr)->Length;     
    }
    //----Label was not found

    if (!CreateIfNotFound) return EFI_NOT_FOUND;

    //----So - lets create it at position

    if (Position && ((UINTN) *Position >= gTableInfo->NumTableEntries)) 
        return EFI_INVALID_PARAMETER; // Position is not valid

    //---Filling gInsertInfo based on passed parameters

    gInsertInfo.IsItInsert = TRUE;
    gInsertInfo.BeforeOrAfter = BeforeOrAfter;

    if (!Position || !*Position)
        if (BeforeOrAfter) gInsertInfo.Position = -1;
        else gInsertInfo.IsItInsert = FALSE;
    else
    {           
        if ((UINTN) *Position >= gTableInfo->NumTableEntries) 
        {
            ClearInsertInfo();
            return EFI_INVALID_PARAMETER;
        }
        gInsertInfo.Position = (UINTN)*Position;
    }    
    TableHdr = AllocTableEntry(sizeof(BOOT_SCRIPT_LABEL) + LabelSize);
    ClearInsertInfo();
    if (!TableHdr) 
        return EFI_OUT_OF_RESOURCES;
	LabelStr.Type = TABLE_TYPE1(EFI_BOOT_SCRIPT_LABEL_OPCODE_OEM);
	LabelStr.Size = LabelSize;
    MemCpy(TableHdr, &LabelStr, sizeof(BOOT_SCRIPT_LABEL));
    TableHdr += sizeof(BOOT_SCRIPT_LABEL);
    MemCpy(TableHdr, (VOID*)Label, LabelSize);
    if (Position) 
        *Position = (VOID*)(gTableInfo->NumTableEntries - 1);
    return EFI_SUCCESS;    
}

/**
    Compare two positions in the boot script table and return their 
    relative position.

        
    @param This Pointer to the S3 Save State Protocol
    @param Position1 first position to compare
    @param Position2 second position to compare
    @param RelativePosition on rerturn Relative Position

    @retval 
        EFI_STATUS, based on result

**/
EFI_STATUS SaveStateCompare(
   IN CONST EFI_S3_SAVE_STATE_PROTOCOL          *This,
   IN       EFI_S3_BOOT_SCRIPT_POSITION         Position1,
   IN       EFI_S3_BOOT_SCRIPT_POSITION         Position2,
   OUT      UINTN                               *RelativePosition)
{
    VOID *First, *Second;
    if (((UINTN) Position1 > gTableInfo->NumTableEntries)  
          || ((UINTN) Position2 > gTableInfo->NumTableEntries))
        return EFI_INVALID_PARAMETER;
    gInsertInfo.BeforeOrAfter = TRUE;
    gInsertInfo.Position = (UINTN)Position1;
    First = InsertPositionInBootScript (0);
    if (!First) 
    {
        ClearInsertInfo();
        return EFI_INVALID_PARAMETER;
    }
    gInsertInfo.Position = (UINTN)Position2;
    Second = InsertPositionInBootScript (0);
    if (!Second) 
    {
        ClearInsertInfo();
        return EFI_INVALID_PARAMETER;
    }
    if (First == Second) *RelativePosition = 0;
    else 
        if (First < Second) *RelativePosition = -1;
        else *RelativePosition = 1;
    ClearInsertInfo();
    return EFI_SUCCESS;
}



EFI_S3_SAVE_STATE_PROTOCOL gS3SaveStateProtocol={
  	SaveStateWrite,
    SaveStateInsert,
    SaveStateLabel,
    SaveStateCompare
};

/**
    Initialize table, if not done yet, and install protocols in SMM.

        
    @param ImageHandle Image handle
    @param SystemTable pointer to system table

    @retval 
        EFI_STATUS, based on result.

**/

EFI_STATUS InitBootScriptSmm(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
	)
{
    EFI_STATUS  Status;
//-------------------------------	
    Status=InitAmiSmmLib(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR (Status);
    if( EFI_ERROR(Status)) return Status;

    Status=InitBootScriptStructure(TRUE);
    if(EFI_ERROR(Status)) return Status;

    // We are in SMM, retrieve the pointer to SMM System Table
    Status=mInternalSmmBase2->GetSmstLocation (mInternalSmmBase2, &gSmst2);
    ASSERT (gSmst2 != NULL);
    if( EFI_ERROR(Status) || (gSmst2 == NULL)) return EFI_UNSUPPORTED;


  // Install SMM PCI Root Bridge I/O Protocol
    Status=gSmst2->SmmInstallProtocolInterface(
                  &mSmmS3SaveHandle,
                  &gEfiS3SmmSaveStateProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  (EFI_S3_SMM_SAVE_STATE_PROTOCOL*)&gS3SaveStateProtocol
                  );

    


    return Status;


}

#endif
//PI 1.1 --

/**
    Initialize table and install protocols.

        
	@param ImageHandle Image handle
    @param SystemTable pointer to system table

    @retval 
        EFI_STATUS, based on result.

**/
EFI_STATUS InitBootScriptDxe(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
	)
{
    EFI_STATUS  Status;

	Status = InitBootScriptStructure(FALSE);
    if(EFI_ERROR(Status)) return Status;

	Status = SystemTable->BootServices->InstallMultipleProtocolInterfaces(
		&ImageHandle,
		&gEfiBootScriptSaveGuid, &gBootScriptSaveProtocol,
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
        &gEfiS3SaveStateProtocolGuid, &gS3SaveStateProtocol,
#endif
		NULL
		);

	ASSERT_EFI_ERROR(Status);

	Status = CreateReadyToBootEvent(
		TPL_CALLBACK,
		CallbackReadyToBoot,
		NULL,
		&gEvtReadyToBoot
	);
	ASSERT_EFI_ERROR(Status);

    return Status;
}
/**
    Initialize table and install protocols.
    Called twice - in and outside SMM.

        
    @param ImageHandle Image handle
    @param SystemTable pointer to system table

    @retval 
        EFI_STATUS, based on result.

**/
EFI_STATUS InitBootScript(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
	)
{
#if PI_SPECIFICATION_VERSION>=0x0001000A
    EFI_STATUS  Status;
    BOOLEAN     InSmm=FALSE;
#endif
//-------
	InitAmiLib(ImageHandle, SystemTable);
//PI 1.1 ++
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    // Retrieve SMM Base2 Protocol,  Do not use gBS from UefiBootServicesTableLib on purpose
    // to prevent inclusion of gBS, gST, and gImageHandle from SMM Drivers unless the 
    // SMM driver explicity declares that dependency. 
    Status = SystemTable->BootServices->LocateProtocol (
                                        &gEfiSmmBase2ProtocolGuid,
                                        NULL,
                                        (VOID **)&mInternalSmmBase2
                                        );
    if( EFI_ERROR(Status)){
        //if we can't find SMM Protocols that's not InSmm initialization
        if(Status==EFI_NOT_FOUND) InSmm=FALSE; 
        else return EFI_UNSUPPORTED;
    } else mInternalSmmBase2->InSmm (mInternalSmmBase2, &InSmm);

    // Check to see if we are already in SMM
    if (!InSmm ) {
#endif
        // We are not in SMM, so SMST is not needed
        return InitBootScriptDxe(ImageHandle,SystemTable);
//PI 1.1 ++
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    } else {
        return InitBootScriptSmm(ImageHandle,SystemTable);
    }
#endif
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
