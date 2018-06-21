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

   TcgCommon.h

   Abstract:

   Header file for TcgCommon.c

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgCommon.h 13    3/19/12 6:14p Fredericko $
//
// $Revision: 13 $
//
// $Date: 3/19/12 6:14p $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TcgCommon.h
//
// Description: 
//  Header file for TcgCommon.c
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _TCG_COMMON_H_
#define _TCG_COMMON_H_

#include <UEFI.h>
#include "AmiTcg\TcgPc.h"
#include "AmiTcg\TcgEfiTpm.h"
#include "AmiTcg\Sha.h"
#include "AmiTcg\Tpm20.h"
#include <HOB.h>
#include "TcgMisc.h"

#define TcgCommonN2HS( v16 ) TcgCommonH2NS( v16 )
#define TcgCommonN2HL( v32 ) TcgCommonH2NL( v32 )
#define EFI_TPL_DRIVER                 6
#define EFI_TPL_APPLICATION         4
#define EFI_TPL_CALLBACK            8
#define EFI_TPL_NOTIFY              16
#define EFI_TPL_HIGH_LEVEL          31
#define STRING_TOKEN( x ) x


#define GET_HOB_TYPE( Hob )     ((Hob).Header->HobType)
#define GET_HOB_LENGTH( Hob )   ((Hob).Header->HobLength)
#define GET_NEXT_HOB( Hob )     ((Hob).Raw + GET_HOB_LENGTH( Hob ))
#define END_OF_HOB_LIST( Hob )  (GET_HOB_TYPE( Hob ) ==\
                                 EFI_HOB_TYPE_END_OF_HOB_LIST)

#define TCGPASSTHROUGH( cb, in, out ) \
    TcgCommonPassThrough(  \
        cb, \
        sizeof (in) / sizeof (*(in)), \
        (in), \
        sizeof (out) / sizeof (*(out)), \
        (out) \
        )

#define TCMPASSTHROUGH( cb, in, out ) \
    TcmCommonPassThrough(  \
        cb, \
        sizeof (in) / sizeof (*(in)), \
        (in), \
        sizeof (out) / sizeof (*(out)), \
        (out) \
        )

extern
UINT16
__stdcall TcgCommonH2NS (
    IN UINT16 Val );

extern
UINT32
__stdcall TcgCommonH2NL (
    IN UINT32 Val );

VOID* GetHob (
    IN UINT16 Type,
    IN VOID   *HobStart  );

EFI_STATUS GetNextGuidHob (
    IN OUT VOID          **HobStart,
    IN EFI_GUID          * Guid,
    OUT VOID             **Buffer,
    OUT UINTN*BufferSize OPTIONAL );

EFI_STATUS TcgGetNextGuidHob(
    IN OUT VOID          **HobStart,
    IN EFI_GUID          * Guid,
    OUT VOID             **Buffer,
    OUT UINTN            *BufferSize OPTIONAL );

extern
EFI_STATUS
__stdcall TcgCommonPassThrough (
    IN VOID                    *CallbackContext,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers );

extern
EFI_STATUS
__stdcall TcmCommonPassThrough (
    IN VOID                    *CallbackContext,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers );

extern
VOID
__stdcall TcgCommonCopyMem (
    IN VOID  *CallbackContext,
    OUT VOID *Dest,
    IN VOID  *Src,
    IN UINTN Len );

EFI_STATUS
__stdcall TcgCommonLogEvent(
    IN VOID          *CallbackContext,
    IN TCG_PCR_EVENT *EvtLog,
    IN OUT UINT32    *TableSize,
    IN UINT32        MaxSize,
    IN TCG_PCR_EVENT *NewEntry, 
    IN UINT8         HashAlgorithm );

extern
EFI_STATUS
__stdcall TcmCommonLogEvent (
    IN VOID          *CallbackContext,
    IN TCM_PCR_EVENT *EvtLog,
    IN OUT UINT32    *TableSize,
    IN UINT32        MaxSize,
    IN TCM_PCR_EVENT *NewEntry );

