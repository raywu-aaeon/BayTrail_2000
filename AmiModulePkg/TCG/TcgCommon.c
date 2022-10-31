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

   TcgCommon.c

   Abstract:

   TCG Commands implemented for both PEI and DXE

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgCommon.c 9     3/19/12 6:13p Fredericko $
//
// $Revision: 9 $
//
// $Date: 3/19/12 6:13p $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TcgCommon.c
//
// Description: 
//  common TCG functions can be found here
//
//<AMI_FHDR_END>
//*************************************************************************
#include "AmiTcg\TcgCommon.h"
#include <AmiDxeLib.h>
#include "token.h"
#include "AmiTcg\Sha1.h"

#define TCGPASSTHROUGH( cb, in, out ) \
    TcgCommonPassThrough(  \
        cb, \
        sizeof (in) / sizeof (*(in)), \
        (in), \
        sizeof (out) / sizeof (*(out)), \
        (out) \
        )


UINT16
__stdcall TcgCommonH2NS(
    IN UINT16 Val )
{
    return TPM_H2NS( Val );
}

UINT32
__stdcall TcgCommonH2NL(
    IN UINT32 Val )
{
    return TPM_H2NL( Val );
}



VOID
__stdcall TcgCommonCopyMem(
    IN VOID  *CallbackContext,
    OUT VOID *Dest,
    IN VOID  *Src,
    IN UINTN Size )
{
    CHAR8 *Destination8;
    CHAR8 *Source8;

    if ( Src < Dest )
    {
        Destination8 = (CHAR8*) Dest + Size - 1;
        Source8      = (CHAR8*) Src + Size - 1;
        while ( Size-- )
        {
            *(Destination8--) = *(Source8--);
        }
    }
    else {
        Destination8 = (CHAR8*) Dest;
        Source8      = (CHAR8*) Src;
        while ( Size-- )
        {
            *(Destination8++) = *(Source8++);
        }
    }
}



EFI_STATUS
__stdcall TcgCommonLogEvent(
    IN VOID          *CallbackContext,
    IN TCG_PCR_EVENT *EvtLog,
    IN OUT UINT32    *TableSize,
    IN UINT32        MaxSize,
    IN TCG_PCR_EVENT *NewEntry )
{
    UINT32   EvtSize;

    EvtSize = NewEntry->EventSize + sizeof (*NewEntry) - 1;

    if ( *TableSize + EvtSize > MaxSize )
    {
        return EFI_OUT_OF_RESOURCES;
    }

    EvtLog = (TCG_PCR_EVENT*)((UINT8*)EvtLog + *TableSize);
    TcgCommonCopyMem( CallbackContext, EvtLog, NewEntry, EvtSize );

    *TableSize += EvtSize;
    return EFI_SUCCESS;
}



EFI_STATUS
__stdcall TcmCommonLogEvent(
    IN VOID          *CallbackContext,
    IN TCM_PCR_EVENT *EvtLog,
    IN OUT UINT32    *TableSize,
    IN UINT32        MaxSize,
    IN TCM_PCR_EVENT *NewEntry )
{
    UINT32   EvtSize;

    EvtSize = NewEntry->EventSize + sizeof (*NewEntry) - 1;

    if ( *TableSize + EvtSize > MaxSize )
    {
        return EFI_OUT_OF_RESOURCES;
    }

    EvtLog = (TCM_PCR_EVENT*)((UINT8*)EvtLog + *TableSize);
    TcgCommonCopyMem( CallbackContext, EvtLog, NewEntry, EvtSize );

    *TableSize += EvtSize;
    return EFI_SUCCESS;
}



EFI_STATUS
__stdcall TcgCommonSha1Start(
    IN VOID             *CallbackContext,
    IN TCG_ALGORITHM_ID AlgId,
    OUT UINT32          *MaxBytes )
{
    EFI_STATUS            Status;
    TPM_1_2_CMD_HEADER    cmdSHA1Start = {
        TPM_H2NS( TPM_TAG_RQU_COMMAND ),
        TPM_H2NL( sizeof (TPM_1_2_CMD_HEADER)),
        TPM_H2NL( TPM_ORD_SHA1Start )
    };
    TPM_1_2_RET_SHA1START retSHA1Start;
    TPM_TRANSMIT_BUFFER   InBuffer[1], OutBuffer[1];

    if ( AlgId != TCG_ALG_SHA )
    {
        return EFI_UNSUPPORTED;
    }

    if(AutoSupportType()){
        cmdSHA1Start.Ordinal = TPM_H2NL(TCM_ORD_SHA1Start);
    }


    InBuffer[0].Buffer  = &cmdSHA1Start;
    InBuffer[0].Size    = sizeof (cmdSHA1Start);
    OutBuffer[0].Buffer = &retSHA1Start;
    OutBuffer[0].Size   = sizeof (retSHA1Start);
    Status              = TCGPASSTHROUGH( CallbackContext, InBuffer, OutBuffer );

    if ( EFI_ERROR( Status ) || retSHA1Start.Header.RetCode != 0 )
    {
        return Status;
    }

    if ( MaxBytes != NULL )
    {
        *MaxBytes = TcgCommonN2HL( retSHA1Start.MaxBytes );
    }
    return EFI_SUCCESS;
}



