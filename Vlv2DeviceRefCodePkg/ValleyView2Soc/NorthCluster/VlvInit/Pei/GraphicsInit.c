/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  GraphicsInit.c

Abstract:

  PEIM to initialize both IGD and PCI graphics card.

--*/

#include "GraphicsInit.h"
#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#endif
#include "PchRegs.h"

#define IGD_ENABLE  1
#define IGD_DISABLE 0

#define IGD_TURBO_EN  1
#define IGD_TURBO_DIS 0

VOID
GraphicsInit (
  IN CONST EFI_PEI_SERVICES                **PeiServices,
  IN VLV_POLICY_PPI          *VlvPolicyPpi
  )
/*++

Routine Description:

  GraphicsInit: Initialize the IGD if no other external graphics is present

Arguments:

  PeiServices          - Pointer to the PEI services table
  VlvPolicyPpi  - VlvPolicyPpi to access the GtConfig related information

Returns:

  None

--*/
{
  DISPLAY_DEVICE  PrimaryDisplay;
  UINT8           GMSData;
  UINT32          PchPcieMmioLength;
  UINT32          IGfxMmioLength;
  UINT32          TotalMmioLength;
  BOOLEAN         IGfxSupported;
  UINT32          DwordReg = 0xFFFFFFFF;

  PchPcieMmioLength = 0;
  IGfxMmioLength    = 0;

  //
  // Set the VGA Decode bits to a good known starting point where both PEG and
  // IGD VGA Decode Bits are both disabled.
  //
  McD2PciCfg16Or (GGC, GGC_IVD_MASK);
  PrimaryDisplay = IGD;

  //
  // Check if IGfx is supported
  //
  IGfxSupported = (BOOLEAN) (McD2PciCfg16 (IGD_R_VID) != 0xFFFF);

  //
  // Check external VGA devices
  //
  CheckOffboardPcieVga (PeiServices, &PchPcieMmioLength, &PrimaryDisplay);

  //
  // If primary display device is IGD or no other display detected then enable IGD
  //
  if (IGfxSupported && (VlvPolicyPpi->GtConfig.InternalGraphics == IGD_ENABLE || PrimaryDisplay == IGD)) {
    DEBUG ((EFI_D_INFO, "IGD enabled.\n"));
    //
    // Program GFX Memory by setting D2.F0.R 050h [7:3]
    //
    McD2PciCfg16And (GGC, ~(GGC_GMS_MASK));
    GMSData = VlvPolicyPpi->GtConfig.IgdDvmt50PreAlloc;
    McD2PciCfg8Or (GGC, (GMSData << GGC_GMS_OFFSET));

    //
    // Program Graphics GTT Memory
    // D2:F0:R50h[9:8] = 01b => 1MB of GTT
    // D2:F0:R50h[9:8] = 10b => 2MB of GTT
    //
    if (VlvPolicyPpi->GtConfig.GttSize == GTT_SIZE_1MB) {
      McD2PciCfg16AndThenOr (GGC, ~(GGC_GGMS_MASK), 1 << GGC_GGMS_OFFSET);
    } else {
      McD2PciCfg16AndThenOr (GGC, ~(GGC_GGMS_MASK), 1 << (GGC_GGMS_OFFSET + 1));
    }

    //
    // Set register D2.F0.R 062h [2:1] = `01b' to set a 256MByte aperture.
    // This must be done before Device 2 registers are enumerated.
    //
    if (VlvPolicyPpi->GtConfig.ApertureSize == APERTURE_SIZE_128MB) {
      McD2PciCfg8And (IGD_MSAC_OFFSET, ~(BIT2 + BIT1));
    } else if (VlvPolicyPpi->GtConfig.ApertureSize == APERTURE_SIZE_256MB) {
      McD2PciCfg8AndThenOr (IGD_MSAC_OFFSET, ~(BIT2 + BIT1), BIT1);
    } else {
      McD2PciCfg8Or (IGD_MSAC_OFFSET, (BIT2 + BIT1));
    }
    //
    // Enable IGD VGA Decode.  This is needed so the Class Code will
    // be correct for the IGD Device when determining which device
    // should be primary.  If disabled, IGD will show up as a non VGA device.
    //
    if (PrimaryDisplay == PCI && VlvPolicyPpi->GtConfig.PrimaryDisplay != IGD) {
      //
      // If IGD is forced to be enabled, but is a secondary display, disable IGD VGA Decode
      //
      McD2PciCfg16Or (GGC, GGC_IVD_MASK);
      DEBUG ((EFI_D_INFO, "IGD VGA Decode is disabled because it's not a primary display.\n"));
    } else {
      McD2PciCfg16And (GGC, ~(GGC_IVD_MASK));

      if(VlvPolicyPpi->GtConfig.IgdTurboEn == IGD_TURBO_DIS) {

        DEBUG ((EFI_D_INFO, "IGD Turbo Disable.\n"));
        MsgBus32Read( VLV_PUNIT,PUNIT_BIOS_CONFIG,DwordReg);
        DEBUG ((EFI_D_INFO, "PUNIT_BIOS_CONFIG1 = 0x%x.\n",DwordReg));
        DwordReg |= B_GFX_TURBO_DIS;
        MsgBus32Write(VLV_PUNIT, PUNIT_BIOS_CONFIG, DwordReg);
        MsgBus32Read( VLV_PUNIT,PUNIT_BIOS_CONFIG,DwordReg);
        DEBUG ((EFI_D_INFO, "PUNIT_BIOS_CONFIG2 = 0x%x.\n",DwordReg));

      } else {

        DEBUG ((EFI_D_INFO, "IGD Turbo Enable.\n"));
        MsgBus32Read( VLV_PUNIT,PUNIT_BIOS_CONFIG,DwordReg);
        DEBUG ((EFI_D_INFO, "PUNIT_BIOS_CONFIG11 = 0x%x.\n",DwordReg));
        DwordReg &= ~B_GFX_TURBO_DIS;
        MsgBus32Write(VLV_PUNIT, PUNIT_BIOS_CONFIG, DwordReg)
        MsgBus32Read( VLV_PUNIT,PUNIT_BIOS_CONFIG,DwordReg);
        DEBUG ((EFI_D_INFO, "PUNIT_BIOS_CONFIG22 = 0x%x.\n",DwordReg));

      }
    }

    FindPciDeviceMmioLength (0, 2, 0, &IGfxMmioLength);

  } else {

    DEBUG ((EFI_D_INFO, "Disable IGD Device.\n"));

    //
    // Disable IGD device
    //
    // Register D0:F0 Offset 50h [7:3] = '00000b'.
    // This prevents UMA memory from being pre-allocated to IGD.
    // Set D0:F0 Offset 50h [9:8] = '00b'.
    // GTT Graphics Memory Size to 0
    //
    McD2PciCfg16AndThenOr (GGC, ~(GGC_GGMS_MASK | GGC_GMS_MASK), GGC_IVD_MASK);

    //
    //When set, the function is disabled (configuration space is disabled).
    //When set, the GVD stops accepting any new requests on the IOSF
    //bus including any new configuration cycle requests to clear this bit.
    //
    McD2PciCfg8Or(0xC4, BIT0);
    VlvPolicyPpi->GtConfig.GttSize           = 0;
    VlvPolicyPpi->GtConfig.IgdDvmt50PreAlloc = 0;
  }

  TotalMmioLength = IGfxMmioLength;
  TotalMmioLength += PchPcieMmioLength;

  DEBUG ((EFI_D_ERROR, "TotalMmioLength:   0x%08X bytes\n", TotalMmioLength));

  //
  // Determine MMIO Size for Dynamic Tolud
  //
  if (VlvPolicyPpi->MemConfig.MaxTolud == 0x00) {
    //
    // if total MMIO need 1GB or over
    //
    if (TotalMmioLength >= 0x40000000) {
      VlvPolicyPpi->GtConfig.MmioSize = 0x800;
    }
    //
    // if total MMIO need 728MB~1GB
    //
    else if (TotalMmioLength >= 0x30000000) {
      VlvPolicyPpi->GtConfig.MmioSize = 0x700;
    }
    //
    // if total MMIO need 512MB~728MB
    //
    else if (TotalMmioLength >= 0x20000000) {
      VlvPolicyPpi->GtConfig.MmioSize = 0x600;
    }
    //
    // if total MMIO need 256MB~512MB
    //
    else if (TotalMmioLength >= 0x10000000) {
      VlvPolicyPpi->GtConfig.MmioSize = 0x500;
    }
    //
    // if total MMIO need less than 256MB
    //
    else {
      VlvPolicyPpi->GtConfig.MmioSize = 0x400;
    }
  }

}

