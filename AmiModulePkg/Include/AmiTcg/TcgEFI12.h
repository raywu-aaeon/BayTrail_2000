/*++

   Copyright (c)  2005 Intel Corporation. All rights reserved
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.


   Module Name:

   TpmEfi.h

   Abstract:

   This file contains definitions of structures for TPM and TCG
   protocols/ppis.

   --*/

//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgEFI12.h 2     5/19/10 5:22p Fredericko $
//
// $Revision: 2 $
//
// $Date: 5/19/10 5:22p $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TcgEFI12.h
//
// Description: 
//  Contains Industry Standard structures for TCG
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _TCGEFI_H_
#define _TCGEFI_H_

#include "AmiTcg\TcgTpm12.h"
#include <IndustryStandard/Tpm12.h>
#include <IndustryStandard/UefiTcgPlatform.h>
#include "AmiTcg\TcgPc.h"

#define _TPM_NTH_BIT( x )             (1 << (x))
#define _TPM_STRUCT_PARTIAL_SIZE( type, field )  \
    ((UINTN)&(((type*)0)->field))

#pragma pack (push)
#pragma pack (1)

//
// TCG_PCClientEventStructure
//
#pragma pack (pop)

//
// Per the EFI Implementation
//
typedef TPM_DIGEST TCG_DIGEST;

//
// TPM_TRANSMIT_BUFFER used by TCG services
//
typedef struct _TPM_TRANSMIT_BUFFER
{
    VOID  *Buffer;
    UINTN Size;
} TPM_TRANSMIT_BUFFER;

//
// Definitons of TPM commands
//

#pragma pack (push)
#pragma pack (1)

typedef struct _TPM_1_2_CMD_HEADER
{
    TPM_TAG          Tag;
    UINT32           ParamSize;
    TPM_COMMAND_CODE Ordinal;
} TPM_1_2_CMD_HEADER;

typedef struct _TPM_1_2_RET_HEADER
{
    TPM_TAG    Tag;
    UINT32     ParamSize;
    TPM_RESULT RetCode;
} TPM_1_2_RET_HEADER;

//
// TPM_Startup
//

typedef struct _TPM_1_2_CMD_STARTUP
{
    TPM_1_2_CMD_HEADER Header;
    TPM_STARTUP_TYPE   StartupType;
} TPM_1_2_CMD_STARTUP;

//
// TPM_SHA1Start
//

typedef struct _TPM_1_2_RET_SHA1START
{
    TPM_1_2_RET_HEADER Header;
    UINT32             MaxBytes;
} TPM_1_2_RET_SHA1START;

//
// TPM_SHA1Update
//

typedef struct _TPM_1_2_CMD_SHA1UPDATE
{
    TPM_1_2_CMD_HEADER Header;
    UINT32             NumBytes;
} TPM_1_2_CMD_SHA1UPDATE;

//
// TPM_SHA1Complete
//

typedef TPM_1_2_CMD_SHA1UPDATE TPM_1_2_CMD_SHA1COMPLETE;

typedef struct _TPM_1_2_RET_SHA1COMPLETE
{
    TPM_1_2_CMD_HEADER Header;
    TCG_DIGEST         Digest;
} TPM_1_2_RET_SHA1COMPLETE;

//
// TPM_SHA1CompleteExtend
//

typedef struct _TPM_1_2_CMD_SHA1COMPLETEEXTEND
{
    TPM_1_2_CMD_HEADER Header;
    TPM_PCRINDEX       PCRIndex;
    UINT32             NumBytes;
} TPM_1_2_CMD_SHA1COMPLETEEXTEND;

#pragma pack (pop)

#endif  // _TCGEFI_H_
