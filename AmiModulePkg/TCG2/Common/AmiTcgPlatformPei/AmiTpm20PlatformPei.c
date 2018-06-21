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
// $Header: /Alaska/SOURCE/Modules/TcgNext/Common/AmiTcgPlatform/AmiTcgPlatformPeiBeforeMem.c 1     10/08/13 12:04p Fredericko $
//
// $Revision: 1 $
//
// $Date: 10/08/13 12:04p $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/TcgNext/Common/AmiTcgPlatform/AmiTcgPlatformPeiBeforeMem.c $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiTcgPlatformPeiBeforeMem.c
//
// Description:	Function file for AmiTcgPlatformPeiBeforeMem
//
//<AMI_FHDR_END>
//*************************************************************************
#include <efi.h>
#include <AmiTcg\TcgCommon.h>
#include "AmiTcg\Sha.h"
#include <AmiTcg\TcgMisc.h>
#include <AmiTcg\TpmLib.h>
#include <token.h>
#include "PPI\TcgService.h"
#include "PPI\TpmDevice.h"
#include <Library/DebugLib.h>
#include <Ppi\ReadOnlyVariable2.h>
#include <Ppi\AmiTcgPlatformPpi.h>
#include <AmiTcg\AmiTcgPlatformPei.h>
#include <AmiTcg\Tpm20Pei.h>
#include <Library/BaseMemoryLib.h>

BOOLEAN CrbSupported();

EFI_STATUS
 Tpm20MeasureCRTMVersion(
    IN CONST EFI_PEI_SERVICES **PeiServices, 
    EFI_TREE_PPI *TrEEPeiPpi);

#if FTpmPlatformProfile == 1
extern EFI_GUID giTpmPpiGuid;
#endif
    
    EFI_FFS_FILE_HEADER *gFfsHeader;

//extern TPM20_MEASURE_CRTM_VERSION_PEI_FUNC_PTR  TPM20_MEASURE_CRTM_VERSION_PEI_FUNC_FUNCTION;
//TPM20_MEASURE_CRTM_VERSION_PEI_FUNC_PTR *Tpm20MeasureCRTMVersionFuncPtr = TPM20_MEASURE_CRTM_VERSION_PEI_FUNC_FUNCTION;

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InternalPeiBuildHobGuid
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
EFI_STATUS InternalPeiBuildHobGuid(
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
    
    DEBUG((-1, "Hob created \n")); 
    ((EFI_HOB_GUID_TYPE*)(*Hob))->Name = *Guid;

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI AmdPspAvailable(
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi );



#if FTpmPlatformProfile == 1
EFI_PEI_NOTIFY_DESCRIPTOR   AmdMemDiscCallback = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &giTpmPpiGuid,
    AmdPspAvailable
};
#endif




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
 Tpm20MeasureCRTMVersion(
    IN CONST EFI_PEI_SERVICES **PeiServices, 
    EFI_TREE_PPI *TrEEPeiPpi)
{
    TrEE_EVENT          *TrEEEventData;
    UINT32              EventSize = 0;
    EFI_GUID            CrtmVersion = CRTM_GUID;
    EFI_STATUS          Status;
    UINT8               *EventDataPtr;
    UINTN               Len;

    EventSize  = sizeof(EFI_GUID);
    
    Len =  sizeof(TrEE_EVENT_HEADER) +  sizeof(UINT32) + EventSize;
    
    Status = (*PeiServices)->AllocatePool(PeiServices, Len, &TrEEEventData);
    
    TrEEEventData->Size = Len;
            
    if(EFI_ERROR(Status))return Status;   
    TrEEEventData->Header.EventType  = EV_S_CRTM_VERSION;
    TrEEEventData->Header.HeaderSize = sizeof(TrEE_EVENT_HEADER);
    TrEEEventData->Header.HeaderVersion = 1; 
    TrEEEventData->Header.PCRIndex      = PCRi_CRTM_AND_POST_BIOS;
    
    EventDataPtr = (UINT8 *)TrEEEventData;

    EventDataPtr += sizeof(TrEE_EVENT_HEADER) + sizeof(UINT32);

    CopyMem(
        EventDataPtr,
        &CrtmVersion,
        EventSize
        );
    
    return TrEEPeiPpi->HashLogExtendEvent(PeiServices, TrEEPeiPpi,
            0, (EFI_PHYSICAL_ADDRESS)EventDataPtr, EventSize, TrEEEventData);
}



