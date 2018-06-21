//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision:  $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        SmmIoTrapDispatch.h
//
// Description: The header file for I/O Trap SMM Dispatch Protocol.
//
//<AMI_FHDR_END>
//*************************************************************************

#ifndef __SMM_IO_TRAP_DISPATCH_PROTOCOL_H__
#define __SMM_IO_TRAP_DISPATCH_PROTOCOL_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <EFI.h>

typedef enum {
  ReadIoCycle = 0,
  WriteIoCycle,
  ReadWriteIoCycle
} EFI_SMM_IOTRAP_OP_TYPE;

typedef enum {
  AccessAny = 0,
  AccessByte,
  AccessWord,
  AccessDWord
} EFI_SMM_ACCESS_WIDTH;

typedef struct {
  UINT16                    Address;
  UINT16                    Length;
  EFI_SMM_IOTRAP_OP_TYPE    TrapOpType;
  EFI_SMM_ACCESS_WIDTH      TrapWidth;
  UINT32                    TrapAddress;
  UINTN                     TrapData;
  UINT32                    TrapRegIndex;
} EFI_SMM_IO_TRAP_DISPATCH_CONTEXT;

typedef struct _EFI_SMM_IO_TRAP_DISPATCH_PROTOCOL \
                                            EFI_SMM_IO_TRAP_DISPATCH_PROTOCOL;

//******************************************************
// EFI_SMM_IO_TRAP_DISPATCH
//******************************************************
typedef VOID (EFIAPI *EFI_SMM_IO_TRAP_DISPATCH) (
    IN EFI_HANDLE                           DispatchHandle,
    IN EFI_SMM_IO_TRAP_DISPATCH_CONTEXT     *Context
);


typedef EFI_STATUS (EFIAPI *EFI_SMM_IO_TRAP_REGISTER) (
    IN EFI_SMM_IO_TRAP_DISPATCH_PROTOCOL    *This,
    IN EFI_SMM_IO_TRAP_DISPATCH             DispatchFunction,
    IN EFI_SMM_IO_TRAP_DISPATCH_CONTEXT     *DispatchContext,
    OUT EFI_HANDLE                          *DispatchHandle
);

typedef EFI_STATUS (EFIAPI *EFI_SMM_IO_TRAP_UNREGISTER) (
    IN EFI_SMM_IO_TRAP_DISPATCH_PROTOCOL    *This,
    IN EFI_HANDLE                           DispatchHandle
);


struct _EFI_SMM_IO_TRAP_DISPATCH_PROTOCOL {
    EFI_SMM_IO_TRAP_REGISTER                Register;
    EFI_SMM_IO_TRAP_UNREGISTER              UnRegister;
};


/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
