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
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgPeiplatform/TcgPeiplatform.c 5     4/26/11 1:46p Fredericko $
//
// $Revision: 5 $
//
// $Date: 4/26/11 1:46p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	TcgPeiPlatform.c
//
// Description:	Function file for TcgPeiPlatform
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <Pei.h>
#include <AmiTcg\TcgCommon.h>
#include <AmiPeiLib.h>
#include <AmiTcg\TcgMisc.h>
#include <PPI/TcgTcmService.h>
#include <Ppi/TcgService.h>
#include <Ppi/TpmDevice.h>
#include <Ppi/AmiTcgPlatformPpi.h>
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include <Ppi\ReadOnlyVariable.h>
#include < AmiTcg\AmiTcgPlatformPei.h>




EFI_STATUS
EFIAPI OnMemoryDiscovered(
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi );


static EFI_PEI_NOTIFY_DESCRIPTOR TcgAmiPlatformInitNotify[] =
{
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gAmiTcgPlatformPpiAfterMem,
        OnMemoryDiscovered
    }
};

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
EFIAPI OnMemoryDiscovered(
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi )
{
    EFI_STATUS                 Status;
    AMI_TCG_PLATFORM_PPI_AFTER_MEM    *AmiTcgPlatformPPI = NULL;
    EFI_BOOT_MODE              BootMode;


    Status =  (*PeiServices)->LocatePpi (
                 PeiServices,
                 &gAmiTcgPlatformPpiAfterMem ,
                 0,
                 NULL,
                 &AmiTcgPlatformPPI);

    if ( EFI_ERROR( Status )){
        Status = (*PeiServices)->NotifyPpi (PeiServices, \
                                   TcgAmiPlatformInitNotify);

        return Status;
    }
 
    ASSERT_PEI_ERROR( PeiServices, Status );

    Status = (*PeiServices)->GetBootMode( PeiServices, &BootMode );
    ASSERT_PEI_ERROR( PeiServices, Status );
    
    Status = AmiTcgPlatformPPI->VerifyTcgVariables(PeiServices);
    Status = AmiTcgPlatformPPI->MemoryPresentFunctioOverride(PeiServices);
    if(EFI_ERROR(Status))return Status;
    
    if((BootMode == BOOT_ON_S3_RESUME) || (BootMode == BOOT_IN_RECOVERY_MODE)){
        return EFI_SUCCESS;
    }

    Status = AmiTcgPlatformPPI->SetPhysicalPresence(PeiServices);
    return (Status);
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgPeiPlatformEntry
//
// Description: 
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
EFIAPI TcgPeiplatformEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices ){

    AMI_TCG_PLATFORM_PPI_BEFORE_MEM     *AmiTcgPlatformPPI = NULL;
    VOID			*Dummy = NULL;
    EFI_STATUS                 Status;
    TCG_PEI_MEMORY_CALLBACK    *MemCallback;

    Status =  (*PeiServices)->LocatePpi (
                 PeiServices,
                 &gAmiTcgPlatformPpiBeforeMem,
                 0,
                NULL,
                &AmiTcgPlatformPPI);
 
  
    if(EFI_ERROR(Status)){
        return EFI_SUCCESS;
    }   

    AmiTcgPlatformPPI->MemoryAbsentFunctionOverride(PeiServices);

    Status =  (*PeiServices)->LocatePpi (
                 PeiServices,
                 &gEfiPeiMemoryDiscoveredPpiGuid,
                 0,
                NULL,
                &Dummy);

    if(Status)
    {
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
    				MemCallback->NotifyDesc.Notify = OnMemoryDiscovered;
    				MemCallback->FfsHeader         = FfsHeader;

    				Status = (*PeiServices)->NotifyPpi( PeiServices,
    							&MemCallback->NotifyDesc );
    		}
    }else{
    	OnMemoryDiscovered(PeiServices, NULL ,NULL);
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