EFI_STATUS Tpm20PeiSendStartup(IN CONST EFI_PEI_SERVICES **PeiServices,
                               EFI_TREE_PPI *TrEEPeiPpi,
                               IN EFI_BOOT_MODE BootMode)  //EIP226550
{
    EFI_STATUS          Status = EFI_SUCCESS;
    TPM2_Startup_Cmd    StartupCmd;
    TPM2_Common_RespHdr StartupReponse;
    UINT32              ReturnSize = 0;
    //EFI_BOOT_MODE       BootMode;  //EIP226550
    UINT8               *CrbSupportOfst = (UINT8 *)(&StartupReponse);
    UINT8               i=0;
    AMI_TCG_PEI_FUNCTION_OVERRIDE_PPI       *PpiOverride;
    EFI_GUID            SkipTpmStartupGuid = AMI_SKIP_TPM_STARTUP_GUID;
    BOOLEAN             SkipTpmStartup = FALSE;
    EFI_HOB_GUID_TYPE       *Hob;
    Tpm20DeviceHob          *TrEEDeviceHob;
    EFI_GUID gTpm20Supporthobguid = TPM20_HOB_GUID;
    TCG_CONFIGURATION       ConfigFlags;
    TCG_PLATFORM_SETUP_INTERFACE *TcgPeiPolicy;
    EFI_GUID                        gTcgPeiPolicyGuid =\
                                        TCG_PLATFORM_SETUP_PEI_POLICY_GUID;

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
        
    Status = TcgPeiPolicy->getTcgPeiPolicy((EFI_PEI_SERVICES **)PeiServices, &ConfigFlags);
        
    if(EFI_ERROR(Status))return Status;

    if(TrEEPeiPpi == NULL)return EFI_INVALID_PARAMETER;
    
    StartupCmd.tag = (TPMI_ST_COMMAND_TAG)TPM_H2NS(TPM_ST_NO_SESSIONS);
    StartupCmd.CommandSize = TPM_H2NL((sizeof(TPM2_Startup_Cmd)));
    StartupCmd.CommandCode = TPM_H2NL(TPM_CC_Startup);

    //Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode); //EIP226550
    //if(EFI_ERROR(Status))return Status;    //EIP226550

    if(BootMode == BOOT_ON_S3_RESUME){
         StartupCmd.StartupType = TPM_H2NS(TPM_SU_STATE);
    }else{
         StartupCmd.StartupType = TPM_H2NS(TPM_SU_CLEAR);
    }

    ReturnSize = (UINT32)sizeof(StartupReponse);

    SetMem((UINT8 *)&StartupReponse,(UINTN)sizeof(StartupReponse), 0);

    DEBUG((-1, "sending TPM20 b4 TCGPassThroughToTpm \n"));
    Status = TrEEPeiPpi->SubmitCommand(TrEEPeiPpi,
                                        sizeof(TPM2_Startup_Cmd),
                                        (UINT8*)&StartupCmd,
                                        ReturnSize,
                                        (UINT8*)&StartupReponse);
    
    if(EFI_ERROR(Status))return Status;
    
    for (i=0; i<0x80; i++)
    {
        if(i%16 == 0 && i!=0){
            DEBUG((-1, "\n")); 
        }
        DEBUG((-1, " %02x", *CrbSupportOfst)); 
        CrbSupportOfst+=1;
    }

    if((StartupReponse.ResponseCode) != TPM_RC_SUCCESS){
          DEBUG((-1, "StartupReponse.ResponseCode = %x \n", StartupReponse.ResponseCode));
          Status = EFI_DEVICE_ERROR;
    }
    
	//EIP226550 >>
    if(BootMode == BOOT_ON_S3_RESUME){
        return Status;
    }
    //EIP226550 <<
    if(Status != EFI_DEVICE_ERROR && !EFI_ERROR(Status)){
        Status = InternalPeiBuildHobGuid((EFI_PEI_SERVICES **)PeiServices, &gTpm20Supporthobguid,
                           (sizeof(Tpm20DeviceHob)),  &Hob);
                    
        TrEEDeviceHob = (Tpm20DeviceHob*)(Hob + 1);
        TrEEDeviceHob->Tpm20DeviceState = 1; 
        if(!CrbSupported()){
            TrEEDeviceHob->InterfaceType = 1; 
        }else{
            TrEEDeviceHob->InterfaceType = 0;   
        }
    }else{
        Status = InternalPeiBuildHobGuid((EFI_PEI_SERVICES **)PeiServices, &gTpm20Supporthobguid,
                                   (sizeof(Tpm20DeviceHob)),  &Hob);
                            
        TrEEDeviceHob = (Tpm20DeviceHob*)(Hob + 1);
        TrEEDeviceHob->Tpm20DeviceState = 0; 
        TrEEDeviceHob->InterfaceType = ConfigFlags.InterfaceSel;
    }

    DEBUG((-1, "StartupReponse.Tag = %x \n", StartupReponse.tag));
    DEBUG((-1, "StartupReponse.Size = %x \n", StartupReponse.ResponsSize));
    DEBUG((-1, "StartupReponse.ResponseCode = %x \n", StartupReponse.ResponseCode));
    return Status;
}




