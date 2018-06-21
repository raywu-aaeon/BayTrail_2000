/*++

Copyright (c) 2004-2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTAm.c

Abstract:

  TDT authetication module for using TDT DXE driver.
  This driver uses the TDT protocol, HECI Protocol and TDT Platform Policy to implement Theft
  Deterrence Technology AM module.

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/


//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//
#define SEC_ALERT_AT_HANDLER_GUID {0xb441df87, 0x8d94, 0x4811, 0x85, 0xf7, 0xf, 0x9a, 0x7b, 0xf8, 0x9d, 0x2a}

#include "TdtAm.h"
#include <Guid/Vlv2Variable.h>

DXE_TDT_POLICY_PROTOCOL         *pTdtPlatformPolicy;

EFI_EVENT  gAlertAtHandlerEvent;

EFI_GUID   gAlertAtHandlerGuid = SEC_ALERT_AT_HANDLER_GUID;

static TDT_BIOS_RECOVERY_CONFIG           gBiosRecoveryGlobalVar;
static AT_STATE_STRUCT                    gAtStateGlobalVar;
static UINT16 gRecoveryAMGlobalVar;

STATIC 
UINTN 
PasswordType (
  UINT32 TimeLeft,
  UINT8 LastTrigger
  )

/*++

Routine Description:
  This routine detects the password type selected by user.

Arguments:
  TimeLeft - Time Left to enter password
  LastTrigger - Reason for AT stolen state

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;


TDT_PERFORM_OPERATION_ID mPerformOperation = TDT_PERFORM_NOTHING;
UINTN        mStolenBootCount;
UINTN        mNotifyDelayTime;


#ifdef EFI_DEBUG

VOID
ShowBuffer(
  UINT8 *Message,
  UINT32 Length
)
{
  UINT32  LineBreak;
  UINT32  Index;
  LineBreak = 0;
  Index     = 0;

  while(Length-- > 0) {
    if(LineBreak == 0) {
      DEBUG((EFI_D_ERROR, "%02x: ", (Index & 0xF0)));
    }

    DEBUG((EFI_D_ERROR, "%02x ", Message[Index++]));
    LineBreak++;
    if(LineBreak == 16) {
      DEBUG((EFI_D_ERROR, "\n"));
      LineBreak = 0;
    }
    if(LineBreak == 8) {
      DEBUG((EFI_D_ERROR, "- "));
    }
  }
  DEBUG((EFI_D_ERROR, "\n"));
  return;
}

#endif  // End Of EFI_DEBUG

UINTN
EFIAPI
StrLenUnaligned (
  IN      CONST CHAR16              *String
)
{
  UINTN                             Length;

  for (Length = 0; *String != L'\0'; String++, Length++) {
  }
  return Length;
}

/*++

Routine Description:

  UI for getting Suspend mode authentication

Arguments:

  pTdt                    Pointer to AT (TDT) protocol
  SrtkPass              Pointer to OTN that user enters - required for authentication

Returns:

  EFI_SUCCESS           Initialization complete.

--*/

EFI_STATUS 
GetSuspendAuthentication(
  EFI_TDT_PROTOCOL *pTdt,
  UINT8            *SuspendToken,
  UINT8            *SrtkPass
  )
{

  UINTN               NonceLength = NONCE_LENGTH;
  UINTN               StrNonceLength = STR_NONCE_LENGTH;
  UINT8               Nonce[NONCE_LENGTH];
  UINT8               NonceStr[STR_NONCE_LENGTH];
  EFI_STATUS          Status = EFI_SUCCESS;
  CHAR16              *UniCodeNonceStr;
  INTN                StrIndex = 0;
  UINT8               BackSpace = 0;
  CHAR16              StarChar[2] = { L' ', L'\0'};
  EFI_INPUT_KEY       Key = {0, 0};

  UniCodeNonceStr = AllocatePool((STR_NONCE_LENGTH + 1) * sizeof(CHAR16));
  SetMem( UniCodeNonceStr, (STR_NONCE_LENGTH + 1)*sizeof(CHAR16), 0 );

  Status = pTdt->GetNonce(pTdt, Nonce);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "GetSuspendAuthentication::GetNonce failed, Status = %r\n", Status));
    gST->ConOut->OutputString(gST->ConOut, L"\r\n ERROR: Could Not Retrieve Nonce from FW");
    return Status;
  }

  Base32Encode(NonceStr, &StrNonceLength , Nonce, NonceLength);
  Uint8ToUnicode(NonceStr, UniCodeNonceStr);
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Refer the following Platform Recovery ID to IT: ");
  gST->ConOut->OutputString(gST->ConOut, UniCodeNonceStr);
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Enter Server based Suspend Token : ");

  do {
    Status = gST->ConIn->ReadKeyStroke( gST->ConIn, &Key );
    if(!EFI_ERROR(Status)) {
      switch (Key.UnicodeChar) {
        case CHAR_NULL:
          if (Key.ScanCode == SCAN_ESC) {
            Status = gST->ConIn->Reset( gST->ConIn, FALSE );
            SrtkPass[0] = L'\0';
            StrIndex = 0;
            return EFI_ABORTED;
          }
          break;
        case CHAR_CARRIAGE_RETURN: // When pressed entered then return with the password as successful
          Status = gST->ConIn->Reset( gST->ConIn, FALSE );
          gST->ConOut->EnableCursor(gST->ConOut, FALSE);
          DEBUG ((EFI_D_ERROR, "TDT::Suspend SRTK Entered : %s\n", SrtkPass));
          SrtkPass[StrIndex] = L'\0';
          StrIndex = 0;
          gST->ConOut->OutputString(gST->ConOut, L"\r\n    Checking Authentication ..");
          if(AsciiStrLen((CHAR8 *) SrtkPass) == 32) {
            DEBUG ((EFI_D_ERROR, "Suspend Password entered is in Base32\n"));
            Base32Decode(SrtkPass, SuspendToken);
          } else if(AsciiStrLen((CHAR8 *) SrtkPass) > 40) {
            DEBUG ((EFI_D_ERROR, "Password entered is in Base10\n"));
            DecimalToHexString(SrtkPass, SuspendToken, MAX_HEX_BYTES - 1);
          } else
            return EFI_INVALID_PARAMETER;
          return EFI_SUCCESS;
        case CHAR_BACKSPACE:
          if (StrIndex > 0 ) {
            BackSpace = 1;
            StrIndex--;
            SrtkPass[StrIndex] = L'\0';
            StarChar[0] = L'\b';
          }
          break;
        default:
          if (StrIndex == TDTAM_SETUP_PASSWORD_LENGTH) {
            break;//Do Nothing
          }
          StarChar[0] = (UINT8) Key.UnicodeChar;
          SrtkPass[StrIndex] = (UINT8) Key.UnicodeChar;
          StrIndex++;
          break;
      } //switch (Key.UnicodeChar)
      //BackSpace handling is bit tricky ... here it is
      if (BackSpace == 1) {
        BackSpace = 0;
        StarChar[0] = L'\b';
        gST->ConOut->OutputString(gST->ConOut, StarChar);
        StarChar[0] = L' ';
        gST->ConOut->OutputString(gST->ConOut, StarChar);
        StarChar[0] = L'\b';
        gST->ConOut->OutputString(gST->ConOut, StarChar);
      } else if (StrIndex < 0) {
        // should never come here ...
        gST->ConOut->EnableCursor(gST->ConOut, FALSE);
        gST->ConOut->SetCursorPosition( gST->ConOut, gST->ConOut->Mode->CursorColumn, gST->ConOut->Mode->CursorRow);
        StrIndex = 0;
      } else if (StrIndex > 0) {
        //printing the "*" for each characters entered
        gST->ConOut->OutputString(gST->ConOut, StarChar);
      }
    }//if(!EFI_ERROR(Status)) - if no error is returned from ConIn
  } while( 1 );
  return EFI_SUCCESS;
}

EFI_STATUS
GetAtStateInfoII(
  IN OUT UINT8                 *AtState,
  IN OUT UINT8                 *AtLastTheftTrigger,
  IN OUT UINT16                *AtLockState,
  IN OUT UINT16                *AtAmPref
)

{
  AT_STATE_STRUCT     ATInfo;
  EFI_STATUS          Status;

  Status = HeciGetAtFwStateInfoMsg (&ATInfo);

  *AtState = ATInfo.AtState;
  *AtLastTheftTrigger = ATInfo.AtLastTheftTrigger;
  *AtLockState = ATInfo.AtLockState;
  *AtAmPref = ATInfo.AtAmPref;

  return Status;
}

UINTN IncStolenBootCount (
)
{
  UINT32      PbaFailedCount;
  UINTN       VarSize;
  UINT32      Attributes;
  EFI_STATUS  Status;

  VarSize = sizeof (PbaFailedCount);
  Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
  Status = gRT->GetVariable (
             L"PBA_FAILED_COUNT",
             &gEfiVlv2VariableGuid,
             &Attributes,
             &VarSize,
             &PbaFailedCount
           );

  if(EFI_ERROR(Status) && Status != EFI_NOT_FOUND) {
    ASSERT(FALSE);
    return 0;
  } else if (Status == EFI_NOT_FOUND) {
    PbaFailedCount = 0;
  }

  PbaFailedCount ++;

  Status = gRT->SetVariable (
                  L"PBA_FAILED_COUNT",
                  &gEfiVlv2VariableGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (PbaFailedCount),
                  &PbaFailedCount
  );
  ASSERT_EFI_ERROR(Status);
  return PbaFailedCount;
}

