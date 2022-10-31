//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
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
// $Header: /Alaska/BIN/Core/Include/AmiDxeLib.h 66    11/08/11 5:22p Oleksiyy $
//
// $Revision: 66 $
//
// $Date: 11/08/11 5:22p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiDxeLib.h
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#ifndef __AMI_DXE_LIB__H__
#define __AMI_DXE_LIB__H__
#ifdef __cplusplus
extern "C" {
#endif
#include <AmiProtocol.h>
#include <AmiLib.h>
#include <Dxe.h>
#include <Protocol/FirmwareVolume.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/HiiUtilities.h>
#include <Protocol/SmmBase.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/BootScriptSave.h>
#include <Protocol/S3SaveState.h>
#include <Library/PerformanceLib.h>

//****************************************************************************/
//		TYPE DECLARATIONS
//****************************************************************************/
//**********************************************************************
//<AMI_THDR_START>
//
// Name: DBE_DATABASE_STRUCT
//
// Fields:	
// ColNum	Type		Description
//	1		UINTN		Initial elements Count of index array to be created
//	2		UINTN		Current RecordCount stored in Database
//	3		UINTN		Number of Indexes in Database
//	4		DBE_INDEX*	Pointerto the indexes information of this Database;
//	5		VOID		*IndexArray;
//
// Description:	Initial structure to hold Database information
//
// Notes:	
//
//<AMI_THDR_END>
//**********************************************************************

typedef struct _DBE_KEY_FIELD DBE_KEY_FIELD;

typedef INTN (EFIAPI *DBE_CMP)(
				IN VOID				*pContext, 
				IN VOID				*pRecord1,
				IN VOID				*pRecord2
);

struct _DBE_KEY_FIELD {
	DBE_CMP	RRcmp;
	DBE_CMP	KRcmp;
	VOID *pContext;
};

typedef struct _DBE_OFFSET_KEY_CONTEXT {
	UINTN				Offset;
	UINTN				Size;
}DBE_OFFSET_KEY_CONTEXT;

typedef struct _DBE_DATABASE{
	EFI_MEMORY_TYPE			MemoryType;
	UINTN					InitialCount;
	UINTN					RecordCount;
	UINTN					KeyCount;
	DBE_KEY_FIELD			*KeyField;
	VOID					*IndexArray;
}DBE_DATABASE;

struct _DLINK;
typedef struct _DLINK DLINK;
struct _DLINK{
    DLINK *pNext, *pPrev;
};

typedef struct _DLIST{
	UINTN Size;
	DLINK *pHead, *pTail;
} DLIST;

//Structure to store pointers to data
//similar to VECTOR
typedef struct _T_ITEM_LIST {
	UINTN				InitialCount;
	UINTN				ItemCount;					
	VOID				**Items;
}T_ITEM_LIST;

//Structure to reserve CSP resources
typedef struct _CSP_RES_ITEM {
	UINT64				ResBase;
	UINTN				ResLength;
	UINTN				ResType;
	UINT64				Attributes;
} CSP_RES_ITEM;

#pragma pack(push,1)
typedef struct{
    EFI_GUID SubtypeGuid;
	UINT32 NumberOfPackages;
} HII_RESOURCE_SECTION_HEADER;
#pragma pack(pop)

//****************************************************************************/
//		MACROS DECLARATIONS
//****************************************************************************/
#define AMI_IRQ_DMA_MASK_VARIABLE_GUID \
  		{ 0xfc8be767, 0x89f1, 0x4d6e, 0x80, 0x99, 0x6f, 0x2, 0x1e, 0xbc, 0x87, 0xcc }

// {395C33FE-287F-413e-A055-8088C0E1D43E}
#define EFI_SMM_RUNTIME_SERVICES_TABLE_GUID \
    { 0x395c33fe, 0x287f, 0x413e, { 0xa0, 0x55, 0x80, 0x88, 0xc0, 0xe1, 0xd4, 0x3e } }

#define OFFSET_CONTEXT(Type, Field) { EFI_FIELD_OFFSET(Type,Field), EFI_FIELD_SIZE(Type,Field) }
#define OFFSET_KEY(Context) {	&OffsetRRCmp, &OffsetKRCmp, &Context }
#define ADDRESS_KEY {	&AddrRRCmp, &AddrKRCmp, NULL }

#define new(Type) Malloc(sizeof(Type))
#define delete(p) FreePool(p)
#define DListEmpty(pList) (!(pList)->pHead)

#ifdef EFI_DEBUG
#define TRACE(Arguments) Trace Arguments

#ifndef ASSERT
#define ASSERT(Condition) if(!(Condition)) { \
	Trace(-1,"ASSERT in %s on %i: %s\n",__FILE__, __LINE__, #Condition);\
	EFI_DEADLOOP() \
	}
#define ASSERT_EFI_ERROR(Status)  ASSERT(!EFI_ERROR(Status))
#endif

#define VERIFY ASSERT
#define VERIFY_EFI_ERROR ASSERT_EFI_ERROR

#else // EFI_DEBUG==0
#define TRACE(Arguments)

#ifndef ASSERT
#define ASSERT(Condition)
#define ASSERT_EFI_ERROR(Status)

#endif

#define VERIFY(x) (x)
#define VERIFY_EFI_ERROR(x) (x)
#endif

#define PROGRESS_CODE(Code)\
	LibReportStatusCode(EFI_PROGRESS_CODE, Code, 0, NULL, NULL)

#define ERROR_CODE(Code, Severity)\
	LibReportStatusCode(EFI_ERROR_CODE|(Severity), Code, 0, NULL, NULL)

#define DEVICE_PROGRESS_CODE(Code,DeviceHandle)\
	LibReportStatusCode(EFI_PROGRESS_CODE, Code, 0, NULL, NULL)

#define DEVICE_ERROR_CODE(Code, Severity, DeviceHandle)\
	LibReportStatusCode(EFI_ERROR_CODE|(Severity), Code, 0, NULL, NULL)

//Performance Measurement Macros
// Library/PerformanceLib.h
/*
#ifdef EFI_DXE_PERFORMANCE
#define PERF_START(handle, token, host, ticker) StartMeasure (handle, token, host, ticker)
#define PERF_END(handle, token, host, ticker)   EndMeasure (handle, token, host, ticker)
#define PERF_CODE(code) code
#else
#define PERF_START(handle, token, host, ticker)
#define PERF_END(handle, token, host, ticker)
#define PERF_CODE(code)
#endif
*/

#define LANGUAGE_RFC_4646
#define LANGUAGE_CODE_ENGLISH    "en-US"

#define HII_RESOURCES_FFS_SECTION_GUID\
    {0x97e409e6, 0x4cc1, 0x11d9, 0x81, 0xf6, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}

//the macro serves as an indicator that this version of library
//contains HII-related functions such as HiiLibGetString/HiiLibSetString
#define HII_LIBRARY_FUNCTIONS_SUPPORT

#if (!defined(BACKWARD_COMPATIBLE_MODE) || defined(SMM_BUILD)) && PI_SPECIFICATION_VERSION >= 0x0001000A
#define pSmst pSmstPi
#define pSmmBase pSmmBasePi
#define GetSmstConfigurationTable GetSmstConfigurationTablePi
#define InitSmmHandler InitSmmHandlerPi
#define InitSmmHandlerEx InitSmmHandlerPi
#define InitAmiSmmLib InitAmiSmmLibPi
#else
#define pSmst pSmstFramework
#define pSmmBase pSmmBaseFramework
#define GetSmstConfigurationTable GetSmstConfigurationTableFramework
#define InitSmmHandler InitSmmHandlerFramework
#define InitSmmHandlerEx InitSmmHandlerExFramework
#define InitAmiSmmLib InitAmiSmmLibFramework
#endif

//Boot Script macros
#if BACKWARD_COMPATIBLE_MODE
#define __PI_BOOT_SCRIPT_MACRO__(This, FrameworkCommand, PiCommand)\
    ( (This==_GetBootScriptSaveInterface())  ? (FrameworkCommand) : (PiCommand) )
#else
#define __PI_BOOT_SCRIPT_MACRO__(This, FrameworkCommand, PiCommand) PiCommand
#endif

// S3SaveStateProtocol
#define BOOT_SCRIPT_S3_IO_WRITE_MACRO(This, Width, Address, Count, Buffer) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_IO_WRITE_MACRO(This, EFI_ACPI_S3_RESUME_SCRIPT_TABLE, Width, Address, Count, Buffer),\
        ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,Width,(UINT64)Address,(UINTN)Count,Buffer)\
    )

