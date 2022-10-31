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
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgPlatform/AmiTcgPlatformPei/AmiTcgPlatformPeiLib.c 16    4/27/12 6:16p Fredericko $
//
// $Revision: 16 $
//
// $Date: 4/27/12 6:16p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiTcgPlatformPeilib.c
//
// Description:	Function file that contains library files for AmiTcgPlatformPei
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Efi.h>
#include <Pei.h>
#include "AmiTcg\TcgCommon.h"
#include <AmiPeiLib.h>
#include "AmiTcg\TcgMisc.h"
#include <PPI/TcgTcmService.h>
#include <Ppi/TcgService.h>
#include <Ppi/TpmDevice.h>
#include "PPI\CpuIo.h"
#include "PPI\LoadFile.h"
#include <Ppi\ReadOnlyVariable.h>
#include <Ppi/TcgPlatformSetupPeiPolicy.h>
#include <AmiTcg\AmiTcgPlatformPei.h>
#include <Ppi/AmiTcgPlatformPpi.h>
#include <Ppi/ReadOnlyVariable.h>
#include <Token.h>


//*************************************************************************
//                      GLOBAL DEFINITIONS
//*************************************************************************
EFI_GUID                Descguid = AMI_TCG_PERM_FLAGS_GUID;
EFI_GUID   gTpmguidEndOfPei = EFI_PEI_END_OF_PEI_PHASE_PPI_GUID;

EFI_GUID gTcgReadOnlyVariablePpiGuid
    = EFI_TCG_PEI_READ_ONLY_VARIABLE_PPI_GUID;


EFI_GUID        TcgBoardEfiGlobalVariableGuid = TCG_EFI_GLOBAL_VARIABLE_GUID;

typedef struct
{
    TPM_1_2_CMD_HEADER hdr;
    UINT32             pcr;
    TCG_DIGEST         digest;  
} TPM_EXTEND_CMD;

typedef struct
{
    TPM_1_2_RET_HEADER rethdr;
    TCG_DIGEST         Outdigest;  
} TPM_EXTEND_RET;

//**********************************************************************
//                  Links
//**********************************************************************
extern MEASURE_CRTM_VERSION_PEI_FUNC_PTR  MEASURE_CRTM_VERSION_PEI_FUNCTION;
MEASURE_CRTM_VERSION_PEI_FUNC_PTR *MeasureCRTMVersionFuncPtr = MEASURE_CRTM_VERSION_PEI_FUNCTION;

extern MEASURE_TCG_PCCLIENT_ID_PEI_FUNC_PTR  MEASURE_TCG_PCCLIENT_ID_PEI_FUNCTION;
MEASURE_TCG_PCCLIENT_ID_PEI_FUNC_PTR *MeasureTCGPcClientSpecIDFuncPtr = MEASURE_TCG_PCCLIENT_ID_PEI_FUNCTION;

extern MEASURE_CORE_DXE_FW_VOL_PEI_FUNC_PTR  MEASURE_CORE_DXE_FW_VOL_PEI_FUNCTION;
MEASURE_CORE_DXE_FW_VOL_PEI_FUNC_PTR *MeasureDxeCoreFwVolFuncPtr = MEASURE_CORE_DXE_FW_VOL_PEI_FUNCTION;


//**********************************************************************
//                  TCG_Helper functions
//**********************************************************************
#pragma pack(1)
typedef struct _TCG_PEI_CALLBACK_CONTEXT
{
    PEI_TPM_PPI      *TpmDevice;
    EFI_PEI_SERVICES **PeiServices;
} TCG_PEI_CALLBACK_CONTEXT;
#pragma pack()