EFI_STATUS
__stdcall TcgCommonSha1Update(
    IN VOID   *CallbackContext,
    IN VOID   *Data,
    IN UINT32 DataLen,
    IN UINT32 MaxBytes )
{
    EFI_STATUS             Status;
    TPM_1_2_CMD_SHA1UPDATE cmdSHA1Update;
    TPM_1_2_RET_HEADER     retSHA1Update;
    TPM_TRANSMIT_BUFFER    InBuffer[2], OutBuffer[1];
    UINT8                  *DataPtr;

    if ( DataLen < 64 )
    {
        return EFI_SUCCESS;
    }

    cmdSHA1Update.Header.Tag     = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdSHA1Update.Header.Ordinal = TPM_H2NL( TPM_ORD_SHA1Update );

    if(AutoSupportType()){
         cmdSHA1Update.Header.Ordinal = TPM_H2NL(TCM_ORD_SHA1Update);
    }

    InBuffer[0].Buffer           = &cmdSHA1Update;
    InBuffer[0].Size             = sizeof (cmdSHA1Update);
    OutBuffer[0].Buffer          = &retSHA1Update;
    OutBuffer[0].Size            = sizeof (retSHA1Update);

    DataPtr = (UINT8*)Data;

    do
    {
        InBuffer[1].Buffer = DataPtr;
        InBuffer[1].Size   = DataLen < MaxBytes ? DataLen : MaxBytes;

        cmdSHA1Update.NumBytes         = TcgCommonH2NL((UINT32)InBuffer[1].Size );
        cmdSHA1Update.Header.ParamSize = TcgCommonH2NL(
            (UINT32)InBuffer[1].Size + sizeof (cmdSHA1Update)
            );

        DataPtr += InBuffer[1].Size;
        DataLen -= (UINT32)InBuffer[1].Size;

        Status = TCGPASSTHROUGH( CallbackContext, InBuffer, OutBuffer );
    } while ( !EFI_ERROR( Status ) && DataLen >= 64 );

    return Status;
}



EFI_STATUS
__stdcall TcgCommonSha1CompleteExtend(
    IN VOID         *CallbackContext,
    IN VOID         *Data,
    IN UINT32       DataLen,
    IN TPM_PCRINDEX PCRIndex,
    OUT TCG_DIGEST  *Digest,
    OUT TCG_DIGEST  *NewPCRValue )
{
    TPM_1_2_CMD_SHA1COMPLETEEXTEND cmdSHA1Complete;
    TPM_1_2_RET_HEADER             retSHA1Complete;
    TPM_TRANSMIT_BUFFER            InBuffer[2], OutBuffer[3];

    if ( DataLen >= 64 )
    {
        return EFI_INVALID_PARAMETER;
    }

    cmdSHA1Complete.Header.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdSHA1Complete.Header.ParamSize = TcgCommonH2NL(sizeof(cmdSHA1Complete) 
                                       + DataLen);
    cmdSHA1Complete.Header.Ordinal = TPM_H2NL( TPM_ORD_SHA1CompleteExtend );

    if(AutoSupportType()){
         cmdSHA1Complete.Header.Ordinal = TPM_H2NL(TCM_ORD_SHA1CompleteExtend);
    }

    cmdSHA1Complete.PCRIndex       = TcgCommonH2NL( PCRIndex );
    cmdSHA1Complete.NumBytes       = TcgCommonH2NL( DataLen );

    InBuffer[0].Buffer = &cmdSHA1Complete;
    InBuffer[0].Size   = sizeof (cmdSHA1Complete);
    InBuffer[1].Buffer = Data;
    InBuffer[1].Size   = DataLen;

    OutBuffer[0].Buffer = &retSHA1Complete;
    OutBuffer[0].Size   = sizeof (retSHA1Complete);
    OutBuffer[1].Buffer = Digest;
    OutBuffer[1].Size   = sizeof (*Digest);
    OutBuffer[2].Buffer = NewPCRValue;
    OutBuffer[2].Size   = sizeof (*NewPCRValue);

    return TCGPASSTHROUGH( CallbackContext, InBuffer, OutBuffer );
}


