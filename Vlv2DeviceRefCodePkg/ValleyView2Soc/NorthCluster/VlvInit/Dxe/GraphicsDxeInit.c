/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  GraphicsDxeInit.c

Abstract:

  DXE driver for Initializing SystemAgent Graphics initialization.

--*/

#ifdef ECP_FLAG
#include "VlvInit.h"
#define S3BootScriptWidthUint32 EfiBootScriptWidthUint32
#define S3BootScriptWidthUint8  EfiBootScriptWidthUint8
#else
#include "GraphicsDxeInit.h"
#include  <Guid/DxeServices.h>
#include <Library/UefiLib.h>
#endif
#include <PchAccess.h>
#include <Library/PchPlatformLib.h>

// AMI_OVERRIDE START - EIP194648
#include <SetupDataDefinition.h>

extern EFI_RUNTIME_SERVICES *gRT;
extern EFI_GUID gEfiSetupVariableGuid;
// AMI_OVERRIDE END

UINT64            GTTMMADR;
UINTN             MCHBAR_BASE;

DXE_VLV_PLATFORM_POLICY_PROTOCOL  *DxePlatformSaPolicy;

VOID
PollGtReady_hang (
  UINT64 Base,
  UINT32 Offset,
  UINT32 Mask,
  UINT32 Result
  )
/*++

Routine Description:

  "Poll Status" for GT Readiness

Arguments:

  Base            - Base address of MMIO
  Offset          - MMIO Offset
  Mask            - Mask
  Result          - Value to wait for

Returns:

  None

--*/
{
  UINT32  GtStatus;
  //UINT16  StallCount;

  //StallCount = 0;

  //
  // Register read
  //
  GtStatus = Mmio32 (Base, Offset);

  while (((GtStatus & Mask) != Result)) {

    GtStatus = Mmio32 (Base, Offset);
    //
    // 1mSec wait
    //
    //gBS->Stall (1000);
    //StallCount = StallCount + 1;
  }

  //ASSERT ((StallCount != GT_WAIT_TIMEOUT));
}


VOID
PollPowerGate (
  UINT32 Mask,
  UINT32 Result
  )
/*++

Routine Description:

  "Poll Status" for GT Readiness

Arguments:

  Base            - Base address of MMIO
  Offset          - MMIO Offset
  Mask            - Mask
  Result          - Value to wait for

Returns:

  None

--*/
{
  UINT32  Data32;

  MsgBus32Read( VLV_PUNIT,PUNIT_PWRGT_STATUS,Data32);


  while (((Data32 & Mask) != Result)) {

    MsgBus32Read( VLV_PUNIT,PUNIT_PWRGT_STATUS,Data32);
    //
    // 0.1mSec wait
    //
    gBS->Stall (100);

  }
}

//
// GT Wait Timeout of ~3 sec
//
#define GT_WAIT_TIMEOUT 3000

BOOT_SCRIPT_REGISTER_SETTING    gSaPowerMeterRegisters[] = {
  // Weights SET1
  0x0, 0xA800, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA804, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA808, 0xFFFFFFFF, 0x0000ff0A,
  0x0, 0xA80C, 0xFFFFFFFF, 0x1D000000,
  0x0, 0xA810, 0xFFFFFFFF, 0xAC004900,
  0x0, 0xA814, 0xFFFFFFFF, 0x000F0000,
  0x0, 0xA818, 0xFFFFFFFF, 0x5A000000,
  0x0, 0xA81C, 0xFFFFFFFF, 0x2600001F,
  0x0, 0xA820, 0xFFFFFFFF, 0x00090000,
  0x0, 0xA824, 0xFFFFFFFF, 0x2000ff00,
  0x0, 0xA828, 0xFFFFFFFF, 0xff090016,
  0x0, 0xA82C, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA830, 0xFFFFFFFF, 0x00000100,
  0x0, 0xA834, 0xFFFFFFFF, 0x00A00F51,
  0x0, 0xA838, 0xFFFFFFFF, 0x000B0000,
  0x0, 0xA83C, 0xFFFFFFFF, 0xcb7D3307,
  0x0, 0xA840, 0xFFFFFFFF, 0x003C0000,
  0x0, 0xA844, 0xFFFFFFFF, 0xFFFF0000,
  0x0, 0xA848, 0xFFFFFFFF, 0x00220000,
  0x0, 0xA84c, 0xFFFFFFFF, 0x43000000,
  0x0, 0xA850, 0xFFFFFFFF, 0x00000800,
  0x0, 0xA854, 0xFFFFFFFF, 0x00000F00,
  0x0, 0xA858, 0xFFFFFFFF, 0x00000021,
  0x0, 0xA85c, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA860, 0xFFFFFFFF, 0x00190000,
  0x0, 0xAA80, 0xFFFFFFFF, 0x00FF00FF,
  0x0, 0xAA84, 0xFFFFFFFF, 0x00000000,
  0x0, 0x1300A4, 0xFFFFFFFF, 0x00000000,

  //Weights SET2
  0x0, 0xA900, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA904, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA908, 0xFFFFFFFF, 0x00000000,
  0x0, 0xa90c, 0xFFFFFFFF, 0x1D000000,
  0x0, 0xa910, 0xFFFFFFFF, 0xAC005000,
  0x0, 0xa914, 0xFFFFFFFF, 0x000F0000,
  0x0, 0xa918, 0xFFFFFFFF, 0x5A000000,
  0x0, 0xa91c, 0xFFFFFFFF, 0x2600001F,
  0x0, 0xa920, 0xFFFFFFFF, 0x00090000,
  0x0, 0xa924, 0xFFFFFFFF, 0x2000ff00,
  0x0, 0xa928, 0xFFFFFFFF, 0xff090016,
  0x0, 0xa92c, 0xFFFFFFFF, 0x00000000,
  0x0, 0xa930, 0xFFFFFFFF, 0x00000100,
  0x0, 0xa934, 0xFFFFFFFF, 0x00A00F51,
  0x0, 0xa938, 0xFFFFFFFF, 0x000B0000,
  0x0, 0xA93C, 0xFFFFFFFF, 0xcb7D3307,
  0x0, 0xA940, 0xFFFFFFFF, 0x003C0000,
  0x0, 0xA944, 0xFFFFFFFF, 0xFFFF0000,
  0x0, 0xA948, 0xFFFFFFFF, 0x00220000,
  0x0, 0xA94C, 0xFFFFFFFF, 0x43000000,
  0x0, 0xA950, 0xFFFFFFFF, 0x00000800,
  0x0, 0xA954, 0xFFFFFFFF, 0x00000000,
  0x0, 0xA960, 0xFFFFFFFF, 0x00000000,

  //Weights SET3
  0x0, 0xaa3c, 0xFFFFFFFF, 0x00000000,
  0x0, 0xaa54, 0xFFFFFFFF, 0x00000000,
  0x0, 0xaa60, 0xFFFFFFFF, 0x00000000,

  // Enable pwrmtr counters
  0x0, 0xA248, 0xFFFFFFFF, 0x00000058,

};