#define BOOT_SCRIPT_S3_IO_READ_WRITE_MACRO(This,Width,Address,Data,DataMask) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_IO_READ_WRITE_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Data,DataMask),\
        ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_IO_READ_WRITE_OPCODE,Width,(UINT64)Address,Data,DataMask)\
    )

#define BOOT_SCRIPT_S3_MEM_WRITE_MACRO(This,Width,Address,Count,Buffer) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_MEM_WRITE_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Count,Buffer),\
	    ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_MEM_WRITE_OPCODE,Width,(UINT64)Address,(UINTN)Count,Buffer)\
    )

#define BOOT_SCRIPT_S3_MEM_READ_WRITE_MACRO(This,Width,Address,Data,DataMask) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_MEM_READ_WRITE_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Data,DataMask),\
	    ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_MEM_READ_WRITE_OPCODE,Width,(UINT64)Address,Data,DataMask)\
    )

#define BOOT_SCRIPT_S3_PCI_CONFIG_WRITE_MACRO(This,Width,Address,Count,Buffer) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_PCI_CONFIG_WRITE_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Count,Buffer),\
	    ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_PCI_CONFIG_WRITE_OPCODE,Width,(UINT64)Address,(UINTN)Count,Buffer)\
    )

#define BOOT_SCRIPT_S3_PCI_CONFIG_READ_WRITE_MACRO(This,Width,Address,Data,DataMask) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_PCI_CONFIG_READ_WRITE_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Data,DataMask),\
	    ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_PCI_CONFIG_READ_WRITE_OPCODE,Width,(UINT64)Address,Data,DataMask)\
    )

