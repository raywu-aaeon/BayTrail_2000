/*++

   Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.


   Module Name:

   sha1.c

   Abstract:

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/sha1.c 9     3/29/11 12:14p Fredericko $
//
// $Revision: 9 $
//
// $Date: 3/29/11 12:14p $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  Sha1.c
//
// Description: 
//  Contians Sha1 functions
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <AmiLib.h>
#include "AmiTcg\Sha1.h"
#include "AmiTcg\TcgCommon.h"

static CONST TCG_DIGEST mSha1DigestInit = {
    {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98,
        0x76, 0x54, 0x32, 0x10,
        0xf0, 0xe1, 0xd2, 0xc3
    }
};



extern
VOID
__stdcall SHA1_transform (
    IN OUT UINT8 *Digest,
    IN UINT8     *M );



EFI_STATUS
__stdcall SHA1_init(
    IN VOID     *CallbackContext,
    IN SHA1_CTX *Sha1Ctx )
{
    Sha1Ctx->Status = EFI_SUCCESS;
    Sha1Ctx->Length = 0;
    Sha1Ctx->Digest = mSha1DigestInit;
    Sha1Ctx->Count  = 0;
    return EFI_SUCCESS;
}

EFI_STATUS
__stdcall SHA1_update(
    IN VOID     *CallbackContext,
    IN SHA1_CTX *Sha1Ctx,
    IN VOID     *Data,
    IN UINTN    DataLen )
{
    UINTN l;
    UINT8 *D;

    Sha1Ctx->Length += DataLen;
    D = (UINT8*)Data;

    if ( Sha1Ctx->Count > 0 )
    {
        l = 64 - Sha1Ctx->Count;

        if ( l <= DataLen )
        {
            TcgCommonCopyMem(
                CallbackContext,
                Sha1Ctx->M + Sha1Ctx->Count,
                D,
                l
                );
            SHA1_transform( Sha1Ctx->Digest.digest, Sha1Ctx->M );
            D       += l;
            DataLen -= l;
        }
        else {
            TcgCommonCopyMem(
                CallbackContext,
                Sha1Ctx->M + Sha1Ctx->Count,
                D,
                DataLen
                );
            Sha1Ctx->Count += DataLen;
            return EFI_SUCCESS;
        }
    }

    Sha1Ctx->Count = DataLen & 63;

    if ( Sha1Ctx->Count > 0 )
    {
        DataLen -= Sha1Ctx->Count;
            TcgCommonCopyMem(
            CallbackContext,
            Sha1Ctx->M,
            D + DataLen,
            Sha1Ctx->Count
            );
    }

    if ((DataLen >>= 6) == 0 )
    {
        return EFI_SUCCESS;
    }

    do
    {
        SHA1_transform( Sha1Ctx->Digest.digest, D );
        D += 64;
    } while ( --DataLen > 0 );
    return EFI_SUCCESS;
}

EFI_STATUS
__stdcall SHA1_final(
    IN VOID        *CallbackContext,
    IN SHA1_CTX    *Sha1Ctx,
    OUT TCG_DIGEST **HashVal
    )
{
    UINTN PadLen, i;

    union
    {
        UINT32 Dwords[2];
        UINT64 Qword;
    }                                 LenInBits;
    UINT8 PadByte;

    if ( Sha1Ctx->Status == EFI_ALREADY_STARTED )
    {
        *HashVal = &Sha1Ctx->Digest;
        return EFI_SUCCESS;
    }

    if ( EFI_ERROR( Sha1Ctx->Status ))
    {
        return Sha1Ctx->Status;
    }

    PadLen  = sizeof (Sha1Ctx->M) - sizeof (Sha1Ctx->Length);
    PadLen -= Sha1Ctx->Count;

    if ((INTN)PadLen <= 0 )
    {
        PadLen += sizeof (Sha1Ctx->M);
    }

    LenInBits.Qword     = Shl64( Sha1Ctx->Length, 3 );
    LenInBits.Dwords[0] = TcgCommonH2NL( LenInBits.Dwords[0] );
    LenInBits.Dwords[1] = TcgCommonH2NL( LenInBits.Dwords[1] );

    PadByte = 0x80;
   
    SHA1_update( CallbackContext,
                 Sha1Ctx,
                 &PadByte,
                 sizeof (PadByte));
    PadByte = 0;

    for ( PadLen--; PadLen > 0; PadLen-- )
    {
        SHA1_update( CallbackContext,
                     Sha1Ctx,
                     &PadByte,
                     sizeof (PadByte));
    }

    SHA1_update(CallbackContext,
        Sha1Ctx,
        &LenInBits.Dwords[1],
        sizeof (*LenInBits.Dwords));

    SHA1_update(CallbackContext,
        Sha1Ctx,
        &LenInBits.Dwords[0],
        sizeof (*LenInBits.Dwords));

    i = sizeof (Sha1Ctx->Digest) / sizeof (UINT32);

    while ( i > 0 )
    {
        i--;
        ((UINT32*)Sha1Ctx->Digest.digest)[i] = 
        TcgCommonH2NL(((UINT32*)Sha1Ctx->Digest.digest)[i]);
    }

    Sha1Ctx->Status = EFI_ALREADY_STARTED;
    Sha1Ctx->Length = 0;
    i               = sizeof (Sha1Ctx->M);

    while ( i > 0 )
    {
        Sha1Ctx->M[--i] = 0;
    }

    return SHA1_final( CallbackContext, Sha1Ctx, HashVal );
}
