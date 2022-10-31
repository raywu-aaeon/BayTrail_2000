/*++

   Copyright (c) 2005 Intel Corporation. All rights reserved
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.


   Module Name:

   TpmLib.h

   Abstract:

   Header file for tpm device drivers

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TpmLib.h 3     5/19/10 5:11p Fredericko $
//
// $Revision: 3 $
//
// $Date: 5/19/10 5:11p $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TpmLib.c
//
// Description: 
//  Header file for TpmLib.c
//
//<AMI_FHDR_END>
//*************************************************************************

#ifndef _TPMLIB_H_
#define _TPMLIB_H_

#include "AmiTcg\TcgPc.h"

#define TisCompleteCommand          TisPrepareSendCommand

extern
EFI_STATUS
__stdcall TisRequestLocality (
    IN TPM_1_2_REGISTERS_PTR TpmReg );

extern
EFI_STATUS
__stdcall TisReleaseLocality (
    IN TPM_1_2_REGISTERS_PTR TpmReg );

extern
EFI_STATUS
__stdcall TisPrepareSendCommand (
    IN TPM_1_2_REGISTERS_PTR TpmReg );

extern
EFI_STATUS
__stdcall TisSendCommand (
    IN TPM_1_2_REGISTERS_PTR TpmReg,
    IN const VOID            *CmdStream,
    IN UINTN                 Size,
    IN BOOLEAN               Last );

extern
EFI_STATUS
__stdcall TisWaitForResponse (
    IN TPM_1_2_REGISTERS_PTR TpmReg );

extern
EFI_STATUS
__stdcall TisReceiveResponse (
    IN TPM_1_2_REGISTERS_PTR TpmReg,
    OUT VOID                 *Buffer,
    OUT UINTN                *Size );

extern
EFI_STATUS
__stdcall IsTpmPresent (
    IN TPM_1_2_REGISTERS_PTR TpmReg );

extern
EFI_STATUS
__stdcall TpmLibPassThrough (
    IN TPM_1_2_REGISTERS_PTR   TpmReg,
    IN UINTN                   NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINTN                   NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers );

extern
EFI_STATUS
__stdcall TpmLibStartup (
    IN UINT16 StartupType );

#endif
