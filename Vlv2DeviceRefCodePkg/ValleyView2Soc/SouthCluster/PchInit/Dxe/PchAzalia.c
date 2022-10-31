/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**


Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchAzalia.c

  @brief
  Initializes the PCH Azalia codec.

**/
#include "PchInit.h"

//
// Module global variables
//
extern EFI_PCH_S3_SUPPORT_PROTOCOL           *mPchS3Support;

EFI_STATUS
SetPollStatusS3Item (
  IN   UINT64                    MmioAddress,
  IN   EFI_BOOT_SCRIPT_WIDTH     Width,
  IN   UINT64                    Mask,
  IN   UINT64                    Value,
  IN   UINT32                    Timeout
  )
/*++

  Routine Description:

    Set a "Poll Status" S3 dispatch item

  Arguments:

    MmioAddress       Address
    Width             Operation Width
    Mask              Mask
    Value             Value to wait for
    Timeout           Timeout value in microseconds

  Returns:

    EFI_SUCCESS       The function completed successfully

--*/
{
  EFI_STATUS                              Status;
#ifndef ECP_FLAG
  EFI_BOOT_SCRIPT_SAVE_PROTOCOL           *mBootScriptSave;
#endif
  STATIC EFI_PCH_S3_PARAMETER_POLL_STATUS S3ParameterPollStatus;
  STATIC EFI_PCH_S3_DISPATCH_ITEM         S3DispatchItem = {
    PchS3ItemTypePollStatus,
    &S3ParameterPollStatus
  };
  EFI_PHYSICAL_ADDRESS                    S3DispatchEntryPoint;

  if (!mPchS3Support) {
    //
    // Get the PCH S3 Support Protocol
    //
    Status = gBS->LocateProtocol (
                    &gEfiPchS3SupportProtocolGuid,
                    NULL,
                    (VOID **) &mPchS3Support
                    );
    ASSERT_EFI_ERROR (Status);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  S3ParameterPollStatus.MmioAddress = MmioAddress;
  S3ParameterPollStatus.Width       = Width;
  S3ParameterPollStatus.Mask        = Mask;
  S3ParameterPollStatus.Value       = Value;

  //
  // 10s
  //
  S3ParameterPollStatus.Timeout = Timeout;
  Status = mPchS3Support->SetDispatchItem (
                            mPchS3Support,
                            &S3DispatchItem,
                            &S3DispatchEntryPoint
                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Save the script dispatch item in the Boot Script
  //
#ifdef ECP_FLAG
  S3BootScriptSaveDispatch (S3DispatchEntryPoint);
#else
  //S3BootScriptSaveDispatch ((VOID *)(UINTN) S3DispatchEntryPoint);
  Status = gBS->LocateProtocol (
                  &gEfiBootScriptSaveProtocolGuid,
                  NULL,
                  (VOID **) &mBootScriptSave
                  );

  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }
  mBootScriptSave->Write (
                    mBootScriptSave,
                    0,
                    EFI_BOOT_SCRIPT_DISPATCH_OPCODE,
                    S3DispatchEntryPoint
                    );

#endif
  return Status;
}

EFI_STATUS
StatusPolling (
  IN      UINT32          StatusReg,
  IN      UINT16          PollingBitMap,
  IN      UINT16          PollingData
  )
/**

  @brief
  Polling the Status bit

  @param[in] StatusReg            The regsiter address to read the status
  @param[in] PollingBitMap        The bit mapping for polling
  @param[in] PollingData          The Data for polling

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             Polling the bit map time out

**/
{
  UINT32  LoopTime;

  for (LoopTime = 0; LoopTime < AZALIA_MAX_LOOP_TIME; LoopTime++) {
    if ((MmioRead16 (StatusReg) & PollingBitMap) == PollingData) {
      break;
    } else {
      PchPmTimerStall (AZALIA_WAIT_PERIOD);
    }
  }

  if (LoopTime >= AZALIA_MAX_LOOP_TIME) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SendCodecCommand (
  IN      UINT32          HdaBar,
  IN OUT  UINT32          *CodecCommandData,
  IN      BOOLEAN         ReadBack
  )
/**

  @brief
  Send the command to the codec via the Immediate Command mechanism is written
  to the IC register

  @param[in] HdaBar               Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecCommandData     The Codec Command to be sent to the codec
  @param[in] ReadBack             Whether to get the response received from the codec

  @retval EFI_DEVICE_ERROR        Device status error, operation failed
  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS  Status;

  Status = StatusPolling (HdaBar + R_HDA_ICS, (UINT16) B_HDA_ICS_ICB, (UINT16) 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ICB bit is not zero before SendCodecCommand! \n"));
    return EFI_DEVICE_ERROR;
  }

  MmioWrite32 (HdaBar + R_HDA_IC, *CodecCommandData);
  MmioOr16 ((UINTN) (HdaBar + R_HDA_ICS), (UINT16) ((B_HDA_ICS_IRV | B_HDA_ICS_ICB)));

  Status = StatusPolling (HdaBar + R_HDA_ICS, (UINT16) B_HDA_ICS_ICB, (UINT16) 0);
  if (EFI_ERROR (Status)) {
    MmioAnd16 ((UINTN) (HdaBar + R_HDA_ICS), (UINT16)~(B_HDA_ICS_ICB));
    return Status;
  }

  if (ReadBack == TRUE) {
    if ((MmioRead16 (HdaBar + R_HDA_ICS) & B_HDA_ICS_IRV) != 0) {
      *CodecCommandData = MmioRead32 (HdaBar + R_HDA_IR);
    } else {
      DEBUG ((EFI_D_ERROR, "SendCodecCommand: ReadBack fail! \n"));
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SendCodecCommandS3Item (
  IN      UINT32          HdaBar,
  IN OUT  UINT32          CodecCommandData
  )
/**

  @brief
  Set a "Send Codec Command" S3 dispatch item

  @param[in] HdaBar               Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecCommandData     The Codec Command to be sent to the codec

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS                                      Status;
#ifndef ECP_FLAG
  EFI_BOOT_SCRIPT_SAVE_PROTOCOL                   *mBootScriptSave;
#endif
  STATIC EFI_PCH_S3_SUPPORT_PROTOCOL              *PchS3Support;
  STATIC EFI_PCH_S3_PARAMETER_SEND_CODEC_COMMAND  S3ParameterSendCodecCommand;
  STATIC EFI_PCH_S3_DISPATCH_ITEM                 S3DispatchItem = {
    PchS3ItemTypeSendCodecCommand,
    &S3ParameterSendCodecCommand
  };
  EFI_PHYSICAL_ADDRESS                            S3DispatchEntryPoint;

  if (!PchS3Support) {
    ///
    /// Get the PCH S3 Support Protocol
    ///
    Status = gBS->LocateProtocol (
                    &gEfiPchS3SupportProtocolGuid,
                    NULL,
                    (VOID **) &PchS3Support
                    );
    ASSERT_EFI_ERROR (Status);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  S3ParameterSendCodecCommand.HdaBar        = HdaBar;
  S3ParameterSendCodecCommand.CodecCmdData  = CodecCommandData;
  Status = PchS3Support->SetDispatchItem (
                           PchS3Support,
                           &S3DispatchItem,
                           &S3DispatchEntryPoint
                           );
  ASSERT_EFI_ERROR (Status);

  //
  // Save the script dispatch item in the Boot Script
  //
#ifdef ECP_FLAG
  S3BootScriptSaveDispatch (S3DispatchEntryPoint);
#else
  //S3BootScriptSaveDispatch ((VOID *)(UINTN)S3DispatchEntryPoint);
  Status = gBS->LocateProtocol (
                  &gEfiBootScriptSaveProtocolGuid,
                  NULL,
                  (VOID **) &mBootScriptSave
                  );

  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    0,
                    EFI_BOOT_SCRIPT_DISPATCH_OPCODE,
                    S3DispatchEntryPoint
                    );
#endif
  return Status;
}

VOID
S3BootScriptMsgBus32Write (
  UINT32 PortId,
  UINT32 Register,
  UINT32 DBuffer
  )
{
  UINT32 Reg;

  //
  // Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI));
  //

  Reg = Register;

  Reg &= MSGBUS_MASKHI;

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (EC_BASE + MC_MCRX),
    1,
    (VOID *) &Reg);

  DEBUG ((EFI_D_ERROR, "MCRX = %x\n", Reg));

  //
  // Mmio32( EC_BASE, MC_MDR ) = Dbuff;
  //

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (EC_BASE + MC_MDR),
    1,
    (VOID *) &DBuffer);

  DEBUG ((EFI_D_ERROR, "MDR = %x\n", DBuffer));

  //
  // Mmio32( EC_BASE, MC_MCR ) = (UINT32)( MsgBusWriteCmd(PortId)  | ((PortId) <<16) | ((Register & MSGBUS_MASKLO)<<8) | MESSAGE_DWORD_EN);
  //

  Register = MsgBusWriteCmd (PortId) | (PortId << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN;

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (EC_BASE + MC_MCR),
    1,
    (VOID *) &Register);

  DEBUG ((EFI_D_ERROR, "MCR = %x\n", Register));
}

EFI_STATUS
DetectAndInitializeAzalia (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  BOOLEAN                             *AzaliaStarted
  )
/**

  @brief
  Initialize the Intel High Definition Audio Codec(s) present in the system.
  For each codec, a predefined codec verb table should be programmed.
  The list contains 32-bit verbs to be sent to the corresponding codec.
  If it is not programmed, the codec uses the default verb table, which may or may not
  correspond to the platform jack information.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar address of this PCH device
  @param[in] AzaliaStarted        Whether Azalia is successfully started

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_INVALID_PARAMETER   Provided VerbTableData is null

**/
{
  EFI_STATUS                    Status;
  UINT32                        Index;
  UINT32                        VendorDeviceId;
  UINT32                        RevisionId;
  UINT8                         ByteReg;
  UINTN                         AzaliaBase;
  UINT8                         AzaliaSDINo;
  UINT32                        HdaBar;
  UINT32                        *VerbTable;
  UINT32                        LoopTime;
  PCH_AZALIA_VERB_TABLE_HEADER  *VerbHeaderTable;
  EFI_PHYSICAL_ADDRESS          BaseAddressBarMem;
  UINT8                         VerbTableNum;
  PCH_AZALIA_CONFIG             *AzaliaConfig;
  UINT32                        Data32And;
  UINT32                        Data32Or;
  UINT32                        CodecCmdData;
//  UINTN                         PciD31F0RegBase;
//  UINT16                        LpcDeviceId;
  UINT16                        Data16;
  UINT16                        Data16And;
  UINT16                        Data16Or;
  UINT16                        BitMask;
  UINT16                        BitValue;

  AzaliaConfig = PchPlatformPolicy->AzaliaConfig;
  AzaliaBase = MmPciAddress (0,
                 PchPlatformPolicy->BusNumber,
                 PCI_DEVICE_NUMBER_PCH_AZALIA,
                 PCI_FUNCTION_NUMBER_PCH_AZALIA,
                 0
                 );

  ///
  /// VLV BIOS Spec Rev x.x Section x.x.x High Definition Audio VC1 Configuration
  ///
  /// Step 1
  /// Set the VT to VC mapping for VC0
  /// RCBA + 0x24 (HDA V0CTL - HDAudio Virtual Channel 0 Resource Control) = 0x80000019
  ///
  MmioWrite32 (RootComplexBar + R_PCH_RCRB_HDA_V0CTL, 0x80000019);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V0CTL),
    1,
    (VOID *) (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V0CTL)
    );

  ///
  /// Step 2
  /// Assign a TC ID for VC1
  /// RCBA + 0x28 (HDA V1CTL - HDAudio Virtual Channel 1 Resource Control) = 0x81000022
  ///
  MmioWrite32 (RootComplexBar + R_PCH_RCRB_HDA_V1CTL,  0x81000022);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V1CTL),
    1,
    (VOID *) (UINTN) (RootComplexBar + R_PCH_RCRB_HDA_V1CTL)
    );

  if (AzaliaConfig->AzaliaVCi == TRUE) {
    //
    // Step 3
    // Set VCi Enable bit (VCIEN) of VCi Resource Control register
    // D27:F0:Reg 0x120 [31]
    //
    MmioWrite32 (AzaliaBase + R_PCH_HDA_VCICTL,  0x81000022);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL)
      );

    ///
    /// Step 4
    /// Enable No Snoop traffic
    /// D27:F0:Reg 0x78[11] (DEVC.NSNPEN - Enable No Snoop) = 1
    ///
    Data16And = (UINT16) (~B_PCH_HDA_DEVC_NSNPEN);
    Data16Or  = (UINT16) (B_PCH_HDA_DEVC_NSNPEN);
    MmioAndThenOr16 (
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      Data16And,
      Data16Or
      );
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      &Data16Or,  // Data to be ORed
      &Data16And  // Data to be ANDed
      );

  } else {
    ///
    /// Step 3
    /// Clear VCi Enable bit (VCIEN) of VCi Resource Control register
    /// D27:F0:Reg 0x120 (VCi Resource Control) = 0x1000022
    ///
    MmioWrite32 (AzaliaBase + R_PCH_HDA_VCICTL,  0x1000022);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_VCICTL)
      );

    ///
    /// Step 4
    /// Disable No Snoop traffic
    /// D27:F0:Reg 0x78[11] (DEVC.NSNPEN - Enable No Snoop) = 0
    ///
    Data16And = (UINT16) (~B_PCH_HDA_DEVC_NSNPEN);
    Data16Or  = (UINT16) 0x00;
    MmioAndThenOr16 (
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      Data16And,
      Data16Or
      );
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVC),
      &Data16Or,  // Data to be ORed
      &Data16And  // Data to be ANDed
      );
  }

  ///
  /// PCH BIOS Spec Rev 0.9.0 Section 9.5
  /// HDMI Codec Enabling
  /// System BIOS is required to perform the steps listed
  /// below in order to detect Codec on PCH Platform HDMI channel
  /// 1. Set D27:F0:C4h[1] = 1b
  /// 2. Set D27:F0:43h[6] = 1b (needs to be restored during S3)
  ///
  if (AzaliaConfig->HdmiCodec == PCH_DEVICE_ENABLE) {
    MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM2), (UINT32) B_PCH_HDA_SEM2_IACE);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_SEM2),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_SEM2)
      );
    MmioOr8 ((UINTN) (AzaliaBase + R_PCH_HDA_TM1), (UINT8) (B_PCH_HDA_TM1_ACCD | B_PCH_HDA_TM1_HAPD));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (AzaliaBase + R_PCH_HDA_TM1),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_TM1)
      );
    ///
    /// Clock gating WA
    ///
    MsgBus32Write(VLV_PUNIT, 0x60, 0xc0);
    MsgBus32Write(VLV_PUNIT, 0x60, 0x0);
    S3BootScriptMsgBus32Write (VLV_PUNIT, 0x60, 0xc0);
    S3BootScriptMsgBus32Write (VLV_PUNIT, 0x60, 0x0);
  } else {
    MmioOr8 ((UINTN) (AzaliaBase + R_PCH_HDA_TM1), (UINT8) (B_PCH_HDA_TM1_HAPD));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (AzaliaBase + R_PCH_HDA_TM1),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_TM1)
      );
  }
  ///
  /// Firstly Initialize Azalia to be not started.
  ///
  *AzaliaStarted = FALSE;

  ///
  /// Allocate resource for HDBAR
  ///
  BaseAddressBarMem = 0x0FFFFFFFF;
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  14,
                  V_PCH_HDA_HDBAR_SIZE,
                  &BaseAddressBarMem,
                  mImageHandle,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// System BIOS should ensure that the High Definition Audio HDBAR D27:F0:Reg 10-17h contains a valid address value
  /// and is enabled by setting D27:F0:Reg 04h[1].
  ///
  HdaBar = (UINT32) BaseAddressBarMem;
  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARL, HdaBar);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL)
    );

  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARU, 0);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU)
    );

  MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_STSCMD), (UINT32) B_PCH_HDA_STSCMD_MSE);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD)
    );

  ///
  /// PCH BIOS Spec Rev 0.5.0 Section 9.6
  /// Additional High Definition Audio Programming Steps
  /// BIOS is required to perform the additional steps listed below for PCH Platform.
  /// Step 1 and 2
  /// Set D27:F0:C4h[13] (B_PCH_HDA_SEM2_LSE) = 1b; not needed as VLV does not have DMI, but should be no harm
  /// Set D27:F0:C4h[10] (B_PCH_HDA_SEM2_MQRDAD) = 1b
  /// SEM2.BUFFER_SIZE_MINIMUM_THRESHOLD, B0:D:F0:Reg0xC4[27:26] = 11b
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = B_PCH_HDA_SEM1_CP;
  MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM1), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_SEM1),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  Data32And = 0xFFFFFFFF;
  Data32Or  = B_PCH_HDA_SEM2_LSE | B_PCH_HDA_SEM2_MQRDAD | B_PCH_HDA_SEM2_BSMT;
  MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM2), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_SEM2),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// SEM3_low.INPUT_STREAM_REQUEST_WATERMARK_SELECT, B0:D:F0:Reg0xC8[12:7] = 0x00
  ///
  MmioWrite32 ((UINTN) (AzaliaBase + R_PCH_HDA_SEM3_LOW), 0x82a30000);

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_SEM3_LOW),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_SEM3_LOW)
    );

  ///
  /// PCH BIOS Spec Rev 0.7.0 Section 9.6
  /// Additional High Definition Audio Programming Steps
  /// Step 3
  /// Set D27:F0:D0h[31] = 0b
  /// This section not needed as VLV does not have DMI, but probably no harm
  ///
  Data32And = ~BIT31;
  Data32Or  = (UINT32) 0x0;
  MmioAnd32 ((UINTN) (AzaliaBase + 0xD0), Data32And);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + 0xD0),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  if (AzaliaConfig->DS == PCH_DEVICE_DISABLE) {
    MmioAnd8 ((UINTN) (AzaliaBase + R_PCH_HDA_DCKSTS), (UINT8) (~B_PCH_HDA_DCKSTS_DS));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (AzaliaBase + R_PCH_HDA_DCKSTS),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_DCKSTS)
      );
  } else if (AzaliaConfig->DA != PCH_DEVICE_DISABLE) {
    if ((MmioRead8 (AzaliaBase + R_PCH_HDA_DCKSTS) & B_PCH_HDA_DCKSTS_DM) == 0) {
      MmioOr8 ((UINTN) (AzaliaBase + R_PCH_HDA_DCKCTL), (UINT8) (B_PCH_HDA_DCKCTL_DA));
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (AzaliaBase + R_PCH_HDA_DCKCTL),
        1,
        (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_DCKCTL)
        );
    }
  }
  ///
  /// PCH BIOS Spec Rev 1.0.0 Section 9.1.3 Codec Initialization Programming Sequence
  /// System BIOS should also ensure that the Controller Reset# bit of Global Control register
  /// in memory-mapped space (HDBAR+08h[0]) is set to 1 and read back as 1.
  /// Deassert the HDA controller RESET# to start up the link
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = (UINT32) (B_HDA_GCTL_CRST);
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = (UINT16) B_HDA_GCTL_CRST;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  SetPollStatusS3Item (
    HdaBar + R_HDA_GCTL,
    (UINT32)BitMask,
    (UINT32)BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );
  ///
  /// PCH BIOS Spec Rev 1.1.0 Section 9.1.3 Codec Initialization Programming Sequence
  /// Read GCAP and write the same value back to the register once after Controller Reset# bit is set
  ///
  Data16  = MmioRead16 (HdaBar + R_HDA_GCAP);
  MmioWrite16 (HdaBar + R_HDA_GCAP, Data16);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (HdaBar + R_HDA_GCAP),
    1,
    (VOID *) (UINTN) (HdaBar + R_HDA_GCAP)
    );

  ///
  /// Clear the "State Change Status Register" STATESTS bits for
  /// each of the "SDIN Stat Change Status Flag"
  ///
  MmioOr8 ((UINTN) (HdaBar + R_HDA_STATESTS), (UINT8) (AZALIA_MAX_SID_MASK));
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint8,
    (UINTN) (HdaBar + R_HDA_STATESTS),
    1,
    (VOID *) (UINTN) (HdaBar + R_HDA_STATESTS)
    );

  ///
  /// Turn off the link and poll RESET# bit until it reads back as 0 to get hardware reset report
  ///
  Data32And = (UINT32) (~B_HDA_GCTL_CRST);
  Data32Or  = (UINT32) 0;
  MmioAnd32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32And);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = 0;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  SetPollStatusS3Item (
    HdaBar + R_HDA_GCTL,
    (UINT32)BitMask,
    (UINT32)BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Reset High Definition Audio (Azalia) Codec Time Out - 1! \n"));
    goto ExitInitAzalia;
  }
  ///
  /// Turn on the link and poll RESET# bit until it reads back as 1
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = (UINT32) (B_HDA_GCTL_CRST);
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );
  ///
  /// For some combo card that will need this delay because each codec has different latency to come out from RESET.
  /// This delay can make sure all codecs be recognized by BIOS after RESET sequence.
  /// Additional delay might be required to allow codec coming out of reset prior to subsequent operations,
  /// please contact your codec vendor for detail. When clearing this bit and setting it afterward,
  /// BIOS must ensure that minimum link timing requirements (minimum RESET# assertion time, etc.) are met..
  ///
  PchPmTimerStall (AzaliaConfig->ResetWaitTimer);
  PmTimerStallS3Item (AzaliaConfig->ResetWaitTimer);

  BitMask = (UINT16) B_HDA_GCTL_CRST;
  BitValue = (UINT16) B_HDA_GCTL_CRST;
  Status = StatusPolling (HdaBar + R_HDA_GCTL, BitMask, BitValue);
  SetPollStatusS3Item (
    HdaBar + R_HDA_GCTL,
    (UINT32)BitMask,
    (UINT32)BitValue,
    AZALIA_WAIT_PERIOD,
    AZALIA_MAX_LOOP_TIME
    );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Reset High Definition Audio (Azalia) Codec Time Out - 2! \n"));
    goto ExitInitAzalia;
  }
  ///
  /// Read the "State Change Status Register" STATESTS bits twice to find out if any SDIN is connected
  /// to a codec.
  ///
  for (LoopTime = 0, ByteReg = 0, AzaliaSDINo = 0; LoopTime < AZALIA_MAX_LOOP_TIME; LoopTime++) {
    ByteReg = MmioRead8 (HdaBar + R_HDA_STATESTS) & AZALIA_MAX_SID_MASK;
    if (ByteReg != 0 && (ByteReg == AzaliaSDINo)) {
      break;
    } else {
      AzaliaSDINo = ByteReg;
    }

    PchPmTimerStall (AZALIA_WAIT_PERIOD);
  }
  ///
  /// BIT3(1000) -- SDI3
  /// BIT2(0100) -- SDI2
  /// BIT1(0010) -- SDI1
  /// BIT0(0001) -- SDI0
  ///
  if (ByteReg == 0) {
    ///
    /// No Azalia Detected
    ///
    ///
    /// Turn off the link
    ///
    DEBUG ((EFI_D_ERROR, "No Azalia device is detected.\n"));
    Data32And = (UINT32) (~B_HDA_GCTL_CRST);
    Data32Or  = (UINT32) 0;
    MmioAnd32 ((UINTN) (HdaBar + R_HDA_GCTL), Data32And);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (HdaBar + R_HDA_GCTL),
      &Data32Or,  /// Data to be ORed
      &Data32And  /// Data to be ANDed
      );
    Status = EFI_DEVICE_ERROR;
    goto ExitInitAzalia;
  }
  ///
  /// PME Enable for Audio controller, this bit is in the resume well
  ///
  if (AzaliaConfig->Pme == PCH_DEVICE_ENABLE) {
    MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_PCS), (UINT32) (B_PCH_HDA_PCS_PMEE));
  }

  for (AzaliaSDINo = 0; AzaliaSDINo < AZALIA_MAX_SID_NUMBER; AzaliaSDINo++, ByteReg >>= 1) {
    if ((ByteReg & 0x1) == 0) {
      ///
      /// SDIx has no Azalia Device
      ///
      DEBUG ((EFI_D_ERROR, "SDI%d has no Azalia device.\n", AzaliaSDINo));
      continue;
    }
    ///
    /// PME Enable for each existing codec, these bits are in the resume well
    ///
    if (AzaliaConfig->Pme != PCH_DEVICE_DISABLE) {
      MmioOr16 (
        (UINTN) (HdaBar + R_HDA_WAKEEN),
        (UINT16) ((B_HDA_WAKEEN_SDI_0 << AzaliaSDINo))
        );
    }
    ///
    /// Verb:  31~28   27  26~20                   19~0
    ///         CAd    1    NID   Verb Command and data
    ///       0/1/2
    ///
    /// Read the Vendor ID/Device ID pair from the attached codec
    ///
    VendorDeviceId  = 0x000F0000 | (AzaliaSDINo << 28);
    Status          = SendCodecCommand (HdaBar, &VendorDeviceId, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Read the Codec Vendor ID/Device ID fail! \n"));
      goto ExitInitAzalia;
    }
    ///
    /// Read the Revision ID from the attached codec
    ///
    RevisionId  = 0x000F0002 | (AzaliaSDINo << 28);
    Status      = SendCodecCommand (HdaBar, &RevisionId, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Read the Codec Revision ID fail! \n"));
      goto ExitInitAzalia;
    }

    RevisionId = (RevisionId >> 8) & 0xFF;

    ///
    /// Get the match codec verb table, RevID of 0xFF applies to all steppings.
    ///
    for (VerbTableNum = 0, VerbHeaderTable = NULL, VerbTable = NULL;
         VerbTableNum < AzaliaConfig->AzaliaVerbTableNum;
         VerbTableNum++) {
      if ((VendorDeviceId == AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader.VendorDeviceId) &&
          ((AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader.RevisionId == 0xFF) ||
          ( RevisionId == AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader.RevisionId))) {
        VerbHeaderTable = &(AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableHeader);
        VerbTable       = AzaliaConfig->AzaliaVerbTable[VerbTableNum].VerbTableData;
        if (VerbTable == 0) {
          DEBUG ((EFI_D_ERROR | EFI_D_INFO, "VerbTableData of VendorID:0x%X is null.\n", VendorDeviceId));
          Status = EFI_INVALID_PARAMETER;
          goto ExitInitAzalia;
        }
        DEBUG ((EFI_D_INFO, "Detected Azalia Codec with verb table, VendorID = 0x%X", VendorDeviceId));
        DEBUG ((EFI_D_INFO, " on SDI%d, revision = 0x%0x.\n", AzaliaSDINo, RevisionId));
        ///
        /// Send the entire list of verbs in the matching verb table one by one to the codec
        ///
        for (Index = 0;
             Index < (UINT32) ((VerbHeaderTable->NumberOfFrontJacks + VerbHeaderTable->NumberOfRearJacks) * 4);
             Index++) {
          ///
          /// Clear CAd Field
          ///
          CodecCmdData  = VerbTable[Index] & (UINT32) ~(BIT31 | BIT30 | BIT29 | BIT28);
          ///
          /// Program CAd Field per the SDI number got during codec detection
          ///
          CodecCmdData  |= (UINT32) (AzaliaSDINo << 28);
          Status        = SendCodecCommand (HdaBar, &CodecCmdData, FALSE);
          if (EFI_ERROR (Status)) {
            ///
            /// Skip the Azalia verb table loading when find the verb table content is not
            /// properly matched with the HDA hardware, though IDs match.
            ///
            DEBUG (
              (EFI_D_ERROR | EFI_D_INFO,
              "Detected Azalia Codec of VendorID:0x%X, error occurs during loading verb table.\n",
              VendorDeviceId)
              );
            goto ExitInitAzalia;
          }
          SendCodecCommandS3Item (HdaBar, CodecCmdData);
        }
        break;
      }
    }

    if (VerbTableNum >= AzaliaConfig->AzaliaVerbTableNum) {
      DEBUG (
        (EFI_D_ERROR,
        "Detected High Definition Audio (Azalia) Codec, VendorID = 0x%08x on SDI%d,",
        VendorDeviceId,
        AzaliaSDINo)
        );
      DEBUG ((EFI_D_ERROR, " but no matching verb table found.\n"));
    }
  }
  ///
  /// end of for
  ///
  *AzaliaStarted  = TRUE;
  Status          = EFI_SUCCESS;