//
// RC6 Settings
//
BOOT_SCRIPT_REGISTER_SETTING    gSaGtRC6Registers[] = {
  0x0, 0xA090,   0xFFFFFFFF, 0x0,
  //
  // RC1e - RC6/6p - RC6pp Wake Rate Limits
  //
  0x0, 0xA09C,   0xFFFFFFFF, 0x280000,
  0x0, 0xA0A8,   0xFFFFFFFF, 0x1E848,
  0x0, 0xA0AC,   0xFFFFFFFF, 0x19,

  //
  // RC Sleep / RCx Thresholds
  //
  0x0, 0xA0B0,   0xFFFFFFFF, 0,
  0x0, 0xA0B8,   0xFFFFFFFF, 0x557
};

//
// Turbo Settings
//
BOOT_SCRIPT_REGISTER_SETTING    gSaGtTruboRegisters[] = {
  //
  // Render/Video/Blitter Idle Max Count
  //
  0x0, 0x2054,    0xFFFFFFFF, 0xA,
  0x0, 0x12054,   0xFFFFFFFF, 0xA,
  0x0, 0x22054,   0xFFFFFFFF, 0xA,

  //
  // RP Down Timeout.
  //
  0x0, 0xA010,   0xFFFFFFFF, 0xF4240
};

BOOT_SCRIPT_REGISTER_SETTING    gSaGtTruboCTLregisters[] = {
  //
  // RP Up/Dwon Threshold
  //
  0x0, 0xA02C,   0xFFFFFFFF, 0xE8E8,
  0x0, 0xA030,   0xFFFFFFFF, 0x3BD08,
  //
  // RP Up/Dwon EI
  //
  0x0, 0xA068,   0xFFFFFFFF, 0x101D0,
  0x0, 0xA06C,   0xFFFFFFFF, 0x55730
};

//
// PAVC STRUCTURE
//
typedef struct {
  UINT8 GraphicsFreqReq;
  UINT8 PmLock;
  UINT8 PavpMode;
  UINT8 UnsolicitedAttackOverride;
  UINT8 ScanDumpMarkers;
} VLV_GT_POST_STRUCT;

VLV_GT_POST_STRUCT              GtPostStruct;

UINT16 GGCToGMSSizeMapping[17] = { 0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512};

