/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PchLpe.c

Abstract:

  Initializes PCH Low Power Audio Engine Device

--*/
#include "PchInit.h"
#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/GlobalNvsArea.h>
#endif
#define GLOBAL_NVS_OFFSET(Field)    (UINTN)((CHAR8*)&((EFI_GLOBAL_NVS_AREA*)0)->Field - (CHAR8*)0)
UINT32                                             gIndex = 0;

typedef struct {
  EFI_PHYSICAL_ADDRESS    base;
  UINT32                              length;
} LPE_MEM_LIST;

LPE_MEM_LIST   gLpeList[] = {
  {0x20000000,   0x00100000},   // 512M
  {0x40000000,   0x00100000},   // 1G
  {0x60000000,   0x00100000},   // 1.5G
  {0x00000000,   0x00000000}    // END
};

#ifdef ECP_FLAG
#include <Protocol/PciIo/PciIo.h>
#include "Pci22.h"
#else
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci22.h>
#endif
#define PCI_DEVICE_NUMBER_PCH_AZALIA       27
#define PCI_FUNCTION_NUMBER_PCH_AZALIA     0
#define R_PCH_HDA_HDBARL                   0x10  // HDA CTL Memory BAR Lower
#define B_PCH_HDA_HDBARL_LBA               0xFFFFC000 // Lower Base Address
#define R_HDA_GCTL                         0x08  // Global Control
#define B_HDA_GCTL_CRST                    BIT0  // Controller Reset
#define B_PCH_PMC_FUNC_DIS_AZALIA          BIT12 // Azalia Disable
#define R_PCH_HDA_TM1                      0x43  // Test Mode 1

EFI_STATUS
ConfigureLpe (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure LPE devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  EFI_STATUS            Status = EFI_NOT_FOUND;
  UINTN                 LpePciMmBase;
  EFI_PHYSICAL_ADDRESS  LpeMmioBase0;
  UINT32                Buffer32;
#ifndef ECP_FLAG
  EFI_PHYSICAL_ADDRESS   LpeAudio = 0;
#endif
  DEBUG ((EFI_D_INFO, "ConfigureLpe() Start\n"));

#ifndef ECP_FLAG
  ///
  /// Allocate ACPI Memory space for Audio Driver
  ///
  gIndex = 0;
  while((gLpeList[gIndex].base != 0x0) && (gLpeList[gIndex].length != 0x0)) {
    LpeAudio = gLpeList[gIndex].base;
    Status = gBS->AllocatePages (
                    AllocateAddress,
                    EfiReservedMemoryType,
                    EFI_SIZE_TO_PAGES (gLpeList[gIndex].length),
                    &LpeAudio
                    );
    if (!EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "LPE Allocate Audio Memory success at 0x%08x\n", gLpeList[gIndex].base));
      break;
    }
    DEBUG((EFI_D_ERROR, "LPE Allocate Audio Memory fail at 0x%08x\n", gLpeList[gIndex].base));
    gIndex++;
  }

  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR(Status);
  }