#define BOOT_SCRIPT_S3_SMBUS_EXECUTE_MACRO(This,SlaveAddress,Command,Operation,PecCheck,Length,Buffer) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_SMBUS_EXECUTE_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,SlaveAddress,Command,Operation,PecCheck,Length,Buffer),\
	    ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_SMBUS_EXECUTE_OPCODE,SlaveAddress,Command,Operation,PecCheck,Length,Buffer)\
    )

#define BOOT_SCRIPT_S3_STALL_MACRO(This,Duration) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_STALL_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Duration),\
	    ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_STALL_OPCODE,Duration)\
    )

#define BOOT_SCRIPT_S3_DISPATCH_MACRO(This,EntryPoint) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_DISPATCH_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,EntryPoint),\
        ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_DISPATCH_2_OPCODE,(EFI_PHYSICAL_ADDRESS)EntryPoint,NULL)\
    )

#define BOOT_SCRIPT_S3_IO_POLL_MACRO(This,Width,Address,Mask,Data) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_IO_POLL_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Mask,Data),\
        ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_IO_POLL_OPCODE,Width,(UINT64)Address,Data,Mask,(UINT64)-1)\
    )

#define BOOT_SCRIPT_S3_IO_POLL2_MACRO(This,Width,Address,Mask,Data,Delay) \
    This->Write(This,EFI_BOOT_SCRIPT_IO_POLL_OPCODE,Width,(UINT64)Address,Data,Mask,(UINT64)Delay)

