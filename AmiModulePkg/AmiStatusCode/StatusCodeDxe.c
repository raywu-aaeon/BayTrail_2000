//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
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
//<AMI_FHDR_START>
//
// Name:	StatusCodeDxe.c
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include "StatusCodeInt.h"
#include <AmiDxeLib.h>
#include <Protocol/DataHub.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/AMIPostMgr.h>
#include <Token.h>


UINT8 String[2048];
UINT16 StringMaxSize = 2048;
//----------------------------------------------------------------------------
ROUTER_STRUCT_HEADER *RouterCallbackStr;
BOOLEAN RouterRecurciveStatus = FALSE;
BOOLEAN InRscHandlerNotification = FALSE;

//----------------------------------------------------------------------------
extern INIT_FUNCTION DXE_INIT_LIST EndOfInitList;
INIT_FUNCTION* InitPartsList[] = {DXE_INIT_LIST NULL};

extern STRING_FUNCTION RT_STRING_LIST EndOfStringList;
STRING_FUNCTION* RtStringList[] = {RT_STRING_LIST NULL};

extern SIMPLE_FUNCTION RT_SIMPLE_LIST EndOfSimpleList;
SIMPLE_FUNCTION* RtSimpleList[] = {RT_SIMPLE_LIST NULL};

extern MISC_FUNCTION RT_MISC_LIST EndOfMiscList;
MISC_FUNCTION* RtMiscList[] = {RT_MISC_LIST NULL};

extern CHECKPOINT_FUNCTION RT_CHECKPOINT_LIST EndOfCheckpointList;
CHECKPOINT_FUNCTION* RtCheckpointList[] = {RT_CHECKPOINT_LIST NULL};

// We are listing both DXE and RT functions here to make sure that
// DXE array is at least as big as the RT array.
// We'll be overwriting DXE handlers with the runtime handlers in RuntimeStatusCodeExitBS.
extern STRING_FUNCTION DXE_STRING_LIST EndOfStringList;
STRING_FUNCTION* StringList[] = {DXE_STRING_LIST NULL, RT_STRING_LIST NULL};

extern SIMPLE_FUNCTION DXE_SIMPLE_LIST EndOfSimpleList;
SIMPLE_FUNCTION* SimpleList[] = {DXE_SIMPLE_LIST NULL, RT_SIMPLE_LIST NULL};

extern MISC_FUNCTION DXE_MISC_LIST EndOfMiscList;
MISC_FUNCTION* MiscList[] = {DXE_MISC_LIST NULL, RT_MISC_LIST NULL};

extern CHECKPOINT_FUNCTION DXE_CHECKPOINT_LIST EndOfCheckpointList;
CHECKPOINT_FUNCTION* CheckpointList[] = {DXE_CHECKPOINT_LIST NULL, RT_CHECKPOINT_LIST NULL};

BOOLEAN StatusRuntime = FALSE;

EFI_DATA_HUB_PROTOCOL *DataHub = NULL;
EFI_CONSOLE_CONTROL_PROTOCOL *ConsoleControl = NULL;
#if CON_OUT_CHECKPOINTS_IN_QUIET_MODE
AMI_POST_MANAGER_PROTOCOL *PostManager = NULL;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL Foreground = {0xFF,0xFF,0xFF,0};
EFI_GRAPHICS_OUTPUT_BLT_PIXEL Background = {0,0,0,0};
static EFI_GUID gAmiPostManagerProtocolGuidLocal = AMI_POST_MANAGER_PROTOCOL_GUID;
#endif
#define SC_ROUTER_ENTRIES 0xF
#define EFI_STANDARD_CALLER_ID_GUID \
  {0xC9DCF469, 0xA7C4, 0x11D5, 0x87, 0xDA, 0x00, 0x06, 0x29, 0x45, 0xC3, 0xB9}
#define EFI_STATUS_CODE_GUID \
  {0xd083e94c, 0x6560, 0x42e4, 0xb6, 0xd4, 0x2d, 0xf7, 0x5a, 0xdf, 0x6a, 0x2a}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RscHandlerNotification
