/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2007 - 2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Hecidrv.c

Abstract:

  HECI driver

--*/

#include "Hecidrv.h"

#ifdef ECP_FLAG
EFI_GUID gEfiHeciProtocolGuid = HECI_PROTOCOL_GUID;
EFI_GUID gDxePlatformSeCPolicyGuid = DXE_PLATFORM_SEC_POLICY_GUID;
EFI_GUID gEfiSeCRcInfoProtocolGuid = EFI_SEC_RC_INFO_PROTOCOL_GUID;
EFI_GUID gEfiTdtOperationProtocolGuid = EFI_TDT_OPERATION_PROTOCOL_GUID;
#endif





extern DXE_SEC_POLICY_PROTOCOL *mDxePlatformSeCPolicy;

#define ONE_SECOND_TIMEOUT  1000000
#define FWU_TIMEOUT         90

//
// Global driver data
//
HECI_INSTANCE *mHeciContext;
EFI_HANDLE    mHeciDrv;
EFI_EVENT     mExitBootServicesEvent;
EFI_EVENT     mLegacyBootEvent;

UINT32
CheckAndFixHeciForAccess (
  VOID
  )
/*++

Routine Description:
  This function provides a standard way to verify the HECI cmd and MBAR regs
  in its PCI cfg space are setup properly and that the local mHeciContext
  variable matches this info.

Arguments:
  None.

Returns:
  VOID

--*/
{
  //
  // Read HECI_MBAR in case it has changed
  //
  mHeciContext->HeciMBAR = HeciPciRead32 (R_HECIMBAR1) & 0xFFFFFFF0;

  //
  // Check if HECI_MBAR is disabled
  //
  if ((HeciPciRead8 (PCI_COMMAND_OFFSET) & (EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER)) !=
      (EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER)
     ) {
    //
    // If cmd reg in pci cfg space is not turned on turn it on.
    //
    HeciPciOr16 (
      PCI_COMMAND_OFFSET,
      EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_SERR
      );
  }

  return mHeciContext->HeciMBAR;
}

VOID
EnableHpet (
  VOID
  )
/*++

Routine Description:

  Enable Hpet function.

Arguments:

  None.

Returns:

  None.

--*/
{

  mHeciContext->HpetTimer = (VOID *)(UINTN)(HPET_ADDRESS);

  //
  // Start the timer so it is up and running
  //
  //AMI_OVERRIDE - EIP144604 HPET protocol support >>
  if ((mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW] & HPET_START) != HPET_START) { 
    mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW]  = HPET_START;
    mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW]  = HPET_START;    
  }
  //AMI_OVERRIDE - EIP144604 HPET protocol support <<
  
  DEBUG ((EFI_D_INFO, "EnableHpet %x %x\n\n ",HPET_GEN_CONFIG_LOW,mHeciContext->HpetTimer));
  return ;

  /*
  VOLATILE UINT32 *HpetConfigReg;

  HpetConfigReg = NULL;
  //
  // Get the High Precision Event Timer base address and enable the memory range
  //
  HpetConfigReg = (UINT32 *) (UINTN) (PCH_RCRB_BASE + R_PCH_RCRB_HPTC);
  switch (*HpetConfigReg & B_PCH_RCRB_HPTC_AS) {
    case 0:
      mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_0);
      break;

    case 1:
      mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_1);
      break;

    case 2:
      mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_2);
      break;

    case 3:
      mHeciContext->HpetTimer = (VOID *) (UINTN) (HPET_ADDRESS_3);
      break;

    default:
      mHeciContext->HpetTimer = NULL;
      break;
  }
  //
  // Read this back to force the write-back.
  //
  *HpetConfigReg = *HpetConfigReg | B_PCH_RCRB_HPTC_AE;

  //
  // Start the timer so it is up and running
  //
  mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW]  = HPET_START;
  mHeciContext->HpetTimer[HPET_GEN_CONFIG_LOW]  = HPET_START;

  return;
  */
}

EFI_STATUS
SeCWarningMessage (
  VOID
  )
