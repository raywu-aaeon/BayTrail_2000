//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgPlatformSetupPeiPolicy/TcgPlatformSetupPeiPolicy.c 3     12/18/11 10:24p Fredericko $
//
// $Revision: 3 $
//
// $Date: 12/18/11 10:24p $
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
// Name: TcgPlatformpeipolicy.c
//
// Description:	Installs Tcg policy from setup variables in Pei
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiPeiLib.h>
#include <Ppi/TcgPlatformSetupPeiPolicy.h>

EFI_GUID  gTcgPlatformSetupPolicyGuid = TCG_PLATFORM_SETUP_PEI_POLICY_GUID;
EFI_GUID  gTcgPeiInternalflagsGuid = PEI_TCG_INTERNAL_FLAGS_GUID;
EFI_GUID  gTcgInternalPeiSyncflagGuid = TCG_PPI_SYNC_FLAG_GUID;


EFI_STATUS
 getTcgPeiPolicy (IN EFI_PEI_SERVICES     **PeiServices ,
                  IN TCG_CONFIGURATION    *ConfigFlags)

{
  EFI_STATUS              Status;
  EFI_PEI_READ_ONLY_VARIABLE_PPI *ReadOnlyVariable;
  EFI_GUID gPeiReadOnlyVariablePpiGuid
                                  = EFI_PEI_READ_ONLY_VARIABLE_PPI_GUID;
  UINTN                   VariableSize = sizeof(SETUP_DATA);
  SETUP_DATA              SetupData;
  EFI_GUID                gSetupGuid = SETUP_GUID;
  UINT8                   DisallowTpmFlag=0;
  UINT8                   SyncVar = 0;
  UINTN                   SyncVarSize = sizeof(UINT8);

  //
  //
  //
  Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gPeiReadOnlyVariablePpiGuid,
                0, NULL,
                &ReadOnlyVariable);

  if(!EFI_ERROR(Status)){

    Status = ReadOnlyVariable->PeiGetVariable(PeiServices,
								L"Setup",
								&gSetupGuid,
								NULL,
								&VariableSize,
								&SetupData);

    if (EFI_ERROR(Status)) {
        ConfigFlags->TpmSupport           = 0;
        ConfigFlags->TcmSupport           = 0; 
        ConfigFlags->TpmEnable            = 0;
        ConfigFlags->TpmAuthenticate      = 0;
        ConfigFlags->TpmOperation         = 0;
    } else {
	    //EIP132975 >>
        // To distinguish the fTPM and D-TPM to be mutex. 
        if( SetupData.fTPM == 1 )
            ConfigFlags->TpmSupport = 0;
        else
            ConfigFlags->TpmSupport           = SetupData.TpmSupport;
		//EIP132975 << 
        ConfigFlags->TcmSupport           = SetupData.TcmSupport;
        ConfigFlags->TpmEnable            = SetupData.TpmEnable ;
        ConfigFlags->TpmAuthenticate      = SetupData.TpmAuthenticate;
        ConfigFlags->TpmOperation         = SetupData.TpmOperation;
    }
    
   
    VariableSize = sizeof(UINT8);
    Status = ReadOnlyVariable->PeiGetVariable(PeiServices,
								L"InternalDisallowTpmFlag",
								&gTcgPeiInternalflagsGuid,
								NULL,
								&VariableSize,
								&DisallowTpmFlag);
    if(EFI_ERROR(Status)){
        Status = EFI_SUCCESS;
        DisallowTpmFlag = 0;
    }

    Status = ReadOnlyVariable->PeiGetVariable(PeiServices,
                            L"TcgInternalSyncFlag",
                            &gTcgInternalPeiSyncflagGuid,
                            NULL,
                            &SyncVarSize,
                            &SyncVar);

    if(EFI_ERROR(Status)){
     SyncVar = 0;
    }  


    ConfigFlags->Reserved1                = 0;
    ConfigFlags->Reserved2                = 0;

    ConfigFlags->TpmHardware              = 0;
    ConfigFlags->TpmEnaDisable            = 0;
    ConfigFlags->TpmActDeact              = 0;
    ConfigFlags->TpmOwnedUnowned          = 0;
    ConfigFlags->TcgSupportEnabled        = 0;
    ConfigFlags->TpmError                 = 0;
    ConfigFlags->PpiSetupSyncFlag         = SyncVar;
    ConfigFlags->Reserved3                = 0;

    ConfigFlags->Reserved4              = 0;
    ConfigFlags->Reserved5              = 0;
  }else{
    ConfigFlags->TpmSupport               = 0;
    ConfigFlags->TcmSupport               = 0;
    ConfigFlags->TpmEnable                = 0 ;
    ConfigFlags->TpmAuthenticate          = 0;
    ConfigFlags->TpmOperation             = 0;
    ConfigFlags->DisallowTpm              = 0;
    ConfigFlags->Reserved1                = 0;
    ConfigFlags->Reserved2                = 0;

    ConfigFlags->TpmHardware              = 0;
    ConfigFlags->TpmEnaDisable            = 0;
    ConfigFlags->TpmActDeact              = 0;
    ConfigFlags->TpmOwnedUnowned          = 0;
    ConfigFlags->TcgSupportEnabled        = 0;
    ConfigFlags->TpmError                 = 0;
    ConfigFlags->PpiSetupSyncFlag         = 0;
    ConfigFlags->Reserved3                = 0;

    ConfigFlags->Reserved4              = 0;
    ConfigFlags->Reserved5              = 0;
  }

  return Status;

}



static TCG_PLATFORM_SETUP_INTERFACE   TcgPlatformSetupInstance = {
    TCG_PLATFORM_SETUP_PEI_PROTOCOL_REVISION_1,
    getTcgPeiPolicy
};

static EFI_PEI_PPI_DESCRIPTOR TcgPlatformSetupPeiPolicyDesc[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI
        | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gTcgPlatformSetupPolicyGuid,
        &TcgPlatformSetupInstance
    }
};



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcgPlatformSetupPolicyEntryPoint
//
// Description:  Entry point for TcgPlatformSetupPolicyEntryPoint
//
// Input:       ImageHandle       Image handle of this driver.
//              SystemTable       Global system service table.
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
EFIAPI
TcgPlatformSetupPeiPolicyEntryPoint (
  IN EFI_FFS_FILE_HEADER *FfsHeader,
  IN EFI_PEI_SERVICES    **PeiServices
)
{
  EFI_STATUS 	                Status;
  EFI_PEI_READ_ONLY_VARIABLE_PPI *ReadOnlyVariable;
  EFI_GUID gPeiReadOnlyVariablePpiGuid
                                  = EFI_PEI_READ_ONLY_VARIABLE_PPI_GUID;
  UINTN                          VariableSize = sizeof(SETUP_DATA);
  SETUP_DATA                     SetupData;
  EFI_GUID                       gSetupGuid = SETUP_GUID;


  Status = (*PeiServices)->LocatePpi(
                PeiServices,
                &gPeiReadOnlyVariablePpiGuid,
                0, NULL,
                &ReadOnlyVariable);

  if (EFI_ERROR(Status))
    return EFI_SUCCESS;

    Status = ReadOnlyVariable->PeiGetVariable(PeiServices,
								L"Setup",
								&gSetupGuid,
								NULL,
								&VariableSize,
								&SetupData);
  if(SetupData.fTPM == 0) Status = (**PeiServices).InstallPpi (PeiServices, TcgPlatformSetupPeiPolicyDesc); //EIP132975 
 
  return Status;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