EFI_STATUS
ClearStolenBootCount (
)
{
  UINT32      PbaFailedCount;
  EFI_STATUS  Status;

  PbaFailedCount = 0;

  Status = gRT->SetVariable (
                  L"PBA_FAILED_COUNT",
                  &gEfiVlv2VariableGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  sizeof (PbaFailedCount),
                  &PbaFailedCount
    );
  ASSERT_EFI_ERROR(Status);
  return Status;
}

EFI_STATUS
GetSeCCapability (
  UINT32      *SeCCapability
)
{
  EFI_STATUS Status;
  GEN_GET_FW_CAPSKU       MsgGenGetFwCapsSku;
  GEN_GET_FW_CAPS_SKU_ACK MsgGenGetFwCapsSkuAck;
  Status = HeciGetFwCapsSkuMsg (&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  *SeCCapability = MsgGenGetFwCapsSkuAck.Data.FWCapSku.Data;
  return EFI_SUCCESS;
}

EFI_STATUS 
TdtAmEntryPoint(
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
)

/*++

Routine Description:

  Entry point for the TDTAm Driver.

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

  EFI_STATUS                Status;
  UINT32                    SeCStatus = 0;
  EFI_HECI_PROTOCOL         *Heci;
  UINT32                    idx;
  EFI_TDT_PROTOCOL          *pTdt = NULL;
  UINT8                     TdtState = 0;
  UINT8                     TdtLastTheftTrigger = 0;
  UINT16                    TdtLockState = 0;
  UINT16                    TdtAmPref = 0;
  TDT_BIOS_RECOVERY_CONFIG  gRecoveryConfig;
  UINT32                    RecoveryStringLength;
//  DXE_SEC_POLICY_PROTOCOL    *mDxePlatformSeCPolicy;
  SECFWCAPS_SKU              FwCapsSku;

  SetMem ((VOID *)&gRecoveryConfig, sizeof (TDT_BIOS_RECOVERY_CONFIG), 0);
  SetMem ((VOID *)&FwCapsSku, sizeof (SECFWCAPS_SKU), 0);

  Status = gBS->LocateProtocol (
                  &gDxePlatformTdtPolicyGuid,
                  NULL,
                  (VOID **) &pTdtPlatformPolicy
                  );

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "TDTAM::No TDT Platform Policy Protocol available"));
    return Status;
  }
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm failed to locate HECI driver, Status = %r\n", Status));
    return Status;
  }

  Status = Heci->GetSeCStatus(&SeCStatus);
  ASSERT_EFI_ERROR(Status);

  if (!CheckTdtStatus(SeCStatus)) {
    PrepareDisplayWarnToShutDownSystem();
    return EFI_SUCCESS;
  }

  //
  // Check Tdt status
  //
  if(pTdtPlatformPolicy->Tdt.TdtConfig == 1 || pTdtPlatformPolicy->Tdt.TdtPBAEnable == 1) {
    //
    // Added delay for FW to complete initialization for ME to initialized
    //
    idx = 0;
    while (SEC_STATUS_SEC_STATE_ONLY(SeCStatus) != SEC_READY  &&
           SEC_STATUS_SEC_STATE_ONLY(SeCStatus) != SEC_IN_RECOVERY_MODE) {
      gBS->Stall(1000000);
      idx++;
      if (idx > 10) {
        ASSERT_EFI_ERROR(SeCStatus);
        break;
      }
      Heci->GetSeCStatus(&SeCStatus);
    }
    //
    // Check if ME is Normal State or Recovery Mode
    //
    if(SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY ||
        SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_IN_RECOVERY_MODE) {

      //
      // Get the ME platform policy.
      //
      GetSeCCapability(&FwCapsSku.Data);
      DEBUG((EFI_D_ERROR, "FWCapSku %x\n", FwCapsSku.Data));
      if(!FwCapsSku.Fields.IntelAT) {
        DEBUG ((EFI_D_ERROR, "TDTAM: Exit Early - MEFWCAPS_SKU_RULE_ID indicates AT does not exist\n"));
        return EFI_SUCCESS;
      } else
        DEBUG ((EFI_D_ERROR, "TDTAM: ME FW SKU Info Variables indicates that AT exists\n"));
      DEBUG ((EFI_D_ERROR, "TDTAM::GetSeCStatus is SEC_READY\n", Status));
      Status = gBS->LocateProtocol (
                      &gEfiTdtProtocolGuid,
                      NULL,
                      (VOID **) &pTdt
                      );
      if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "TDTAm::TDT Protocol failed, Status = %r\n", Status));
        return Status;
      }

      do {
        //
        // If TDT is not reday, always retry
        //
        Status = GetAtStateInfoII(&TdtState, &TdtLastTheftTrigger, &TdtLockState, &TdtAmPref);
      } while (EFI_ERROR(Status));

      DEBUG((EFI_D_ERROR, "ATState %x %x %x %x\n", TdtState, TdtLastTheftTrigger, TdtLockState, TdtAmPref));
      if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "TDTAm::GetTdtSeCRule failed, Status = %r\n", Status));
        return Status;
      }
      //
      // Update Static Variable
      //
      gAtStateGlobalVar.AtState = TdtState;
      gAtStateGlobalVar.AtLastTheftTrigger = TdtLastTheftTrigger;
      gAtStateGlobalVar.AtLockState = TdtLockState;
      gAtStateGlobalVar.AtAmPref = TdtAmPref;

      gRecoveryAMGlobalVar = TdtAmPref;

      DEBUG((EFI_D_ERROR, "SeCTdtRuleDate TdtState = %x\n", TdtState ));
      DEBUG((EFI_D_ERROR, "SeCTdtRuleDate TdtAmPref = %x\n", TdtAmPref ));

      //
      // Read TDT BIOS Recovery Configuration which is needed in ValidatePreferredAM() ..
      //
      if ((TdtState == TDT_STATE_ACTIVE) ||
          (TdtState == TDT_STATE_STOLEN) ||
          (TdtState == TDT_STATE_SUSPEND)) {
        GetRecoveryConfig(&gRecoveryConfig, &RecoveryStringLength);
        CopyMem(&gBiosRecoveryGlobalVar, &gRecoveryConfig, RecoveryStringLength);
      }
      //
      // Check for PBA_ERROR_THRESHOLDS Level .... i.e. if PBA fails for x number of times then BIOS AM will
      // Ignore the PREFERRED_AM Selection
      //
//      Status = ValidatePreferredAM(&TdtState, &TdtAmPref);
//      DEBUG((EFI_D_ERROR, "TDTAm::ValidatePreferredAM, TdtAmPref = %d\n", TdtAmPref));
      //
      // Ignore the PREFERRED_AM Selection
      //
//      if(EFI_ERROR(Status)) {
//        DEBUG((EFI_D_ERROR, "TDTAm::ValidatePreferredAM failed, Status = %r\n", Status));
//      }

      DEBUG((EFI_D_ERROR, "TDTAM: TdtState %x TdtAmPref %x TdtLockState %x\n", TdtState, TdtAmPref, TdtLockState));
      //
      // CPT 7.0 features
      //
      if (TdtState == TDT_STATE_ACTIVE) {
        if (pTdtPlatformPolicy->Tdt.TdtEnterSuspendState != 0) {
          mPerformOperation = TDT_PERFORM_SUSPEND;
        }
        ClearStolenBootCount();
      } else if (TdtState == TDT_STATE_SUSPEND) {
        DEBUG((EFI_D_ERROR, "TDTAM: Set mPerformOperation to 1\n"));
        mPerformOperation = TDT_PERFORM_SUSPEND;
        ClearStolenBootCount();
      } else if (TdtState == TDT_STATE_STOLEN && TdtLockState == 1) {
        //
        // TdtLockState=0 is for Do Nothing Policy and can only be set by ISV for DTimer expire
        // Policy. So we let go BIOS AM and system boot when LockState = 0
        //
        mStolenBootCount = IncStolenBootCount();
        if(pTdtPlatformPolicy->Tdt.TdtConfig == 1 && pTdtPlatformPolicy->Tdt.TdtPBAEnable == 1) {
          if (TdtAmPref == TDT_AM_SELECTION_TDTAM) {
            mPerformOperation = TDT_PERFORM_ATAM_RECOVERY;
          } else if (TdtAmPref == TDT_AM_SELECTION_PBAM) {
            if (mStolenBootCount > gBiosRecoveryGlobalVar.PbaOverRideThreshold) {
              mPerformOperation = TDT_PERFORM_ATAM_RECOVERY;
            } else {
              mNotifyDelayTime = mStolenBootCount * TDT_BASE_DELAY_TIME;
              mPerformOperation = TDT_PERFORM_NOTIFY;
            }
          }
        } else if (pTdtPlatformPolicy->Tdt.TdtConfig == 0 && pTdtPlatformPolicy->Tdt.TdtPBAEnable == 1) {
          if (TdtAmPref == TDT_AM_SELECTION_TDTAM) {
            mNotifyDelayTime = mStolenBootCount * TDT_BASE_DELAY_TIME;
            mPerformOperation = TDT_PERFORM_NOTIFY;
          } else if (TdtAmPref == TDT_AM_SELECTION_PBAM) {
            if (mStolenBootCount > gBiosRecoveryGlobalVar.PbaOverRideThreshold) {
              mNotifyDelayTime = (mStolenBootCount - gBiosRecoveryGlobalVar.PbaOverRideThreshold) * TDT_BASE_DELAY_TIME;
              mPerformOperation = TDT_PERFORM_NOTIFY;
            }
          }
        } else if (pTdtPlatformPolicy->Tdt.TdtConfig == 1 && pTdtPlatformPolicy->Tdt.TdtPBAEnable == 0) {
          mPerformOperation = TDT_PERFORM_ATAM_RECOVERY;
        }
      } else {
        ClearStolenBootCount();
        DEBUG ((EFI_D_ERROR, "GetTdtState::System Not Stolen - Good\n"));
      }

    } else {
      DEBUG ((EFI_D_ERROR, "TDT::TDTAm SEC_READY Failed\n"));
      Status = EFI_DEVICE_ERROR;
    }

  } else {
    DEBUG ((EFI_D_ERROR, "TDT::TDT Disabled in the BIOS\n"));
  }

  DEBUG((EFI_D_ERROR, "TDTAM: Before Exit %r\n", Status));
  if (!EFI_ERROR(Status)) {
    SetTdtInfo();
    InstallTdtOperation(ImageHandle);
  }
  return Status;
}

VOID DisplayATNotifyScreen(
  UINTN            DelayTime
)
{
  CHAR16                            TimeLeftStr[32];
  CHAR16                            AuthTimeLeftStr[32];
  CHAR16                            LastTriggerStr[32];
  UINT32                            CRow;
  UINT32                            CCol;
  EFI_STATUS                        Status;
  EFI_TDT_PROTOCOL          *pTdt = NULL;

  UINT32 AuthTimeLeft;
  UINT32 Interval;

  UINT8 tmp1[32] = "Disable Timer Expired";
  UINT8 tmp2[32] = "Stolen Message Received";
  UINT8 tmp3[32] = "Logon Threshold Exceeded";
  UINT8 tmp4[32] = "Platform Attack Detected";
  UINT8 tmp5[32] = "Unknown";


  if (DelayTime == 0) return;
  Status = gBS->LocateProtocol (
                  &gEfiTdtProtocolGuid,
                  NULL,
                  (VOID **) &pTdt
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAM: DisplayATNotifyScreen Can't find the TDT Protocol\n"));
    AuthTimeLeft = (UINT32) DelayTime + MIN_SYSTEM_RUN_TIME;
  } else {
    DEBUG((EFI_D_ERROR, "TDTAM: DisplayATNotifyScreen find the TDT Protocol\n"));
    Status = pTdt->GetTimerInfo(pTdt, &Interval, &AuthTimeLeft);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "TDTAM: DisplayATNotifyScreen TDT Protocol Can't Get get the TimeLeft\n"));
      AuthTimeLeft = (UINT32) DelayTime + MIN_SYSTEM_RUN_TIME;
    } else {
      DEBUG((EFI_D_ERROR, "TDTAM: DisplayATNotifyScreen TDT Protocol Get get the TimeLeft %x\n", AuthTimeLeft));
    }
  }

  if (DelayTime > AuthTimeLeft - MIN_SYSTEM_RUN_TIME) {
    DelayTime = AuthTimeLeft - MIN_SYSTEM_RUN_TIME;
  }

  switch(gAtStateGlobalVar.AtLastTheftTrigger) {
    case 1:
      Uint8ToUnicode(tmp1, LastTriggerStr);
      break;
    case 2:
      Uint8ToUnicode(tmp2, LastTriggerStr);
      break;
    case 3:
      Uint8ToUnicode(tmp3, LastTriggerStr);
      break;
    case 4:
      Uint8ToUnicode(tmp4, LastTriggerStr);
      break;
    default:
      Uint8ToUnicode(tmp5, LastTriggerStr);
      break;
  }

  UnicodeValueToString(TimeLeftStr,  0, DelayTime, 0);
  UnicodeValueToString(AuthTimeLeftStr,  0, AuthTimeLeft, 0);

  gST->ConOut->ClearScreen( gST->ConOut);
  gST->ConOut->SetCursorPosition( gST->ConOut, 10, gST->ConOut->Mode->CursorRow + 1);
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Intel(R) AT system lock due to: ");
  gST->ConOut->OutputString(gST->ConOut, LastTriggerStr);
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Time left to authenticate via software: ");
  gST->ConOut->OutputString(gST->ConOut, AuthTimeLeftStr);
  gST->ConOut->OutputString(gST->ConOut, L" Seconds.\r\n    Time left to boot: ");
  CRow = gST->ConOut->Mode->CursorRow;
  CCol = gST->ConOut->Mode->CursorColumn;
  gST->ConOut->OutputString(gST->ConOut, TimeLeftStr);
  gST->ConOut->OutputString(gST->ConOut, L" Second.  ");

  DisplayIsvStrings();

  do {
    gBS->Stall(1000000);//Wait a second
    DelayTime --;
    UnicodeValueToString(TimeLeftStr,  0, DelayTime, 0);
    gST->ConOut->SetCursorPosition( gST->ConOut, CCol, CRow);
    gST->ConOut->OutputString(gST->ConOut, TimeLeftStr);
    gST->ConOut->OutputString(gST->ConOut, L" Second.  ");
  } while (DelayTime > 0);
  gST->ConOut->ClearScreen( gST->ConOut);

}

VOID DisplayATNotify()
{
  DisplayATNotifyScreen(10);
}

EFI_STATUS Init3g(
  EFI_TDT_PROTOCOL          *pTdt
)
/*++

Routine Description:

  Entry point for the TDTAm Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS       Initialization complete.
  EFI_NOT_READY     Timeout waiting for FW to return healthy status for NIC Radio

--*/
{

  EFI_STATUS                Status;
  UINT8                     RadioStatus = 0;
  UINT8                     NetworkStatus = 0;
  UINT8                     temp = 0;

  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Initializing WWAN Interface..... ");
  Status = pTdt->InitWWAN(pTdt);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::Init3g Failed to Initialize WWAN interface, Status = %r\n", Status));
    gST->ConOut->OutputString(gST->ConOut, L"FAIL");
    return Status;
  }

  gST->ConOut->OutputString(gST->ConOut, L"COMPLETE");
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Waiting for Radio to be on and network to be attached.....");
  while((RadioStatus != 1) && (NetworkStatus != 1)) { //RadioStatus = 0 means the radio is off, NetworkStatus= 0 means that network is not attached
    Status = pTdt->GetWWANNicStatus(pTdt, &RadioStatus, &NetworkStatus);
    gBS->Stall(1000000);//Wait a second
    temp++;
    if (temp > 10) {
      DEBUG((EFI_D_ERROR, "TDTAm::Init3g Fail - RadioStatus=%d, NetworkStatus=%d, Status = %r\n", RadioStatus, NetworkStatus, Status));
      gST->ConOut->OutputString(gST->ConOut, L"\r\n    WWAN Interface Down\n");
      return EFI_NOT_READY;
    }
  }

  gST->ConOut->OutputString(gST->ConOut, L"\r\n    WWAN Sucessfully Initialized\n");
  gBS->Stall(1000000);//Wait a second
  return EFI_SUCCESS;

}