#define BOOT_SCRIPT_S3_MEM_POLL_MACRO(This,Width,Address,Mask,Data) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_MEM_POLL_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Mask,Data),\
        ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_MEM_POLL_OPCODE,Width,(UINT64)Address,Data,Mask,(UINT64)-1)\
    )

#define BOOT_SCRIPT_S3_MEM_POLL2_MACRO(This,Width,Address,Mask,Data,Delay) \
    This->Write(This,EFI_BOOT_SCRIPT_MEM_POLL_OPCODE,Width,(UINT64)Address,Data,Mask,(UINT64)Delay)

#define BOOT_SCRIPT_S3_PCI_POLL_MACRO(This,Width,Address,Mask,Data) \
	__PI_BOOT_SCRIPT_MACRO__(This,\
        BOOT_SCRIPT_PCI_POLL_MACRO(This,EFI_ACPI_S3_RESUME_SCRIPT_TABLE,Width,Address,Mask,Data),\
        ((EFI_S3_SAVE_STATE_PROTOCOL*)This)->Write((EFI_S3_SAVE_STATE_PROTOCOL*)This,EFI_BOOT_SCRIPT_PCI_CONFIG_POLL_OPCODE,Width,(UINT64)Address,Data,Mask,(UINT64)-1)\
    )

#define BOOT_SCRIPT_S3_PCI_POLL2_MACRO(This,Width,Address,Mask,Data,Delay) \
    This->Write(This,EFI_BOOT_SCRIPT_PCI_CONFIG_POLL_OPCODE,Width,(UINT64)Address,Data,Mask,(UINT64)Delay)

#define BOOT_SCRIPT_S3_DISPATCH_2_MACRO(This,EntryPoint,Context) \
	This->Write(This,EFI_BOOT_SCRIPT_DISPATCH_2_OPCODE,(EFI_PHYSICAL_ADDRESS)EntryPoint,(EFI_PHYSICAL_ADDRESS)Context)

#define BOOT_SCRIPT_S3_PCI_CONFIG2_WRITE_MACRO(This,Width,Segment,Address,Count,Buffer) \
	This->Write(This,EFI_BOOT_SCRIPT_PCI_CONFIG2_WRITE_OPCODE,Width,(UINT16)Segment,(UINT64)Address,(UINTN)Count,Buffer)

#define BOOT_SCRIPT_S3_PCI_CONFIG2_READ_WRITE_MACRO(This,Width,Segment,Address,Data,DataMask) \
	This->Write(This,EFI_BOOT_SCRIPT_PCI_CONFIG2_READ_WRITE_OPCODE,Width,(UINT16)Segment,(UINT64)Address,Data,DataMask)

#define BOOT_SCRIPT_S3_PCI_CFG2_POLL_MACRO(This,Width,Segment,Address,Mask,Result,Delay) \
    This->Write(This,EFI_BOOT_SCRIPT_PCI_CONFIG2_POLL_OPCODE,Width,(UINT16)Segment,(UINT64)Address,Mask,Result,(UINT64)Delay)


//****************************************************************************/
//		VARIABLE DECLARATIONS
//****************************************************************************/
extern EFI_SYSTEM_TABLE			*pST;
extern EFI_BOOT_SERVICES		*pBS;
extern EFI_RUNTIME_SERVICES		*pRS;
#if (!defined(BACKWARD_COMPATIBLE_MODE) || defined(SMM_BUILD)) 
extern EFI_SMM_BASE2_PROTOCOL	*pSmmBasePi;
extern EFI_SMM_SYSTEM_TABLE2	*pSmstPi;
#else
extern EFI_SMM_BASE_PROTOCOL	*pSmmBaseFramework;
extern EFI_SMM_SYSTEM_TABLE		*pSmstFramework;
#endif
extern EFI_HANDLE               TheImageHandle;
extern EFI_GUID					guidDevicePath;
extern EFI_GUID					guidFV;
//****************************************************************************/
//		FUNCTION DECLARATIONS
//****************************************************************************/
VOID * GetEfiConfigurationTable(
	IN EFI_SYSTEM_TABLE *SystemTable,
	IN EFI_GUID			*Guid
);

