/*++

This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor.  This file may be modified by the user, subject to
the additional terms of the license agreement

--*/

/*++
Copyright (c)  2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

 SeCPolicyInitDxe.c

Abstract:


--*/
#include "SeCPolicyInitDxe.h"

extern EFI_GUID gEfiSetupVariableGuid;
extern EFI_GUID gSeCPlatformReadyToBootGuid; // EIP142314

//
// TS DIMM thermal polling Smbus Address.
// This is platform specific.
//
UINT8 mTsDimmSmbusAddress[] = {0x30, 0x34};

DXE_SEC_POLICY_PROTOCOL  mDxePlatformSeCPolicy = { 0 };

SEC_OPERATION_PROTOCOL   mSeCOperationProtocol = {
    GetPlatformSeCInfo,
    SetPlatformSeCInfo,
    PerformSeCOperation
};

UINT8 HeciHmrfpoLockResult;

//
// Function implementations
//
EFI_STATUS
EFIAPI
SeCPlatformPolicyEntryPoint(
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
)
/*++

Routine Description:

  Entry point for the SeC Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
{
    EFI_STATUS                      Status;
    EFI_EVENT                       ReadyToBootEvent;
    SETUP_DATA			            SystemConfiguration;
    UINTN                           VarSize = sizeof(SETUP_DATA);
#if SecurityPkg_SUPPORT //EIP150790 
	UINT32                          MaxCommandSize;
  	UINT32                          MaxResponseSize;
#endif //EIP150790 
  
    DEBUG((EFI_D_ERROR, "SeCPlatformPolicyEntryPoint ++ "));

    mDxePlatformSeCPolicy.SeCConfig.TrConfig = AllocateZeroPool(sizeof(TR_CONFIG));

    //
    // ME DXE Policy Init
    //
    mDxePlatformSeCPolicy.Revision = DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_7;
    Status = gRT->GetVariable(L"Setup",	
                  &gEfiSetupVariableGuid,
                  NULL,
                  &VarSize,
                  &SystemConfiguration);
    //
    // Initialzie the Me Configuration
    //
    if(EFI_ERROR(Status))
    {
        mDxePlatformSeCPolicy.SeCConfig.EndOfPostEnabled        = 1;

    }else
    {
        mDxePlatformSeCPolicy.SeCConfig.EndOfPostEnabled        = SystemConfiguration.SeCEOPEnable;
    }
    mDxePlatformSeCPolicy.SeCConfig.HeciCommunication        = 1;
    mDxePlatformSeCPolicy.SeCConfig.SeCFwDownGrade           = 0;
    mDxePlatformSeCPolicy.SeCConfig.SeCLocalFwUpdEnabled     = 0;
    mDxePlatformSeCPolicy.SeCConfig.TrConfig->SMBusECMsgLen = TR_CONFIG_EC_MSG_LEN_20;
    mDxePlatformSeCPolicy.SeCConfig.TrConfig->SMBusECMsgPEC = TR_CONFIG_PEC_DISABLED;
    mDxePlatformSeCPolicy.SeCConfig.TrConfig->DimmNumber    = 2;
    mDxePlatformSeCPolicy.SeCConfig.TrConfig->SmbusAddress  = mTsDimmSmbusAddress;
    mDxePlatformSeCPolicy.SeCConfig.TrConfig->TrEnabled = 0;
    mDxePlatformSeCPolicy.SeCConfig.SeCFwImageType = INTEL_SEC_1_5MB_FW;
    mDxePlatformSeCPolicy.SeCConfig.PlatformBrand = INTEL_STAND_MANAGEABILITY_BRAND;
    mDxePlatformSeCPolicy.SeCReportError = ShowSeCReportError;
    mDxePlatformSeCPolicy.SeCPlatformHook = PlatformHook;

#if SecurityPkg_SUPPORT //EIP150790 
  Status = Tpm2RequestUseTpm ();
  if (!EFI_ERROR (Status)) {
    Status = Tpm2GetCapabilityMaxCommandResponseSize (&MaxCommandSize, &MaxResponseSize);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilityMaxCommandResponseSize fail!\n"));
    } else {
      mDxePlatformSeCPolicy.PttConfig.MaxCommandSize  = MaxCommandSize;
      mDxePlatformSeCPolicy.PttConfig.MaxResponseSize = MaxResponseSize;
      DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilityMaxCommandResponseSize - %08x, %08x\n", MaxCommandSize, MaxResponseSize));
    }
  }
#endif //EIP150790 
    //
    // Install the EFI_MANAGEMENT_ENGINE_PROTOCOL interface
    //
    Status = gBS->InstallMultipleProtocolInterfaces(
                 &ImageHandle,
                 &gDxePlatformSeCPolicyGuid,
                 &mDxePlatformSeCPolicy,
                 &gEfiSeCOperationProtocolGuid,
                 &mSeCOperationProtocol,
                 NULL
             );
    ASSERT_EFI_ERROR(Status);
    // EIP142314 After executing cmd "closemnf" of fpt.exe, POST time will be longer >>
    /*
    Status = EfiCreateEventReadyToBootEx(
                 TPL_CALLBACK,
                 SeCPolicyReadyToBootEvent,
                 (VOID *) &ImageHandle,
                 &ReadyToBootEvent
             );
    */
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    SeCPolicyReadyToBootEvent,
                    NULL,
                    &gSeCPlatformReadyToBootGuid,
                    &ReadyToBootEvent
                    );  
    // EIP142314 After executing cmd "closemnf" of fpt.exe, POST time will be longer <<
    ASSERT_EFI_ERROR(Status);

    DEBUG((EFI_D_ERROR, "SeCPlatformPolicyEntryPoint -- "));
    return Status;
}