VOID
SetPwdTimeOut (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
)
/*++

Routine Description:
  This creates a Password Timeout for user input

Arguments:
  EFI_EVENT          - EFI_EVENT
  Pointer to a Boolean value for timeout

Returns:
  Boolean value for Timeout
--*/

{
  BOOLEAN    *Timeout;
  Timeout = (BOOLEAN *)Context;

  if (Timeout != NULL)
    *Timeout = TRUE;
}


EFI_STATUS 
TimerCreateTimer(
  EFI_EVENT        *Event,
  EFI_EVENT_NOTIFY Callback,
  VOID             *Context,
  EFI_TIMER_DELAY  Delay,
  UINT64           Trigger,
  EFI_TPL          CallBackTPL
  )
/*++

Routine Description:
  This creates a Timer for checking user input

Arguments:
  EFI_EVENT          - EFI_EVENT
  EFI_EVENT_NOTIFY - A call back funtion that needs to be called
  This Context
  Time delay
  EFI_TPL - A callback TPL

Returns:
  EFI_SUCCESS           The function completed successfully.
--*/


{
  EFI_STATUS Status;
  UINT32 EventType = EVT_TIMER;

  if ( Callback != NULL )
    EventType |= EVT_NOTIFY_SIGNAL;

  Status = gBS->CreateEvent(
             EventType,
             CallBackTPL,
             Callback,
             Context,
             Event
           );

  if ( EFI_ERROR( Status ) )
    return Status;

  Status = gBS->SetTimer( *Event, Delay, Trigger );
  if ( EFI_ERROR( Status ) )
    TimerStopTimer( Event );

  return Status;
}



EFI_STATUS TimerStopTimer(
  EFI_EVENT *Event
)