/*++
Routine Description:

  Show warning message to user.

Arguments:

  None.

Returns:

  Status.

--*/
{
  HECI_FWS_REGISTER SeCFirmwareStatus;

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  //
  // Check for ME FPT Bad & FT BUP LD FLR
  //
  if (SeCFirmwareStatus.r.FptBad != 0 || SeCFirmwareStatus.r.FtBupLdFlr != 0) {
    SeCReportError (MSG_SEC_FW_UPDATE_FAILED);
  }

  return EFI_SUCCESS;
}

VOID
DeviceStatusSave (
  VOID
  )
/*++

Routine Description:

  Store the current value of DEVEN for S3 resume path

Arguments:

  None

Returns:

  None

--*/
{
// TODO: need some details on this function
//  UINT32  Data;

  //
  // Read RCBA register for saving
  //
//  Data = Mmio16 (PCH_RCRB_BASE, R_PCH_RCRB_FD2);

//  SCRIPT_MEM_WRITE (
//   EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
//    EfiBootScriptWidthUint16,
//    (UINTN) (PCH_RCRB_BASE + R_PCH_RCRB_FD2),
//   1,
//    &Data
//    );

//  Data = Mmio16 (PCH_RCRB_BASE, R_PCH_RCRB_USB_MISCCTL);
  /*
  SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint16,
    (UINTN) (PCH_RCRB_BASE + R_PCH_RCRB_USB_MISCCTL),
    1,
    &Data
    );

  Data = Mmio16 (PCH_RCRB_BASE, R_PCH_RCRB_FDSW);

  SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint16,
    (UINTN) (PCH_RCRB_BASE + R_PCH_RCRB_FDSW),
    1,
    &Data
    );
  */
}

EFI_STATUS
SeCDeviceConfigure (
  VOID
  )
/*++

Routine Description:

  Disable ME Devices when needed and save DEVEN

Arguments:

  None

Returns:

  EFI_SUCCESS - Always return EFI_SUCCESS

--*/
{
  return EFI_SUCCESS;
}


EFI_STATUS
SeCEndOfPostEvent (
  VOID
  )