VOID
ShowSeCReportError(
    IN SEC_ERROR_MSG_ID            MsgId
)
/*++

Routine Description:

  Show Me Error message.

Arguments:

  MsgId   Me error message ID.

Returns:

  None.

--*/
{
    gST->ConOut->ClearScreen(gST->ConOut);

    switch(MsgId) {
    case MSG_EOP_ERROR:
        gST->ConOut->OutputString(gST->ConOut, L"Error sending End Of Post message to ME, System HALT!\n");
        break;

    case MSG_SEC_FW_UPDATE_FAILED:
        gST->ConOut->OutputString(gST->ConOut, L"ME FW Update Failed, please try again!\n");
        break;

    case MSG_ASF_BOOT_DISK_MISSING:
        gST->ConOut->OutputString(gST->ConOut, L"Boot disk missing, please insert boot disk and press ENTER\r\n");
        break;

    case MSG_KVM_TIMES_UP:
        gST->ConOut->OutputString(gST->ConOut, L"Error!! Times up and the KVM session was cancelled!!");
        break;

    case MSG_KVM_REJECTED:
        gST->ConOut->OutputString(gST->ConOut, L"Error!! The request has rejected and the KVM session was cancelled!!");
        break;

    case MSG_HMRFPO_LOCK_FAILURE:
        gST->ConOut->OutputString(gST->ConOut, L"(A7) Me FW Downgrade - Request MeSpiLock Failed\n");
        break;

    case MSG_HMRFPO_UNLOCK_FAILURE:
        gST->ConOut->OutputString(gST->ConOut, L"(A7) Me FW Downgrade - Request MeSpiEnable Failed\n");
        break;

    case MSG_SEC_FW_UPDATE_WAIT:
        gST->ConOut->OutputString(
            gST->ConOut,
            L"Intel(R) Firmware Update is in progress. It may take up to 90 seconds. Please wait.\n"
        );
        break;

    case MSG_ILLEGAL_CPU_PLUGGED_IN:
        gST->ConOut->OutputString(
            gST->ConOut,
            L"\n\n\rAn unsupported CPU/PCH configuration has been identified.\n"
        );
        gST->ConOut->OutputString(
            gST->ConOut,
            L"\rPlease refer to the Huron River Platform Validation Matrix\n\rfor supported CPU/PCH combinations."
        );
        break;

    case MSG_KVM_WAIT:
        gST->ConOut->OutputString(gST->ConOut, L"Waiting Up to 8 Minutes For KVM FW.....");
        break;

    default:
        DEBUG((EFI_D_ERROR, "This Message Id hasn't been defined yet, MsgId = %x\n", MsgId));
        break;
    }

    gBS->Stall(HECI_MSG_DELAY);

}