EFI_STATUS
PmInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
/*++

Routine Description:

  Initialize GT PowerManagement of SystemAgent.

Arguments:

  ImageHandle         - Handle for the image of this driver
  DxePlatformSaPolicy - SA DxePlatformPolicy protocol

Returns:

  EFI_SUCCESS       - GT Power Management initialization complete

--*/
{
  UINT8                         i;
  UINT32                        Data32And;
  UINT32                        Data32Or;

  UINT32                        Data32;
  UINT32                        RegOffset;
  UINT32                        Data32Mask;
  UINT32                        Result;
  UINT32                        PcBase;
  UINT32                        uGCC;
  UINT32                        GMSsize;
  UINT16                        gSaGtPowerMeterRegistersSize;
  BOOT_SCRIPT_REGISTER_SETTING  *gSaGtPowerMeterRegisters;
  UINT32                        IgdDeviceID;
  //
  // PmInit Initialization
  //
  DEBUG ((EFI_D_ERROR, "Initializing GT PowerManagement\n"));

  IgdDeviceID = (McD2PciCfg32(0x0))>> 16;

  //
  // Allows the render and media wells to wake.  Used by Punit as part of s0ix save/restore.
  // GUnit Register 0x130090[0]:  0: ForceAwake Disable
  //                              1: ForceAwake Enable
  Data32Or                        = 0x00000001;
  Data32And                       = 0xfffffffe;
  Mmio32AndThenOr (GTTMMADR, 0x130090, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite(
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + 0x130090),
    &Data32Or,
    &Data32And
    );
  //
  // Confirm that bit0=1 so that the allow-wake has taken effect.
  //
  Data32Mask  = 0x1;
  Result      = 0x1;

  DEBUG ((EFI_D_ERROR, "Polling allow-wake bit\n"));
  PollGtReady_hang (GTTMMADR, 0x130094, Data32Mask, Result);

  S3BootScriptSaveMemPoll(
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + 0x130094),
    &Data32Mask,
    &Result,
    50,
    200000
    );

  if(DxePlatformSaPolicy->ForceWake == 1){
    //
    // Render Force Wake
    //
    RegOffset                     = 0x1300B0;
    Data32                        = 0x80008000;
    Mmio32 (GTTMMADR, RegOffset)  = Data32;

    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32
      );

    //
    // Render Force Wake Acknowledge Bit
    //
    Data32Mask  = 0x8000;
    Result      = 0x8000;

    DEBUG ((EFI_D_ERROR, "Polling Render Force Wake Acknowledge Bit\n"));
    PollGtReady_hang(GTTMMADR, 0x1300B4, Data32Mask, Result);
    S3BootScriptSaveMemPoll(
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + 0x1300B4),
      &Data32Mask,
      &Result,
      50,
      200000
      );

    //
    // Media ForceWakeReq
    //
    RegOffset                     = 0x1300B8;
    Data32                        = 0x80008000;
    Mmio32 (GTTMMADR, RegOffset)  = Data32;
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32
      );

    //
    // Media ForceWakeReq Acknowledge Bit
    //
    Data32Mask  = 0x8000;
    Result      = 0x8000;

    DEBUG ((EFI_D_ERROR, "Polling Media ForceWakeReq Acknowledge Bit\n"));
    PollGtReady_hang (GTTMMADR, 0x1300BC, Data32Mask, Result);
    S3BootScriptSaveMemPoll(
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + 0x1300BC),
      &Data32Mask,
      &Result,
      50,
      200000
      );
  }


  //
  // W/A - X0:261954/A0:261955
  //
  Data32Or                        = 0x1;
  Data32And                       = 0xfffffff0;
  Mmio32AndThenOr (GTTMMADR, 0x182060, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite(
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + 0x182060),
    &Data32Or,
    &Data32And
  );

  if(DxePlatformSaPolicy->PmWeights == 1) {
    gSaGtPowerMeterRegisters      = gSaPowerMeterRegisters;
    gSaGtPowerMeterRegistersSize  = sizeof (gSaPowerMeterRegisters);
    //
    // GT Power Meter Weights & Control Registers programming
    //
    for (i = 0; i < (gSaGtPowerMeterRegistersSize / sizeof (BOOT_SCRIPT_REGISTER_SETTING)); ++i) {
      RegOffset                     = gSaGtPowerMeterRegisters[i].Offset;
      Data32And                     = gSaGtPowerMeterRegisters[i].AndMask;
      Data32Or                      = gSaGtPowerMeterRegisters[i].OrMask;
      Mmio32 (GTTMMADR, RegOffset)  = Data32Or;
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (GTTMMADR + RegOffset),
        1,
        &Data32Or
        );
    }
  }
  //
  //Valleyview s4683544, During debug of GFX power meter issue, BIOS needs to program the PUNIT_GPU_EC_VIRUS.
  //Without this, power meter is returning all FF.  need to be initialized in BIOS as part of GFX Turbo enable.
  //
  // GFX Weights - Scaling Factor
  //

  if (DxePlatformSaPolicy->DptfSettings.SdpProfile == 4) {
    MsgBus32Write(VLV_PUNIT, PUNIT_GPU_EC_VIRUS, 0x11940);  //Value provided by architecture.
  } else {
    MsgBus32Write(VLV_PUNIT, PUNIT_GPU_EC_VIRUS, 0xcf08);  //Value provided by architecture.
  }

  //
  // GfxPause
  //
  RegOffset                     = 0xa000;
  Data32                        = 0x00071388;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );

  if (DxePlatformSaPolicy->EuControl) {
    //
    // Dynamic EU Control Settings
    //
    Data32                    = 0x4;
    Mmio32 (GTTMMADR, 0xA080) = Data32;
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + 0xA080),
      1,
      &Data32
      );
  }

  //
  // Lock Eco Bit Settings
  //
  if (DxePlatformSaPolicy->PmLock) {
    RegOffset                     = 0xa180;
    Data32                        = 0x80000000;
    Mmio32 (GTTMMADR, RegOffset)  = Data32;
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32
    );
  }

  //
  // DOP Clock Gating
  //
  if (DxePlatformSaPolicy->DopClockGating) {
    RegOffset                     = 0x9424;
    Data32                        = 0x1;
    Mmio32 (GTTMMADR, RegOffset)  = Data32;
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32
      );
  }

  //
  //  MBCunit will send the VCR ( Fuse) writes as NP-W
  //
  Data32Or                        = 0x10000;
  Data32And                       = 0xfffeffff;
  Mmio32AndThenOr (GTTMMADR, 0x907c, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite(
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + 0x907c),
    &Data32Or,
    &Data32And
    );

  //
  // RC6 Settings
  //
  if(DxePlatformSaPolicy->EnableRenderStandby) {
    for (i = 0; i < sizeof (gSaGtRC6Registers) / sizeof (BOOT_SCRIPT_REGISTER_SETTING); ++i) {
      RegOffset                     = gSaGtRC6Registers[i].Offset;
      Data32And                     = gSaGtRC6Registers[i].AndMask;
      Data32Or                      = gSaGtRC6Registers[i].OrMask;
      Mmio32 (GTTMMADR, RegOffset)  = Data32Or;
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (GTTMMADR + RegOffset),
        1,
        &Data32Or
        );
    }

  }

  //
  // Turbo Settings
  //
  for (i = 0; i < sizeof (gSaGtTruboRegisters) / sizeof (BOOT_SCRIPT_REGISTER_SETTING); ++i) {
    RegOffset                     = gSaGtTruboRegisters[i].Offset;
    Data32And                     = gSaGtTruboRegisters[i].AndMask;
    Data32Or                      = gSaGtTruboRegisters[i].OrMask;
    Mmio32 (GTTMMADR, RegOffset)  = Data32Or;
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32Or
      );
  }
  //
  // These registers should only be programmed by BIOS if Gfx Turbo is enabled
  //
  if(DxePlatformSaPolicy->EnableIGDTurbo) {
    for (i = 0; i < sizeof (gSaGtTruboCTLregisters) / sizeof (BOOT_SCRIPT_REGISTER_SETTING); ++i) {
      RegOffset                     = gSaGtTruboCTLregisters[i].Offset;
      Data32And                     = gSaGtTruboCTLregisters[i].AndMask;
      Data32Or                      = gSaGtTruboCTLregisters[i].OrMask;
      Mmio32 (GTTMMADR, RegOffset)  = Data32Or;
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (GTTMMADR + RegOffset),
        1,
        &Data32Or
        );
    }
  }

  //
  // RP Idle Hysteresis
  //
  Data32                    = 0xA;
  Mmio32 (GTTMMADR, 0xA070) = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