/*++
Routine Description:

  Send SEC the BIOS end of Post message.

Arguments:

  None.

Returns:

  Status.

--*/
{
  EFI_STATUS        Status;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCStatus;
  UINT32 HeciBar1Value, HAlivenessResponse, Timeout;
  TDT_OPERATION_PROTOCOL    *TdtOp;
  TDT_INFOMATION            TdtInfo;
  AT_STATE_STRUCT           ATInfo;
  BOOLEAN                   BypassEOP;
  //
  // Init SEC Policy Library
  //
  Status = SeCPolicyLibInit ();
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // If ATAM is unavailable and the platform is in stolen state,
  // while Recovery Module is set to ATAM, don't send EOP command.
  //
  BypassEOP = FALSE;
  Status = gBS->LocateProtocol (
                 &gEfiTdtOperationProtocolGuid,
                 NULL,
                 (VOID **) &TdtOp
                 );
  if (!EFI_ERROR(Status)) {
    Status = TdtOp->GetPlatformTdtInfo(
                    &TdtInfo
                    );
    if ((!EFI_ERROR(Status)) && ((TdtInfo.TdtEnabled == 0) && (TdtInfo.TdtState == TDT_STATE_STOLEN))) {
      Status = HeciGetAtFwStateInfoMsg (&ATInfo);
      if ((!EFI_ERROR(Status)) && (ATInfo.AtAmPref == TDT_AM_SELECTION_TDTAM)) {
        BypassEOP = TRUE;
      }
    }
  }


  if (!BypassEOP) {
    Status = gBS->LocateProtocol (
                    &gEfiHeciProtocolGuid,
                    NULL,
                    (VOID **) &Heci
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Check SEC Status
      //
      Status = Heci->GetSeCStatus (&SeCStatus);
      ASSERT_EFI_ERROR (Status);

      //
      // Send EOP message when SEC is ready.  Do not care about if SEC FW INIT is completed.
      //
      if ((SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_IN_RECOVERY_MODE) ||
          (SEC_STATUS_SEC_STATE_ONLY (SeCStatus) == SEC_READY)) {
        if (SeCEndOfPostEnabled ()) {
          //
          // Send SEC the BIOS Extensions exit code and End Of Post message.
          // if no success, we display an error message and halt the system.
          //
          DEBUG ((EFI_D_ERROR, "Sending  <<<< EOP >>>>...\n"));
          Status = HeciSendEndOfPostMessage ();

          if ((EFI_ERROR (Status)) && (Status != EFI_UNSUPPORTED)) {
            SeCReportError (MSG_EOP_ERROR);
            CpuDeadLoop ();
          }
        } // End of EOP setup option
      }
    } // End of EFI_ERROR of locate HECI driver
  }

  //
  //  Before completing POST, BIOS must clear SICR_HOST_ALIVENESS_REQ.ALIVENESS_REQ as its last action, before handing over control to the OS.
  //
  HeciBar1Value =  HeciPciRead32 (HECI_BAR1);

  Mmio32And(HeciBar1Value, R_SICR_HOST_ALIVENESS_REQ, (~B_ALIVENESS_REQ));

  DEBUG ((EFI_D_ERROR, "Before completing POST, BIOS must clear SICR_HOST_ALIVENESS_REQ.ALIVENESS_REQ \n"));

  HAlivenessResponse =  Mmio32(HeciBar1Value, R_HICR_HOST_ALIVENESS_RESP);

  Timeout = 0;
  while(((HAlivenessResponse & B_ALIVENESS_ACK) != B_ALIVENESS_ACK) && (Timeout < 5) ) {
    gBS->Stall (ONE_SECOND_TIMEOUT);
    HAlivenessResponse = Mmio32(HeciBar1Value, R_HICR_HOST_ALIVENESS_RESP);
    DEBUG ((EFI_D_ERROR, "Read HOST Alive ACK: %x %x\n", HAlivenessResponse, Timeout));
    Timeout++;
  }

  // TODO:
  // Note that after host writes to this bit, it must wait for an acknowledgement that is generated by bit <TBD> before assuming that SeC has received the request.
  // Until such acknowledgement is received, Host must not generate additional requests or request cancellations on this bit.
  //
  if (Timeout >= 5) {
    DEBUG ((EFI_D_INFO, "Timeout occurred waiting for host aliveness ACK.\n"));
  } else {
    DEBUG ((EFI_D_INFO, "SEC host aliveness ACK received.\n"));
  }
  //
  // PAVP enabling code moved to GraphicsDxeInit
  //
  // PciOr32 (PCI_LIB_ADDRESS (0, 2, 0, 0x74), 0x00000007);

  return EFI_SUCCESS;
}

EFI_STATUS
HeciTrConfig (
  VOID
  )
/*++
Routine Description:

  Send Thermal Reporting Configuration to SEC if Thermal Reporting is enabled

Arguments:

  None

Returns:

  Status.

--*/
{
  EFI_STATUS        Status;
  EFI_HECI_PROTOCOL *Heci;

  //
  // Init SEC Policy Library
  //
  Status = SeCPolicyLibInit ();
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (!EFI_ERROR (Status)) {
    if (SeCTrEnabled ()) {
      HeciTrConfigMsg (Heci, mDxePlatformSeCPolicy->SeCConfig.TrConfig);
    }
  }

  return Status;
}


VOID
CheckFwUpdInProgress (
  VOID
  )
/*++
Routine Description:

  Halt Boot for up to 90 seconds if Bit 11 of FW Status Register (FW_UPD_IN_PROGRESS) is set

Arguments:

  VOID

Returns:

  VOID.

--*/
{
  HECI_FWS_REGISTER FwStatus;
  UINT8             StallCount;
  EFI_STATUS        Status;

  StallCount  = 0;
  Status      = mHeciContext->HeciCtlr.GetSeCStatus (&FwStatus.ul);
  if (!EFI_ERROR (Status)) {
    if (FwStatus.ul & SEC_FW_UPDATES_IN_PROGRESS) {
      Status = SeCPolicyLibInit ();
      if (Status == EFI_SUCCESS) {
        SeCReportError (MSG_SEC_FW_UPDATE_WAIT);
      }
    }

    while ((FwStatus.ul & SEC_FW_UPDATES_IN_PROGRESS) && (StallCount < FWU_TIMEOUT)) {
      gBS->Stall (ONE_SECOND_TIMEOUT);
      StallCount  = StallCount + 1;
      Status      = mHeciContext->HeciCtlr.GetSeCStatus (&FwStatus.ul);
    }
  }

  return ;
}