#endif

  LpePciMmBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPE,
                      PCI_FUNCTION_NUMBER_PCH_LPE,
                      0
                    );
  LpeMmioBase0 = 0;
  Buffer32     = 0;

  if (PchPlatformPolicy->DeviceEnabling->LpeEnabled == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting LPE Audio into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (LpePciMmBase + R_PCH_LPE_PCS), B_PCH_LPE_PCS_PS);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (LpePciMmBase + R_PCH_LPE_PCS),
      1,
      (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPE;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (LpePciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_LPE_BAR0_ALIGNMENT,
                      V_PCH_LPE_BAR0_SIZE,
                      &LpeMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD), (UINT32) ~(B_PCH_LPE_STSCMD_BME | B_PCH_LPE_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD),
          1,
          (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((LpeMmioBase0 & B_PCH_LPE_BAR0_BA) == LpeMmioBase0) && (LpeMmioBase0 != 0));
        MmioWrite32 ((UINTN) (LpePciMmBase + R_PCH_LPE_BAR0), (UINT32) (LpeMmioBase0 & B_PCH_LPE_BAR0_BA));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (LpePciMmBase + R_PCH_LPE_BAR0),
          1,
          (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_BAR0)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD), (UINT32) (B_PCH_LPE_STSCMD_BME | B_PCH_LPE_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD),
          1,
          (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD)
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD), (UINT32) ~(B_PCH_LPE_STSCMD_BME | B_PCH_LPE_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD),
          1,
          (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (LpePciMmBase + R_PCH_LPE_BAR0), (UINT32) (0x00));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (LpePciMmBase + R_PCH_LPE_BAR0),
          1,
          (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_BAR0)
          );
        gDS->FreeMemorySpace (LpeMmioBase0, (UINT64) V_PCH_LPE_BAR0_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "LPE Audio not present, skipping.\n"));
      PchPlatformPolicy->DeviceEnabling->LpeEnabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_LPE;
    }
  }

  DEBUG ((EFI_D_INFO, "ConfigureLpe() End\n"));

  return EFI_SUCCESS;
}

//VOID
//UpdateLpeDeviceBar (
//  EFI_PCI_IO_PROTOCOL      *PciIo,
//  EFI_PHYSICAL_ADDRESS     BaseAddress,
//  UINT8                    BarIndex
//  )
//{
//  UINT8          NewCommand;
//  UINT32         NewBar;
//  EFI_STATUS     Status;
//  UINT16         Command;
//
//  Status = PciIo->Pci.Read (
//                    PciIo,
//                    EfiPciIoWidthUint16,
//                    PCI_COMMAND_OFFSET,
//                    1,
//                    &Command
//                    );
//
//  NewCommand = 0;
//  Status = PciIo->Pci.Write(
//                    PciIo,
//                    EfiPciIoWidthUint8,
//                    PCI_COMMAND_OFFSET,
//                    1,
//                    &NewCommand
//                    );
//
//  NewBar = (UINT32)BaseAddress;
//  Status = PciIo->Pci.Write(
//                    PciIo,
//                    EfiPciIoWidthUint32,
//                    PCI_BASE_ADDRESSREG_OFFSET + BarIndex * sizeof(UINT32),
//                    1,
//                    &NewBar
//                    );
//
//  Status = PciIo->Pci.Write(
//                    PciIo,
//                    EfiPciIoWidthUint16,
//                    PCI_COMMAND_OFFSET,
//                    1,
//                    &Command
//                    );
//}


VOID
UpdateLpeDeviceBar (
  EFI_PCI_IO_PROTOCOL      *PciIo,
  EFI_PHYSICAL_ADDRESS     BaseAddress,
  UINT8                    BarIndex
  )
{
  UINT8          NewCommand;
  UINT32         NewBar;
  EFI_STATUS     Status;
  UINT16         Command;

  Status = PciIo->Pci.Read (
                    PciIo,
                    EfiPciIoWidthUint16,
                    PCI_COMMAND_OFFSET,
                    1,
                    &Command
                    );

  NewCommand = 0;
  Status = PciIo->Pci.Write(
                    PciIo,
                    EfiPciIoWidthUint8,
                    PCI_COMMAND_OFFSET,
                    1,
                    &NewCommand
                    );

  NewBar = (UINT32)BaseAddress;
  Status = PciIo->Pci.Write(
                    PciIo,
                    EfiPciIoWidthUint32,
                    PCI_BASE_ADDRESSREG_OFFSET + BarIndex * sizeof(UINT32),
                    1,
                    &NewBar
                    );

  Status = PciIo->Pci.Write(
                    PciIo,
                    EfiPciIoWidthUint16,
                    PCI_COMMAND_OFFSET,
                    1,
                    &Command
                    );
}