extern
EFI_STATUS
__stdcall TcgCommonSha1Start (
    IN VOID             *CallbackContext,
    IN TCG_ALGORITHM_ID AlgId,
    OUT UINT32          *MaxBytes );

extern
EFI_STATUS
__stdcall TcgCommonSha1Update (
    IN VOID   *CallbackContext,
    IN VOID   *Data,
    IN UINT32 DataLen,
    IN UINT32 MaxBytes );

extern
EFI_STATUS
__stdcall TcgCommonSha1CompleteExtend (
    IN VOID         *CallbackContext,
    IN VOID         *Data,
    IN UINT32       DataLen,
    IN TPM_PCRINDEX PCRIndex,
    OUT TCG_DIGEST  *Digest,
    OUT TCG_DIGEST  *NewPCRValue );


EFI_STATUS
__stdcall TcmCommonSha1CompleteExtend(
    IN VOID         *CallbackContext,
    IN VOID         *Data,
    IN UINT32       DataLen,
    IN TPM_PCRINDEX PCRIndex,
    OUT TCM_DIGEST  *Digest,
    OUT TCM_DIGEST  *NewPCRValue );

extern
EFI_STATUS
__stdcall TcgCommonExtend (
    IN VOID         *CallbackContext,
    IN TPM_PCRINDEX PCRIndex,
    IN TCG_DIGEST   *Digest,
    OUT TCG_DIGEST  *NewPCRValue );

EFI_STATUS
__stdcall TcmCommonExtend(
    IN  VOID         *CallbackContext,
    IN  TPM_PCRINDEX PCRIndex,
    IN  TCM_DIGEST   *Digest,
    OUT TCM_DIGEST  *NewPCRValue );

extern
EFI_STATUS
__stdcall SHA1HashAll (
    IN VOID            *CallbackContext,
    IN VOID            *HashData,
    IN UINTN           HashDataLen,
    OUT UINT8          *Digest
    );

EFI_STATUS EfiLibGetSystemConfigurationTable(
    IN EFI_GUID *TableGuid,
    IN OUT VOID **Table );

extern
BOOLEAN
__stdcall AutoSupportType (
    );

#pragma pack(push,1)

typedef union {
  UINT8 sha1[SHA1_DIGEST_SIZE];
  UINT8 sha256[SHA256_DIGEST_SIZE];
} TPM_COMM_DIGEST_UNION;

typedef struct{
  UINT16                  HashAlgId;
  TPM_COMM_DIGEST_UNION   Digest;
}TPM_COMM_DIGEST;

#define   HASH_ALG_COUNT         2 

typedef struct {
  UINT32            Count;
  TPM_COMM_DIGEST   Digests[HASH_ALG_COUNT];
}TPM_COMM_DIGEST_LIST;

typedef struct {
    TPMI_DH_PCR                pcrHandle;
    TPM_COMM_DIGEST_LIST       DigestValue;
} PCR_Extend_In;

typedef struct {
    TPMI_ST_COMMAND_TAG       Tag;
    UINT32                    CommandSize;
    TPM_CC                    CommandCode;
    PCR_Extend_In             inputParameters;
    UINT32                    authorizationSize;
    TPMS_AUTH_SESSION_COMMAND pwapAuth;
} TPM2_PCRExtend_cmd_t;

typedef struct {
    TPMI_ST_COMMAND_TAG         Tag;
    UINT32                      RespondSize;
    TPM_RC                      ResponseCode;
    UINT32                      parameterSize;
    TPMS_AUTH_SESSION_RESPONSE  pwapAuth;
} TPM2_PCRExtend_res_t;

EFI_STATUS
__stdcall SHA2HashAll(
    IN  VOID            *CallbackContext,
    IN  VOID            *HashData,
    IN  UINTN           HashDataLen,
    OUT UINT8           *Digest
);


#pragma pack(pop)


#endif