EFI_STATUS
EFIAPI AmdPspAvailable(
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *Ppi )
{
    
    EFI_STATUS Status;
    EFI_GUID TrEEPeiGuid = AMI_PEI_TREE_SERVICE_PPI_GUID;
    EFI_TREE_PPI *TrEEPeiPpi = NULL;
    EFI_BOOT_MODE       BootMode;  //EIP226550
    
    Status = (*PeiServices)->LocatePpi(
                      PeiServices,
                      &TrEEPeiGuid,
                      0, NULL,
                      &TrEEPeiPpi);

    if(EFI_ERROR(Status) || TrEEPeiPpi == NULL )return Status;
    
	//EIP226550 >>
    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if(EFI_ERROR(Status))return Status;
      
    //send TPM 2.0 Startup
    Status = Tpm20PeiSendStartup(PeiServices, TrEEPeiPpi, BootMode);
    
    if(BootMode == BOOT_ON_S3_RESUME){
        return Status;
    }
    //EIP226550 <<
	
    Status = Tpm20MeasureCRTMVersion( PeiServices, TrEEPeiPpi);
    return Status; 
}





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
EFIAPI AmiTpm20PlatformPeiEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN CONST EFI_PEI_SERVICES    **PeiServices 
){
    EFI_STATUS Status;
    EFI_GUID TrEEPeiGuid = AMI_PEI_TREE_SERVICE_PPI_GUID;
    EFI_TREE_PPI *TrEEPeiPpi = NULL;
    EFI_BOOT_MODE       BootMode;  //EIP226550
    gFfsHeader = FfsHeader;
    
#if FTpmPlatformProfile == 1
    Status = (**PeiServices).NotifyPpi (PeiServices, &AmdMemDiscCallback);
    return EFI_SUCCESS;   
#else
    
    Status = (*PeiServices)->LocatePpi(
                    PeiServices,
                    &TrEEPeiGuid,
                    0, NULL,
                    &TrEEPeiPpi);

    if(EFI_ERROR(Status) || TrEEPeiPpi == NULL )return Status;
    
	//EIP226550 >>
    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if(EFI_ERROR(Status))return Status;
    
    //send TPM 2.0 Startup
    Status = Tpm20PeiSendStartup(PeiServices, TrEEPeiPpi, BootMode);
    
    if(BootMode == BOOT_ON_S3_RESUME){
        return Status;
    }
    //EIP226550 <<
	
    Status = Tpm20MeasureCRTMVersion( PeiServices, TrEEPeiPpi);
    return Status;
#endif
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
