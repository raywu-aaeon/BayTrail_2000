//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/xTcgPei.c 35    3/19/12 6:27p Fredericko $
//
// $Revision: 35 $
//
// $Date: 3/19/12 6:27p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  xTcgPei.c
//
// Description:
//        Contians entry point function for TcgPei Subcomponent
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <Pei.h>
#include <AmiTcg\TcgCommon.h>
#include <AmiPeiLib.h>
#include <AmiTcg\TcgMisc.h>
#include <ppi\AmiTcgPlatformPpi.h>
#include <Ppi/TcgService.h>
#include <Ppi/TpmDevice.h>
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include <Ppi/TcgPlatformSetupPeiPolicy.h>



EFI_STATUS
EFIAPI TpmPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );

EFI_STATUS
EFIAPI TcmPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );


EFI_STATUS
EFIAPI TcgPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );

EFI_STATUS
EFIAPI TcgTcmPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );

EFI_STATUS TcgPeiBuildHobGuid(
    IN EFI_PEI_SERVICES **PeiServices,
    IN EFI_GUID         *Guid,
    IN UINTN            DataLength,
    OUT VOID            **Hob );

UINT8 GetPlatformSupportType()
{
   return (AutoSupportType());
}

static AMI_TCG_PLATFORM_PPI PlatformTypePpi = {
    GetPlatformSupportType
};

static EFI_PEI_PPI_DESCRIPTOR mPlatformPpiList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI
        | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gAmiPlatformSecurityChipGuid,
        &PlatformTypePpi
    }
};


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   CommonTcgPeiEntryPoint
//
// Description: Entry point for Tcg PEI component
//
//
// Input:       IN      EFI_FFS_FILE_HEADER       *FfsHeader
//              IN      EFI_PEI_SERVICES          **PeiServices,
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
EFIAPI CommonTcgPeiEntryPoint(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS                     Status;
    VOID                           *Context = NULL;
    BOOLEAN                        DeviceType = FALSE;
    TCG_PLATFORM_SETUP_INTERFACE   *TcgPeiPolicy = NULL;
    EFI_GUID                        gTcgPeiPolicyGuid =\
                                        TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    TCG_CONFIGURATION              ConfigFlags;
    CHAR16                         Monotonic[] = L"MonotonicCounter";
    EFI_GUID                       Guid        = AMI_GLOBAL_VARIABLE_GUID;
    EFI_GUID                       TcgGuid     = AMI_TCG_RESETVAR_HOB_GUID;
    UINTN                          Size        = sizeof(UINT32);
    UINT32                         Counter;
    EFI_PEI_READ_ONLY_VARIABLE_PPI *ReadOnlyVariable;
    EFI_HOB_GUID_TYPE               *Hob;
    BOOLEAN                         ResetAllTcgVar = FALSE;
    EFI_GUID                        gTcgReadOnlyVariablePpiGuid
                                        = EFI_TCG_PEI_READ_ONLY_VARIABLE_PPI_GUID;


    Status = (*PeiServices)->InstallPpi( PeiServices, &mPlatformPpiList[0] );
    if ( EFI_ERROR( Status ))
    {
        return EFI_UNLOAD_IMAGE;
    }

    Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gTcgPeiPolicyGuid,
                0, NULL,
                &TcgPeiPolicy);

    if(EFI_ERROR(Status) || TcgPeiPolicy == NULL )return Status;

    Status = (*PeiServices)->LocatePpi(
        PeiServices,
        &gTcgReadOnlyVariablePpiGuid,
        0, NULL,
        &ReadOnlyVariable
        );

    if(EFI_ERROR(Status) || ReadOnlyVariable == NULL )return Status;

    Status = ReadOnlyVariable->PeiGetVariable( PeiServices, Monotonic, &Guid,
                                            NULL, &Size, &Counter );

    if ( EFI_ERROR( Status ))
    {

        ResetAllTcgVar = TRUE;
        Status         = TcgPeiBuildHobGuid(
            PeiServices,
            &TcgGuid,
            sizeof (BOOLEAN),
            &Hob );
    
        Hob++;
        (*PeiServices)->CopyMem( Hob, &ResetAllTcgVar, sizeof (ResetAllTcgVar));
    }

    if(!AutoSupportType()){
       Status = TpmPeiEntry( FfsHeader, PeiServices );
       if ( EFI_ERROR( Status )){
       return Status;}
    }else{
       Status = TcmPeiEntry( FfsHeader, PeiServices );
       if ( EFI_ERROR( Status )){
       return Status;}
    }

    Status = TcgPeiPolicy->getTcgPeiPolicy(PeiServices, &ConfigFlags);

    if ( ConfigFlags.TpmSupport == 0x00  || EFI_ERROR( Status ))
    {
        return EFI_SUCCESS;
    }

    if(!AutoSupportType()){
       Status = TcgPeiEntry( FfsHeader, PeiServices );
    }else{
       Status = TcgTcmPeiEntry( FfsHeader, PeiServices );
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
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
