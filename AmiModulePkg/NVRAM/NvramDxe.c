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
// $Header: /Alaska/SOURCE/Core/Modules/NVRAM/NVRAMDXE.c 106   2/22/13 4:42p Oleksiyy $
//
// $Revision: 106 $
//
// $Date: 2/22/13 4:42p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	NVRAMDXE.h
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include "NVRAM.h"
#include <AmiDxeLib.h>
#include <Protocol/Variable.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/MonotonicCounter.h>
#include <Protocol/FlashProtocol.h>
#include <Library/AmiCriticalSectionLib.h>
#include <Token.h>
#include <Protocol/SmmVariable.h> //CSP20131017 
#if NV_SIMULATION == 0
#define HYBRID_NV_INTERFACE
#endif

#define HASH_SHA256_LEN 32

#define UEFI21_SPECIFIC_VARIABLE_ATTRIBUTES (EFI_VARIABLE_HARDWARE_ERROR_RECORD |\
                                            EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS)

#define UEFI23_1_SPECIFIC_VARIABLE_ATTRIBUTES (EFI_VARIABLE_HARDWARE_ERROR_RECORD |\
                                            EFI_VARIABLE_APPEND_WRITE |\
                                            UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES)

//##### Security ######
#if AuthVariable_SUPPORT
VOID AuthVariableServiceInit (VOID);
VOID AuthVariableServiceInitSMM (VOID);
VOID AuthServiceVirtualFixup(VOID);
EFI_STATUS VerifyVariable (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32   *Attributes,
    IN VOID     **Data,
    IN UINTN    *DataSize, 
    IN VOID     *OldData,   
    IN UINTN     OldDataSize,
    IN OUT EXT_SEC_FLAGS *ExtFlags
);
#else
VOID AuthVariableServiceInit (VOID) {};
VOID AuthVariableServiceInitSMM (VOID) {};
VOID AuthServiceVirtualFixup(VOID) {};
EFI_STATUS VerifyVariable (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32   *Attributes,
    IN VOID     **Data,
    IN UINTN    *DataSize, 
    IN VOID     *OldData,   
    IN UINTN     OldDataSize,
   IN OUT EXT_SEC_FLAGS *ExtFlags
){
//UEFI 2.3.1 requres to return SECURITY_VIOLATION error if no Auth Var support present
    if (*Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES)
        return EFI_INVALID_PARAMETER; 
    else
        return EFI_SUCCESS;
}
#endif //#if AuthVariable_SUPPORT

#define ALL_VARIABLE_ATTRIBUTES ( EFI_VARIABLE_NON_VOLATILE |\
                                  EFI_VARIABLE_BOOTSERVICE_ACCESS |\
                                  EFI_VARIABLE_RUNTIME_ACCESS|\
                                  UEFI23_1_SPECIFIC_VARIABLE_ATTRIBUTES)

#define VALATILE_VARIABLE_STORE_SIZE 0x10000

#define BEGIN_CRITICAL_SECTION(Cs) \
    { EFI_STATUS Status = BeginCriticalSection(Cs);\
      ASSERT(Status==EFI_SUCCESS || Status==EFI_ACCESS_DENIED);\
      if (EFI_ERROR(Status)) return Status;\
    }
#define END_CRITICAL_SECTION(Cs) VERIFY_EFI_ERROR(EndCriticalSection(Cs))

#ifdef EFI_DEBUG
#define NVRAM_TRACE(Arguments) { if (!Runtime) TRACE(Arguments); }
#else
#define NVRAM_TRACE(Arguments)
#endif

#if defined(NVRAM_SMI_FULL_PROTECTION) &&  (NVRAM_SMI_FULL_PROTECTION == 1)
#define DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME 1
#endif

//============================================================================
// Type definitions
typedef BOOLEAN (*NVRAM_UPDATE_ROUTINE)(IN VOID *Address, UINTN Size);
typedef BOOLEAN (*NVRAM_PROGRAM_ROUTINE)(IN VOID *Address, UINTN Size, VOID *Data);
typedef BOOLEAN (*COPY_VAR_STORE_FILTER)(
    IN VOID *Context, CHAR16* VarName, EFI_GUID* VarGuid, 
    NVRAM_STORE_INFO *Info
);

typedef struct
{
    NVRAM_UPDATE_ROUTINE EnableUpdate;
    NVRAM_UPDATE_ROUTINE DisableUpdate;
    NVRAM_UPDATE_ROUTINE EraseBytes;
    NVRAM_PROGRAM_ROUTINE WriteBytes;
} VARIABLE_INTERFACE;

typedef struct
{
    UINT32 Mode;
    UINT32 HeaderLength;
    UINT32 NvramMode;
    UINT32 InfoCount, LastInfoIndex;
    NVRAM_STORE_INFO NvramInfo[10];
    NVRAM_STORE_INFO *NvInfo, *MemInfo;
    VARIABLE_INTERFACE *NvInterface, *MemInterface;
    UINT8 *BackupAddress;
#ifdef HYBRID_NV_INTERFACE
    UINTN AddressDelta;
    UINT8* NvramFlashAddress;
#endif
} VARSTORE_INFO;

typedef EFI_STATUS (HOOK_GET_VARIABLE)(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
);
typedef EFI_STATUS (HOOK_SET_VARIABLE)(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
);

typedef EFI_STATUS (HOOK_GET_NEXT_VARIABLE_NAME)(
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
);

typedef EFI_STATUS (*SHOW_BOOT_TIME_VARIABLES)(BOOLEAN Show);

typedef struct{
    SHOW_BOOT_TIME_VARIABLES ShowBootTimeVariables;
} AMI_NVRAM_CONTROL_PROTOCOL;

#define LTEB_GUID  \
    {0xC8BCA618, 0xBFC6, 0x46B7, 0x8D, 0x19, 0x83, 0x14, 0xE2, 0xE5, 0x6E, 0xC1}

//======================================================================
//Function Prototypes
EFI_STATUS NvramReinitialize();
EFI_STATUS InitVolatileStore(
    NVRAM_STORE_INFO *Info, UINTN Size,
    UINTN HeaderLength, UINT8 Flags,
    BOOLEAN Runtime
);
BOOLEAN CheckTheHeader(NVRAM_STORE_INFO *Info, UINT32 HeaderLength, BOOLEAN Update);

//======================================================================
// Global and extern variables
extern const BOOLEAN NvramMonotonicCounterSupport;
extern BOOLEAN NvramChecksumSupport;
extern const BOOLEAN FlashNotMemoryMapped;
static BOOLEAN Runtime = FALSE;
static BOOLEAN HideBtVariables = TRUE;
static EFI_GUID gAmiNvramControlProtocolGuid = { 0xf7ca7568, 0x5a09, 0x4d2c, { 0x8a, 0x9b, 0x75, 0x84, 0x68, 0x59, 0x2a, 0xe2 } };

#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
BOOLEAN NoAccessOutsideofSmm = FALSE;
#endif

VARSTORE_INFO VarStoreInfo;
CRITICAL_SECTION NvramCs = NULL;

const static UINT32 NvramSignature = NVAR_SIGNATURE;                                    
extern const BOOLEAN NvramRtGarbageCollectionSupport;
EFI_PHYSICAL_ADDRESS NvramDriverBuffer = 0; // Runtime buffer for use in Runtime with the size of 3 times NVRAM_SIZE
// First part (2 times NVRAM_SIZE) usialy used for TmpBuffer, while last part for VolatileStore.
UINT32 NvramDriverBufferSize = 0;     
BOOLEAN FirstPartAlloc = FALSE;
BOOLEAN SecondPartAlloc = FALSE;                

static FLASH_PROTOCOL *Flash = NULL;
extern HOOK_GET_VARIABLE GET_VAR_LIST EndOfGetVariableHook;
extern HOOK_SET_VARIABLE SET_VAR_LIST EndOfSetVariableHook;
extern HOOK_GET_NEXT_VARIABLE_NAME GET_NEXT_VAR_NAME_LIST EndOfGetNextVarNameHook;

HOOK_GET_VARIABLE* GetVariableHookList[]=
    {GET_VAR_LIST NULL};
HOOK_SET_VARIABLE* SetVariableHookList[]=
    {SET_VAR_LIST NULL};
HOOK_GET_NEXT_VARIABLE_NAME* GetNextVarNameHookList[]=
    {GET_NEXT_VAR_NAME_LIST NULL};

EFI_STATUS ShowBootTimeVariables (BOOLEAN Show);
AMI_NVRAM_CONTROL_PROTOCOL NvramControl = {ShowBootTimeVariables };

EFI_STATUS GetVariableHook(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
){
    UINTN i;
    EFI_STATUS Result = EFI_UNSUPPORTED;
    for(i=0; GetVariableHookList[i] && (Result == EFI_UNSUPPORTED); i++) 
        Result = GetVariableHookList[i](VariableName, VendorGuid, Attributes, DataSize, Data);
    return Result;
}

EFI_STATUS SetVariableHook(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
){
    UINTN i;
    EFI_STATUS Result = EFI_UNSUPPORTED;
    for(i=0; SetVariableHookList[i] && (Result == EFI_UNSUPPORTED); i++) 
        Result = SetVariableHookList [i](VariableName, VendorGuid, Attributes, DataSize, Data);
    return Result;
}

EFI_STATUS GetNextVarNameHook(
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
){
    UINTN i;
    EFI_STATUS Result = EFI_UNSUPPORTED;
    for(i=0; GetNextVarNameHookList[i] && (Result == EFI_UNSUPPORTED); i++) 
        Result = GetNextVarNameHookList [i](VariableNameSize, VariableName, VendorGuid);
    return Result;
}
//--- GetVariable, SetVariable and GetNextVarName Hooks END ------

