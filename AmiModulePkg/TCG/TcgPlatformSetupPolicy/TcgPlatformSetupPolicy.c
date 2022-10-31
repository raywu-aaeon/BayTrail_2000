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
// $Header: /Alaska/SOURCE/Modules/TCG/TcgPlatformSetupPolicy/TcgPlatformSetupPolicy.c 6     12/15/11 3:30p Fredericko $
//
// $Revision: 6 $
//
// $Date: 12/15/11 3:30p $
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
// Name: TcgPlatformSetupPolicy.c
//
// Description:	Policy file to allow reading and update of TCG policy
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiDxeLib.h>
#include <Protocol\TcgPlatformSetupPolicy.h>

TCG_PLATFORM_SETUP_PROTOCOL *TcgPlatformSetupInstance  = NULL;

EFI_HANDLE gImageHandle;
static TCG_CONFIGURATION  InitialConfigFlags;
EFI_GUID  gTcgInternalSyncflagGuid = TCG_PPI_SYNC_FLAG_GUID;
EFI_GUID  gTcgInternalflagsGuid = TCG_INTERNAL_FLAGS_GUID;


EFI_STATUS
 UpdateTcgStatusFlags (TCG_CONFIGURATION *StatusFlags, BOOLEAN UpdateNvram)

{
  EFI_STATUS              Status;
  UINTN                   VariableSize  = sizeof(SETUP_DATA);
  SETUP_DATA              SetupDataBuffer;
  UINTN                   SetupVariableSize = sizeof(SETUP_DATA);
  UINT32                  SetupVariableAttributes;
  EFI_GUID                gSetupGuid = SETUP_GUID;
  TCG_PLATFORM_SETUP_PROTOCOL *NewTcgPlatformSetupInstance  = NULL;
  TCG_PLATFORM_SETUP_PROTOCOL *OldTcgPlatformSetupInstance  = NULL;

  if(InitialConfigFlags.DisallowTpm == 1)return EFI_INVALID_PARAMETER;

  Status = pBS->LocateProtocol (&gTcgPlatformSetupPolicyGuid,  NULL, &OldTcgPlatformSetupInstance);
  if (EFI_ERROR (Status)) {
      return Status;
  }

  Status = pBS->AllocatePool (
              EfiBootServicesData,
              sizeof (TCG_PLATFORM_SETUP_PROTOCOL),
              (VOID**)&NewTcgPlatformSetupInstance
           );

  if(StatusFlags == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  if(TcgPlatformSetupInstance == NULL) return EFI_OUT_OF_RESOURCES;

  NewTcgPlatformSetupInstance->ConfigFlags.TpmSupport               = InitialConfigFlags.TpmSupport;
  NewTcgPlatformSetupInstance->ConfigFlags.TcmSupport               = StatusFlags->TcmSupport;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmEnable                = StatusFlags->TpmEnable ;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmAuthenticate          = InitialConfigFlags.TpmAuthenticate;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmOperation             = StatusFlags->TpmOperation;
  NewTcgPlatformSetupInstance->ConfigFlags.DisallowTpm              = 0;
  NewTcgPlatformSetupInstance->ConfigFlags.Reserved1                = StatusFlags->Reserved1;
  NewTcgPlatformSetupInstance->ConfigFlags.Reserved2                = StatusFlags->Reserved2;

  NewTcgPlatformSetupInstance->ConfigFlags.TpmHardware              = StatusFlags->TpmHardware;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmEnaDisable            = StatusFlags->TpmEnaDisable;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmActDeact              = StatusFlags->TpmActDeact;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmOwnedUnowned          = StatusFlags->TpmOwnedUnowned;
  NewTcgPlatformSetupInstance->ConfigFlags.TcgSupportEnabled        = StatusFlags->TcgSupportEnabled ;
  NewTcgPlatformSetupInstance->ConfigFlags.TpmError                 = StatusFlags->TpmError;
  NewTcgPlatformSetupInstance->ConfigFlags.PpiSetupSyncFlag         = StatusFlags->PpiSetupSyncFlag;
  NewTcgPlatformSetupInstance->ConfigFlags.Reserved3                = StatusFlags->Reserved3;

  NewTcgPlatformSetupInstance->ConfigFlags.Reserved4              = StatusFlags->Reserved4;
  NewTcgPlatformSetupInstance->ConfigFlags.Reserved5              = StatusFlags->Reserved5;

  NewTcgPlatformSetupInstance->UpdateStatusFlags = UpdateTcgStatusFlags;

  Status = pBS->UninstallMultipleProtocolInterfaces(
               gImageHandle,
               &gTcgPlatformSetupPolicyGuid,
               OldTcgPlatformSetupInstance,      
               NULL
           );

  Status = pBS->InstallMultipleProtocolInterfaces (
               &gImageHandle,
               &gTcgPlatformSetupPolicyGuid,      
               NewTcgPlatformSetupInstance,
               NULL
           );

  if (EFI_ERROR (Status)) {
        return Status;
      }

  if(UpdateNvram){

        Status = pRS->GetVariable (
                            L"Setup",
                            &gSetupGuid,
                            &SetupVariableAttributes,
                            &SetupVariableSize,
                            &SetupDataBuffer);

       SetupDataBuffer.TpmEnable            =   NewTcgPlatformSetupInstance->ConfigFlags.TpmEnable;
       SetupDataBuffer.TpmSupport           =   NewTcgPlatformSetupInstance->ConfigFlags.TpmSupport;
       SetupDataBuffer.TcmSupport           =   NewTcgPlatformSetupInstance->ConfigFlags.TcmSupport;   
       SetupDataBuffer.TpmAuthenticate      =   NewTcgPlatformSetupInstance->ConfigFlags.TpmAuthenticate;
       SetupDataBuffer.TpmOperation         =   NewTcgPlatformSetupInstance->ConfigFlags.TpmOperation;
       SetupDataBuffer.TpmEnaDisable        =   NewTcgPlatformSetupInstance->ConfigFlags.TpmEnaDisable;
       SetupDataBuffer.TpmActDeact          =   NewTcgPlatformSetupInstance->ConfigFlags.TpmActDeact;
       SetupDataBuffer.TpmHrdW              =   NewTcgPlatformSetupInstance->ConfigFlags.TpmHardware;
       SetupDataBuffer.TpmOwnedUnowned      =   NewTcgPlatformSetupInstance->ConfigFlags.TpmOwnedUnowned;
       SetupDataBuffer.TpmError             =   NewTcgPlatformSetupInstance->ConfigFlags.TpmError;
       SetupDataBuffer.TcgSupportEnabled    =   NewTcgPlatformSetupInstance->ConfigFlags.TcgSupportEnabled;

       SetupVariableAttributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |EFI_VARIABLE_RUNTIME_ACCESS |
                                 EFI_VARIABLE_NON_VOLATILE;

       Status = pRS->SetVariable (
                          L"Setup",
                          &gSetupGuid,
                          SetupVariableAttributes,
                          SetupVariableSize,
                          &SetupDataBuffer);   

       Status = pRS->SetVariable (
                          L"TcgInternalSyncFlag",
                          &gTcgInternalSyncflagGuid,
                          SetupVariableAttributes,
                          sizeof(UINT8),
                          &(NewTcgPlatformSetupInstance->ConfigFlags.PpiSetupSyncFlag));   
  }

  return Status;
}



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
TcgPlatformSetupPolicyEntryPoint (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
  EFI_STATUS 	                Status;
  UINTN                         VariableSize  = 0;
  SETUP_DATA                    SetupDataBuffer;
  SETUP_DATA                   *SetupData = &SetupDataBuffer;
  UINTN                         SetupVariableSize;
  UINT32                        SetupVariableAttributes;
  EFI_GUID                      gSetupGuid = SETUP_GUID;
  UINT8                         SyncVar;
  UINT8                         DisallowTpmFlag;
  UINTN                         TempSizeofSyncVar = sizeof(UINT8);

  InitAmiLib (ImageHandle, SystemTable);

  SetupVariableSize = sizeof (SETUP_DATA);

  Status = pBS->AllocatePool (
              EfiBootServicesData,
              sizeof (TCG_PLATFORM_SETUP_PROTOCOL),
              (VOID**)&TcgPlatformSetupInstance
           );


  if (EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = pRS->GetVariable (
                            L"InternalDisallowTpmFlag",
                            &gTcgInternalflagsGuid,
                            &SetupVariableAttributes,
                            &TempSizeofSyncVar,
                            &DisallowTpmFlag);


  if(EFI_ERROR(Status)){
    DisallowTpmFlag = 0;
    Status = EFI_SUCCESS;
  }  
   else if(DisallowTpmFlag == 1)
  {
        MemSet(&TcgPlatformSetupInstance->ConfigFlags, sizeof(TCG_CONFIGURATION), 0);
        TcgPlatformSetupInstance->ConfigFlags.DisallowTpm              = 1;
        MemSet(&InitialConfigFlags, sizeof(TCG_CONFIGURATION), 0);

        InitialConfigFlags.DisallowTpm = 1;

        TcgPlatformSetupInstance->Revision = TCG_PLATFORM_SETUP_PROTOCOL_REVISION_1;
        TcgPlatformSetupInstance->UpdateStatusFlags = UpdateTcgStatusFlags;      

        Status = pRS->GetVariable (
                            L"Setup",
                            &gSetupGuid,
                            &SetupVariableAttributes,
                            &SetupVariableSize,
                            &SetupDataBuffer);


        SetupDataBuffer.TpmEnable            =   0;
        SetupDataBuffer.TpmSupport           =   0;
        SetupDataBuffer.TcmSupport           =   0;   
        SetupDataBuffer.TpmAuthenticate      =   0;
        SetupDataBuffer.TpmOperation         =   0;
        SetupDataBuffer.TpmEnaDisable        =   0;
        SetupDataBuffer.TpmActDeact          =   0;
        SetupDataBuffer.TpmHrdW              =   0;
        SetupDataBuffer.TpmOwnedUnowned      =   0;
        SetupDataBuffer.TpmError             =   0;
        SetupDataBuffer.SuppressTcg          =   DisallowTpmFlag;
        SetupDataBuffer.TcgSupportEnabled    =   0;

        SetupVariableAttributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |EFI_VARIABLE_RUNTIME_ACCESS |
                                   EFI_VARIABLE_NON_VOLATILE;

        Status = pRS->SetVariable (
                      L"Setup",
                      &gSetupGuid,
                      SetupVariableAttributes,
                      SetupVariableSize,
                      &SetupDataBuffer);

        return Status;
  }


  Status = pRS->GetVariable (
                            L"TcgInternalSyncFlag",
                            &gTcgInternalSyncflagGuid,
                            &SetupVariableAttributes,
                            &TempSizeofSyncVar,
                            &SyncVar);

  if(EFI_ERROR(Status)){
    SyncVar = 0;
  }  


  Status = pRS->GetVariable (
                            L"Setup",
                            &gSetupGuid,
                            &SetupVariableAttributes,
                            &SetupVariableSize,
                            &SetupDataBuffer);

  TcgPlatformSetupInstance->Revision = TCG_PLATFORM_SETUP_PROTOCOL_REVISION_1;

  if (EFI_ERROR(Status)) 
  {
    MemSet(&TcgPlatformSetupInstance->ConfigFlags, sizeof(TCG_CONFIGURATION), 0);
    SetupData = NULL;
  } else {

    MemSet(&TcgPlatformSetupInstance->ConfigFlags, sizeof(TCG_CONFIGURATION), 0);
	//EIP132975 >>
    if( SetupData->fTPM == 1 )
        TcgPlatformSetupInstance->ConfigFlags.TpmSupport = 0;
    else
        TcgPlatformSetupInstance->ConfigFlags.TpmSupport       = (SetupData->TpmSupport);
	//EIP132975 <<
    TcgPlatformSetupInstance->ConfigFlags.TcmSupport       = (SetupData->TcmSupport);
    TcgPlatformSetupInstance->ConfigFlags.TpmEnable        = (SetupData->TpmEnable);
    TcgPlatformSetupInstance->ConfigFlags.TpmAuthenticate  = (SetupData->TpmAuthenticate);
    TcgPlatformSetupInstance->ConfigFlags.TpmOperation             = (SetupData->TpmOperation);
    TcgPlatformSetupInstance->ConfigFlags.PpiSetupSyncFlag = SyncVar;
    TcgPlatformSetupInstance->ConfigFlags.Reserved1        = 0;
    TcgPlatformSetupInstance->ConfigFlags.Reserved2        = 0;
    TcgPlatformSetupInstance->ConfigFlags.DisallowTpm      = DisallowTpmFlag;
    TcgPlatformSetupInstance->ConfigFlags.TpmHardware     = (SetupData->TpmHrdW);
    TcgPlatformSetupInstance->ConfigFlags.TpmEnaDisable   = (SetupData->TpmEnaDisable);
    TcgPlatformSetupInstance->ConfigFlags.TpmActDeact     = (SetupData->TpmActDeact);
    TcgPlatformSetupInstance->ConfigFlags.TpmOwnedUnowned     = (SetupData->TpmOwnedUnowned);
    TcgPlatformSetupInstance->ConfigFlags.TcgSupportEnabled   = (SetupData->TcgSupportEnabled);
    TcgPlatformSetupInstance->ConfigFlags.TpmError        = (SetupData->TpmError);
    TcgPlatformSetupInstance->ConfigFlags.Reserved3 = 0;
    TcgPlatformSetupInstance->ConfigFlags.Reserved4 = 0;
    TcgPlatformSetupInstance->ConfigFlags.Reserved5 = 0;
  }

  TcgPlatformSetupInstance->UpdateStatusFlags = UpdateTcgStatusFlags;

   InitialConfigFlags.TpmSupport               =     TcgPlatformSetupInstance->ConfigFlags.TpmSupport;
   InitialConfigFlags.TcmSupport               =     TcgPlatformSetupInstance->ConfigFlags.TcmSupport;
   InitialConfigFlags.TpmEnable                =     TcgPlatformSetupInstance->ConfigFlags.TpmEnable;
   InitialConfigFlags.TpmAuthenticate          =     TcgPlatformSetupInstance->ConfigFlags.TpmAuthenticate;
   InitialConfigFlags.TpmOperation             =     TcgPlatformSetupInstance->ConfigFlags.TpmOperation;
   InitialConfigFlags.Reserved1                =     TcgPlatformSetupInstance->ConfigFlags.Reserved1 ;
   InitialConfigFlags.Reserved2                =     TcgPlatformSetupInstance->ConfigFlags.Reserved2;

   InitialConfigFlags.TpmHardware              = TcgPlatformSetupInstance->ConfigFlags.TpmHardware;
   InitialConfigFlags.TpmEnaDisable            = TcgPlatformSetupInstance->ConfigFlags.TpmEnaDisable;
   InitialConfigFlags.TpmActDeact              = TcgPlatformSetupInstance->ConfigFlags.TpmActDeact;
   InitialConfigFlags.TpmOwnedUnowned          = TcgPlatformSetupInstance->ConfigFlags.TpmOwnedUnowned;
   InitialConfigFlags.TcgSupportEnabled        = TcgPlatformSetupInstance->ConfigFlags.TcgSupportEnabled;
   InitialConfigFlags.TpmError                 = TcgPlatformSetupInstance->ConfigFlags.TpmError;
   InitialConfigFlags.PpiSetupSyncFlag         = TcgPlatformSetupInstance->ConfigFlags.PpiSetupSyncFlag;
   InitialConfigFlags.Reserved3                = TcgPlatformSetupInstance->ConfigFlags.Reserved3;

   InitialConfigFlags.Reserved4              = TcgPlatformSetupInstance->ConfigFlags.Reserved4;
   InitialConfigFlags.Reserved5              = TcgPlatformSetupInstance->ConfigFlags.Reserved5;

  //
  //
  Status = pBS->InstallMultipleProtocolInterfaces (
               &ImageHandle,
               &gTcgPlatformSetupPolicyGuid,      
               TcgPlatformSetupInstance,
               NULL
           );

  gImageHandle  = ImageHandle;

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
