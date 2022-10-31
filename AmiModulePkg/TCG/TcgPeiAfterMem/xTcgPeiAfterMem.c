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
// $Header: /Alaska/SOURCE/Modules/TCG/xTcgPeiAfterMem.c 4     12/12/11 3:31p Fredericko $
//
// $Revision: 4 $
//
// $Date: 12/12/11 3:31p $
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
#include "PPI\TcgService.h"
#include "PPI\TpmDevice.h"
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include "PPI\TcgPlatformSetupPeiPolicy.h"



EFI_STATUS
EFIAPI TcgPeiMemoryCallbackEntry(
    IN EFI_PEI_SERVICES    **PeiServices 
);

EFI_STATUS
EFIAPI TcgTcmPeiMemoryCallbackEntry(
    IN EFI_PEI_SERVICES    **PeiServices 
);


typedef struct _TCG_PEI_MEMORY_CALLBACK
{
    EFI_PEI_NOTIFY_DESCRIPTOR NotifyDesc;
    EFI_FFS_FILE_HEADER       *FfsHeader;
} TCG_PEI_MEMORY_CALLBACK;


EFI_STATUS
EFIAPI  TcgPeiMemoryEntry(
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi );


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   OnMemoryDiscovered
//
// Description: Call Memory Present initialization on memory Installation
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//              IN      EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
//              IN      VOID                      *Ppi
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
EFIAPI TcgPeiMemoryEntry(
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi )
{
    EFI_STATUS                 Status;
   PEI_TCG_PPI                 *TcgPpi = NULL;

    Status =  (*PeiServices)->LocatePpi (
                 PeiServices,
                 &gPeiTcgPpiGuid ,
                 0,
                 NULL,
                 &TcgPpi);

    if ( EFI_ERROR( Status )){
        return Status;
    }
    
    if(AutoSupportType())
    {
        TcgTcmPeiMemoryCallbackEntry(PeiServices);
    }else{
        TcgPeiMemoryCallbackEntry(PeiServices);
    }

    return (Status);
}


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
EFIAPI ReInstallTcgServiceAfterMem(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS                     Status;
    TCG_PLATFORM_SETUP_INTERFACE   *TcgPeiPolicy = NULL;
    EFI_GUID                        gTcgPeiPolicyGuid =\
                                        TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    TCG_PEI_MEMORY_CALLBACK         *MemCallback;
    TCG_CONFIGURATION              ConfigFlags;

    Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gTcgPeiPolicyGuid,
                0, NULL,
                &TcgPeiPolicy);

    if(EFI_ERROR(Status))return Status;

    Status = TcgPeiPolicy->getTcgPeiPolicy(PeiServices, &ConfigFlags);

    if (ConfigFlags.TpmSupport == 0x00  || EFI_ERROR( Status ))
    {
        return EFI_SUCCESS;
    }


    Status = (**PeiServices).AllocatePool(
                    PeiServices,
                    sizeof (TCG_PEI_MEMORY_CALLBACK),
                    &MemCallback);

    if ( !EFI_ERROR( Status ))
    {
        MemCallback->NotifyDesc.Flags
            = (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK
               | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
        MemCallback->NotifyDesc.Guid   = &gEfiPeiMemoryDiscoveredPpiGuid;
        MemCallback->NotifyDesc.Notify = TcgPeiMemoryEntry;
        MemCallback->FfsHeader         = FfsHeader;

        Status = (*PeiServices)->NotifyPpi( PeiServices,
                                  &MemCallback->NotifyDesc );
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
