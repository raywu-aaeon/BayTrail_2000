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
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgPlatform/AmiTcgPlatformPei/AmiTcgPlatformPeiAfterMem.c 17    4/27/12 6:19p Fredericko $
//
// $Revision: 17 $
//
// $Date: 4/27/12 6:19p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiTcgPlatformPeiAfterMem.c
//
// Description:	Function file for AmiTcgPlatformPeiAfterMem
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
#include <AmiTcg\AmiTcgPlatformPei.h>
#include <Ppi/AmiTcgPlatformPpi.h>



EFI_GUID gAmiTcmSignalguid              =  AMI_TCM_CALLBACK_GUID;
EFI_GUID gAmiLegacyTpmguid              =  AMI_TPM_LEGACY_GUID;

static EFI_PEI_PPI_DESCRIPTOR TcmInitPpi[] = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gAmiTcmSignalguid,
  NULL
};

static EFI_PEI_PPI_DESCRIPTOR LegacyTpmInitPpi[] = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gAmiLegacyTpmguid,
  NULL
};


#if TCG_LEGACY == 1
    EFI_STATUS Configure_Tpm_Chip( );
#endif

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   VerifyTcgVariables
//
// Description: Function to check whether we need to reset TCG variables
//
//
// Input:       EFI_PEI_SERVICES **PeiServices
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
EFI_STATUS VerifyTcgVariables(
    EFI_PEI_SERVICES **PeiServices )
{
    AMI_TCG_PEI_FUNCTION_OVERRIDE_PPI       *VerifyVarOverride;
    EFI_GUID                       VarOverrideguid = AMI_VERIFY_TCG_VARIABLES_GUID;
    EFI_STATUS						Status;

    
    Status = (*PeiServices)->LocatePpi(
                    PeiServices,
                    &VarOverrideguid,
                    0, NULL,
                    &VerifyVarOverride);

    if(!EFI_ERROR(Status)){
        return (VerifyVarOverride->Function(PeiServices));
    }

    return EFI_SUCCESS;
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   Set_TPMPhysicalPresence
//
// Description: Sets TPM physical Presence
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
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
EFI_STATUS Set_TPMPhysicalPresence(
    IN EFI_PEI_SERVICES **PeiServices )
{
    EFI_STATUS               Status;
    TPM_RSP_COMMAND_HDR      RspHdr;

    AMI_TCG_PEI_FUNCTION_OVERRIDE_PPI   *SetPhysicalOverride;
    EFI_GUID                            Overrideguid = AMI_SET_PHYSICAL_PRESENCE_GUID;
    struct
    {
        TPM_RQU_COMMAND_HDR CmdHdr;
        UINT8               Data[0x4];
    } cmd;

    UINT16        physical_CMD_on = TPM_H2NS(TPM_PHYSICAL_PRESENCE_CMD_ENABLE );
    UINT16        physical_on     = TPM_H2NS(TPM_PHYSICAL_PRESENCE_PRESENT );
    PEI_TPM_PPI   *TpmPpi         = NULL;         
    PEI_TCG_PPI   *TcgPpi         = NULL;


    Status = (*PeiServices)->LocatePpi(
                    PeiServices,
                    &Overrideguid,
                    0, NULL,
                    &SetPhysicalOverride);

    if(!EFI_ERROR(Status)){
        return (SetPhysicalOverride->Function(PeiServices));
    }

    cmd.CmdHdr.tag =     TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmd.CmdHdr.paramSize = TPM_H2NL((UINT32)( sizeof (cmd.CmdHdr)
                           + sizeof(TPM_PHYSICAL_PRESENCE)));

    cmd.CmdHdr.ordinal = TPM_H2NL(TSC_ORD_PhysicalPresence );

    if ( Lock_TPMPhysicalPresence( PeiServices ))
    {
        physical_on = TPM_H2NS( TPM_PHYSICAL_PRESENCE_LOCK );
    }else{
        if(*(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF00) == 0x15D1){
            Status = ContinueTPMSelfTest( PeiServices );
        }
    }

    Status = LocateTcgPpi(PeiServices, &TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    Status = TpmPpi->Init( TpmPpi, PeiServices );
    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    MemCpy( cmd.Data, &physical_CMD_on, sizeof(TPM_PHYSICAL_PRESENCE));

    Status =TcgPpi->TCGPassThroughToTpm(
        TcgPpi,
        PeiServices,
        (sizeof (cmd.CmdHdr) + sizeof(TPM_PHYSICAL_PRESENCE)),
        (UINT8*)&cmd,
        sizeof (RspHdr),
        (UINT8*)&RspHdr );

    MemCpy( cmd.Data, &physical_on, sizeof(TPM_PHYSICAL_PRESENCE));

    Status = TcgPpi->TCGPassThroughToTpm(
        TcgPpi,
        PeiServices,
        (sizeof (cmd.CmdHdr) + sizeof(TPM_PHYSICAL_PRESENCE)),
        (UINT8*)&cmd,
        sizeof (RspHdr),
        (UINT8*)&RspHdr );

    Status = TpmPpi->Close( TpmPpi, PeiServices );
    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    if ( RspHdr.returnCode != 0 )
    {
        return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   Set_TCMPhysicalPresence
//
// Description: Sets TCM physical Presence
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
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
EFI_STATUS Set_TCMPhysicalPresence(
    IN EFI_PEI_SERVICES **PeiServices )
{
    EFI_STATUS               Status;
    TPM_RSP_COMMAND_HDR      RspHdr;

    struct
    {
        TPM_RQU_COMMAND_HDR CmdHdr;
        UINT8               Data[0x4];
    } cmd;

    UINT16  physical_CMD_on = TPM_H2NS(TPM_PHYSICAL_PRESENCE_CMD_ENABLE );
    UINT16  physical_on     = TPM_H2NS(TPM_PHYSICAL_PRESENCE_PRESENT );
    PEI_TPM_PPI     *TpmPpi = NULL;         
    PEI_TCM_PPI     *TcgPpi = NULL;

    cmd.CmdHdr.tag =     TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmd.CmdHdr.paramSize = TPM_H2NL((UINT32)( sizeof (cmd.CmdHdr)
                           + sizeof(TPM_PHYSICAL_PRESENCE)));

    cmd.CmdHdr.ordinal = TPM_H2NL(TCM_TSC_ORD_PhysicalPresence );
    

    if ( Lock_TPMPhysicalPresence( PeiServices ))
    {
        physical_on = TPM_H2NS( TPM_PHYSICAL_PRESENCE_LOCK );
    }

    Status = LocateTcmPpi(PeiServices, &TpmPpi, &TcgPpi);
    ASSERT_PEI_ERROR( PeiServices, Status );

    MemCpy( cmd.Data, &physical_CMD_on, sizeof(TPM_PHYSICAL_PRESENCE));

    Status = TcgPpi->TCMPassThroughToTcm(
        TcgPpi,
        PeiServices,
        (sizeof (cmd.CmdHdr) + sizeof(TPM_PHYSICAL_PRESENCE)),
        (UINT8*)&cmd,
        sizeof (RspHdr),
        (UINT8*)&RspHdr );

    MemCpy( cmd.Data, &physical_on, sizeof(TPM_PHYSICAL_PRESENCE));

    Status = TcgPpi->TCMPassThroughToTcm(
        TcgPpi,
        PeiServices,
        (sizeof (cmd.CmdHdr) + sizeof(TPM_PHYSICAL_PRESENCE)),
        (UINT8*)&cmd,
        sizeof (RspHdr),
        (UINT8*)&RspHdr );

    if ( RspHdr.returnCode != 0 )
    {
        return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  TcgPeiGetRawImage
//
// Description:    Loads binary from RAW section of main firwmare volume
//
// Input:       IN EFI_PEI_SERVICES **PeiServices
//              IN OUT VOID         **Buffer
//              IN OUT UINT16       *size
//              IN     EFI_GUID     guid
//
// Output:        EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI TcgPeiGetRawImage(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT VOID         **Buffer,
    IN OUT UINT16       *size,
    EFI_GUID            guid )
{
    EFI_STATUS                 Status;
    EFI_FIRMWARE_VOLUME_HEADER *pFV;
    UINTN                      FvNum   = 0;
    EFI_FFS_FILE_HEADER        *ppFile = NULL;
    MPDRIVER_LEGHEADER         *Temp;
    BOOLEAN                    Found = FALSE;

    while ( TRUE )
    {
        Status = (*PeiServices)->FfsFindNextVolume( PeiServices, FvNum, &pFV );

        if ( EFI_ERROR( Status ))
        {
            return Status;
        }

        ppFile = NULL;

        while ( TRUE )
        {
            Status = (*PeiServices)->FfsFindNextFile( PeiServices,
            										  EFI_FV_FILETYPE_ALL,
                                                      pFV,
                                                      &ppFile );

            if ( Status == EFI_NOT_FOUND )
            {
                break;
            }

            if ( guidcmp( &ppFile->Name, &guid ) == 0 )
            {
                Found = TRUE;
                break;
            }
        }

        if ( Found )
        {
            break;
        }
        else {
            FvNum++;
        }
    }

    (*PeiServices)->FfsFindSectionData( PeiServices,
                                        EFI_SECTION_RAW,
                                        ppFile,
                                        Buffer );

    if ( Buffer == NULL )
    {
        return EFI_NOT_FOUND;
    }

    Temp  = ((MPDRIVER_LEGHEADER*)(((UINT8* )ppFile )+sizeof(EFI_FFS_FILE_HEADER)));
    *size = Temp->Size;
    
    (*PeiServices)->AllocatePool(PeiServices, *size,  Buffer );
    (*PeiServices)->CopyMem(*Buffer,( ((UINT8* )ppFile )+sizeof(EFI_FFS_FILE_HEADER) ), *size) ;

    return Status;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   MemoryPresentEntry
//
// Description: This function performs TPM MP initialization
//
//
// Input:       IN     EFI_PEI_SERVICES  **PeiServices,
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
EFIAPI MemoryPresentEntry(
    IN EFI_PEI_SERVICES **PeiServices )
{
    EFI_STATUS                     Status;
    EFI_HOB_GUID_TYPE              *Hob;
    TCG_LOG_HOB                    *TcgLog;
    PEI_TPM_PPI                    *TpmPpi = NULL;         
    PEI_TCG_PPI                    *TcgPpi = NULL;
    EFI_HOB_GUID_TYPE              *ptrBootMode;
    EFI_BOOT_MODE                  BootMode;
    EFI_HOB_GUID_TYPE              *MpHobType;
    EFI_PHYSICAL_ADDRESS           MPRuntime;
    void                           *TcgMPBuffer = NULL;
    FAR32LOCALS                    InitCall;
    void                           *MPRuntimePtr = NULL;
    UINT16                         Pages      = 0;
    UINT32                         Offset     = 0;
    UINT8                          FuncID     = 1;
    void                           *ParamIN   = NULL;
    void                           *ParamOut  = NULL;
    UINT32                         RetVal     = 0;
    EFI_FFS_FILE_HEADER            *FfsHeader = NULL;
    EFI_PEI_CPU_IO_PPI             *CpuIo      = (*PeiServices)->CpuIo;
    UINT8                          MPFILEERROR = 0xFA;
    BOOLEAN                        ResetMor = FALSE;
#if TCG_LEGACY == 1
    BOOLEAN			   TpmLegBin = TRUE;
    EFI_GUID                       MpFileGuid  = EFI_TCG_MPDriver_GUID;
#else
    BOOLEAN			   TpmLegBin = FALSE;
    EFI_GUID                       MpFileGuid  = EFI_TCM_MPDriver_GUID;
#endif

    AMI_TCG_PEI_FUNCTION_OVERRIDE_PPI       *MpOverride;
    EFI_GUID                Overrideguid =  AMI_MEMORY_PRESENT_FUNCTION_OVERRIDE_GUID;

    EFI_GUID gPeiEfiAmiTcgWakeEventDataHobGuid =  \
                                EFI_TCG_WAKE_EVENT_DATA_HOB_GUID;
    EFI_GUID gEfiPeiAmiTcgLogHobGuid           = EFI_TCG_LOG_HOB_GUID;
    EFI_GUID gEfiTcgCapHobGuid              =  EFI_TCG_CAP_HOB_GUID;
    EFI_GUID gEfiTcgMpDriverHobGuid = EFI_TCG_MPDriver_HOB_GUID;


    Status = (*PeiServices)->LocatePpi(
                                PeiServices,
                                &Overrideguid,
                                0, NULL,
                                &MpOverride);

    if(!EFI_ERROR(Status)){
        return (MpOverride->Function(PeiServices));
    }

#if TCG_LEGACY == 1
    Status = Configure_Tpm_Chip( );
    if ( EFI_ERROR( Status ))
    {
         PEI_TRACE((-1, PeiServices,
        "Device not configured for legacy IO aborting TPM initialization\n"));
        return Status;
    }
#endif

    Status = (*PeiServices)->GetBootMode( PeiServices, &BootMode );
    ASSERT_PEI_ERROR( PeiServices, Status );

    if((AutoSupportType()) || (TpmLegBin == TRUE)){

        PEI_TRACE((-1, PeiServices,"Setting up Binary Images\n"));

        Status = TcgPeiGetRawImage( PeiServices, &TcgMPBuffer, &Pages, MpFileGuid );

        if ( TcgMPBuffer == NULL )
        {
            PEI_TRACE((-1, PeiServices,
            "Unable to Find TCM OEM MPDriver!!! Please make sure TCM porting is done correctly\n"));
            PEI_TRACE((-1, PeiServices,"Unrecoverable Error. HALTING SYSTEM\n"));
            CpuIo->Io.Write( PeiServices, CpuIo, 0, 0x80, 1, &MPFILEERROR );
             while ( 1 )
            {
                ;
            }
        }   

        (*PeiServices)->AllocatePages( PeiServices,
                                   EfiRuntimeServicesCode,
                                   (UINTN)((Pages / 4096)+1),
                                   &MPRuntime );

        MPRuntimePtr = (void*)MPRuntime;
        MemCpy( MPRuntimePtr, TcgMPBuffer, Pages );

        Offset     = ((MPDRIVER_LEGHEADER*)MPRuntimePtr)->CodeP;
        MPRuntime += Offset;

        //Assuming we are in Protected mode with flat address selector 10 as
        //set by startup32.asm
        InitCall.Offset   = (UINT32)MPRuntime;
        InitCall.Selector = SEL_flatCS;
        InitCall.Codep    = ((MPDRIVER_LEGHEADER*)MPRuntimePtr)->CodeP;
        InitCall.Size     = Pages;

        //create Hob to pass PEI Capabilities information
        Status = TcgPeiBuildHobGuid(
          PeiServices,
          &gEfiTcgMpDriverHobGuid,
          sizeof (FAR32LOCALS),
          &MpHobType );

        ASSERT_PEI_ERROR( PeiServices, Status );
        MpHobType++;
        (*PeiServices)->CopyMem( MpHobType, &InitCall, sizeof (FAR32LOCALS));
        if ( EFI_ERROR( Status )) {
            return Status;
        }
        
        if(AutoSupportType())
        {
            Status = (*PeiServices)->InstallPpi( PeiServices, TcmInitPpi );
            if ( EFI_ERROR( Status )) {
                return Status;
            }
        }else{//legacy IO support for TPM
            Status = (*PeiServices)->InstallPpi( PeiServices, LegacyTpmInitPpi );
            if ( EFI_ERROR( Status )) {
                return Status;
            }
        }

 #if (StartupCmd_SelfTest_State == 1)
    Status = SendStartupandSelftest(PeiServices,BootMode);
    if(EFI_ERROR(Status))return Status;  //if startup or selftest fails, treat it as a fatal error; return
 #endif
     }
       
 #if (StartupCmd_SelfTest_State == 0)
    Status = SendStartupandSelftest(PeiServices,BootMode);
    if(EFI_ERROR(Status))return Status;  //if startup or selftest fails, treat it as a fatal error; return
 #endif

    if((BootMode == BOOT_ON_S3_RESUME) || (BootMode == BOOT_IN_RECOVERY_MODE)){
        return EFI_SUCCESS;
    }

    Status = TcgPeiBuildHobGuid(
        PeiServices,
        &gPeiEfiAmiTcgWakeEventDataHobGuid,
        sizeof (BootMode),
        &ptrBootMode );

    ASSERT_PEI_ERROR( PeiServices, Status );
    ptrBootMode++;
    (*PeiServices)->CopyMem( ptrBootMode, &BootMode, sizeof (BootMode));

    //even if TPM is deactivated still build hob but
    //don't populate it.
    Status = TcgPeiBuildHobGuid(
        PeiServices,
        &gEfiPeiAmiTcgLogHobGuid,
        sizeof (*TcgLog) + TCG_LOG_MAX_TABLE_SIZE,
        &Hob );

    ASSERT_PEI_ERROR( PeiServices, Status );

    TcgLog = (TCG_LOG_HOB*)(Hob + 1);
    (*PeiServices)->SetMem( TcgLog, sizeof (*TcgLog), 0 );
    TcgLog->TableMaxSize = TCG_LOG_MAX_TABLE_SIZE;

    if(!AutoSupportType())
    {
        Status = MeasureCRTMVersionFuncPtr( PeiServices );
        ASSERT_PEI_ERROR( PeiServices, Status );
    }else{
        Status = MeasureTcmCRTMVersion( PeiServices );
        ASSERT_PEI_ERROR( PeiServices, Status );
    }

    Status = LocateTcgPpi(PeiServices, &TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    return Status;
}




static AMI_TCG_PLATFORM_PPI_AFTER_MEM  mAmiTcgPlatformPPI = {
    Set_TPMPhysicalPresence,
    MemoryPresentEntry,
    VerifyTcgVariables
};


static EFI_PEI_PPI_DESCRIPTOR mAmiTcgPlatformPPIListAfterMem[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI
        | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gAmiTcgPlatformPpiAfterMem,
        &mAmiTcgPlatformPPI
    }
};




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   AmiTcgPlatformPEI_EntryAfterMem
//
// Description: Installs AMIplatform PPI for initialization in PEI after 
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
EFIAPI AmiTcgPlatformPEI_EntryAfterMem(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices 
){
    EFI_STATUS Status;

    Status = (*PeiServices)->InstallPpi( PeiServices, mAmiTcgPlatformPPIListAfterMem );
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
