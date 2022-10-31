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
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgPlatform/AmiTcgPlatformPei/AmiTcgPlatformPeiBeforeMem.c 8     4/27/12 6:18p Fredericko $
//
// $Revision: 8 $
//
// $Date: 4/27/12 6:18p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiTcgPlatformPeiBeforeMem.c
//
// Description:	Function file for AmiTcgPlatformPeiBeforeMem
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
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include <Ppi\ReadOnlyVariable.h>
#include <Ppi\AmiTcgPlatformPpi.h>
#include <AmiTcg\AmiTcgPlatformPei.h>


EFI_GUID  gTpmInitializedguid = PEI_TPM_INITIALIZED_PPI_GUID;

static EFI_PEI_PPI_DESCRIPTOR Tpm_Initialized[] =
{ 
	{
		EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
		&gTpmInitializedguid, NULL 
	}
};


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   MemoryAbsentEntry
//
// Description: This function performs TPM MA initialization
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
MemoryAbsentEntry(
    IN EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS              Status;
    EFI_BOOT_MODE           BootMode;
    void                    *TcgDrvBuffer = NULL;
    UINTN                   Pages         = 0;
    EFI_GUID                guid          = EFI_TCG_MADriver_GUID;
    EFI_HOB_GUID_TYPE       *MAHobType;
    MASTRUCT                MAHob;
    FAR32LOCALS             CommonLegX;
    EFI_GUID                gEfiTcgMADriverHobGuid = EFI_TCG_MADriver_HOB_GUID;
    AMI_TCG_PEI_FUNCTION_OVERRIDE_PPI       *PpiOverride;
    EFI_GUID                Overrideguid = AMI_MEMORY_ABSENT_OVERRIDE_GUID;
    EFI_GUID                SkipTpmStartupGuid = AMI_SKIP_TPM_STARTUP_GUID;
    BOOLEAN                 SkipTpmStartup = FALSE;

    Status = (*PeiServices)->LocatePpi(
                    PeiServices,
                    &SkipTpmStartupGuid,
                    0, NULL,
                    &PpiOverride);

    if(!EFI_ERROR(Status)) {
        SkipTpmStartup = TRUE;
    }

    Status = (*PeiServices)->LocatePpi(
                    PeiServices,
                    &Overrideguid,
                    0, NULL,
                    &PpiOverride);

    if(!EFI_ERROR(Status)){
        return (PpiOverride->Function(PeiServices));
    }

    Status = (*PeiServices)->GetBootMode( PeiServices, &BootMode );
    ASSERT_PEI_ERROR( PeiServices, Status );

#if (StartupCmd_SelfTest_State == 1)
        if(!AutoSupportType()){
            if (!SkipTpmStartup) {
              Status = TcgPeiTpmStartup( PeiServices, BootMode );
              ASSERT_PEI_ERROR( PeiServices, Status );
   	        }

            Status = ContinueTPMSelfTest( PeiServices );
            ASSERT_PEI_ERROR( PeiServices, Status );

            if(!EFI_ERROR(Status)){       
              	(*PeiServices)->InstallPpi(PeiServices, Tpm_Initialized);
            }

            if ((BootMode == BOOT_ON_S3_RESUME) || (BootMode == BOOT_IN_RECOVERY_MODE))
            {
                return Status;
            }
        }
#else
        if ((BootMode == BOOT_ON_S3_RESUME) || (BootMode == BOOT_IN_RECOVERY_MODE))
        {
          return Status;
        }
#endif

    if(AutoSupportType()){
        Status = FillDriverLocByFile(&CommonLegX.Offset,PeiServices,&guid,&TcgDrvBuffer,&Pages);
        if(EFI_ERROR(Status))return EFI_NOT_FOUND;
        if ( CommonLegX.Offset == 0 )
        {
            return EFI_NOT_FOUND;
        }

        MAHob.Offset   = CommonLegX.Offset;
        MAHob.Selector = SEL_flatCS;
        MAHob.Codep    = ((MPDRIVER_LEGHEADER*)((UINT8*)(TcgDrvBuffer)))->CodeP;

        Status = TcgPeiBuildHobGuid(
            PeiServices,
            &gEfiTcgMADriverHobGuid,
            sizeof (MASTRUCT),
            &MAHobType );

        ASSERT_PEI_ERROR( PeiServices, Status );
        MAHobType++;

        (*PeiServices)->CopyMem( MAHobType, &MAHob, sizeof (MASTRUCT));
        return Status;
    }

    return EFI_SUCCESS;
}




static AMI_TCG_PLATFORM_PPI_BEFORE_MEM  mAmiTcgPlatformPPI = {
    MemoryAbsentEntry,
};


static EFI_PEI_PPI_DESCRIPTOR mAmiTcgPlatformPPIListBeforeMem[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI
        | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gAmiTcgPlatformPpiBeforeMem ,
        &mAmiTcgPlatformPPI
    }
};




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   AmiTcgPlatformPEI_EntryBeforeMem
//
// Description: Installs AMIplatform PPI for initialization in PEI before 
//              memory is installed
//
// Input:        IN EFI_FFS_FILE_HEADER *FfsHeader,
//               IN EFI_PEI_SERVICES    **PeiServices
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
EFIAPI AmiTcgPlatformPEI_EntryBeforeMem(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices 
){
    EFI_STATUS Status;

    Status = (*PeiServices)->InstallPpi( PeiServices, mAmiTcgPlatformPPIListBeforeMem );
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
