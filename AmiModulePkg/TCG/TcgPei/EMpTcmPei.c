//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/EMpTcmPei.c 8     5/09/12 6:13p Fredericko $
//
// $Revision: 8 $
//
// $Date: 5/09/12 6:13p $
//**********************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name: EMpTpmPei.c
//
// Description:
// Handles legacy calls into 32bit Binaries
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <Pei.h>
#include <AmiTcg\TcgTpm12.h>
#include <AmiTcg\TpmLib.h>
#include <AmiPeiLib.h>
#include "AmiTcg\TcgMisc.h"
#include "token.h"
#include "AmiTcg\TcgCommon.h"
#include <Ppi/TcgService.h>
#include <Ppi/TpmDevice.h>


#define _CR( Record, TYPE,\
        Field )((TYPE*) ((CHAR8*) (Record)- (CHAR8*) &(((TYPE*) 0)->Field)))

#define TPM_PEI_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TPM_PEI_PRIVATE_DATA, TpmPpi )

typedef struct _TPM_PEI_PRIVATE_DATA
{
    PEI_TPM_PPI TpmPpi;
} TPM_PEI_PRIVATE_DATA;

EFI_GUID  gTcmCallbackguid = AMI_TCM_CALLBACK_GUID;
EFI_GUID      guid     = EFI_TCG_MPDriver_HOB_GUID;

typedef struct _TCM_PEI_CALLBACK
{
    EFI_PEI_NOTIFY_DESCRIPTOR NotifyDesc;
    EFI_FFS_FILE_HEADER       *FfsHeader;

} TCM_PEI_CALLBACK;