DBE_DATABASE* DbeCreateDatabase(UINTN KeyCount, UINTN InitialCount);

EFI_STATUS DbeInsert(DBE_DATABASE *Database,VOID* Record);

EFI_STATUS DbeLocateKey(IN DBE_DATABASE *Database,IN UINTN KeyNo,IN	VOID *KeyValue,OUT VOID	**Record,OUT INT8 *Vicinity, OPTIONAL OUT	UINTN *RecordIndex);

EFI_STATUS DbeGoToIndex(IN DBE_DATABASE	*Database,IN UINTN KeyNo, IN INTN Index, OUT VOID **Record);

EFI_STATUS DbeDelete(IN DBE_DATABASE *Database, IN VOID* Data, BOOLEAN FreeData);

INTN OffsetRRCmp(
	IN VOID *pContext, IN VOID *pRecord1, IN VOID *pRecord2
);

INTN OffsetKRCmp(
	IN DBE_OFFSET_KEY_CONTEXT *pContext, IN VOID *pKey, IN VOID *pRecord
);

INTN AddrRRCmp(
	IN VOID* *pContext, VOID *pRecord1, VOID *pRecord2
);

INTN AddrKRCmp(
	IN DBE_OFFSET_KEY_CONTEXT *pContext, VOID *pKey, VOID *pRecord
);

//List functions
VOID DListInit(DLIST* pList);
VOID DListAdd(DLIST* pList, DLINK* pElement);
VOID DListDelete(DLIST* pList, DLINK* pElement);
VOID DListInsert(DLIST* pList, DLINK* pElement, DLINK* pAfter);

/*
EFI_STATUS DbeAddIndexKeyEntry(DBE_DATABASE *Database, UINTN KeyNo, VOID* Data);

EFI_STATUS DbeNext(IN DBE_DATABASE *Database,IN UINTN KeyNo,IN OUT VOID **Record);

EFI_STATUS DbePrev(IN DBE_DATABASE *Database,IN UINTN KeyNo,IN OUT VOID **Record);

EFI_STATUS DbeFirst(IN DBE_DATABASE *Database,IN UINTN KeyNo, OUT VOID **Record);

EFI_STATUS DbeLast(IN DBE_DATABASE *Database,IN UINTN KeyNo, OUT VOID **Record);

EFI_STATUS DbeDelIndexKeyEntry(IN DBE_DATABASE *Database, IN UINTN KeyNo, IN VOID* Data);

VOID* DbeGetKeyPtrValue(IN DBE_DATABASE *Database, IN UINTN KeyNo, VOID* Data);

VOID* DbeGetKeyPtrAddr(IN DBE_DATABASE *Database, IN UINTN KeyNo, VOID* Data);

VOID* DbeGetKeyPtrPtr(IN DBE_DATABASE *Database, IN UINTN KeyNo, VOID* Data);
*/

#ifdef EFI_DEBUG
void Trace(UINTN /* Level*/, CHAR8 * /*sFormat*/,...);
void PrintDebugMessageVaList(UINTN /* Level*/, CHAR8 * /*sFormat*/, VA_LIST /*ArgList*/);
BOOLEAN GetImageName(EFI_HANDLE ImageHandle, CHAR8 *sName);
#endif
BOOLEAN GetControllerName(EFI_HANDLE Controller, CHAR16 **wsName);

void* Malloc(UINTN Size);

void* MallocZ(UINTN Size);

VOID InitAmiLib(
	IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
);