ExitInitAzalia:
  ///
  /// Clear AZBAR and disable memory map access
  ///
  MmioAnd32 ((UINTN) (AzaliaBase + R_PCH_HDA_STSCMD), (UINT32) (~B_PCH_HDA_STSCMD_MSE));
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_STSCMD)
    );

  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARL, 0);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARL)
    );

  MmioWrite32 (AzaliaBase + R_PCH_HDA_HDBARU, 0);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_HDBARU)
    );

  gDS->FreeMemorySpace (
        BaseAddressBarMem,
        V_PCH_HDA_HDBAR_SIZE
        );

  return Status;
}

EFI_STATUS
ConfigureAzalia (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  BOOLEAN                             *AzaliaEnable
  )
/**

  @brief
  Detect and initialize the type of codec (AC'97 and HDA) present in the system.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] AzaliaEnable         Returned with TRUE if Azalia High Definition Audio codec
                                  is detected and initialized.

  @retval EFI_SUCCESS            Codec is detected and initialized.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resources to initialize the codec.

**/
{
  EFI_STATUS  Status;
  UINTN       AzaliaBase;
  PCH_STEPPING stepping;

  DEBUG ((EFI_D_INFO, "ConfigureAzalia() Start\n"));

  *AzaliaEnable = FALSE;
  AzaliaBase    = MmPciAddress (0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_AZALIA,
                    PCI_FUNCTION_NUMBER_PCH_AZALIA,
                    0
                    );

  stepping = PchStepping();
  if (stepping >= PchC0) {
    ///
    /// If all codec devices are to be disabled, skip the detection code
    ///
    if (PchPlatformPolicy->DeviceEnabling->Azalia == PCH_DEVICE_DISABLE) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Skip Azalia Codec detection.\n"));
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting Azalia Controller into D3 Hot State.\n"));
      MmioOr32 ((UINTN) (AzaliaBase + R_PCH_HDA_PCS), V_PCH_HDA_PCS_PS3);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (AzaliaBase + R_PCH_HDA_PCS),
        1,
        (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_PCS)
        );
      return EFI_SUCCESS;
    }
  }
  Status = DetectAndInitializeAzalia (PchPlatformPolicy, RootComplexBar, AzaliaEnable);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Azalia detection / initialization failure!\n"));

    if (PchPlatformPolicy->DeviceEnabling->Azalia == PCH_DEVICE_ENABLE) {
      *AzaliaEnable = TRUE;
    }
  }

  if (stepping < PchC0) {
    if (PchPlatformPolicy->DeviceEnabling->Azalia == PCH_DEVICE_DISABLE) {
      *AzaliaEnable = TRUE;
    }
  }
  DEBUG ((EFI_D_INFO, "ConfigureAzalia() End\n"));
  return EFI_SUCCESS;
}
