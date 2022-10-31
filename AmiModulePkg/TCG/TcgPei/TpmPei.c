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

   TpmPei.h

   Abstract:

   PEI driver for TPM devices

   --*/
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TpmPei.c 17    4/01/11 11:13a Fredericko $
//
// $Revision: 17 $
//
// $Date: 4/01/11 11:13a $
//*************************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TpmPei.c
//
// Description: 
//  Abstracted functions for Tpm ppis' are defined here
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <Pei.h>
#include <AmiTcg\TcgTpm12.h>
#include <AmiTcg\TpmLib.h>
#include <AmiPeiLib.h>
#include <token.h>
#include "AmiTcg\TcgCommon.h"
#include <Ppi/TcgService.h>
#include <Ppi/TpmDevice.h>

#define _CR( Record, TYPE,\
             Field )((TYPE*) ((CHAR8*) (Record) - (CHAR8*) &(((TYPE*) 0)->Field)))
             
#define INTEL_VID 0x8086
#define TCG_VID_OFFSET 0x0F00

BOOLEAN iTPMVerifyMeStatus( );

#define TPM_PEI_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TPM_PEI_PRIVATE_DATA, TpmPpi )

typedef struct _TPM_PEI_PRIVATE_DATA
{
    PEI_TPM_PPI          TpmPpi;
    EFI_PHYSICAL_ADDRESS BaseAddr;
} TPM_PEI_PRIVATE_DATA;

#if TCG_LEGACY == 1

typedef struct _TPM_PEI_CALLBACK
{
    EFI_PEI_NOTIFY_DESCRIPTOR NotifyDesc;
    EFI_FFS_FILE_HEADER       *FfsHeader;

} TPM_PEI_CALLBACK;


VOID	TpmPeiCallMPDriver(IN EFI_PEI_SERVICES **PeiServices, IN UINT8 CFuncID, 
							TPMTransmitEntryStruct *CData, UINT32* OUT CRetVal);

EFI_GUID  gTpmCallbackguid = AMI_TPM_LEGACY_GUID;
#endif


#if TCG_LEGACY == 0
EFI_STATUS
EFIAPI TpmPeiInit(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    TPM_PEI_PRIVATE_DATA *Private;

    Private = TPM_PEI_PRIVATE_DATA_FROM_THIS( This );
    return TisRequestLocality(
               (TPM_1_2_REGISTERS_PTR)( UINTN ) Private->BaseAddr);
}

EFI_STATUS
EFIAPI TpmPeiClose(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    TPM_PEI_PRIVATE_DATA *Private;

    Private = TPM_PEI_PRIVATE_DATA_FROM_THIS( This );
    return TisReleaseLocality(
               (TPM_1_2_REGISTERS_PTR)( UINTN ) Private->BaseAddr);
}