UINTN DPLength(EFI_DEVICE_PATH_PROTOCOL *pDp);
VOID* DPCopy(EFI_DEVICE_PATH_PROTOCOL *pDp);
VOID* DPAdd(EFI_DEVICE_PATH_PROTOCOL *pDp1, EFI_DEVICE_PATH_PROTOCOL *pDp2);
VOID* DPAddNode(EFI_DEVICE_PATH_PROTOCOL *pDp1, EFI_DEVICE_PATH_PROTOCOL *pDp2);
VOID* DPAddInstance(EFI_DEVICE_PATH_PROTOCOL *pDp1, EFI_DEVICE_PATH_PROTOCOL *pDp2);
VOID* DPGetLastNode(EFI_DEVICE_PATH_PROTOCOL *pDp);
INTN  DPCmp(EFI_DEVICE_PATH_PROTOCOL *pDp1, EFI_DEVICE_PATH_PROTOCOL *pDp2);
VOID* DPCut(EFI_DEVICE_PATH_PROTOCOL *pDp);
BOOLEAN DPIsOneOf(EFI_DEVICE_PATH_PROTOCOL *pAll, EFI_DEVICE_PATH_PROTOCOL *pOne, BOOLEAN ExactMatch);
VOID* DPNextInstance(EFI_DEVICE_PATH_PROTOCOL **ppDp, UINTN *pSize);
VOID* DPCreateNode(UINT8 Type, UINT8 SubType, UINT16 Length);
BOOLEAN DPIsMultiInstance(EFI_DEVICE_PATH_PROTOCOL* pDp);
EFI_DEVICE_PATH_PROTOCOL* DPGetEndNode(EFI_DEVICE_PATH_PROTOCOL *pDp);

EFI_EVENT TimerStart(BOOLEAN *pTimerFlag, UINT64 Delay);
void TimerStop(BOOLEAN TimerFlag, EFI_EVENT Event);

EFI_STATUS FvReadPe32Image (
  IN EFI_GUID	*NameGuid,
  IN OUT VOID	**Buffer,
  IN OUT UINTN	*BufferSize,
  OUT UINT32	*AuthenticationStatus);

EFI_STATUS ReadImage (
	IN BOOLEAN BootPolicy, IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
	IN OUT VOID **ppSource, IN OUT UINTN *pSourceSize OPTIONAL,
	OUT UINTN *pImageSize OPTIONAL,
	OUT EFI_HANDLE *phDevice OPTIONAL, OUT EFI_DEVICE_PATH_PROTOCOL **ppPath OPTIONAL
);

//Tests if region1(s-e) belongs to the region2(S-E)
//returns EFI_ACCESS_DENIED if Belongs=FALSE and region 1 belong to region 2
//returns EFI_ACCESS_DENIED if Belongs=TRUE and region 1 don't belong to region 2
//returns EFI_SUCCESS in all other cases.
EFI_STATUS TestRegions(UINT64 s, UINT64 e, UINT64 S, UINT64 E, BOOLEAN Belongs);

EFI_STATUS AppendItemLst(T_ITEM_LIST *Lst, VOID* pRes);

EFI_STATUS InsertItemLst(T_ITEM_LIST *Lst, VOID* pRes, UINTN ItemIndex);

EFI_STATUS DeleteItemLst(T_ITEM_LIST *Lst, UINTN ItemIndex, BOOLEAN FreeData);

VOID ClearItemLst(T_ITEM_LIST *Lst, BOOLEAN FreeData);

EFI_STATUS CopyItemLst(T_ITEM_LIST *Lst, T_ITEM_LIST **NewLstPtr);

EFI_STATUS GetEfiVariable(
	IN CHAR16 *sName, IN EFI_GUID *pGuid,
	OUT UINT32 *pAttributes OPTIONAL,
	IN OUT UINTN *pDataSize, OUT VOID **ppData
);

EFI_STATUS GetDefaultLang(UINT16 *DefaultLang);