//Update Interfaces:
//Memory-based
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DummyUpdateRoutine
//
// Description: This function is dummy function doing nothing and always returning TRUE
//
// Input:       IN VOID *Address - Memory address
//              UINTN Size - Size
//
// Output:      TRUE always
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN DummyUpdateRoutine(IN VOID *Address, UINTN Size) {return TRUE;}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MemErase
//
// Description: This function sets Size amount memory starting from Address to FLASH_EMPTY_BYTE 
//
// Input:       IN VOID *Address - Memory address
//              UINTN Size - Size
//
// Output:      TRUE always
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN MemErase(IN VOID *Address, UINTN Size)
{
    MemSet(Address,Size,FLASH_EMPTY_BYTE);
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MemProgram
//
// Description: This function writs Data to memory starting from Address 
//
// Input:       IN VOID *Address - Memory address
//              UINTN Size - Size of data to write
//              VOID *Data - pointer to data to write
//
// Output:      TRUE always
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN MemProgram(IN VOID *Address, UINTN Size, VOID *Data)
{
    MemCpy(Address,Data,Size);
    return TRUE;
}

//Flash-based
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvEnableWrites
//
// Description: This function enables writes to flash starting from Address 
//
// Input:       IN VOID *Address - Start address
//              UINTN Size - Size of flash to be write enabled
//
// Output:      TRUE always
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN NvEnableWrites(IN VOID *Address, UINTN Size)
{
    UINT8* Start = (UINT8*)Address;
    UINT8* p;
    Flash->DeviceWriteEnable();
    
    for (p=(UINT8*)BLOCK(Start); p<Start+Size; p+=FlashBlockSize) FlashBlockWriteEnable(p);
    
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvDisableWrites
//
// Description: This function disables writes to flash starting from Address 
//
// Input:       IN VOID *Address - Start address
//              UINTN Size - Size of flash to be write disable
//
// Output:      TRUE always
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN NvDisableWrites(IN VOID *Address, UINTN Size)
{
    UINT8* Start = (UINT8*)Address;
    UINT8* p;
    
    for (p=(UINT8*)BLOCK(Start); p<Start+Size; p+=FlashBlockSize) FlashBlockWriteDisable(p);
    
    Flash->DeviceWriteDisable();
    return TRUE;
}

#define INT_SIZE sizeof(INTN)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsProgrammableOrEqual
//
// Description: This function checks if defined memory chunks are the same and if destenation is programmable
//
// Input:       IN  UINT8 *pDest - pointer to memory to check
//              IN  UINT8 *pSource - pointer to the source to compare with
//              IN  UINTN Size - size of the chunk
//              OUT BOOLEAN *pIsProgrammable - Will be set to TRUE if pDest is programable
//              OUT BOOLEAN *pIsEqual - Will be set to TRUE if pDest and pSource has the same content
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IsProgrammableOrEqual(
    IN  UINT8           *pDest,
    IN  UINT8           *pSource,
    IN  UINTN           Size,
    OUT BOOLEAN         *pIsProgrammable,
    OUT BOOLEAN         *pIsEqual
)
{
    *pIsEqual = TRUE;
    *pIsProgrammable = TRUE;
    
    // loops through the destination looking to see if the data is the same
    //  as the source, or if the Destination has already bee erased
    if (!( (UINTN)pDest & (INT_SIZE-1) || (UINTN)pSource & (INT_SIZE-1) ))
    {
        UINTN *Dest = (UINTN*)pDest, *Source = (UINTN*)pSource;
        
        for ( ; Size >= INT_SIZE; Size -= INT_SIZE)
        {
            UINTN s,d;
            
            if (FLASH_EMPTY_BYTE==0)
            {
                s=~*Source;
                if (FlashNotMemoryMapped) 
                    Flash->Read((UINT8 *)Dest, INT_SIZE, (UINT8*)&d);
                else 
                    d=*Dest;
            }
            
            else
            {
                s=*Source;
		        if (FlashNotMemoryMapped)
                {
                    Flash->Read((UINT8 *)Dest, INT_SIZE, (UINT8*)&d);
		            d=~d;
                }
                else 
                    d=~*Dest;
            }
            
            if ((d&s)!=0)
            {
                *pIsProgrammable = FALSE;
                *pIsEqual = FALSE;
                return;
            }
        if (FlashNotMemoryMapped)
        {
		    Flash->Read((UINT8 *)Dest, INT_SIZE, (UINT8*)&d);
	        if (d != *Source++) *pIsEqual = FALSE;
            Dest++;
        }
        else 
            if (*Dest++ != *Source++) *pIsEqual = FALSE;

        }
    }
    
    // since the address may not be INT_SIZE aligned, this checks
    //  the rest of the data
    for ( ; Size > 0; Size--)
    {
        UINT8 s,d;
        
        if (FLASH_EMPTY_BYTE==0)
        {
            s=~*pSource;
            if (FlashNotMemoryMapped)
	            Flash->Read(pDest, 1, &d);
            else
                d=*pDest;

        }
        else
        {
            s=*pSource;
            if (FlashNotMemoryMapped)
            {
	            Flash->Read(pDest, 1, &d);
	            d=~d;
            }
            else
                d=~*pDest;

        }
        
        if ((d&s)!=0)
        {
            *pIsProgrammable = FALSE;
            *pIsEqual = FALSE;
            return;
        }
        if (FlashNotMemoryMapped)
        {
	        Flash->Read(pDest, 1, &d);
	        if ( d != *pSource++) *pIsEqual = FALSE;
            pDest++;
        }
        else
            if (*pDest++ != *pSource++)
                *pIsEqual = FALSE;

    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsClean
//
// Description: This function checks if defined memory chunk is clean
//
// Input:       IN  UINT8 *pDest - pointer to memory to check
//              IN  UINTN Size - size of the chunk
//
// Output:      BOOLEAN - TRUE if pDest is clean, otherwice FALSE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static BOOLEAN IsClean(
    IN  UINT8           *pDest,
    IN  UINTN           Size
)
{
    // loops through the destination looking to see if the buffer is empty
    if (!( (UINTN)pDest & (INT_SIZE-1) ))
    {
        for ( ; Size >= INT_SIZE; Size -= INT_SIZE, pDest += INT_SIZE)
	    {
            if (FlashNotMemoryMapped)
            {
                UINTN nData=0;
		        Flash->Read(pDest, INT_SIZE, (UINT8*)&nData);
		        if (nData != FlashEmpty) return FALSE;
            }
            else
                if (*(UINTN*)pDest != FlashEmpty) return FALSE;
	    }
    }
    
    // since the address may not be INT_SIZE aligned, this checks
    //  the rest of the data
    for ( ; Size > 0; Size--, pDest++)
	{
        if (FlashNotMemoryMapped)
        {
            UINT8 nData=0;
		    Flash->Read(pDest, 1, (UINT8*)&nData);
		    if (nData != FLASH_EMPTY_BYTE) return FALSE;
        }
        else
            if (*pDest != FLASH_EMPTY_BYTE) return FALSE;
	}

    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvProgram
//
// Description: This function programms Size amount of flash starting with Address
//
// Input:       IN VOID *Address - pointer to flash to programm
//              IN  UINTN Size - size of the data to programm
//              VOID *Data - pointer to the data to be programed
//
// Output:      BOOLEAN - TRUE if pDest is clean, otherwice FALSE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN NvProgram(IN VOID *Address, UINTN Size, VOID *Data)
{
    BOOLEAN Status;
    BOOLEAN Programmable,Equal;
    
    if (Size==0) return TRUE;
    
    IsProgrammableOrEqual(Address,Data,Size,&Programmable,&Equal);
    
    if (Equal) return TRUE;
    
    if (!Programmable) return FALSE;
    
    Status = FlashProgram(Address,Data,(UINT32)Size);
    
    if (Status)
    {
        if (FlashNotMemoryMapped)
        {
            UINT8 nData=0;
            UINTN Count=0;
	        for ( Count=0; Count<Size; Count++)
	        {
                Flash->Read((UINT8 *)Address+Count, 1, &nData );
	            Status = ( nData == *((UINT8 *)Data+Count) ); 
                if (!Status) break;
	        }
        }
        else
            Status = ( MemCmp(Address,Data,Size) == 0 );

    }
    
    if (!Runtime && !Status) ERROR_CODE(DXE_FLASH_UPDATE_FAILED,EFI_ERROR_MAJOR);
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SelectiveAllocate
//
// Description: This function returns pointer to allocated memory to use as
//              temp buffer for any occasions. If NvramDriverBuffer is not 0 - 
//              memory allocated previously will be used. If not - function will 
//              allocate memory with the Size - and return pointer to it.
//
// Input:       IN BOOLEAN  FirstPart - if TRUE - first part of NvramDriverBuffer 
//                                      will be used.
//              IN  UINTN Size - size of memory to allocate.
//
// Output:      UINT8* - pointer to allocated memory.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8* SelectiveAllocate (BOOLEAN FirstPart, UINTN Size)
{
    if (NvramDriverBuffer){
        if (FirstPart){
            if (!FirstPartAlloc){
                FirstPartAlloc = TRUE;
                return (UINT8*)NvramDriverBuffer;
            }else return NULL;
        }else{
            if (!SecondPartAlloc){
                SecondPartAlloc = TRUE;
                return (UINT8*)(NvramDriverBuffer + NvramDriverBufferSize / 3 * 2);
            }else return NULL;  
        } 
    }else{  
        if (!Runtime){
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
            if (NoAccessOutsideofSmm){
                UINT8 *Buffer;
                EFI_STATUS Status = pSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,Size, (VOID**)&Buffer
                );
                if (EFI_ERROR(Status)) return NULL;
                return Buffer;
            }
            else
#endif
            return (UINT8*)Malloc(Size);
        }
    }
    return NULL;  
}
        
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SelectiveFree
//
// Description:   This function free memory allocated for temp buffer 
//              for any occasions. If address passed to this function fits 
//              (NvramDriverBuffer +  it's size) range memory allocated in  
//              NvramDriverBuffer will be released. If not - function will 
//              call FreePool routine.
//
// Input:       UINT8* Target - pointer to the begining of memory to free
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
            
VOID SelectiveFree (UINT8* Target)
{
    if (NvramDriverBuffer){
        if (Target == (UINT8*) NvramDriverBuffer){
            FirstPartAlloc = FALSE;
            return;
        }
        if (Target == (UINT8*) (NvramDriverBuffer + NvramDriverBufferSize / 3 * 2)){
            SecondPartAlloc = FALSE;
            return;
        } 
    }
    if (!Runtime){
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
            if (NoAccessOutsideofSmm) pSmst->SmmFreePool (Target);
            else
#endif
        pBS->FreePool(Target);
    }
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvErase
//
// Description: This function erases Size amount of flash starting with Address
//
// Input:       IN VOID *Address - pointer to flash to erase
//              IN  UINTN Size - size of the flash to erase
//
// Output:      BOOLEAN - TRUE or FALSE based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN NvErase(IN VOID *Address, UINTN Size)
{
    UINT8* pStart = (UINT8*)Address;
    UINT8* pEnd = pStart+Size;
    UINT8* pStartBlock = (UINT8*)BLOCK(pStart);
    UINT8* pEndBlock = (UINT8*)BLOCK(pEnd-1)+FlashBlockSize;
    UINTN PrologSize = pStart - pStartBlock;
    UINTN EpilogSize = pEndBlock-pEnd;
    UINT8* pBuffer=NULL;
    UINT8* p;
    BOOLEAN ok;
    
    if ( Size==0 || IsClean(Address,Size) ) return TRUE;
    
    if (PrologSize+EpilogSize)
    {
        pBuffer = SelectiveAllocate (TRUE, (PrologSize + EpilogSize));
        if (pBuffer == NULL) return FALSE;
        MemCpy(pBuffer,pStartBlock,PrologSize);
        MemCpy(pBuffer+PrologSize,pEnd,EpilogSize);
    }
    
    //erase first block
    if (ok=FlashEraseBlock(pStartBlock))
    {
        if (PrologSize) ok=NvProgram(pStartBlock,PrologSize,pBuffer);
        
        if (ok)
        {
            //erase other blocks
            for (p=pStartBlock+=FlashBlockSize; p!=pEndBlock; p+=FlashBlockSize)
            {
                if (!(ok=FlashEraseBlock(p))) break;
            }
            
            if (ok && EpilogSize) ok=NvProgram(pEnd,EpilogSize,pBuffer+PrologSize);
        }
    }
    
    if (pBuffer) SelectiveFree (pBuffer);
    
    if (!ok) ERROR_CODE(DXE_FLASH_UPDATE_FAILED,EFI_ERROR_MAJOR);
    
    return ok;
}

VARIABLE_INTERFACE MemInterface = {DummyUpdateRoutine, DummyUpdateRoutine, MemErase, MemProgram};
VARIABLE_INTERFACE NvInterface = {NvEnableWrites, NvDisableWrites, NvErase, NvProgram};

#ifdef HYBRID_NV_INTERFACE

UINTN HybridGetAddressDelta(UINT8 *Address, UINTN Size){
    if (VarStoreInfo.NvInfo == 0) return VarStoreInfo.AddressDelta;
//Sometimes VarStoreInfo.NvInterface is used to update
//store other than VarStoreInfo.NvramInfo.
//For example during UpdateFtVarstore it is used to update backup store.
//This function is used to fall-back to non-hybrid interface if varstore other than
//VarStoreInfo.NvramInfo is being updated.
    return
    (   Address >= VarStoreInfo.NvInfo->NvramAddress
     &&     Address+Size 
        <=    VarStoreInfo.NvInfo->NvramAddress 
            + VarStoreInfo.NvInfo->NvramSize
    ) ? VarStoreInfo.AddressDelta : 0;
}

BOOLEAN HybridEnableWrites(IN VOID *Address, UINTN Size){
    UINTN Delta = HybridGetAddressDelta(Address,Size);
	return NvEnableWrites(Delta+(UINT8*)Address, Size);
}

BOOLEAN HybridDisableWrites(IN VOID *Address, UINTN Size){
    UINTN Delta = HybridGetAddressDelta(Address,Size);
	return NvDisableWrites(Delta+(UINT8*)Address, Size);
}

BOOLEAN HybridErase(IN VOID *Address, UINTN Size){
    BOOLEAN ok;
    UINTN Delta = HybridGetAddressDelta(Address,Size);
	ok = NvErase(Delta+(UINT8*)Address, Size);
    // Zero Delta implies that there is no memory buffer
    if (Delta==0) return ok;
    if (ok) return MemErase(Address, Size);
    // Flash update operation has failed.
    // The memory buffer is out of synch with the flash device.
    // Re-synchronize the mememory buffer by reading flash device.
    if (FlashNotMemoryMapped) 
        Flash->Read(Delta+(UINT8*)Address, Size, Address);
    else 
        MemCpy(Address, Delta+(UINT8*)Address, Size);
    return FALSE;
}

BOOLEAN HybridProgram(IN VOID *Address, UINTN Size, VOID *Data){
    BOOLEAN ok;
    UINTN Delta = HybridGetAddressDelta(Address,Size);
	ok = NvProgram(Delta+(UINT8*)Address, Size, Data);
    // Zero Delta implies that there is no memory buffer
    if (Delta==0) return ok;
    if (ok) return MemProgram(Address, Size, Data);
    // Flash update operation has failed.
    // The memory buffer is out of synch with the flash device.
    // Re-synchronize the mememory buffer by reading flash device.
    if (FlashNotMemoryMapped) 
        Flash->Read(Delta+(UINT8*)Address, Size, Address);
    else 
        MemCpy(Address, Delta+(UINT8*)Address, Size);
    return FALSE;
}

VARIABLE_INTERFACE HybridInterface = {HybridEnableWrites, HybridDisableWrites, HybridErase, HybridProgram};

VOID InitHybridInterface(NVRAM_STORE_INFO *NvStore){
    EFI_STATUS Status;
    //We can't use InitVolatileStore here because it will screw up SelectiveAllocate/SelectiveFree logic.
    
    //TODO: When DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME is defined, we assume that
    //DXE Variable services will be replaced prior to transitioning to runtime.
    //If this will not happen, the NVRAM driver may not work properly.
    //It may either crash (or crash the Windows), or generate false garbage collections.
    //To support fall back scenario the memory allocation below and registration of the virtual address change event must be changed
	VarStoreInfo.NvramFlashAddress = NvStore->NvramAddress;
    Status = pBS->AllocatePool (
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
        EfiBootServicesData,
#else
        EfiRuntimeServicesData,
#endif
        NvStore->NvramSize, (VOID**)&NvStore->NvramAddress
    );
    if (!EFI_ERROR(Status)){
        VarStoreInfo.NvInterface=&HybridInterface;
        MemCpy(NvStore->NvramAddress,VarStoreInfo.NvramFlashAddress,NvStore->NvramSize);
        NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM: working in hybrid mode\n"));
    }else{
        VarStoreInfo.NvInterface=&NvInterface;
        NvStore->NvramAddress = VarStoreInfo.NvramFlashAddress;
    }
    //We altered content of the varstore. Let's reinitialize it.
    NvInitInfoBuffer(
        NvStore, VarStoreInfo.HeaderLength,
        NVRAM_STORE_FLAG_NON_VALATILE
    );
    VarStoreInfo.AddressDelta=VarStoreInfo.NvramFlashAddress-NvStore->NvramAddress;
}

BOOLEAN SwapVarstoresInHybridMode(NVRAM_STORE_INFO *BackupInfo){
    UINT8* OriginalNvramFlashAddress;
	if ( VarStoreInfo.NvramFlashAddress == 0 ) return FALSE;
	OriginalNvramFlashAddress=VarStoreInfo.NvramFlashAddress;

    VarStoreInfo.NvramFlashAddress = VarStoreInfo.BackupAddress;
    // Zero AddressDelta implies that the memory buffer is no longer used.
    if (VarStoreInfo.AddressDelta!=0){
        VarStoreInfo.AddressDelta=VarStoreInfo.NvramFlashAddress-VarStoreInfo.NvInfo->NvramAddress;
    
        MemCpy(
            VarStoreInfo.NvInfo->NvramAddress,VarStoreInfo.BackupAddress,
            VarStoreInfo.NvInfo->NvramSize
        );
    }
    BackupInfo->NvramAddress = VarStoreInfo.NvInfo->NvramAddress;
    BackupInfo->NvramSize = VarStoreInfo.NvInfo->NvramSize;
    NvInitInfoBuffer(
        BackupInfo, VarStoreInfo.HeaderLength, VarStoreInfo.NvInfo->Flags
    );
    VarStoreInfo.BackupAddress = OriginalNvramFlashAddress;
	return TRUE;
}

VOID HybridStoreToRealStore(NVRAM_STORE_INFO *InInfo, NVRAM_STORE_INFO *OutInfo){
    *OutInfo = *InInfo;
    OutInfo->NvramAddress = VarStoreInfo.NvramFlashAddress;
    OutInfo->NvramGuidsAddress = (EFI_GUID*)((UINT8*)OutInfo->NvramGuidsAddress + VarStoreInfo.AddressDelta);
    OutInfo->pEndOfVars += VarStoreInfo.AddressDelta;
    OutInfo->pFirstVar += VarStoreInfo.AddressDelta;
    OutInfo->pLastReturned=0;
}

#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
VOID ReallocateHybridVarstoreToSmm(){
    NVRAM_STORE_INFO *Info = VarStoreInfo.NvInfo;
    UINT8* SmmBuffer;
    UINT32 i;
    UINT8* NvramAddress;
    UINT8* EndOfVars;

    EFI_STATUS Status = pSmst->SmmAllocatePool (
        EfiRuntimeServicesData,
        Info->NvramSize, &SmmBuffer
    );
    if (EFI_ERROR(Status)){
        // Zero AddressDelta implies that the memory buffer is no longer used.
        VarStoreInfo.AddressDelta=0;
        return;
    }
    MemCpy(
        SmmBuffer,Info->NvramAddress,Info->NvramSize
    );
    VarStoreInfo.AddressDelta=SmmBuffer-Info->NvramAddress;
    NvramAddress = Info->NvramAddress;
    EndOfVars= Info->pEndOfVars;
    // Fix up the main NVRAM store and nested variable stores.
    // An example of a nested store is the defaults store.
    for (i=0; i<VarStoreInfo.InfoCount; i++){
        // If a store is embedded into the main NVRAM store, 
        // fix it up with the SMM buffer address
        if (   VarStoreInfo.NvramInfo[i].NvramAddress >= NvramAddress
            && VarStoreInfo.NvramInfo[i].pEndOfVars <= EndOfVars
        ){
            VarStoreInfo.NvramInfo[i].NvramAddress += VarStoreInfo.AddressDelta;
            VarStoreInfo.NvramInfo[i].NvramGuidsAddress = (EFI_GUID*)((UINT8*)VarStoreInfo.NvramInfo[i].NvramGuidsAddress + VarStoreInfo.AddressDelta);
            VarStoreInfo.NvramInfo[i].pEndOfVars += VarStoreInfo.AddressDelta;
            VarStoreInfo.NvramInfo[i].pFirstVar += VarStoreInfo.AddressDelta;
            VarStoreInfo.NvramInfo[i].pLastReturned=0;
        }
    }
    VarStoreInfo.AddressDelta=VarStoreInfo.NvramFlashAddress-SmmBuffer;
}
#endif // DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
#endif

#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
VOID ReallocateVolatileVarstoreToSmm(){
    NVRAM_STORE_INFO *Info = VarStoreInfo.MemInfo;
    UINT8* SmmBuffer;
    UINTN Delta;

    EFI_STATUS Status = pSmst->SmmAllocatePool (
        EfiRuntimeServicesData,
        Info->NvramSize, &SmmBuffer
    );
    if (EFI_ERROR(Status)){
        //TODO: Shutdown non-valatile services;
        ASSERT(FALSE);
        return;
    }
    MemCpy(
        SmmBuffer,Info->NvramAddress,Info->NvramSize
    );
    Delta=SmmBuffer-Info->NvramAddress;
    Info->NvramAddress = SmmBuffer;
    Info->NvramGuidsAddress = (EFI_GUID*)((UINT8*)Info->NvramGuidsAddress + Delta);
    Info->pEndOfVars += Delta;
    Info->pFirstVar += Delta;
    Info->pLastReturned=0;
}
#endif

///////////////////////////////////////////////////////////////////////////
//Cache operations
#define NV_CACHE_SUPPORT 1

typedef struct
{
    DLINK Link;
    NVAR *Nvar;
    NVAR *FinalDataNvar;
    UINT32 InfoIndex;
    CHAR16 Name[8];
} VAR_INDEX_ENTRY;

typedef struct
{
    DLIST LetterIndex['z'-'a'+1];
    DLIST OtherIndex;
    DLIST Blanks;
    UINT8 *Buffer;
    UINTN UsedSize;
    UINTN BufferSize;
} VAR_INDEX;

BOOLEAN AreBtVariablesHidden()
{
    return HideBtVariables && Runtime;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsNvramRuntime
//
// Description: This function returns the value of Runtime local variable
//
// Input:       None
//
// Output:      BOOLEAN - current value of Runtime local variable
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsNvramRuntime()
{
    return Runtime;
}


EFI_STATUS ShowBootTimeVariables (BOOLEAN Show){
    HideBtVariables = !Show;
    TRACE((-1,"Setting HideBtVariables to %d\n",HideBtVariables));
    return EFI_SUCCESS;
}

EFI_STATUS InstallNvramControlSmmProtocol(){
    return pSmst->SmmInstallConfigurationTable(
        pSmst, &gAmiNvramControlProtocolGuid, &NvramControl, sizeof(NvramControl) 
    );
}


#if !NV_CACHE_SUPPORT
VAR_INDEX *VarIndexPtr=NULL;
#else
#define VARIABLE_INDEX_SIZE 0x10000

VAR_INDEX VarIndex;
VAR_INDEX *VarIndexPtr=&VarIndex;


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexGetList
//
// Description: This function gets list of variables starting with FirstLetter
//
// Input:       CHAR16 FirstLetter - letter to search
//
// Output:      DLIST* - pointer to list of Var starting with FirstLetter
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
DLIST* IndexGetList(CHAR16 FirstLetter)
{
    //convert to lower case
    if (FirstLetter>='A'&&FirstLetter<='Z') FirstLetter -= 'A';
    else if (FirstLetter>='a'&&FirstLetter<='z') FirstLetter -= 'a';
    else return &VarIndexPtr->OtherIndex;

    return &VarIndexPtr->LetterIndex[FirstLetter];
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexGetListNvar
//
// Description: This function gets list of variables starting with First Letter of Nvar
//
// Input:       NVAR *Nvar - pointer to Var 
//
// Output:      DLIST* - pointer to list of Var 
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
DLIST* IndexGetListNvar(NVAR *Nvar)
{
    VOID *NamePtr;
    CHAR16 FirstLetter;

    NamePtr = NvGetName(Nvar);
    FirstLetter = (Nvar->flags&NVRAM_FLAG_ASCII_NAME) ? *(CHAR8*)NamePtr : *(CHAR16*)NamePtr;
    return IndexGetList(FirstLetter);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexFindVariableEntry
//
// Description: This function searches for Var with specific GUID
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OUT UINTN *VariableNameSize - size of Var name 
//              OUT NVRAM_STORE_INFO **Info - double pointer to NVRAM_STORE_INFO structure
//              OUT BOOLEAN *UnindexedExists - indicator, that there is unindexed variables
//
// Output:      VAR_INDEX_ENTRY* - pointer to Var's Index entry found; NULL if not found
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VAR_INDEX_ENTRY* IndexFindVariableEntry(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINTN *VariableNameSize,
    OUT NVRAM_STORE_INFO **Info, OUT BOOLEAN *UnindexedExists
)
{
    DLINK *Link;
    VAR_INDEX_ENTRY* Entry;
    UINTN Count;
    DLIST *List;
    BOOLEAN CorruptedEntryFound;

    if (VarIndexPtr==NULL)
    {
        *UnindexedExists=TRUE;
        return NULL;
    }

    List =  IndexGetList(VariableName[0]);
    CorruptedEntryFound = FALSE;

    for (Link=List->pHead, Count=0; Link!=NULL; Link=Link->pNext, Count++)
    {
        UINTN i;
        Entry = OUTTER(Link,Link, VAR_INDEX_ENTRY);

        for (i=0; i<sizeof(Entry->Name)/sizeof(CHAR16)&&Entry->Name[i]; i++)
            if (Entry->Name[i]!=VariableName[i+1]) break;

        if (Entry->Name[i]&&Entry->Name[i]!=VariableName[i+1]) continue;

        if(!NvIsVariable( Entry->Nvar, 
                          &VarStoreInfo.NvramInfo[Entry->InfoIndex]
            )
        ){
            CorruptedEntryFound = TRUE;
            continue;       
        }

        if (NvVarEq(
                    Entry->Nvar,VariableName,VendorGuid,
                    VariableNameSize,&VarStoreInfo.NvramInfo[Entry->InfoIndex]
                )
           ) break;
    }

    //entry found
    if (Link!=NULL)
    {
        *Info=&VarStoreInfo.NvramInfo[Entry->InfoIndex];
        //VariableNameSize has already been initialized by the NvVarEq call
        //Make sure the cached final data Nvar is valid
        if (Entry->FinalDataNvar==NULL)
            Entry->FinalDataNvar=NvGetDataNvar(Entry->Nvar, *Info);
            
        return Entry;
    }

    *UnindexedExists=CorruptedEntryFound || Count<List->Size;
    return NULL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexFindVariable
//
// Description: This function searches for Var with specific GUID
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OUT UINTN *VariableNameSize - size of Var name 
//              OUT NVRAM_STORE_INFO **Info - double pointer to NVRAM_STORE_INFO structure
//              OUT BOOLEAN *UnindexedExists - indicator, that there is unindexed variables
//
// Output:      NVAR* - pointer to Var found; NULL if not found
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
NVAR* IndexFindVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINTN *VariableNameSize,
    OUT NVRAM_STORE_INFO **Info, OUT BOOLEAN *UnindexedExists
)
{
    VAR_INDEX_ENTRY* Entry = IndexFindVariableEntry(
                                VariableName, VendorGuid,
                                VariableNameSize,
                                Info, UnindexedExists
                                );
    if (Entry != NULL) return Entry->Nvar;
    
    return NULL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexGetBlankEntry
//
// Description: This function searches for the blank entry in VarIndexPtr structure
//
// Input:       NONE
//
// Output:      VAR_INDEX_ENTRY* - pointer to the blank entry, NUUL if no space
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VAR_INDEX_ENTRY* IndexGetBlankEntry()
{
    VAR_INDEX_ENTRY* Entry;
    //---Check if there is space in buffer-----
    if (VarIndexPtr->UsedSize<=VarIndexPtr->BufferSize-sizeof(VAR_INDEX_ENTRY))
    {
        Entry = (VAR_INDEX_ENTRY*)(VarIndexPtr->Buffer+VarIndexPtr->UsedSize);
        VarIndexPtr->UsedSize+=sizeof(VAR_INDEX_ENTRY);
    }
    //---If not - check in blanks--------
    else if (!DListEmpty(&VarIndexPtr->Blanks))
    {
        Entry = OUTTER(
                    VarIndexPtr->Blanks.pTail,
                    Link, VAR_INDEX_ENTRY
                );
        DListDelete(&VarIndexPtr->Blanks,&Entry->Link);
    }

    else
    {
        Entry = NULL;
    }

    return Entry;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexAddVariable
//
// Description: This function adds new Var 
//
// Input:       NVAR *Nvar - pointer to Var 
//              IN NVRAM_STORE_INFO *Info - pointer to NVRAM_STORE_INFO structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexAddVariable(
    IN NVAR *Nvar, IN NVRAM_STORE_INFO *Info
)
{
    DLIST *List;
    VAR_INDEX_ENTRY *Entry;
    UINTN i;
    VOID *NamePtr;

    if (VarIndexPtr==NULL) return ;

    List =  IndexGetListNvar(Nvar);
    Entry = IndexGetBlankEntry();

    if (Entry==NULL)
    {
        List->Size++;
        return;
    }

    Entry->Nvar=Nvar;
    Entry->FinalDataNvar=NULL;
    Entry->InfoIndex= (UINT32)(Info-&VarStoreInfo.NvramInfo[0]);
    ASSERT(Entry->InfoIndex<VarStoreInfo.InfoCount);
    NamePtr=NvGetName(Nvar);

    if (Nvar->flags&NVRAM_FLAG_ASCII_NAME)
    {
        CHAR8 *N = (CHAR8*)NamePtr+1;

        for (i=0; i<sizeof(Entry->Name)/sizeof(CHAR16)-1&&N[i]!=0; i++)
            Entry->Name[i]=N[i];
    }

    else
    {
        CHAR16 *N = (CHAR16*)NamePtr+1;

        for (i=0; i<sizeof(Entry->Name)/sizeof(CHAR16)-1&&N[i]!=0; i++)
            Entry->Name[i]=N[i];
    }

    Entry->Name[i]=0;
    DListAdd(List,&Entry->Link);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexUpdateVariable
//
// Description: This function searces for Var and if it is found updates it, or add new var otherwise
//
// Input:       NVAR *Nvar - pointer to Var 
//              IN NVRAM_STORE_INFO *Info - pointer to NVRAM_STORE_INFO structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexUpdateVariable(
    IN NVAR *Nvar, IN NVRAM_STORE_INFO *Info
)
{
    VAR_INDEX_ENTRY *Entry=NULL;
    DLIST *List =  IndexGetListNvar(Nvar);
    DLINK *Link;

    if (VarIndexPtr==NULL) return ;

    for (Link=List->pHead; Link!=NULL; Link=Link->pNext)
    {
        VAR_INDEX_ENTRY *TmpEntry = OUTTER(Link,Link, VAR_INDEX_ENTRY);

        if(!NvIsVariable(TmpEntry->Nvar,
                         &VarStoreInfo.NvramInfo[TmpEntry->InfoIndex]
            )
        ) continue;

        if (NvarEqNvar(TmpEntry->Nvar,&VarStoreInfo.NvramInfo[TmpEntry->InfoIndex],Nvar,Info))
        {
            Entry=TmpEntry;
            break;
        }
    }

    if (Entry==NULL)
    {
        IndexAddVariable(Nvar,Info);
    }
    else
    {
        Entry->Nvar=Nvar;
        Entry->FinalDataNvar=NULL;
        Entry->InfoIndex= (UINT32)(Info-&VarStoreInfo.NvramInfo[0]);
        ASSERT(Entry->InfoIndex<VarStoreInfo.InfoCount);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexDeleteVariable
//
// Description: This function deletes Var and adds it's entry in VarIndexPtr to blanks
//
// Input:       NVAR *Nvar - pointer to Var to delete
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexDeleteVariable(NVAR *Nvar)
{
    VAR_INDEX_ENTRY *Entry=NULL;
    DLIST *List =  IndexGetListNvar(Nvar);
    DLINK *Link;

    if (VarIndexPtr==NULL) return ;

    for (Link=List->pHead; Link!=NULL; Link=Link->pNext)
    {
        Entry = OUTTER(Link,Link, VAR_INDEX_ENTRY);

        if (Entry->Nvar==Nvar)
        {
            DListDelete(List,&Entry->Link);
            DListAdd(&VarIndexPtr->Blanks,&Entry->Link);
            return;
        }
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexDeleteVariableEntry
//
// Description: This function deletes Var and adds it's entry in VarIndexPtr to blanks
//
// Input:       VAR_INDEX_ENTRY *Entry - pointer to Entry to delete
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexDeleteVariableEntry(VAR_INDEX_ENTRY *Entry)
{
    DLIST *List;
    if (Entry == NULL) return;

    List = IndexGetListNvar(Entry->Nvar);
    
    if (VarIndexPtr==NULL) return ;

    DListDelete(List,&Entry->Link);
    DListAdd(&VarIndexPtr->Blanks,&Entry->Link);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexReset
//
// Description: This function resets the VarIndexPtr and updates VarStoreInfo structures
//
// Input:       NONE
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexReset()
{
    INTN i;

    if (VarIndexPtr==NULL) return ;

    VarIndexPtr->UsedSize=0;

    for (i=0; i<='z'-'a'; i++) DListInit(&VarIndexPtr->LetterIndex[i]);
    //---Reset fields in VarIndexPtr---------------
    DListInit(&VarIndexPtr->OtherIndex);
    DListInit(&VarIndexPtr->Blanks);

    for (i=(INTN)VarStoreInfo.InfoCount-1; i>=0; i--)
    {
        NVRAM_STORE_INFO *Info = &VarStoreInfo.NvramInfo[i];
        NVAR *Nvar=(NVAR*)Info->pFirstVar;

        if (!NvIsVariable(Nvar,Info)) continue;

        for (; Nvar!=NULL; Nvar=NvGetNextValid(Nvar,Info))
            IndexUpdateVariable(Nvar,Info);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexInit
//
// Description: This function allocates memory for Var Index Buffer and fils pointer to it in 
//              VarIndexPtr stucture
//
// Input:       NONE
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexInit()
{
    EFI_STATUS Status = pBS->AllocatePool(
                            EfiBootServicesData,
                            VARIABLE_INDEX_SIZE,
                            (VOID**)&VarIndexPtr->Buffer
                        );

    if (EFI_ERROR(Status))
    {
        VarIndexPtr=NULL;
        return;
    }
    VarIndexPtr->BufferSize = VARIABLE_INDEX_SIZE;
    IndexReset();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IndexExitBs
//
// Description: This function resets VarIndexPtr to NULL on Exit BS
//
// Input:       NONE
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID IndexExitBs()
{
    VarIndexPtr = NULL;
}

VOID IndexVirtualFixup()
{

}
#endif
///////////////////////////////////////////////////////////////////////////
//Basic operations
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsEnoughSpace
//
// Description: This function checks is there is Size space avaiable in NVRAM_STORE_INFO
//
// Input:       NVRAM_STORE_INFO* Info - pointer to NVRAM_STORE_INFO stucture
//              UINTN Size - Size to check for
//
// Output:      BOOLEAN - TRUE if space avaiable, FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsEnoughSpace(NVRAM_STORE_INFO* Info, UINTN Size)
{
    return (   Info->pEndOfVars + Size
               <= Info->NvramAddress+Info->NvramSize - sizeof(EFI_GUID)*(Info->NextGuid+1) - sizeof(NVAR)
           );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RecoverFromFlashUpdateFailure
//
// Description: This function checks is there was a flash update failure and signal about it
//              to CheckStore
//
// Input:       NVRAM_STORE_INFO* Info - pointer to NVRAM_STORE_INFO stucture
//              VOID *EndOfVars - End of Vars pointer
//              UINTN DataSize - Size of data, that was programed
//              NVAR *PrevVar - pointer to previous instance of Var
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID RecoverFromFlashUpdateFailure(
    NVRAM_STORE_INFO *Info, VOID *EndOfVars, UINTN DataSize, NVAR *PrevVar 
)
{
    UINT8 *p;
    NVAR *NewVar;
    
    //Nothing has been programmed. Just return
    if (EndOfVars==Info->pEndOfVars) return;
    
    NewVar = (NVAR*)Info->pEndOfVars;
    //Check if variable size has been programmed correctly
    if (NewVar->size==DataSize)
    {
        // Let's see if the signature is correct.
        // If the signature is correct, let's see if 
        // the 'next' field has been properly updated.
        if (   NewVar->signature==NvramSignature
            && NvGetDataNvar(PrevVar, Info) != NewVar
        ){
            // if we are here, the next field has not been properly udpated
            // leave pInfo->pEndOfVars at the address of the last record.
            Info->LastVarSize=(VAR_SIZE_TYPE)DataSize;
            return;
        }
        //Yes, the size has been programmed correctly.
        //NvGetNextNvar routine is smart enough to skip
        //invalid records with valid size.
        Info->pEndOfVars+=DataSize;//Update Info->pEndOfVars
        Info->LastVarSize=0;//Opearaion failed, set Info->LastVarSize to zero
        return;
    }
    
    //check if something has actually been programmed
    for (p=Info->pEndOfVars; p<(UINT8*)EndOfVars && *p==FLASH_EMPTY_BYTE; p++) ;
    
    //if nothing has been programmed, just return
    if (p==EndOfVars) return;
    
    //if we reached this point, NVRAM store is corruped
    //Set pInfo->pEndOfVars at the address of corruption.
    //This will trigger recovery operation
    //(See 'Error Recovery' section at the end of the DxeSetVariable routine).
    Info->pEndOfVars = p;
    Info->LastVarSize=(VAR_SIZE_TYPE)DataSize;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetVarAuthExtFlags
//
// Description: This function returns Security fields Mc and KeyHash form existing 
//              Var in Var store
//              It is called called every time variable with one of
//              UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES is being verified.
//
// Input:       NVAR *Var pointer to the 1st instance of Var
//              NVRAM_STORE_INFO *pInfo - pointer to NVRAM store structure
//              OUT EXT_SEC_FLAGS->MonotonicCount - value of MC or TIME stamp 
//              OUT EXT_SEC_FLAGS->KeyHash - pointer to memory, allocated by caller, 
//              where Hash of PublicKeyDigest will be returned.
//
// Output:      *EXT_SEC_FLAGS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
GetVarAuthExtFlags(
    IN NVAR *Var, 
    IN NVRAM_STORE_INFO *pInfo, 
    OUT EXT_SEC_FLAGS *ExtFlags
)
{
    if(Var==NULL || pInfo==NULL || ExtFlags==NULL)
        return;
// Return Authenticate attributes along with NonVolitile attrib
    if ( pInfo->Flags & NVRAM_STORE_FLAG_NON_VALATILE )
        ExtFlags->AuthFlags |=EFI_VARIABLE_NON_VOLATILE;

    if(Var->flags & (NVRAM_FLAG_AUTH_WRITE | NVRAM_FLAG_EXT_HEDER)) {
        ExtFlags->AuthFlags |= (UINT8)(*NvGetExtFlags (Var) & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES) ;//(NVRAM_eFLAG_TIME_BASED_AUTH_WRITE | NVRAM_eFLAG_AUTH_WRITE));
        ExtFlags->Mc = (*(UINT64*)(NvGetExtFlags (NvGetDataNvar(Var, pInfo)) + 1));
        MemCpy(&ExtFlags->KeyHash[0], (VOID*)(NvGetExtFlags (Var) + 1 + 8), HASH_SHA256_LEN);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FlashMemFlashWrite
//
// Description: This function used to copy some information from one part of 
//      flash to another, when flash is not memory mapped. It uses temporary 
//      buffer one or more times.
//
// Input:       IN VARIABLE_INTERFACE *Interface - pointer to Variable interface
//              IN OUT UINT8 **PointerTo - Destination
//              IN VOID *From - Source
//              IN UINTN SizeToWrite - in bytes.
//              IN NVRAM_STORE_INFO *Info - pointer to NVRAM_STORE_INFO stucture
//
// Output:      *EXT_SEC_FLAGS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END> 
BOOLEAN 
FlashMemFlashWrite(
    IN VARIABLE_INTERFACE *Interface, 
    IN OUT UINT8 **PointerTo, 
    IN VOID *From, 
    IN UINTN SizeToWrite,
    IN NVRAM_STORE_INFO *Info
)
{
    UINT8   TempMemory [256];
    UINT8   *TempFrom = (UINT8*)From;
    UINT16  TempSize;
    BOOLEAN Result;
    
    do
    {
        Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);	
        if (SizeToWrite <= 256)
            TempSize = (UINT16) SizeToWrite;
        else 
        {
            TempSize = 256;
            SizeToWrite -= 256;
        }
        MemCpy(TempMemory, TempFrom, TempSize);
        Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);
        Result = Interface->WriteBytes(*PointerTo, TempSize, TempMemory);
        *PointerTo += TempSize;
        if ((TempSize < 256) || (SizeToWrite == 0) || !Result) break;
        TempFrom += TempSize;
    }
    while (TRUE);
    return Result;
}
#if AuthVariable_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindVariable
//
// Description: This function searches for Var with specific GUID and Name
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OUT UINT32* Attributes - Pointer to memory where Attributes will be returned 
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var is loacted
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FindVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes ,
    IN OUT UINTN *DataSize, OUT VOID **Data
)
{
    UINTN VariableNameSize;
    NVRAM_STORE_INFO *Info;
    NVAR *Nvar;

    Info = VarStoreInfo.NvInfo;
    Nvar = (NVAR*)NvFindVariable(VariableName,VendorGuid,&VariableNameSize,Info);

    if ((Nvar != NULL) && (Data != NULL))
    {
//        if (Runtime && !(Nvar->flags & NVRAM_FLAG_RUNTIME)) return EFI_NOT_FOUND;
        if (Attributes)
            if (EFI_ERROR(NvGetAttributesFromNvar(Nvar, Info, Attributes)))
                return EFI_NOT_FOUND;
            
        *Data = (UINT8*)NvGetData(Nvar, VariableNameSize, DataSize, Info);

        return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND;
}
#endif
                
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CreateVariableEx
//
// Description: This function adds new variable to Varstore
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINT32 Attributes - attributes of Variable
//              IN UINTN DataSize - size of Var data
//              IN VOID *Data - pointer to the Var data
//              OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//              (OPTIONAL) NVAR* OldVar - in case of garbage collection pointer to the first i
//              nstance of variable to get Monotonic Counter and Flags from
//              (OPTIONAL) NVRAM_STORE_INFO *FromInfo - in case of garbage collection pointer to the
//              store info strucrure - to get Public Key from
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateVariableEx(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data,
    NVRAM_STORE_INFO *Info, VARIABLE_INTERFACE *Interface, OPTIONAL NVAR* OldVar, EXT_SEC_FLAGS *ExtSecFlags, 
    OPTIONAL VOID *AppendData, OPTIONAL UINTN AppendDataSize
)
{
    UINTN Size=sizeof(NVAR) + AppendDataSize, ExtSize = 0;
    NVAR Var = {FLASH_EMPTY_SIGNATURE,0,FLASH_EMPTY_NEXT,NVRAM_FLAG_VALID};
    CHAR16* s;
    EFI_GUID* Guid;
    INT16 guid=0;
    BOOLEAN ok;
    UINT8 *p, *p1, Dummy, ExtFlags = 0;

    if (Attributes & EFI_VARIABLE_RUNTIME_ACCESS)
        Var.flags |= NVRAM_FLAG_RUNTIME;
#if AuthVariable_SUPPORT
    if ( Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD )
        Var.flags |= NVRAM_FLAG_HARDWARE_ERROR_RECORD;
    if (Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES/*EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS*/)
    {
        Var.flags |= NVRAM_FLAG_EXT_HEDER | NVRAM_FLAG_AUTH_WRITE;
        ExtSize = 1 + 8 + HASH_SHA256_LEN + 2; //ExtFlags + Monotonic Counter + PublicKeyDigest Hash + ExtSize
        ExtFlags |= Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES;
        // We store PublicKeyDigest Hash only in the first instance of Var in store to save space
    }
#endif
    if (NvramChecksumSupport) 
    {
        Var.flags |= NVRAM_FLAG_EXT_HEDER;
        if (ExtSize == 0) ExtSize += 4; // ExtSize - Ext Flag, Checksumm and Size
        else ExtSize += 1; // Add Checksumm Feild
        ExtFlags |= NVRAM_EXT_FLAG_CHECKSUM;
    }
        
    //find GUID in the GUID area
    for ( Guid=Info->NvramGuidsAddress
               ;   Guid>Info->NvramGuidsAddress-Info->NextGuid
            && guidcmp(Guid,VendorGuid)
            ; Guid--
        ) guid++;
        
    if (((OldVar != NULL) && (OldVar->flags & NVRAM_FLAG_GUID)) || (Info->NextGuid>255 && guid>=Info->NextGuid))
    {
        Var.flags |= NVRAM_FLAG_GUID;
        Size+=sizeof(EFI_GUID);
    }
    
    else
    {
        Size+=1;
    }
    
    //Alalize the name. Check if it has non basic Latin characters
    for (s=VariableName; *s&&!*((CHAR8*)s+1); s++) ;
    
    if (!*s)
    {
        Var.flags |= NVRAM_FLAG_ASCII_NAME;
        Size+=s-VariableName+1;
    }
    
    else
    {
        while (*s++);
        
        Size+=(UINT8*)s-(UINT8*)VariableName;
    }
    
    Size += DataSize + ExtSize;
    
    if (!IsEnoughSpace(Info,Size)) return EFI_OUT_OF_RESOURCES;
    
    Var.size=(VAR_SIZE_TYPE)Size;
    Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);
    p = Info->pEndOfVars;
    
    do
    {
        //Program Var at pEndOfVars
        ok=Interface->WriteBytes(p,sizeof(NVAR),&Var);
        p += sizeof(NVAR);
        
        if (!ok) break;
        
        //Program Guid
        if (Var.flags&NVRAM_FLAG_GUID)
        {
            //program the whole guid
            ok=Interface->WriteBytes(p,sizeof(EFI_GUID),VendorGuid);
            p+=sizeof(EFI_GUID);
        }
        
        else
        {
            //program the whole guid at NVRAM_GUIDS_ADDRESS - NextGuid
            if (guid==Info->NextGuid)
            {
                ok=Interface->WriteBytes(
                       (UINT8*)(Info->NvramGuidsAddress-Info->NextGuid),
                       sizeof(EFI_GUID),VendorGuid
                   );
                Info->NextGuid++;
                
                if (!ok) break;
            }
            
            //program guid(1 byte) at pEndOfVars+sizeof(NVAR);
            ok=Interface->WriteBytes(p++,1,&guid);
        }
        
        if (!ok) break;
        
        //Program name and data
        if (Var.flags&NVRAM_FLAG_ASCII_NAME)
        {
            UINT8* q;
            
            for (q=(UINT8*)VariableName; *(CHAR16*)q && ok ; q+=2,p++)
                ok = Interface->WriteBytes(p,1,q);
                
            if (ok) ok = Interface->WriteBytes(p++,1,q); //zero at end
        }
        
        else
        {
            UINT16* q;
            
            for (q=VariableName; *q && ok ; q++,p+=2)
                ok = Interface->WriteBytes(p,2,q);
                
            if (ok)
            {
                ok = Interface->WriteBytes(p,2,q); //zero at end
                p+=2;
            }
        }
        
        if (!ok) break;

        //AppendWrite after Garbage collection
        if (Attributes & EFI_VARIABLE_APPEND_WRITE) {  
            if (FlashNotMemoryMapped){
                ok=FlashMemFlashWrite(Interface, &p, AppendData, AppendDataSize, Info);
            }    
            else
            {
                ok=Interface->WriteBytes(p,AppendDataSize,AppendData);
                p+=AppendDataSize;
            }
            if (!ok) break;
        }
        
        //Program data
        ok=Interface->WriteBytes(p,DataSize,Data);
        p+=DataSize;
        p1=p; 
        
        if (!ok) break;
        
        //---Fill Ext area---
        if (Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES/*EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS*/)
        {   
            ok=Interface->WriteBytes(p, 1, &ExtFlags);//--Ext Flag 
            if (!ok) break;
            p += 1;
            ok=Interface->WriteBytes(p, 8, &ExtSecFlags->Mc);//--Monotonic counter
            if (!ok) break;
            p += 8;
            ok=Interface->WriteBytes(p, HASH_SHA256_LEN, &ExtSecFlags->KeyHash[0]);//--Public Key Hash
            if (!ok) break;
            p = Info->pEndOfVars + Size - 2;
            ok=Interface->WriteBytes(p, 2,&ExtSize);//--Ext Size 
            if (!ok) break;
         }
        if (NvramChecksumSupport)
        {   
            if (!(Var.flags & NVRAM_FLAG_AUTH_WRITE)) // ???NVRAM_FLAG_EXT_HEDER
            {   
                p = p1 + ExtSize; 
                //write ExtSize
                ok=Interface->WriteBytes(p-sizeof(VAR_SIZE_TYPE),sizeof(VAR_SIZE_TYPE),&ExtSize);
                if (!ok) break;
			    //write ExtFlags
                ok=Interface->WriteBytes(p1, 1, &ExtFlags);
                if (!ok) break;
			    //write checksum
                if (FlashNotMemoryMapped)
                    Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);	

                Dummy = NvCalculateNvarChecksum((NVAR*)Info->pEndOfVars);

                if (FlashNotMemoryMapped)
                    Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);

			    //adjust to exclude empty checksum field
                Dummy += FLASH_EMPTY_BYTE;
                ok=Interface->WriteBytes(p-3, 1, &Dummy);
                if (!ok) break;
            }
            else
            {
                p = p1 + ExtSize; 
                if (FlashNotMemoryMapped)
                    Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);	

                Dummy = NvCalculateNvarChecksum((NVAR*)Info->pEndOfVars);

			    if (FlashNotMemoryMapped)
                    Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);
                //adjust to exclude empty checksum field
                Dummy += FLASH_EMPTY_BYTE;
                ok=Interface->WriteBytes(p-3, 1, &Dummy);
                if (!ok) break;
            }
        }
        //write a signature
        ok=Interface->WriteBytes(
               Info->pEndOfVars,sizeof(NvramSignature),(VOID*)&NvramSignature
           );

    }
    while (FALSE);
    
    Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);
    
    if (ok)
    {
        Info->pEndOfVars += Size;
        Info->LastVarSize=(VAR_SIZE_TYPE)Size;
    }
    
    else
    {
        RecoverFromFlashUpdateFailure(Info,p,Size,(NVAR*)Info->pEndOfVars);
    }
    
    return (ok) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

EFI_STATUS CreateVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data,
    NVRAM_STORE_INFO *Info, VARIABLE_INTERFACE *Interface, OPTIONAL NVAR* OldVar, EXT_SEC_FLAGS *ExtSecFlags
)
{
    return CreateVariableEx(VariableName, VendorGuid, Attributes, DataSize, 
                            Data, Info, Interface, OldVar, ExtSecFlags, NULL, 0);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateVariable
//
// Description: This function updates variable 
//
// Input:       IN NVAR *OldVar - pointer to the old Var
//              IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN UINTN NameSize - size of the Var mane
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINT32 Attributes - attributes of Variable
//              IN UINTN DataSize - size of Var data
//              IN VOID *Data - pointer to the Var data
//              OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS UpdateVariable(
    IN NVAR *OldVar, IN CHAR16 *VariableName, IN UINTN NameSize,
    IN EFI_GUID *VendorGuid, IN UINT32 Attributes,
    IN UINTN DataSize, IN VOID *Data,
    IN OUT VAR_INDEX_ENTRY *OldVarEntry OPTIONAL,
    IN EXT_SEC_FLAGS *ExtSecFlags,
    NVRAM_STORE_INFO *Info, VARIABLE_INTERFACE *Interface
)
{
    UINTN OldDataSize;
    VOID *OldData;
    BOOLEAN ok;
    UINTN Size, ExtSize = 0;
    NVAR Var = {FLASH_EMPTY_SIGNATURE,0,FLASH_EMPTY_NEXT,NVRAM_FLAG_VALID|NVRAM_FLAG_DATA_ONLY};
    UINT8 *p, *p1, Dummy, ExtFlags = 0;

    if (!NvAttribEq(OldVar,Attributes,Info)) return EFI_INVALID_PARAMETER;
    
    if (OldVarEntry != NULL && OldVarEntry->FinalDataNvar != NULL)
        OldData = NvGetData( OldVarEntry->FinalDataNvar, NameSize, &OldDataSize, Info );
    else
        OldData = NvGetData( OldVar, NameSize, &OldDataSize, Info );

    if (Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES/*EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS*/)
    {
        Var.flags |= NVRAM_FLAG_EXT_HEDER | NVRAM_FLAG_AUTH_WRITE;
        ExtSize = 1 + 8 + 2; //ExtFlags + Monotonic Counter + ExtSize
        // AuthFlags only set in 1st instance
        // ExtFlags |= Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES;
    }
    //if new data equal to the old data don't do anything
    if (NvramChecksumSupport)
    {   ExtFlags |= NVRAM_EXT_FLAG_CHECKSUM;
        if (ExtSize == 0) ExtSize += 4; // ExtSize - Ext Flag, Checksumm and Size
        else ExtSize += 1; // Add Checksumm Feild
        if (    (Attributes & EFI_VARIABLE_APPEND_WRITE) == 0 
		     && OldDataSize == DataSize && !MemCmp(OldData, Data, DataSize) 
			 && (OldVar->flags & NVRAM_FLAG_EXT_HEDER))
            return EFI_SUCCESS;
        else Var.flags |= NVRAM_FLAG_EXT_HEDER;
    }
    else
        if (    (Attributes & EFI_VARIABLE_APPEND_WRITE) == 0 
			 && OldDataSize == DataSize && !MemCmp(OldData, Data, DataSize)) 
            return EFI_SUCCESS;

    // Append write implementation     
    if (Attributes & EFI_VARIABLE_APPEND_WRITE)   
        Size=sizeof(NVAR)+ OldDataSize + DataSize + ExtSize;
    else
        Size=sizeof(NVAR)+ DataSize + ExtSize;
    
    if (!IsEnoughSpace(Info,Size)) return EFI_OUT_OF_RESOURCES;
    
    Var.size=(VAR_SIZE_TYPE)Size;
    Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);
    p = Info->pEndOfVars;
    
    //Program Var at pEndOfVars
    do
    {
        ok=Interface->WriteBytes(p,sizeof(NVAR),&Var);
        p += sizeof(NVAR);
        
        if (!ok) break;
        
        //Program data
        //AppendWrite 
        if (Attributes & EFI_VARIABLE_APPEND_WRITE) {  
            if (FlashNotMemoryMapped){
                ok=FlashMemFlashWrite(Interface, &p, OldData, OldDataSize, Info);
            }    
            else
            {
                ok=Interface->WriteBytes(p,OldDataSize,OldData);
                p+=OldDataSize;
            }
            if (!ok) break;
        }
        // end Append write
        ok=Interface->WriteBytes(p,DataSize,Data);
        p+=DataSize;
        p1 = p;
        if (!ok) break;

        //---Fill Ext area---
        if (Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES/*EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS*/)
        {   
            ok=Interface->WriteBytes(p, 1, &ExtFlags);//--Ext Flag 
            if (!ok) break;
            p += 1;
            ok=Interface->WriteBytes(p, 8, &ExtSecFlags->Mc);//--Monotonic counter
            if (!ok) break;
            p = Info->pEndOfVars + Size - 2;
            ok=Interface->WriteBytes(p, 2,&ExtSize);//--Ext Size 
            if (!ok) break;
         }
        if (NvramChecksumSupport)
        {   
            if (!(Var.flags & NVRAM_FLAG_AUTH_WRITE)) // ??? should be ExtFlags?
            {   
                p = p1 + ExtSize; 
                //write ExtSize
                ok=Interface->WriteBytes(p-sizeof(VAR_SIZE_TYPE),sizeof(VAR_SIZE_TYPE),&ExtSize);
                if (!ok) break;
			    //write ExtFlags
                ok=Interface->WriteBytes(p-ExtSize, 1, &ExtFlags);
                if (!ok) break;
			    //write checksum
                if (FlashNotMemoryMapped)
                    Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);	

                Dummy = NvCalculateNvarChecksum((NVAR*)Info->pEndOfVars);

                if (FlashNotMemoryMapped)
                    Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);

			    //adjust to exclude empty checksum field
                Dummy += FLASH_EMPTY_BYTE;
                ok=Interface->WriteBytes(p-3, 1, &Dummy);
                if (!ok) break;
            }
            else
            {
                p = p1 + ExtSize; 
                if (FlashNotMemoryMapped)
                    Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);	

                Dummy = NvCalculateNvarChecksum((NVAR*)Info->pEndOfVars);

                if (FlashNotMemoryMapped)
                    Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);

			        //adjust to exclude empty checksum field
                Dummy += FLASH_EMPTY_BYTE;
                ok=Interface->WriteBytes(p-3, 1, &Dummy);
                if (!ok) break;
            }
        }    
        //write a signature
        ok=Interface->WriteBytes(Info->pEndOfVars,sizeof(NvramSignature),(VOID*)&NvramSignature);
        
        if (!ok) break;
        //set Var.next;

        if (FlashNotMemoryMapped)
            Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);	
            
        if (OldVarEntry != NULL && OldVarEntry->FinalDataNvar != NULL)
            OldVar=OldVarEntry->FinalDataNvar;
        else
            OldVar=NvGetDataNvar(OldVar,Info);

        if (FlashNotMemoryMapped)
            Interface->EnableUpdate(Info->NvramAddress,Info->NvramSize);

        Var.next=(UINT32)(Info->pEndOfVars-(UINT8*)OldVar);
        ok=Interface->WriteBytes((UINT8*)OldVar+NEXT_OFFSET,NEXT_SIZE,(UINT8*)&Var+NEXT_OFFSET);
    }
    while (FALSE);
    
    Interface->DisableUpdate(Info->NvramAddress,Info->NvramSize);
    
    if (ok)
    {
		if (OldVarEntry != NULL)
	        OldVarEntry->FinalDataNvar = (NVAR*)Info->pEndOfVars;
        Info->pEndOfVars += Size;
        Info->LastVarSize=(VAR_SIZE_TYPE)Size;
    }
    
    else
    {
        RecoverFromFlashUpdateFailure(Info,p,Size,OldVar);
    }
    
    return (ok) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DeleteVariable
//
// Description: This function delets variable 
//
// Input:       IN NVAR *Var - pointer to the Var to be deleted
//              OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DeleteVariable(IN NVAR *Var, NVRAM_STORE_INFO *Info, VARIABLE_INTERFACE *Interface)
{
    UINT8 flag = Var->flags^NVRAM_FLAG_VALID;//invert validity bit
    BOOLEAN ok;
    CHAR8 *WriteAddress = (CHAR8*)Var+FLAG_OFFSET;
    UINTN WriteSize = FLAG_SIZE;
    Interface->EnableUpdate(WriteAddress,WriteSize);
    ok=Interface->WriteBytes(WriteAddress,WriteSize,&flag);
    Interface->DisableUpdate(WriteAddress,WriteSize);
    return (ok) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CopyVariable
//
// Description: This function copyes variable from one Nvram store to another
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINTN DataSize - size of Var data
//              IN VOID *Data - pointer to the Var data temprary storadge
//              OUT NVRAM_STORE_INFO *FromInfo -  pointer to NVRAM_STORE_INFO structure to copy from
//              OUT NVRAM_STORE_INFO *ToInfo -  pointer to NVRAM_STORE_INFO structure to copy to
//              VARIABLE_INTERFACE *ToInterface - pointer to the interface with the Variable routins
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CopyVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
//Buffer used for temporary variable data storage
    IN UINTN DataSize, IN VOID *Data,
    NVRAM_STORE_INFO *FromInfo, NVRAM_STORE_INFO *ToInfo,
    VARIABLE_INTERFACE *ToInterface
)
{
    EFI_STATUS Status;
    UINT32 Attributes;
    NVAR *nVar;
    EXT_SEC_FLAGS ExtSecFlags = {0,0,{0}};
    
    Status = NvGetVariable(
                 VariableName, VendorGuid, &Attributes, &DataSize, Data,
                 FromInfo, &nVar
             );
             
    if (EFI_ERROR(Status)) return Status;

    GetVarAuthExtFlags(nVar, FromInfo, &ExtSecFlags);

    if (nVar->flags & NVRAM_FLAG_AUTH_WRITE)
        return CreateVariable(
               VariableName, VendorGuid, Attributes, DataSize, Data,
               ToInfo, ToInterface, nVar, &ExtSecFlags);
    else
        return CreateVariable(
               VariableName, VendorGuid, Attributes, DataSize, Data,
               ToInfo, ToInterface, nVar, NULL);
}

//Varstore maintenance
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SkipSpecificNvar
//
// Description: Helper varstore filtering function. 
//              Skips the variable that corresponds to the passed in NVAR.
//
// Output:      BOOLEAN
//                  TRUE - skip the variable
//                  FALSE - do not skip the variable
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SkipSpecificNvar(
    NVAR *SkipVar, CHAR16* VarName, EFI_GUID* VarGuid, 
    NVRAM_STORE_INFO *Info
){
    return SkipVar!=NULL && NvVarEq(SkipVar, VarName, VarGuid, NULL, Info);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SkipVarsWithDefaults
//
// Description: Helper varstore filtering function. 
//              Skips the variables that have defaults.
//
// Output:      BOOLEAN
//                  TRUE - skip the variable
//                  FALSE - do not skip the variable
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SkipVarsWithDefaults(
    NVRAM_STORE_INFO *DefInfo, CHAR16* VarName, EFI_GUID* VarGuid,
    NVRAM_STORE_INFO *Info
){
    return    DefInfo!=NULL 
           && (    Wcscmp(VarName, (CHAR16*)StdDefaults)==0
                || Wcscmp(VarName, (CHAR16*)MfgDefaults)==0
                || NvFindVariable(VarName, VarGuid, NULL, DefInfo) != NULL
              );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CopyVariables
//
// Description: This function copies variables from one varstore to another
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure to copy from
//              OUT NVRAM_STORE_INFO *NewInfo -  pointer to NVRAM_STORE_INFO structure to copy to
//              VARIABLE_INTERFACE *NewInterface - pointer to the interface with the update routins
//              COPY_VAR_STORE_FILTER FilterFunction - filtering function
//              VOID *FilterContext - filtering function context
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CopyVariables(
    IN NVRAM_STORE_INFO* Info,
    OUT NVRAM_STORE_INFO *NewInfo,
    VARIABLE_INTERFACE *NewInterface,
    COPY_VAR_STORE_FILTER FilterFunction,
    VOID *FilterContext
)
{
    EFI_STATUS Status;
    VOID *TmpBuffer, *TmpData;
    CHAR16 *TmpName;
    UINTN NvramSize = Info->NvramSize;
    EFI_GUID Guid;
    
    TmpBuffer = SelectiveAllocate (TRUE, NvramSize*2);   
    
    if (TmpBuffer==NULL) return EFI_OUT_OF_RESOURCES;
    
    TmpName = TmpBuffer; TmpData = (UINT8*)TmpBuffer+NvramSize;
    TmpName[0]=0;
    
    //Copy variables
    do
    {
        UINTN Size = NvramSize;
        Status = NvGetNextVariableName(
                     &Size, TmpName, &Guid, Info, FALSE
                 );
                 
        if (EFI_ERROR(Status)) break;
        
        //check if the variable needs to be skipped
        if (   FilterFunction!=NULL 
            && FilterFunction(FilterContext, TmpName, &Guid, Info)
        ) continue;
            
        Status=CopyVariable(
                   TmpName, &Guid, NvramSize, TmpData, Info, NewInfo, NewInterface
               );
               
        if (EFI_ERROR(Status))
        {
            if (Status==EFI_NOT_FOUND) continue;
            
            SelectiveFree (TmpBuffer);
            return Status;
        }
    }
    while (TRUE);
    
    SelectiveFree (TmpBuffer);
    
    if (Status==EFI_NOT_FOUND) Status=EFI_SUCCESS;
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CopyVarStore
//
// Description: This function copyes varstore to a new location
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure to copy from
//              OUT NVRAM_STORE_INFO *NewInfo -  pointer to NVRAM_STORE_INFO structure to copy to
//              VARIABLE_INTERFACE *NewInterface - pointer to the interface with the update routins
//              COPY_VAR_STORE_FILTER FilterFunction - filtering function
//              VOID *FilterContext - filtering function context
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CopyVarStore(
    IN NVRAM_STORE_INFO* Info,
    OUT NVRAM_STORE_INFO *NewInfo,
    VARIABLE_INTERFACE *NewInterface,
    COPY_VAR_STORE_FILTER FilterFunction,
    VOID *FilterContext
)
{
    BOOLEAN ok;
    
    //This function assumes that the following
    //fields of NewInfo have been initialized:
    //  NvramAddress, NvramSize, NvramGuidsAddress, Flags
    if ((Runtime) && (!NvramDriverBuffer)) return EFI_OUT_OF_RESOURCES;
    
    NewInterface->EnableUpdate(NewInfo->NvramAddress,NewInfo->NvramSize);
    ok=NewInterface->EraseBytes(NewInfo->NvramAddress,NewInfo->NvramSize);
    NewInterface->DisableUpdate(NewInfo->NvramAddress,NewInfo->NvramSize);
    NewInfo->pFirstVar = NewInfo->NvramAddress + VarStoreInfo.HeaderLength;
    NewInfo->pEndOfVars = NewInfo->pFirstVar;
    NewInfo->LastVarSize = 0;
    NewInfo->pLastReturned = NULL;
    NewInfo->NextGuid = 0;
    
    if (!ok) return EFI_DEVICE_ERROR;
    
    //Copy the header
    NewInterface->EnableUpdate(NewInfo->NvramAddress,NewInfo->NvramSize);
    ok=NewInterface->WriteBytes(
           NewInfo->NvramAddress,
           VarStoreInfo.HeaderLength,
           Info->NvramAddress
       );
    NewInterface->DisableUpdate(NewInfo->NvramAddress,NewInfo->NvramSize);
    
    if (!ok) return EFI_DEVICE_ERROR;
    
    
    return CopyVariables(Info,NewInfo,NewInterface,FilterFunction,FilterContext);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CopyVarStoreToMemStore
//
// Description: This function allocates memory copyes varstore to a new location
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure to copy from
//              IN NVAR* SkipVar - pointer to var to be skiped
//              OUT NVRAM_STORE_INFO *NewInfo -  pointer to NVRAM_STORE_INFO structure which was created
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CopyVarStoreToMemStore(
    IN NVRAM_STORE_INFO* Info,
    IN NVAR* SkipVar, OUT NVRAM_STORE_INFO *NewInfo
)
{
    EFI_STATUS Status;
    
    if ((Runtime) && (!NvramDriverBuffer)) return EFI_OUT_OF_RESOURCES;
    if (!Runtime) PROGRESS_CODE(DXE_NVRAM_CLEANUP);
    NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM Garbage Collection\n"));
    
    Status = InitVolatileStore(
                 NewInfo, Info->NvramSize, VarStoreInfo.HeaderLength, Info->Flags, FALSE
             );
             
    if (EFI_ERROR(Status)) return Status;
    
    Status = CopyVarStore(
        Info,NewInfo,&MemInterface,SkipSpecificNvar,SkipVar
    );
    
    if (EFI_ERROR(Status)) SelectiveFree (NewInfo->NvramAddress);
    else CheckTheHeader(NewInfo, VarStoreInfo.HeaderLength, TRUE);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetNvramFfsFileState
//
// Description: This function updates State of Ffs file with new bits described by NewState
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//              EFI_FFS_FILE_STATE NewState - New state to update to
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetNvramFfsFileState(
    NVRAM_STORE_INFO *Info, VARIABLE_INTERFACE *Interface,
    EFI_FFS_FILE_STATE NewState
)
{
    BOOLEAN ok;
    EFI_FFS_FILE_STATE *StatePtr = GetNvramFfsFileStatePtr(Info);
    
    if (StatePtr==NULL) return EFI_DEVICE_ERROR;
    
    if (FlashEmpty!=0) NewState = ~NewState & *StatePtr;
    else               NewState |= *StatePtr;
    
    Interface->EnableUpdate(StatePtr,sizeof(NewState));
    ok = Interface->WriteBytes(StatePtr,sizeof(NewState),&NewState);
    Interface->DisableUpdate(StatePtr,sizeof(NewState));
    return (ok) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ResetFfsFileStateInMemory
//
// Description: This function sets State of Ffs file to new value described by NewState
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              EFI_FFS_FILE_STATE NewState - New state to set
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ResetFfsFileStateInMemory(
    NVRAM_STORE_INFO *Info, EFI_FFS_FILE_STATE NewState
)
{
    EFI_FFS_FILE_STATE *StatePtr = GetNvramFfsFileStatePtr(Info);
    
    if (StatePtr==NULL) return EFI_DEVICE_ERROR;
    
    if (FlashEmpty!=0) NewState = ~NewState;
    
    *StatePtr = NewState;
    return (*StatePtr == NewState) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckTheHeader
//
// Description: This function validates correctness of the header fields used by the driver
//              and fixes them if Update parameter is TRUE
//
// Input:       IN NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure to check
//              IN UINT32 HeaderLength - length of the NVRAM header
//              IN BOOLEAN Update -  If TRUE, invalid header will be updated to fix the problem
//
// Output:      BOOLEAN
//                TRUE - No problems detected
//                FALSE - Problem(s) detected
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CheckTheHeader(NVRAM_STORE_INFO *Info, UINT32 HeaderLength, BOOLEAN Update){
    BOOLEAN HeaderValid = TRUE;
    EFI_FIRMWARE_VOLUME_HEADER *Fv = (EFI_FIRMWARE_VOLUME_HEADER*)Info->NvramAddress;

    //check the signature
    if (   Fv->Signature!=FV_SIGNATURE 
        || Fv->FvLength!=Info->NvramSize
        || Fv->HeaderLength!=HeaderLength-sizeof(EFI_FFS_FILE_HEADER)
    ){
        if (Update){
            Fv->Signature=FV_SIGNATURE;
            Fv->FvLength=Info->NvramSize;
            Fv->HeaderLength=(UINT16)(HeaderLength-sizeof(EFI_FFS_FILE_HEADER));
        }
        HeaderValid = FALSE;
    }
    if (!IsMainNvramStoreValid(Info, VarStoreInfo.BackupAddress, NULL)){
        if (Update){
            ResetFfsFileStateInMemory(
               Info,
                 EFI_FILE_HEADER_CONSTRUCTION 
               | EFI_FILE_HEADER_VALID 
               | EFI_FILE_DATA_VALID
            );
        }
        HeaderValid = FALSE;
    }
    return HeaderValid;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateFtVarstore
//
// Description: This function copyes Varstore to a new location and makes this copy main Varstore
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//              NVRAM_STORE_INFO* NewInfo - pointer to the new NVRAM_STORE_INFO Varstore 
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS UpdateFtVarstore(
    NVRAM_STORE_INFO* Info, VARIABLE_INTERFACE *Interface, NVRAM_STORE_INFO* NewInfo
)
{
    NVRAM_STORE_INFO BackupInfo;
    EFI_STATUS Status;
    
    BackupInfo.NvramAddress = VarStoreInfo.BackupAddress;
    BackupInfo.NvramSize = Info->NvramSize;
    NvInitInfoBuffer(
        &BackupInfo, VarStoreInfo.HeaderLength, Info->Flags
    );
// State Transitions:
//    Main              Backup            Valid Store
// = Initial State After Firmware Flashing
// 1. DATA_VALID        HEADER_INVALID    Main
// = Update  Cycle 1
// 2. DATA_VALID        HEADER_VALID      Main
// 3. MARKED_FOR_UPDATE HEADER_VALID      Main
// 4. MARKED_FOR_UPDATE DATA_VALID        Backup
// = Update  Cycle 1 is Over
// = Update  Cycle 2
// 5. HEADER_VALID      DATA_VALID        Backup
// 6. HEADER_VALID      MARKED_FOR_UPDATE Backup
// 7. DATA_VALID        MARKED_FOR_UPDATE Main
// = Update  Cycle 2 is Over
// = Update  Cycle 3
// 8. DATA_VALID        HEADER_VALID      Main
// Stae 8 == State 2
    Status=ResetFfsFileStateInMemory(
               NewInfo, EFI_FILE_HEADER_CONSTRUCTION|EFI_FILE_HEADER_VALID
           );
           
    if (EFI_ERROR(Status)) return Status;
    
    Status = CopyVarStore(NewInfo,&BackupInfo,Interface,NULL,NULL);
    
    if (EFI_ERROR(Status)) return Status;
    
    Status=SetNvramFfsFileState(Info, Interface, EFI_FILE_MARKED_FOR_UPDATE);
    
    if (EFI_ERROR(Status)) return Status;
    
    Status=SetNvramFfsFileState(&BackupInfo, Interface, EFI_FILE_DATA_VALID);
    
    if (EFI_ERROR(Status)) return Status;
#ifdef HYBRID_NV_INTERFACE
    if (!SwapVarstoresInHybridMode(&BackupInfo)){
#endif
    VarStoreInfo.BackupAddress = Info->NvramAddress;
#ifdef HYBRID_NV_INTERFACE
	}
#endif
    *Info = BackupInfo;
    NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM areas swapped(NVRAM address: %X; Backup address: %X).\n", Info->NvramAddress, VarStoreInfo.BackupAddress));
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateVarstore
//
// Description: This function copyes Varstore to a new location and makes this copy main Varstore
//              if old varstore flag has NVRAM_STORE_FLAG_NON_VALATILE and Backupadress is valid, else it
//              just copy Varstor to new location
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//              NVRAM_STORE_INFO* NewInfo - pointer to the new NVRAM_STORE_INFO Varstore 
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS UpdateVarstore(
    NVRAM_STORE_INFO* Info, VARIABLE_INTERFACE *Interface, NVRAM_STORE_INFO* NewInfo
)
{
    EFI_STATUS Status;
    
    if (   (Info->Flags & NVRAM_STORE_FLAG_NON_VALATILE)
        && VarStoreInfo.BackupAddress!=0
        && CheckTheHeader(Info, VarStoreInfo.HeaderLength, FALSE)
    )
    {
        Status = UpdateFtVarstore(Info,Interface,NewInfo);
    }
    
    else
    {
        Status = CopyVarStore(NewInfo,Info,Interface,NULL,NULL);
    }
    
    if (!EFI_ERROR(Status))
    {
        if (Info==VarStoreInfo.NvInfo) NvramReinitialize();
        
#if NV_CACHE_SUPPORT
        else IndexReset();
        
#endif
    }
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvResetConfiguration
//
// Description: This function resets NvRam configuration
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//              VARIABLE_INTERFACE *Interface - pointer to the interface with the Variable routins
//              BOOLEAN ExternalDefaults - if TRUE - use external defaults
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NvResetConfiguration(
    NVRAM_STORE_INFO* Info, VARIABLE_INTERFACE *Interface, 
    BOOLEAN ExternalDefaults, BOOLEAN PreserveVariablesWithNoDefaults
)
{
    UINT16 HeaderLength = VarStoreInfo.HeaderLength;
    NVRAM_STORE_INFO NewInfo;
    NVRAM_STORE_INFO DefaultsInfo;
    NVRAM_STORE_INFO *DefInfo = NULL;
    EFI_STATUS Status;
    VOID *TmpBuffer;
    UINTN NvramSize = Info->NvramSize;
    BOOLEAN HeaderIsValid;

    PROGRESS_CODE(DXE_CONFIGURATION_RESET);
    NVRAM_TRACE((TRACE_DXE_CORE,
           "NVRAM Reset: ExternalDefaults=%d; PreserveVariablesWithNoDefaults=%d\n",
           ExternalDefaults,PreserveVariablesWithNoDefaults
    ));
    
    Status = InitVolatileStore(
                 &NewInfo, Info->NvramSize, VarStoreInfo.HeaderLength, Info->Flags, FALSE
             );
             
    if (EFI_ERROR(Status)) return Status;
    
    MemCpy(NewInfo.NvramAddress, Info->NvramAddress, HeaderLength);
    HeaderIsValid  = CheckTheHeader(&NewInfo, HeaderLength, TRUE);
    if (!HeaderIsValid){
        PreserveVariablesWithNoDefaults=FALSE;
        NVRAM_TRACE((TRACE_DXE_CORE, "NVRAM Reset: NVRAM FV header is corrupted\n"));
    }
    if (ExternalDefaults)
    {
        // {9221315B-30BB-46b5-813E-1B1BF4712BD3}
#define SETUP_DEFAULTS_FFS_GUID { 0x9221315b, 0x30bb, 0x46b5, { 0x81, 0x3e, 0x1b, 0x1b, 0xf4, 0x71, 0x2b, 0xd3 } }
        static EFI_GUID SetupDefaultsFfsGuid = SETUP_DEFAULTS_FFS_GUID;
        EFI_FIRMWARE_VOLUME_PROTOCOL *Fv;
        EFI_HANDLE *FvHandle;
        UINTN Number,i;
        UINT32 Authentication;

        Status = pBS->LocateHandleBuffer(
                     ByProtocol,&gEfiFirmwareVolume2ProtocolGuid, NULL, &Number, &FvHandle
                 );
        
        for (i=0; i<Number; i++)
        {
            Status=pBS->HandleProtocol(FvHandle[i], &gEfiFirmwareVolume2ProtocolGuid, (VOID**) &Fv);
            
            if (EFI_ERROR(Status)) continue;
            
            DefaultsInfo.NvramAddress=NULL;
            DefaultsInfo.NvramSize=0;
            Status=Fv->ReadSection (
                       Fv,&SetupDefaultsFfsGuid,
                       EFI_SECTION_RAW, 0,
                       (VOID**)&DefaultsInfo.NvramAddress, &DefaultsInfo.NvramSize,
                       &Authentication
                   );
                   
            if (!EFI_ERROR(Status))
            {
                NVRAM_TRACE((TRACE_DXE_CORE, "NVRAM: External Defaults Found: Size=%X\n",DefaultsInfo.NvramSize));
                NvInitInfoBuffer(
                    &DefaultsInfo, 0,
                    NVRAM_STORE_FLAG_NON_VALATILE
                    | NVRAM_STORE_FLAG_READ_ONLY
                    | NVRAM_STORE_FLAG_DO_NOT_ENUMERATE
                );
                DefInfo = &DefaultsInfo;
                break;
            }
        }
    }
    
    else
    {
        DefaultsInfo.NvramAddress = NULL;
        DefInfo = Info;
    }
    
    if (DefInfo!=NULL)
    {
        TmpBuffer = SelectiveAllocate (TRUE, NvramSize);
    
        if (TmpBuffer==NULL)
        {
            SelectiveFree (NewInfo.NvramAddress);
            return EFI_OUT_OF_RESOURCES;
        }
        Status=CopyVariable(
                   (CHAR16*)StdDefaults, (EFI_GUID*)&AmiDefaultsVariableGuid,
                   NvramSize,TmpBuffer,DefInfo,&NewInfo,&MemInterface
               );
                   
        if (Status!=EFI_OUT_OF_RESOURCES)
        {
            Status=CopyVariable(
                       (CHAR16*)MfgDefaults, (EFI_GUID*)&AmiDefaultsVariableGuid,
                       NvramSize,TmpBuffer,DefInfo,&NewInfo,&MemInterface
                   );
        }

        SelectiveFree (TmpBuffer);

        if (PreserveVariablesWithNoDefaults){
            NVRAM_STORE_INFO DefVarInfo;
            if (NvGetDefaultsInfo(StdDefaults,DefInfo,&DefVarInfo) != NULL)
                Status=CopyVariables(
                  Info, &NewInfo, &MemInterface, SkipVarsWithDefaults ,&DefVarInfo
                );
        }
        if (DefaultsInfo.NvramAddress!=NULL )
        {
            pBS->FreePool(DefaultsInfo.NvramAddress);
        }
        // don't reset NVRAM if no defaults found
        if (NewInfo.pEndOfVars == NewInfo.pFirstVar) DefInfo=NULL;
    }
    
    if (   Status!=EFI_OUT_OF_RESOURCES 
        && ( DefInfo!=NULL || !HeaderIsValid )
        && (    Info->pEndOfVars - Info->NvramAddress!= NewInfo.pEndOfVars - NewInfo.NvramAddress
             || DefInfo!=Info
           )
    ) Status=UpdateVarstore(Info,Interface,&NewInfo);
       
    SelectiveFree (NewInfo.NvramAddress);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsStoreOk
//
// Description: This function checks is Varstore is correct
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//
// Output:      BOOLEAN - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsStoreOk(NVRAM_STORE_INFO *Info)
{
    return
        (   Info->LastVarSize==0
            || NvIsVariable((NVAR*)(Info->pEndOfVars-Info->LastVarSize),Info)
        )
        && (ReadUnaligned32((UINT32*)Info->pEndOfVars) ==(UINT32)FlashEmpty)
        && (ReadUnaligned16 ((UINT16*)&(((NVAR*)Info->pEndOfVars)->size))==FLASH_EMPTY_SIZE)
        && *(UINT32*)(Info->NvramGuidsAddress-Info->NextGuid)==(UINT32)FlashEmpty;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckStore
//
// Description: This function checks if Varstore is correct and otherwise tries to 
//              Reinitialize NvRam, copy Varstore to memory and prints messages
//              based on situation.
//
// Input:       OUT NVRAM_STORE_INFO *Info -  pointer to NVRAM_STORE_INFO structure
//
// Output:      BOOLEAN - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CheckStore(BOOLEAN Recover)
{
    NVRAM_STORE_INFO *Info = VarStoreInfo.NvInfo;
    NVRAM_STORE_INFO NewInfo;
    EFI_STATUS Status;
    BOOLEAN MainStoreValid;
    BOOLEAN BackupStoreValid;
#ifdef HYBRID_NV_INTERFACE
   NVRAM_STORE_INFO FlashInfo;
   HybridStoreToRealStore(Info,&FlashInfo);
#endif
    MainStoreValid = IsMainNvramStoreValid(
#ifdef HYBRID_NV_INTERFACE
        &FlashInfo,
#else
        VarStoreInfo.NvInfo, 
#endif
        VarStoreInfo.BackupAddress, &BackupStoreValid
    );
    if (!MainStoreValid){
        if (BackupStoreValid){
            NVRAM_STORE_INFO BackupInfo;
            NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM Store is not valid. Switching to Backup Store.\n"));
#ifdef HYBRID_NV_INTERFACE
            if (!SwapVarstoresInHybridMode(&BackupInfo)){
#endif
            BackupInfo.NvramAddress = VarStoreInfo.BackupAddress;
            BackupInfo.NvramSize = VarStoreInfo.NvInfo->NvramSize;
            NvInitInfoBuffer(
                &BackupInfo, VarStoreInfo.HeaderLength, VarStoreInfo.NvInfo->Flags
            );
            VarStoreInfo.BackupAddress = VarStoreInfo.NvInfo->NvramAddress;
#ifdef HYBRID_NV_INTERFACE
			}
#endif
            *VarStoreInfo.NvInfo = BackupInfo;
            NvramReinitialize();
            MainStoreValid = TRUE;
#ifdef HYBRID_NV_INTERFACE
            HybridStoreToRealStore(Info,&FlashInfo);
#endif
        }else{//both main and backup stores are invliad; force the recovery.
            Recover = TRUE;
        }
    }
#ifdef HYBRID_NV_INTERFACE
    if (MainStoreValid && IsStoreOk(&FlashInfo)) return;
#else
    if (MainStoreValid && IsStoreOk(Info)) return;
#endif
    
    if (!Recover)
    {
        NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM inconsistency detected. Reinitializing.\n"));
#ifdef HYBRID_NV_INTERFACE
        //When inconsystency is detected, re-read the flash content into the memory buffer
        //(they may be out of synchronization)
        MemCpy(
            VarStoreInfo.NvInfo->NvramAddress,
            VarStoreInfo.NvramFlashAddress,
            VarStoreInfo.NvInfo->NvramSize
        );
#endif
        NvramReinitialize();
        
        if (IsStoreOk(Info)) return;
    }
    NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM corruption detected.\n"));

    Status = CopyVarStoreToMemStore(Info,NULL,&NewInfo);
    
    if (EFI_ERROR(Status)){
        SelectiveFree (NewInfo.NvramAddress);
        return ;
    }
    
    UpdateVarstore(Info,VarStoreInfo.NvInterface,&NewInfo);
    SelectiveFree (NewInfo.NvramAddress);
}

//High level routines
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeGetVariable
//
// Description: This function searches for Var with specific GUID and Name
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OPTIONAL OUT UINT32* Attributes - Pointer to memory where Attributes will be returned 
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var will be returned
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeGetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
)
{
    EFI_STATUS Status;
    UINT32 Attrib;
    
    if (!VariableName || !VendorGuid || !DataSize || !Data && *DataSize)
        return EFI_INVALID_PARAMETER;
    Status = GetVariableHook (
                 VariableName,VendorGuid,Attributes,DataSize,Data
             );
    if (Status != EFI_UNSUPPORTED) return Status;        
    CheckStore(FALSE);
#if NV_CACHE_SUPPORT
    {
        UINTN VariableNameSize;
        NVRAM_STORE_INFO *Info;
        BOOLEAN UnindexedExists;
        NVAR *Nvar = NULL;
        VAR_INDEX_ENTRY *NvarEntry = IndexFindVariableEntry(
                         VariableName,VendorGuid,&VariableNameSize,
                         &Info, &UnindexedExists
                     );
                     
        if (NvarEntry!=NULL)
        {
            Nvar = NvarEntry->Nvar;
            if (AreBtVariablesHidden() && !(Nvar->flags & NVRAM_FLAG_RUNTIME)) return EFI_NOT_FOUND;
            
            // Get the attributes from the first entry, data-only entries do not store attribs
            if (Attributes)
                if (EFI_ERROR(NvGetAttributesFromNvar(Nvar, Info, Attributes)))
                    return EFI_NOT_FOUND;
                
            // Get the data using the cached data-only entry, if possible
            return NvGetVariableFromNvar(
                       NvarEntry->FinalDataNvar,VariableNameSize,NULL,DataSize,Data,Info, NULL
                   );
        }
        
        if (!UnindexedExists) return EFI_NOT_FOUND;
    }
#endif
    Status = NvGetVariable2(
                 VariableName, VendorGuid, &Attrib,
                 DataSize, Data,
                 VarStoreInfo.InfoCount, VarStoreInfo.NvramInfo
             );
             
    if (!EFI_ERROR(Status))
    {
        if (AreBtVariablesHidden() && !(Attrib & EFI_VARIABLE_RUNTIME_ACCESS)) return EFI_NOT_FOUND;
        
        if (Attributes) *Attributes=Attrib;
    }
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeGetNextVariableName
//
// Description: This function searches for next Var after Var with specific name and GUID and returns it's Name. 
//
// Input:       IN OUT UINTN *VariableNameSize - size of Varname - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              IN OUT CHAR16 *VariableName - pointer where Var Name in Unicode will be stored
//              IN OUT EFI_GUID *VendorGuid - pointer to menory where Var GUID is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeGetNextVariableName(
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
)
{
    EFI_STATUS Status;
    
    if ( !VariableNameSize || !VariableName || !VendorGuid)
        return EFI_INVALID_PARAMETER;
    Status = GetNextVarNameHook (
                 VariableNameSize, VariableName, VendorGuid
             );
    if (Status != EFI_UNSUPPORTED) return Status;    
    CheckStore(FALSE);
    Status = NvGetNextVariableName2(
                 VariableNameSize, VariableName, VendorGuid,
                 VarStoreInfo.InfoCount, VarStoreInfo.NvramInfo,
                 &VarStoreInfo.LastInfoIndex,AreBtVariablesHidden()
             );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeSetVariable
//
// Description: This function sets Var with specific GUID, Name and attributes
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINT32 Attributes - Attributes of the Var
//              IN UINTN DataSize - size of Var
//              IN VOID *Data - Pointer to memory where Var data is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeSetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
)
{
    NVAR    *Var = NULL;
    UINTN NameSize;
    BOOLEAN NvVariableFound;
    NVRAM_STORE_INFO *Info=NULL;
    VARIABLE_INTERFACE *Interface;
    EFI_STATUS Status;
    NVRAM_STORE_INFO NewInfo;
#if NV_CACHE_SUPPORT
    BOOLEAN UnindexedExists;
    BOOLEAN UpdateIndex;
    VAR_INDEX_ENTRY *VarEntry;
#endif
    UINTN OldDataSize=0;
    VOID *OldData=NULL;
    EXT_SEC_FLAGS ExtSecFlags = {0, 0,{0}};
    if (
        !VariableName || VariableName[0]==0 || !VendorGuid
        || ( Attributes & ~ALL_VARIABLE_ATTRIBUTES)
        || (Attributes & EFI_VARIABLE_RUNTIME_ACCESS) && !(Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS )
        || DataSize && !Data
        || Runtime && Attributes && (
                                        !(Attributes & EFI_VARIABLE_NON_VOLATILE) 
                                     || !(Attributes & EFI_VARIABLE_RUNTIME_ACCESS) && AreBtVariablesHidden()
                                    )
    ) return EFI_INVALID_PARAMETER;
    Status = SetVariableHook (
                 VariableName,VendorGuid,Attributes,DataSize,Data
             );
    if (Status != EFI_UNSUPPORTED) return Status;
    if (DataSize>MAX_NVRAM_VARIABLE_SIZE) return EFI_OUT_OF_RESOURCES;
    
    CheckStore(FALSE);
#if NV_CACHE_SUPPORT
    //UpdateIndex defines if indexed address has to be updated.
    //Typically we don't have to update index because
    //index contains address of the first NVAR for a particular variable,
    //which does not change when we update variable data.
    //However, there is one exception from the rule.
    //We need to update the index, when indexed address points to the default value of the variable.
    //In this case UpdateIndex will be set to TRUE
    UpdateIndex = FALSE;
    VarEntry = IndexFindVariableEntry(
                   VariableName,VendorGuid,&NameSize,
                   &Info, &UnindexedExists
               );
          
    if (VarEntry!=NULL)
    {
        Var = VarEntry->Nvar;
        if (Info==VarStoreInfo.NvInfo) NvVariableFound=TRUE;
        else if (Info==VarStoreInfo.MemInfo) NvVariableFound=FALSE;
        else
        {
            //We are setting UnindexedExists to TRUE because
            //there may be other variable instances in varstores with lower priority
            Var=NULL; VarEntry=NULL; UpdateIndex=TRUE; UnindexedExists=TRUE;
        }
    }
    
    if (Var==NULL && UnindexedExists)
    {
        NvVariableFound=TRUE;
        Var = NvFindVariable(VariableName,VendorGuid, &NameSize, VarStoreInfo.NvInfo);
        
        if (Var==NULL)
        {
            NvVariableFound=FALSE;
            Var = NvFindVariable(VariableName,VendorGuid, &NameSize, VarStoreInfo.MemInfo);
        }
    }
    
#else
    NvVariableFound=TRUE;
    Var = NvFindVariable(VariableName,VendorGuid, &NameSize, VarStoreInfo.NvInfo);
    
    if (Var==NULL)
    {
        NvVariableFound=FALSE;
        Var = NvFindVariable(VariableName,VendorGuid, &NameSize, VarStoreInfo.MemInfo);
    }
    
#endif
    if( Var!=NULL )
    {
        // It's illegal to set boot time variable at run time.
        if (AreBtVariablesHidden() && !(Var->flags & NVRAM_FLAG_RUNTIME)) return EFI_INVALID_PARAMETER;
        if (NvVariableFound)
        {
            Info = VarStoreInfo.NvInfo;
            Interface = VarStoreInfo.NvInterface;
        }
        else
        {
            Info = VarStoreInfo.MemInfo;
            Interface = VarStoreInfo.MemInterface;
        }
        GetVarAuthExtFlags(Var, Info, &ExtSecFlags);
#if NV_CACHE_SUPPORT
        if (VarEntry != NULL && VarEntry->FinalDataNvar != NULL)
            OldData = NvGetData( VarEntry->FinalDataNvar, NameSize, &OldDataSize, Info);
        else
#endif
            OldData = NvGetData( Var, NameSize, &OldDataSize, Info);
    } else {
        if (Attributes & EFI_VARIABLE_NON_VOLATILE)
        {
            Info = VarStoreInfo.NvInfo;
            Interface = VarStoreInfo.NvInterface;
        }
        else
        {
#if NV_CACHE_SUPPORT
            //if UpdateIndex is TRUE, default value exists
            //If default value exists, the variable is non valatile
            //Attributes for existing variable can not be changed
            if (UpdateIndex) return EFI_INVALID_PARAMETER;
#endif
            Info = VarStoreInfo.MemInfo;
            Interface = VarStoreInfo.MemInterface;
        }
        OldDataSize = 0;
        OldData = NULL;
    }
#if AuthVariable_SUPPORT
    Status = VerifyVariable(VariableName, VendorGuid, &Attributes, &Data, &DataSize, OldData, OldDataSize, &ExtSecFlags);
    if (EFI_ERROR(Status)) {
    // case for SigDb Append mode. EFI_ALREADY_STARTED treat as Ok to exit SetVar
        if(Status == EFI_ALREADY_STARTED) 
            Status = EFI_SUCCESS;
        return Status;
    }
#endif //#if AuthVariable_SUPPORT

// Function called with empty access attributes - the variable shall be erased.
    if(!Attributes)
        DataSize = 0;
    //
    // Verification passed.
    //
    if ((!(Attributes & EFI_VARIABLE_APPEND_WRITE) && !DataSize) || 
         !(Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS) )
    {
        //delete
        if (Var==NULL) return
#if NV_CACHE_SUPPORT
                //if UpdateIndex is TRUE, default value (which can not be deleted ) exists
                (UpdateIndex) ? EFI_WRITE_PROTECTED :
#endif
                EFI_NOT_FOUND;
                
        if (Runtime && !NvVariableFound) return EFI_INVALID_PARAMETER;
        
        Status = DeleteVariable(Var,Info,Interface);
#if NV_CACHE_SUPPORT
        
        if (!EFI_ERROR(Status))
        {
            //The variable has just been deleted from the primary varstore.
            //However, we may still have variable instance in other stores.
            INT32 i;
            NVAR *v;
            
            if (NvVariableFound)
            {
                for (i=VarStoreInfo.InfoCount-1; i>=0; i--)
                {
                    if (    Info == &VarStoreInfo.NvramInfo[i]
                            || (Info->Flags & NVRAM_STORE_FLAG_NON_VALATILE)==0
                       ) continue;
                       
                    v = NvFindVariable(
                            VariableName,VendorGuid, &NameSize,
                            &VarStoreInfo.NvramInfo[i]
                        );
                        
                    if (v!=NULL)
                    {
                        IndexUpdateVariable(v,&VarStoreInfo.NvramInfo[i]);
                        return Status;
                    }
                }
            }
            
            //If other instances are not found, delete the index entry
            IndexDeleteVariableEntry(VarEntry);
        }
        
#endif
        return Status;
    }
    
    if (Var==NULL)
    {
        //create new
        Status = CreateVariable(
                     VariableName,VendorGuid,Attributes,
                     DataSize, Data, Info,Interface, NULL, &ExtSecFlags// NULL
                 );
#if NV_CACHE_SUPPORT
        if (!EFI_ERROR(Status))
        {
        	if ( (    VarStoreInfo.NvramMode & NVRAM_MODE_DEFAULT_CONFIGURATION)!=NVRAM_MODE_DEFAULT_CONFIGURATION
        	       || Info!=VarStoreInfo.NvInfo ) {
        		if (UpdateIndex)
        			IndexUpdateVariable(
        					(NVAR*)(Info->pEndOfVars-Info->LastVarSize),Info
        			);
        		else
        			IndexAddVariable(
        					(NVAR*)(Info->pEndOfVars-Info->LastVarSize),Info
        			);
        	}
        	
        }
        
#endif
    }
    
    else
    {
        //update existing
        Status = UpdateVariable(
                     Var,VariableName,NameSize,VendorGuid,Attributes,
                     DataSize, Data,
#if NV_CACHE_SUPPORT
                     VarEntry,
#else
                     NULL,
#endif

                    &ExtSecFlags,
                     Info, Interface
                 );
    }
    
    //Error Recovery
    if (   Status==EFI_OUT_OF_RESOURCES
            || Status==EFI_DEVICE_ERROR && !IsStoreOk(Info)
       )
    {
        NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM: SetVariable failed. Status=%r. Starting Recovery...\n",Status));
        NewInfo.NvramAddress = NULL;
        if (EFI_ERROR(CopyVarStoreToMemStore(Info,Var,&NewInfo))){
            if (NewInfo.NvramAddress!=NULL) SelectiveFree (NewInfo.NvramAddress);
            return Status;
        }
        if (Attributes & EFI_VARIABLE_APPEND_WRITE)
            Status = CreateVariableEx(
                        VariableName,VendorGuid,Attributes,
                        DataSize, Data, &NewInfo, &MemInterface, NULL, &ExtSecFlags, 
                        OldData, OldDataSize
                    );
        else
            Status = CreateVariable(
                        VariableName,VendorGuid,Attributes,
                        DataSize, Data, &NewInfo, &MemInterface, NULL, &ExtSecFlags
                    );         
                 
        if (!EFI_ERROR(Status))
        {
            Status = UpdateVarstore(Info,Interface,&NewInfo);
        }
        
        else
        {
            CheckStore(TRUE);
        }
        
        SelectiveFree (NewInfo.NvramAddress);
        NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM: SetVariable Status after recovery=%r.\n",Status));
    }
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeGetVariableSafe
//
// Description: This function searches for Var with specific GUID and Name 
//              beginning and ending critical section 
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OPTIONAL OUT UINT32* Attributes - Pointer to memory where Attributes will be returned 
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var will be returned
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeGetVariableSafe(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
)
{
    EFI_STATUS Status;
    BEGIN_CRITICAL_SECTION(NvramCs);
    Status = DxeGetVariable(
                 VariableName,VendorGuid,Attributes,DataSize,Data
             );
    END_CRITICAL_SECTION(NvramCs);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeGetNextVariableNameSafe
//
// Description: This function searches for Var folowing after Var with specific name and GUID 
//              and returns it's Name, beginning and ending critical section . 
//
// Input:       IN OUT UINTN *VariableNameSize - size of Varname - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              IN OUT CHAR16 *VariableName - pointer where Var Name in Unicode will be stored
//              IN OUT EFI_GUID *VendorGuid - pointer to menory where Var GUID is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeGetNextVariableNameSafe(
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
)
{
    EFI_STATUS Status;
    BEGIN_CRITICAL_SECTION(NvramCs);
    Status = DxeGetNextVariableName(
                 VariableNameSize,VariableName,VendorGuid
             );
    END_CRITICAL_SECTION(NvramCs);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeSetVariableSafe
//
// Description: This function sets Var with specific GUID, Name and attributes
//              beginning and ending critical section. 
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINT32 Attributes - Attributes of the Var
//              IN UINTN DataSize - size of Var
//              IN VOID *Data - Pointer to memory where Var data is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeSetVariableSafe(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
)
{
    EFI_STATUS Status;
    BEGIN_CRITICAL_SECTION(NvramCs);
    Status = DxeSetVariable(
                 VariableName,VendorGuid,Attributes,DataSize,Data
             );
    END_CRITICAL_SECTION(NvramCs);
    return Status;
}

//Initialization
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitVolatileStore
//
// Description: This function allocates memory and inits NVRAM_STORE_INFO structure. 
//
// Input:       NVRAM_STORE_INFO *pInfo - pointer to NVRAM store structure
//              UINTN HeaderSize - Size of the header 
//              UINT8 Flags - default Flags
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InitVolatileStore(
    NVRAM_STORE_INFO *Info, UINTN Size,
    UINTN HeaderLength, UINT8 Flags,
    BOOLEAN Runtime
)
{                                      
    EFI_STATUS      Status = EFI_SUCCESS;

    if ((NvramDriverBuffer) && (Flags & NVRAM_STORE_FLAG_NON_VALATILE)) {
        Info->NvramAddress = SelectiveAllocate (FALSE, Size);
        if (Info->NvramAddress == NULL) return EFI_OUT_OF_RESOURCES;
    }else{
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
        if (NoAccessOutsideofSmm){
            Status = pSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,Size, (VOID**)&Info->NvramAddress
                    );
            if (EFI_ERROR(Status)) return Status;
        }
        else
#endif
        {
            Status = pBS->AllocatePool (
                (Runtime) ? EfiRuntimeServicesData : EfiBootServicesData,
                Size, (VOID**)&Info->NvramAddress
            );
            if (EFI_ERROR(Status)) return Status;
        }
    }
    
    Info->NvramSize = Size;
    MemSet(Info->NvramAddress,Info->NvramSize,FLASH_EMPTY_BYTE);
    NvInitInfoBuffer(
        Info,HeaderLength,Flags
    );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EnumerateStore
//
// Description: This function searches for last Ver and fills the NVRAM_STORE_INFO structure. 
//
// Input:       NVRAM_STORE_INFO *pInfo - pointer to NVRAM store structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID EnumerateStore(NVRAM_STORE_INFO *Info)
{
    NVAR *Var, *LastVar;
    INT16 NextGuid;
    //Init NextGuid and pEndOfVars
    Var = (NVAR*)Info->pFirstVar;
    LastVar = Var;
    NextGuid=0;
    
    if (NvIsVariable(Var,Info))
    {
        //We assume that the first variable pInfo->pFirstVar is always valid
        //It's OK since we checked for pInfo->pFirstVar validity
        //during during NVRAM_STORE_INFO initialization in NvInitInfoBuffer routine
        for (; Var; Var=NvGetNextNvar(Var,Info))
        {
            if ( ( Var->flags&(NVRAM_FLAG_DATA_ONLY|NVRAM_FLAG_GUID) ) == 0 )
            {
                INT16 guid = *(UINT8*)(Var+1);
                
                if (guid>NextGuid) NextGuid=guid;
            }
            
            LastVar = Var;
        }
        
        NextGuid++;
        Info->LastVarSize = LastVar->size;
        Info->pEndOfVars = (UINT8*)LastVar+LastVar->size;
        Info->NextGuid = NextGuid;

        // Skip incomplete NVAR record at the end of the NVRAM area (if any)
        LastVar = (NVAR*)Info->pEndOfVars;
        if (   LastVar->size != FLASH_EMPTY_SIZE
            &&     LastVar->size 
                <= (UINT8*)(Info->NvramGuidsAddress-NextGuid) - Info->pEndOfVars
        ){
            Info->LastVarSize += LastVar->size;
            Info->pEndOfVars += LastVar->size;
        }
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarStoreDiscovery
//
// Description: This function searches for nested Verstores and adds them to the NVRAM_STORE_INFO structure. 
//
// Input:       NVRAM_STORE_INFO *pInfo - pointer to NVRAM store structure
//              NVRAM_STORE_INFO *MemInfo - pointer to NVRAM store structure in memory
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID VarStoreDiscovery(NVRAM_STORE_INFO *NvInfo, NVRAM_STORE_INFO *MemInfo)
{
    VarStoreInfo.InfoCount = 0;
    VarStoreInfo.LastInfoIndex = 0;
    
    //Handle Manufacturing mode
    if (   VarStoreInfo.NvramMode & NVRAM_MODE_MANUFACTORING
            && NvGetDefaultsInfo(
                MfgDefaults,NvInfo,&VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount]
            ) != NULL
       )
    {
        VarStoreInfo.InfoCount++;
    }
    
    //Handle Default config mode
    if (!(VarStoreInfo.NvramMode & NVRAM_MODE_DEFAULT_CONFIGURATION))
    {
        //add regular NVRAM
        VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount]=*NvInfo;
        VarStoreInfo.NvInfo = &VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount];
        VarStoreInfo.InfoCount++;
        
        //add defaults
        if (NvGetDefaultsInfo(
                    StdDefaults,NvInfo,&VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount]
                ) != NULL
           )
        {
            VarStoreInfo.InfoCount++;
        }
    }
    
    else
    {
        //add defaults
        if (NvGetDefaultsInfo(
                    StdDefaults,NvInfo,&VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount]
                ) != NULL
           )
        {
            VarStoreInfo.InfoCount++;
        }
        
        //add regular NVRAM
        VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount]=*NvInfo;
        VarStoreInfo.NvInfo = &VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount];
        VarStoreInfo.InfoCount++;
    }
    
    VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount] = *MemInfo;
    VarStoreInfo.MemInfo=&VarStoreInfo.NvramInfo[VarStoreInfo.InfoCount];
    VarStoreInfo.InfoCount++;
    //Init NextGuid and pEndOfVars
    EnumerateStore(VarStoreInfo.NvInfo);
}

//--- Hook to chek NvRam data
BOOLEAN DxeIsNvramDataCompatible(IN EFI_GET_VARIABLE GetVariable);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvramInitialize
//
// Description: This function initializes Varstore
//
// Input:       NONE
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NvramInitialize()
{
    static EFI_GUID guidHobList = HOB_LIST_GUID;
    NVRAM_STORE_INFO NvStore, MemStore;
    NVRAM_HOB *NvramHob = (NVRAM_HOB*)GetEfiConfigurationTable(pST,&guidHobList);
    NVRAM_STORE_INFO DefaultsInfo;
    BOOLEAN UseExternalDefaults=FALSE;
    BOOLEAN PreserveVariablesWithNoDefaults=FALSE, NvramHobWasFound=TRUE;
    extern const UINTN NvramSize;
    
    if (!NvramHob) return EFI_NOT_FOUND;
    
    if (!EFI_ERROR(FindNextHobByGuid((EFI_GUID*)&AmiNvramHobGuid, (VOID**)&NvramHob)))
    {
        
    	if (NvramRtGarbageCollectionSupport) {
    		NvramDriverBufferSize = NvramHob->NvramSize * 3;
    		if( EFI_ERROR( pBS->AllocatePages(
    				AllocateAnyPages, EfiRuntimeServicesData,
    				EFI_SIZE_TO_PAGES(NvramDriverBufferSize),
    				&NvramDriverBuffer )
    		)) NvramDriverBuffer = 0;
    	} 

    	MemSet(&VarStoreInfo,sizeof(VarStoreInfo),0);
    	VarStoreInfo.HeaderLength = NvramHob->HeaderLength;
    	VarStoreInfo.NvramMode = NvramHob->NvramMode;
    	VarStoreInfo.BackupAddress = (UINT8*)NvramHob->BackupAddress;
#ifdef EFI_DEBUG
    	if (VarStoreInfo.BackupAddress)
    		NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM back up address: %X\n", VarStoreInfo.BackupAddress));
#endif
    	VarStoreInfo.MemInterface=&MemInterface;
    	InitVolatileStore(
    			&MemStore,
    			VALATILE_VARIABLE_STORE_SIZE,
    			VarStoreInfo.HeaderLength,0,TRUE
    	);
    }
    else
    {
    	MemSet(&VarStoreInfo,sizeof(VarStoreInfo),0);
    	NvramHobWasFound = FALSE;
    	VarStoreInfo.NvramMode = NVRAM_MODE_SIMULATION;
    	VarStoreInfo.HeaderLength = NvramHeaderLength;
    	VarStoreInfo.MemInterface=&MemInterface;
    	InitVolatileStore(
    	    			&MemStore,
    	    			VALATILE_VARIABLE_STORE_SIZE,
    	    			VarStoreInfo.HeaderLength,0,TRUE
    	    	);
    	
    }
    //TODO: comment out the code below for now.
    // Switching to simulation during recovery is not necessarily a good idea.
    // For example, it can cause endless reboot if recovery is enabled during the 
    // very first boot after BIOS flashing. Ideally we should switch to simulation 
    // only when NVRAM is incompatible with the FV_MAIN drivers 
    // (which is checked later in this function).  While enabling the simulation 
    // we should also "notify" (notification mechanism TBD) the reflash module 
    // that if FV_MAIN is updated, NVRAM must also be updated.
/*
    BootMode = GetBootMode();
    if (BootMode==BOOT_IN_RECOVERY_MODE)
        VarStoreInfo.NvramMode |= NVRAM_MODE_SIMULATION;
*/
    //If we are in the simulation mode let's create new
    //memory based variable store and initialze it with
    //content of the NVRAM flash area.
    if (VarStoreInfo.NvramMode & NVRAM_MODE_SIMULATION)
    {
        VarStoreInfo.NvInterface=&MemInterface;

        InitVolatileStore(
            &NvStore, 
            (NvramHobWasFound) ? NvramHob->NvramSize : NvramSize,
            VarStoreInfo.HeaderLength,
            NVRAM_STORE_FLAG_NON_VALATILE,TRUE
        );
        if (NvramHobWasFound)
        	MemCpy(NvStore.NvramAddress,(UINT8*)NvramHob->NvramAddress,NvStore.NvramSize);
        //We altered content of the varstore. Let's reinitialize it.
        NvInitInfoBuffer(
            &NvStore, VarStoreInfo.HeaderLength,
            NVRAM_STORE_FLAG_NON_VALATILE
        );
        //no backup in the simulation mode
        VarStoreInfo.BackupAddress=0;
        NVRAM_TRACE((TRACE_DXE_CORE,"NVRAM: working in simulation mode\n"));
    }
    
    else
    {
        VarStoreInfo.NvInterface=&NvInterface;
        NvStore.NvramAddress = (UINT8*)NvramHob->NvramAddress;
        NvStore.NvramSize = NvramHob->NvramSize;
        NvInitInfoBuffer(
            &NvStore,
            VarStoreInfo.HeaderLength,
            NVRAM_STORE_FLAG_NON_VALATILE
        );
    }

    //If FV header is corrupted or built-in defaults are not found,
    //reset NVRAM data; use external defaults.
    if (   ((EFI_FIRMWARE_VOLUME_HEADER*)NvStore.NvramAddress)->Signature!=FV_SIGNATURE
        || NvGetDefaultsInfo(StdDefaults, &NvStore, &DefaultsInfo) == NULL
    ){
        UseExternalDefaults = TRUE;
        PreserveVariablesWithNoDefaults = TRUE;
        VarStoreInfo.NvramMode |= NVRAM_MODE_RESET_CONFIGURATION;
    }
    //Handle NVRAM reset
    if (VarStoreInfo.NvramMode & NVRAM_MODE_RESET_CONFIGURATION)
    {
        NvResetConfiguration(
            &NvStore,VarStoreInfo.NvInterface,
            UseExternalDefaults, PreserveVariablesWithNoDefaults
        );
        VarStoreInfo.NvramMode&=~NVRAM_MODE_RESET_CONFIGURATION;
    }
#ifdef HYBRID_NV_INTERFACE
        InitHybridInterface(&NvStore);
#endif    
    VarStoreDiscovery(&NvStore,&MemStore);
#if NV_CACHE_SUPPORT
    IndexInit();
#endif
    
    if (!DxeIsNvramDataCompatible(DxeGetVariableSafe))
    {
        NVRAM_TRACE((TRACE_DXE_CORE, "NVRAM DXE: Incompactible NVRAM detected\n"));
        NvResetConfiguration(
            VarStoreInfo.NvInfo, VarStoreInfo.NvInterface, 
            TRUE, TRUE
        );
    }
    
    return EFI_SUCCESS;
}

EFI_STATUS NvramReinitialize()
{
    NVRAM_STORE_INFO NvStore, MemStore;
    
    NvStore.NvramAddress = VarStoreInfo.NvInfo->NvramAddress;
    NvStore.NvramSize = VarStoreInfo.NvInfo->NvramSize;
    MemStore = *VarStoreInfo.MemInfo;
    NvInitInfoBuffer(
        &NvStore,
        VarStoreInfo.HeaderLength,
        NVRAM_STORE_FLAG_NON_VALATILE
    );
    VarStoreDiscovery(&NvStore,&MemStore);
#if NV_CACHE_SUPPORT
    IndexReset();
#endif
    return EFI_SUCCESS;
}
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
static EFI_GUID guidVariable = EFI_VARIABLE_ARCH_PROTOCOL_GUID;
static EFI_GUID guidVariableWrite = EFI_VARIABLE_WRITE_ARCH_PROTOCOL_GUID;
static EFI_GUID guidMonotonicCounter = EFI_MONOTONIC_COUNTER_ARCH_PROTOCOL_GUID;
extern EFI_GUID gAmiGlobalVariableGuid;
UINT32 CounterLo=0, CounterHi=0;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetNextHighMonotonicCount
//
// Description: This function gets Next High Monotonic Count 
//
// Input:       OUT UINT32 *HighCount - pointer to where result will be stored
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetNextHighMonotonicCount(OUT UINT32 *HighCount)
{
    EFI_STATUS Status;
    UINTN Size = sizeof(UINT32);
    
    if (!HighCount) return EFI_INVALID_PARAMETER;
    
    if (!CounterHi)
    {
        Status = pRS->GetVariable(L"MonotonicCounter", &gAmiGlobalVariableGuid,
                                  NULL, &Size, &CounterHi
                                 );
                                 
        if (EFI_ERROR(Status) && Status != EFI_NOT_FOUND)
            return Status;
    }
    
    ++CounterHi;
    Status = pRS->SetVariable(
                 L"MonotonicCounter", &gAmiGlobalVariableGuid,
                 EFI_VARIABLE_NON_VOLATILE|EFI_VARIABLE_BOOTSERVICE_ACCESS|EFI_VARIABLE_RUNTIME_ACCESS,
                 Size, &CounterHi
             );
             
    if (EFI_ERROR(Status)) return Status;
    
    *HighCount = CounterHi;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetNextMonotonicCount
//
// Description: This function gets Next low and high Monotonic Count 
//
// Input:       OUT UINT64 *Count - pointer to where result will be stored
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetNextMonotonicCount(OUT UINT64 *Count)
{
    if (!Count) return EFI_INVALID_PARAMETER;
    
    ((UINT32*)Count)[0] = CounterLo++;
    //--- If low counter overflovs - GetNextHighMonotonicCount
    if (!CounterLo) return  GetNextHighMonotonicCount(&((UINT32*)Count)[1]); 
    //--- If not - get current HighMonotonicCount
    else ((UINT32*)Count)[1] = CounterHi;
    
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   QueryVariableInfo
//
// Description: This function returns parameters of VarStore with passed attributes
//
// Input:       IN UINT32 Attributes - Atributes to search for
//              OUT UINT64 *MaximumVariableStorageSize - Maximum Variable Storage Size
//              OUT UINT64 *RemainingVariableStorageSize - Remaining Variable Storage Size
//              OUT UINT64 *MaximumVariableSize - Maximum Variable Size
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS QueryVariableInfo (
    IN UINT32 Attributes,
    OUT UINT64 *MaximumVariableStorageSize,
    OUT UINT64 *RemainingVariableStorageSize,
    OUT UINT64 *MaximumVariableSize
)
{
    UINT64 StorageSize,RemainingStorage,MaxVarSize;
    NVRAM_STORE_INFO *Info;
    UINTN SizeOfVarSizeType = MAX_NVRAM_VARIABLE_SIZE;
    
    if ( !Attributes || (Attributes & ~ALL_VARIABLE_ATTRIBUTES)
            ||(Attributes & EFI_VARIABLE_RUNTIME_ACCESS) && !(Attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS )
            || !MaximumVariableStorageSize || !RemainingVariableStorageSize || !MaximumVariableSize
       )
        return EFI_INVALID_PARAMETER;
        
    Info =   (Attributes & EFI_VARIABLE_NON_VOLATILE)
             ? VarStoreInfo.NvInfo : VarStoreInfo.MemInfo;
    StorageSize=Info->NvramSize;
    RemainingStorage=(UINT8*)(
                         (EFI_GUID*)(Info->NvramAddress+Info->NvramSize)
                         - Info->NextGuid-1
                     )-Info->pEndOfVars;
    MaxVarSize=  (SizeOfVarSizeType>RemainingStorage)
                 ? RemainingStorage : SizeOfVarSizeType;
    MaxVarSize -=  sizeof(NVAR) + sizeof(EFI_GUID);
    
    if ((INTN)MaxVarSize<0) MaxVarSize=0;
    
    *MaximumVariableStorageSize = StorageSize;
    *RemainingVariableStorageSize = RemainingStorage;
    *MaximumVariableSize = MaxVarSize;
    return EFI_SUCCESS;
}

VOID NvramNotInSmmExitBs();
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   VarExitBS
//
//  Description: This function will be called when Exit BS will signaled
//  will update data to work in RunTime.
//
//  Input:  IN EFI_EVENT Event - signalled event
//          IN VOID *Context - calling context
//
//  Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID VarExitBS (IN EFI_EVENT Event, IN VOID *Context)
{
    Runtime = TRUE;
#if NV_CACHE_SUPPORT
    IndexExitBs();
#endif
    //When we call this function from SMM, Event is NULL
    if (Event!=NULL) NvramNotInSmmExitBs();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarStoreVirtualFixup
//
// Description: This function will be invoked to convert
//              runtime pointers to virtual address
//
// Input:   NVRAM_STORE_INFO *Info - pointer to NVRAM_STORE_INFO structure to fixup
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID VarStoreVirtualFixup(NVRAM_STORE_INFO *Info)
{
    pRS->ConvertPointer(0,(VOID**)&Info->pFirstVar);
    pRS->ConvertPointer(0,(VOID**)&Info->pEndOfVars);
    pRS->ConvertPointer(0,(VOID**)&Info->NvramGuidsAddress);
    pRS->ConvertPointer(0,(VOID**)&Info->NvramAddress);
    Info->pLastReturned=NULL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarInterfaceVirtualFixup
//
// Description: This function will be invoked to convert
//              runtime pointers to virtual address
//
// Input:   VARIABLE_INTERFACE *Interface - pointer to VARIABLE_INTERFACE structure to fixup
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID VarInterfaceVirtualFixup(VARIABLE_INTERFACE *Interface)
{
    pRS->ConvertPointer(0,(VOID**)&Interface->EnableUpdate);
    pRS->ConvertPointer(0,(VOID**)&Interface->DisableUpdate);
    pRS->ConvertPointer(0,(VOID**)&Interface->EraseBytes);
    pRS->ConvertPointer(0,(VOID**)&Interface->WriteBytes);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VarVirtAddressChange
//
// Description: This function will be invoked to convert whole Var infrustructure
//              and pRS runtime pointers to virtual address
//
//  Input:      IN EFI_EVENT Event - signalled event
//              IN VOID *Context - calling context
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID VarVirtAddressChange (IN EFI_EVENT Event, IN VOID *Context)
{
    UINTN i;
    VarStoreVirtualFixup(VarStoreInfo.NvInfo);
#ifdef HYBRID_NV_INTERFACE
    pRS->ConvertPointer(0,(VOID**)&VarStoreInfo.NvramFlashAddress);
    VarStoreInfo.AddressDelta=VarStoreInfo.NvramFlashAddress-VarStoreInfo.NvInfo->NvramAddress;
#endif    
    for (i=0; i< VarStoreInfo.InfoCount; i++)
        if (VarStoreInfo.NvInfo!=&VarStoreInfo.NvramInfo[i])
            VarStoreVirtualFixup(&VarStoreInfo.NvramInfo[i]);

    for (i=0; GetVariableHookList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(GetVariableHookList[i]));
    }

    for (i=0; SetVariableHookList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(SetVariableHookList[i]));
    }
  
    for (i=0; GetNextVarNameHookList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(GetNextVarNameHookList[i]));
    }
        
    pRS->ConvertPointer(0,(VOID**)&VarStoreInfo.NvInfo);
    pRS->ConvertPointer(0,(VOID**)&VarStoreInfo.MemInfo);
    VarInterfaceVirtualFixup(VarStoreInfo.NvInterface);
    VarInterfaceVirtualFixup(VarStoreInfo.MemInterface);
    pRS->ConvertPointer(0,(VOID**)&VarStoreInfo.NvInterface);
    pRS->ConvertPointer(0,(VOID**)&VarStoreInfo.MemInterface);
    if (VarStoreInfo.BackupAddress!=NULL)
        pRS->ConvertPointer(0,(VOID**)&VarStoreInfo.BackupAddress);
    if (NvramDriverBuffer) 
        pRS->ConvertPointer(0,(VOID**)&NvramDriverBuffer);
    pRS->ConvertPointer(0,(VOID**)&NvramCs);
    VarStoreInfo.LastInfoIndex = 0;
    pRS->ConvertPointer(0, (VOID**)&Flash);
#if NV_CACHE_SUPPORT
    IndexVirtualFixup();
#endif
    //Fixup global data for for a virtual address space.
    FlashVirtualFixup(pRS);
    //pRS is converted by the library
    //(call to InitAmiRuntimeLib initiates the processes)
    AuthServiceVirtualFixup();
}

///////////////////////////////////////////////////////////////////

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MonotonicCounterInit
//
// Description: This function initialize pBS MonotonicCount servicies and installs 
//              corresponding protocols
//
//  Input:      IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - pointer to System Table
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID MonotonicCounterInit(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_HANDLE Handle = NULL;
    UINT32 HighCount;
    InitAmiLib(ImageHandle,SystemTable);
    pRS->GetNextHighMonotonicCount = GetNextHighMonotonicCount;
    pBS->GetNextMonotonicCount = GetNextMonotonicCount;
    //Init Hi part of the monotonic counter
    GetNextHighMonotonicCount(&HighCount);
    pBS->InstallMultipleProtocolInterfaces(
        &Handle,
        &guidMonotonicCounter, NULL,
        NULL
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvRamInit
//
// Description: This function initialize pRS Var servicies and MonotonicCount servicies 
//              (if MONOTONIC_COUNT_IN_NVRAM token is set) installs 
//              corresponding protocols
//
//  Input:      IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - pointer to System Table
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID NvRamInit(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_HANDLE Handle = NULL;
    EFI_EVENT Event;
    EFI_STATUS Status;

    //TODO: When DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME is defined, we assume that
    //DXE Variable services will be replaced prior to transitioning to runtime.
    //If this will not happen, the NVRAM driver may not work properly.
    //It may either crash (or crash the Windows), or generate false garbage collections.
    //If support fall back scenario the memory registration of the virtual address change event 
    //and cdoe that allocates hybrid mode buffer must be changed
    //TODO: When DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME is defined, we can allocate MemStore buffer in boot time memory.
    InitAmiRuntimeLib(
        ImageHandle, SystemTable, VarExitBS, 
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
        NULL
#else
        VarVirtAddressChange
#endif
    );
    PROGRESS_CODE(DXE_NVRAM_INIT);

    Status = pBS->LocateProtocol(&gFlashProtocolGuid, NULL, (VOID**)&Flash);
    ASSERT_EFI_ERROR(Status); 

    NvramInitialize();
// We need to know when to switch to the Runtime mode.
// Switch to runtime happens on exit boot services or legacy boot event.
// exit boot services callabck is installed by the InitAmiRuntimeLib function.
// Let's install legacy boot callbakc here.
    CreateLegacyBootEvent(TPL_CALLBACK, &VarExitBS, NULL, &Event);
    pRS->GetNextVariableName = DxeGetNextVariableNameSafe;
    pRS->GetVariable = DxeGetVariableSafe;
    pRS->SetVariable = DxeSetVariableSafe;
    pRS->QueryVariableInfo = QueryVariableInfo;

    pBS->InstallMultipleProtocolInterfaces(
        &Handle,
        &guidVariable, NULL,
        &guidVariableWrite, NULL,
        NULL
    );
    //--- Install MonotonicCount servicies (if NVRAM_MONOTONIC_COUNTER_SUPPORT token is set) 
    if (NvramMonotonicCounterSupport) MonotonicCounterInit(ImageHandle, SystemTable);
}


//**********************************************************************
// NVRAM SMM BEGIN
//**********************************************************************
//when this type is changed, initialization of the NvRamMailboxBuffer variable must also be changed.
typedef struct
{
//TODO: StopRtDataAccess - must be the first field; NvramSmi driver is using it
    BOOLEAN StopRtDataAccess;
    VARSTORE_INFO *VarStoreInfoPtr;
    VAR_INDEX *VarIndexPtr;
    BOOLEAN SwitchToRuntime;
    CRITICAL_SECTION NvramCs;
    EFI_PHYSICAL_ADDRESS NvramDriverBuffer;
    UINT32 NvramBuffSize;
} NVRAM_MAILBOX;

#define NVRAM_MAILBOX_ADDRESS_VARIABLE_GUID \
    {0x54913a6d, 0xf4ee, 0x4cdb, 0x84, 0x75, 0x74, 0x6, 0x2b, 0xfc, 0xec, 0xf5}

EFI_GUID NvRamMailboxVariableGuid = NVRAM_MAILBOX_ADDRESS_VARIABLE_GUID;
NVRAM_MAILBOX *NvRamMailbox;

/*************** OUTSIDE SMM **********************************/

NVRAM_MAILBOX NvRamMailboxBuffer = {FALSE, &VarStoreInfo, NULL, FALSE, NULL, 0, 0};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   LegacyToEfiFn
//
// Description: Legacy2Efi callback notification
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
LegacyToEfiFn   (
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
   // NvramCsPtr = &NvramCs;
    Runtime = FALSE;
    NvRamMailbox->SwitchToRuntime = FALSE;    

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvramNotInSmmInit
//
// Description: This function initialize pRS Var, MonotonicCount servicies 
//              (if MONOTONIC_COUNT_IN_NVRAM token is set) and installs 
//              corresponding protocols and sets NvRamMailbox Var not in SMM
//
//  Input:      IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - pointer to System Table
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NvramNotInSmmInit(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{

    EFI_GUID    pLTEBGuid = LTEB_GUID;	
    EFI_EVENT   Event;

    EFI_STATUS Status;

    CreateCriticalSection(&NvramCs);
    NvRamInit(ImageHandle, SystemTable);
    NvRamMailboxBuffer.VarIndexPtr = VarIndexPtr;
    NvRamMailboxBuffer.NvramCs = NvramCs;
    if (NvramRtGarbageCollectionSupport) {
        NvRamMailboxBuffer.NvramDriverBuffer = NvramDriverBuffer;
        NvRamMailboxBuffer.NvramBuffSize = NvramDriverBufferSize;
    }
    NvRamMailbox = &NvRamMailboxBuffer;

    AuthVariableServiceInit ();

    Status = pBS->CreateEventEx( EVT_NOTIFY_SIGNAL,
			         TPL_CALLBACK,
				 LegacyToEfiFn,
				 NULL,
				 &pLTEBGuid,
				 &Event);

    Status = pRS->SetVariable(
               L"NvRamMailbox",&NvRamMailboxVariableGuid,
               EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof(NvRamMailbox), &NvRamMailbox
           );
    ASSERT_EFI_ERROR(Status); 
   
    return EFI_SUCCESS;
}
/*************** INSIDE SMM **********************************/
VARSTORE_INFO *RtVarStoreInfoPtr;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SynchronizeVarStore
//
// Description: This function synchronizes NVRAM_STORE_INFO structure after enterin SMM
//
//  Input:      NVRAM_STORE_INFO *Source - source NVRAM_STORE_INFO structure
//              NVRAM_STORE_INFO *Target - synchronized NVRAM_STORE_INFO structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SynchronizeVarStore(NVRAM_STORE_INFO *Source, NVRAM_STORE_INFO *Target)
{
    UINTN Delta = Target->NvramAddress - Source->NvramAddress;
    Target->pFirstVar = Source->pFirstVar + Delta;
    Target->pEndOfVars = Source->pEndOfVars + Delta;
    Target->NextGuid = Source->NextGuid;
    Target->LastVarSize = Source->LastVarSize;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SynchronizeVarStoreInfo
//
// Description: This function synchronizes all NVRAM_STORE_INFO structures in VARSTORE_INFO after enterin SMM
//
//  Input:      VARSTORE_INFO *From - source VARSTORE_INFO structure
//              VARSTORE_INFO *To - synchronized VARSTORE_INFO structure
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SynchronizeVarStoreInfo(VARSTORE_INFO *From, VARSTORE_INFO *To)
{
    UINT32 i;
    To->InfoCount = From->InfoCount;
    
    for (i=0; i<From->InfoCount; i++)
    {
        SynchronizeVarStore(&From->NvramInfo[i], &To->NvramInfo[i]);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SynchronizeBeforeOperation
//
// Description: This function synchronizes Runtime and non Runtime Var Structures after enterin SMM
//              before each operation with Variables
//
//  Input:      NONE
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SynchronizeBeforeOperation()
{
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
if (!NoAccessOutsideofSmm)
#endif
{
    SynchronizeVarStoreInfo(
        RtVarStoreInfoPtr,&VarStoreInfo
    );
    VarStoreInfo.NvInfo = (NVRAM_STORE_INFO*)(
                              (UINT8*)RtVarStoreInfoPtr->NvInfo - (UINT8*)NvRamMailbox->VarStoreInfoPtr
                              + (UINT8*)&VarStoreInfo
                          );
    VarStoreInfo.MemInfo = (NVRAM_STORE_INFO*)(
                               (UINT8*)RtVarStoreInfoPtr->MemInfo - (UINT8*)NvRamMailbox->VarStoreInfoPtr
                               + (UINT8*)&VarStoreInfo
                           );
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
    if (NvRamMailbox->StopRtDataAccess){
        NoAccessOutsideofSmm=TRUE;
#if NV_CACHE_SUPPORT
//TODO: This has to be reviewed if RT Index support is added
        //Shutdown the index
        IndexExitBs();
#endif
        ReallocateVolatileVarstoreToSmm();
#ifdef HYBRID_NV_INTERFACE
        ReallocateHybridVarstoreToSmm();
#endif
        END_CRITICAL_SECTION(NvramCs);
        CreateCriticalSection(&NvramCs);
        BeginCriticalSection(NvramCs);
    }
#endif
}
    if (!Runtime && NvRamMailbox->SwitchToRuntime){
        VarExitBS(NULL,NULL);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SynchronizeAfterOperation
//
// Description: This function synchronizes Runtime and non Runtime Var Structures after enterin SMM
//              after each operation with Variables
//
//  Input:      NONE
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SynchronizeAfterOperation()
{
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
if (!NoAccessOutsideofSmm)
#endif
{
    SynchronizeVarStoreInfo(
        &VarStoreInfo, RtVarStoreInfoPtr
    );
    RtVarStoreInfoPtr->NvInfo = (NVRAM_STORE_INFO*)(
                                    (UINT8*)VarStoreInfo.NvInfo - (UINT8*)&VarStoreInfo
                                    + (UINT8*)NvRamMailbox->VarStoreInfoPtr
                                );
    RtVarStoreInfoPtr->MemInfo = (NVRAM_STORE_INFO*)(
                                     (UINT8*)VarStoreInfo.MemInfo - (UINT8*)&VarStoreInfo
                                     + (UINT8*)NvRamMailbox->VarStoreInfoPtr
                                 );
}
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmmGetVariable
//
// Description: This function searches for Var with specific GUID and Name in SMM
//              synchronizing Varstors before and after operation
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              OPTIONAL OUT UINT32* Attributes - Pointer to memory where Attributes will be returned 
//              IN OUT UINTN *DataSize - size of Var - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              OUT VOID *Data - Pointer to memory where Var will be returned
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SmmGetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
)
{
    EFI_STATUS Status;
    BEGIN_CRITICAL_SECTION(NvramCs);
    SynchronizeBeforeOperation();
    Status = DxeGetVariable(
                 VariableName, VendorGuid, Attributes, DataSize, Data
             );
    SynchronizeAfterOperation();
    END_CRITICAL_SECTION(NvramCs);
    return Status;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmmSetVariable
//
// Description: This function sets Var with specific GUID, Name and attributes in SMM
//              synchronizing Varstors before and after operation
//
// Input:       IN CHAR16 *VariableName - pointer to Var Name in Unicode
//              IN EFI_GUID *VendorGuid - pointer to Var GUID
//              IN UINT32 Attributes - Attributes of the Var
//              IN UINTN DataSize - size of Var
//              IN VOID *Data - Pointer to memory where Var data is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SmmSetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
)
{
    EFI_STATUS Status;
    
    BEGIN_CRITICAL_SECTION(NvramCs);
    SynchronizeBeforeOperation();
    Status = DxeSetVariable(
                 VariableName, VendorGuid, Attributes, DataSize, Data
             );
    SynchronizeAfterOperation();
    END_CRITICAL_SECTION(NvramCs);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmmGetNextVariableName
//
// Description: This function searches for next Var after Var with specific name and GUID and returns it's Name
//              in SMM synchronizing Varstors before and after operation. 
//
// Input:       IN OUT UINTN *VariableNameSize - size of Varname - if smaller than actual EFI_BUFFER_TOO_SMALL 
//              will be returned and DataSize will be set to actual size needed
//              IN OUT CHAR16 *VariableName - pointer where Var Name in Unicode will be stored
//              IN OUT EFI_GUID *VendorGuid - pointer to menory where Var GUID is stored
//
// Output:      EFI_STATUS - based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SmmGetNextVariableName (
    IN OUT UINTN *VariableNameSize,
    IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid
)
{
    EFI_STATUS  Status;
    
    BEGIN_CRITICAL_SECTION(NvramCs);
    SynchronizeBeforeOperation();
    Status = DxeGetNextVariableName(
                 VariableNameSize, VariableName, VendorGuid
             );
    SynchronizeAfterOperation();
    END_CRITICAL_SECTION(NvramCs);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmmQueryVariableInfo
//
// Description: This function returns parameters of VarStore with passed attributes in SMM
//              synchronizing Varstors before operation.
//
// Input:       IN UINT32 Attributes - Atributes to search for
//              OUT UINT64 *MaximumVariableStorageSize - Maximum Variable Storage Size
//              OUT UINT64 *RemainingVariableStorageSize - Remaining Variable Storage Size
//              OUT UINT64 *MaximumVariableSize - Maximum Variable Size
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SmmQueryVariableInfo (
    IN UINT32 Attributes,
    OUT UINT64 *MaximumVariableStorageSize,
    OUT UINT64 *RemainingVariableStorageSize,
    OUT UINT64 *MaximumVariableSize
)
{
    EFI_STATUS  Status;
    
    SynchronizeBeforeOperation();
    Status = QueryVariableInfo(
                 Attributes, MaximumVariableStorageSize,
                 RemainingVariableStorageSize, MaximumVariableSize
             );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvramSmmInit
//
// Description: This function gets NvRamMailbox and initialize pRS Var servicies and
//              corresponding structures
//
//  Input:      IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - pointer to System Table
//
// Output:      EFI_STATUS based on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NvramSmmInit(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS  Status;
	
	//CSP20131017 - WHCK test TPM ISSUE. >>
    EFI_HANDLE                      Handle;
    EFI_SMM_VARIABLE_PROTOCOL  *mSmmVariable;
	//CSP20131017 - WHCK test TPM ISSUE. <<
	    
    UINTN       Size=sizeof(NvRamMailbox);
    
    Status = pST->RuntimeServices->GetVariable(
                 L"NvRamMailbox",&NvRamMailboxVariableGuid,
                 NULL, &Size, &NvRamMailbox
             );
             
    if (EFI_ERROR(Status)) return Status;
    
    RtVarStoreInfoPtr = NvRamMailbox->VarStoreInfoPtr;
    VarStoreInfo=*RtVarStoreInfoPtr;
    VarIndexPtr=NvRamMailbox->VarIndexPtr;
    NvramCs=NvRamMailbox->NvramCs;

    if (NvramRtGarbageCollectionSupport){
        NvramDriverBuffer = NvRamMailbox->NvramDriverBuffer;
        NvramDriverBufferSize = NvRamMailbox->NvramBuffSize;
    }

    VarStoreInfo.MemInterface=&MemInterface;
    VarStoreInfo.NvInfo =
        &VarStoreInfo.NvramInfo[
            RtVarStoreInfoPtr->NvInfo - RtVarStoreInfoPtr->NvramInfo
        ];
    VarStoreInfo.MemInfo =
        &VarStoreInfo.NvramInfo[
            RtVarStoreInfoPtr->MemInfo - RtVarStoreInfoPtr->NvramInfo
        ];
#ifdef HYBRID_NV_INTERFACE
    VarStoreInfo.NvInterface=&HybridInterface;
#else
    if (!(VarStoreInfo.NvramMode & NVRAM_MODE_SIMULATION))
	    VarStoreInfo.NvInterface=&NvInterface;
    else
	    VarStoreInfo.NvInterface=&MemInterface;
#endif
        
    pRS->GetNextVariableName = SmmGetNextVariableName;
    pRS->GetVariable = SmmGetVariable;
    pRS->SetVariable = SmmSetVariable;
    pRS->QueryVariableInfo = SmmQueryVariableInfo;
    Status = pBS->LocateProtocol(&gFlashSmmProtocolGuid, NULL, &Flash);
    ASSERT_EFI_ERROR (Status);
    
	//CSP20131017 - WHCK test TPM ISSUE. >>
    // insatll the SMM variable protocol
    Status = pSmst->SmmAllocatePool (
        EfiRuntimeServicesData,
        sizeof(EFI_SMM_VARIABLE_PROTOCOL), &mSmmVariable
    );
    ASSERT_EFI_ERROR (Status); 

    mSmmVariable->SmmGetVariable  = SmmGetVariable;
    mSmmVariable->SmmGetNextVariableName  = SmmGetNextVariableName;
    mSmmVariable->SmmSetVariable  = SmmSetVariable;
    mSmmVariable->SmmQueryVariableInfo  = SmmQueryVariableInfo;
    
    Handle = NULL;
    Status = pSmst->SmmInstallProtocolInterface (
                      &Handle,
                      &gEfiSmmVariableProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      mSmmVariable
                      );
    ASSERT_EFI_ERROR (Status);    
	//CSP20131017 - WHCK test TPM ISSUE. <<
	    
    AuthVariableServiceInitSMM();
    InstallNvramControlSmmProtocol();
    return EFI_SUCCESS;
}

/*************** INSIDE and OUSIDE SMM ************************/
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NvramNotInSmmExitBs
//
// Description: This function signals that exit BS event was taken place and switches
//              SwitchToRuntime field in NvRamMailbox
//
//  Input:      NONE
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID NvramNotInSmmExitBs()
{
    NvRamMailbox->SwitchToRuntime = !NvRamMailbox->SwitchToRuntime;
#ifdef DO_NOT_ACCESS_DATA_OUTSIDE_OF_SMM_AT_RUNTIME
    //Make a dummy SMM GetVariable call to process SwitchToRuntime
    //make sure GetVariable is substituted
    if (pRS->GetVariable!=DxeGetVariableSafe){
        static EFI_GUID DummyGuid={0};
        UINTN Size = 0;
        UINTN Dummy;
        pRS->GetVariable(L"Dummy", &DummyGuid,NULL, &Size, &Dummy);
    }
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	NvRamSmmEntry
//
// Description:	This function is the entry point for this DXE. This function
//				installs NvRam services in and outside SMM.
//
// Input:		IN EFI_HANDLE ImageHandle -	Image handle
//				IN EFI_SYSTEM_TABLE *SystemTable -	Pointer to the system table
//
// Output:		EFI_STATUS - Status based on errors that occurred while waiting for
//				time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EFIAPI NvRamSmmEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{
    InitAmiLib(ImageHandle,SystemTable);
    return InitSmmHandlerEx(
               ImageHandle, SystemTable, NvramSmmInit, NvramNotInSmmInit
           );
}
//**********************************************************************
// NVRAM SMM END
//**********************************************************************
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