/*++

Routine Description:
  This stops the Timer created by TimerCreateTimer()

Arguments:
  EFI_EVENT          - EFI_EVENT

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/



{
  EFI_STATUS Status = EFI_SUCCESS;

  if ( ( Event == NULL ) || ( *Event == NULL ) )
    return Status;

  gBS->CloseEvent( *Event );
  *Event = NULL;

  return Status;
}

VOID ProcessSuspendMode(
  EFI_EVENT   Event,
  VOID        *ParentImageHandle,
  BOOLEAN     *SetTdtEnterSuspendState,
  UINT8       *TdtEnterSuspendState
)
/*++

Routine Description:
  This function determines whether we should enter or exit Suspend Mode

Arguments:
  EFI_EVENT          - EFI_EVENT that invoked this function
  ParentImageHandle - the parent proccess handle that invoked this function

Returns:
  None

--*/
{

  EFI_TDT_PROTOCOL          *pTdt = NULL;
  EFI_STATUS                Status = EFI_SUCCESS;
  EFI_INPUT_KEY             Key = {0, 0};
  UINT8                     SrtkPass[TDTAM_SETUP_PASSWORD_LENGTH];
  UINT8                     SuspendToken[TDTAM_SETUP_TOKEN_LENGTH];
  UINT8                     SuspendModeAttempts = 3;

//  gBS->CloseEvent(Event);

  //
  // Assume don't need reset the SetupVariable of Tdt Suspend in platform code
  //
  *SetTdtEnterSuspendState = FALSE;

  //
  //Locate TDT protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiTdtProtocolGuid,
                  NULL,
                  (VOID **) &pTdt
                  );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::ProcessSuspendMode Could not locate AT Protocol, Status = %r\n", Status));
    return;
  }

  //Get TDT state
  if((gAtStateGlobalVar.AtState == TDT_STATE_SUSPEND) &&
      ((pTdtPlatformPolicy->Tdt.TdtConfig != 0) || (pTdtPlatformPolicy->Tdt.TdtPBAEnable != 1))) {
    while(SuspendModeAttempts > 0) {
      gST->ConOut->ClearScreen( gST->ConOut);
      DisplayIsvStrings();
      gST->ConOut->OutputString(gST->ConOut, L"\r\n Intel(R) AT in Suspended State: Exit? (y/n)?");
      do {
        Status = gBS->CheckEvent (gST->ConIn->WaitForKey);
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        if (!EFI_ERROR (Status)) {
          if (Key.UnicodeChar == L'y' || Key.UnicodeChar == L'Y') {
            gST->ConOut->OutputString(gST->ConOut, L"\r\n    Exit Suspend Mode.....");
            SetMem ((VOID *)SrtkPass, TDTAM_SETUP_PASSWORD_LENGTH, 0);
            SrtkPass[0] = L'\0';
            SetMem ((VOID *)SuspendToken, TDTAM_SETUP_TOKEN_LENGTH, 0);
            Status = GetSuspendAuthentication(pTdt, SuspendToken, SrtkPass);
            if(EFI_ERROR(Status)) {
              DEBUG((EFI_D_ERROR, "TDTAm::GetSuspendAuthentication failed, Status = %r\n", Status));
              gST->ConOut->OutputString(gST->ConOut, L"Failed to Authenticate");
              gBS->Stall (1000000);
              break;
            }
            Status = pTdt->SetSuspendState(pTdt, TDTHI_SUSPEND_EXIT, SuspendToken);
            if(EFI_ERROR(Status)) {
              DEBUG((EFI_D_ERROR, "TDTAm::SetSuspendState failed, Status = %r\n", Status));
              gST->ConOut->OutputString(gST->ConOut, L"\r\n    SetSuspendState Failed to Exit");
              gBS->Stall (1000000);
              break;
            }
            //Successfully Exited Suspend Mode so update BIOS setup
            gST->ConOut->OutputString(gST->ConOut, L"\r\n    Successfully Exited Suspend Mode");
            gBS->Stall (1000000);
            pTdtPlatformPolicy->Tdt.TdtEnterSuspendState = 0;
            goto Exit;
          }//if (Key.UnicodeChar == L'y' || Key.UnicodeChar == L'Y') {
          else if (Key.UnicodeChar == L'n' || Key.UnicodeChar == L'N') {
            gST->ConOut->OutputString(gST->ConOut, L"\r\n    Stay in Suspend Mode....");
            gBS->Stall (1000000); //Wait one second so user can see screen
            goto Exit;
          }
        }
      } while (1); //Do/while loop until user selects yes or no
      --SuspendModeAttempts;
      if(SuspendModeAttempts == 0) {
        gST->ConOut->OutputString(gST->ConOut, L"\r\n\n\n Exceeded Max Attempts - Exiting ....");
        gBS->Stall (1000000); //Wait one second so user can see screen
      }
    }//while(SuspendModeAttempts > 0)

    //if(gAtStateGlobalVar.AtState == TDT_STATE_SUSPEND){
  } else if((gAtStateGlobalVar.AtState == TDT_STATE_ACTIVE) &&
            (pTdtPlatformPolicy->Tdt.TdtConfig != 0)) {
    if(pTdtPlatformPolicy->Tdt.TdtEnterSuspendState) {
      while(SuspendModeAttempts > 0) {
        gST->ConOut->ClearScreen( gST->ConOut);
        DisplayIsvStrings();
        gST->ConOut->OutputString(gST->ConOut, L"\r\n    User Has Requested To Enter Intel(R) AT Suspend Mode.....");
        SetMem ((VOID *)SrtkPass, TDTAM_SETUP_PASSWORD_LENGTH, 0);
        SrtkPass[0] = L'\0';
        SetMem ((VOID *)SuspendToken, TDTAM_SETUP_TOKEN_LENGTH, 0);
        Status = GetSuspendAuthentication(pTdt, SuspendToken, SrtkPass);
        if(EFI_ERROR(Status)) {
          DEBUG((EFI_D_ERROR, "TDTAm::GetSuspendAuthentication failed, Status = %r\n", Status));
          gST->ConOut->OutputString(gST->ConOut, L"Failed to Authenticate ");
          gBS->Stall (1000000); //Wait one second so user can see screen
        } else {
          Status = pTdt->SetSuspendState(pTdt, TDTHI_SUSPEND_ENTER, SuspendToken);
          if(EFI_ERROR(Status)) {
            DEBUG((EFI_D_ERROR, "TDTAm::SetSuspendState failed, Status = %r\n", Status));
            gST->ConOut->OutputString(gST->ConOut, L"\r\n    Failed to Put Platform into Suspended Mode");
            gBS->Stall (1000000); //Wait one second so user can see screen
          } else {
            gST->ConOut->OutputString(gST->ConOut, L"\r\n    Successfully Put Platform in Suspended Mode - Exiting TDTAM....");
            gBS->Stall (1000000); //Wait one second so user can see screen
            goto Exit;
          }
        }
        --SuspendModeAttempts;
      }//while(SuspendModeAttempts > 0)
      if(SuspendModeAttempts == 0) {
        gST->ConOut->OutputString(gST->ConOut, L"\r\n\n\n Exceeded Max Attempts - Exiting ....");
        gBS->Stall (1000000); //Wait one second so user can see screen
        pTdtPlatformPolicy->Tdt.TdtEnterSuspendState = 0;
      }
    } else//if(pTdtPlatformPolicy->Tdt.TdtEnterSuspendState){
      return; //Don't do anything else when AT is ACTIVE unless user requested to enter Suspend Mode
  } else {
    return; //Don't do anything
  }

Exit:
  gST->ConOut->ClearScreen( gST->ConOut);

  //
  // Need reset the SetupVariable of Tdt Suspend in platform code
  //
  *SetTdtEnterSuspendState = TRUE;
  *TdtEnterSuspendState    = pTdtPlatformPolicy->Tdt.TdtEnterSuspendState;

  return;
}

UINT8 CheckRecoveryPassword(
  EFI_EVENT   Event,
  VOID        *ParentImageHandle
)

/*++

Routine Description:
  This function check for the AT password entered by the user.

Arguments:
  EFI_EVENT          - EFI_EVENT that invoked this function
  ParentImageHandle - the parent proccess handle that invoked this function

Returns:
  None

--*/