VOID TcmPeiCallMPDriver (
    IN EFI_PEI_SERVICES    **PeiServices,
    IN UINT8               CFuncID,
    TPMTransmitEntryStruct *CData,
    UINT32* OUT            CRetVal );



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTcmPeiInit
//
// Description: Call to OEM driver to initialize TCM
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
EFIAPI EMpTcmPeiInit(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI EMpTcmPeiInit2(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
     return EFI_NOT_AVAILABLE_YET;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTcmPeiClose
//
// Description: Call to OEM driver to Close TCM
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
EFIAPI EMpTcmPeiClose(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
      return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI EMpTcmPeiClose2(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
       return EFI_NOT_AVAILABLE_YET;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTcmPeiGetStatusInfo
//
// Description: Function call to get status infomation
//
//
// Input:       IN  PEI_TPM_PPI               *This,
//              IN  EFI_PEI_SERVICES          **PeiServices
//
// Output:      EFI_UNSUPPORTED
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI EMpTcmPeiGetStatusInfo(
    IN PEI_TPM_PPI      *This,
    IN EFI_PEI_SERVICES **PeiServices )
{
    return EFI_UNSUPPORTED;
}






//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTcmPeiSHA1Setup
//
// Description: Helper function to help with Command transmission to TCM
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
EFIAPI EMpTcmPeiSHA1Setup(
    IN PEI_TPM_PPI         *This,
    IN EFI_PEI_SERVICES    **PeiServices,
    IN UINTN               NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER *InputBuffers,
    IN OUT void            **Buffer,
    IN UINT8               *SHA_ARRAY )
{
    TPM_1_2_CMD_HEADER *SHA1HeaderCheck;
    void               *Context = NULL;


    //check if this an SHA1 update or Complete call and set the buffer
    //else no need to update the buffer is the same
    SHA1HeaderCheck = (TPM_1_2_CMD_HEADER*)InputBuffers->Buffer;

    if ( SHA1HeaderCheck->Ordinal !=
         TPM_H2NL(
                   TCM_ORD_SHA1CompleteExtend ) && SHA1HeaderCheck->Ordinal !=
         TPM_H2NL( TCM_ORD_SHA1Update ))
    {
        //no need to modify buffers just pass it through
        return FALSE;
    }


    TcgCommonCopyMem( Context,
                      SHA_ARRAY,
                      InputBuffers[0].Buffer,
                      InputBuffers[0].Size );

    TcgCommonCopyMem( Context,
                      &SHA_ARRAY[InputBuffers[0].Size],
                      InputBuffers[1].Buffer,

                      InputBuffers[1].Size );

    *Buffer =  SHA_ARRAY;
    return TRUE;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EMpTcmPeiTransmit
//
// Description: TCM Transmit function
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
EFIAPI EMpTcmPeiTransmit(
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
    TPM_1_2_CMD_HEADER     *SHA1HeaderCheck;
    void                   *Context = NULL;
    UINT8                  i = 0;
    UINT8                  loc = 0;

    if ( !EMpTcmPeiSHA1Setup( This, PeiServices, NoInputBuffers, InputBuffers,
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

        SHA1HeaderCheck = (TPM_1_2_CMD_HEADER*)InputBuffers->Buffer;

        if ( SHA1HeaderCheck->Ordinal == TPM_H2NL( TCM_ORD_SHA1CompleteExtend ))
        {
            FillBuff         = TRUE;
            FillESI.pbOutBuf = (UINT32)(EFI_PHYSICAL_ADDRESS)SHA_ARRAY_OUT;
            FillESI.dwOutLen = (OutputBuffers[0].Size + OutputBuffers[1].Size 
                               + OutputBuffers[2].Size);
        }

        if ( !FillBuff )
        {
            FillESI.pbOutBuf = (UINT32)(EFI_PHYSICAL_ADDRESS)OutputBuffers->Buffer;
            FillESI.dwOutLen = (UINT32)OutputBuffers->Size;
        }
    }

    Private = TPM_PEI_PRIVATE_DATA_FROM_THIS( This );
    TcmPeiCallMPDriver( PeiServices, FuncID, &FillESI, &Ret );

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


EFI_STATUS
EFIAPI EMpTcmPeiTransmit2(
    IN PEI_TPM_PPI             *This,
    IN EFI_PEI_SERVICES        **PeiServices,
    IN UINTN                   NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINTN                   NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    return EFI_NOT_AVAILABLE_YET;
}






//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmPeiCallMPDriver
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
VOID TcmPeiCallMPDriver(
    IN EFI_PEI_SERVICES    **PeiServices,
    IN UINT8               CFuncID,
    TPMTransmitEntryStruct * CData,
    UINT32* OUT            CRetVal )
{
    UINT32                 ReturnVal;
    FAR32LOCALS            CommonLegX;
    EFI_GUID               guidMA = EFI_TCG_MADriver_HOB_GUID;
    EFI_GUID               guidMP = EFI_TCG_MPDriver_HOB_GUID;
    TPMTransmitEntryStruct IntCData = *CData;
    UINT32                 Address  = (UINT32)&IntCData;
    FAR32LOCALS            InternalMPLoc;

    
    FillDriverLoc( &InternalMPLoc.Offset, PeiServices, &guid );

    CommonLegX.Offset   = 0;
    CommonLegX.Selector = 0;
    CommonLegX.Offset   = InternalMPLoc.Offset;
    CommonLegX.Selector = SEL_flatCS;

    _asm {
        xor   eax, eax
        mov   al, [CFuncID]
        cmp   eax, MP_FUNCTION_TRANSMIT
        jnz   Empty_Buff
        mov   ESI, Address
Empty_Buff:
        call fword ptr [CommonLegX];
        mov  ReturnVal, eax
    }
    *CRetVal = ReturnVal;
}


static TPM_PEI_PRIVATE_DATA   mTcmPrivate = {
    {
        EMpTcmPeiInit,
        EMpTcmPeiClose,
        EMpTcmPeiGetStatusInfo,
        EMpTcmPeiTransmit
    }
};

static TPM_PEI_PRIVATE_DATA   mTcmPrivate2 = {
    {
        EMpTcmPeiInit2,
        EMpTcmPeiClose2,
        EMpTcmPeiGetStatusInfo,
        EMpTcmPeiTransmit2
    }
};

static EFI_PEI_PPI_DESCRIPTOR mPpiList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI
        | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gPeiTpmPpiGuid,
        &mTcmPrivate.TpmPpi
    }
};

static EFI_PEI_PPI_DESCRIPTOR mPpiList2[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI
        | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gPeiTpmPpiGuid,
        &mTcmPrivate2.TpmPpi
    }
};



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InitTcmEntry
//
// Description: Entry point for TCM ppi initialization after memory is installed
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
EFIAPI InitTcmEntry(
IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi)
{
    EFI_STATUS              Status;
    EFI_PHYSICAL_ADDRESS    TPM_Base = (EFI_PHYSICAL_ADDRESS)PORT_TPM_IOMEMBASE;
    FAR32LOCALS             InternalMPLoc;
    EFI_PEI_PPI_DESCRIPTOR  *PpiDesc;
    PEI_TCG_PPI             *TcmPpi;

    Status = IsTpmPresent((TPM_1_2_REGISTERS_PTR)( UINTN ) TPM_Base );
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    FillDriverLoc( &InternalMPLoc.Offset, PeiServices, &guid );
    if ( InternalMPLoc.Offset == 0 ) 
    {
        return EFI_NOT_FOUND;
    }

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gPeiTpmPpiGuid,
                            0,
                            &PpiDesc,
                            &TcmPpi);
    if(EFI_ERROR(Status))
    {
        return Status;
    }

    
    Status = (*PeiServices)->ReInstallPpi( PeiServices, PpiDesc, &mPpiList[0] );
    if ( EFI_ERROR( Status ))
    {
        return EFI_UNLOAD_IMAGE;
    }

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gPeiTpmPpiGuid,
                            0,
                            &PpiDesc,
                            &TcmPpi );

    if ( EFI_ERROR( Status ))
    {
        return EFI_UNLOAD_IMAGE;
    }

    return Status;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmPeiEntry
//
// Description: Entry for TcmPei Device when TCM_Support is enabled
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
EFIAPI TcmPeiEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS           Status;
    TCM_PEI_CALLBACK    *TcmCallback;

    Status = (**PeiServices).AllocatePool(
                               PeiServices,
                               sizeof (TCM_PEI_CALLBACK),
                               &TcmCallback);

    if ( !EFI_ERROR( Status ))
    {
        TcmCallback->NotifyDesc.Flags
            = (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK
               | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
        TcmCallback->NotifyDesc.Guid   = &gTcmCallbackguid;
        TcmCallback->NotifyDesc.Notify = InitTcmEntry;
        TcmCallback->FfsHeader         = FfsHeader;

        Status = (*PeiServices)->NotifyPpi( PeiServices,
                                  &TcmCallback->NotifyDesc );
    }

    Status = (*PeiServices)->InstallPpi( PeiServices, &mPpiList2[0] );
    if ( EFI_ERROR( Status ))
    {
        return EFI_UNLOAD_IMAGE;
    }

    return Status;
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
