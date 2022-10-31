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

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name:        SmmChildDispatch.h
//
// Description: SMM Child dispatcher functions and data structures definition
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>
#ifndef __SMM_CHILD_DISPATCH__H__
#define __SMM_CHILD_DISPATCH__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <AmiDxeLib.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmStandbyButtonDispatch2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmIoTrapDispatch2.h>

#define AMI_SMM_SW_CONTEXT             EFI_SMM_SW_REGISTER_CONTEXT
#define AMI_SMM_SX_CONTEXT             EFI_SMM_SX_REGISTER_CONTEXT
#define AMI_SMM_PERIODIC_TIMER_CONTEXT \
                                      EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT
#define AMI_SMM_USB_CONTEXT            EFI_SMM_USB_REGISTER_CONTEXT
#define AMI_SMM_GPI_CONTEXT            EFI_SMM_GPI_REGISTER_CONTEXT
#define AMI_SMM_STANDBY_BUTTON_CONTEXT \
                                      EFI_SMM_STANDBY_BUTTON_REGISTER_CONTEXT
#define AMI_SMM_POWER_BUTTON_CONTEXT   EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT
#define AMI_SMM_IO_TRAP_CONTEXT        EFI_SMM_IO_TRAP_REGISTER_CONTEXT

#pragma pack(push, 1)
    typedef struct {
        UINT64	Period;
        UINT64	SmiTickInterval;
        UINT64	ElapsedTime;
    } EFI_SMM_PERIODIC_TIMER_DISPATCH_CONTEXT;

    typedef enum {
        EfiSmmSwSmi,
        EfiSmmSxSmi,
        EfiSmmPeriodicTimerSmi,
        EfiSmmUsbSmi,
        EfiSmmGpiSmi,
        EfiSmmStandbyButtonSmi,
        EfiSmmPowerButtonSmi,
        EfiSmmIoTrapSmi,
        EfiSmmMaxSmi
    } EFI_SMM_SMI;

    typedef union {
        AMI_SMM_SW_CONTEXT               SwContext;
        AMI_SMM_SX_CONTEXT               SxContext;
        AMI_SMM_PERIODIC_TIMER_CONTEXT   TimerContext;
        AMI_SMM_USB_CONTEXT              UsbContext;
        AMI_SMM_GPI_CONTEXT              GpiContext;
        AMI_SMM_STANDBY_BUTTON_CONTEXT   SBtnContext;
        AMI_SMM_POWER_BUTTON_CONTEXT     PBtnContext;
        AMI_SMM_IO_TRAP_CONTEXT          IoTrapContext;
    } EFI_SMM_SMI_CONTEXT;

    typedef EFI_STATUS(*SMI_GENERIC_CALLBACK)(
        IN EFI_HANDLE           DispatchHandle,
        IN CONST VOID           *DispatchContext OPTIONAL,
        IN OUT VOID             *CommBuffer OPTIONAL,
        IN OUT UINTN            *CommBufferSize OPTIONAL
    );

    typedef struct {
        DLINK                   Link;
        UINT32                  Signature;
        SMI_GENERIC_CALLBACK    Callback;
        UINT8                   Context[1];
    } HANDLER_LINK;

//------------------------------------------------------------------------
//              SMI Handler protocol functions prototypes
//------------------------------------------------------------------------

    typedef EFI_STATUS(*ADD_HANDLER)(
        IN VOID *Context
    );

    typedef EFI_STATUS(*REMOVE_HANDLER)(
        IN VOID *Context
    );

    typedef EFI_STATUS(*VERIFY_CONTEXT)(
        IN VOID *Context
    );

    typedef BOOLEAN(*GET_CONTEXT)(
        VOID
    );

    typedef VOID (*DISPATCH_SMI)(
        VOID
    );

    typedef struct {
        ADD_HANDLER    AddHandler;
        REMOVE_HANDLER RemoveHandler;
        VERIFY_CONTEXT VerifyContext;
        GET_CONTEXT    GetContext;
        DISPATCH_SMI   DispatchSmi;
    } SMI_HANDLER_PROTOCOL;

    typedef struct {
        EFI_SMM_SMI          SmiType;
        SMI_HANDLER_PROTOCOL HandlerProtocol;
        DLIST                RegisteredCallbacks;
    } SMM_CHILD_DISPATCHER;

#pragma pack(pop)

//-----------------------------------------------------------------------
//              SW SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmSwAddHandler(IN VOID  *Context);
    EFI_STATUS SmmSwRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmSwVerifyContext(IN VOID  *Context);
    BOOLEAN SmmSwGetContext(VOID);
    VOID SmmSwDispatchSmi(VOID);

//*************** SW SMI Handler Porting hooks ***************************
    VOID SwSmiEnable(VOID);
    VOID SwSmiDisable(VOID);
    VOID SwSmiClear(VOID);
    BOOLEAN SwSmiDetect(UINT16 *Type);
    UINTN GetEAX(VOID);