{

  EFI_TDT_PROTOCOL *pTdt = NULL;
  EFI_STATUS               Status;
  UINTN PasswordLength = TDTAM_SETUP_PASSWORD_LENGTH;
  UINT8 *PasswordEntered;
  UINT8 *Hash;
  UINT8 Nonce[NONCE_LENGTH];
  UINTN NonceLength = NONCE_LENGTH;
  UINT8 NonceStr[STR_NONCE_LENGTH];
  UINTN StrNonceLength = STR_NONCE_LENGTH;
  UINTN NoOfRetries = 0;
  UINT8 IsAuthenticated = 0;
  UINT32 TimeLeft;
  UINT32 Interval;
  UINT8 *Uint8Pass;
  UINT8 *SrtkPass;
  CHAR16 *PasswordIn;
  UINT32 PassType = 0;
  UINT8  TdtState = 0;
  UINT8  TdtLastTheftTrigger = 0;
  UINT16 TdtLockState = 0;
  UINT16 TdtAmPref = 0;
  UINT8  temp = 0;
  UINTN usrRsp;

//  gBS->CloseEvent(Event);
  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   1\n"));
  NoOfRetries = pTdtPlatformPolicy->Tdt.TdtRecoveryAttempt;
  DEBUG((EFI_D_ERROR, "TDTAm::TdtRecoveryAttempt read from setup is : %d\n", NoOfRetries));
  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   2\n"));

  if (!NoOfRetries) {
    DEBUG((EFI_D_ERROR, "TDTAm::No of Password recovery attempt is set to invalid.\n"));
    IsAuthenticated = 0;
    return IsAuthenticated;

  }
  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   3\n"));
  // Allocating memory for password
  PasswordEntered = NULL;
  PasswordIn = NULL;
  Uint8Pass = NULL;
  SrtkPass = NULL;
  Hash = NULL;
  PasswordEntered = AllocatePool((TDTAM_SETUP_PASSWORD_LENGTH + 1) * sizeof(UINT8));
  if (PasswordEntered == NULL) {
    DEBUG((EFI_D_ERROR, "TDTAm::Out of resource.\n"));
    IsAuthenticated = 0;
    goto tdt_cleanup;
  }
  PasswordIn = AllocatePool((TDTAM_SETUP_PASSWORD_LENGTH + 1) * sizeof(CHAR16));
  if (PasswordIn == NULL) {
    DEBUG((EFI_D_ERROR, "TDTAm::Out of resource.\n"));
    IsAuthenticated = 0;
    goto tdt_cleanup;
  }
  Uint8Pass = AllocatePool((TDTAM_SETUP_PASSWORD_LENGTH + 1) * sizeof(UINT8));
  if (Uint8Pass == NULL) {
    DEBUG((EFI_D_ERROR, "TDTAm::Out of resource.\n"));
    IsAuthenticated = 0;
    goto tdt_cleanup;
  }
  SrtkPass = AllocatePool((TDTAM_SETUP_PASSWORD_LENGTH + 1) * sizeof(UINT8));
  if (SrtkPass == NULL) {
    DEBUG((EFI_D_ERROR, "TDTAm::Out of resource.\n"));
    IsAuthenticated = 0;
    goto tdt_cleanup;
  }

  Hash = AllocatePool((TDTAM_SETUP_PASSWORD_LENGTH + 1) * sizeof(UINT8));
  if (Hash == NULL) {
    DEBUG((EFI_D_ERROR, "TDTAm::Out of resource.\n"));
    IsAuthenticated = 0;
    goto tdt_cleanup;
  }

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   4\n"));


  Status = gBS->LocateProtocol (
                  &gEfiTdtProtocolGuid,
                  NULL,
                  (VOID **) &pTdt
                  );

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   5\n"));

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::CheckRecoveryPassword Tdt Protocol failed, Status = %r\n", Status));
    IsAuthenticated = 0;
    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   6\n"));
    goto tdt_cleanup;
  }

  Status = pTdt->GetNonce(pTdt, Nonce);

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   7\n"));
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::GetNonce failed, Status = %r\n", Status));
    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   8\n"));
    //return Status;
  }

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   9\n"));
  Base32Encode(NonceStr, &StrNonceLength , Nonce, NonceLength);

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   10\n"));

  //This call is to SEC Kernel and this expect SEC to intialized i.e. NORMAL STATE
  Status = pTdt->GetTdtSeCRule(pTdt, &TdtState, &TdtLastTheftTrigger, &TdtLockState, &TdtAmPref );
  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   11\n"));

  Status = pTdt->GetTimerInfo(pTdt, &Interval, &TimeLeft);
  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   12 %x\n", TimeLeft));

  gST->ConIn->Reset( gST->ConIn, FALSE );

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   13\n"));
  //Give retries based on NoOfRetries
  while(NoOfRetries) {

    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   14\n"));
    usrRsp = PasswordType(TimeLeft, TdtLastTheftTrigger);

    if (usrRsp == 0) {
      IsAuthenticated = 0;
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   15\n"));
      break;

    }

    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   16\n"));
    if (usrRsp == 1 || usrRsp == 2) {
      NoOfRetries--;
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   17\n"));
      SetMem( PasswordIn, (PasswordLength + 1)*sizeof(CHAR16), 0 );
      PasswordIn[0] = L'\0';

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   18\n"));
      SetMem( PasswordEntered, (PasswordLength + 1)*sizeof(UINT8), 0 );
      PasswordEntered[0] = L'\0';

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   19\n"));
      SetMem( Hash, (PasswordLength + 1)*sizeof(UINT8), 0 );
      Hash[0] = L'\0';

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   20\n"));
      SetMem( SrtkPass, (TDTAM_SETUP_PASSWORD_LENGTH + 1)*sizeof(UINT8), 0 );
      SrtkPass[0] = L'\0';

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   21\n"));
      Status = pTdt->GetTimerInfo(pTdt, &Interval, &TimeLeft);
      // get the passwrd from user

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   22\n"));
      Status = GetRecoveryPassword(PasswordEntered, PasswordLength, NULL, TimeLeft, NonceStr, PasswordIn, TdtLastTheftTrigger, usrRsp);

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   23 %a \n", PasswordEntered));
      if(Status) {
        DEBUG((EFI_D_ERROR, "TDTAm::ComputeHash failed, Status = %r\n", Status));
        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   24\n"));
        break;
      }
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   25\n"));
      PasswordLength = AsciiStrLen((CHAR8 *)PasswordEntered);

      // User selected user password option for recovery and if the length is 0 then ignore it ...
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   26 %d\n", PasswordLength));
      if( usrRsp == 1 && PasswordLength != 0) {

        //ComputeHash using ME TDT Service call and then send Hash to ME to validate the user password
        //SHA1 should be BIOS, since SHA1 is not part of of MPG BIOS, so we are relying on ME to compute HASH
        //for user password.
        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   27\n"));
        Status = pTdt->ComputeHash(pTdt, PasswordEntered, Hash);

        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   28\n"));
        if(EFI_ERROR(Status)) {
          DEBUG((EFI_D_ERROR, "TDTAm::ComputeHash failed, Status = %r\n", Status));
          break;
        }
        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   29\n"));

        PassType = TDT_CREDENTIAL_TYPE_USER_PASSPHRASE;
        // Send the SHA1 based Hash to verify the recovery authentication
        Status = pTdt->AuthenticateCredential(pTdt, Hash, &PassType, &IsAuthenticated);
//        NoOfRetries--;
        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   30\n"));

      }
      // User selected Basee32 based Server based recovery, for BASE32 the length is always 32
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   31\n"));
      if((usrRsp == 2) && PasswordLength == 32) {

        PassType = TDT_CREDENTIAL_TYPE_SRTK;
        DEBUG ((EFI_D_ERROR, "Password entered is SRTK BASE32 PASSWORD\n"));
        // Decoded value is 20 byte hex
        Base32Decode(PasswordEntered, SrtkPass);
        Status = pTdt->AuthenticateCredential(pTdt, SrtkPass, &PassType, &IsAuthenticated);
//        NoOfRetries--;

        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   32\n"));

      }
      // User selected Basee10 based Server based recovery, for BASE10 the length is always more than 40
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   33\n"));
      if((usrRsp == 2) && PasswordLength > 40) {
        PassType = TDT_CREDENTIAL_TYPE_SRTK;
        DEBUG ((EFI_D_ERROR, "Password entered is SRTK BASE10 PASSWORD\n"));
        // Decoded value is 20 byte hex
        DecimalToHexString(PasswordEntered, SrtkPass, MAX_HEX_BYTES - 1);
        Status = pTdt->AuthenticateCredential(pTdt, SrtkPass, &PassType, &IsAuthenticated);

//        NoOfRetries--;

        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   34\n"));
      }

      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   35\n"));
    }//if (usrRsp == 1 || usrRsp == 2) {
    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   1\n"));
    if(usrRsp == 3) {
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   36\n"));
      gST->ConOut->ClearScreen( gST->ConOut);
      gST->ConOut->SetCursorPosition( gST->ConOut, 2, 2);
      Status =  Init3g(pTdt);
      gBS->Stall(1000000);//Wait a second
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   37\n"));
      if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "TDTAm::CheckRecoveryPassword Tdt Protocol failed, Status = %r\n", Status));
        gST->ConOut->OutputString(gST->ConOut, L"\r\n    Please check SIM card or Radio Power Switch \n");
        gBS->Stall(1000000);//Wait a second
        NoOfRetries--;
        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   38\n"));
      } //if(EFI_ERROR(Status)) {
      else {
        gST->ConOut->OutputString(gST->ConOut, L"Waiting for SMS recovery....");
        temp = 0;
        DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   39\n"));
        while(1) {
          Status = pTdt->GetTdtSeCRule(pTdt, &TdtState, &TdtLastTheftTrigger, &TdtLockState, &TdtAmPref);
          if(TdtState != TDT_STATE_ACTIVE) {
            if (temp > TDT_SMS_RECOVERY_MAX_TIMEOUT) {
              DEBUG((EFI_D_ERROR, "TDTAm::CheckRecoveryPassword Timeout Waiting for FW State to Change to Active\n"));
              gST->ConOut->OutputString(gST->ConOut, L"TIMEOUT\n\n");
              IsAuthenticated = 0;
              NoOfRetries--;
              break;
            }
            gBS->Stall(1000000);//Wait a second
            temp++;
            DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   40\n"));
          } else {
            IsAuthenticated = 1;
            gST->ConOut->OutputString(gST->ConOut, L"COMPLETE\n\n");
            break;
          }
        } //while(1){
      }//else
    }//if((usrRsp == 3){

    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   41\n"));
    gST->ConOut->SetCursorPosition( gST->ConOut, 10, gST->ConOut->Mode->CursorRow + 1);
    if(IsAuthenticated == 1) {
      gST->ConOut->OutputString(gST->ConOut, L"\r\n    Recovery Successful. Continue with the Boot ....");
      gBS->Stall (1000000);
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   42\n"));
      break;
    } else {
      gST->ConOut->OutputString(gST->ConOut, L"\r\n    Recovery Failed. Please try again ....");
      gBS->Stall (2000000);
      DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   43\n"));
    }
    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   44\n"));

  }//while(NoOfRetries)