VOID
UpdateLpeResource (
  EFI_PCI_IO_PROTOCOL     *PciIo
  )
{
  VOID                                  *Resources;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR     *ptr;
  EFI_STATUS                            Status;
  UINT8                                 BarIndex;
  EFI_PHYSICAL_ADDRESS                  BaseAddress;

  for (BarIndex = 0; BarIndex < PCI_MAX_BAR; BarIndex ++) {
    Status = PciIo->GetBarAttributes (//scan pci device's bar
                      PciIo,
                      BarIndex,
                      NULL,
                      &Resources
                      );
    if (EFI_ERROR(Status)) {
      continue;
    }

    ptr = Resources;
    if (ptr->ResType != ACPI_ADDRESS_SPACE_TYPE_MEM) {
#ifdef ECP_FLAG
      FreePool (Resources);
#else
      gBS->FreePool (Resources);
#endif
      continue;
    }

    DEBUG((DEBUG_ERROR, "Devices  %x %x %x %x %x %lx %lx %lx %lx %lx\n",
      ptr->Desc, ptr->Len, ptr->ResType, ptr->GenFlag, ptr->SpecificFlag,
      ptr->AddrSpaceGranularity, ptr->AddrRangeMin, ptr->AddrRangeMax,
      ptr->AddrTranslationOffset, ptr->AddrLen
      ));

    BaseAddress = 0xFFFFFFFF;
    Status = gDS->AllocateMemorySpace (
                    EfiGcdAllocateMaxAddressSearchBottomUp,
                    EfiGcdMemoryTypeMemoryMappedIo,
                    HighBitSet64(ptr->AddrLen),
                    ptr->AddrLen,
                    &BaseAddress,
                    mImageHandle,
                    NULL
                    );

    //ASSERT_EFI_ERROR (Status);
    DEBUG((DEBUG_ERROR, "New Resource is %x %lx\n", BarIndex, BaseAddress));
    UpdateLpeDeviceBar (PciIo, BaseAddress, BarIndex);
#ifdef ECP_FLAG
    FreePool (Resources);
#else
    gBS->FreePool (Resources);
#endif
  }
}