EFI_STATUS ReadImageResource(
	EFI_HANDLE ImageHandle, EFI_GUID *pGuid, 
	VOID **ppData, UINTN *pDataSize
);

EFI_STATUS RegisterProtocolCallback(
	IN EFI_GUID *pProtocol, IN EFI_EVENT_NOTIFY	NotifyFunction,
	IN VOID *pNotifyContext, OUT EFI_EVENT *pEvent,
	OUT VOID **ppRegistration
);

EFI_STATUS LoadResources(
	EFI_HANDLE ImageHandle, UINTN NumberOfCallbacks,
	CALLBACK_INFO *pCallBack, INIT_HII_PACK InitFunction
);

EFI_STATUS LoadStrings(
	EFI_HANDLE ImageHandle, EFI_HII_HANDLE *pHiiHandle
);

CHAR8* GetBestSupportedLanguage (
    IN EFI_HII_HANDLE   HiiHandle,
    ...
);

EFI_STATUS HiiLibGetString(
	IN EFI_HII_HANDLE HiiHandle, IN STRING_REF StringId, 
	IN OUT UINTN *StringSize, OUT EFI_STRING String
);
EFI_STATUS HiiLibSetString(
	IN EFI_HII_HANDLE HiiHandle, IN STRING_REF StringId, IN EFI_STRING String
);
EFI_STATUS HiiLibPublishPackages(
    IN VOID *PackagePointers, IN UINTN NumberOfPackages, 
    IN EFI_GUID *PackageGuid, IN EFI_HANDLE DriverHandle OPTIONAL,
    OUT EFI_HII_HANDLE *HiiHandle
);
EFI_STATUS HiiLibGetBrowserData(
    IN OUT UINTN *BufferSize, OUT VOID *Buffer, 
    IN CONST EFI_GUID *VarStoreGuid, OPTIONAL 
    IN CONST CHAR16 *VarStoreName  OPTIONAL
);
EFI_STATUS HiiLibSetBrowserData(
    IN UINTN BufferSize, IN VOID *Buffer, 
    IN CONST EFI_GUID *VarStoreGuid, OPTIONAL 
    IN CONST CHAR16 *VarStoreName  OPTIONAL
);
EFI_STATUS HiiLibGetGlyphWidth(
    IN CHAR16 Char, 
    OUT UINT16 *Width
);

EFI_STATUS HiiLibGetGlyphInfo(
	IN VOID *FontHandle, 
    IN CHAR16 Char,
    OUT UINT32 *GlyphWidth, OPTIONAL
    OUT UINT32 *GlyphHeight OPTIONAL
);

EFI_STATUS LibGetDxeSvcTbl(DXE_SERVICES **ppDxe OPTIONAL);

EFI_STATUS LibAllocCspResource(CSP_RES_ITEM *ResTable, UINTN ResCount,
							EFI_HANDLE ImgHandle,EFI_HANDLE CntrHandle);

EFI_STATUS AllocCspResource(DXE_SERVICES *Dxe, CSP_RES_ITEM *ResTable, UINTN ResCount,
							EFI_HANDLE ImgHandle,EFI_HANDLE CntrHandle, BOOLEAN AddSpace);

#if (!defined(BACKWARD_COMPATIBLE_MODE) || defined(SMM_BUILD)) 
EFI_STATUS InitSmmHandlerPi(
	IN EFI_HANDLE		ImageHandle,
	IN EFI_SYSTEM_TABLE	*SystemTable,
	IN EFI_STATUS		(*InSmmFunction)(EFI_HANDLE,EFI_SYSTEM_TABLE*),
	IN EFI_STATUS		(*NotInSmmFunction)(EFI_HANDLE,EFI_SYSTEM_TABLE *) OPTIONAL
);

VOID* GetSmstConfigurationTablePi(IN EFI_GUID *TableGuid);