EFI_STATUS
EFIAPI TpmPeiGetStatusInfo(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI TpmPeiTransmit(
    IN PEI_TPM_PPI             *This,
    IN EFI_PEI_SERVICES        **PeiServices,
    IN UINTN                   NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINTN                   NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    TPM_PEI_PRIVATE_DATA  *Private;
    TPM_1_2_REGISTERS_PTR TpmReg;

    Private = TPM_PEI_PRIVATE_DATA_FROM_THIS( This );
    TpmReg  = (TPM_1_2_REGISTERS_PTR)( UINTN ) Private->BaseAddr;
    return TpmLibPassThrough(
               TpmReg,
               NoInputBuffers,
               InputBuffers,
               NoOutputBuffers,
               OutputBuffers
               );
}
#else

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTpmPeiInit
//
// Description: Call to OEM driver to initialize TPM
//
//
// Input:       IN      PEI_TPM_PPI       *This,
//              IN      EFI_PEI_SERVICES  **PeiServices
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TpmPeiInit(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTpmPeiClose
//
// Description: Call to OEM driver to Close TPM
//
//
// Input:       IN  PEI_TPM_PPI       *This,
//              IN  EFI_PEI_SERVICES  **PeiServices
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TpmPeiClose(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    return EFI_SUCCESS;
}



EFI_STATUS
EFIAPI TpmPeiGetStatusInfo(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    return EFI_UNSUPPORTED;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTpmPeiSHA1Setup
//
// Description: Helper function to help with Command transmission to TPM
//
//
// Input:       IN  PEI_TPM_PPI               *This,
//              IN  EFI_PEI_SERVICES          **PeiServices
//              IN  UINTN                     NoInputBuffers
//              IN  TPM_TRANSMIT_BUFFER       *InputBuffers
//              IN  OUT void                  **Buffer
//              IN  UINT8                      *SHA_ARRAY
//
// Output:      BOOLEAN
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN
EFIAPI TpmPeiSHA1Setup(
    IN PEI_TPM_PPI         *This,
    IN EFI_PEI_SERVICES    **PeiServices,
    IN UINTN               NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER *InputBuffers,
    IN OUT void            **Buffer,
    IN UINT8               *SHA_ARRAY )
{
    TPM_1_2_CMD_HEADER *SHA1HeaderCheck;
    void               *Context = NULL;
    UINT8              i = 0, loc = 0;

    //check if this an SHA1 update or Complete call and set the buffer
    //else no need to update the buffer is the same
    SHA1HeaderCheck = (TPM_1_2_CMD_HEADER*)InputBuffers->Buffer;

    if ( SHA1HeaderCheck->Ordinal !=
         TPM_H2NL(TPM_ORD_SHA1CompleteExtend ) && SHA1HeaderCheck->Ordinal !=
         TPM_H2NL( TPM_ORD_SHA1Update )  &&  SHA1HeaderCheck->Ordinal != TPM_H2NL(TPM_ORD_Extend))
    {
        //no need to modify buffers just pass it through
        return FALSE;
    }

    if ( NoInputBuffers > 1 )
    {
        //if we get here it means the buffer needs to be modified!
        for (; i < NoInputBuffers; i++ )
        {
            TcgCommonCopyMem( Context,
                              &SHA_ARRAY[loc],
                              InputBuffers[i].Buffer,
                              InputBuffers[i].Size );

            loc += InputBuffers[i].Size;
        }
    }
    else {
        return FALSE;
    }

    *Buffer =  SHA_ARRAY;
    return TRUE;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTpmPeiTransmit
//
// Description: TPM Transmit function
//
//
// Input:       IN      PEI_TPM_PPI               *This,
//              IN      EFI_PEI_SERVICES          **PeiServices,
//              IN      UINTN                     NoInputBuffers,
//              IN      TPM_TRANSMIT_BUFFER       *InputBuffers,
//              IN      UINTN                     NoOutputBuffers,
//              IN OUT  TPM_TRANSMIT_BUFFER       *OutputBuffers
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
EFIAPI TpmPeiTransmit(
    IN PEI_TPM_PPI             *This,
    IN EFI_PEI_SERVICES        **PeiServices,
    IN UINTN                   NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINTN                   NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    TPM_PEI_PRIVATE_DATA   *Private;
    TPMTransmitEntryStruct FillESI;
    UINT8                  FuncID = MP_FUNCTION_TRANSMIT;
    UINT32                 Ret;
    void                   *buffer;
    UINT8                  SHA_ARRAY[INTERNAL_SHA_ARRAY_LENGTH];
    UINT8                  SHA_ARRAY_OUT[INTERNAL_SHA_ARRAY_LENGTH];
    BOOLEAN                FillBuff = FALSE;
    void                   *Context = NULL;
    UINT8                  i = 0;
    UINT8                  loc = 0;


    if ( !TpmPeiSHA1Setup( This, PeiServices, NoInputBuffers, InputBuffers,
                              &buffer, SHA_ARRAY ))
    {
        FillESI.pbInBuf  = (UINT32)(EFI_PHYSICAL_ADDRESS)InputBuffers->Buffer;
        FillESI.dwInLen  = (UINT32)InputBuffers->Size;
        FillESI.pbOutBuf = (UINT32)(EFI_PHYSICAL_ADDRESS)OutputBuffers->Buffer;
        FillESI.dwOutLen = (UINT32)OutputBuffers->Size;
    }
    else {
        FillESI.pbInBuf = (UINT32)(EFI_PHYSICAL_ADDRESS)SHA_ARRAY;
        FillESI.dwInLen = 0;

        for (; i < NoInputBuffers; i++ )
        {
            FillESI.dwInLen += (UINT32)InputBuffers[i].Size;
        }
        i                = 0;
        FillESI.pbOutBuf = (UINT32)(EFI_PHYSICAL_ADDRESS)SHA_ARRAY_OUT;
        FillBuff         = TRUE;
        FillESI.dwOutLen = 0;

        for (; i < NoOutputBuffers; i++ )
        {
            FillESI.dwOutLen += OutputBuffers[i].Size;
        }
    }

    Private = TPM_PEI_PRIVATE_DATA_FROM_THIS( This );
    TpmPeiCallMPDriver( PeiServices, FuncID, &FillESI, &Ret );

    if ( FillBuff ) //setup output buffer for the interface
    {
        i = 0;
        for (; i < NoOutputBuffers; i++ )
        {        
            TcgCommonCopyMem( Context,
                              OutputBuffers[i].Buffer,
                              &SHA_ARRAY_OUT[loc],
                              OutputBuffers[i].Size );
                              
            loc += OutputBuffers[i].Size;
        }
    }
    return EFI_SUCCESS;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TpmPeiCallMPDriver
//
// Description: Interface to performing MP driver function calls in PEI
//              Uses _ASM directives. Return Value in EAX return as status
//
//
// Input:       IN   EFI_PEI_SERVICES **PeiServices,
//              IN   UINT8 CFuncID,
//              IN   TPMTransmitEntryStruct * CData,
//              OUT  UINT32* CRetVal
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
VOID TpmPeiCallMPDriver(
    IN EFI_PEI_SERVICES    **PeiServices,
    IN UINT8               CFuncID,
    TPMTransmitEntryStruct * CData,
    UINT32* OUT            CRetVal )
{
    UINT32                 ReturnVal;
    FAR32LOCALS            CommonLegX;
    EFI_GUID               guidMA = EFI_TCG_MADriver_HOB_GUID;
    EFI_GUID               guidMP = EFI_TCG_MPDriver_HOB_GUID;
    ESPFUNCSTRUCT          EspStruct;
    VOID                   *Temp = &ReturnVal;
    MASTRUCT               MA;
    UINT32                 MAStart;
    UINT32                 EspVal;


    TPMTransmitEntryStruct IntCData = *CData;
    UINT32                 Address  = (UINT32)&IntCData;

    CommonLegX.Offset   = 0;
    CommonLegX.Selector = 0;

    if (((UINT32)Temp & (UINT32)0xff000000) == (UINT32)0xff000000 )
    {
        FillDriverLoc( &MA.Offset, PeiServices, &guidMA );
        MAStart             = MA.Offset - MA.Codep;
        CommonLegX.Offset   = MA.Offset;
        CommonLegX.Selector = SEL_flatCS;
        Temp                = &EspStruct;

        if ( CommonLegX.Offset == NULL )
        {
            return;
        }

        _asm {
            //fill esp
            push esi
            mov  esi, [ReturnAdd]
            mov  [EspStruct.ReturnAddress], esi
            mov  esi, [MAStart]
            mov  [EspStruct.Header], esi
            pop  esi
            mov  EspVal, esp            // Save ESP
            mov  al, [CFuncID]
            mov  [EspStruct.FuncNum], al
            mov  esp, Temp
            call fword ptr [CommonLegX]
            mov  esp, EspVal            // Restore ESP
ReturnAdd:
            mov  ReturnVal, eax
        }

        *CRetVal = ReturnVal;
        return;
    }
    else {
        FillDriverLoc( &CommonLegX.Offset, PeiServices, &guidMP );
        CommonLegX.Selector = SEL_flatCS;

        _asm {
            xor  eax, eax
            mov  al, [CFuncID]
            cmp  eax, MP_FUNCTION_TRANSMIT
            jnz  Empty_Buff
            mov  ESI, Address
Empty_Buff:
            mov  EspVal, esp            // Save ESP
            call fword ptr [CommonLegX];
            mov  ReturnVal, eax
	    mov  esp, EspVal            // Restore ESP

        }
        *CRetVal = ReturnVal;
    }
}

#endif

static TPM_PEI_PRIVATE_DATA   mTpmPrivate = {
    {
        TpmPeiInit,
        TpmPeiClose,
        TpmPeiGetStatusInfo,
        TpmPeiTransmit
    },
    TPM_BASE_ADDRESS
};


#if TCG_LEGACY == 1
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InitLegacyTpmEntry
//
// Description: Entry point for TPM initialization after memory is installed
//
//
// Input:       IN      EFI_FFS_FILE_HEADER       *FfsHeader
//              IN      EFI_PEI_SERVICES          **PeiServices
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI InitLegacyTpmEntry(
IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi)
{
    UINT8                  FuncID;
    TPMTransmitEntryStruct EmptyBuf;
    UINT32                 Ret;

    FuncID  = MA_FUNCTION_INIT;
    TpmPeiCallMPDriver( PeiServices, FuncID, &EmptyBuf, &Ret );
    if ( !Ret )
    {
        return EFI_SUCCESS;
    }
    return EFI_DEVICE_ERROR;
}
#endif


static EFI_PEI_PPI_DESCRIPTOR mPpiList[] = {
    EFI_PEI_PPI_DESCRIPTOR_PPI
    | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gPeiTpmPpiGuid,
    &mTpmPrivate.TpmPpi
};



EFI_STATUS
EFIAPI TpmPeiEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS Status;
    BOOLEAN    Temp = TRUE;
#if TCG_LEGACY == 1
    TPM_PEI_CALLBACK    *Callback;
#endif

    (*PeiServices)->InstallPpi( PeiServices, mPpiList );

#if TCG_LEGACY == 1
     Status = (**PeiServices).AllocatePool(
                               PeiServices,
                               sizeof (TPM_PEI_CALLBACK),
                               &Callback);

    if ( !EFI_ERROR( Status ))
    {
        Callback->NotifyDesc.Flags
            = (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK
               | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
        Callback->NotifyDesc.Guid   = &gTpmCallbackguid;
        Callback->NotifyDesc.Notify = InitLegacyTpmEntry;
        Callback->FfsHeader         = FfsHeader;

        Status = (*PeiServices)->NotifyPpi( PeiServices,
                                  &Callback->NotifyDesc );
    }
    return EFI_SUCCESS;
#endif

    if ( *((UINT16*)(UINTN)( TPM_BASE_ADDRESS + TCG_VID_OFFSET )) ==
         (UINT16)INTEL_VID )
    {
        Temp = iTPMVerifyMeStatus( );
    }

    if ( Temp )
    {
        Status = IsTpmPresent((TPM_1_2_REGISTERS_PTR)(
                                  UINTN ) mTpmPrivate.BaseAddr );

        if ( EFI_ERROR( Status ))
        {
            return EFI_UNLOAD_IMAGE;
        }
    }
    return Status;
}