VOID
FindPciDeviceMmioLength (
  IN UINT32  BusNum,
  IN UINT32  DevNum,
  IN UINT32  FunNum,
  OUT UINT32 *MmioLength
  )
/*++

Routine Description:

  Find the MMIO size that a given PCI device requires

Arguments:

  BusNum     - Bus number of the device
  DevNum     - device Number of the device
  FunNum     - Function number of the device
  MmioLength - MMIO Length in bytes

Returns:

  None

--*/
{
  UINT32  CurrentMmioLength;
  UINT32  SavedBAR;
  UINT32  i;
  UINT64  BarAlign;
  UINT8   ClassCode;

  *MmioLength = 0;
  BarAlign    = PCI_BAR_OLD_ALIGN;

  ClassCode = MmPci8 (0, BusNum, DevNum, FunNum, PCI_BCC);
  if (ClassCode == PCI_CLASS_BRIDGE) {
    return;
  }

  for (i = PCI_BAR0; i <= PCI_BAR5; i += 4) {
    SavedBAR = MmPci32 (0, BusNum, DevNum, FunNum, i);
    //
    // Check BAR is read-only or not
    //
    MmPci32And (0, BusNum, DevNum, FunNum, i, (UINT32) PCI_BAR_NOCHANGE);
    MmPci32Or (0, BusNum, DevNum, FunNum, i, BarAlign);
    if (SavedBAR == MmPci32 (0, BusNum, DevNum, FunNum, i)) {
      //
      // Restore BAR as original value
      //
      MmPci32 (0, BusNum, DevNum, FunNum, i) = SavedBAR;
      continue;
    }
    //
    // If BAR is not memory map, skip it
    //
    if ((SavedBAR & BIT0) != 0) {
      //
      // Restore BAR as original value
      //
      MmPci32 (0, BusNum, DevNum, FunNum, i) = SavedBAR;
      continue;
    }
    //
    // Calculate the MMIO length through BAR
    //
    CurrentMmioLength = ~(MmPci32 (0, BusNum, DevNum, FunNum, i) &~0xF) + 1;
    *MmioLength += CurrentMmioLength;

    //
    // Restore BAR as original value
    //
    MmPci32 (0, BusNum, DevNum, FunNum, i) = SavedBAR;
    //
    // Skip next index if BAR is 64bit address
    //
    if ((SavedBAR & (BIT1 + BIT2)) == 0x4) {
      i += 4;
    }
  }
}