EFI_STATUS
LockConfig (
  VOID
  )
{
  EFI_STATUS        Status;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;
  HECI_FWS_REGISTER SeCFirmwareStatus;
#if CF9_REG_LOCK_ENABLE 
  UINTN             Address;
#endif
  UINT32            Data;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (!EFI_ERROR (Status)) {
    ///
    /// Check SeC Status
    ///
    Status = Heci->GetSeCMode (&SeCMode);
    ASSERT_EFI_ERROR (Status);

    SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

    Data = 0;
#if CF9_REG_LOCK_ENABLE
    if ((((SeCMode == SEC_MODE_NORMAL) || (SeCMode == SEC_MODE_TEMP_DISABLED)) && !(SeCFirmwareStatus.r.ManufacturingMode))) {
      Data |= B_PCH_PMC_PMIR_CF9LOCK;
    }

    Address = PciRead32( PCI_LIB_ADDRESS (
                DEFAULT_PCI_BUS_NUMBER_PCH,
                PCI_DEVICE_NUMBER_PCH_LPC,
                PCI_FUNCTION_NUMBER_PCH_LPC,
                R_PCH_LPC_PMC_BASE
                ));
    Address &= B_PCH_LPC_PMC_BASE_BAR;
    MmioAndThenOr32 (
      Address + R_PCH_PMC_PMIR,
      (UINT32) (~(B_PCH_PMC_PMIR_CF9LOCK | B_PCH_PMC_PMIR_CF9GR)),
      (UINT32) Data
    );
#endif
  }

  return Status;
}

VOID
SeCReadyToBootEvent (
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
  EFI_STATUS  Status;
//  EFI_EVENT   AmtReadyToBootEvent;
//  EFI_EVENT   SeCPlatformReadyToBootEvent;
  UINT32      SeCMode;
  UINT32      SeCStatus;

  DEBUG ((EFI_D_ERROR, "SeCReadyToBootEvent ++\n"));

  //
  // We will trigger all events in order
  //
  /*
  Status = gBS->CreateEventEx (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_CALLBACK,
                  SeCEmptyEvent,
                  NULL,
                  &gAmtReadyToBootGuid,
                  &AmtReadyToBootEvent
                  );
   ASSERT_EFI_ERROR (Status);
   if (!EFI_ERROR (Status)) {
     gBS->SignalEvent (AmtReadyToBootEvent);
     gBS->CloseEvent (AmtReadyToBootEvent);
   }
  */

//  Status = gBS->CreateEventEx (
//                  EFI_EVENT_NOTIFY_SIGNAL,
//                  EFI_TPL_CALLBACK,
//                  SeCEmptyEvent,
//                  NULL,
//                  &gSeCPlatformReadyToBootGuid,
//                  &SeCPlatformReadyToBootEvent
//                  );
//  ASSERT_EFI_ERROR (Status);
//  if (!EFI_ERROR (Status)) {
//    gBS->SignalEvent (SeCPlatformReadyToBootEvent);
//    gBS->CloseEvent (SeCPlatformReadyToBootEvent);
//  }

  HeciGetSeCMode(&SeCMode);
  HeciGetSeCStatus(&SeCStatus);
  if ((SeCMode == SEC_MODE_NORMAL) &&
      ((SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_IN_RECOVERY_MODE) ||
       (SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY))) {


    CheckFwUpdInProgress ();
    SeCPlatformHook ();

    Status = SeCWarningMessage ();
    ASSERT_EFI_ERROR (Status);

    Status = SeCEndOfPostEvent ();
    ASSERT_EFI_ERROR (Status);

    Status = LockConfig ();
    ASSERT_EFI_ERROR (Status);
  }

  gBS->CloseEvent (Event);

  DEBUG ((EFI_D_ERROR, "SeCReadyToBootEvent --\n"));
  return;
}

