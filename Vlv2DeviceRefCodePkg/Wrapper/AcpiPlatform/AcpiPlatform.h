/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiPlatform.h

Abstract:

  This is an implementation of the ACPI platform driver.  Requirements for
  this driver are defined in the Tiano ACPI External Product Specification,
  revision 0.3.6.


--*/

#ifndef _ACPI_PLATFORM_H_
#define _ACPI_PLATFORM_H_

//
// Statements that include other header files
//
#include <Protocol/PlatformCpu.h>
#include <base.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FirmwareVolume.h>
#include <Library/PcdLib.h>
#include <IndustryStandard/HighPrecisionEventTimerTable.h>
#include <IndustryStandard/Acpi.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/MpService.h>
#include <Protocol/CpuIo.h>
#include <Industrystandard/Acpi30.h>
#include <Industrystandard/Acpi20.h>
#include <library/HobLib.h>
#include <AlertStandardFormattable.h>
#include "CMOSMap.h"
#include <Protocol/GlobalNvsArea.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/DpsdRsa1024AndSha256SignatureVerification.h>
#undef EFI_EVENT_RUNTIME_CONTEXT
#include <Library/SbPolicy.h>
#include <Library/NbPolicy.h>
//
// Global variables
//
EFI_GLOBAL_NVS_AREA_PROTOCOL  mGlobalNvsArea;

//
// ACPI table information used to initialize tables.
//
// SLP 2.0 equates
#define SLP20_MAGIC_NUMBER        0x00000001
#define SLP20_VERIFIED_INDICATOR  0x91827364
#define EFI_ACPI_OEM_REVISION     0x00000003
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32 ('A', 'M', 'I', ' ')
#define EFI_ACPI_CREATOR_REVISION 0x0100000D

#define WPCN381U_CONFIG_INDEX     0x2E
#define WPCN381U_CONFIG_DATA      0x2F
#define WPCN381U_CHIP_ID          0xF4
#define WDCP376_CHIP_ID           0xF1

#define MOBILE_PLATFORM 1
#define DESKTOP_PLATFORM 2

//
// +++++++++++++++++++++++++++++++++++++++++++++++++++
// ESS - GV3 Definitions
// +++++++++++++++++++++++++++++++++++++++++++++++++++
//
#pragma pack(1)

typedef struct {
    UINT32 PlatformId;
    UINT8  IoApicEnable;
    UINT8  NMEN;		// Node Manager Enable (aka NPTM)
    UINT8  TpmEnable      :1;
    UINT8  CStateEnable   :1;
    UINT8  OSC2Report     :1;
    UINT8  C6Enable       :1;
    UINT8  C7Enable       :1;
    UINT8  MonitorMwaitEnable :1;
    UINT8  PStateEnable   :1;
    UINT8  Rsvd           :1;
    UINT8  PSDT           :2;     // P-State Dependency Type
    UINT8  StandbyToS1    :1;
    UINT8  KBPresent      :1;
    UINT8  MousePresent   :1;
    UINT8  DualIohPresent :1;
    UINT8  DummyBits      :2;
    UINT32 CpuIdInfo; // Used in PciCrs.asl to determine Havendale/Lynnfield Processor
    UINT32 Tolm;      // Used in PciCrs.asl
    UINT16 OsSelect;              // O/S Type. This is for ASL only.
    UINT32 IgdOpRegionAddress;    // IGD OpRegion Starting Address
    UINT8  IgdBootType;           // IGD Boot Display Device
    UINT8  IgdPanelType;          // IGD Panel Type CMOs option
    UINT8  IgdTvFormat;           // IGD TV Format CMOS option
    UINT8  IgdTvMinor;            // IGD TV Minor Format CMOS option
    UINT8  IgdPanelScaling;       // IGD Panel Scaling
    UINT8  IgdBlcConfig;          // IGD BLC Configuration
    UINT8  IgdBiaConfig;          // IGD BIA Configuration
    UINT8  IgdSscConfig;          // IGD SSC Configuration
    UINT8  IgdDvmtMemSize;        // IGD DVMT Memory Size
    UINT8  IgdFunc1Enable;        // IGD Function 1 Enable
    UINT8  IgdHpllVco;            // HPLL VCO
    UINT8  IgdSciSmiMode;         // GMCH SMI/SCI mode (0=SCI)
    UINT8  IgdPAVP;               // IGD PAVP data
} BIOS_ACPI_PARAM;