VOID
CheckOffboardPcieVga (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN OUT UINT32               *PchPcieMmioLength,
  IN OUT DISPLAY_DEVICE       *PrimaryDisplay
  )
/*++

Routine Description:

  CheckOffboardPcieVga: Check if off board PCIe graphics Card is present

Arguments:

  PeiServices        - Pointer to the PEI services table
  PchPcieMmioLength  - Total PCIe MMIO length on all PCH root ports
  PrimaryDisplay     - Primary Display - default is IGD

Returns:

  None

--*/
{
  UINT8   PortNo;
  UINT32  PcieBusNum;
  UINT8   Bus;
  UINT8   Dev;
  UINT8   Func;
  UINT8   MaxFunction;
  UINT8   SubBusNum;
  UINT8   HeaderType;
  UINT16  Buffer16;
  BOOLEAN CardDetect;
  UINT32  MmioLength;

  MmioLength = 0;

  //
  // Initialize Secondary and Subordinate bus number for first Pcie root port
  //
  PcieBusNum  = 0x00010100;

  SubBusNum   = 0;

  CardDetect  = FALSE;

  for (PortNo = 0; PortNo < PCH_PCIE_MAX_ROOT_PORTS; PortNo++) {
    //
    // Check if root port exists
    //
    if (MmPci16 (0, 0, 0x1C, PortNo, R_PCH_PCI_PCI_VID) == 0xFFFF) {
      continue;
    }

    MmPci32 (0, 0, 0x1c, PortNo, R_PCH_PCI_PCI_PBN) = PcieBusNum;
    Bus = MmPci8 (0, 0, 0x1c, PortNo, R_PCH_PCI_PCI_SCBN);

    //
    // Assign temporary subordinate bus number so that device this bridge can be seen
    //
    MmPci8 (0, 0, 0x1c, PortNo, R_PCH_PCI_PCI_SBBN) = 0xff;

    //
    // A config write is required in order for the device to re-capture the Bus number,
    // according to PCI Express Base Specification, 2.2.6.2
    // Write to a read-only register VendorID to not cause any side effects.
    //
    MmPci16 (0, Bus, 0, 0, PCI_VID) = 0;

    SubBusNum = EnumerateDownstream (Bus);
    //
    // Update the actual subordinate bus number
    //
    MmPci8 (0, 0, 0x1c, PortNo, R_PCH_PCI_PCI_SBBN) = SubBusNum;
    PcieBusNum = (SubBusNum + 1) << 8;
  }

  for (Bus = 1; Bus <= SubBusNum; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      //
      // Read Vendor ID to check if device exists
      // if no device exists, then check next device
      //
      if (MmPci16 (0, Bus, Dev, 0, R_PCH_PCI_PCI_VID) == 0xFFFF) {
        continue;
      }

      //
      // Check for a multifunction device
      //
      HeaderType = MmPci8 (0, Bus, Dev, 0, R_PCH_PCI_PCI_HEADTYP);
      if ((HeaderType & B_PCH_PCI_PCI_HEADTYP_MFD) != 0) {
        MaxFunction = 7;
      } else {
        MaxFunction = 0;
      }

      for (Func = 0; Func <= MaxFunction; Func++) {
        if (MmPci16 (0, Bus, Dev, Func, R_PCH_PCI_PCI_VID) == 0xFFFF) {
          continue;
        }

        FindPciDeviceMmioLength (Bus, Dev, Func, &MmioLength);
        *PchPcieMmioLength += MmioLength;

        //
        // Video cards can have Base Class 0 with Sub-class 1
        // or Base Class 3.
        //
        if (MmPci16 (0, Bus, Dev, Func, R_PCH_PCI_PCI_SCC) == 0x0300) {
          if (CardDetect != TRUE) {
            *PrimaryDisplay = PCI;
            DEBUG ((EFI_D_ERROR, "PCH PCIe Graphics Card enabled.\n"));
            CardDetect = TRUE;
          }
        }
      }
    }
  }

  //
  // Clear bus number on all the bridges that we have opened so far.
  // We have to do it in the reverse Bus number order.
  //
  for (Bus = SubBusNum; Bus >= 1; Bus--) {
    for (Dev = 0; Dev < 32; Dev++) {
      //
      // Read Vendor ID to check if device exists
      // if no device exists, then check next device
      //
      if (MmPci16 (0, Bus, Dev, 0, R_PCH_PCI_PCI_VID) == 0xFFFF) {
        continue;
      }

      Buffer16 = MmPci16 (0, Bus, Dev, 0, R_PCH_PCI_PCI_SCC);
      //
      // Clear Bus Number for PCI/PCI Bridge Device
      //
      if (Buffer16 == 0x0604) {
        MmPci32 (0, Bus, Dev, 0, R_PCH_PCI_PCI_PBN) = 0;
      }
    }
  }
  //
  // Clear bus numbers. PCIe retrain will use temporary bus number.
  //