//-----------------------------------------------------------------------
//              SX SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmSxAddHandler(IN VOID  *Context);
    EFI_STATUS SmmSxRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmSxVerifyContext(IN VOID  *Context);
    BOOLEAN SmmSxGetContext(VOID);
    VOID SmmSxDispatchSmi(VOID);

//*************** SX SMI Handler Porting hooks ***************************
    VOID SxSmiEnable(VOID);
    VOID SxSmiDisable(VOID);
    VOID SxSmiClear(VOID);
    BOOLEAN SxSmiDetect(UINT16 *Type);
    VOID PutToSleep(IN VOID *Context);

//-----------------------------------------------------------------------
//              Periodic timer SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmTimerAddHandler(IN VOID  *Context);
    EFI_STATUS SmmTimerRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmTimerVerifyContext(IN VOID  *Context);
    BOOLEAN SmmTimerGetContext(VOID);
    VOID SmmTimerDispatchSmi(VOID);

//*************** Periodic timer SMI Handler Porting hooks ***************************
    VOID TimerSmiEnable(VOID);
    VOID TimerSmiDisable(VOID);
    VOID TimerSmiClear(VOID);
    BOOLEAN TimerSmiDetect(UINT16 *Type);
    VOID TimerSetInterval(UINT64 Interval);

//-----------------------------------------------------------------------
//              USB SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmUsbAddHandler(IN VOID  *Context);
    EFI_STATUS SmmUsbRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmUsbVerifyContext(IN VOID  *Context);
    BOOLEAN SmmUsbGetContext(VOID);
    VOID SmmUsbDispatchSmi(VOID);

//*************** USB SMI Handler Porting hooks ***************************
    VOID UsbSmiSet(UINT16 ControllerType);
    VOID UsbSmiClear(UINT16 ControllerType);
    BOOLEAN UsbSmiDetect(UINT16 *Type);
    UINT16 GetControllerType(EFI_DEVICE_PATH_PROTOCOL *Device);

//-----------------------------------------------------------------------
//              GPI SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmGpiAddHandler(IN VOID  *Context);
    EFI_STATUS SmmGpiRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmGpiVerifyContext(IN VOID  *Context);
    BOOLEAN SmmGpiGetContext(VOID);
    VOID SmmGpiDispatchSmi(VOID);

//*************** GPI SMI Handler Porting hooks ***************************
    VOID GpiSmiSet(UINT16 Type);
    VOID GpiSmiClear(UINT16 Type);
    BOOLEAN GpiSmiDetect(UINT16 *Type);

//-----------------------------------------------------------------------
//              Standby button SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmSButtonAddHandler(IN VOID  *Context);
    EFI_STATUS SmmSButtonRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmSButtonVerifyContext(IN VOID  *Context);
    BOOLEAN SmmSButtonGetContext(VOID);
    VOID SmmSButtonDispatchSmi(VOID);

//*************** Standby button SMI Handler Porting hooks *****************
    VOID SButtonSmiEnable(VOID);
    VOID SButtonSmiDisable(VOID);
    VOID SButtonSmiClear(VOID);
    BOOLEAN SButtonSmiDetect(UINT16 *Type);

//-----------------------------------------------------------------------
//              Power button SMI Handler functions
//-----------------------------------------------------------------------

    EFI_STATUS SmmPButtonAddHandler(IN VOID  *Context);
    EFI_STATUS SmmPButtonRemoveHandler(IN VOID  *Context);
    EFI_STATUS SmmPButtonVerifyContext(IN VOID  *Context);
    BOOLEAN SmmPButtonGetContext(VOID);
    VOID SmmPButtonDispatchSmi(VOID);

//*************** Power button SMI Handler Porting hooks *****************
    VOID PButtonSmiEnable(VOID);
    VOID PButtonSmiDisable(VOID);
    VOID PButtonSmiClear(VOID);
    BOOLEAN PButtonSmiDetect(UINT16 *Type);

//---------------------------------------------------------------------------
//                     I/O Trap SMI Handler functions
//---------------------------------------------------------------------------

    EFI_STATUS  SmmIoTrapAddHandler(IN VOID *Context);
    EFI_STATUS  SmmIoTrapRemoveHandler(IN VOID *Context);
    EFI_STATUS  SmmIoTrapVerifyContext(IN VOID *Context);
    BOOLEAN     SmmIoTrapGetContext(VOID);
    VOID        SmmIoTrapDispatchSmi(VOID);

//------------------ I/O Trap SMI Handler Porting hooks ---------------------

    VOID        IoTrapSmiSet(IN EFI_SMM_IO_TRAP_REGISTER_CONTEXT *Context);
    VOID        IoTrapSmiReset(IN EFI_SMM_IO_TRAP_REGISTER_CONTEXT *Context);
    VOID        IoTrapSmiEnable(VOID);
    VOID        IoTrapSmiDisable(VOID);
    VOID        IoTrapSmiClear(VOID);
    BOOLEAN     IoTrapSmiDetect(OUT EFI_SMM_IO_TRAP_REGISTER_CONTEXT *Context);

//*************** All purpose SMI Porting hooks *************************
    VOID ClearAllSmi(VOID);
    VOID DisableAllSmi(VOID);


    /****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif
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