VOID
PlatformHook (
  VOID
  )
/*++
Routine Description:

  Hook for Platform.

Arguments:

  VOID

Returns:

  Status.

--*/
{
#if SecurityPkg_SUPPORT //EIP150790 
  UINTN                         Index;
  TPML_ALG_PROPERTY             AlgList;
  TPM2B_AUTH                    NewAuth;
  EFI_GLOBAL_NVS_AREA           *mGlobalNvsAreaPtr;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsAreaProtocol;
  EFI_STATUS                    Status;
  
  Status = Tpm2RequestUseTpm ();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TPM not detected!\n"));
    return;
  }

  //
  // Send HierarchyChangeAuth command to TPM2
  //
  NewAuth.t.size = SHA1_DIGEST_SIZE;
  Status = Tpm2GetCapabilitySupportedAlg (&AlgList);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilitySupportedAlg fail!\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilitySupportedAlg - %08x\n", AlgList.count));
    for (Index = 0; Index < AlgList.count; Index++) {
      DEBUG ((EFI_D_ERROR, "alg - %x\n", AlgList.algProperties[Index].alg));
      switch (AlgList.algProperties[Index].alg) {
      case TPM_ALG_SHA1:
        if (NewAuth.t.size < SHA1_DIGEST_SIZE) {
          NewAuth.t.size = SHA1_DIGEST_SIZE;
        }
        break;
      case TPM_ALG_SHA256:
        if (NewAuth.t.size < SHA256_DIGEST_SIZE) {
          NewAuth.t.size = SHA256_DIGEST_SIZE;
        }
        break;
      case TPM_ALG_SHA384:
        if (NewAuth.t.size < SHA384_DIGEST_SIZE) {
          NewAuth.t.size = SHA384_DIGEST_SIZE;
        }
        break;
      case TPM_ALG_SHA512:
        if (NewAuth.t.size < SHA512_DIGEST_SIZE) {
          NewAuth.t.size = SHA512_DIGEST_SIZE;
        }
        break;
      case TPM_ALG_SM3_256:
        // TBD: Spec not define TREE_BOOT_HASH_ALG_SM3_256
        break;
      }
    }
  }

  RandomSeed (NULL, 0);
  RandomBytes (NewAuth.t.buffer, sizeof(NewAuth.t.buffer));
  Status = Tpm2HierarchyChangeAuth (TPM_RH_PLATFORM, NULL, &NewAuth);
  if (PcdGetBool(PcdFTPMNotRespond) || (EFI_ERROR(Status))) {
    DEBUG((EFI_D_ERROR, "Tpm2HierarchyChangeAuth failed!\n"));
    //
    // If TPM2_HierarchyChangeAuth fails with an error code other than TPM_RC_FAILURE, 
    // the BIOS should NOT publish the ACPI table
    //
    Status = gBS->LocateProtocol (&gEfiGlobalNvsAreaProtocolGuid, NULL, &GlobalNvsAreaProtocol);
    if (!EFI_ERROR (Status)) { 
      mGlobalNvsAreaPtr = GlobalNvsAreaProtocol->Area;
      mGlobalNvsAreaPtr->TpmEnable = 0;
    }
  }
//EIP150790 >>
#else
  DEBUG ((EFI_D_ERROR, "TPM 2.0 not supported!\n"));
  return;
#endif
//EIP150790 <<
}