EFI_STATUS
ReAssignLpeResource (
  )
{
  EFI_STATUS              Status;
  UINTN                   HandleCount;
  EFI_HANDLE              *Handles;
  EFI_PCI_IO_PROTOCOL     *PciIo;
  UINTN                   Index;
  UINTN                   Segment;
  UINTN                   BusNum;
  UINTN                   DeviceNum;
  UINTN                   FunctionNum;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  for(Index = 0; Index < HandleCount; Index ++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = PciIo->GetLocation(
                      PciIo,
                      &Segment,
                      &BusNum,
                      &DeviceNum,
                      &FunctionNum
                      );

    if ((BusNum == DEFAULT_PCI_BUS_NUMBER_PCH) &&
        (DeviceNum == PCI_DEVICE_NUMBER_PCH_LPE) &&
        (FunctionNum == PCI_FUNCTION_NUMBER_PCH_LPE)) {
        DEBUG((EFI_D_ERROR, "Update Device %x %x %x\n", BusNum, DeviceNum, FunctionNum));
        UpdateLpeResource (PciIo);
    }
  }
#ifdef ECP_FLAG
  FreePool (Handles);
#else
  gBS->FreePool(Handles);
#endif
  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureLpeAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Hide PCI config space of LPE device and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN                         LpePciMmBase;
  UINT32                        LpeMmioBase0;
  UINT32                        LpeMmioBase1;
  UINT32                        LpeMmioBase2;
  UINT32                        Buffer32;
  EFI_STATUS                    AcpiTablePresent;
#ifndef ECP_FLAG
  EFI_STATUS                    Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
#endif
  UINTN                         LpeCacheAddress;
  UINTN                         LpeCacheLength;
  
  DEBUG ((EFI_D_INFO, "ConfigureLpeAtBoot() Start\n"));

  ///
  /// Initialize Variables
  ///
  LpePciMmBase     = 0;
  LpeMmioBase0     = 0;
  LpeMmioBase1     = 0;
  LpeMmioBase2     = 0;
  Buffer32         = 0;
  AcpiTablePresent = EFI_NOT_FOUND;
  ///
  /// Locate ACPI table
  ///
  AcpiTablePresent = InitializePchAslUpdateLib ();
  ///
  /// Update LPE device ACPI variables
  ///
  if (!EFI_ERROR (AcpiTablePresent)) {
    LpePciMmBase = MmPciAddress (0,
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     PCI_DEVICE_NUMBER_PCH_LPE,
                     PCI_FUNCTION_NUMBER_PCH_LPE,
                     0
                     );

    LpeCacheAddress = LpePciMmBase + 0xA8;
    LpeCacheLength = LpePciMmBase + 0xAC;

    switch (PchStepping()) {
      case PchA0:
      case PchA1:
      case PchB0:
      case PchB1:
      case PchB2:
      case PchB3:
        //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled. >>
#if defined(CRID_SUPPORT) && CRID_SUPPORT
        if(MmioRead32 ((UINTN) (PMC_BASE_ADDRESS + R_PCH_PMC_CRID)) & B_PCH_PMC_CRID_RID_SEL ) 
        {
          LpeMmioBase0 = MmioRead32 ((UINTN) (LpePciMmBase + R_PCH_LPE_BAR0)) & B_PCH_LPE_BAR0_BA;
          LpeCacheAddress = LpeMmioBase0 + 0x144000;
          LpeCacheLength = LpeMmioBase0 + 0x144004;
        }
#endif
        //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled. <<
        break;

      case PchC0:
      default:
        LpeMmioBase0 = MmioRead32 ((UINTN) (LpePciMmBase + R_PCH_LPE_BAR0)) & B_PCH_LPE_BAR0_BA;
        LpeCacheAddress = LpeMmioBase0 + 0x144000;
        LpeCacheLength = LpeMmioBase0 + 0x144004;
        break;
    }

    MmioWrite32 ((UINTN) (LpeCacheAddress), (UINT32)gLpeList[gIndex].base);
    Buffer32 = MmioRead32(LpeCacheAddress);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (LpeCacheAddress),
      1,
      &Buffer32
      );
    MmioWrite32 ((UINTN) (LpeCacheLength), (UINT32)gLpeList[gIndex].length);
    Buffer32 = MmioRead32(LpeCacheLength);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (LpeCacheLength),
      1,
      &Buffer32
      );

    if (PchPlatformPolicy->DeviceEnabling->LpeEnabled == 2) {
      DEBUG ((EFI_D_INFO, "Switching LPE Audio into ACPI Mode.\n"));

#ifndef ECP_FLAG
      ReAssignLpeResource ();
#endif

      LpeMmioBase0 = MmioRead32 ((UINTN) (LpePciMmBase + R_PCH_LPE_BAR0)) & B_PCH_LPE_BAR0_BA;
      LpeMmioBase1 = MmioRead32 ((UINTN) (LpePciMmBase + R_PCH_LPE_BAR1)) & B_PCH_LPE_BAR1_BA;
#ifndef ECP_FLAG
      Status = gBS->LocateProtocol (
                      &gEfiGlobalNvsAreaProtocolGuid,
                      NULL,
                      (VOID **) &GlobalNvsArea
                      );
      if (EFI_ERROR(Status)) {
        return Status;
      }

      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + GLOBAL_NVS_OFFSET(LPEBar0)) = LpeMmioBase0;
      DEBUG ((EFI_D_INFO, "Switching LPE Audio into ACPI Mode LpeMmioBase1=%x--------------------\n",LpeMmioBase0));

      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + GLOBAL_NVS_OFFSET(LPEBar1)) = LpeMmioBase1;
      DEBUG ((EFI_D_INFO, "Switching LPE Audio into ACPI Mode LpeMmioBase1=%x--------------------\n",LpeMmioBase1));

      *(UINT32*)((CHAR8*)GlobalNvsArea->Area + GLOBAL_NVS_OFFSET(LPEBar2)) = (UINT32)gLpeList[gIndex].base;
      DEBUG ((EFI_D_INFO, "Switching LPE Audio into ACPI Mode LpeMmioBase2=%x--------------------\n",gLpeList[gIndex].base));