EFI_STATUS InitAmiSmmLibPi(
    IN EFI_HANDLE	    ImageHandle,
    IN EFI_SYSTEM_TABLE	*SystemTable
);
#else
EFI_STATUS InitSmmHandlerFramework(
	IN EFI_HANDLE		ImageHandle,
	IN EFI_SYSTEM_TABLE	*SystemTable,
	IN EFI_STATUS		(*InSmmFunction)(EFI_HANDLE,EFI_SYSTEM_TABLE*),
	IN EFI_STATUS		(*NotInSmmFunction)(EFI_HANDLE,EFI_SYSTEM_TABLE *) OPTIONAL
);

EFI_STATUS InitSmmHandlerExFramework(
	IN EFI_HANDLE		ImageHandle,
	IN EFI_SYSTEM_TABLE	*SystemTable,
	IN EFI_STATUS		(*InSmmFunction)(EFI_HANDLE,EFI_SYSTEM_TABLE*),
	IN EFI_STATUS		(*NotInSmmFunction)(EFI_HANDLE,EFI_SYSTEM_TABLE *) OPTIONAL
);

VOID* GetSmstConfigurationTableFramework(IN EFI_GUID *TableGuid);

EFI_STATUS InitAmiSmmLibFramework(
    IN EFI_HANDLE	    ImageHandle,
    IN EFI_SYSTEM_TABLE	*SystemTable
);
#endif

EFI_STATUS CreateReadyToBootEvent(
    IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY	NotifyFunction,
	IN VOID *pNotifyContext, OUT EFI_EVENT *pEvent
);
EFI_STATUS CreateLegacyBootEvent(
    IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY	NotifyFunction,
	IN VOID *pNotifyContext, OUT EFI_EVENT *pEvent
);

EFI_STATUS LibReportStatusCode(
	IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
	IN UINT32 Instance, IN EFI_GUID *CallerId OPTIONAL,
	IN EFI_STATUS_CODE_DATA *Data OPTIONAL
);

EFI_STATUS AmiIsaIrqMask(UINT16 *IsaIrqMask, BOOLEAN Get);
EFI_STATUS AmiIsaDmaMask(UINT8 *IsaIrqMask, BOOLEAN Get);

//returns current boot mode
EFI_BOOT_MODE GetBootMode();

//Performance Measurement Functions
EFI_STATUS StartMeasure (
  IN EFI_HANDLE Handle, IN CHAR16 *Token, 
  IN CHAR16 *Host, IN UINT64 Ticker
);

EFI_STATUS EndMeasure (
  IN EFI_HANDLE Handle, IN CHAR16 *Token,
  IN CHAR16 *Host, IN UINT64 Ticker
);

VOID WriteBootToOsPerformanceData();

typedef enum {
    FillOsLoaderLoadImageStart,
    FillOsLoaderStartImageStart,
    FillExitBootServicesEntry,
    FillExitBootServicesExit
} FPDT_FILL_TYPE;

VOID AmiFillFpdt (IN FPDT_FILL_TYPE FieldToFill);

VOID EFIAPI InitAmiRuntimeLib(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
    IN EFI_EVENT_NOTIFY ExitBsCallback OPTIONAL,
    IN EFI_EVENT_NOTIFY GoVirtualCallback OPTIONAL
);
//The EfiAtRuntime and  EfiGoneVirtual functions will only work 
//if library has been initialized with InitAmiRuntimeLib
BOOLEAN EFIAPI EfiAtRuntime();
BOOLEAN EFIAPI EfiGoneVirtual();

BOOLEAN LanguageCodesEqual(CONST CHAR8* LangCode1, CONST CHAR8* LangCode2);
EFI_STATUS LibInitStatusCodePtr();
#ifdef PEI_BUILD
#define _GetBootScriptSaveInterface() NULL
#else
VOID* _GetBootScriptSaveInterface();
#endif
EFI_STATUS IsValidDevicePath(
    IN EFI_DEVICE_PATH_PROTOCOL *pDp
);
/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