EFI_STATUS
__stdcall TcmCommonSha1CompleteExtend(
    IN VOID         *CallbackContext,
    IN VOID         *Data,
    IN UINT32       DataLen,
    IN TPM_PCRINDEX PCRIndex,
    OUT TCM_DIGEST  *Digest,
    OUT TCM_DIGEST  *NewPCRValue )
{
    TPM_1_2_CMD_SHA1COMPLETEEXTEND cmdSHA1Complete;
    TPM_1_2_RET_HEADER             retSHA1Complete;
    TPM_TRANSMIT_BUFFER            InBuffer[2], OutBuffer[3];

    if ( DataLen >= 64 )
    {
        return EFI_INVALID_PARAMETER;
    }

    cmdSHA1Complete.Header.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdSHA1Complete.Header.ParamSize = TcgCommonH2NL(sizeof(cmdSHA1Complete) 
                                       + DataLen);
    cmdSHA1Complete.Header.Ordinal = TPM_H2NL( TPM_ORD_SHA1CompleteExtend );

    if(AutoSupportType()){
         cmdSHA1Complete.Header.Ordinal = TPM_H2NL(TCM_ORD_SHA1CompleteExtend);
    }

    cmdSHA1Complete.PCRIndex       = TcgCommonH2NL( PCRIndex );
    cmdSHA1Complete.NumBytes       = TcgCommonH2NL( DataLen );

    InBuffer[0].Buffer = &cmdSHA1Complete;
    InBuffer[0].Size   = sizeof (cmdSHA1Complete);
    InBuffer[1].Buffer = Data;
    InBuffer[1].Size   = DataLen;

    OutBuffer[0].Buffer = &retSHA1Complete;
    OutBuffer[0].Size   = sizeof (retSHA1Complete);
    OutBuffer[1].Buffer = Digest;
    OutBuffer[1].Size   = sizeof (*Digest);
    OutBuffer[2].Buffer = NewPCRValue;
    OutBuffer[2].Size   = sizeof (*NewPCRValue);

    return TCGPASSTHROUGH( CallbackContext, InBuffer, OutBuffer );
}



EFI_STATUS
__stdcall TcgCommonExtend(
    IN VOID         *CallbackContext,
    IN TPM_PCRINDEX PCRIndex,
    IN TCG_DIGEST   *Digest,
    OUT TCG_DIGEST  *NewPCRValue )
{
    TPM_1_2_CMD_HEADER  cmdHeader;
    TPM_1_2_RET_HEADER  retHeader;
    TPM_TRANSMIT_BUFFER InBuffer[3], OutBuffer[2];

    InBuffer[0].Buffer = &cmdHeader;
    InBuffer[0].Size   = sizeof (cmdHeader);
    InBuffer[1].Buffer = &PCRIndex;
    InBuffer[1].Size   = sizeof (PCRIndex);
    InBuffer[2].Buffer = Digest->digest;
    InBuffer[2].Size   = sizeof (Digest->digest);

    OutBuffer[0].Buffer = &retHeader;
    OutBuffer[0].Size   = sizeof (retHeader);
    OutBuffer[1].Buffer = NewPCRValue->digest;
    OutBuffer[1].Size   = sizeof (NewPCRValue->digest);

    cmdHeader.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdHeader.ParamSize = TPM_H2NL(sizeof (cmdHeader) 
                          + sizeof (PCRIndex) + sizeof (Digest->digest));

    cmdHeader.Ordinal = TPM_H2NL( TPM_ORD_Extend );
    PCRIndex          = TcgCommonH2NL( PCRIndex );

    return TCGPASSTHROUGH( CallbackContext, InBuffer, OutBuffer );
}




EFI_STATUS
__stdcall TcmCommonExtend(
    IN  VOID         *CallbackContext,
    IN  TPM_PCRINDEX PCRIndex,
    IN  TCM_DIGEST   *Digest,
    OUT TCM_DIGEST  *NewPCRValue )
{
    TPM_1_2_CMD_HEADER  cmdHeader;
    TPM_1_2_RET_HEADER  retHeader;
    TPM_TRANSMIT_BUFFER InBuffer[3], OutBuffer[2];

    InBuffer[0].Buffer = &cmdHeader;
    InBuffer[0].Size   = sizeof (cmdHeader);
    InBuffer[1].Buffer = &PCRIndex;
    InBuffer[1].Size   = sizeof (PCRIndex);
    InBuffer[2].Buffer = Digest->digest;
    InBuffer[2].Size   = sizeof (Digest->digest);

    OutBuffer[0].Buffer = &retHeader;
    OutBuffer[0].Size   = sizeof (retHeader);
    OutBuffer[1].Buffer = NewPCRValue->digest;
    OutBuffer[1].Size   = sizeof (NewPCRValue->digest);

    cmdHeader.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdHeader.ParamSize = TPM_H2NL(sizeof (cmdHeader) 
                          + sizeof (PCRIndex) + sizeof (Digest->digest));

    cmdHeader.Ordinal = TPM_H2NL( TCM_ORD_Extend );
    PCRIndex          = TcgCommonH2NL( PCRIndex );

    return TCGPASSTHROUGH( CallbackContext, InBuffer, OutBuffer );
}