#endif
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD),
        (UINT32) (B_PCH_LPE_STSCMD_INTR_DIS | B_PCH_LPE_STSCMD_BME | B_PCH_LPE_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD),
        1,
        (VOID *) (UINTN) (LpePciMmBase + R_PCH_LPE_STSCMD)
        );
      ///
      /// Switch to ACPI Mode
      ///
      PchMsgBusAndThenOr32AddToS3Save (
        PCH_LPE_PORT_ID,
        R_PCH_LPE_PCICFGCTR1,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_LPE_PCICFGCTR1_ACPI_INT_EN1 | B_PCH_LPE_PCICFGCTR1_PCI_CFG_DIS1),
        PCH_LPE_PRIVATE_READ_OPCODE,
        PCH_LPE_PRIVATE_WRITE_OPCODE
        );

    }
    // Audio clock workarounds
    //<iosfsb bar:0x0,device:0x0,function:0x0,offset:0x48,portid:0xa9,readop:0x6,size:0x4,tap:SOCDFX_TAP2IOSFSB_STAP0,writeop:0x7,>
    //vlv.ccu.plt_clk_ctrl_3 = 3
    //      PchMsgBusAndThenOr32(0xa9, 0x48, Dbuff, 0xffffff00, 0x3, 0x6, 0x7)
    PchMsgBusAndThenOr32AddToS3Save (
      0xa9, // CCU
      0x48, // offset
      Buffer32,
      0xFFFFFF00, // AND
      0x3,  // OR
      PCH_LPE_PRIVATE_READ_OPCODE,
      PCH_LPE_PRIVATE_WRITE_OPCODE
      );
  }

  DEBUG ((EFI_D_INFO, "ConfigureLpeAtBoot() End\n"));

  return EFI_SUCCESS;
}

VOID
HDA_4683125_WA (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  UINTN                         AzaliaBase;
  UINT32                        HdaBar;

  AzaliaBase = MmPciAddress (0,
                 0,
                 PCI_DEVICE_NUMBER_PCH_AZALIA,
                 PCI_FUNCTION_NUMBER_PCH_AZALIA,
                 0
               );
  HdaBar = MmioRead32 (AzaliaBase + R_PCH_HDA_HDBARL)&B_PCH_HDA_HDBARL_LBA;
  DEBUG((EFI_D_ERROR,"HdaBar=0x%08X\n",HdaBar));
  MmioWrite8 ((AzaliaBase + R_PCH_HDA_TM1), 0x0D7); // BIOS w/a for HDMI audio issue
  S3BootScriptSaveMemWrite(
    EfiBootScriptWidthUint32,
    (UINTN) (AzaliaBase + R_PCH_HDA_TM1),
    1,
    (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_TM1)
    );
  MmioOr32 ((UINTN) (HdaBar + R_HDA_GCTL), (UINT32) (B_HDA_GCTL_CRST));
  S3BootScriptSaveMemWrite(
    EfiBootScriptWidthUint32,
    (UINTN) (HdaBar + R_HDA_GCTL),
    1,
    (VOID *) (UINTN) (HdaBar + R_HDA_GCTL)
    );
  MmioOr32 ((UINTN) (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS), (UINT32) (B_PCH_PMC_FUNC_DIS_AZALIA));
  S3BootScriptSaveMemWrite(
    EfiBootScriptWidthUint32,
    (UINTN) (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS),
    1,
    (VOID *) (UINTN) (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS)
    );
}