//    (UINTN) (GTTMMADR + 0xA024),
    (UINTN) (GTTMMADR + 0xA070),	// AMI_OVERRIDE
    1,
    &Data32
    );

  if(DxePlatformSaPolicy->EnableRenderStandby) {
    //
    // HW RC6 Control Settings, For B0/B1 stepping, write 0x00000000 to disable HW RC6.
    //
    if((PchStepping() == PchB0) || (PchStepping() == PchB1)) {
      Data32 = 0x00000000;
    } else {
      Data32 = 0x11000000;
    }

    Mmio32 (GTTMMADR, 0xA090) = Data32;
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + 0xA090),
      1,
      &Data32
      );
  }
  //
  // RP Control
  //
  Data32                    = 0x592;
  Mmio32 (GTTMMADR, 0xA024) = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + 0xA024),
    1,
    &Data32
    );

  //
  // Enable PM Interrupts
  //
  RegOffset                     = 0x44024;
  Data32                        = 0x03000000;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );

  RegOffset                     = 0x4402C;
  Data32                        = 0x03000076;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );
  //
  // Changed to disable all interrupts. Driver will enable what it needs.
  //
  RegOffset                     = 0xA168;
  Data32                        = 0x7E;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );


  //
  // Programs  and lock the Power Context Base Register (PCBR 18_2120h) to point
  // to a 24KB block of memory in graphics stolen data memory.  The
  // power context save data is stored in this area.
  //

  //
  // Get the base of Protected Content Memory: BDSMbase + GMS size - WOPCMSZ - PowerContextSize
  //
  uGCC = McD2PciCfg32(0x50);
  GMSsize = GGCToGMSSizeMapping[(uGCC & (BIT7|BIT6|BIT5|BIT4|BIT3))>>3];
  PcBase = (McD2PciCfg32 (0x5C) & 0xFFF00000) + (GMSsize-1) * 0x100000 - 0x6000;
  DEBUG ((EFI_D_ERROR, "PcBase = 0x%x\n", PcBase));

  RegOffset                     = 0x182120;
  Data32                        = PcBase | LockBit;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );

  //
  //Part of s4753494 to be more aggressive on clock gating.
  //
  RegOffset                     = 0x9400;
  Data32                        = 0x0;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );

  RegOffset                     = 0x9404;
  Data32                        = 0x0;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );

  RegOffset                     = 0x9408;
  Data32                        = 0x0;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );

  RegOffset                     = 0x940c;
  Data32                        = 0x0;
  Mmio32 (GTTMMADR, RegOffset)  = Data32;
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (GTTMMADR + RegOffset),
    1,
    &Data32
    );


  return EFI_SUCCESS;
}