tdt_cleanup:
  FreePool(PasswordEntered);
  FreePool(Hash);
  FreePool(PasswordIn);
  FreePool(Uint8Pass);
  FreePool(SrtkPass);
  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   45\n"));


  if(IsAuthenticated != 1) {
    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   46\n"));
    // if the the Password check fails then reboot the system
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    Intel(R) AT Recovery Failed.");
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    System will shutdown..");
    gBS->Stall (1000000);
    gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);

  }  else   {

    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   47\n"));
    ClearStolenBootCount();
    // Calling ValidatePreferredAM to reset the PbafailedCount ....
    //if (gRecoveryAMGlobalVar == TDT_AM_SELECTION_PBAM) {
    //  TdtState = TDT_STATE_ACTIVE;
    //  TdtAmPref = TDT_AM_SELECTION_PBAM;
    //  Status = ValidatePreferredAM(&TdtState, &TdtAmPref);
    // Do not care return status ... here ...
    //}
    DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   48\n"));
    // Password check passwd and continue with boot
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    Intel(R) AT Recovery Successful.");
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    System Boot Continue .....");
    gBS->Stall (1000000);

  }

  DEBUG((EFI_D_ERROR, "CheckRecoveryPassword   49\n"));
  gST->ConOut->ClearScreen( gST->ConOut);
  return (IsAuthenticated);
}

STATIC
UINTN
PasswordType (
  UINT32 TimeLeft,
  UINT8 TdtLastTheftTrigger
)
/*++

Routine Description:
  This routine detects the password type selected by user.

Arguments:
  TimeLeft - Time Left to enter password
  LastTrigger - Reason for AT stolen state
Returns:
  EFI_SUCCESS           The function completed successfully.

--*/

{

  EFI_STATUS                        Status;
  UINTN                             UsrRsp = 0;
  CHAR16                            *TimeLeftStr;
  CHAR16                            *LastTriggerStr;
  EFI_TDT_PROTOCOL                  *pTdt = NULL;
  UINT32                            CRow;
  UINT32                            CCol;
  EFI_INPUT_KEY                     Key = {0, 0};
  CHAR16                            *TheftTriggers;
  CHAR16                            TempChar[2] = { L' ', L'\0'};

  UINT8 tmp1[32] = "Disable Timer Expired";
  UINT8 tmp2[32] = "Stolen Message Received";
  UINT8 tmp3[32] = "Logon Threshold Exceeded";
  UINT8 tmp4[32] = "Platform Attack Detected";
  UINT8 tmp5[32] = "Unknown";



  Status = gBS->LocateProtocol (
                  &gEfiTdtProtocolGuid,
                  NULL,
                  (VOID **) &pTdt
                  );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::LocateProtocol Tdt failed, Status = %r\n", Status));
    return UsrRsp;
  }





  TimeLeftStr = AllocatePool((2 + 1) * sizeof(CHAR16));
  LastTriggerStr = AllocatePool((2 + 1) * sizeof(CHAR16));
  TheftTriggers = AllocatePool((32 + 1) * sizeof(CHAR16));

  SetMem( TimeLeftStr, (2 + 1)*sizeof(CHAR16), 0 );
  SetMem( LastTriggerStr, (2 + 1)*sizeof(CHAR16), 0 );
  SetMem( TheftTriggers, (32 + 1)*sizeof(CHAR16), 0 );


  Key.UnicodeChar = 0;
  Key.ScanCode = 0;

  UnicodeValueToString(TimeLeftStr,  0, TimeLeft, 0);

  if (TdtLastTheftTrigger == 1) {
    Uint8ToUnicode(tmp1, TheftTriggers);
  } else if (TdtLastTheftTrigger == 2) {
    Uint8ToUnicode(tmp2, TheftTriggers);
  } else if (TdtLastTheftTrigger == 3) {
    Uint8ToUnicode(tmp3, TheftTriggers);
  } else if (TdtLastTheftTrigger == 4) {
    Uint8ToUnicode(tmp4, TheftTriggers);
  } else {
    Uint8ToUnicode(tmp5, TheftTriggers);
  }
  gST->ConOut->ClearScreen( gST->ConOut);
  gST->ConOut->SetCursorPosition( gST->ConOut, 10, gST->ConOut->Mode->CursorRow + 1);
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Intel(R) AT system lock due to: ");
  gST->ConOut->OutputString(gST->ConOut, TheftTriggers);

  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Time Left to enter Password : ");
  gST->ConOut->OutputString(gST->ConOut, TimeLeftStr);
  gST->ConOut->OutputString(gST->ConOut, L" Second");

  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Please select one of the following for platform recovery:");
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    1 - User Password");
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    2 - Server Token Password");
//  gST->ConOut->OutputString(gST->ConOut, L"\r\n    3 - WWAN SMS Unlock");

  gST->ConOut->OutputString(gST->ConOut, L"\r\n\n    Select one of the above options to proceed ...\r\n");

  CRow = gST->ConOut->Mode->CursorRow;
  CCol = gST->ConOut->Mode->CursorColumn;


  DisplayIsvStrings();



  UsrRsp = 0;
  do {

    Status = gBS->CheckEvent (gST->ConIn->WaitForKey);
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);

    if (!EFI_ERROR (Status)) {

      if (Key.UnicodeChar == L'1') {
        gST->ConOut->OutputString(gST->ConOut, L"\r\n    User Password Selected");
        UsrRsp = 1;
        break;

      } else if (Key.UnicodeChar == L'2') {
        gST->ConOut->OutputString(gST->ConOut, L"\r\n    Server Token Selected");
        UsrRsp = 2;
        break;

      }
      /*      else if (Key.UnicodeChar == L'3') {
              gST->ConOut->OutputString(gST->ConOut, L"\r\n    WWAN Recovery Selected\n");
              UsrRsp = 3;// WWAN NIC Initialized ..
              break;

            } */
      else {
        TempChar[0] = Key.UnicodeChar;
        gST->ConOut->OutputString(gST->ConOut, L"\r\n    Invalid Selection, Press 1 or 2");
        gST->ConOut->SetCursorPosition( gST->ConOut, CCol, CRow);
        UsrRsp = 0;
      }
    }

  } while (1);

  return UsrRsp;

}