EFI_STATUS
__stdcall SHA1HashAll(
    IN VOID            *CallbackContext,
    IN VOID            *HashData,
    IN UINTN           HashDataLen,
    OUT TCG_DIGEST*Digest
)
{
    EFI_STATUS     Status;
    SHA1_CTX       Sha1Ctx;

    TCG_DIGEST     *Sha1Digest;

    Status = SHA1_init( CallbackContext, &Sha1Ctx );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = SHA1_update( CallbackContext, &Sha1Ctx, HashData, HashDataLen );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = SHA1_final( CallbackContext, &Sha1Ctx, &Sha1Digest );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    TcgCommonCopyMem(
        CallbackContext,
        Digest->digest,
        Sha1Digest->digest,
        sizeof (Digest->digest));

Exit:
    return Status;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetHob
//
// Description: Find instance of a HOB type in a HOB list
//
// Input:
//      Type          The HOB type to return.
//      HobStart      The first HOB in the HOB list.
//
// Output:
//      Pointer to the Hob matching the type or NULL
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
VOID* GetHob(
    IN UINT16 Type,
    IN VOID   *HobStart  )
{
    EFI_PEI_HOB_POINTERS Hob;

    Hob.Raw = HobStart;

    //
    // Return input if not found
    //
    if ( HobStart == NULL )
    {
        return HobStart;
    }

    //
    // Parse the HOB list, stop if end of list or matching type found.
    //
    while ( !END_OF_HOB_LIST( Hob ))
    {
        if ( Hob.Header->HobType == Type )
        {
            break;
        }

        Hob.Raw = GET_NEXT_HOB( Hob );
    }

    //
    // Return input if not found
    //
    if ( END_OF_HOB_LIST( Hob ))
    {
        return HobStart;
    }

    return (VOID*)(Hob.Raw);
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   LocateATcgHob
//
// Description:
//
// Input:
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_GUID gEfiAmiCommonHobListGuid = TCG_EFI_HOB_LIST_GUID;
VOID* LocateATcgHob(
    UINTN                   NoTableEntries,
    EFI_CONFIGURATION_TABLE *ConfigTable,
    EFI_GUID                *HOB_guid )
{
    VOID *HobStart;
    VOID *PtrHob;

    while ( NoTableEntries > 0 )
    {
        NoTableEntries--;

        if ((!MemCmp(
                 &ConfigTable[NoTableEntries].VendorGuid,
                 &gEfiAmiCommonHobListGuid, sizeof(EFI_GUID)
                 )))
        {
            HobStart = ConfigTable[NoTableEntries].VendorTable;

            if ( !EFI_ERROR(
            		TcgGetNextGuidHob( &HobStart, HOB_guid, &PtrHob, NULL ) //EIP146351_146352
                     ))
            {
                return PtrHob;
            }
        }
    }
    return NULL;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetNextGuidHob
//
// Description: Find GUID HOB
//
// Input:       HobStart    A pointer to the start hob.
//              Guid        A pointer to a guid.
// Output:
//              Buffer          A pointer to the buffer.
//              BufferSize      Buffer size.
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS TcgGetNextGuidHob( //EIP146351_146352
    IN OUT VOID          **HobStart,
    IN EFI_GUID          * Guid,
    OUT VOID             **Buffer,
    OUT UINTN            *BufferSize OPTIONAL )
{
    EFI_STATUS           Status;
    EFI_PEI_HOB_POINTERS GuidHob;

    if ( Buffer == NULL )
    {
        return EFI_INVALID_PARAMETER;
    }

    for ( Status = EFI_NOT_FOUND; EFI_ERROR( Status );)
    {
        GuidHob.Raw = *HobStart;

        if ( END_OF_HOB_LIST( GuidHob ))
        {
            return EFI_NOT_FOUND;
        }

        GuidHob.Raw = GetHob( EFI_HOB_TYPE_GUID_EXTENSION, *HobStart );

        if ( GuidHob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION )
        {
            if ( CompareGuid( Guid, &GuidHob.Guid->Name ))
            {
                Status  = EFI_SUCCESS;
                *Buffer = (VOID*)((UINT8*)(&GuidHob.Guid->Name) 
                          + sizeof (EFI_GUID));

                if ( BufferSize != NULL )
                {
                    *BufferSize = GuidHob.Header->HobLength
                                  - sizeof (EFI_HOB_GUID_TYPE);
                }
            }
        }

        *HobStart = GET_NEXT_HOB( GuidHob );
    }

    return Status;
}