EFI_STATUS
PavpInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
/*++

Routine Description:

  Initialize PAVP feature of SystemAgent.

Arguments:

  ImageHandle         - Handle for the image of this driver
  DxePlatformSaPolicy - SA DxePlatformPolicy protocol

Returns:

  EFI_SUCCESS - PAVP initialization complete

--*/
{

  UINT32                DwordData;
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  BaseAddress;

  //
  // PAVP Initialization
  //
  DEBUG ((EFI_D_ERROR, "Initializing PAVP\n"));

  McD2PciCfg32And (PAVPC, ~(PAVPC_HVYMODSEL_MASK | PAVPC_PCMBASE_MASK | PAVPC_PAVPE_MASK | PAVPC_PCME_MASK));
  McD2PciCfg16Or (PAVPC, PAVPC_PCME_MASK | PAVPC_PAVPE_MASK);
  
  if (GtPostStruct.PavpMode == PAVP_SERPENT_MODE) {
    McD2PciCfg16AndThenOr (PAVPC, ~(PAVPC_HVYMODSEL_MASK), 1 << PAVPC_HVYMODSEL_OFFSET);
    
    BaseAddress = 0x1f000000;
    Status = gBS->AllocatePages (
                    AllocateAddress,
                    EfiReservedMemoryType,
                    EFI_SIZE_TO_PAGES (0x1000000),
                    &BaseAddress
                    );
    ASSERT_EFI_ERROR (Status);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (GtPostStruct.PavpMode == PAVP_LITE_MODE) {
    BaseAddress = 0x1f000000;
    Status = gBS->AllocatePages (
                    AllocateAddress,
                    EfiReservedMemoryType,
                    EFI_SIZE_TO_PAGES (0x100000),
                    &BaseAddress
                    );
    ASSERT_EFI_ERROR (Status);
  }

  if (GtPostStruct.UnsolicitedAttackOverride) {
    McD2PciCfg32Or (PAVPC, (BIT4));
  } else {
    McD2PciCfg32And (PAVPC, ~(BIT4));
  }

  if (GtPostStruct.PmLock) {
    //
    // Lock PAVPC Register
    //
    McD2PciCfg16Or (PAVPC, PAVPC_PAVPLCK_MASK);

    DwordData = McD2PciCfg32 (PAVPC);
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (MmPciAddress (0,
      0,
      2,
      0,
      PAVPC)),
      1,
      &DwordData
      );
  }

  return EFI_SUCCESS;
}