VOID
HmrfpoEnable(
    VOID
)
{

    EFI_STATUS Status;
    UINT8      HeciHmrfpoEnableResult;
  CHAR16     *FailReason;
  EFI_INPUT_KEY      Key;
    HeciHmrfpoEnableResult  = HMRFPO_ENABLE_UNKNOWN_FAILURE;
    Status                  = HeciHmrfpoEnable(0, &HeciHmrfpoEnableResult);
    if(Status == EFI_SUCCESS && HeciHmrfpoEnableResult == HMRFPO_ENABLE_SUCCESS) {
        DEBUG((EFI_D_ERROR, "SeC FW Downgrade !!- Step A6\n"));

        HeciSendCbmResetRequest(CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
        CpuDeadLoop();
    }
    DEBUG((EFI_D_ERROR, "SeC FW Downgrade Error !!- Step A8, the Status is %r, The result is %x\n", Status, HeciHmrfpoEnableResult));
  if (EFI_ERROR(Status)) {
    FailReason = L"HECI COMMAND ERROR";
  } else {
    switch(HeciHmrfpoEnableResult) {
      case HMRFPO_ENABLE_LOCKED:
        FailReason = L"HMRFPO ENABLE LOCKED";
        break;
      case HMRFPO_NVAR_FAILURE:
        FailReason = L"HMRFPO NVAR FAILURE";
        break;
      case HMRFOP_ATP_POLICY:
        FailReason = L"HMRFPO ATP POLICY";
        break;
      default:
        FailReason = L"HMRFPO ENABLE UNKNOWN FAILURE";
        break;
    }
  }
  CreatePopUp (EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE, &Key, L"Hmrfpo Enable Failed Because", FailReason, L"Press Any Key To Reset System", NULL);
  HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
}

UINT32
GetSeCOpMode()
{
    HECI_FWS_REGISTER                   SeCFirmwareStatus;
    SeCFirmwareStatus.ul = HeciPciRead32(R_SEC_FW_STS0);

    DEBUG((EFI_D_ERROR, "R_SEC_FW_STS0 is %08x %x\n", SeCFirmwareStatus.ul, SeCFirmwareStatus.r.SeCOperationMode));
    return SeCFirmwareStatus.r.SeCOperationMode;
}

UINT32
GetSeCHMRFPOStatus()
{

    UINT8         HMRFPOStatus;
    HeciHmrfpoGetStatus(&HMRFPOStatus);
    if(HMRFPOStatus == 0) {
        return 0;
    } else if(HMRFPOStatus == 1) {
        return 0;
    }
    return 1;
}

UINT32
GetSeCFwUpdateStatus()
{
    UINT32 Result;
    HeciGetLocalFwUpdate(&Result);
    return Result;
}

EFI_STATUS
GetSeCFwVersion(
    SEC_VERSION_INFO *SeCVersion
)
{
    EFI_STATUS  Status;
    GEN_GET_FW_VER_ACK    MsgGenGetFwVersionAckData;
    Status = HeciGetFwVersionMsg(&MsgGenGetFwVersionAckData);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    SeCVersion->CodeMajor = MsgGenGetFwVersionAckData.Data.CodeMajor;
    SeCVersion->CodeMinor = MsgGenGetFwVersionAckData.Data.CodeMinor;
    SeCVersion->CodeHotFix = MsgGenGetFwVersionAckData.Data.CodeHotFix;
    SeCVersion->CodeBuildNo = MsgGenGetFwVersionAckData.Data.CodeBuildNo;
    return EFI_SUCCESS;
}

EFI_STATUS
GetSeCCapability(
    UINT32      *SeCCapability
)
{
    EFI_STATUS Status;
    GEN_GET_FW_CAPSKU       MsgGenGetFwCapsSku;
    GEN_GET_FW_CAPS_SKU_ACK MsgGenGetFwCapsSkuAck;
    Status = HeciGetFwCapsSkuMsg(&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    *SeCCapability = MsgGenGetFwCapsSkuAck.Data.FWCapSku.Data;
    return EFI_SUCCESS;
}

EFI_STATUS
GetSeCFeature(
    UINT32      *SeCFeature
)
{
    EFI_STATUS Status;
    SECFWCAPS_SKU            RuleData;
    Status = HeciGetFwFeatureStateMsg(&RuleData);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    *SeCFeature = RuleData.Data;
    return EFI_SUCCESS;
}

//(CSP20130313D+)>>
UINT32
GetSeCFsIntegrityVal()
{
    HECI_FWS1_REGISTER      SeCFirmwareStatus1;
    MFS_CORRUPTION_INFO     MfsCorruptionInfo;

    SeCFirmwareStatus1.ul = HeciPciRead32(R_SEC_FW_STS1);
    if(SeCFirmwareStatus1.r.MFS_CORRUPT != 0) {
        MfsCorruptionInfo.ul = HeciPciRead32(R_MFS_CORRUPTION_INFO);
        return MfsCorruptionInfo.ul;
    } else {
        return 0;
    }
}
//(CSP20130313D+)<<

UINT32
CheckSeCExist()
{
    UINT32 DeviceID;

    DeviceID = PchMmio32(PchPciDeviceMmBase(0x00, 0x1A, 0x00), 0x00);
    DEBUG((EFI_D_INFO, "DeviceID %08x\n", DeviceID));
    DeviceID = (DeviceID & 0xFFFF0000) >> 16;
    if(!(DeviceID >= S_SEC_DevID_RANGE_LO && DeviceID <= S_SEC_DevID_RANGE_HI)) {
        return 0;
    }
    //
    // Check for SEC MemValid status
    //
    if((HeciPciRead32(R_SEC_MEM_REQ) & B_SEC_MEM_REQ_INVALID) == B_SEC_MEM_REQ_INVALID) {
        //
        // SEC failed to start so no HECI
        //
        DEBUG((EFI_D_ERROR, "SEC Not Req Mem, tread as not exist.\n"));
        return 0;
    }

    return 1;
}

SEC_INFOMATION  mSeCInfo;
BOOLEAN         mSeCInfoInited = FALSE;
VOID
InitSeCInfo(
    VOID
)
{
    EFI_STATUS Status;
    mSeCInfo.SeCExist = CheckSeCExist();
    if(mSeCInfo.SeCExist == 0) {
        mSeCInfoInited = TRUE;
        return;
    }
    mSeCInfo.SeCOpMode = GetSeCOpMode();
    mSeCInfo.SeCEnable = (mSeCInfo.SeCOpMode == 0) ? 1 : 0;
    mSeCInfo.SeCOpEnable = (mSeCInfo.SeCOpMode == 0 || mSeCInfo.SeCOpMode == 3) ? 1 : 0;
    mSeCInfo.SeCFsIntegrityVal = GetSeCFsIntegrityVal();    //(CSP20130313D+)
    mSeCInfo.HmrfpoEnable = GetSeCHMRFPOStatus();
    mSeCInfo.FwUpdate= GetSeCFwUpdateStatus();
    Status = GetSeCFwVersion(&mSeCInfo.SeCVer);
    if(EFI_ERROR(Status)) {
        mSeCInfo.SeCVerValid = FALSE;
    } else {
        mSeCInfo.SeCVerValid = TRUE;
    }
    Status = GetSeCCapability(&mSeCInfo.SeCCapability);
    if(EFI_ERROR(Status)) {
        mSeCInfo.SeCCapabilityValid = FALSE;
    } else {
        mSeCInfo.SeCCapabilityValid = TRUE;
    }
    Status = GetSeCFeature(&mSeCInfo.SeCFeature);
    if(EFI_ERROR(Status)) {
        mSeCInfo.SeCFeatureValid = FALSE;
    } else {
        mSeCInfo.SeCFeatureValid = TRUE;
    }
    Status = HeciGetOemTagMsg(&mSeCInfo.SeCOEMTag);
    if(EFI_ERROR(Status)) {
        mSeCInfo.SeCOEMTagValid = FALSE;
    } else {
        mSeCInfo.SeCOEMTagValid = TRUE;
    }
    mSeCInfoInited = TRUE;
}



EFI_STATUS
GetPlatformSeCInfo(
    OUT SEC_INFOMATION * SeCInfo
)
{

    if(!mSeCInfoInited) {
        InitSeCInfo();
    }

    CopyMem(SeCInfo, &mSeCInfo, sizeof(SEC_INFOMATION));
    return EFI_SUCCESS;
}

EFI_STATUS
SetPlatformSeCInfo(
    IN  SEC_INFOMATION * SeCInfo
)
{
    if(mSeCInfo.SeCExist == 0) {
        return EFI_UNSUPPORTED;
    }
    if(mSeCInfo.SeCEnable != SeCInfo->SeCEnable) {
        if(SeCInfo->SeCEnable == 1) {
            HeciSetSeCEnableMsg();
        } else {
            HeciSetSeCDisableMsg(0);
        }
    }

    if(mSeCInfo.FwUpdate != SeCInfo->FwUpdate && SeCInfo->SeCEnable == 1) { //EIP175761
        HeciSetLocalFwUpdate((UINT8)SeCInfo->FwUpdate);
        mSeCInfo.FwUpdate = SeCInfo->FwUpdate;
    }

    if(mSeCInfo.HmrfpoEnable != SeCInfo->HmrfpoEnable && SeCInfo->SeCEnable == 1) { //EIP175761
        if(SeCInfo->HmrfpoEnable == 0) {
        } else {
            HmrfpoEnable();
        }
        CpuDeadLoop();
    }
    return EFI_SUCCESS;
}

EFI_STATUS
PerformSeCUnConfiguration(
    VOID
)
{
    EFI_STATUS      Status;
    UINT32	    	  CmdStatus;

    Status = HeciSeCUnconfigurationMsg(&CmdStatus);
    if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "HeciSetSeCEnableMsg Status = %r\n", Status));
        ASSERT_EFI_ERROR(Status);
        return Status;
    }

    DEBUG((EFI_D_ERROR, "Command Status = %r\n", CmdStatus));



    // wait for status
    Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
    DEBUG((EFI_D_ERROR, "HeciSeCUnconfigurationStatusMsg Status = %r CmdStatus = %x\n", Status, CmdStatus));
    while(CmdStatus == SEC_UNCONFIG_IN_PROGRESS) {
        Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
        DEBUG((EFI_D_ERROR, "HeciSeCUnconfigurationStatusMsg Status = %r CmdStatus = %x\n", Status, CmdStatus));
        if(EFI_ERROR(Status)) {
            DEBUG((EFI_D_ERROR, "HeciGetFwFeatureStateMsg Status = %r\n", Status));
            ASSERT_EFI_ERROR(Status);
            return Status;
        }
    }
    if(CmdStatus == SEC_UNCONFIG_SUCCESS) {
        // Send Global reset
        HeciSendCbmResetRequest(CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
        CpuDeadLoop();

    } else if(CmdStatus == SEC_UNCONFIG_ERROR) {
        DEBUG((EFI_D_ERROR, "Error ! UnConfiguration Error happened\n"));
        ASSERT_EFI_ERROR(Status);
        return Status;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
PerformCheckSeCUnConfiguration(
    VOID
)
{
    UINT32      CmdStatus;
    EFI_STATUS  Status;
    /*

          3.7.3.  SeC Unconfiguration - Common BIOS Flow On Boot
           On each boot, check SeC Unconfg status.
           While (SeC Unconfig Status == ME_UNCONFIG_IN_PROGRESS)
            o Wait for unconfigure completion.
           If (SeC Unconfig Status == ME_UNCONFIG_FINISHED)
            o Force a Global Reset.
           Else
            o Display error.
      */
    //    Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
    //    DEBUG ((EFI_D_ERROR, "UNCONFIGURATION Status = %r\n", CmdStatus));
    // EFI_DEADLOOP();
    Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    while(CmdStatus == SEC_UNCONFIG_IN_PROGRESS) {
        Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
        if(EFI_ERROR(Status)) {
            DEBUG((EFI_D_ERROR, "HeciGetFwFeatureStateMsg Status = %r\n", Status));
            ASSERT_EFI_ERROR(Status);
            return Status;
        }
    }

    if(CmdStatus == SEC_UNCONFIG_SUCCESS) {
        DEBUG((EFI_D_ERROR, "UNCONFIGURATION Status = SEC_UNCONFIG_SUCCESS\n"));
        // Send Global reset
        HeciSendCbmResetRequest(CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
        CpuDeadLoop();

    } else if(CmdStatus == SEC_UNCONFIG_ERROR) {
        DEBUG((EFI_D_ERROR, "Error ! UnConfiguration Error happened\n"));
        ASSERT_EFI_ERROR(Status);
        return Status;
    }

    return Status;

}

#define HMRFPO_B3_ENABLE

EFI_STATUS
SeCHmrfpoDisable(
    VOID
)
/*++
Routine Description:

  Send the HMRFPO_DISABLE MEI message.

Arguments:

  Event             - The event that triggered this notification function
  ParentImageHandle - Pointer to the notification functions context

Returns:

  Status.

--*/
{
#ifdef HMRFPO_B3_ENABLE
    UINT32  FWstatus;
    UINT32   WriteValue;
    UINT8   StallCount;
    //UINTN  Index;
    FWstatus    = 0;
    WriteValue  = 0;
    StallCount  = 0;

    DEBUG((EFI_D_ERROR, "(B3) SeC FW Downgrade - Send the HMRFPO_DISABLE MEI message\n"));

    WriteValue  = HeciPciRead32(R_GEN_STS);
    WriteValue  = WriteValue & BRNGUP_HMRFPO_DISABLE_CMD_MASK;
    WriteValue  = WriteValue | 0x30000000;
    DEBUG((EFI_D_ERROR, "SeC FW Downgrade Writing %x to register %x of PCI space\n", WriteValue, R_GEN_STS));
    //
    // Set the highest Byte of General Status Register (Bits 28-31)
    //
    HeciPciWrite32(R_GEN_STS, WriteValue);
    FWstatus = HeciPciRead32(R_SEC_FW_STS0);
    while
    (
        ((FWstatus & BRNGUP_HMRFPO_DISABLE_OVR_MASK) != BRNGUP_HMRFPO_DISABLE_OVR_RSP) &&
        (StallCount < FW_MSG_DELAY_TIMEOUT)
    ) {
        DEBUG((EFI_D_ERROR, "SeC FW Downgrade - SEC HMRFPO Disable Status = 0x%x\n", FWstatus));
        FWstatus = HeciPciRead32(R_SEC_FW_STS0);
        gBS->Stall(FW_MSG_DELAY);
        StallCount = StallCount + 1;
    }

    if((FWstatus & BRNGUP_HMRFPO_DISABLE_OVR_MASK) == BRNGUP_HMRFPO_DISABLE_OVR_RSP) {
        DEBUG((EFI_D_ERROR, "SeC FW Downgrade Disable Msg Received Successfully\n"));
    } else {
        DEBUG((EFI_D_ERROR, "SeC FW Downgrade Disable Msg ACK not Received\n"));

    }
    //
    // Hide SEC devices so we don't get a yellow bang in OS with disabled devices
    //

//    for(Index = 0; Index < 10; Index ++) {
//        DEBUG((EFI_D_ERROR, "wait %x\n", Index));

//        gBS->Stall(1000000);
//    }
//  gRT->ResetSystem (EfiResetWarm, EFI_SUCCESS, 0, NULL);
//  DisableAllSECDevices ();

// gBS->CloseEvent (Event);
#endif // HMRFPO_B3_ENABLE
    return EFI_SUCCESS;
}

EFI_STATUS
HmrfpoDisableCallBack(
    EFI_EVENT   Event,
    VOID        *Context
)
{
    return SeCHmrfpoDisable();
}

EFI_STATUS
PerformCheckHMRFPO(
    VOID
)
{
    EFI_STATUS        Status;
    EFI_HECI_PROTOCOL *Heci;
    UINT32            SeCMode;
    UINT32            SeCStatus;
    HECI_FWS_REGISTER SeCFirmwareStatus;


    SeCFirmwareStatus.ul = HeciPciRead32(R_SEC_FW_STS0);

    Status = gBS->LocateProtocol(
                 &gEfiHeciProtocolGuid,
                 NULL,
                 &Heci
             );
    ASSERT_EFI_ERROR(Status);

    Status = Heci->GetSeCMode(&SeCMode);
    ASSERT_EFI_ERROR(Status);

    Status = Heci->GetSeCStatus(&SeCStatus);
    ASSERT_EFI_ERROR(Status);
    //
    // (B1) Whcih mode ?
    //
    if (SeCMode != SEC_MODE_NORMAL){
      //
      // (B3) Call the HMRFPO_DISABLE
      //
        if((SeCFirmwareStatus.r.SeCOperationMode == SEC_OPERATION_MODE_SECOVR_HECI_MSG) &&
                (SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY)
          ) {
            EFI_EVENT HmrfpoDisableEvent;
            //SeCHmrfpoDisable();
            Status = EfiCreateEventReadyToBootEx(
                        TPL_CALLBACK,
                        HmrfpoDisableCallBack,
                        NULL,
                        &HmrfpoDisableEvent );
        }
    }
    return EFI_SUCCESS;
}

EFI_STATUS
PerformSeCOperation(
    IN  UINTN  SeCOpId
)
{
    if(CheckSeCExist() == 0) {
        return EFI_UNSUPPORTED;
    }
    switch(SeCOpId) {
    case SEC_OP_UNCONFIGURATION:
        return PerformSeCUnConfiguration();
        break;
    case SEC_OP_CHECK_UNCONFIG:
        return PerformCheckSeCUnConfiguration();
        break;
    case SEC_OP_CHECK_HMRFPO:
        return PerformCheckHMRFPO();
        break;
    default:
        ASSERT(FALSE);
        break;
    }


    return EFI_SUCCESS;
}

EFI_STATUS
SeCPolicyReadyToBootEvent(
    EFI_EVENT           Event,
    VOID                *ParentImageHandle
)
/*++
Routine Description:

  Signal a event for SeC ready to boot.

Arguments:

  Event             - The event that triggered this notification function
  ParentImageHandle - Pointer to the notification functions context

Returns:

  Status.

--*/
{
    EFI_HECI_PROTOCOL *Heci;
    EFI_STATUS        Status;
    UINT32            SeCMode;
    UINT32            SeCStatus;
    SETUP_DATA        SystemConfiguration;
    UINTN             VarSize;
    HECI_FWS_REGISTER SeCFirmwareStatus;
    UINT32            FactoryDefaultBase;
    UINT32            FactoryDefaultLimit;
    UINT64            Nonce;
    UINT32            Attributes = 0; //EIP168675
  
  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);
  VarSize = sizeof(SETUP_DATA);

    DEBUG((EFI_D_ERROR, "SeCPolicyReadyToBootEvent ++\n"));


    Status = gBS->LocateProtocol(
                 &gEfiHeciProtocolGuid,
                 NULL,
                 &Heci
             );
    if(EFI_ERROR(Status)) {
        return Status;
    }

    Status = Heci->GetSeCMode(&SeCMode);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    Status = Heci->GetSeCStatus(&SeCStatus);
    if(EFI_ERROR(Status)) {
        return Status;
    }

  if (SeCMode == SEC_MODE_NORMAL) {
      //
      // (A7) The BIOS sends the HMRFPO Lock MEI message and continues the normal boot
      //
      HeciHmrfpoLockResult = HMRFPO_LOCK_SUCCESS;
      //
      // The SEC firmware will ignore the HMRFPO LOCK command if SEC is in SEC manufacturing mode
      //
      if ((SeCFirmwareStatus.r.ManufacturingMode == 0) &&
          ((SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_IN_RECOVERY_MODE) || 
           (SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY))) {
  
        DEBUG ((EFI_D_ERROR, "(A7) SeC FW Downgrade - The BIOS sends the HMRFPO Lock SECI message and continues the normal boot\n"));
  
        FactoryDefaultBase  = 0;
        FactoryDefaultLimit = 0;
        Status              = HeciHmrfpoLock (&Nonce, &FactoryDefaultBase, &FactoryDefaultLimit, &HeciHmrfpoLockResult);
        if (Status != EFI_SUCCESS) {
          HeciHmrfpoLockResult = HMRFPO_LOCK_FAILURE;
        }
      }
  }
    if((SeCMode == SEC_MODE_NORMAL) &&(SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY)) {
        if(mDxePlatformSeCPolicy.SeCConfig.EndOfPostEnabled == 1) {
            mSeCInfo.SeCEOPDone = 1;
            Status = gRT->GetVariable(
	                          L"Setup",
	                          &gEfiSetupVariableGuid,
	                          &Attributes, //EIP168675
	                          &VarSize,
	                          &SystemConfiguration
	                          );	  
            ASSERT_EFI_ERROR(Status);
            SystemConfiguration.SeCEOPDone = 1;

            Status = gRT->SetVariable (
	                          L"Setup",
	                          &gEfiSetupVariableGuid,
	                          Attributes, //EIP168675
	                          sizeof(SETUP_DATA),
	                          &SystemConfiguration
	                          );
        } 
    } 
    ASSERT_EFI_ERROR(Status);

    gBS->CloseEvent(Event);

    DEBUG((EFI_D_ERROR, "SeCPolicyReadyToBootEvent --\n"));
    return EFI_SUCCESS;
}