EFI_STATUS
__stdcall TcgCommonPassThrough(
    IN VOID                    *CallbackContext,
    IN UINT32                  NoInputBuffers,
    IN TPM_TRANSMIT_BUFFER     *InputBuffers,
    IN UINT32                  NoOutputBuffers,
    IN OUT TPM_TRANSMIT_BUFFER *OutputBuffers )
{
    TCG_PEI_CALLBACK_CONTEXT *Ctx;

    Ctx = (TCG_PEI_CALLBACK_CONTEXT*)CallbackContext;

    return Ctx->TpmDevice->Transmit(
               Ctx->TpmDevice,
               Ctx->PeiServices,
               NoInputBuffers,
               InputBuffers,
               NoOutputBuffers,
               OutputBuffers
               );
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgPeiBuildHobGuid
//
// Description: Internal abstracted function to create a Hob
//
// Input:       IN  EFI_PEI_SERVICES  **PeiServices,
//              IN  EFI_GUID          *Guid,
//              IN  UINTN             DataLength,
//              OUT VOID              **Hob
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
EFI_STATUS TcgPeiBuildHobGuid(
    IN EFI_PEI_SERVICES **PeiServices,
    IN EFI_GUID         *Guid,
    IN UINTN            DataLength,
    OUT VOID            **Hob )
{
    EFI_STATUS Status;

    Status = (*PeiServices)->CreateHob(
        PeiServices,
        EFI_HOB_TYPE_GUID_EXTENSION,
        (UINT16) ( sizeof (EFI_HOB_GUID_TYPE) + DataLength ),
        Hob
        );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    ((EFI_HOB_GUID_TYPE*)(*Hob))->Name = *Guid;

    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   FillDriverLocByFile
//
// Description: Helper function to locate a Driver by guid and fill in
//              Offset data about it. Mainly for MA Driver
//
//
// Input:       IN OUT UINT32* Offset,
//              IN     EFI_PEI_SERVICES  **ps,
//              IN     EFI_GUID *Driveguid,
//              IN OUT VOID **MAStart,
//              IN OUT UINTN *MASize
//
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS FillDriverLocByFile(
    IN OUT UINT32       * Offset,
    IN EFI_PEI_SERVICES **ps,
    IN EFI_GUID         *Driveguid,
    IN OUT VOID         **MAStart,
    IN OUT UINTN        *MASize )
{
    UINT8                      * Temp;
    EFI_FIRMWARE_VOLUME_HEADER *pFV;
    EFI_FFS_FILE_HEADER        *ppFile;
    EFI_STATUS                 Status;
    UINTN                      Instance = 0;
    MPDRIVER_LEGHEADER         *Buffer;
    UINT32                     CodeSec = 0;

    while ( TRUE )
    {
        Status = (*ps)->FfsFindNextVolume( ps, Instance, &pFV );

        if ( EFI_ERROR( Status ))
        {
            return  Status; 
        }

        ppFile = NULL;
        //
        // Start new search in volume
        //
        while ( TRUE )
        {
            Status = (*ps)->FfsFindNextFile( ps,
                                             EFI_FV_FILETYPE_FREEFORM,
                                             pFV,
                                             &ppFile );

            if ( Status == EFI_NOT_FOUND )
            {
                break;
            }

            if ( CompareGuid( &(ppFile->Name), Driveguid ))
            {
                Temp = ((UINT8*) ppFile + sizeof (EFI_FFS_FILE_HEADER)
                       + sizeof(ppFile->Size));
                Buffer   = (MPDRIVER_LEGHEADER*)(Temp + 1);
                CodeSec  = Buffer->CodeP;
                *Offset  = (UINT32)Buffer + CodeSec;
                *MAStart = Buffer;
                *MASize  = (UINTN)Buffer->Size;
                return Status;
            }
        }
        Instance += 1;
    }
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   FillDriverLoc
//
// Description: Minor function to fill in MPDriver Offsets for TPM
//              Device PPI
//
//
// Input:       IN  OUT  UINT32* Offset,
//              IN       EFI_PEI_SERVICES  **ps,
//              IN       EFI_GUID *Driveguid
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
VOID FillDriverLoc(
    IN OUT UINT32       * Offset,
    IN EFI_PEI_SERVICES **ps,
    IN EFI_GUID         *Driveguid )
{
    EFI_HOB_GUID_TYPE *DrvHob;
    UINT8             * Temp;

    (*ps)->GetHobList( ps, &DrvHob );
    while ( !EFI_ERROR( FindNextHobByType( EFI_HOB_TYPE_GUID_EXTENSION, &DrvHob )))
    {
        if ((DrvHob->Header.HobType == EFI_HOB_TYPE_GUID_EXTENSION)
            && (guidcmp( &DrvHob->Name, Driveguid )) == 0 )
        {
            Temp    = (UINT8*)++DrvHob;
            *Offset = *(UINT32*) Temp;
            break;
        }
    }
}






//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   LocateTcgPpi
//
// Description: Locates and initializes TCG Ppi
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
EFI_STATUS LocateTcgPpi(
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_TPM_PPI      **gTpmDevicePpi,
    IN PEI_TCG_PPI      **gTcgPpi
)
{
    EFI_STATUS      Status;

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gPeiTpmPpiGuid,
                            0, NULL,
                            gTpmDevicePpi);     


    if(EFI_ERROR(Status))return Status;

     Status =  (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gPeiTcgPpiGuid,
                            0, NULL,
                            gTcgPpi);

    return Status;
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   LocateTcgPpi
//
// Description: Locates and initializes TCG Ppi
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
EFI_STATUS LocateTcmPpi(
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_TPM_PPI      **gTpmDevicePpi,
    IN PEI_TCM_PPI      **gTcmPpi
)
{
    EFI_STATUS      Status;

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gPeiTpmPpiGuid,
                            0, NULL,
                            gTpmDevicePpi);     


    if(EFI_ERROR(Status))return Status;

     Status =  (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gPeiTcgPpiGuid,
                            0, NULL,
                            gTcmPpi);

    return Status;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgPeiTpmStartup
//
// Description: Sends Initial TPM Startup Command
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//              IN      EFI_BOOT_MODE                BootMode
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
 TcgPeiTpmStartup(
    IN EFI_PEI_SERVICES **PeiServices,
    IN EFI_BOOT_MODE    BootMode )
{
    EFI_STATUS               Status;
    TPM_1_2_CMD_STARTUP      cmdStartup;
    TPM_1_2_RET_HEADER       retHeader;
    TPM_STARTUP_TYPE         TpmSt;
    PEI_TPM_PPI              *TpmPpi = NULL;         
    PEI_TCG_PPI              *TcgPpi = NULL;
    TCG_PLATFORM_SETUP_INTERFACE   *TcgPeiPolicy = NULL;
    EFI_GUID                       gTcgPeiPolicyGuid =\
                                      TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    TCG_CONFIGURATION              ConfigFlags;

    TpmSt = TPM_ST_CLEAR;

    Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gTcgPeiPolicyGuid,
                0, NULL,
                &TcgPeiPolicy);

    if(EFI_ERROR(Status) || TcgPeiPolicy == NULL )return Status;

    Status = TcgPeiPolicy->getTcgPeiPolicy(PeiServices, &ConfigFlags);

    if(EFI_ERROR(Status))return Status;

    if(ConfigFlags.DisallowTpm == 1)
    {
        BootMode = BOOT_IN_RECOVERY_MODE;  //deactivate the TPM
    }

    if ( BootMode == BOOT_ON_S3_RESUME )
    {
        TpmSt = TPM_ST_STATE;
    }

#if (TCG_CONVENTIONAL_BIOS_6_1)

    if ( BootMode == BOOT_IN_RECOVERY_MODE )
    {
        TpmSt = TPM_ST_DEACTIVATED;
    }
#endif

    Status = LocateTcgPpi(PeiServices,&TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    Status = TpmPpi->Init(TpmPpi, PeiServices );
    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    cmdStartup.Header.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdStartup.Header.ParamSize = TPM_H2NL( sizeof (cmdStartup));
    cmdStartup.Header.Ordinal   = TPM_H2NL( TPM_ORD_Startup );
    cmdStartup.StartupType      = TPM_H2NS( TpmSt );

    Status = TcgPpi->TCGPassThroughToTpm(
                  TcgPpi,
                  PeiServices,
                  sizeof (cmdStartup),
                  (UINT8*)&cmdStartup,
                  sizeof (retHeader),
                  (UINT8*)&retHeader);


    Status = TpmPpi->Close(TpmPpi, PeiServices );
    if (EFI_ERROR( Status )){
         return Status;
    }


    PEI_TRACE((-1, PeiServices,
               "Tcg Startup Command Return Code: size: %x; retCode:%x; tag:%x; bytes %08x\n",
               TPM_H2NL(retHeader.ParamSize ),
               TPM_H2NL(retHeader.RetCode ),
               (UINT32)
               TPM_H2NS(retHeader.Tag )));

    if ( retHeader.ParamSize == 0 )
    {
        return EFI_DEVICE_ERROR;
    }
    return (Status | TPM_H2NL( retHeader.RetCode ));
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmPeiStartup
//
// Description: Sends Initial TPM Startup Command
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//              IN      EFI_BOOT_MODE                BootMode
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
static
EFI_STATUS
__stdcall TcmPeiStartup(
    IN EFI_PEI_SERVICES **PeiServices,
    IN EFI_BOOT_MODE    BootMode )
{
    EFI_STATUS               Status;
    TPM_1_2_CMD_STARTUP      cmdStartup;
    TPM_1_2_RET_HEADER       retHeader;
    TPM_STARTUP_TYPE         TpmSt;
    PEI_TPM_PPI              *TpmPpi = NULL;         
    PEI_TCM_PPI              *TcgPpi = NULL;

    TpmSt = TPM_ST_CLEAR;

    if ( BootMode == BOOT_ON_S3_RESUME )
    {
        TpmSt = TPM_ST_STATE;
    }

#if (TCG_CONVENTIONAL_BIOS_6_1)

    if ( BootMode == BOOT_IN_RECOVERY_MODE )
    {
        TpmSt = TPM_ST_DEACTIVATED;
    }
#endif

    Status = LocateTcmPpi(PeiServices,&TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    Status = TpmPpi->Init(TpmPpi, PeiServices );
    if ( EFI_ERROR( Status ))
    {
        return Status;
    }


    cmdStartup.Header.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdStartup.Header.ParamSize = TPM_H2NL( sizeof (cmdStartup));
    cmdStartup.Header.Ordinal   = TPM_H2NL( TCM_ORD_Startup );
    
    cmdStartup.StartupType      = TcgCommonH2NS( TpmSt );

    Status = TcgPpi->TCMPassThroughToTcm(
                  TcgPpi,
                  PeiServices,
                  sizeof (cmdStartup),
                  (UINT8*)&cmdStartup,
                  sizeof (retHeader),
                  (UINT8*)&retHeader);


    Status = TpmPpi->Close(TpmPpi, PeiServices );
    if (EFI_ERROR( Status )){
         return Status;
    }

    PEI_TRACE((-1, PeiServices,
               "Tcg Startup Command Return Code: size: %x; retCode:%x; tag:%x; bytes %08x\n",
               TPM_H2NL(retHeader.ParamSize ),
               TPM_H2NL(retHeader.RetCode ),
               (UINT32)
               TPM_H2NS(retHeader.Tag )));

    if ( retHeader.ParamSize == 0 )
    {
        return EFI_DEVICE_ERROR;
    }
    return (Status | TPM_H2NL( retHeader.RetCode ));
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   ContinueTPMSelfTest
//
// Description: Executes ContinueSelfTest operation on TPM. Certain TPM
//              operation require this operation to be execute before.
//
// Input:       IN  EFI_PEI_SERVICES **ps
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS ContinueTPMSelfTest(
    IN EFI_PEI_SERVICES **ps )
{
    TPM_1_2_CMD_HEADER  cmd;
    TPM_1_2_RET_HEADER  result;
    EFI_STATUS          Status;
    PEI_TPM_PPI         *TpmPpi = NULL;         
    PEI_TCG_PPI         *TcgPpi = NULL;

#if defined DONT_SEND_SELFTEST_TILL_READY_TO_BOOT && DONT_SEND_SELFTEST_TILL_READY_TO_BOOT == 1
    if(*(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF00) == 0x15D1)
    {
        return EFI_SUCCESS;
    }
#endif

    Status = LocateTcgPpi(ps,&TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    Status = TpmPpi->Init(TpmPpi, ps );
    if ( EFI_ERROR( Status )){
         return Status;
    }

    cmd.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmd.ParamSize = TPM_H2NL( sizeof (cmd));
    cmd.Ordinal   = TPM_H2NL( TPM_ORD_ContinueSelfTest );

    Status =  TcgPpi->TCGPassThroughToTpm(
                    TcgPpi,
                    ps,
                    sizeof (cmd),
                    (UINT8*)&cmd,
                    sizeof (result),
                    (UINT8*)&result );

    PEI_TRACE((-1, ps, "TCG Pei: Self Test : status=%x; RetCode=%x\n", Status,
               TPM_H2NL( result.RetCode )));

    Status = TpmPpi->Close(TpmPpi, ps );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }
    else if ( result.RetCode != 0 )
    {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   ContinueTCMSelfTest
//
// Description: Executes ContinueSelfTest operation on TPM. Certain TPM
//              operation require this operation to be execute before.
//
// Input:       IN  EFI_PEI_SERVICES **ps
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************

EFI_STATUS ContinueTCMSelfTest(
    IN EFI_PEI_SERVICES **ps )
{
    TPM_1_2_CMD_HEADER       cmd;
    TPM_1_2_RET_HEADER       result;
    EFI_STATUS               Status;
    PEI_TPM_PPI         *TpmPpi = NULL;         
    PEI_TCG_PPI         *TcgPpi = NULL;


    Status = LocateTcgPpi(ps,&TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    Status = TpmPpi->Init(TpmPpi, ps );
    if ( EFI_ERROR( Status )){
         return Status;
    }

    cmd.Tag       = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmd.ParamSize = TPM_H2NL( sizeof (cmd));
    cmd.Ordinal   = TPM_H2NL(TCM_ORD_ContinueSelfTest );

    Status =  TcgPpi->TCGPassThroughToTpm(
                    TcgPpi,
                    ps,
                    sizeof (cmd),
                    (UINT8*)&cmd,
                    sizeof (result),
                    (UINT8*)&result );

    PEI_TRACE((-1, ps, "TCG Pei: Self Test : status=%x; RetCode=%x\n", Status,
               TPM_H2NL( result.RetCode )));

    Status = TpmPpi->Close(TpmPpi, ps );

    if ( EFI_ERROR( Status ))
    {
        return Status;
    }

    if ( result.RetCode != 0 )
    {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SendStartupandSelftest
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
EFI_STATUS SendStartupandSelftest(
    IN EFI_PEI_SERVICES **PeiServices,
    IN EFI_BOOT_MODE    BootMode
)
{
    EFI_STATUS      Status;
    EFI_GUID        SkipTpmStartupGuid = AMI_SKIP_TPM_STARTUP_GUID;
    BOOLEAN         SkipTpmStartup = FALSE;
    AMI_TCG_PEI_FUNCTION_OVERRIDE_PPI *PpiOverride;
    TCG_PLATFORM_SETUP_INTERFACE   *TcgPeiPolicy = NULL;
    EFI_GUID                        gTcgPeiPolicyGuid =\
                                      TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    TCG_CONFIGURATION              ConfigFlags;

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
                &gTcgPeiPolicyGuid,
                0, NULL,
                &TcgPeiPolicy);

    if(EFI_ERROR(Status) || TcgPeiPolicy == NULL )return Status;

    Status = TcgPeiPolicy->getTcgPeiPolicy(PeiServices, &ConfigFlags);

    if(EFI_ERROR(Status))return Status;

    if(ConfigFlags.DisallowTpm == 1)
    {
        BootMode = BOOT_IN_RECOVERY_MODE;  //deactivate the TPM
        Status = TcgPeiTpmStartup( PeiServices, BootMode );
        return Status;
    }
    
    if(!AutoSupportType()){
       if (!SkipTpmStartup) {
	        Status = TcgPeiTpmStartup( PeiServices, BootMode );
 	        if(Status){  
  	          //sartup command failed
   	         return EFI_DEVICE_ERROR;
    		}
        }
                       
        Status = ContinueTPMSelfTest( PeiServices );
         if(Status){  
            //Selftest command failed
            return EFI_DEVICE_ERROR;
        }
    }else{
         Status = TcmPeiStartup( PeiServices, BootMode );
         if(Status){  
            //sartup command failed
            return EFI_DEVICE_ERROR;
        }

        Status = ContinueTCMSelfTest( PeiServices );
        if(Status){  
            //selftest command failed
            return EFI_DEVICE_ERROR;
        }
    }
    return Status;
}








//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TCGPEI_GETCAP
//
// Description: Executes TPM operation to read capabilities
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//
// Output:      TPM_Capabilities_PermanentFlag
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
TPM_Capabilities_PermanentFlag INTTCGPEI_GETCAP(
    IN EFI_PEI_SERVICES **PeiServices )
{
    TPM_Capabilities_PermanentFlag  *cap = NULL;
    EFI_STATUS                      Status;
    TPM_GetCapabilities_Input       cmdGetCap;
    UINT8                           result[0x100];
    PEI_TPM_PPI                     *TpmPpi = NULL;         
    PEI_TCG_PPI                     *TcgPpi = NULL;
    EFI_GUID TcgPpiguid                    =  PEI_TCG_PPI_GUID;
    EFI_GUID TpmDevicePpiguid              =  PEI_TPM_PPI_GUID;

    cmdGetCap.Tag         = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdGetCap.ParamSize   = TPM_H2NL( sizeof (cmdGetCap));
    cmdGetCap.CommandCode = TPM_H2NL( TPM_ORD_GetCapability );
    cmdGetCap.caparea     = TPM_H2NL( TPM_CAP_FLAG );
    cmdGetCap.subCapSize  = TPM_H2NL( 4 ); // subCap is always 32bit long
    cmdGetCap.subCap      = TPM_H2NL( TPM_CAP_FLAG_PERMANENT );

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &TpmDevicePpiguid,
                            0, NULL,
                            &TpmPpi);     

    ASSERT_PEI_ERROR( PeiServices, Status );

    Status = TpmPpi->Init(TpmPpi, PeiServices);
   
    if(Status){
        MemSet(result,sizeof(TPM_Capabilities_PermanentFlag), 0);
        cap = (TPM_Capabilities_PermanentFlag*)result;
        return *cap;
    }

    Status =  (*PeiServices)->LocatePpi(
                            PeiServices,
                            &TcgPpiguid,
                            0, NULL,
                            &TcgPpi);

    ASSERT_PEI_ERROR( PeiServices, Status );


    Status = TcgPpi->TCGPassThroughToTpm(
                TcgPpi,
                PeiServices,
                sizeof(cmdGetCap),
                (UINT8*)&cmdGetCap,
                0x100,
                result );

    cap = (TPM_Capabilities_PermanentFlag*)result;

    PEI_TRACE((-1, PeiServices,
               "GetCapability: %r; size: %x; retCode:%x; tag:%x; bytes %08x\n",
               Status,TPM_H2NL( cap->ParamSize ), TPM_H2NL(cap->RetCode ),
               (UINT32)TPM_H2NS(cap->tag ), TPM_H2NL( *(UINT32*)&cap->disabled )));

 
    Status = TpmPpi->Close(TpmPpi, PeiServices);

    return *cap;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TCGPEI_GETCAP
//
// Description: Executes TPM operation to read capabilities
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//
// Output:      TPM_Capabilities_PermanentFlag
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
TPM_Capabilities_PermanentFlag NuvotonProprietaryGetFlags(
    IN EFI_PEI_SERVICES **PeiServices )
{
    TPM_Capabilities_PermanentFlag  cap;
    EFI_STATUS                      Status;
    TPM_RQU_COMMAND_HDR             cmdGetTpmStatus;
    UINT8                           result[0x100];
    PEI_TPM_PPI                     *TpmPpi = NULL;         
    PEI_TCG_PPI                     *TcgPpi = NULL;
    EFI_GUID TcgPpiguid                    =  PEI_TCG_PPI_GUID;
    EFI_GUID TpmDevicePpiguid              =  PEI_TPM_PPI_GUID;

    cmdGetTpmStatus.tag         = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdGetTpmStatus.paramSize   = TPM_H2NL( sizeof (TPM_RQU_COMMAND_HDR));
    cmdGetTpmStatus.ordinal     = TPM_H2NL( NTC_ORD_GET_TPM_STATUS );

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &TpmDevicePpiguid,
                            0, NULL,
                            &TpmPpi);     

    ASSERT_PEI_ERROR( PeiServices, Status );

    Status = TpmPpi->Init(TpmPpi, PeiServices);

    MemSet(&cap,sizeof(TPM_Capabilities_PermanentFlag), 0);           

    if(Status){
        return cap;
    }

    Status =  (*PeiServices)->LocatePpi(
                            PeiServices,
                            &TcgPpiguid,
                            0, NULL,
                            &TcgPpi);

    ASSERT_PEI_ERROR( PeiServices, Status );


    Status = TcgPpi->TCGPassThroughToTpm(
                TcgPpi,
                PeiServices,
                sizeof(cmdGetTpmStatus),
                (UINT8*)&cmdGetTpmStatus,
                0x100,
                result );
          
    if(((NUVOTON_SPECIFIC_FLAGS *)result)->RetCode == 0)
    {
        if(((NUVOTON_SPECIFIC_FLAGS *)result)->isdisabled){
            cap.disabled = 1; 
        }

        if(((NUVOTON_SPECIFIC_FLAGS *)result)->isdeactivated){
            cap.deactivated = 1; 
        }

        if(((NUVOTON_SPECIFIC_FLAGS *)result)->isOwnerSet){
            cap.ownership = 1; 
        }
    }else{
            cap.RetCode = ((NUVOTON_SPECIFIC_FLAGS *)result)->RetCode;
    }

 
    Status = TpmPpi->Close(TpmPpi, PeiServices);

    return cap;
}






//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TCGPEI_GETCAP
//
// Description: Executes TPM operation to read capabilities
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//
// Output:      TPM_Capabilities_PermanentFlag
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
TPM_Capabilities_PermanentFlag TCGPEI_GETCAP(
    IN EFI_PEI_SERVICES **PeiServices )
{
    TPM_PERM_FLAGS                 *CapPpi;
    EFI_GUID                       guid = AMI_TCG_PERM_FLAGS_GUID;
    EFI_STATUS                     Status;
    TPM_Capabilities_PermanentFlag Cap;
    EFI_PEI_PPI_DESCRIPTOR  *FlagsPpiDesc;
    TPM_PERM_FLAGS          *FlagsPpi;

    Status = (*PeiServices)->LocatePpi(
                        PeiServices,
                        &guid,
                        0,
                        NULL,
                        &CapPpi);
    
    
    if (EFI_ERROR(Status)){
        //
        // Allocate descriptor and PPI structures
        Status = (*PeiServices)->AllocatePool (PeiServices, sizeof (EFI_PEI_PPI_DESCRIPTOR), &FlagsPpiDesc);
        ASSERT_PEI_ERROR (PeiServices, Status);
        (*PeiServices)->SetMem ((VOID*) FlagsPpiDesc, sizeof (EFI_PEI_PPI_DESCRIPTOR), 0);

        Status = (*PeiServices)->AllocatePool (PeiServices, sizeof (TPM_PERM_FLAGS), &FlagsPpi);
        ASSERT_PEI_ERROR (PeiServices, Status);
        (*PeiServices)->SetMem ((VOID*) FlagsPpi, sizeof (TPM_PERM_FLAGS), 0);

        Cap = INTTCGPEI_GETCAP( PeiServices );
        (*PeiServices)->CopyMem(&FlagsPpi->Capabilities, &Cap, sizeof(TPM_Capabilities_PermanentFlag));
    
        FlagsPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
        FlagsPpiDesc->Ppi = FlagsPpi;

        FlagsPpiDesc->Guid = &Descguid;
        Status = (**PeiServices).InstallPpi (PeiServices, FlagsPpiDesc);
        ASSERT_PEI_ERROR (PeiServices, Status);

        return Cap;
    }
    else{
     return CapPpi->Capabilities;
    }
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TCGPEI_GETCAP
//
// Description: Executes TPM operation to read capabilities
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
//
// Output:      TPM_Capabilities_PermanentFlag
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
TCM_Capabilities_PermanentFlag TCMPEI_GETCAP(
    IN EFI_PEI_SERVICES **PeiServices )
{
    TCM_Capabilities_PermanentFlag * cap = NULL;
    EFI_STATUS                     Status;
    TPM_GetCapabilities_Input      cmdGetCap;
    UINT8                          result[0x100];
    PEI_TPM_PPI                    *TpmPpi = NULL;         
    PEI_TCM_PPI                    *TcgPpi = NULL;


    cmdGetCap.Tag         = TPM_H2NS( TPM_TAG_RQU_COMMAND );
    cmdGetCap.ParamSize   = TPM_H2NL( sizeof (cmdGetCap));
    cmdGetCap.CommandCode = TPM_H2NL( TCM_ORD_GetCapability );

    cmdGetCap.caparea     = TPM_H2NL( TPM_CAP_FLAG );
    cmdGetCap.subCapSize  = TPM_H2NL( 4 ); // subCap is always 32bit long
    cmdGetCap.subCap      = TPM_H2NL( TPM_CAP_FLAG_PERMANENT );

    Status = LocateTcmPpi(PeiServices, &TpmPpi, &TcgPpi);
    ASSERT_PEI_ERROR( PeiServices, Status );

    Status = TpmPpi->Init(TpmPpi, PeiServices );
    ASSERT_PEI_ERROR( PeiServices, Status );

    Status = TcgPpi->TCMPassThroughToTcm(
                TcgPpi,
                PeiServices,
                sizeof(cmdGetCap),
                (UINT8*)&cmdGetCap,
                0x100,
                result );

    cap = (TCM_Capabilities_PermanentFlag*)result;

    PEI_TRACE((-1, PeiServices,
               "GetCapability: %r; size: %x; retCode:%x; tag:%x; bytes %08x\n",
               Status,TPM_H2NL( cap->ParamSize ), TPM_H2NL(cap->RetCode ),
               (UINT32)TPM_H2NS(cap->tag ), TPM_H2NL( *(UINT32*)&cap->disabled )));

    Status = TpmPpi->Close(TpmPpi, PeiServices );
    ASSERT_PEI_ERROR( PeiServices, Status );

    return *cap;
}






//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   MeasureCRTMVersion
//
// Description: Measures EFI CRTM Version
//              Demo Version[546BFB1E1D0C4055A4AD4EF4BF17B83A]
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
EFI_STATUS
 MeasureCRTMVersion(
    IN EFI_PEI_SERVICES **PeiServices )
{
    EFI_TCG_PCR_EVENT TcgEvent;
    UINT32            EventNum;
    UINTN             Len         = sizeof(EFI_GUID);
    EFI_GUID          CrtmVersion = CRTM_GUID;
    PEI_TPM_PPI       *TpmPpi = NULL;         
    PEI_TCG_PPI       *TcgPpi = NULL;
    EFI_STATUS        Status;

    TcgEvent.Header.PCRIndex      = 0;
    TcgEvent.Header.EventType     = EV_S_CRTM_VERSION;
    TcgEvent.Header.EventDataSize = Len;

    Status = LocateTcgPpi(PeiServices,&TpmPpi, &TcgPpi);
    if(EFI_ERROR(Status))return EFI_NOT_FOUND;

    MemCpy(
        &TcgEvent.Event.SCrtmVersion,
        &CrtmVersion,
        Len
        );

    return TcgPpi->TCGHashLogExtendEvent(
               TcgPpi,
               PeiServices,
               (UINT8*)&TcgEvent.Event,
               TcgEvent.Header.EventDataSize,
               (TCG_PCR_EVENT*)&TcgEvent,
               &EventNum
               );
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   MeasureTcmCRTMVersion
//
// Description: Measures EFI CRTM Version
//              Demo Version[546BFB1E1D0C4055A4AD4EF4BF17B83A]
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
EFI_STATUS
 MeasureTcmCRTMVersion(
    IN EFI_PEI_SERVICES **PeiServices )
{
    EFI_TCM_PCR_EVENT TcmEvent;
    UINT32            EventNum;
    UINTN             Len         = sizeof(EFI_GUID);
    EFI_GUID          CrtmVersion = CRTM_GUID;
    PEI_TPM_PPI       *TpmPpi = NULL;         
    PEI_TCM_PPI       *TcgPpi = NULL;
    EFI_STATUS        Status;

    TcmEvent.Header.PCRIndex      = 0;
    TcmEvent.Header.EventType     = EV_S_CRTM_VERSION;
    TcmEvent.Header.EventDataSize = Len;

    MemCpy(&TcmEvent.Event.SCrtmVersion,
    &CrtmVersion, Len);

    Status = LocateTcmPpi(PeiServices, &TpmPpi, &TcgPpi);
    ASSERT_PEI_ERROR( PeiServices, Status );

    return TcgPpi->TCMHashLogExtendEvent(TcgPpi,
               PeiServices,
               (UINT8*)&TcmEvent.Event,
               TcmEvent.Header.EventDataSize,
               (TCM_PCR_EVENT*)&TcmEvent,
               &EventNum);
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   MeasureTCGPcClientSpecID
//
// Description: Includes a measurement of the TcgSpecID into PCR[0]
//
//
// Input:       IN  PEI_TCG_PPI* tcg,
//              IN  EFI_PEI_SERVICES      **ps
//
// Output:
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS MeasureTCGPcClientSpecID(
    IN EFI_PEI_SERVICES **ps, 
    IN PEI_TCG_PPI      *tcg )
{
    TCG_PCR_EVENT                 ev;
    TCG_PCClientSpecIDEventStruct TcgInfo;
    TCG_VendorInfoStruct          TcgVenInfo;
    UINT32                        n;
    EFI_STATUS                    status;
    UINT8                         i = 0, times = 0;

    PEI_TRACE((-1, ps, "TCG Pei: TCG_PcClientSpecID\n"));

    MemCpy(
        TcgVenInfo.TCGBIOSVENDOR,
        TCG_BIOS_VENDOR,
        sizeof(TcgVenInfo.TCGBIOSVENDOR));

    MemCpy(
        TcgVenInfo.TCGOEMID,
        TCG_OEM_ID,
        sizeof(TcgVenInfo.TCGOEMID));

    //fill in pc-client spec id
    TcgInfo.PlatformClass     = TCG_PLATFORM_CLASS;
    TcgInfo.BIOSTypeInterface = TCG_BIOS_TYPE_INTERFACE;
    TcgInfo.BIOSTypeMapping   = TCG_BIOS_TYPE_MAPPING;
    TcgInfo.SpecVersionMajor  = TCG_SPEC_VERSION_MAJOR;
    TcgInfo.SpecVersionMinor  = TCG_SPEC_VERSION_MINOR;
    TcgInfo.SpecErrata        = TCG_SPEC_ERRATA;
    TcgInfo.Reserved          = 0x00;
    TcgInfo.VendorInfo        = &TcgVenInfo;
    TcgInfo.VendorInfoSize    = sizeof(TcgInfo.VendorInfo );


    ev.PCRIndex   = PCRi_CRTM_AND_POST_BIOS;
    ev.EventType  = EV_SPECIFICATION_IDENTIFIER;
    ev.EventSize  = sizeof (TcgInfo);
    ev.Event[0] = *(UINT8*)&TcgInfo;

M_TRY_AGAIN:
    status = tcg->TCGHashLogExtendEvent(tcg,
                                        ps,
                                        (UINT8*)&TcgInfo,
                                        sizeof(TcgInfo),
                                        &ev,
                                        &n );

     if ( EFI_ERROR( status ))
        {
            if ( status == EFI_NOT_READY )
                {
                    i = 0;
                    do
                    {
                        i++;
                    } while ( i < 200 );

                    if ( times > 0x2 )
                    {
                        PEI_TRACE((-1, ps,"xTcgPei::Could not measure PC_CLIENTSPECID \n"));
                        goto M_TRY_DONE;
                    }
                    times++;
                    goto M_TRY_AGAIN;
                }
        }
M_TRY_DONE:
    return status;
}








//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   MeasureDxeCoreFwVol
//
// Description: Measures the firmware volume as a EV_POST_CODE event
//
//
// Input:       IN    PEI_TCG_PPI* tcg,
//              IN    EFI_PEI_SERVICES              **ps,
//              IN    EFI_FIRMWARE_VOLUME_HEADER    *fwv
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
EFI_STATUS MeasureDxeCoreFwVol(
    IN PEI_TCG_PPI                * tcg,
    IN EFI_PEI_SERVICES           **ps,
    IN EFI_FIRMWARE_VOLUME_HEADER *fwv )
{ 
    PEI_EFI_POST_CODE       ev;
    UINT32                  n;
    EFI_STATUS              status;
    void                    *Context = NULL;;


    PEI_TRACE((-1, ps, "TCG Pei: measure FwMain: at %x size %d\n", fwv,
               fwv->FvLength));
        
    ev.Header.PCRIndex      = PCRi_CRTM_AND_POST_BIOS;
    ev.Header.EventType     = EV_POST_CODE;
    ev.Header.EventDataSize = sizeof (EFI_TCG_EV_POST_CODE);
    ev.Event.PostCodeAddress = (EFI_PHYSICAL_ADDRESS)FV_MAIN_BASE;

#if PARTIALLY_MEASURE_FVMAIN == 1
    #if x64_BUILD
        ev.Event.PostCodeLength = (UINT64)TCG_FV_MAIN_SIZE;
    #else
        ev.Event.PostCodeLength = (UINTN)TCG_FV_MAIN_SIZE;
    #endif
#else
#if defined x64_BUILD &&  x64_BUILD == 1
        ev.Event.PostCodeLength = (UINT64)fwv->FvLength;
#else
        ev.Event.PostCodeLength = (UINTN)fwv->FvLength;
#endif
#endif
   status = tcg->TCGHashLogExtendEvent( tcg, ps, (UINT8*)fwv,
                                            (UINT32)ev.Event.PostCodeLength,
                                            (TCG_PCR_EVENT*)&ev, &n );

    return status;

}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmMeasureDxeCoreFwVol
//
// Description: Measures the firmware volume as a EV_POST_CODE event
//
//
// Input:       IN    PEI_TCM_PPI* tcg,
//              IN    EFI_PEI_SERVICES              **ps,
//              IN    EFI_FIRMWARE_VOLUME_HEADER    *fwv
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
EFI_STATUS TCMMeasureDxeCoreFwVol(
    IN PEI_TCM_PPI                * tcg,
    IN EFI_PEI_SERVICES           **ps,
    IN EFI_FIRMWARE_VOLUME_HEADER *fwv )
{
    TCM_PEI_EFI_POST_CODE   tcmev;
    UINT32            n;
    EFI_STATUS        status;
    TCG_DIGEST        TempDigest;
    void              *Context = NULL;;


    PEI_TRACE((-1, ps, "TCG Pei: measure FwMain: at %x size %d\n", fwv,
               fwv->FvLength));

    tcmev.Header.PCRIndex      = PCRi_CRTM_AND_POST_BIOS;
    tcmev.Header.EventType     = EV_POST_CODE;
    tcmev.Header.EventDataSize = sizeof (EFI_TCG_EV_POST_CODE );

    SHA1HashAll( Context, fwv, (UINTN)fwv->FvLength, &TempDigest );

    tcmev.Event.PostCodeAddress = (EFI_PHYSICAL_ADDRESS)&TempDigest;
#if x64_BUILD
    tcmev.Event.PostCodeLength = (UINT64)TPM_SHA1_160_HASH_LEN;
#else
    tcmev.Event.PostCodeLength = (UINTN)TPM_SHA1_160_HASH_LEN;
#endif

    status = tcg->TCMHashLogExtendEvent( tcg, ps, (UINT8*)&TempDigest,
                                         (UINT32)tcmev.Event.PostCodeLength,
                                         (TCM_PCR_EVENT*)&tcmev, &n );

    return status;
}






//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   Lock_TPMPhysicalPresence
//
// Description: check whether to Lock TPM physical Presence
//
//
// Input:       IN      EFI_PEI_SERVICES          **PeiServices,
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
BOOLEAN Lock_TPMPhysicalPresence(
    IN EFI_PEI_SERVICES **PeiServices )
{
    EFI_STATUS                     Status;
    EFI_PEI_READ_ONLY_VARIABLE_PPI *ReadOnlyVariable;
    EFI_GUID                       gAmiTcgEfiOSVarguid
                                         = AMI_TCG_EFI_OS_VARIABLE_GUID;
    UINTN                          OSVarSize = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR                 Temp;
    UINT32                         Attribs = EFI_VARIABLE_NON_VOLATILE
                                    | EFI_VARIABLE_BOOTSERVICE_ACCESS
                                    | EFI_VARIABLE_RUNTIME_ACCESS;
    UINTN                          Size = sizeof(UINT8);
    UINTN                          INTSize = sizeof(UINT32);
    UINT32                         OwnerCap = 0;
    EFI_GUID                       peiTcgEfiGlobalVariableGuid \
                                            = TCG_EFI_GLOBAL_VARIABLE_GUID;
    TCG_PLATFORM_SETUP_INTERFACE   *TcgPeiPolicy = NULL;
    EFI_GUID                       gTcgPeiPolicyGuid =\
                                        TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
    TCG_CONFIGURATION              ConfigFlags;
    TPM_Capabilities_PermanentFlag Cap;
   
    Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gTcgPeiPolicyGuid,
                0, NULL,
                &TcgPeiPolicy);

    if(EFI_ERROR(Status) || TcgPeiPolicy == NULL )return Status;

    Status = TcgPeiPolicy->getTcgPeiPolicy(PeiServices, &ConfigFlags);

    Status = (*PeiServices)->LocatePpi(
        PeiServices,
        &gTcgReadOnlyVariablePpiGuid,
        0, NULL,
        &ReadOnlyVariable );

    Temp.RQST    = 0;
    Temp.RCNT    = 0;
    Temp.ERROR   = 0;
    Temp.Flag    = 0;
    Temp.AmiMisc = 0;

    Status = ReadOnlyVariable->PeiGetVariable(
        		PeiServices,
        		L"AMITCGPPIVAR",
        		&gAmiTcgEfiOSVarguid,
        		NULL,
        		&OSVarSize,
        		&Temp );

    if(Status == EFI_NOT_FOUND)
    {
        if(*(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF00) != 0x1050) //not winbond
        {
            Cap = INTTCGPEI_GETCAP(PeiServices);

        }else{  //winbond

            Cap = NuvotonProprietaryGetFlags(PeiServices);
        }

        if(ConfigFlags.TpmEnable != (~( Cap.disabled | Cap.deactivated )& BIT00))
        {
            return FALSE;
        }

        return TRUE;
    }

    Status = ReadOnlyVariable->PeiGetVariable(
                PeiServices,
                L"TcgINTPPI",
                &peiTcgEfiGlobalVariableGuid,
                &Attribs,
                &INTSize,
                &OwnerCap );

    if( OwnerCap != 0 )
    {
        return FALSE;
    }

    //if error RQST should stay 0
    if ((Temp.RQST & 0xFF) != 0 )
    {
        return FALSE;
    }

    if (ConfigFlags.TpmOperation )
    {
        return FALSE;
    }

    if(*(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF00) != 0x1050) //not winbond
    {
        Cap = INTTCGPEI_GETCAP(PeiServices);

    }else{  //winbond

        Cap = NuvotonProprietaryGetFlags(PeiServices);
    }

    if(ConfigFlags.TpmEnable != (~( Cap.disabled | Cap.deactivated )& BIT00))
    {
        if(ConfigFlags.PpiSetupSyncFlag == 0)
            {
                return FALSE;
            }
    }

    return TRUE;
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