VOID
PostPmInitCallBack (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
/*++

Routine Description:
  Do Post GT PM Init Steps after VBIOS Initialization.

Arguments:
  Event             A pointer to the Event that triggered the callback.
  Context           A pointer to private data registered with the callback function.

Returns:
  EFI_SUCCESS       - GC_TODO


--*/
{
  UINT32      RegOffset;
  UINT32      Data32;
  UINT32      Data32Mask;
  UINT32      Result;
  UINT64      OriginalGTTMMADR;
  UINT32      LoGTBaseAddress;
  UINT32      HiGTBaseAddress;
 // AMI_OVERRIDE - S3BootScriptSaveMemPoll function doesn・t work properly issue - CSP20130723_A >>
  UINT8       ByteData;
  UINT8       CmdData;

  CmdData = McD2PciCfg8 (IGD_R_CMD);
  // AMI_OVERRIDE - S3BootScriptSaveMemPoll function doesn・t work properly issue - CSP20130723_A <<
  //
  // Enable Bus Master, I/O and Memory access on 0:2:0
  //
  McD2PciCfg8Or (IGD_R_CMD, (BIT2 | BIT1));
  // AMI_OVERRIDE - S3BootScriptSaveMemPoll function doesn・t work properly issue - CSP20130723_A >>
  ByteData = McD2PciCfg8 (IGD_R_CMD);
  S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint8,
      (UINTN) (MmPciAddress (0,0,2,0,IGD_R_CMD)),
      1,
      &ByteData
      );
  // AMI_OVERRIDE - S3BootScriptSaveMemPoll function doesn・t work properly issue - CSP20130723_A <<

  //
  // only 32bit read/write is legal for device 0:2:0
  //
  OriginalGTTMMADR  = (UINT64) (McD2PciCfg32 (IGD_R_GTTMMADR));
  OriginalGTTMMADR  = LShiftU64 ((UINT64) McD2PciCfg32 (IGD_R_GTTMMADR + 4), 32) | (OriginalGTTMMADR);

  //
  // 64bit GTTMADR does not work for S3 save script table since it is executed in PEIM phase
  // Program temporarily 32bits GTTMMADR for POST and S3 resume
  //
  LoGTBaseAddress                   = (UINT32) (GTTMMADR & 0xFFFFFFFF);
  HiGTBaseAddress                   = (UINT32) RShiftU64 ((GTTMMADR & 0xFFFFFFFF00000000), 32);
  McD2PciCfg32 (IGD_R_GTTMMADR)     = LoGTBaseAddress;
  McD2PciCfg32 (IGD_R_GTTMMADR + 4) = HiGTBaseAddress;

  //
  // only 32bit read/write is legal for device 0:2:0
  //

  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (MmPciAddress (0,
    0,
    2,
    0,
    IGD_R_GTTMMADR)),
    1,
    &LoGTBaseAddress
    );


  //
  // only 32bit read/write is legal for device 0:2:0
  //
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (MmPciAddress (0,
    0,
    2,
    0,
    IGD_R_GTTMMADR + 4)),
    1,
    &HiGTBaseAddress
    );


  if(DxePlatformSaPolicy->ForceWake == 1) {
    //
    // Un-Render Force Wake
    //
    RegOffset                     = 0x1300B0;
    Data32                        = 0x80000000;
    Mmio32 (GTTMMADR, RegOffset)  = Data32;

    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32
      );


    Data32Mask  = 0x8000;
    Result      = 0;
    PollGtReady_hang (GTTMMADR, 0x1300B4, Data32Mask, Result);
    S3BootScriptSaveMemPoll(
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + 0x1300B4),
      &Data32Mask,
      &Result,
      50,
      200000
      );

    RegOffset                     = 0x1300B8;
    Data32                        = 0x80000000;
    Mmio32 (GTTMMADR, RegOffset)  = Data32;

    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + RegOffset),
      1,
      &Data32
      );


    Data32Mask  = 0x8000;
    Result      = 0;
    PollGtReady_hang (GTTMMADR, 0x1300BC, Data32Mask, Result);
    S3BootScriptSaveMemPoll(
      S3BootScriptWidthUint32,
      (UINTN) (GTTMMADR + 0x1300BC),
      &Data32Mask,
      &Result,
      50,
      200000
      );
  }
  // AMI_OVERRIDE - S3BootScriptSaveMemPoll function doesn・t work properly issue - CSP20130723_A >>
  S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint8,
      (UINTN) (MmPciAddress (0,0,2,0,IGD_R_CMD)),
      1,
      &CmdData
      );
  // AMI_OVERRIDE - S3BootScriptSaveMemPoll function doesn・t work properly issue - CSP20130723_A <<
  //
  // Restore original GTTMMADR
  //
  LoGTBaseAddress                   = (UINT32) (OriginalGTTMMADR & 0xFFFFFFFF);
  HiGTBaseAddress                   = (UINT32) RShiftU64 ((OriginalGTTMMADR & 0xFFFFFFFF00000000), 32);
  McD2PciCfg32 (IGD_R_GTTMMADR)     = LoGTBaseAddress;
  McD2PciCfg32 (IGD_R_GTTMMADR + 4) = HiGTBaseAddress;
  //
  // only 32bit read/write is legal for device 0:2:0
  //
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (MmPciAddress (0,
    0,
    2,
    0,
    IGD_R_GTTMMADR)),
    1,
    &LoGTBaseAddress
    );

  //
  // only 32bit read/write is legal for device 0:2:0
  //
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint32,
    (UINTN) (MmPciAddress (0,
    0,
    2,
    0,
    IGD_R_GTTMMADR + 4)),
    1,
    &HiGTBaseAddress
    );

  //
  // Lock the following registers, GGC, BDSM, BGSM
  //
  McD2PciCfg32Or (IGD_R_GGC, LockBit);
  McD2PciCfg32Or (IGD_R_BDSM, LockBit);
  McD2PciCfg32Or (IGD_R_BGSM, LockBit);

  gBS->CloseEvent (Event);
  //
  // Return final status
  //
  return;
}

