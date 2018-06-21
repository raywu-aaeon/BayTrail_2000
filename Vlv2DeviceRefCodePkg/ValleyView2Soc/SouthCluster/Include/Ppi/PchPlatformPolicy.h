/**
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
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
  PchPlatformPolicy.h

  @brief 
  PCH policy PPI produced by a platform driver specifying various
  expected PCH settings. This PPI is consumed by the PCH PEI modules.

**/
#ifndef PCH_PLATFORM_POLICY_H_
#define PCH_PLATFORM_POLICY_H_
//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//


#include "PchRegs.h"

//
#define PCH_PLATFORM_POLICY_PPI_GUID \
  { \
    0x15344673, 0xd365, 0x4be2, 0x85, 0x13, 0x14, 0x97, 0xcc, 0x7, 0x61, 0x1d \
  }

extern EFI_GUID                         gPchPlatformPolicyPpiGuid;

///
/// Forward reference for ANSI C compatibility
///
typedef struct _PCH_PLATFORM_POLICY_PPI PCH_PLATFORM_POLICY_PPI;

///
/// PPI revision number
/// Any backwards compatible changes to this PPI will result in an update in the revision number
/// Major changes will require publication of a new PPI
///
/// Revision 1:    Original version
///
#define PCH_PLATFORM_POLICY_PPI_REVISION_1  1
#define PCH_PLATFORM_POLICY_PPI_REVISION_2  2
#define PCH_PLATFORM_POLICY_PPI_REVISION_3  3
#define PCH_PLATFORM_POLICY_PPI_REVISION_4  4
#define PCH_PLATFORM_POLICY_PPI_REVISION_5  5
//
// Generic definitions for device enabling/disabling used by PCH code.
//
#define PCH_DEVICE_ENABLE   1
#define PCH_DEVICE_DISABLE  0

typedef struct {
  UINT8  ThermalDataReportEnable  : 1;   // OBSOLETE from Revision 5 !!! DO NOT USE !!!
  UINT8  MchTempReadEnable        : 1;
  UINT8  PchTempReadEnable        : 1;
  UINT8  CpuEnergyReadEnable      : 1;
  UINT8  CpuTempReadEnable        : 1;
  UINT8  Cpu2TempReadEnable       : 1;
  UINT8  TsOnDimmEnable           : 1;
  UINT8  Dimm1TempReadEnable      : 1;

  UINT8  Dimm2TempReadEnable      : 1;
  UINT8  Dimm3TempReadEnable      : 1;
  UINT8  Dimm4TempReadEnable      : 1;
  UINT8  Rsvdbits                 : 5;
} PCH_THERMAL_REPORT_CONTROL;
//
// ---------------------------- HPET Config -----------------------------
//
typedef struct {
  BOOLEAN Enable; /// Determines if enable HPET function
  UINT32  Base;   /// The HPET base address
} PCH_HPET_CONFIG;


///
/// ---------------------------- SATA Config -----------------------------
///
typedef enum {
  PchSataModeIde,
  PchSataModeAhci,
  PchSataModeRaid,
  PchSataModeMax
} PCH_SATA_MODE;

///
/// ---------------------------- PCI Express Config -----------------------------
///
typedef enum {
  PchPcieAuto,
  PchPcieGen1,
  PchPcieGen2
} PCH_PCIE_SPEED;

typedef struct {
  PCH_PCIE_SPEED  PcieSpeed[PCH_PCIE_MAX_ROOT_PORTS];
} PCH_PCIE_CONFIG;

///
/// ---------------------------- IO APIC Config -----------------------------
///
typedef struct {
  UINT8 IoApicId;
} PCH_IOAPIC_CONFIG;

///
/// ------------ General PCH Platform Policy PPI definition ------------
///
struct _PCH_PLATFORM_POLICY_PPI {
  UINT8                         Revision;
  UINT8                         BusNumber;  // Bus Number of the PCH device
  UINT32                        SpiBase;    // SPI Base Address.
  UINT32                        PmcBase;    // PMC Base Address.
  UINT32                        SmbmBase;   // SMB Memory Base Address.
  UINT32                        IoBase;     // IO Base Address.
  UINT32                        IlbBase;    // Intel Legacy Block Base Address.
  UINT32                        PUnitBase;  // PUnit Base Address.
  UINT32                        Rcba;       // Root Complex Base Address.
  UINT32                        MphyBase;   // MPHY Base Address.
  UINT16                        AcpiBase;   // ACPI I/O Base address.
  UINT16                        GpioBase;   // GPIO Base address
  PCH_HPET_CONFIG               *HpetConfig;
  PCH_SATA_MODE                 SataMode;
  PCH_PCIE_CONFIG               *PcieConfig;
  PCH_IOAPIC_CONFIG             *IoApicConfig;
  BOOLEAN                       EhciPllCfgEnable;
  UINT8                   XhciWorkaroundSwSmiNumber;
  UINT8							SataOddPort; //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements
};

#endif