/*  for (PortNo = 0; PortNo < PCH_PCIE_MAX_ROOT_PORTS; PortNo++) {
    //
    // Clear bus numbers so that PCIe slots are hidden
    //
    MmPci32 (0, 0, 0x1c, PortNo, R_PCH_PCI_PCI_PBN) = 0;
  }*/
}

UINT8
EnumerateDownstream (
  IN UINT8  BusNum
  )
/*++

Routine Description:

  This function enumerate all downstream bridge.

Arguments:

  BusNum  - Primary bus number of current bridge.

Returns:

  BusNum: return current bus number if current bus is an enpoint device.
  SubBus: return subordinate bus number if current bus is a bridge.

--*/
{
  UINT8   DevNum;
  UINT16  Buffer16;
  UINT8   SubBus;
  UINT8   SecBus;

  SubBus  = 0;

  SecBus  = BusNum;

  for (DevNum = 0; DevNum < 32; DevNum++) {
    //
    // Read Vendor ID to check if device exists
    // if no device exists, then check next device
    //
    if (MmPci16 (0, BusNum, DevNum, 0, R_PCH_PCI_PCI_VID) == 0xFFFF) {
      continue;
    }

    Buffer16 = MmPci16 (0, BusNum, DevNum, 0, R_PCH_PCI_PCI_SCC);
    //
    // Check for PCI/PCI Bridge Device Base Class 6 with subclass 4
    //
    if (Buffer16 == 0x0604) {
      SecBus++;
      MmPci8 (0, BusNum, DevNum, 0, R_PCH_PCI_PCI_PBN)  = BusNum;
      MmPci8 (0, BusNum, DevNum, 0, R_PCH_PCI_PCI_SCBN) = SecBus;
      //
      // Assign temporary subordinate bus number so that device behind this bridge can be seen
      //
      MmPci8 (0, BusNum, DevNum, 0, R_PCH_PCI_PCI_SBBN) = 0xff;

      //
      // A config write is required in order for the device to re-capture the Bus number,
      // according to PCI Express Base Specification, 2.2.6.2
      // Write to a read-only register VendorID to not cause any side effects.
      //
      MmPci16 (0, SecBus, 0, 0, PCI_VID) = 0;

      //
      // Enumerate bus behind this bridge by calling this funstion recursively
      //
      SubBus = EnumerateDownstream (SecBus);
      //
      // Update the correct subordinate bus number
      //
      MmPci8 (0, BusNum, DevNum, 0, R_PCH_PCI_PCI_SBBN) = SubBus;
      SecBus = SubBus;
    }
  }

  if (SubBus == 0) {
    return BusNum;
  } else {
    return SubBus;
  }
}