EFI_STATUS
GraphicsDxeInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
/*++

Routine Description:

Initialize GT Post Routines.

Arguments:

  ImageHandle         -     Handle for the image of this driver
  DxePlatformSaPolicy -     SA DxePlatformPolicy protocol

Returns:

  EFI_SUCCESS - GT POST initialization complete

--*/
{
  EFI_PHYSICAL_ADDRESS  MemBaseAddress;
  UINT32                LoGTBaseAddress;
  UINT32                HiGTBaseAddress;
  EFI_STATUS            Status;
  UINT8                 ByteData;
  UINTN                 DwordData;
  EFI_EVENT             mConOutEvent;
  VOID                  *gConOutNotifyReg;

  UINT32                Data32Mask;
  UINT32                Result;

// AMI_OVERRIDE START - EIP194648
  UINTN                 SetupSize;
  SETUP_DATA            SetupData;
  UINT32                SetupAttr;
  UINT32                Data32;
  
  SetupSize = sizeof (SETUP_DATA);
  Status = gRT->GetVariable (
                    L"Setup",
                    &gEfiSetupVariableGuid,
                    &SetupAttr,
                    &SetupSize,
                    &SetupData
                    );
  ASSERT_EFI_ERROR (Status);
  
  MsgBus32Read(VLV_FUSEEPNC, 0x2c, Data32);
  Data32 = (Data32 >> 24) & 0x1f;   // FB_GFX_MAX_NONTURBO_FUSE_MSB[28:24]
  
  if (Data32 == 0x4) {
      SetupData.InternalGraphics = 0;
      gRT->SetVariable(
                  L"Setup",
                  &gEfiSetupVariableGuid,
                  SetupAttr,
                  SetupSize,
                  &SetupData
                  );
      ASSERT_EFI_ERROR (Status);
  }
// AMI_OVERRIDE END

  GTTMMADR    = 0;
  Status      = EFI_SUCCESS;
  MCHBAR_BASE = McD0PciCfg64 (0x48) &~BIT0;
//  UINT32 Dbuff=0xFFFFFFFF;
// clock gating workarounds
////itp.chipsets["SOCDFX_TAP2IOSFSB_STAP0"].regbus.iosfreg(0,0x14,0x6b,6,0,0,0xf,0, 0xa0009)
//  MsgBus32Write(VLV_CCK, 0x6b, 0x000a0009);
//
////itp.chipsets["SOCDFX_TAP2IOSFSB_STAP0"].regbus.iosfreg(0,0x14,0x6C,6,0,0,0xf,0, 0xa0009)
//  MsgBus32Write(VLV_CCK, 0x6c, 0x000a0009);


//1 0x60 = 0xc0          //Display
//2 0x60 = 0xfff0c0       //Tx/Rx Lanes
//3 0x60 = 0xfffcc0         //Common lane
  MsgBus32Write(VLV_PUNIT, PUNIT_PWRGT_CNT_CTRL, 0xc0);
  Data32Mask  = 0xc0;
  Result      = 0xc0;
  PollPowerGate (Data32Mask, Result);

  MsgBus32Write(VLV_PUNIT, PUNIT_PWRGT_CNT_CTRL, 0xfff0c0);
  Data32Mask  = 0xfff0c0;
  Result      = 0xfff0c0;
  PollPowerGate (Data32Mask, Result);

  MsgBus32Write(VLV_PUNIT, PUNIT_PWRGT_CNT_CTRL, 0xfffcc0);
  Data32Mask  = 0xfffcc0;
  Result      = 0xfffcc0;
  PollPowerGate (Data32Mask, Result);

//Followed by ungating in this order
//1 0x60 = 0xf00cc0                 //Ungating Tx only
//2 0x60 = 0xf000c0                //Ungating Common lane only
//3 0x60 = 0xf00000                //Ungating Display
  MsgBus32Write(VLV_PUNIT, PUNIT_PWRGT_CNT_CTRL, 0xf00cc0);
  Data32Mask  = 0xfffcc0;
  Result      = 0xf00cc0;
  PollPowerGate (Data32Mask, Result);

  MsgBus32Write(VLV_PUNIT, PUNIT_PWRGT_CNT_CTRL, 0xf000c0);
  Data32Mask  = 0xffffc0;
  Result      = 0xf000c0;
  PollPowerGate (Data32Mask, Result);

  MsgBus32Write(VLV_PUNIT, PUNIT_PWRGT_CNT_CTRL, 0xf00000);
  Data32Mask  = 0xfffff0;
  Result      = 0xf00000;
  PollPowerGate (Data32Mask, Result);

// device id w/a
// itp.chipsets["SOCDFX_TAP2IOSFSB_STAP0"].regbus.iosfreg(0,6,0x0,5,0,0,0xf,0,0x0f318086)
//   MsgBus32Write(VLV_GUNIT, 0x0, 0x0f318086);
  //
  // If device 0:2:0 (Internal Graphics Device, or GT) is enabled, then Program GTTMMADR,
  //
  if (McD2PciCfg16 (IGD_R_VID) != 0xFFFF) {
    gDS     = NULL;
    Status  = EfiGetSystemConfigurationTable (&gEfiDxeServicesTableGuid, (VOID **) &gDS);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      //
      // If failed to get gDS, return with error.
      //
      return Status;
    }

    if (gDS!=NULL)   {

// AMI_OVERRIDE - Fix S3 can't wake issue. P052013A+ >>
//      //
//      // Enable Bus Master, I/O and Memory access on 0:2:0
//      //
//      McD2PciCfg8Or (IGD_R_CMD, (BIT2 | BIT1 | BIT0));
// AMI_OVERRIDE - Fix S3 can't wake issue. P052013A+ <<

      //
      // Means Allocate 4MB for GTTMADDR
      //
      MemBaseAddress = 0x0ffffffff;

      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateMaxAddressSearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      GTT_MEM_ALIGN,
                      GTTMMADR_SIZE_4MB,
                      &MemBaseAddress,
                      ImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);

      //
      // Program GT PM Settings if GTTMMADR allocation is Successful
      //
      GTTMMADR                          = (UINTN) MemBaseAddress;

      LoGTBaseAddress                   = (UINT32) (MemBaseAddress & 0xFFFFFFFF);
      HiGTBaseAddress                   = (UINT32) RShiftU64 ((MemBaseAddress & 0xFFFFFFFF00000000), 32);
      McD2PciCfg32 (IGD_R_GTTMMADR)     = LoGTBaseAddress;
      McD2PciCfg32 (IGD_R_GTTMMADR + 4) = HiGTBaseAddress;

      DwordData                         = McD2PciCfg32 (IGD_R_GTTMMADR);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (MmPciAddress (0,
        0,
        2,
        0,
        IGD_R_GTTMMADR)),
        1,
        &DwordData
        );

      DwordData = McD2PciCfg32 (IGD_R_GTTMMADR + 4);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (MmPciAddress (0,
        0,
        2,
        0,
        IGD_R_GTTMMADR + 4)),
        1,
        &DwordData
        );

