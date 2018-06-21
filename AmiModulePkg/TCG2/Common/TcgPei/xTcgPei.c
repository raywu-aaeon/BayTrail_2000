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
// $Header: /Alaska/SOURCE/Modules/TcgNext/Common/TcgPei/xTcgPei.c 1     10/08/13 12:02p Fredericko $
//
// $Revision: 1 $
//
// $Date: 10/08/13 12:02p $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/TcgNext/Common/TcgPei/xTcgPei.c $
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
#include <EFI.h>
#include <AmiTcg\TcgCommon.h>
#include <AmiTcg\TcgMisc.h>
#include <ppi\AmiTcgPlatformPpi.h>
#include "PPI\TcgService.h"
#include "PPI\TpmDevice.h"
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include "PPI\TcgPlatformSetupPeiPolicy.h"
#include <AmiTcg\Tpm20.h>
#include <Library\DebugLib.h>


EFI_GUID        gTcgPpiGuid = PEI_TCG_PPI_GUID;
EFI_GUID        gPlatformGuid = AMI_TCG_PLATFORM_PPI_GUID;

#define AMI_GLOBAL_VARIABLE_GUID \
    {0x1368881,0xc4ad,0x4b1d,0xb6,0x31,0xd5,0x7a,0x8e,0xc8,0xdb,0x6b}


EFI_STATUS
EFIAPI TpmPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices );

EFI_STATUS
EFIAPI TcmPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices );


EFI_STATUS
EFIAPI TcgPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices );

EFI_STATUS
EFIAPI TcgTcmPeiEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices );

EFI_STATUS TcgPeiBuildHobGuid(
    IN CONST  EFI_PEI_SERVICES **PeiServices,
    IN EFI_GUID         *Guid,
    IN UINTN            DataLength,
    OUT VOID            **Hob );

EFI_STATUS
EFIAPI Tpm20CrbEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices );

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
        &gPlatformGuid,
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
CommonTcgPeiEntryPoint(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS                     Status;
    VOID                           *Context = NULL;
    BOOLEAN                        DeviceType = FALSE;
    TCG_PLATFORM_SETUP_INTERFACE   *TcgPeiPolicy = NULL;
    EFI_GUID                        gTcgPeiPolicyGuid =\
                                        TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    TCG_CONFIGURATION              ConfigFlags;
    CHAR16                         Monotonic[] = L"MonotonicCounter";
#if defined(CORE_COMBINED_VERSION) && (CORE_COMBINED_VERSION > 262797)
    EFI_GUID                       Guid        = AMI_GLOBAL_VARIABLE_GUID;
#else
    EFI_GUID                       Guid        = EFI_GLOBAL_VARIABLE;
#endif
    EFI_GUID                       TcgGuid     = AMI_TCG_RESETVAR_HOB_GUID;
    UINTN                          Size        = sizeof(UINT32);
    UINT32                         Counter;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI  *ReadOnlyVariable;
    EFI_HOB_GUID_TYPE               *Hob;
    BOOLEAN                         ResetAllTcgVar = FALSE;


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
    
    Status = TcgPeiPolicy->getTcgPeiPolicy( (EFI_PEI_SERVICES    **)PeiServices, &ConfigFlags);
    
    if(ConfigFlags.DeviceType == 1)return EFI_SUCCESS;

    
    Status = (*PeiServices)->LocatePpi(
        PeiServices,
        &gEfiPeiReadOnlyVariable2PpiGuid,
        0, NULL,
        &ReadOnlyVariable
        );

    if(EFI_ERROR(Status) || ReadOnlyVariable == NULL )return Status;

    Status = ReadOnlyVariable->GetVariable(ReadOnlyVariable, Monotonic, &Guid,
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
       DEBUG((-1, "TpmPeiEntry results = %r \n", Status)); 
       if ( EFI_ERROR( Status )){ 
             return Status; 
       }
    }else{
       Status = TcmPeiEntry( FfsHeader, PeiServices );
       if ( EFI_ERROR( Status )){
       return Status;}
    }

    if ( ConfigFlags.TpmSupport == 0x00  || EFI_ERROR( Status ))
    {
        DEBUG((-1, "ConfigFlags.TpmSupport == 0x00  || EFI_ERROR( Status )\n")); 
        return EFI_SUCCESS;
    }


    DEBUG((-1,  "TcgPeiEntry processing\n")); 
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
