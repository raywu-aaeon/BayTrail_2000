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
// Name:        SmmChildDispatchProtocol.h
//
// Description: SMM Child dispatcher protocols functions definition
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>
#ifndef __SMM_CHILD_DISPATCH_PROTOCOL__H__
#define __SMM_CHILD_DISPATCH_PROTOCOL__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmStandbyButtonDispatch2.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <Protocol/SmmIoTrapDispatch2.h>

//************* Generic register/unregister handler functions ********************

    EFI_STATUS RegisterHandler(
        IN  EFI_SMM_SMI           Type,
        IN  SMI_GENERIC_CALLBACK  Function,
        IN  VOID                  *Context,
        IN  UINTN                 ContextSize,
        OUT EFI_HANDLE            *Handle
    );

    EFI_STATUS UnregisterHandler(
        IN EFI_SMM_SMI  Type,
        IN EFI_HANDLE   Handle
    );

//******************** Sw SMI protocol functions *********************************

    EFI_STATUS EfiSmmSwRegister(
        IN CONST EFI_SMM_SW_DISPATCH2_PROTOCOL      *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2       Function,
        IN CONST EFI_SMM_SW_REGISTER_CONTEXT        *Context,
        OUT      EFI_HANDLE                         *Handle
    );

    EFI_STATUS EfiSmmSwUnregister(
        IN CONST EFI_SMM_SW_DISPATCH2_PROTOCOL      *This,
        IN       EFI_HANDLE                         Handle
    );

//******************** Sx SMI protocol functions *********************************

    EFI_STATUS EfiSmmSxRegister(
        IN CONST EFI_SMM_SX_DISPATCH2_PROTOCOL      *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2       Function,
        IN CONST EFI_SMM_SX_REGISTER_CONTEXT        *Context,
        OUT      EFI_HANDLE                         *Handle
    );

    EFI_STATUS EfiSmmSxUnregister(
        IN CONST EFI_SMM_SX_DISPATCH2_PROTOCOL      *This,
        IN EFI_HANDLE                               Handle
    );

//******************** Periodic timer SMI protocol functions *********************

    EFI_STATUS EfiSmmTimerRegister(
        IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2              Function,
        IN CONST EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT   *Context,
        OUT      EFI_HANDLE                                *Handle
    );

    EFI_STATUS EfiSmmTimerUnregister(
        IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL *This,
        IN       EFI_HANDLE                                Handle
    );

    EFI_STATUS EfiSmmTimerGetNextShorterInterval(
        IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL *This,
        IN OUT   UINT64                                    **SmiTickInterval
    );

//******************** Usb SMI protocol functions ********************************

    EFI_STATUS EfiSmmUsbRegister(
        IN CONST EFI_SMM_USB_DISPATCH2_PROTOCOL     *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2       Function,
        IN CONST EFI_SMM_USB_REGISTER_CONTEXT       *Context,
        OUT      EFI_HANDLE                         *Handle
    );

    EFI_STATUS EfiSmmUsbUnregister(
        IN CONST EFI_SMM_USB_DISPATCH2_PROTOCOL     *This,
        IN       EFI_HANDLE                         Handle
    );

//******************** Gpi SMI protocol functions ********************************

    EFI_STATUS EfiSmmGpiRegister(
        IN CONST EFI_SMM_GPI_DISPATCH2_PROTOCOL     *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2       Function,
        IN CONST EFI_SMM_GPI_REGISTER_CONTEXT       *Context,
        OUT      EFI_HANDLE                         *Handle
    );

    EFI_STATUS EfiSmmGpiUnregister(
        IN CONST EFI_SMM_GPI_DISPATCH2_PROTOCOL     *This,
        IN EFI_HANDLE                               Handle
    );

//******************** Standby button SMI protocol functions *********************

    EFI_STATUS EfiSmmSButtonRegister(
        IN CONST EFI_SMM_STANDBY_BUTTON_DISPATCH2_PROTOCOL *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2              Function,
        IN CONST EFI_SMM_STANDBY_BUTTON_REGISTER_CONTEXT   *Context,
        OUT      EFI_HANDLE                                *Handle
    );

    EFI_STATUS EfiSmmSButtonUnregister(
        IN CONST EFI_SMM_STANDBY_BUTTON_DISPATCH2_PROTOCOL *This,
        IN       EFI_HANDLE                                Handle
    );

//******************** Power button SMI protocol functions *********************

    EFI_STATUS EfiSmmPButtonRegister(
        IN CONST EFI_SMM_POWER_BUTTON_DISPATCH2_PROTOCOL *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2            Function,
        IN CONST EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT   *Context,
        OUT      EFI_HANDLE                              *Handle
    );

    EFI_STATUS EfiSmmPButtonUnregister(
        IN CONST EFI_SMM_POWER_BUTTON_DISPATCH2_PROTOCOL *This,
        IN       EFI_HANDLE                              Handle
    );

//--------------------- I/O Trap SMI protocol functions ---------------------

    EFI_STATUS EfiSmmIoTrapRegister(
        IN CONST EFI_SMM_IO_TRAP_DISPATCH2_PROTOCOL *This,
        IN       EFI_SMM_HANDLER_ENTRY_POINT2       Function,
        IN OUT   EFI_SMM_IO_TRAP_REGISTER_CONTEXT   *Context,
        OUT      EFI_HANDLE                         *Handle
    );

    EFI_STATUS EfiSmmIoTrapUnregister(
        IN CONST EFI_SMM_IO_TRAP_DISPATCH2_PROTOCOL *This,
        IN       EFI_HANDLE                         Handle
    );

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