EFI_STATUS GetRecoveryPassword(
  UINT8 *PasswordEntered,
  UINTN PasswordLength,
  UINTN *pTimeOut,
  UINT32 TimeLeft,
  UINT8 *NonceStr,
  CHAR16 *PasswordIn,
  UINT8 TdtLastTheftTrigger,
  UINTN usrRsp

)
/*++

Routine Description:
  This GetRecoveryPassword() process the AT recovery password user input.

Arguments:
  PasswordEntered - Pointer to an array of ASCII user input
  PasswordLength - Integer value for password length
  pTimeOut - Integer value of password Timeout
  TimeLeft - UINT32 value of Timeleft to enter password
  NonceStr - Pointer to an array of ASCII nonce
  PasswordIn - Pointer to an array of UNICODE user input
  LastTrigger - Reason for AT stolen state in ASCII
  usrRsp - User response in Integer

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/

{

  EFI_STATUS          Status;
  INTN                StrIndex;
  UINT8               BackSpace = 0;
  UINTN               TimeOutValue;
  EFI_EVENT           timer;
  CHAR16              *TimeLeftStr;
  CHAR16              *UniCodeNonceStr;
  CHAR16              *LastTriggerStr;

  volatile BOOLEAN    bTimeOut = FALSE;
  EFI_INPUT_KEY       Key = {0, 0};
  CHAR16              StarChar[2] = { L' ', L'\0'};
  CHAR16              *TheftTriggers;

  UINT8 tmp1[32] = "Disable Timer Expired";
  UINT8 tmp2[32] = "Stolen Message Received";
  UINT8 tmp3[32] = "Logon Threshold Exceeded";
  UINT8 tmp4[32] = "Platform Attack Detected";
  UINT8 tmp5[32] = "Unknown";

  TheftTriggers = AllocatePool((32 + 1) * sizeof(CHAR16));
  UniCodeNonceStr = AllocatePool((STR_NONCE_LENGTH + 1) * sizeof(CHAR16));
  TimeLeftStr = AllocatePool((2 + 1) * sizeof(CHAR16));
  LastTriggerStr = AllocatePool((2 + 1) * sizeof(CHAR16));

  SetMem( UniCodeNonceStr, (STR_NONCE_LENGTH + 1)*sizeof(CHAR16), 0 );

  SetMem( TimeLeftStr, (2 + 1)*sizeof(CHAR16), 0 );

  SetMem( LastTriggerStr, (2 + 1)*sizeof(CHAR16), 0 );
  SetMem( TheftTriggers, (32 + 1)*sizeof(CHAR16), 0 );

  PasswordEntered[0] = L'\0';
  PasswordIn[0] = L'\0';

  Key.UnicodeChar = 0;
  Key.ScanCode = 0;
  StrIndex = 0;

  UnicodeValueToString(TimeLeftStr,  0, TimeLeft, 0);
  Uint8ToUnicode(NonceStr, UniCodeNonceStr);

  if (TdtLastTheftTrigger == 1) {
    Uint8ToUnicode(tmp1, TheftTriggers);
  } else if (TdtLastTheftTrigger == 2) {
    Uint8ToUnicode(tmp2, TheftTriggers);
  } else if (TdtLastTheftTrigger == 3) {
    Uint8ToUnicode(tmp3, TheftTriggers);
  } else if (TdtLastTheftTrigger == 4) {
    Uint8ToUnicode(tmp4, TheftTriggers);
  } else {
    Uint8ToUnicode(tmp5, TheftTriggers);
  }

  gST->ConOut->ClearScreen( gST->ConOut);
  gST->ConOut->SetCursorPosition( gST->ConOut, 2, 2);
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Intel(R) AT system lock due to: ");
  gST->ConOut->OutputString(gST->ConOut, TheftTriggers);

  if (usrRsp == 2 || usrRsp == 3 ) {
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    Refer the following Platform Recovery ID to IT: ");
    gST->ConOut->OutputString(gST->ConOut, UniCodeNonceStr);
  }
  gST->ConOut->OutputString(gST->ConOut, L"\r\n    Time Left to enter Password : ");
  gST->ConOut->OutputString(gST->ConOut, TimeLeftStr);
  gST->ConOut->OutputString(gST->ConOut, L" Second");

  DisplayIsvStrings();

  if (usrRsp == 1) {
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    Enter System Recovery Password : ");
  } else if (usrRsp == 2) {
    gST->ConOut->OutputString(gST->ConOut, L"\r\n    Enter Server based Recovery : ");
  }


  do {
    TimeOutValue = pTimeOut ? *pTimeOut : 0;
    if(TimeOutValue) {
      timer = NULL;
      Status = TimerCreateTimer (&timer, SetPwdTimeOut, (VOID *)&bTimeOut, TimerRelative, TimeOutValue * TIMER_ONE_SECOND, TPL_NOTIFY);
      if(EFI_ERROR(Status)) {
        DEBUG ((EFI_D_ERROR, "TDT::Error - Could Not Create Timer Using Core Resources - Early Exit\n"));
        return EFI_TIMEOUT;
      }
    }
    // While the timeout has not expired
    while ( ! bTimeOut ) {

      Status = gST->ConIn->ReadKeyStroke( gST->ConIn, &Key );
      if ( !(EFI_ERROR(Status)) ) {
        break;
      }
    }

    if(TimeOutValue)
      TimerStopTimer( &timer );

    if(bTimeOut) {
      Status = gST->ConIn->Reset( gST->ConIn, FALSE );

      PasswordEntered[0] = L'\0';
      PasswordIn[0] = L'\0';

      break;
    }

    DEBUG ((EFI_D_ERROR, "TDT::Timeout : %d\n", TimeOutValue));
    gST->ConOut->EnableCursor( gST->ConOut, TRUE );
    switch (Key.UnicodeChar) {
      case CHAR_NULL:
        // Ignore ESC and ask the password again rather than EFI_ABORT
        if (Key.ScanCode == SCAN_ESC) {

          Status = gST->ConIn->Reset( gST->ConIn, FALSE );
          PasswordEntered[0] = L'\0';
          PasswordIn[0] = L'\0';
          StrIndex = 0;
          gST->ConOut->OutputString(gST->ConOut, L"\r\n Escaped Pressed. Restart again ....");
          gBS->Stall (1000000);
          return EFI_SUCCESS;
        }
        break;

      case CHAR_CARRIAGE_RETURN:
        // When pressed entered then return with the password as successful
        Status = gST->ConIn->Reset( gST->ConIn, FALSE );
        StrIndex = 0;
        gST->ConOut->OutputString(gST->ConOut, L"\r\n    Checking Recovery Password ..");
        gBS->Stall (1000000);
        DEBUG ((EFI_D_ERROR, "TDT::Passphrase Entered : %s\n", PasswordEntered));

        gST->ConOut->EnableCursor(gST->ConOut, FALSE);
        return EFI_SUCCESS;

      case CHAR_BACKSPACE:
        if (StrIndex > 0 ) {
          BackSpace = 1;
          StrIndex--;
          PasswordEntered[StrIndex] = L'\0';
          PasswordIn[StrIndex] = L'\0';
          StarChar[0] = L'\b';

        }
        break;

      default:
        StarChar[0] = L'*';
        PasswordEntered[StrIndex] = (UINT8) Key.UnicodeChar;
        PasswordIn[StrIndex] = Key.UnicodeChar;
        if (usrRsp == 2)  StarChar[0] = Key.UnicodeChar;
        StrIndex++;
        if (StrIndex == 50) {
          gST->ConOut->OutputString(gST->ConOut, L"\r\n    Exceeded Max password size of 50 charaters... Shutting down system");
          gBS->Stall (1000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);

          return EFI_SUCCESS;
        }
        break;

    }

    //BackSpace handling is bit tricky ... here it is
    if (BackSpace == 1) {
      BackSpace = 0;
      StarChar[0] = L'\b';
      gST->ConOut->OutputString(gST->ConOut, StarChar);
      StarChar[0] = L' ';
      gST->ConOut->OutputString(gST->ConOut, StarChar);
      StarChar[0] = L'\b';
      gST->ConOut->OutputString(gST->ConOut, StarChar);

    } else if (StrIndex < 0) {
      // should never come here ...
      gST->ConOut->EnableCursor(gST->ConOut, FALSE);
      gST->ConOut->SetCursorPosition( gST->ConOut, gST->ConOut->Mode->CursorColumn, gST->ConOut->Mode->CursorRow);
      StrIndex = 0;

    } else if (StrIndex > 0) {
      //printing the "*" for each characters entered

      gST->ConOut->OutputString(gST->ConOut, StarChar);

    }

  } while( 1 );

  if(pTimeOut != NULL)
    *pTimeOut = 0;
  gST->ConOut->EnableCursor(gST->ConOut, FALSE);
  return EFI_TIMEOUT;

}



EFI_STATUS
ValidatePreferredAM (
  UINT8 *TdtState,
  UINT16 *TdtAmPref
)

{
  EFI_STATUS                        Status;
  UINTN                             VarSize;
  UINT8                             PbaFailedCount;
  UINT32                            Attributes;

  VarSize = sizeof (UINT8);


  if (*TdtAmPref == TDT_AM_SELECTION_TDTAM )
    return EFI_SUCCESS;

  if (*TdtState == TDT_STATE_ACTIVE) {

    Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;

    Status = gRT->GetVariable (
                    L"PBA_FAILED_COUNT",
                    &gEfiVlv2VariableGuid,
                    &Attributes,
                    &VarSize,
                    &PbaFailedCount
                    );

    if(EFI_ERROR(Status) && Status != EFI_NOT_FOUND) {
      DEBUG((EFI_D_ERROR, "TDTAm::ValidatePreferredAM failed, Status = %r\n", Status));
      return Status;
    }
    DEBUG((EFI_D_ERROR, "TDTAm::ValidatePreferredAM PBACount %x\n", PbaFailedCount));
    if (Status == EFI_NOT_FOUND || PbaFailedCount > 0) {

      DEBUG((EFI_D_ERROR, "TDTAM::ValidatePreferredAM In State Active PbaFailedCount not yet defined\n" ));

      // This will be the case 1st time after enrollment when  PbaFailedCount i.e PREFERRED_AM is not defined
      // Define this variable here ....
      PbaFailedCount = 0;
      gRT->SetVariable (
        L"PBA_FAILED_COUNT",
        &gEfiVlv2VariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
        sizeof (UINT8),
        &PbaFailedCount
      );
    } // close (Status == EFI_NOT_FOUND || PbaFailedCount > 0)
  } // close (*TdtState == TDT_STATE_ACTIVE)

  // Now get NVRAM Varible that store the PBA_Preferred_Failed count .. i.e. STOLEN_STATE which
  // which is incremented every boot

  if (*TdtState == TDT_STATE_STOLEN && *TdtAmPref == TDT_AM_SELECTION_PBAM ) {

    VarSize = sizeof (UINT8);
    Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    Status = gRT->GetVariable (
                    L"PBA_FAILED_COUNT",
                    &gEfiVlv2VariableGuid,
                    &Attributes,
                    &VarSize,
                    &PbaFailedCount
                    );

    if(EFI_ERROR(Status) && Status != EFI_NOT_FOUND) {
      DEBUG((EFI_D_ERROR, "TDTAm::ValidatePreferredAM failed, Status = %r\n", Status));
      return Status;
    }
    DEBUG((EFI_D_ERROR, "TDTAm::ValidatePreferredAM PBACount %x\n", PbaFailedCount));

    if (Status == EFI_NOT_FOUND) {
      DEBUG((EFI_D_ERROR, "TDTAM::ValidatePreferredAM In State Stolen PbaFailedCount not yet defined\n" ));

      // This will be the case 1st time after enrollment when  PbaFailedCount i.e PREFERRED_AM is not defined and
      // AT somehow got into stolen mode
      // Define this variable here ....

      PbaFailedCount = 1;
      gRT->SetVariable (
        L"PBA_FAILED_COUNT",
        &gEfiVlv2VariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
        sizeof (UINT8),
        &PbaFailedCount
      );
    } else if (PbaFailedCount > 0 && PbaFailedCount == (UINT8) gBiosRecoveryGlobalVar.PbaOverRideThreshold ) {


      // Set the PREFERRED_AM to BIOS AM here ....
      if (pTdtPlatformPolicy->Tdt.TdtConfig == 1) {
        *TdtAmPref = TDT_AM_SELECTION_TDTAM;
        DEBUG((EFI_D_ERROR, "TDTAM::ValidatePreferredAM PbaFailedCount  = %d\n", PbaFailedCount ));
      }
    } else {
      // Increment the PbaFailedCount  count here ....


      PbaFailedCount = PbaFailedCount + 1;
      gRT->SetVariable (
        L"PBA_FAILED_COUNT",
        &gEfiVlv2VariableGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
        sizeof (UINT8),
        &PbaFailedCount
      );

      DEBUG((EFI_D_ERROR, "TDTAM::ValidatePreferredAM PbaFailedCount incremented here:%d\n", PbaFailedCount));
      DEBUG((EFI_D_ERROR, "TDTAM::ValidatePreferredAM gBiosRecoveryGlobalVar.PbaOverRideThreshold:%d\n",
             gBiosRecoveryGlobalVar.PbaOverRideThreshold));


    }
  }
  return EFI_SUCCESS;
}



EFI_STATUS
DisplayIsvStrings ()
{

  UINT32 CRow;
  UINT32 CCol;

  UINT8 CurrentLanguage[4];
  EFI_TDT_PROTOCOL    *pTdt = NULL;
  EFI_STATUS           Status;
  UINT32               StringId;
  UINT8                *IsvString;
  UINT32               StringLength = 0;
  UINTN                PlatformRecvIdLen;
  CHAR16               *IsvIdStr;
  UINT32               IsvId = 0;

  // Get ISV Id

  IsvIdStr = AllocatePool(ISV_PLATFORM_ID_LENGTH * sizeof(CHAR16));
  SetMem( IsvIdStr, ISV_PLATFORM_ID_LENGTH * sizeof(CHAR16), 0 );

  StringId = TDT_VENDOR_STRING_ID_RECOVERY_HELP; // 1
  // Allocating memory for password
  IsvString = AllocatePool((RECOVERY_STRING_LENGTH + 1) * sizeof(UINT8));
  SetMem( IsvString, (RECOVERY_STRING_LENGTH + 1)*sizeof(UINT8), 0 );

  GetCurrentLang (CurrentLanguage);

  DEBUG((EFI_D_ERROR, "TdtAm::BIOS Default language = %s\n", (CHAR8 *)CurrentLanguage));

  Status = gBS->LocateProtocol (
                  &gEfiTdtProtocolGuid,
                  NULL,
                  (VOID **) &pTdt
                  );


  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::DisplayIsvString Tdt Protocol failed, Status = %r\n", Status));
    return Status;
  }

  Status = pTdt->GetRecoveryString(pTdt, &StringId, IsvString, &StringLength);
  if (StringLength > sizeof(TDT_BIOS_RECOVERY_CONFIG)) {
    StringLength = sizeof(TDT_BIOS_RECOVERY_CONFIG);
  }

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::GetRecoveryString failed, Status = %r\n", Status));
    // do not return check if ISV Id can be retrived
    // Let it continue even if the error ....
  }

  Status = pTdt->GetIsvId(pTdt, &IsvId);

  if(EFI_ERROR(Status)) {
    // Let it continue even if the error ....
    DEBUG((EFI_D_ERROR, "TDTAm::GetIsvId failed, Status = %r\n", Status));
  }




  DEBUG((EFI_D_ERROR, "TDTAm::IsvString StringLength = %d\n", StringLength));

  PlatformRecvIdLen = StrLenUnaligned(gBiosRecoveryGlobalVar.IsvPlatformId);

  DEBUG((EFI_D_ERROR, "TDTAm::IsvString StringLength = %d\n", PlatformRecvIdLen));
  DEBUG((EFI_D_ERROR, "TDTAm::IsvString StringLength = %s\n", gBiosRecoveryGlobalVar.IsvPlatformId));

  if (StringLength > 0 || PlatformRecvIdLen > 0 || IsvId > 0) {

    CRow = gST->ConOut->Mode->CursorRow;
    CCol = gST->ConOut->Mode->CursorColumn;

    gST->ConOut->OutputString(gST->ConOut, L"\r\n\n\n\n\n\n\n\n\n\n\n\n\n\n    ");
    if (StringLength > 0) {
      gST->ConOut->OutputString(gST->ConOut, (CHAR16 *)IsvString );
    }
    if (PlatformRecvIdLen > 0) {
      gST->ConOut->OutputString(gST->ConOut, L"\r\n    Platform ID: ");
      gST->ConOut->OutputString(gST->ConOut, (CHAR16 *)gBiosRecoveryGlobalVar.IsvPlatformId );
    }

    if (IsvId) {
      UnicodeValueToString(IsvIdStr,  0, (UINT32) IsvId, 0);
      gST->ConOut->OutputString(gST->ConOut, L"\r\n    Intel(R) AT service provider Id:");
      gST->ConOut->OutputString(gST->ConOut, IsvIdStr);
    }

    gST->ConOut->SetCursorPosition( gST->ConOut, CCol, CRow);

  }

  FreePool(IsvString);
  FreePool(IsvIdStr);
  return EFI_SUCCESS;

}

EFI_STATUS
GetRecoveryConfig(
  TDT_BIOS_RECOVERY_CONFIG  *RecoveryConfig,
  UINT32 *RecoveryStringLength
)

{
  EFI_TDT_PROTOCOL *pTdt = NULL;
  EFI_STATUS               Status;
  UINT32 StringId;
  UINT8  *RawRecoveryConfig;
  UINT32  StringLength = 0;

  StringId = TDT_CUSTOM_RECOVERY_ID_CONFIGURATIONS; //2

  Status = gBS->LocateProtocol (
                  &gEfiTdtProtocolGuid,
                  NULL,
                  (VOID **) &pTdt
                  );


  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::GetRecoveryConfig Tdt Protocol failed, Status = %r\n", Status));
    return Status;
  }

  // Allocating memory for password
  RawRecoveryConfig = AllocatePool((RECOVERY_STRING_LENGTH ) * sizeof(UINT8));
  SetMem( RawRecoveryConfig, (RECOVERY_STRING_LENGTH)*sizeof(UINT8), 0 );

  Status = pTdt->GetRecoveryString(pTdt, &StringId, RawRecoveryConfig, &StringLength);

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "TDTAm::GetRecoveryConfig failed to get Recovery String ID-2, Status = %r\n", Status));
  }

  DEBUG((EFI_D_ERROR, "TDTAm::GetRecoveryConfig StringLength = %d\n", StringLength));

  if (StringLength > sizeof(TDT_BIOS_RECOVERY_CONFIG)) {
    StringLength = sizeof(TDT_BIOS_RECOVERY_CONFIG);
  }
  CopyMem((UINT8 *)RecoveryConfig, RawRecoveryConfig, StringLength);
  *RecoveryStringLength = StringLength;

  FreePool(RawRecoveryConfig);
  return EFI_SUCCESS;

}


EFI_STATUS
GetCurrentLang (
  OUT     UINT8              *Lang
)
/*++

Routine Description:

  Determine what is the current language setting

Arguments:

  Lang      - Pointer of system language

Returns:

  Status code

--*/
{
  EFI_STATUS  Status;
  UINTN       Size;
  CHAR8       Language[4];

  //
  // Getting the system language and placing it into our Global Data
  //
  Size = sizeof (Language);

  Status = gRT->GetVariable (
                  L"Lang",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &Size,
                  Language
                  );

  if (EFI_ERROR (Status)) {
    AsciiStrCpy (Language, "eng");
  }

  //
  // Null-terminate the value
  //
  CopyMem(Lang, (UINT8 *)Language, 4);

  return Status;
}