typedef struct _AML_GV3_STATE_PACKAGE {
    UINT8   PkgOp;                // 0x12
    UINT8   PkgLeadByte;
    UINT8   NoData;
    UINT8   DWPrefix0;
    UINT32  CoreFreq;
    UINT8   DWPrefix1;
    UINT32  PowerConsumption;
    UINT8   DWPrefix2;
    UINT32  TransactionLatency;
    UINT8   DWPrefix3;
    UINT32  BusMasterLatency;
    UINT8   DWPrefix4;
    UINT32  ControlValue;
    UINT8   DWPrefix5;
    UINT32  StatusValue;
} AML_GV3_STATE_PACKAGE;

typedef struct _AML_PSS_PACKAGE {
    UINT8                 NameOp; // 0x08
    UINT8                 Sign[4];
    UINT8                 PkgOp;  // 0x12
    UINT8                 Res1[2];
    UINT8                 NoOfGV3Packages;
    AML_GV3_STATE_PACKAGE Pstates[16];
} AML_PSS_PACKAGE;

typedef struct _SMM_APICID_MAP {
    UINT32  ApicID;
    UINT16  PhysicalCpuNumber;
    UINT16  CpuNumber;
    UINT16  Version;
} SMM_APICID_MAP;

typedef struct _SMM_GV3_FVID {
    UINT16  VidValue : 8;
    UINT16  FreqRatio : 8;
    UINT16  SystemBusFreq : 16;
} SMM_GV3_FVID;

#pragma pack()

#define GV3_SSDT_OEM_TABLE_IDBASE 0x4000

//
// Private Driver Data
//
//
// Define Union of IO APIC & Local APIC structure;
//
typedef union {
    EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE AcpiLocalApic;
    EFI_ACPI_2_0_IO_APIC_STRUCTURE              AcpiIoApic;
    struct {
        UINT8 Type;
        UINT8 Length;
    } AcpiApicCommon;
} ACPI_APIC_STRUCTURE_PTR;

//
// Protocol private structure definition
//
EFI_STATUS
InstallAcpiPlatform(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
/*++

Routine Description:

  Entry point of the ACPI platform driver.


Arguments:

  ImageHandle  -  EFI_HANDLE: A handle for the image that is initializing this driver.
  SystemTable  -  EFI_SYSTEM_TABLE: A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
;

EFI_ACPI_TABLE_VERSION
GetAcpiTableVersion(
    VOID
)
/*++

Routine Description:

  Get Acpi Table Version.

Arguments:

  ImageHandle   EFI_HANDLE: A handle for the image that is initializing this driver
  SystemTable   EFI_SYSTEM_TABLE: A pointer to the EFI system table

Returns:

  EFI_SUCCESS:              Driver initialized successfully
  EFI_LOAD_ERROR:           Failed to Initialize or has been loaded
  EFI_OUT_OF_RESOURCES:     Could not allocate needed resources

--*/
;

EFI_STATUS
AcpiPlatformGetOemFields(
    OUT UINT8   *OemId,
    OUT UINT64  *OemTableId,
    OUT UINT32  *OemRevision
)
/*++

Routine Description:

  The funtion returns Oem specific information of Acpi Platform.

Arguments:

  OemId        -  OemId returned.
  OemTableId   -  OemTableId returned.
  OemRevision  -  OemRevision returned.

Returns:

  EFI_STATUS  -  Status of function execution.

--*/
;

EFI_ACPI_TABLE_VERSION
AcpiPlatformGetAcpiSetting(
    VOID
)
/*++

Routine Description:

  The function returns Acpi table version.

Arguments:

  None.

Returns:

  EFI_ACPI_TABLE_VERSION  -  Acpi table version encoded as a UINT32.

--*/
;

EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
/*++

Routine Description:

  Entry point for Acpi platform driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
;

VOID *
EFIAPI
SetMem(
    OUT VOID  *Buffer,
    IN UINTN  Length,
    IN UINT8  Value
);

VOID *
EFIAPI
CopyMem(
    OUT VOID       *DestinationBuffer,
    IN CONST VOID  *SourceBuffer,
    IN UINTN       Length
);

VOID *
EFIAPI
ZeroMem(
    OUT VOID  *Buffer,
    IN UINTN  Length
);

#endif