// AMI_OVERRIDE - Fix S3 can't wake issue. P052013A+ >>
      //
      // Enable Bus Master, I/O and Memory access on 0:2:0
      //
      McD2PciCfg8Or (IGD_R_CMD, (BIT2 | BIT1 | BIT0));
      ByteData = McD2PciCfg8 (IGD_R_CMD);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint8,
        (UINTN) (MmPciAddress (0,
            0,
            2,
            0,
            IGD_R_CMD)),
        1,
        &ByteData
        );
// AMI_OVERRIDE - Fix S3 can't wake issue. P052013A+ <<


      //
      // Inititalize GtPostStruct components
      //
      GtPostStruct.GraphicsFreqReq            = 0;
      GtPostStruct.PmLock                     = 1;
      GtPostStruct.PavpMode                   = DxePlatformSaPolicy->PavpMode;
      GtPostStruct.UnsolicitedAttackOverride  = 0;
      GtPostStruct.ScanDumpMarkers            = 0;

      if (GtPostStruct.PavpMode != 0) {
        Status = PavpInit (ImageHandle, DxePlatformSaPolicy);
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }


      PmInit (ImageHandle, DxePlatformSaPolicy);


      //
      // Do POST GT PM Init Steps after VBIOS Initialization in DoPostPmInitCallBack
      //
      Status = gBS->CreateEvent (
                      EVT_NOTIFY_SIGNAL,
                      TPL_CALLBACK,
                      PostPmInitCallBack,
                      NULL,
                      &mConOutEvent
                      );

      ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR (Status)) {
        return Status;
      }

#ifdef SG_SUPPORT

  DEBUG ((DEBUG_INFO, "Initializing Switchable Graphics (Dxe)\n"));
  SwitchableGraphicsInit (ImageHandle, DxePlatformSaPolicy);

#endif

      Status = gBS->RegisterProtocolNotify (
                      &gEfiGraphicsOutputProtocolGuid,
                      mConOutEvent,
                      &gConOutNotifyReg
                      );

// AMI_OVERRIDE - Fix S3 can't wake issue. P052013A+ >>
      //
      // Disable Bus Master, I/O and Memory access on 0:2:0
      //
      McD2PciCfg8And (IGD_R_CMD, ~(BIT2 | BIT1 | BIT0));
      ByteData = McD2PciCfg8 (IGD_R_CMD);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint8,
        (UINTN) (MmPciAddress (0,
            0,
            2,
            0,
            IGD_R_CMD)),
        1,
        &ByteData
        );

      McD2PciCfg64 (IGD_R_GTTMMADR) = 0;
      
      DwordData                         = McD2PciCfg32 (IGD_R_GTTMMADR);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (MmPciAddress (0,
            0,
            2,
            0,
            IGD_R_GTTMMADR)),
        1,
        &DwordData
        );

      DwordData = McD2PciCfg32 (IGD_R_GTTMMADR + 4);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (MmPciAddress (0,
            0,
            2,
            0,
            IGD_R_GTTMMADR + 4)),
        1,
        &DwordData
        );
// AMI_OVERRIDE - Fix S3 can't wake issue. P052013A+ <<

      //
      // Free allocated resources
      //
      gDS->FreeMemorySpace (MemBaseAddress, GTTMMADR_SIZE_4MB);
    } else {
      return EFI_NOT_FOUND;
    }
  }

  return EFI_SUCCESS;
}



VOID
PollGtReady (
  UINT64 Base,
  UINT32 Offset,
  UINT32 Mask,
  UINT32 Result
  )
/*++

Routine Description:

  "Poll Status" for GT Readiness

Arguments:

  Base            - Base address of MMIO
  Offset          - MMIO Offset
  Mask            - Mask
  Result          - Value to wait for

Returns:

  None

--*/
{
  UINT32  GtStatus;
  UINT16  StallCount;

  StallCount = 0;

  //
  // Register read
  //
  GtStatus = Mmio32 (Base, Offset);

  while (((GtStatus & Mask) != Result) && (StallCount < GT_WAIT_TIMEOUT)) {

    GtStatus = Mmio32 (Base, Offset);
    //
    // 1mSec wait
    //
    gBS->Stall (1000);
    StallCount = StallCount + 1;
  }

  ASSERT ((StallCount != GT_WAIT_TIMEOUT));
}