TDT_INFOMATION  mTdtInfo;

TDT_OPERATION_PROTOCOL   mTdtOperationProtocol = {
  GetPlatformTdtInfo,
  GetPlatformTdtOperation,
  PerformTdtOperation
};

VOID
SetTdtInfo (
)
{
  if (pTdtPlatformPolicy->Tdt.TdtConfig == 1) {
    mTdtInfo.TdtEnabled = 1;
  } else {
    mTdtInfo.TdtEnabled = 0;
  }
  if (pTdtPlatformPolicy->Tdt.TdtPBAEnable == 1) {
    mTdtInfo.PBACapable = 1;
  } else {
    mTdtInfo.PBACapable = 0;
  }
  if (gAtStateGlobalVar.AtState == 0) {
    mTdtInfo.TdtEnrolled = 0;
  } else {
    mTdtInfo.TdtEnrolled = 1;
  }
  mTdtInfo.TdtState = gAtStateGlobalVar.AtState;
  DEBUG((EFI_D_ERROR, "Set Tdt info %x\n", mTdtInfo.TdtEnrolled));
  mTdtInfo.TdtWWAN = 0;
}

BOOLEAN
CheckTdtStatus(
  IN UINT32          SeCStatus
)
{
  return TRUE;
}

VOID
PrepareDisplayWarnToShutDownSystem (
)
{
}

EFI_STATUS
GetPlatformTdtInfo (
  OUT TDT_INFOMATION *TdtInfo
)
{
  CopyMem(TdtInfo, &mTdtInfo, sizeof(TDT_INFOMATION));
  return EFI_SUCCESS;
}

EFI_STATUS
GetPlatformTdtOperation (
  OUT TDT_PERFORM_OPERATION_ID *TdtOperation
)
{
  *TdtOperation = mPerformOperation;
  return EFI_SUCCESS;
}

EFI_STATUS
PerformTdtOperation (
  BOOLEAN     *SetTdtEnterSuspendState,
  UINT8       *TdtEnterSuspendState
)
{

  DEBUG((EFI_D_ERROR, "TDTAM: PerformTdtOperation Protocol %x\n", mPerformOperation));
  if (mPerformOperation == TDT_PERFORM_SUSPEND) {
    ProcessSuspendMode(NULL, NULL, SetTdtEnterSuspendState, TdtEnterSuspendState);
  } else if (mPerformOperation == TDT_PERFORM_ATAM_RECOVERY) {
    CheckRecoveryPassword(NULL, NULL);
  } else if (mPerformOperation == TDT_PERFORM_NOTIFY) {
    DisplayATNotifyScreen(mNotifyDelayTime);
  }
  return EFI_SUCCESS;
}

EFI_STATUS
InstallTdtOperation(
  IN EFI_HANDLE ImageHandle
)
{
  DEBUG((EFI_D_ERROR, "TDTAM: InstallTdtOperation Protocol\n"));
  return gBS->InstallMultipleProtocolInterfaces (
                &ImageHandle,
                &gEfiTdtOperationProtocolGuid,
                &mTdtOperationProtocol,
                NULL
                );

}
