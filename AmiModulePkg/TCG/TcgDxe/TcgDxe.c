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

   TcgDxe.c

   Abstract:

   DXE Driver that provides TCG services

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgDxe.c 33    5/09/12 6:37p Fredericko $
//
// $Revision: 33 $
//
// $Date: 5/09/12 6:37p $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TcgDxe.c
//
// Description: 
//  Abstracted functions for Tcg protocol are defined here
//
//<AMI_FHDR_END>
//*************************************************************************
#include "AmiTcg\TcgCommon.h"
#include <AmiTcg\Sha1.h>
#include <AmiTcg\TcgMisc.h>
#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiPeiLib.h>
#include <Protocol\TcgTcmService.h>
#include <Protocol/AcpiSupport.h>
#include "AmiTcg\TcgPc.h"
#include <Protocol\TcgService.h>
#include <Protocol\TpmDevice.h>
#include <Protocol\FirmwareVolume2.h>


EFI_GUID gEfiAmiDTcgLogHobGuid = EFI_TCG_LOG_HOB_GUID;
#pragma pack (1)
typedef struct
{
    EFI_PHYSICAL_ADDRESS PostCodeAddress;
    #if x64_BUILD
    UINT64               PostCodeLength;
    #else
    UINTN                PostCodeLength;
    #endif
} EFI_TCG_EV_POST_CODE;

typedef struct
{
    EFI_TCG_PCR_EVENT_HEADER Header;
    EFI_TCG_EV_POST_CODE     Event;
} PEI_EFI_POST_CODE;
#pragma pack()


typedef struct _TCG_DXE_PRIVATE_DATA
{
    EFI_TCG_PROTOCOL        TcgServiceProtocol;
    EFI_TPM_DEVICE_PROTOCOL *TpmDevice;
} TCG_DXE_PRIVATE_DATA;


typedef struct _TCM_DXE_PRIVATE_DATA
{
    EFI_TCM_PROTOCOL        TcgServiceProtocol;
    EFI_TPM_DEVICE_PROTOCOL *TpmDevice;
} TCM_DXE_PRIVATE_DATA;




EFI_STATUS EFIAPI TcgDxeLogEvent (
    IN EFI_TCG_PROTOCOL *This,
    IN TCG_PCR_EVENT    *TCGLogData,
    IN OUT UINT32       *EventNumber,
    IN UINT32           Flags );

EFI_STATUS EFIAPI TcmDxeLogEvent(
    IN EFI_TCM_PROTOCOL *This,
    IN TCM_PCR_EVENT    *TCGLogData,
    IN OUT UINT32       *EventNumber,
    IN UINT32           Flags );


EFI_GUID gEfiTcgCapHobGuid = EFI_TCG_CAP_HOB_GUID;
static UINTN    TcmBootVar = 0;
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmBootDone
//
// Description: SetEfiOSTransitions
//
// Input:       IN  EFI_EVENT       efiev
//              IN  VOID            *ctx
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
void TcmBootDone(
    IN EFI_EVENT efiev,
    IN VOID      *ctx )
{
    TcmBootVar = 1;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EfiOSReadyToBoot
//
// Description: Sets ready to boot callback on ready to boot
//
// Input:   NONE    
//
// Output:   EFI_STATUS   
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TcmOSTransition()
{
   EFI_EVENT  ReadToBootEvent;
   EFI_STATUS Status;

   #if defined(EFI_EVENT_SIGNAL_READY_TO_BOOT)\
        && EFI_SPECIFICATION_VERSION < 0x20000
       
         Status = pBS->CreateEvent( EFI_EVENT_SIGNAL_READY_TO_BOOT,
                                   EFI_TPL_CALLBACK,
                                   TcmBootDone, NULL, &ReadToBootEvent );
        
   #else
        Status = CreateReadyToBootEvent( EFI_TPL_CALLBACK,
                                         TcmBootDone,
                                         NULL,
                                         &ReadToBootEvent );
   #endif

   return Status;
}


#define _CR( Record, TYPE,\
       Field )((TYPE*) ((CHAR8*) (Record) - (CHAR8*) &(((TYPE*) 0)->Field)))

#define TCG_DXE_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TCG_DXE_PRIVATE_DATA, TcgServiceProtocol )

#define TCM_DXE_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TCM_DXE_PRIVATE_DATA, TcgServiceProtocol )


TCG_ACPI_TABLE                        mTcgAcpiTableTemplate = {
    {
        EFI_ACPI_3_0_TRUSTED_COMPUTING_PLATFORM_ALLIANCE_CAPABILITIES_TABLE_SIGNATURE,
        sizeof (TCG_ACPI_TABLE)
        //
        // Compiler initializes the remaining bytes to 0
        // These fields should be filled in in production
        //
    },
    0,
    TPM_LOG_AREA_MAX_LEN,
    (EFI_PHYSICAL_ADDRESS)( -1 )
};

static TPM_Capabilities_PermanentFlag TcgDxe_Cap;