EFI_STATUS
InitializeHECI (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  HECI driver entry point used to initialize support for the HECI device.

Arguments:
  ImageHandle - Standard entry point parameter.
  SystemTable - Standard entry point parameter.

Returns:
  EFI_STATUS

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   ReadyToBootEvent;
  UINT32    DeviceInfo;
  UINT32    Data32;
  DEBUG ((EFI_D_ERROR, "InitializeHECI     1\n"));

  DEBUG ((EFI_D_ERROR, "InitializeHeciPrivate ++ \n "));
  DEBUG ((EFI_D_ERROR, "Send Shadow Done Message Start\n"));
  Data32 = HeciPciRead32 (0x64);
  DEBUG ((EFI_D_ERROR, "Send Shadow Done Message read Offset 0x64 is %x \n", Data32));
  Data32 |= 1;
  DEBUG ((EFI_D_ERROR, "Send Shadow Done Message write Offset 0x64 is %x try to send Shadow Done Message \n", Data32));
  HeciPciWrite32(0x64, Data32);
  DEBUG ((EFI_D_ERROR, "Sent Shadow Done Message End\n"));

  //
  // Store HECI vendor and device information away
  //
  DeviceInfo = HeciPciRead16 (PCI_DEVICE_ID_OFFSET);

  //
  // Check for HECI PCI device availability
  //
  if(!(DeviceInfo >= S_SEC_DevID_RANGE_LO && DeviceInfo <= S_SEC_DevID_RANGE_HI)) {
    DEBUG ((EFI_D_INFO, "SeC Device ID: %x\n", DeviceInfo));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_ERROR, "InitializeHECI ++\n"));

  mHeciDrv      = ImageHandle;
  mHeciContext  = AllocateZeroPool (sizeof (HECI_INSTANCE));
  //
  // Initialize HECI protocol pointers
  //
  if (mHeciContext != NULL) {
    mHeciContext->HeciCtlr.ResetHeci    = ResetHeciInterface;
    mHeciContext->HeciCtlr.SendwACK     = HeciSendwACK;
    mHeciContext->HeciCtlr.ReadMsg      = HeciReceive;
    mHeciContext->HeciCtlr.SendMsg      = HeciSend;
    mHeciContext->HeciCtlr.InitHeci     = HeciInitialize;
    mHeciContext->HeciCtlr.ReInitHeci   = HeciReInitialize;
    mHeciContext->HeciCtlr.SeCResetWait  = SeCResetWait;
    mHeciContext->HeciCtlr.GetSeCStatus  = HeciGetSeCStatus;
    mHeciContext->HeciCtlr.GetSeCMode    = HeciGetSeCMode;

  }

  //
  // Install the HECI interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHeciContext->Handle,
                  &gEfiHeciProtocolGuid,
                  &mHeciContext->HeciCtlr,
                  NULL
                  );
  //
  // Initialize the HECI device
  //
  Status = InitializeHeciPrivate ();

  if ((EFI_ERROR (Status)) || (mHeciContext == NULL)) {
    //
    // Don't install on ERR
    //
    DEBUG ((EFI_D_ERROR, "HECI not initialized - Removing devices from PCI space!\n"));
    //
    // Store the current value of DEVEN for S3 resume path
    //
    DeviceStatusSave ();

    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }
  //
  // Initialize the SeC Reference Code Information
  //
  mHeciContext->SeCRcInfo.Revision = SEC_RC_INFO_PROTOCOL_REVISION_1;

  //
  // SeC Reference Code formats 0xAABBCCDD
  //   DD - Build Number
  //   CC - Reference Code Revision
  //   BB - Reference Code Minor Version
  //   AA - Reference Code Major Version
  // Example: SeC Reference Code 0.7.1 should be 00 07 01 00 (0x00070100)
  //
  mHeciContext->SeCRcInfo.RCVersion.Data = SEC_RC_VERSION;

  //
  // Install the SeC Reference Code Information
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHeciContext->Handle,
                  &gEfiSeCRcInfoProtocolGuid,
                  &mHeciContext->SeCRcInfo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }
  //
  // Create an Ready to Boot event.
  //

  Status = EfiCreateEventReadyToBootEx (
            TPL_CALLBACK,
            SeCReadyToBootEvent,
            (VOID *) &ImageHandle,
            &ReadyToBootEvent
            );
  ASSERT_EFI_ERROR (Status);

 

  DEBUG ((EFI_D_ERROR, "InitializeHECI --\n"));
  return Status;
}