//
// Description:
//      Executes delayed calls (when current TPL is higher that one router was 
//      registered with) to registered status code routers.
//
// Input:
//      N EFI_EVENT        Event - signaled event.
//      IN VOID            *Context - pointer to structure with parameters.
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID RscHandlerNotification (
    IN EFI_EVENT        Event,
    IN VOID             *Context
    )
{
    RSC_HANDLER_CALLBACK_ENTRY  *CallbackEntry;
    EFI_PHYSICAL_ADDRESS        Address;
    RSC_DATA_ENTRY              *RscData;

    CallbackEntry = (RSC_HANDLER_CALLBACK_ENTRY *) Context;


    // Traverse the status code data buffer to parse all data to report.

    Address = CallbackEntry->StatusCodeDataBuffer;
    InRscHandlerNotification = TRUE;
    while (Address < CallbackEntry->EndPointer) 
    {
        RscData = (RSC_DATA_ENTRY *) (UINTN) Address;
        CallbackEntry->RscHandlerCallback (
                        RscData->Type,
                        RscData->Value,
                        RscData->Instance,
                        &RscData->CallerId,
                        &RscData->Data
                        );

        Address += (sizeof (RSC_DATA_ENTRY) + RscData->Data.Size);
    }

    CallbackEntry->EndPointer = CallbackEntry->StatusCodeDataBuffer;
    InRscHandlerNotification = FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RouterRegister
//
// Description: Register the callback function for ReportStatusCode() notification 
//              in DXE and Runtime.
//
// Input:
//      IN EFI_RSC_HANDLER_CALLBACK Callback - A pointer to a function of type 
//                        EFI_RSC_HANDLER_CALLBACK that is to be registered.
//      IN EFI_TPL Tpl -  TPL at which callback can be safely invoked.
//
// Output:
//      EFI_SUCCESS           The function was successfully registered.
//      EFI_INVALID_PARAMETER The callback function was NULL.
//      EFI_OUT_OF_RESOURCES  The internal buffer ran out of space. No more functions can be
//                            registered.
//      EFI_ALREADY_STARTED   The function was already registered. It can't be registered again.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS RouterRegister (
    IN EFI_RSC_HANDLER_CALLBACK   Callback,
    IN EFI_TPL                    Tpl
    )
{
    EFI_STATUS                    Status;
    RSC_HANDLER_CALLBACK_ENTRY    *CallbackEntry;
    UINT32                        i;
    VOID                          *Ptr, *UnregisteredPtr=NULL;

    if (Callback == NULL) 
        return EFI_INVALID_PARAMETER;
    if (RouterCallbackStr == NULL)
        return EFI_OUT_OF_RESOURCES;
  
    Ptr = (UINT8*) RouterCallbackStr + sizeof (ROUTER_STRUCT_HEADER);

    for (i = 0; i != RouterCallbackStr->RegisteredEntries; i++) 
    {
        CallbackEntry = Ptr;

        if (CallbackEntry->RscHandlerCallback == NULL) 
        {
            UnregisteredPtr = Ptr;
        }
        else 
        {
            if (CallbackEntry->RscHandlerCallback == Callback) 
            // If the function was already registered. It can't be registered again.
                return EFI_ALREADY_STARTED;
        }
        Ptr = (UINT8*) Ptr + sizeof (RSC_HANDLER_CALLBACK_ENTRY);
    }
    if (UnregisteredPtr == NULL) //No Unregistered entries
    {
        if (RouterCallbackStr->RegisteredEntries == RouterCallbackStr->TotalEntries)
            return EFI_OUT_OF_RESOURCES; // And all entries are taken already - exit
        CallbackEntry = Ptr;
        RouterCallbackStr->RegisteredEntries++;
    }
    else
        CallbackEntry = UnregisteredPtr; // Will fill uregistered entry
    
    CallbackEntry->RscHandlerCallback = Callback;
    CallbackEntry->Tpl                = Tpl;

    // If TPL of registered callback funtion is not TPL_HIGH_LEVEL, then event should be created
    // for it, and related buffer for status code data should be prepared.
    // Here the data buffer must be prepared in advance, because Report Status Code Protocol might
    // be invoked under TPL_HIGH_LEVEL and no memory allocation is allowed then.
    // If TPL is TPL_HIGH_LEVEL, then all status code will be reported immediately, without data
    // buffer and event trigger.
    if ((Tpl != TPL_HIGH_LEVEL) && !StatusRuntime) 
    {
        VOID     *StatusCodeDataBuff;
        Status = pBS->AllocatePool (EfiRuntimeServicesData, EFI_PAGE_SIZE, &StatusCodeDataBuff);
        ASSERT_EFI_ERROR (Status);
        CallbackEntry->StatusCodeDataBuffer = (EFI_PHYSICAL_ADDRESS) (UINTN) StatusCodeDataBuff;
        CallbackEntry->BufferSize = EFI_PAGE_SIZE;
        CallbackEntry->EndPointer = CallbackEntry->StatusCodeDataBuffer;
        Status = pBS->CreateEvent (
                        EVT_NOTIFY_SIGNAL,
                        Tpl,
                        RscHandlerNotification,
                        CallbackEntry,
                        &CallbackEntry->Event
                        );
        ASSERT_EFI_ERROR (Status);
    }

    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RouterUnregister
//
// Description:  Remove a previously registered callback function from the notification list.
//
// Input:
//      IN EFI_RSC_HANDLER_CALLBACK Callback - A pointer to a function of type 
//                                  EFI_RSC_HANDLER_CALLBACK that is to be unregistered.
//
// Output:
//      EFI_SUCCESS           The function was successfully unregistered.
//      EFI_INVALID_PARAMETER The callback function was NULL.
//      EFI_NOT_FOUND         The callback function was not found to be unregistered.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS RouterUnregister (IN EFI_RSC_HANDLER_CALLBACK Callback)
{
    RSC_HANDLER_CALLBACK_ENTRY    *CallbackEntry;
    UINT32                        i;
    VOID                          *Ptr;

    if (Callback == NULL) 
        return EFI_INVALID_PARAMETER;
    if (RouterCallbackStr == NULL)
        return EFI_NOT_FOUND;

    Ptr = (UINT8*) RouterCallbackStr + sizeof (ROUTER_STRUCT_HEADER);

    for (i = 0; i != RouterCallbackStr->RegisteredEntries; i++) 
    {
        CallbackEntry = Ptr;

        if (CallbackEntry->RscHandlerCallback == Callback) 
        {
      
         // If the function is found in list, delete it and return.

            if ((CallbackEntry->Tpl != TPL_HIGH_LEVEL) && (CallbackEntry->StatusCodeDataBuffer != 0))
            {
                pBS->FreePool ((VOID *) (UINTN) CallbackEntry->StatusCodeDataBuffer);
                pBS->CloseEvent (CallbackEntry->Event);
            }
        pBS->SetMem(Ptr, sizeof(RSC_HANDLER_CALLBACK_ENTRY), 0);//Clear all data in entry
        return EFI_SUCCESS;
        }
        Ptr = (UINT8*) Ptr + sizeof (RSC_HANDLER_CALLBACK_ENTRY);
    }

  return EFI_NOT_FOUND;
}

EFI_RSC_HANDLER_PROTOCOL  RscHandlerProtocol = {
  RouterRegister,
  RouterUnregister
  };


BOOLEAN IsItRecursiveCall (
  IN OUT  BOOLEAN                   *Value,
  IN      BOOLEAN                   CompareWith,
  IN      BOOLEAN                   SvitchTo
  )
{   
    if (*Value == CompareWith)
    {
        *Value = SvitchTo;
        return CompareWith;
    }
    else return *Value;
  
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReportDxeAndRtRouter
//
// Description:
//  Top level status code reporting routine exposed by the status code protocol/PPI.
//  Calls the various types of status code handlers
//  (SimpleStatusReport, StringStatusReport, MiscStatusReport, PerformErrorCodeAction)
//  Generates string from the status code data to pass to StringStatusReport function.
//
// Input:
//      IN VOID *PeiServices - pointer to the PEI Boot Services table
//      IN EFI_STATUS_CODE_TYPE Type - the type and severity of the error that occurred
//      IN EFI_STATUS_CODE_VALUE Value - the Class, subclass and Operation that caused the error
//      IN UINT32 Instance -
//      IN EFI_GUI *CallerId OPTIONAL - The GUID of the caller function
//      IN EFI_STATUS_CODE_DATA *Data OPTIONAL - the extended data field that contains additional info
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReportDxeAndRtRouter (
    IN EFI_STATUS_CODE_TYPE     Type,
    IN EFI_STATUS_CODE_VALUE    Value,
    IN UINT32                   Instance,
    IN EFI_GUID                 *CallerId  OPTIONAL,
    IN EFI_STATUS_CODE_DATA     *Data      OPTIONAL
    )
{

    RSC_HANDLER_CALLBACK_ENTRY    *CallbackEntry;
    RSC_DATA_ENTRY                *RscData;
    EFI_STATUS                    Status;
    VOID                          *NewBuffer = NULL, *Ptr;
    EFI_TPL                       CurrentTpl = TPL_HIGH_LEVEL;
    UINT32                        i;
 
    if ((RouterCallbackStr == NULL) || (IsItRecursiveCall (&RouterRecurciveStatus, FALSE, TRUE) == TRUE)) 
        return EFI_ACCESS_DENIED;
    for (i = 0; i != RouterCallbackStr->RegisteredEntries; i++) 
    {
    
        if (i == 0)  
            Ptr = (UINT8*) RouterCallbackStr + sizeof (ROUTER_STRUCT_HEADER);
        else
            Ptr = (UINT8*) Ptr + sizeof (RSC_HANDLER_CALLBACK_ENTRY);
        CallbackEntry = Ptr;

        if (CallbackEntry->RscHandlerCallback == NULL) continue;//Unregistered function
    
        if (!StatusRuntime)
        {
            CurrentTpl = pBS->RaiseTPL (TPL_HIGH_LEVEL);
            pBS->RestoreTPL (CurrentTpl);
        }

        if ((CallbackEntry->Tpl >= CurrentTpl) || StatusRuntime) 
        {
            CallbackEntry->RscHandlerCallback (Type, Value, Instance, CallerId, Data);
            continue;
        }


        // If callback is registered with TPL lower than TPL_HIGH_LEVEL, event must be signaled at boot time to possibly wait for
        // allowed TPL to report status code. Related data should also be stored in data buffer.

        RscData = (RSC_DATA_ENTRY *) (UINTN) CallbackEntry->EndPointer;
        CallbackEntry->EndPointer += sizeof (RSC_DATA_ENTRY);
        if (Data != NULL) 
            CallbackEntry->EndPointer += Data->Size;


        // If data buffer is about to be used up (7/8 here), try to reallocate a buffer with double size, if not at TPL_HIGH_LEVEL.

        if (CallbackEntry->EndPointer > (CallbackEntry->StatusCodeDataBuffer + (CallbackEntry->BufferSize / 8) * 7)) 
        {
            if ((CurrentTpl < TPL_HIGH_LEVEL) && !InRscHandlerNotification)
            {
                Status = pBS->AllocatePool (EfiRuntimeServicesData, CallbackEntry->BufferSize * 2, &NewBuffer);
                //ASSERT_EFI_ERROR (Status);
                if (NewBuffer != NULL && CallbackEntry->StatusCodeDataBuffer != 0)
                {
                    pBS->SetMem(NewBuffer, CallbackEntry->BufferSize * 2, 0);
                    MemCpy (NewBuffer, (VOID*)CallbackEntry->StatusCodeDataBuffer, CallbackEntry->BufferSize);
                    pBS->FreePool ((VOID *) (UINTN) CallbackEntry->StatusCodeDataBuffer);
                }

                if (NewBuffer != NULL) 
                {
                    CallbackEntry->EndPointer = (EFI_PHYSICAL_ADDRESS) (UINTN) NewBuffer + (CallbackEntry->EndPointer - CallbackEntry->StatusCodeDataBuffer);
                    CallbackEntry->StatusCodeDataBuffer = (EFI_PHYSICAL_ADDRESS) (UINTN) NewBuffer;
                    CallbackEntry->BufferSize *= 2;
                }
            }
        }


        // If data buffer is used up, do not report for this time.

        if (CallbackEntry->EndPointer > (CallbackEntry->StatusCodeDataBuffer + CallbackEntry->BufferSize))
            continue;

        RscData->Type      = Type;
        RscData->Value     = Value;
        RscData->Instance  = Instance;
        if (CallerId != NULL) 
            MemCpy (&RscData->CallerId, CallerId, sizeof (EFI_GUID));
    
        if (Data != NULL) 
            MemCpy (&RscData->Data, Data, Data->HeaderSize + Data->Size);

        pBS->SignalEvent (CallbackEntry->Event);

    }
    // Restore the Recursive status of report

    IsItRecursiveCall (&RouterRecurciveStatus, TRUE, FALSE);
    return EFI_SUCCESS;
}

EFI_STATUS DxeRtReportStatusCode (
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
    IN UINT32 Instance, IN EFI_GUID *CallerId OPTIONAL,
    IN EFI_STATUS_CODE_DATA *Data OPTIONAL
)
{        
    ReportDxeAndRtRouter(Type, Value, Instance, CallerId, Data); 
  
    return AmiReportStatusCode(NULL, Type, Value, Instance, CallerId, Data, String);
}

EFI_STATUS_CODE_PROTOCOL  StatusCodeProtocol  = {
  DxeRtReportStatusCode 
};



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ScRuntimeVirtualAddressChange
//
// Description:
//      Updates pointers to RSC structure and to callback functions inside structure.
//
// Input:
//      N EFI_EVENT        Event - signaled event.
//      IN VOID            *Context - Context
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ScRuntimeVirtualAddressChange (IN EFI_EVENT Event, IN VOID *Context)
{

    RSC_HANDLER_CALLBACK_ENTRY      *CallbackEntry;
    VOID                            *Ptr;
    UINTN i;

    for (i=0; SimpleList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(SimpleList[i]));
    }

    for (i=0; StringList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(StringList[i]));
    }
  
    for (i=0; MiscList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(MiscList[i]));
    }

    for (i=0; CheckpointList[i]; i++){
    pRS->ConvertPointer(0,(VOID**)&(CheckpointList[i]));
    }

    if (RouterCallbackStr == NULL)
        return;
    for (i = 0; i != RouterCallbackStr->RegisteredEntries; i++) 
    {
    
        if (i == 0)  
            Ptr = (UINT8*) RouterCallbackStr + sizeof (ROUTER_STRUCT_HEADER);
        else
            Ptr = (UINT8*) Ptr + sizeof (RSC_HANDLER_CALLBACK_ENTRY);
        CallbackEntry = Ptr;

        if (CallbackEntry->RscHandlerCallback == NULL) continue;//Unregistered function
        else pRS->ConvertPointer(0,(VOID**)&(CallbackEntry->RscHandlerCallback));
    
    }

    pRS->ConvertPointer(0,(VOID**)&RouterCallbackStr);

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RuntimeStatusCodeExitBS
//
// Description: Exit Boot Services event handler.
//  Terminates boot time status code processing
//
// Input:
//      IN EFI_EVENT Event - not used
//      IN VOID *Context - not used
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#define SwitchToRtHandlers(Array)\
	for(i=0; Rt##Array[i]; i++){Array[i]=Rt##Array[i];} Array[i]=NULL
VOID RuntimeStatusCodeExitBS (IN EFI_EVENT Event, IN VOID *Context)
{
	UINTN i;
	StatusRuntime = TRUE;
	SwitchToRtHandlers(StringList);
	SwitchToRtHandlers(SimpleList);
	SwitchToRtHandlers(MiscList);
	SwitchToRtHandlers(CheckpointList);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DxeRuntimeInitStatusCode
//
// Description:
//  Entry point of the StatusCode Runtime Driver.
//  Calls InitStatusCodeParts function, installs the Status Code Protocol,
//  and registers event handlers for the legacy boot and exit boot services events.
//  RuntimeInitStatusCode installs different protocol based on value of the
//  EFI_SPECIFICATION_VERSION SDL token (defined in Core.sdl).
//  If value of the EFI_SPECIFICATION_VERSION constant is less than 0x20000,
//  framework EFI_STATUS_CODE_ARCH_PROTOCOL_GUID protocol is installed;
//  otherwise, PI EFI_STATUS_CODE_RUNTIME_PROTOCOL_GUID protocol is installed.
//  PI Status Code Protocol is initialized with the address of RuntimeReportStatusCode function.
//
// Input:
//      *ImageHandle - The firmware allocate handle for the EFI image
//      *SystemTable - pointer to the EFI System Table
//
// Output: EFI_STATUS
//
// Modified: StatusCode Arch Protocol- now published
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
DxeRuntimeInitStatusCode(
    IN EFI_HANDLE         ImageHandle,  
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
    EFI_STATUS     		Status;
    EFI_HANDLE              	DxeRtStatusCodeHandle = NULL;

    InitAmiRuntimeLib(
        ImageHandle, SystemTable, RuntimeStatusCodeExitBS, ScRuntimeVirtualAddressChange
    );

    InitStatusCodeParts(ImageHandle,SystemTable);

    Status = pBS->AllocatePool (EfiRuntimeServicesData, 
                     sizeof(ROUTER_STRUCT_HEADER) + (sizeof(RSC_HANDLER_CALLBACK_ENTRY) * SC_ROUTER_ENTRIES), 
                                   (VOID**) &RouterCallbackStr); 
    if (EFI_ERROR(Status)) return Status;

    pBS->SetMem(RouterCallbackStr, sizeof(ROUTER_STRUCT_HEADER) + 
                     (sizeof(RSC_HANDLER_CALLBACK_ENTRY) * SC_ROUTER_ENTRIES), 0);
    RouterCallbackStr->TotalEntries = SC_ROUTER_ENTRIES;

    Status = pBS->InstallMultipleProtocolInterfaces (
                  &DxeRtStatusCodeHandle,
                  &gEfiRscHandlerProtocolGuid,
                  &RscHandlerProtocol,
                  &gEfiStatusCodeRuntimeProtocolGuid,
                  &StatusCodeProtocol,
                  NULL
                  );
    ASSERT_EFI_ERROR (Status);

    return EFI_SUCCESS;
}

//<AMI_GHDR_START>
//----------------------------------------------------------------------------
// Name:    Data_Hub_Logging
//
// Description:
//  DataHub Logging Functions:
//   - DataHubInint
//   - DataHubInstalled
//   - DataHubStatusCode
//----------------------------------------------------------------------------
//<AMI_GHDR_END>

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DataHubStatusCode
//
// Description: Logs status code data into the data hub
//
// Input:
//  IN  EFI_STATUS_CODE_TYPE        Type
//  IN  EFI_STATUS_CODE_VALUE       Value
//  IN  UINT32                      Instance
//  IN  EFI_GUID                    *CallerId OPTIONAL
//  IN  EFI_STATUS_CODE_DATA        *Data OPTIONAL
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DataHubStatusCode (
    IN VOID *Dummy, //dummy parameter for compatibility with PEI callback interface
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
    IN UINT32 Instance, IN CONST EFI_GUID *CallerId OPTIONAL,
    IN EFI_STATUS_CODE_DATA *Data OPTIONAL
)
{
    EFI_STATUS Status;
    DATA_HUB_STATUS_CODE_DATA_RECORD *Record;
    UINT32 Size;
    UINT8 CodeType = STATUS_CODE_TYPE(Type);
    
    static EFI_GUID gEfiCallerIdGuid = EFI_STANDARD_CALLER_ID_GUID;
    static EFI_GUID gEfiStatusCodeGuid = EFI_STATUS_CODE_GUID;
    static UINT8 DataHubClasses[] = {EFI_DATA_CLASS_PROGRESS_CODE,EFI_DATA_CLASS_ERROR,EFI_DATA_CLASS_DEBUG};
    
    if (DataHub == NULL) return EFI_NOT_AVAILABLE_YET;
    
    //Do not log debug codes
    if (CodeType==EFI_DEBUG_CODE) return EFI_SUCCESS;
    
    if (CodeType > sizeof(DataHubClasses)) return EFI_INVALID_PARAMETER;
    
    if (Data!=NULL)
        Size =  EFI_FIELD_OFFSET(DATA_HUB_STATUS_CODE_DATA_RECORD,Data)
                +Data->HeaderSize+Data->Size;
    else
        Size = sizeof(DATA_HUB_STATUS_CODE_DATA_RECORD);
        
    pBS->AllocatePool(EfiBootServicesData,Size,(VOID**)&Record); 
    //---Fill Record structure--------
    Record->CodeType = Type;
    Record->Value = Value;
    Record->Instance = Instance;
    
    if (CallerId==NULL)  Record->CallerId = gEfiCallerIdGuid;
    else Record->CallerId = *CallerId;
    
    if (Data==NULL)
        pBS->SetMem(&Record->Data,sizeof(EFI_STATUS_CODE_DATA),0);
    else
        pBS->CopyMem(&Record->Data,Data,Data->HeaderSize+Data->Size);
        
    Status = DataHub->LogData(
                 DataHub,
                 &gEfiStatusCodeGuid,
                 &gEfiStatusCodeRuntimeProtocolGuid,
                 DataHubClasses[CodeType-1],
                 Record,
                 Size
             );
    pBS->FreePool(Record);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DataHubInstalled
//
// Description: Callback routine. Activates data hub status code logging once DataHub protocol is installed.
//
// Input:
//      IN EFI_EVENT Event - Signalled Event
//      IN VOID *Context - calling context
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID DataHubInstalled (IN EFI_EVENT Event, IN VOID *Context)
{
    EFI_STATUS Status;
    
    if (DataHub!=NULL) return;
    
    Status = pBS->LocateProtocol(&gEfiDataHubProtocolGuid,NULL,(VOID**)&DataHub); 
    
    if (EFI_ERROR(Status)) DataHub=NULL;
    else pBS->CloseEvent(Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DataHubInit
//
// Description: Initializes data hub error logging infrastructure.
//  This function is called from InitStatusCodeParts function (in DXE only).
//
// Input:
//      IN EFI_HANDLE *ImageHandle
//      IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DataHubInit(
    IN VOID *ImageHandle, IN VOID *SystemTable  
)
{
    EFI_EVENT Event;
    VOID *Registration;
    //Register Data Hub Callback
    EFI_STATUS Status = RegisterProtocolCallback(
                            &gEfiDataHubProtocolGuid, DataHubInstalled, NULL,
                            &Event, &Registration
                        );
                        
    if (EFI_ERROR(Status)) return Status;
    
    pBS->SignalEvent(Event);
    return Status;
}

//<AMI_GHDR_START>
//----------------------------------------------------------------------------
// Name:    Con_Out_Checkpoints
//
// Description:
//  ConOut checkpoints functions:
//   - ConOutCpInit
//   - ConOutInstalled
//   - StopConOutCheckpoints
//   - ConOutCheckpoint
//----------------------------------------------------------------------------
//<AMI_GHDR_END>

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConOutCheckpoint
//
// Description: Displays checkpoints at the right bottom corner or the ConOut devices.
//
// Input:
//  IN VOID *Dummy - not used. Dummy parameter for compatibility with PEI callback interface.
//  IN UINT8 Checkpoint  - checkpoint to display
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ConOutCheckpoint (
    IN VOID *Dummy,
    IN UINT8 Checkpoint
)
{
    EFI_STATUS Status;
    EFI_CONSOLE_CONTROL_SCREEN_MODE Mode;
    CHAR16 s[10];
    
    if (ConsoleControl==NULL || pST->ConOut==NULL
#if CON_OUT_CHECKPOINTS_IN_QUIET_MODE
            || PostManager == NULL
#endif
       ) return;
       
    Status = ConsoleControl->GetMode(ConsoleControl,&Mode,NULL,NULL);
    
    if (EFI_ERROR(Status)) return;
    
    //TODO: try Sprintf and convert
    Swprintf(s, L"%X",Checkpoint);
    
    if (Mode==EfiConsoleControlScreenText)
    {
        UINTN X,Y, MaxX, MaxY;
        pST->ConOut->QueryMode(pST->ConOut,pST->ConOut->Mode->Mode, &MaxX, &MaxY);
        //Get cursor position
        X = pST->ConOut->Mode->CursorColumn;
        Y = pST->ConOut->Mode->CursorRow;
        //Set cursor and print string
        pST->ConOut->SetCursorPosition(pST->ConOut, MaxX-3, MaxY-1);
        pST->ConOut->OutputString(pST->ConOut, s);
        //Restore cursor position
        pST->ConOut->SetCursorPosition(pST->ConOut, X, Y);
    }
    
#if CON_OUT_CHECKPOINTS_IN_QUIET_MODE
    else  // EfiConsoleControlScreenGraphics
    {
        PostManager->DisplayQuietBootMessage(
            s, 0, 0, CA_AttributeRightBottom,
            Foreground, Background
        );
    }
    
#endif
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   StopConOutCheckpoints
//
// Description: Read To Boot Callback routine. Terminates ConOut checkpoint display.
//
// Input:
//      IN EFI_EVENT Event - Signalled Event
//      IN VOID *Context - calling context
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID StopConOutCheckpoints (IN EFI_EVENT Event, IN VOID *Context)
{
    ConsoleControl=NULL;
    pBS->CloseEvent(Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConOutInstalled
//
// Description: Callback routine. Activates ConOut checkpoint display once
//  ConOut field in the UEFI ststem table is populated.
//
// Input:
//      IN EFI_EVENT Event - Signalled Event
//      IN VOID *Context - calling context
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ConOutInstalled (IN EFI_EVENT Event, IN VOID *Context)
{
    EFI_STATUS Status;
    
    if (ConsoleControl!=NULL) return;
    
#if CON_OUT_CHECKPOINTS_IN_QUIET_MODE
    Status = pBS->LocateProtocol(&gAmiPostManagerProtocolGuidLocal,NULL,(VOID**)&PostManager);
    
    if (EFI_ERROR(Status)) PostManager=NULL;
    
#endif
    Status = pBS->LocateProtocol(&gEfiConsoleControlProtocolGuid,NULL,(VOID**)&ConsoleControl);
    
    if (EFI_ERROR(Status)) ConsoleControl=NULL;
    else pBS->CloseEvent(Event);
    
    CreateReadyToBootEvent(TPL_CALLBACK, &StopConOutCheckpoints, NULL, &Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConOutCpInit
//
// Description: Initializes ConOut checkpoint display code.
//  This function is called from InitStatusCodeParts function (in DXE only).
//
// Input:
//      IN EFI_HANDLE *ImageHandle
//      IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ConOutCpInit (
    IN VOID *ImageHandle, IN VOID *SystemTable 
)
{
    EFI_EVENT Event;
    VOID *Registration;
    //Register Data Hub Callback
    EFI_STATUS Status = RegisterProtocolCallback(
                            &gEfiConsoleControlProtocolGuid, ConOutInstalled, NULL,
                            &Event, &Registration
                        );
                        
    if (EFI_ERROR(Status)) return Status;
    
    pBS->SignalEvent(Event);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ResetOrResume
//
// Description: Error Code Action.
//              Attempts to perform a system reset. If reset fails, returns.
//
// Input:
//      VOID *PeiServices - not used
//      EFI_STATUS_CODE_VALUE Value - Value of the error code that triggered the action.
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ResetOrResume(
    IN VOID *PeiServices, IN EFI_STATUS_CODE_VALUE Value
)
{
    pRS->ResetSystem(EfiResetCold,0,0,NULL);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Delay
//
// Description: Stalls execution for a passed in number of microseconds
//
// Input:
//      VOID *PeiServices - not used
//      UINT32 Microseconds - the number of microseconds to stall execution
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID Delay(VOID **PeiServices, UINT32 Microseconds)
{
    pBS->Stall(Microseconds);
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