EFI_STATUS
__stdcall TcgCommonPassThrough(
    IN VOID                    *Context,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    TCG_DXE_PRIVATE_DATA *Private;

    Private = TCG_DXE_PRIVATE_DATA_FROM_THIS( Context );
    return Private->TpmDevice->Transmit(
               Private->TpmDevice,
               NoInputBuffers,
               InputBuffers,
               NoOutputBuffers,
               OutputBuffers
               );
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmCommonPassThrough
//
// Description: Helper function for TCM transmit command
//
// Input:       VOID *Context
//              UINT32 NoInputBuffers
//              TPM_TRANSMIT_BUFFER InputBuffers
//              UINT32 NoOutputBuffers
//              TPM_TRANSMIT_BUFFER OutputBuffers
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
__stdcall TcmCommonPassThrough(
    IN VOID                    *Context,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    TCM_DXE_PRIVATE_DATA *Private;

    Private = TCM_DXE_PRIVATE_DATA_FROM_THIS( Context );
    return Private->TpmDevice->Transmit(
               Private->TpmDevice,
               NoInputBuffers,
               InputBuffers,
               NoOutputBuffers,
               OutputBuffers
               );
}


EFI_STATUS EFIAPI TcgDxeStatusCheck(
    IN EFI_TCG_PROTOCOL                 *This,
    OUT TCG_EFI_BOOT_SERVICE_CAPABILITY *ProtocolCapability,
    OUT UINT32                          *TCGFeatureFlags,
    OUT EFI_PHYSICAL_ADDRESS            *EventLogLocation,
    OUT EFI_PHYSICAL_ADDRESS            *LastEvent )
{
    TCG_LOG_HOB   *TcgLog;
    TCG_PCR_EVENT *EventStart;
    UINTN         Index;

    if ( ProtocolCapability != NULL )
    {
        pBS->SetMem( ProtocolCapability, sizeof (*ProtocolCapability), 0 );
        ProtocolCapability->Size = sizeof (TCG_EFI_BOOT_SERVICE_CAPABILITY);
        ProtocolCapability->StructureVersion.Major = TCG_SPEC_VERSION_MAJOR;
        ProtocolCapability->StructureVersion.Minor = TCG_SPEC_VERSION_MINOR;
        ProtocolCapability->StructureVersion.RevMajor = 0;
        ProtocolCapability->StructureVersion.RevMinor = 0;
        ProtocolCapability->ProtocolSpecVersion.Major = TCG_SPEC_VERSION_MAJOR;
        ProtocolCapability->ProtocolSpecVersion.Minor = TCG_SPEC_VERSION_MINOR;
        ProtocolCapability->ProtocolSpecVersion.RevMajor = 0;
        ProtocolCapability->ProtocolSpecVersion.RevMinor = 0;
        ProtocolCapability->HashAlgorithmBitmap          = 1;    // SHA-1
        ProtocolCapability->TPMPresentFlag               = 1;   // TPM is present.
        ProtocolCapability->TPMDeactivatedFlag    = TcgDxe_Cap.deactivated;
    }

    if ( TCGFeatureFlags != NULL )
    {
        *TCGFeatureFlags = 0;
    }

    EventStart = (TCG_PCR_EVENT*)(UINTN)mTcgAcpiTableTemplate.LogStart;
    TcgLog     = (TCG_LOG_HOB*)EventStart;
    TcgLog--;

    if ( EventLogLocation != NULL )
    {
        *EventLogLocation
            = (EFI_PHYSICAL_ADDRESS)( UINTN ) mTcgAcpiTableTemplate.LogStart;
    }

    if ( LastEvent != NULL )
    {
        if ( TcgLog->EventNum == 0 )
        {
            *LastEvent = 0;
        }
        else {
            Index = TcgLog->EventNum;

            do
            {
                *LastEvent = (EFI_PHYSICAL_ADDRESS)( UINTN ) EventStart;
                EventStart = (TCG_PCR_EVENT*)(UINTN)(
                    *LastEvent
                    + _TPM_STRUCT_PARTIAL_SIZE( TCG_PCR_EVENT, Event[  0] )
                    + EventStart->EventSize
                    );
            } while ( --Index > 0 );
        }
    }

    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgTcmDxeStatusCheck
//
// Description: Tcm Dxe status check function
//
// Input:       IN EFI_TCM_PROTOCOL                 *This,
//              OUT TCM_EFI_BOOT_SERVICE_CAPABILITY *ProtocolCapability,
//              OUT UINT32                          *TCGFeatureFlags,
//              OUT EFI_PHYSICAL_ADDRESS            *EventLogLocation,
//              OUT EFI_PHYSICAL_ADDRESS            *LastEvent
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS EFIAPI TcgTcmDxeStatusCheck(
    IN EFI_TCM_PROTOCOL                 *This,
    OUT TCM_EFI_BOOT_SERVICE_CAPABILITY *ProtocolCapability,
    OUT UINT32                          *TCGFeatureFlags,
    OUT EFI_PHYSICAL_ADDRESS            *EventLogLocation,
    OUT EFI_PHYSICAL_ADDRESS            *LastEvent )
{
    TCG_LOG_HOB   *TcgLog;
    TCM_PCR_EVENT *EventStart;
    UINTN         Index;

    if((AutoSupportType()) && (TcmBootVar == 1)){
        pBS->SetMem( ProtocolCapability, sizeof (TCM_EFI_BOOT_SERVICE_CAPABILITY), 0 );
        if ( TCGFeatureFlags != NULL )*TCGFeatureFlags = 0;
        if ( LastEvent != NULL )*LastEvent = 0;
        return EFI_UNSUPPORTED;
    }

    if ( ProtocolCapability != NULL )
    {
        pBS->SetMem( ProtocolCapability, sizeof (*ProtocolCapability), 0 );
        ProtocolCapability->Size = sizeof (TCG_EFI_BOOT_SERVICE_CAPABILITY);
        ProtocolCapability->StructureVersion.Major = TCG_SPEC_VERSION_MAJOR;
        ProtocolCapability->StructureVersion.Minor = TCG_SPEC_VERSION_MINOR;
        ProtocolCapability->StructureVersion.RevMajor = 0;
        ProtocolCapability->StructureVersion.RevMinor = 0;
        ProtocolCapability->ProtocolSpecVersion.Major = TCG_SPEC_VERSION_MAJOR;
        ProtocolCapability->ProtocolSpecVersion.Minor = TCG_SPEC_VERSION_MINOR;
        ProtocolCapability->ProtocolSpecVersion.RevMajor = 0;
        ProtocolCapability->ProtocolSpecVersion.RevMinor = 0;
        ProtocolCapability->HashAlgorithmBitmap          = 1;    // SHA-1
        ProtocolCapability->TPMPresentFlag               = 1;   // TPM is present.
        ProtocolCapability->TPMDeactivatedFlag    = TcgDxe_Cap.deactivated;
    }

    if ( TCGFeatureFlags != NULL )
    {
        *TCGFeatureFlags = 0;
    }

    EventStart = (TCM_PCR_EVENT*)(UINTN)mTcgAcpiTableTemplate.LogStart;
    TcgLog     = (TCG_LOG_HOB*)EventStart;
    TcgLog--;

    if ( EventLogLocation != NULL )
    {
        *EventLogLocation
            = (EFI_PHYSICAL_ADDRESS)( UINTN ) mTcgAcpiTableTemplate.LogStart;
    }

    if ( LastEvent != NULL )
    {
        if ( TcgLog->EventNum == 0 )
        {
            *LastEvent = 0;
        }
        else {
            Index = TcgLog->EventNum;

            do
            {
                *LastEvent = (EFI_PHYSICAL_ADDRESS)( UINTN ) EventStart;
                EventStart = (TCM_PCR_EVENT*)(UINTN)(
                    *LastEvent
                    + _TPM_STRUCT_PARTIAL_SIZE( TCM_PCR_EVENT, Event[  0] )
                    + EventStart->EventSize
                    );
            } while ( --Index > 0 );
        }
    }

    return EFI_SUCCESS;
}






EFI_STATUS EFIAPI TcgDxeHashAll(
    IN EFI_TCG_PROTOCOL *This,
    IN UINT8            *HashData,
    IN UINT64           HashDataLen,
    IN TCG_ALGORITHM_ID AlgorithmId,
    IN OUT UINT64       *HashedDataLen,
    IN OUT UINT8        **HashedDataResult )
{
    if ( AlgorithmId != TCG_ALG_SHA )
    {
        return EFI_UNSUPPORTED;
    }

    if ( *HashedDataResult == NULL || *HashedDataLen == 0 )
    {
        *HashedDataLen = sizeof (TCG_DIGEST);
        pBS->AllocatePool( EfiBootServicesData,
                           (UINTN)*HashedDataLen,
                           HashedDataResult );

        if ( *HashedDataResult == NULL )
        {
            return EFI_OUT_OF_RESOURCES;
        }
    }

    return SHA1HashAll(
                    This,
                    (VOID*)(UINTN)HashData,
                    (UINTN)HashDataLen,
                    (TCG_DIGEST*)*HashedDataResult);
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  TcmDxeHashAll 
//
// Description: TcmDxeHashAll function [SHA1]
//
// Input:           IN EFI_TCG_PROTOCOL *This,
//                  IN UINT8            *HashData,
//                  IN UINT64           HashDataLen,
//                  IN TCG_ALGORITHM_ID AlgorithmId,
//                  IN OUT UINT64       *HashedDataLen,
//                  IN OUT UINT8        **HashedDataResult
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS EFIAPI TcmDxeHashAll(
    IN EFI_TCM_PROTOCOL *This,
    IN UINT8            *HashData,
    IN UINT64           HashDataLen,
    IN TCG_ALGORITHM_ID AlgorithmId,
    IN OUT UINT64       *HashedDataLen,
    IN OUT UINT8        **HashedDataResult )
{

    if((AutoSupportType()) && (TcmBootVar == 1)){
         return EFI_UNSUPPORTED;
    }

    if ( AlgorithmId != TCG_ALG_SHA )
    {
        return EFI_UNSUPPORTED;
    }

    if ( *HashedDataResult == NULL || *HashedDataLen == 0 )
    {
        *HashedDataLen = sizeof (TCG_DIGEST);
        pBS->AllocatePool( EfiBootServicesData,
                           (UINTN)*HashedDataLen,
                           HashedDataResult );

        if ( *HashedDataResult == NULL )
        {
            return EFI_OUT_OF_RESOURCES;
        }
    }

    return SHA1HashAll(
                    This,
                    (VOID*)(UINTN)HashData,
                    (UINTN)HashDataLen,
                    (TCG_DIGEST*)*HashedDataResult);
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Name:  TcgDxeHashLogExtendEventTpm
//
// Description: TcgDxe common function to Hash, Log and Extend data using TPM
//
// Input:       *This
//              *HashData
//              HashDataLen
//              AlgorithmId,
//              *TCGLogData,
//              *EventNum,
//              *EventLogLastEntry
//
// Output:     EFI STATUS
//
// Modified:
//
// Referrals:  TcgCommonSha1Start, TcgCommonSha1Start, TcgCommonSha1CompleteExtend
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TcgDxeHashLogExtendEventTpm(
    IN EFI_TCG_PROTOCOL      *This,
    IN EFI_PHYSICAL_ADDRESS  HashData,
    IN UINT64                HashDataLen,
    IN TCG_ALGORITHM_ID      AlgorithmId,
    IN OUT TCG_PCR_EVENT     *TCGLogData,
    IN OUT UINT32            *EventNum,
    OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry )
{
    EFI_STATUS           Status;
    UINT32               Sha1MaxBytes;
    TCG_DIGEST           NewPCRValue;
    TCG_DXE_PRIVATE_DATA *Private;

#if defined LOG_EV_EFI_ACTION && LOG_EV_EFI_ACTION == 0
    if(TCGLogData->EventType == EV_EFI_ACTION)
    {
        return EFI_SUCCESS;
    }
#endif

    Private = TCG_DXE_PRIVATE_DATA_FROM_THIS( This );

    Status = Private->TpmDevice->Init( Private->TpmDevice );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = TcgCommonSha1Start( This, TCG_ALG_SHA, &Sha1MaxBytes );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = TcgCommonSha1Update(
        This,
        (UINT8 *)HashData,
        (UINT32)HashDataLen,
        Sha1MaxBytes
        );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

#if defined(TCG_DEBUG_MODE) && (TCG_DEBUG_MODE == 1)
    HashData    += (UINTN)(HashDataLen & ~63);
    HashDataLen &= 63;
#else
    HashData    += (HashDataLen & ~63);
    HashDataLen &= 63;
#endif

    Status = TcgCommonSha1CompleteExtend(
        This,
        (UINT8 *)HashData,
        (UINT32)HashDataLen,
        TCGLogData->PCRIndex,
        &TCGLogData->Digest,
        &NewPCRValue
        );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = TcgDxeLogEvent( This, TCGLogData, EventNum, 1 );

Exit:
    Private->TpmDevice->Close( Private->TpmDevice );
    return Status;
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Name:  TcgDxeHashLogExtendEventTcm
//
// Description: TcgDxe common function to Hash, Log and Extend data using TPM
//
// Input:       *This
//              *HashData
//              HashDataLen
//              AlgorithmId,
//              *TCGLogData,
//              *EventNum,
//              *EventLogLastEntry
//
// Output:     EFI STATUS
//
// Modified:
//
// Referrals:  TcgCommonSha1Start, TcgCommonSha1Start, TcgCommonSha1CompleteExtend
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TcgDxeHashLogExtendEventTcm(
    IN EFI_TCM_PROTOCOL      *This,
    IN EFI_PHYSICAL_ADDRESS  HashData,
    IN UINT64                HashDataLen,
    IN TCG_ALGORITHM_ID      AlgorithmId,
    IN OUT TCM_PCR_EVENT     *TCGLogData,
    IN OUT UINT32            *EventNum,
    OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry )
{
    EFI_STATUS           Status;
    UINT32               Sha1MaxBytes;
    TCM_DIGEST           NewPCRValue;
    TCM_DXE_PRIVATE_DATA *Private;

    Private = TCM_DXE_PRIVATE_DATA_FROM_THIS( This );

    if((AutoSupportType()) && (TcmBootVar == 1)){
        return EFI_UNSUPPORTED;
    }

    Status = TcgCommonSha1Start( This, TCG_ALG_SHA, &Sha1MaxBytes );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = TcgCommonSha1Update(
        This,
        (UINT8 *)HashData,
        (UINT32)HashDataLen,
        Sha1MaxBytes
        );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    HashData    += (HashDataLen & ~63);
    HashDataLen &= 63;

    Status = TcmCommonSha1CompleteExtend(
        This,
        (UINT8 *)HashData,
        (UINT32)HashDataLen,
        TCGLogData->PCRIndex,
        &TCGLogData->Digest,
        &NewPCRValue
        );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }

    Status = TcmDxeLogEvent( This, TCGLogData, EventNum, 1 );

Exit:
    return Status;
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  TcgDxeLogEvent
//
// Description: Logs TCG events in DXE
//
// Input:               IN EFI_TCG_PROTOCOL *This,
//                      IN TCG_PCR_EVENT    *TCGLogData,
//                      IN OUT UINT32       *EventNumber,
//                      IN UINT32           Flags
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS EFIAPI TcgDxeLogEvent(
    IN EFI_TCG_PROTOCOL *This,
    IN TCG_PCR_EVENT    *TCGLogData,
    IN OUT UINT32       *EventNumber,
    IN UINT32           Flags )
{
    EFI_STATUS           Status;
    TCG_LOG_HOB          *TcgLog;
    TCG_DXE_PRIVATE_DATA *Private;
    TCG_DIGEST           NewPCR;

    Private = TCG_DXE_PRIVATE_DATA_FROM_THIS( This );

#if defined LOG_EV_EFI_ACTION && LOG_EV_EFI_ACTION == 0
    if(TCGLogData->EventType == EV_EFI_ACTION)
    {
        return EFI_SUCCESS;
    }
#endif

    Status = EFI_SUCCESS;

    if ( !(Flags & 1))
    {
        Status = Private->TpmDevice->Init( Private->TpmDevice );

        if ( !EFI_ERROR( Status ))
        {
        Status = TcgCommonExtend(
            This,
            TCGLogData->PCRIndex,
            &TCGLogData->Digest,
            &NewPCR
            );
        }
        Private->TpmDevice->Close( Private->TpmDevice );
    }

    if ( !TcgDxe_Cap.deactivated )
    {
        TcgLog = (TCG_LOG_HOB*)(UINTN)mTcgAcpiTableTemplate.LogStart;
        TcgLog--;

        if ( !EFI_ERROR( Status ))
        {
            Status = TcgCommonLogEvent(
                This,
                (TCG_PCR_EVENT*)(TcgLog + 1),
                &TcgLog->TableSize,
                TcgLog->TableMaxSize,
                TCGLogData
                );

            if ( !EFI_ERROR( Status ))
            {
                TcgLog->EventNum++;
                *EventNumber = TcgLog->EventNum;
            }
        }
    }
    return Status;
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  TcmDxeLogEvent
//
// Description: Logs TCM events in DXE
//
// Input:               IN EFI_TCG_PROTOCOL *This,
//                      IN TCG_PCR_EVENT    *TCGLogData,
//                      IN OUT UINT32       *EventNumber,
//                      IN UINT32           Flags
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS EFIAPI TcmDxeLogEvent(
    IN EFI_TCM_PROTOCOL *This,
    IN TCM_PCR_EVENT    *TCGLogData,
    IN OUT UINT32       *EventNumber,
    IN UINT32           Flags )
{
    EFI_STATUS           Status;
    TCG_LOG_HOB          *TcgLog;
    TCM_DXE_PRIVATE_DATA *Private;
    TCM_DIGEST           NewPCR;

    Private = TCM_DXE_PRIVATE_DATA_FROM_THIS( This );

    if((AutoSupportType()) && (TcmBootVar == 1)){
        return EFI_UNSUPPORTED;
    }

    Status = EFI_SUCCESS;

    if ( !(Flags & 1))
    {
        if ( !EFI_ERROR( Status ))
        {
            Status = TcmCommonExtend(
                This,
                TCGLogData->PCRIndex,
                &TCGLogData->Digest,
                &NewPCR);
        }
    }

    if ( !TcgDxe_Cap.deactivated )
    {
        TcgLog = (TCG_LOG_HOB*)(UINTN)mTcgAcpiTableTemplate.LogStart;
        TcgLog--;

        if ( !EFI_ERROR( Status ))
        {
            Status = TcmCommonLogEvent(
                This,
                (TCM_PCR_EVENT*)(TcgLog + 1),
                &TcgLog->TableSize,
                TcgLog->TableMaxSize,
                TCGLogData
                );

            if ( !EFI_ERROR( Status ))
            {
                TcgLog->EventNum++;
                *EventNumber = TcgLog->EventNum;
            }
        }
    }
    return Status;
}






EFI_STATUS EFIAPI TcgDxePassThroughToTpm(
    IN EFI_TCG_PROTOCOL *This,
    IN UINT32           TpmInputParamterBlockSize,
    IN UINT8            *TpmInputParamterBlock,
    IN UINT32           TpmOutputParameterBlockSize,
    IN UINT8            *TpmOutputParameterBlock )
{
    TPM_TRANSMIT_BUFFER InBuffer[1], OutBuffer[1];
    EFI_STATUS Status;
    TCG_DXE_PRIVATE_DATA              *Private;

    //some applications might not set init command before making this call.
    //Just set init commands[locality zero for them]
    Private = TCG_DXE_PRIVATE_DATA_FROM_THIS( This );
    Status = Private->TpmDevice->Init( Private->TpmDevice );
 
    InBuffer[0].Buffer  = TpmInputParamterBlock;
    InBuffer[0].Size    = TpmInputParamterBlockSize;
    OutBuffer[0].Buffer = TpmOutputParameterBlock;
    OutBuffer[0].Size   = TpmOutputParameterBlockSize;

    Status = TcgCommonPassThrough(
        This,
        sizeof (InBuffer) / sizeof (*InBuffer),
        InBuffer,
        sizeof (OutBuffer) / sizeof (*OutBuffer),
        OutBuffer
        );

    Private->TpmDevice->Close( Private->TpmDevice );
    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  TcgDxePassThroughToTcm
//
// Description: Helper function for TCM transmit function
//
// Input:               IN EFI_TCM_PROTOCOL *This,
//                      IN UINT32           TpmInputParamterBlockSize,
//                      IN UINT8            *TpmInputParamterBlock,
//                      IN UINT32           TpmOutputParameterBlockSize,
//                      IN UINT8            *TpmOutputParameterBlock
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:   
//
// Notes:       
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS EFIAPI TcgDxePassThroughToTcm(
    IN EFI_TCM_PROTOCOL *This,
    IN UINT32           TpmInputParamterBlockSize,
    IN UINT8            *TpmInputParamterBlock,
    IN UINT32           TpmOutputParameterBlockSize,
    IN UINT8            *TpmOutputParameterBlock )
{
    TPM_TRANSMIT_BUFFER InBuffer[1], OutBuffer[1];
    EFI_STATUS Status;
    TCM_DXE_PRIVATE_DATA              *Private;

    //some applications might not set init command before making this call.
    //Just set init commands[locality zero for them]
    Private = TCM_DXE_PRIVATE_DATA_FROM_THIS( This );

    if((AutoSupportType()) && (TcmBootVar == 1)){
        return EFI_UNSUPPORTED;
    }

    InBuffer[0].Buffer  = TpmInputParamterBlock;
    InBuffer[0].Size    = TpmInputParamterBlockSize;
    OutBuffer[0].Buffer = TpmOutputParameterBlock;
    OutBuffer[0].Size   = TpmOutputParameterBlockSize;

    Status = TcmCommonPassThrough(
        This,
        sizeof (InBuffer) / sizeof (*InBuffer),
        InBuffer,
        sizeof (OutBuffer) / sizeof (*OutBuffer),
        OutBuffer
        );

    return EFI_SUCCESS;
}


EFI_STATUS EFIAPI TcgDxeHashLogExtendEvent(
    IN EFI_TCG_PROTOCOL      *This,
    IN EFI_PHYSICAL_ADDRESS  HashData,
    IN UINT64                HashDataLen,
    IN TCG_ALGORITHM_ID      AlgorithmId,
    IN OUT TCG_PCR_EVENT     *TCGLogData,
    IN OUT UINT32            *EventNumber,
    OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry )
{
    EFI_STATUS Status;
    UINT64 DigestSize;
    UINT8                             *HashResult;

#if defined LOG_EV_EFI_ACTION && LOG_EV_EFI_ACTION == 0
    if(TCGLogData->EventType == EV_EFI_ACTION)
    {
        return EFI_SUCCESS;
    }
#endif

    DigestSize = sizeof (TCGLogData->Digest);
    HashResult = TCGLogData->Digest.digest;
    Status     = TcgDxeHashAll(
        This,
        (UINT8 *)HashData,
        HashDataLen,
        AlgorithmId,
        &DigestSize,
        &HashResult
        );

    if ( !EFI_ERROR( Status ))
    {
        Status = TcgDxeLogEvent(
            This,
            TCGLogData,
            EventNumber,
            0
            );
    }
    return Status;
}


static TCG_DXE_PRIVATE_DATA mTcgDxeData = {
    {
        TcgDxeStatusCheck,
        TcgDxeHashAll,
        TcgDxeLogEvent,
        TcgDxePassThroughToTpm,
        TcgDxeHashLogExtendEvent
    },
    NULL
};


static TCM_DXE_PRIVATE_DATA mTcmDxeData = {
    {
        TcgTcmDxeStatusCheck,
        TcmDxeHashAll,
        TcmDxeLogEvent,
        TcgDxePassThroughToTcm,
        TcgDxeHashLogExtendEventTcm       
    },
    NULL
};

//***********************************************************************
//                      MOR RELATED FUNCTIONS
//***********************************************************************

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   OverwriteSystemMemory
//
// Description: Overwrites system memory
//
// Input:      
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS OverwriteSystemMemory(
)
{

  EFI_STATUS                           Status;
  UINT8                                TmpMemoryMap[1];
  UINTN                                MapKey;
  UINTN                                DescriptorSize;
  UINT32                               DescriptorVersion;
  UINTN                                MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR                *MemoryMap;
  EFI_MEMORY_DESCRIPTOR                *MemoryMapPtr;
  BOOLEAN                              IsRuntime;
  UINTN                                Index;
  UINT64                                Size;
  IsRuntime = FALSE;

  //
  // Get System MemoryMapSize
  //
  MemoryMapSize = 1;
  Status = pBS->GetMemoryMap (
                  &MemoryMapSize,
                  (EFI_MEMORY_DESCRIPTOR *)TmpMemoryMap,
                  &MapKey,
                  &DescriptorSize,
                  &DescriptorVersion
                  );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);
  //
  // Enlarge space here, because we will allocate pool now.
  //
  MemoryMapSize += EFI_PAGE_SIZE;
  Status = pBS->AllocatePool (
                  EfiBootServicesData,
                  MemoryMapSize,
                  (VOID**)&MemoryMap
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Get System MemoryMap
  //
  Status = pBS->GetMemoryMap (
                  &MemoryMapSize,
                  MemoryMap,
                  &MapKey,
                  &DescriptorSize,
                  &DescriptorVersion
                  );
  ASSERT_EFI_ERROR (Status);

  MemoryMapPtr = MemoryMap;
  //
  // Search the request Address
  //
  for (Index = 0; Index < (MemoryMapSize / DescriptorSize); Index++) {
      switch (MemoryMap->Type){
            case EfiMemoryMappedIO:
            case EfiReservedMemoryType:
            case EfiRuntimeServicesCode:
            case EfiRuntimeServicesData:
            case EfiUnusableMemory:
            case EfiMemoryMappedIOPortSpace:
            case EfiPalCode:
            case EfiACPIReclaimMemory:
            case EfiACPIMemoryNVS:
            case EfiBootServicesCode:
            case EfiBootServicesData:
            case EfiLoaderCode:
            case EfiLoaderData:
            case EfiMaxMemoryType:
                break;
            default: 
               Size = Shl64(MemoryMap->NumberOfPages, EFI_PAGE_SHIFT); 
               MemSet((VOID*)MemoryMap->PhysicalStart, (UINTN)Size, 0); //EIP132975 
        }
    MemoryMap = (EFI_MEMORY_DESCRIPTOR *)((UINTN)MemoryMap + DescriptorSize);
  }

  //
  // Done
  //
  pBS->FreePool (MemoryMapPtr);

  return IsRuntime;



}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   ReadMORValue
//
// Description: Reads TCG MOR variable
//
// Input:       IN  EFI_PEI_SERVICES  **PeiServices,
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
VOID ReadMORValue( )
{
    CHAR16     UefiMor[]   = L"MemoryOverwriteRequestControl";
    EFI_GUID   MorUefiGuid = MEMORY_ONLY_RESET_CONTROL_GUID;
    UINT8      mor         = 0;
    UINTN      size        = sizeof(mor);
    EFI_STATUS Status;

   
    Status = pRS->GetVariable( UefiMor, &MorUefiGuid,
                               NULL, &size, &mor );

    if(EFI_ERROR(Status))return;

    if ( mor == 1 )
    {

        //clear memory
        TRACE((-1,"MOR: before Clear memory"));
        Status = OverwriteSystemMemory();
        TRACE((-1,"MOR: After Clear memory"));
	}	
}




//**********************************************************************
//                      MOR FUNCTIONS END
//**********************************************************************
//**********************************************************************
//<AMI_PHDR_START>
//
// Name: OnAcpiInstalled
//
// Description: Adds Tcg Table to Acpi Tables
//
// Input:       IN      EFI_EVENT ev
//              IN      Callback Context *ctx
//
// Output:      Device path size
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS OnAcpiInstalled(
    IN EFI_EVENT ev,
    IN VOID      *ctx )
{
    EFI_STATUS                Status;
    EFI_ACPI_SUPPORT_PROTOCOL *acpi;
    UINTN                     handle = 0;

    TRACE((TRACE_ALWAYS, "Adding TCG ACPI table...\n"));
    Status = pBS->LocateProtocol( &gEfiAcpiSupportGuid, NULL, &acpi );

    if ( EFI_ERROR( Status ))
    {
        return EFI_ABORTED;
    }

    mTcgAcpiTableTemplate.Header.Revision = TCG_TBL_REV;
    MemCpy( mTcgAcpiTableTemplate.Header.OemId,
            TCG_OEMID,
            sizeof(mTcgAcpiTableTemplate.Header.OemId));

    mTcgAcpiTableTemplate.Header.OemTableId      = TCG_TBL_OEM_ID;
    mTcgAcpiTableTemplate.Header.OemRevision     = TCG_TBL_OEM_REV;
    mTcgAcpiTableTemplate.Header.CreatorId       = TCG_CREATOR_ID;
    mTcgAcpiTableTemplate.Header.CreatorRevision = TCG_CREATOR_REVISION;
    mTcgAcpiTableTemplate.Reserved               = TCG_PLATFORM_CLASS;

    Status = acpi->SetAcpiTable( acpi, &mTcgAcpiTableTemplate, TRUE,
                                 EFI_ACPI_TABLE_VERSION_ALL,
                                 &handle );
    return Status;
}





EFI_STATUS
EFIAPI SetTcgAcpiTable()
{
   EFI_STATUS                Status;
   EFI_ACPI_SUPPORT_PROTOCOL *acpi;
   EFI_EVENT                 ev;
   UINTN                        handle = 0;
   static    VOID            *reg = NULL;

   TRACE((TRACE_ALWAYS, "SetTcgAcpiTable....\n"));
   Status = pBS->LocateProtocol( &gEfiAcpiSupportGuid, NULL, &acpi );

   if(EFI_ERROR(Status)){
    
      Status = pBS->CreateEvent( EFI_EVENT_NOTIFY_SIGNAL,
    		  	  	  	  	  	  TPL_CALLBACK,
                                   OnAcpiInstalled,
                                   NULL,
                                   &ev );

      TRACE((TRACE_ALWAYS, "Status = %r\n"));
      ASSERT( !EFI_ERROR( Status ));
      if(EFI_ERROR(Status))return Status;
      Status = pBS->RegisterProtocolNotify( &gEfiAcpiSupportGuid, ev, &reg );
      return Status;
   }

   mTcgAcpiTableTemplate.Header.Revision = TCG_TBL_REV;
   MemCpy( mTcgAcpiTableTemplate.Header.OemId,TCG_OEMID,
           sizeof(mTcgAcpiTableTemplate.Header.OemId));

    mTcgAcpiTableTemplate.Header.OemTableId      = TCG_TBL_OEM_ID;
    mTcgAcpiTableTemplate.Header.OemRevision     = TCG_TBL_OEM_REV;
    mTcgAcpiTableTemplate.Header.CreatorId       = TCG_CREATOR_ID;
    mTcgAcpiTableTemplate.Header.CreatorRevision = TCG_CREATOR_REVISION;
    mTcgAcpiTableTemplate.Reserved               = TCG_PLATFORM_CLASS;

    Status = acpi->SetAcpiTable( acpi, &mTcgAcpiTableTemplate, TRUE,
                                 EFI_ACPI_TABLE_VERSION_ALL,
                                 &handle );

    return Status;
}


static EFI_STATUS CopyLogToAcpiNVS(
    void )
{
    EFI_STATUS Status;
    TCG_LOG_HOB                       *TcgLog = NULL;
    void**                                 DummyPtr;

    TcgLog = (TCG_LOG_HOB*)                   LocateATcgHob(
        pST->NumberOfTableEntries,
        pST->ConfigurationTable,
        &gEfiAmiDTcgLogHobGuid );

    DummyPtr = &TcgLog;

    if ( *DummyPtr == NULL )
    {
        return EFI_NOT_FOUND;
    }

    Status = pBS->AllocatePages(
        AllocateMaxAddress,
        EfiACPIMemoryNVS,
        EFI_SIZE_TO_PAGES( mTcgAcpiTableTemplate.LogMaxLength + sizeof (*TcgLog)),
        (UINT64*)(UINTN)&mTcgAcpiTableTemplate.LogStart
        );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    pBS->SetMem(
        (VOID*)((UINTN)mTcgAcpiTableTemplate.LogStart),
        (UINTN)mTcgAcpiTableTemplate.LogMaxLength,
        0x00 // Clean up this region with this value.
        );

    TcgLog->TableMaxSize = mTcgAcpiTableTemplate.LogMaxLength;
    
    if(TcgDxe_Cap.deactivated){
            TcgLog->EventNum = 0;
            TcgLog->TableSize = 0;

            pBS->CopyMem(
            (VOID*)(UINTN)mTcgAcpiTableTemplate.LogStart,
            TcgLog,
            sizeof (TCG_LOG_HOB));

        mTcgAcpiTableTemplate.LogStart += sizeof (*TcgLog);
    }else{
       pBS->CopyMem(
        (VOID*)(UINTN)mTcgAcpiTableTemplate.LogStart,
        TcgLog,
        TcgLog->TableSize + sizeof (*TcgLog)
        );
        mTcgAcpiTableTemplate.LogStart += sizeof (*TcgLog);
    }

    Status = SetTcgAcpiTable();

    return Status;
}


typedef struct _TCG_DXE_FWVOL_LIST
{
    EFI_LIST_ENTRY Link;
    EFI_HANDLE FvHandle;
} TCG_DXE_FWVOL_LIST;

static EFI_LIST_ENTRY mMeasuredFvs = {
    &mMeasuredFvs,
    &mMeasuredFvs
};

static EFI_STATUS AddFvToMeasuredFvList(
    EFI_HANDLE FvHandle )
{
    TCG_DXE_FWVOL_LIST                *NewEntry;

    pBS->AllocatePool( EfiBootServicesData, sizeof (*NewEntry), &NewEntry );

    if ( NewEntry == NULL )
    {
        return EFI_OUT_OF_RESOURCES;
    }

    NewEntry->FvHandle = FvHandle;
    InsertTailList( &mMeasuredFvs, &NewEntry->Link );
    return EFI_SUCCESS;
}

static EFI_STATUS HashAllFilesInFv(
    IN SHA1_CTX        *Sha1Ctx,
    IN EFI_FIRMWARE_VOLUME_PROTOCOL
                       *FwVol,
    IN EFI_FV_FILETYPE FileType )
{
    EFI_STATUS Status;
    VOID                              *KeyBuffer = NULL;
    EFI_GUID FileName;
    EFI_FV_FILE_ATTRIBUTES FileAttr;
    UINTN FileSize;
    VOID                              *FileBuffer;
    UINT32 AuthStat;


    Status = pBS->AllocatePool( EfiBootServicesData, FwVol->KeySize, KeyBuffer );

    if ( KeyBuffer != NULL )
    {
        pBS->SetMem( KeyBuffer, FwVol->KeySize, 0 );
    }

    if ( KeyBuffer == NULL )
    {
        return EFI_OUT_OF_RESOURCES;
    }

    do
    {
        Status = FwVol->GetNextFile(
            FwVol,
            KeyBuffer,
            &FileType,
            &FileName,
            &FileAttr,
            &FileSize
            );

        if ( !EFI_ERROR( Status ))
        {
            FileBuffer = NULL;
            Status     = FwVol->ReadFile(
                FwVol,
                &FileName,
                &FileBuffer,
                &FileSize,
                &FileType,
                &FileAttr,
                &AuthStat
                );
            ASSERT( !EFI_ERROR( Status ));

            SHA1_update( NULL, Sha1Ctx, FileBuffer, FileSize );
            pBS->FreePool( FileBuffer );
        }
    } while ( !EFI_ERROR( Status ));

    pBS->FreePool( KeyBuffer );
    return EFI_SUCCESS;
}



static EFI_STATUS MeasureFv(
    IN EFI_TCG_PROTOCOL *This,
    IN EFI_HANDLE       FvHandle )
{
    EFI_STATUS Status;
    EFI_FIRMWARE_VOLUME_PROTOCOL      *FwVol;
    EFI_LIST_ENTRY                    *Link;
    TCG_DXE_FWVOL_LIST                *FwVolList;
    SHA1_CTX Sha1Ctx;
    TCG_DIGEST                        *FvDigest;
    EFI_TCG_PCR_EVENT TcgEvent;
    UINT32 EventNum;

    for ( Link = mMeasuredFvs.ForwardLink;
          Link != &mMeasuredFvs;
          Link = Link->ForwardLink )
    {
        FwVolList = _CR( Link, TCG_DXE_FWVOL_LIST, Link );

        if ( FvHandle == FwVolList->FvHandle )
        {
            return EFI_SUCCESS;
        }
    }

    Status = AddFvToMeasuredFvList( FvHandle );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    Status = pBS->HandleProtocol(
        FvHandle,
        &gEfiFirmwareVolume2ProtocolGuid,
        &FwVol
        );
    ASSERT( !EFI_ERROR( Status ));

    SHA1_init( NULL, &Sha1Ctx );
    Status = HashAllFilesInFv( &Sha1Ctx, FwVol, EFI_FV_FILETYPE_DRIVER );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }
    Status = HashAllFilesInFv( &Sha1Ctx, FwVol, EFI_FV_FILETYPE_APPLICATION );

    if ( EFI_ERROR( Status ))
    {
        goto Exit;
    }
    SHA1_final( NULL, &Sha1Ctx, &FvDigest );

    TcgEvent.Header.PCRIndex      = PCRi_OPROM_CODE;
    TcgEvent.Header.EventType     = EV_EVENT_TAG;
    TcgEvent.Event.Tagged.EventID = EV_ID_OPROM_EXECUTE;
    TcgEvent.Event.Tagged.EventSize
        = sizeof (TcgEvent.Event.Tagged.EventData.OptionRomExecute);
    TcgEvent.Header.EventDataSize
        = _TPM_STRUCT_PARTIAL_SIZE( struct _EFI_TCG_EV_TAG, EventData )
        + TcgEvent.Event.Tagged.EventSize;

    TcgEvent.Event.Tagged.EventData.OptionRomExecute.PFA      = 0;
    TcgEvent.Event.Tagged.EventData.OptionRomExecute.Reserved = 0;
    TcgEvent.Event.Tagged.EventData.OptionRomExecute.Hash     = *FvDigest;

    Status = TcgDxeHashLogExtendEvent(
        This,
        (UINTN)&TcgEvent.Event,
        TcgEvent.Header.EventDataSize,
        TCG_ALG_SHA,
        (TCG_PCR_EVENT*)&TcgEvent,
        &EventNum,
        0
        );

Exit:
    return Status;
}




static
VOID
EFIAPI OnFwVolInstalled(
    IN EFI_EVENT Event,
    IN VOID      *Context )
{
    EFI_STATUS Status;
    EFI_HANDLE                        *Handles;
    UINTN NumHandles;

    Handles    = NULL;
    NumHandles = 0;
    Status     = pBS->LocateHandleBuffer(
        ByRegisterNotify,
        NULL,
        *(VOID**)Context,
        &NumHandles,
        &Handles
        );

    ASSERT(!EFI_ERROR( Status ));

    while (!EFI_ERROR( Status ) && NumHandles > 0 )
    {
        NumHandles--;
        Status = MeasureFv( &mTcgDxeData.TcgServiceProtocol,
                            Handles[NumHandles] );
    }

    if ( Handles != NULL )
    {
        pBS->FreePool( Handles );
    }
}

#define FAST_BOOT_VARIABLE_GUID \
    { 0xb540a530, 0x6978, 0x4da7, 0x91, 0xcb, 0x72, 0x7, 0xd7, 0x64, 0xd2, 0x62 }
EFI_GUID FastBootVariableGuid = FAST_BOOT_VARIABLE_GUID;
EFI_GUID AmitcgefiOsVariableGuid = AMI_TCG_EFI_OS_VARIABLE_GUID;



VOID AbortFastbootOnTpmAction()
{
    UINTN       Size = 0;
    EFI_STATUS  Status;
    VOID *Fastboot = NULL;
    UINTN          PpiSize = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;

     Status = pRS->GetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               NULL, \
                               &PpiSize, \
                               &Temp );

    if(EFI_ERROR(Status))return;

    if (Temp.RQST != 0)
    {

        pRS->SetVariable(L"LastBoot",
                         &FastBootVariableGuid,
                         EFI_VARIABLE_NON_VOLATILE | 
                         EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                         EFI_VARIABLE_RUNTIME_ACCESS,
                         Size,
                         (VOID *)&Fastboot);

        Status = pRS->SetVariable(L"FastEfiBootOption", 
                               &FastBootVariableGuid,
                               EFI_VARIABLE_NON_VOLATILE | 
                               EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                               EFI_VARIABLE_RUNTIME_ACCESS,
                               Size,
                              (VOID *)&Fastboot);
       
    }

}



//*******************************************************************************
//<AMI_PHDR_START>
//
// Procedure:   FindAndMeasureDxeFWVol
//
// Description: 
//
// Input:      
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// TODO: USE FFS intergrity CHKsum algorithm for hash
// Notes:
//<AMI_PHDR_END>
//******************************************************************************
EFI_STATUS FindAndMeasureDxeFWVol()
{
    PEI_EFI_POST_CODE       ev;
    UINT32                   n;
    UINTN                   last;
    EFI_STATUS              Status;
    EFI_GUID                      NameGuid =\
                            {0x7739f24c, 0x93d7, 0x11d4,\
                             0x9a, 0x3a, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d};
    UINTN                         Size;
    void                          *Buffer = NULL;
    EFI_TCG_PROTOCOL			  *TcgProtocol;
    VOID                          *HobStart;
    UINTN                          TableEntries;
    EFI_PEI_HOB_POINTERS           FirmwareVolumeHob;
    BOOLEAN                        Found=FALSE;
   
    TableEntries = pST->NumberOfTableEntries;

    while ( TableEntries > 0 )
    {
        TableEntries--;

        if ((!MemCmp(
                 &pST->ConfigurationTable[TableEntries].VendorGuid,
                 &NameGuid, sizeof(EFI_GUID))))
        {            
            HobStart = pST->ConfigurationTable[TableEntries].VendorTable;
            FirmwareVolumeHob.Raw = GetHob (EFI_HOB_TYPE_FV, HobStart);
            if (FirmwareVolumeHob.Header->HobType != EFI_HOB_TYPE_FV) {
                 continue;
            }
            break;   
        }
    }        
   
    for (Status = EFI_NOT_FOUND; EFI_ERROR (Status);) {
        if (END_OF_HOB_LIST (FirmwareVolumeHob)) {
          return EFI_NOT_FOUND;
        }

        if (GET_HOB_TYPE (FirmwareVolumeHob) == EFI_HOB_TYPE_FV) {
            if (((UINT64)FirmwareVolumeHob.FirmwareVolume->BaseAddress) == FV_MAIN_BASE) {
                Found = TRUE;
                break;
            }
        }
        
        FirmwareVolumeHob.Raw = GET_NEXT_HOB (FirmwareVolumeHob);
    }

    if(Found== FALSE)return EFI_NOT_FOUND;

    TRACE((-1,"TcgDxe:: Found Volume: Base = %x Length = %x",\
         FirmwareVolumeHob.FirmwareVolume->BaseAddress,\
         FirmwareVolumeHob.FirmwareVolume->Length));

    Status = pBS->AllocatePool(
                    EfiBootServicesData, 
					(UINTN)FirmwareVolumeHob.FirmwareVolume->Length, //EIP132975 
					&Buffer);

    if(EFI_ERROR(Status) || Buffer == NULL) return EFI_OUT_OF_RESOURCES;
  
     Status = pBS->LocateProtocol (&gEfiTcgProtocolGuid,\
                                                 NULL, &TcgProtocol);
     if(EFI_ERROR(Status)){
                  return Status;
      }
//EIP139219 EIP139833 >>	  
/*
      if(FirmwareVolumeHob.FirmwareVolume->BaseAddress == FV_MAIN_BASE)
      {
         if(FirmwareVolumeHob.FirmwareVolume->Length > TCG_SIZE){
              Size = TCG_SIZE;
         }else{
             Size = (UINTN)FirmwareVolumeHob.FirmwareVolume->Length;		//(CSP20131003B)
         }

         pBS->CopyMem(Buffer, (UINT8 *)(EFI_PHYSICAL_ADDRESS)FirmwareVolumeHob.FirmwareVolume->BaseAddress,\
                    Size);

      }else{

        Buffer = (UINT8 *)(EFI_PHYSICAL_ADDRESS)FirmwareVolumeHob.FirmwareVolume->BaseAddress;
        Size = (UINTN)FirmwareVolumeHob.FirmwareVolume->Length;				//(CSP20131003B)
      }
      
      ev.Header.PCRIndex      = PCRi_CRTM_AND_POST_BIOS;
      ev.Header.EventType     = EV_POST_CODE;
      ev.Header.EventDataSize = sizeof (EFI_TCG_EV_POST_CODE);
      ev.Event.PostCodeAddress = \
                    (EFI_PHYSICAL_ADDRESS)FirmwareVolumeHob.FirmwareVolume->BaseAddress;

  #if defined x64_BUILD &&  x64_BUILD == 1
      ev.Event.PostCodeLength = Size;
  #else
      ev.Event.PostCodeLength = Size;
  #endif
*/
     // +> Using the DXE Image Rom and Fix Length to measue the TCG CRTM Log, to avoid the Memory changed cause the Measure Value change.
     Size = 0x100;
     pBS->CopyMem(Buffer, (UINT8 *)(EFI_PHYSICAL_ADDRESS)FV_MAIN_BASE,
                         Size);
     ev.Header.PCRIndex      = PCRi_CRTM_AND_POST_BIOS;
     ev.Header.EventType     = EV_POST_CODE;
     ev.Header.EventDataSize = sizeof (EFI_TCG_EV_POST_CODE);
     ev.Event.PostCodeAddress = \
                         (EFI_PHYSICAL_ADDRESS)FV_MAIN_BASE;
     // <+ End TCG CRTM Measure.
//EIP139219 EIP139833 <<
                            
      Status = TcgProtocol->HashLogExtendEvent (TcgProtocol,
				                            (EFI_PHYSICAL_ADDRESS)Buffer,
				                            Size,
				                            TCG_ALG_SHA,
				                            (TCG_PCR_EVENT*)&ev,
				                            &n,
				                            (EFI_PHYSICAL_ADDRESS *)&last); //EIP132975 
      return Status;
}



EFI_STATUS
EFIAPI TcgDxeEntry(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS Status;
    BOOLEAN Support  = FALSE;
    TPM_GetCapabilities_Input   cmdGetCap;
    TPM_RQU_COMMAND_HDR         NuvotoncmdGetTpmStatus;
    UINT8                       result[0x100];
   
    Status = pBS->LocateProtocol(
                &gEfiTpmDeviceProtocolGuid,
                NULL,
                &mTcgDxeData.TpmDevice);    

     Status = pBS->LocateProtocol(
        &gEfiTpmDeviceProtocolGuid,
        NULL,
        &mTcmDxeData.TpmDevice); 
  
    if ( EFI_ERROR( Status )){
        return Status;
    }

    Status = CopyLogToAcpiNVS( );
 
    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    if(*(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF00) != 0x1050) 
    {
        cmdGetCap.Tag         = TPM_H2NS( TPM_TAG_RQU_COMMAND );
        cmdGetCap.ParamSize   = TPM_H2NL( sizeof (cmdGetCap));

        if(AutoSupportType()){
            cmdGetCap.CommandCode = TPM_H2NL( TCM_ORD_GetCapability );
            cmdGetCap.CommandCode = TPM_H2NL( TCM_ORD_GetCapability );
            cmdGetCap.caparea     = TPM_H2NL( TPM_CAP_FLAG );   
        }else{
            cmdGetCap.CommandCode = TPM_H2NL( TPM_ORD_GetCapability );
            cmdGetCap.CommandCode = TPM_H2NL( TPM_ORD_GetCapability );
            cmdGetCap.caparea     = TPM_H2NL( TPM_CAP_FLAG );
        }

        cmdGetCap.subCapSize  = TPM_H2NL( 4 ); // subCap is always 32bit long
        cmdGetCap.subCap      = TPM_H2NL( TPM_CAP_FLAG_PERMANENT );

        if(AutoSupportType()){
              Status = TcgDxePassThroughToTcm( &mTcmDxeData.TcgServiceProtocol,
                                               sizeof (cmdGetCap),
                                               (UINT8*)&cmdGetCap,
                                               sizeof (TPM_Capabilities_PermanentFlag),
                                               (UINT8*)&TcgDxe_Cap );
         }else{
                Status = TcgDxePassThroughToTpm( &mTcgDxeData.TcgServiceProtocol,
                                             sizeof (cmdGetCap),
                                             (UINT8*)&cmdGetCap,
                                             sizeof (TPM_Capabilities_PermanentFlag),
                                             (UINT8*)&TcgDxe_Cap );
         }
    }else{
        MemSet(&TcgDxe_Cap,sizeof(TPM_Capabilities_PermanentFlag), 0);
        NuvotoncmdGetTpmStatus.tag         = TPM_H2NS( TPM_TAG_RQU_COMMAND );
        NuvotoncmdGetTpmStatus.paramSize   = TPM_H2NL( sizeof (TPM_RQU_COMMAND_HDR));
        NuvotoncmdGetTpmStatus.ordinal     = TPM_H2NL( NTC_ORD_GET_TPM_STATUS );

        Status = TcgDxePassThroughToTpm( &mTcgDxeData.TcgServiceProtocol,
                                    sizeof (NuvotoncmdGetTpmStatus),
                                    (UINT8*)&NuvotoncmdGetTpmStatus,
                                    0x100,
                                    result );

        if(((NUVOTON_SPECIFIC_FLAGS *) result)->RetCode == 0)
        {
            if(((NUVOTON_SPECIFIC_FLAGS *)result)->isdisabled){
               TcgDxe_Cap.disabled = 1; 
            }

            if(((NUVOTON_SPECIFIC_FLAGS *)result)->isdeactivated){
                TcgDxe_Cap.deactivated = 1; 
            }

            if(((NUVOTON_SPECIFIC_FLAGS *)result)->isOwnerSet){
                TcgDxe_Cap.ownership = 1; 
            }
        }else{

            TcgDxe_Cap.RetCode = ((NUVOTON_SPECIFIC_FLAGS *)result)->RetCode;
        }
    }

    if ( TPM_H2NL(TcgDxe_Cap.RetCode)!=0)
    {
        return EFI_SUCCESS;
    }

    Support = AutoSupportType();

    ReadMORValue();

    if(!Support){
        Status = pBS->InstallMultipleProtocolInterfaces(
                   &ImageHandle,
                   &gEfiTcgProtocolGuid,
                   &mTcgDxeData.TcgServiceProtocol,
                   NULL);

        FindAndMeasureDxeFWVol();
        AbortFastbootOnTpmAction();        
        return Status;
    }else{

        TcmOSTransition();
        return pBS->InstallMultipleProtocolInterfaces(
                   &ImageHandle,
                   &gEfiTcgProtocolGuid,
                   &mTcmDxeData.TcgServiceProtocol,
                   NULL);
    }

}

