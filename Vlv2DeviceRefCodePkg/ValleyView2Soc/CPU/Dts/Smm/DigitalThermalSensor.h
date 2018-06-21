/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DigitalThermalSensor.h

Abstract:

  Defines and prototypes for the Digital Thermal Sensor SMM driver

--*/

#ifndef _DIGITAL_THERMAL_SENSOR_H_
#define _DIGITAL_THERMAL_SENSOR_H_

//
// Include files
//
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <ArchProtocol/Cpu/Cpu.h>
#include <Protocol/SmmSxDispatch/SmmSxDispatch.h>
#else
#include <Protocol/Cpu.h>
#endif
//#include <Guid/SetupVariable.h>    // AMI_OVERRIDE
#include <Library/SmmIoLib.h>
#ifndef ECP_FLAG
#include <Protocol/SmmSxDispatch.h>
#endif
#include <Protocol/SmmSwDispatch.h>
#include "DTSVariable.h"
#include "PchAccess.h"
#include <Library/BaseLib.h>    // AMI_OVERRIDE


#define   PLATFORM_SETUP_VARIABLE_NAME                L"Setup"    // AMI_OVERRIDE
//
// CPUID definitions
//
#define CPUID_EXT_FAMILY                  0x0FF00000
#define CPUID_EXT_MODEL                   0x000F0000
#define CPUID_FAMILY                      0x00000F00
#define CPUID_MODEL                       0x000000F0
#define CPUID_STEPPING                    0x0000000F
#define CPUID_FAMILY_MODEL                0x00000FF0
#define CPUID_FAMILY_MODEL_STEPPING       0x00000FFF
#define CPUID_FULL_FAMILY_MODEL           0x0FFF0FF0
#define CPUID_FULL_FAMILY_MODEL_STEPPING  0x0FFF0FFF

#define CPUID_FULL_FAMILY_MODEL_PENRYN    0x00010670

#define CPUID_FAMILY_MODEL_STEP_YONAH_A1  0x000006E1
#define CPUID_FAMILY_MODEL_STEP_YONAH_B0  0x000006E4

//
// Maximum temperature (usually 100 degrees Celsius) used with offsets.
//
#define TJ_MAX                                110
#define TJ_MAX_105                        105
#define TJ_MAX_100                        100
#define TJ_MAX_90                         90
#define TJ_MAX_85                         85
#define DTS_CRITICAL_TEMPERATURE          255

#define DTS_SAMPLE_RATE                   0x10

#define DTS_HYBRID_ENABLE_FOR_PENRYN      BIT6

#define MSR_EXT_CONFIG                    0xEE
  #define TJ_MAX_INDICATOR                 (1 << 30)

#define EFI_MSR_DTS_CAL_CTRL              0x15F

#define EFI_MSR_IA32_THERM_INTERRUPT      0x19B
  #define TH1_VALUE                         8
  #define TH1_ENABLE                        (1 << 15)
  #define TH2_VALUE                         16
  #define TH2_ENABLE                        (1 << 23)
  #define OFFSET_MASK                       (0x7F)
  #define OVERHEAT_INTERRUPT_ENABLE         (1 << 4)


//#define EFI_MSR_IA32_THERM_STATUS       0x19C
  #define B_OUT_OF_SPEC_STATUS              (1 << 4)
  #define B_OUT_OF_SPEC_STATUS_LOG          (1 << 5)
  #define B_THERMAL_THRESHOLD_1_STATUS      (1 << 6)
  #define B_THERMAL_THRESHOLD_1_STATUS_LOG  (1 << 7)
  #define B_THERMAL_THRESHOLD_2_STATUS      (1 << 8)
  #define B_THERMAL_THRESHOLD_2_STATUS_LOG  (1 << 9)
  #define B_READING_VALID                   (1 << 31)

#define EFI_MSR_PIC_SENS_CFG              0x1AA
  #define B_LOCK_THERMAL_INT                (1 << 22)
  #define B_BYPASS_FILTER                   (1 << 4)

#define THERM_STATUS_LOG_MASK             (B_THERMAL_THRESHOLD_2_STATUS_LOG | B_THERMAL_THRESHOLD_1_STATUS_LOG | B_OUT_OF_SPEC_STATUS_LOG)
#define THERM_STATUS_THRESHOLD_LOG_MASK   (B_THERMAL_THRESHOLD_2_STATUS_LOG | B_THERMAL_THRESHOLD_1_STATUS_LOG)


#define B_DTS_IO_TRAP                     (1 << 2)
#define R_ACPI_GPE_CNTL                   0x42                 // ACPI PM IO register 42h
#define R_ACPI_SMI_EN                     0x30                 // ACPI PM IO register 30h
#define B_SWGPE_CTRL                      (1 << 1)
#define DTS_IO_TRAP_REGISTER_LOW_DWORD    (0x00040001 + ICH_DTS_IO_TRAP_BASE_ADDRESS)           // DigitalThermalSensor IO Trap High DWord value
#define DTS_IO_TRAP_REGISTER_HIGH_DWORD   0x000200F0                                            // DigitalThermalSensor IO Trap High DWord value
#define LOCAL_APIC_THERMAL_DEF            0xFEE00330
  #define B_INTERRUPT_MASK                  (1 << 16)
  #define B_DELIVERY_MODE                   (0x07 << 8)
  #define V_MODE_SMI                        (0x02 << 8)
  #define B_VECTOR                          (0xFF << 0)

#define DTS_NUMBER_THRESHOLD_RANGES               9            // How many ranges are in the threshold table
#define PENRYN_DTS_NUMBER_THRESHOLD_RANGES        6            // How many level in DigitalThermalSensor Threshold table
#define ATOM_DTS_NUMBER_THRESHOLD_RANGES         16            // How many level in DigitalThermalSensor Threshold table
#define INIT_AP_DTS_FUNCTION              10                   // Enable AP DigitalThermalSensor function
#define INIT_DTS_FUNCTION_AFTER_S3        20                   // Enable Digital Thermal Sensor function after resume from S3
#define DISABLE_UPDATE_DTS_EVERY_SMI      30                   // Disable update DTS temperature and threshold value in every SMI

#define INIT_DTS_SCF_MIN                  0x10                 // SCF Minimum value.
#define INIT_DTS_SCF_UNITY                0x20                 // SCF Unity Value.
#define INIT_DTS_SCF_MAX                  0x30                 // SCF Maximum value.
#define UPDATE_DTS_EVERY_SMI              TRUE                 // Update DTS temperature and threshold value in every SMI

//
// Enumerate a DTS event type
//
typedef enum {
  DtsEventNone,
  DtsEventThreshold,
  DtsEventOutOfSpec,
  DtsEventMax
} DTS_EVENT_TYPE;


//
// Function declarations
//

#ifdef ECP_FLAG
EFI_STATUS
DtsSmiCallback(
  IN EFI_HANDLE             SmmImageHandle,
  IN OUT VOID               *CommunicationBuffer,
  IN OUT UINTN              *SourceSize
  );
#else
EFI_STATUS
DtsSmiCallback (
  IN EFI_HANDLE             SmmImageHandle,
  IN     CONST VOID        *ContextData,        OPTIONAL
  IN OUT VOID               *CommunicationBuffer,    OPTIONAL
  IN OUT UINTN              *SourceSize    OPTIONAL
  );
#endif


EFI_STATUS
DigitalThermalSensorInit (
  VOID
  );

VOID
DigitalThermalSensorEnable (
  VOID                      *Buffer
  );

VOID
DigitalThermalSensorSetSwGpeSts (
  VOID
  );

VOID
DigitalThermalSensorEventCheckMsr (
  IN  VOID                  *Buffer
  );

BOOLEAN
DigitalThermalSensorEventCheck (
  DTS_EVENT_TYPE            *EventType
  );

VOID
DigitalThermalSensorSetThreshold (
  VOID                      *Buffer
  );

VOID
DigitalThermalSensorEnableSmi (
  VOID                      *Buffer
  );

VOID
DigitalThermalSensorDisableSmi (
  VOID                      *Buffer
  );

EFI_STATUS
InstallDigitalThermalSensor (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  );

EFI_STATUS
EnableDtsInterrupts (
  VOID                      *Buffer
  );

EFI_STATUS
DisableDtsInterrupts (
  VOID
  );

STATIC
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE   Procedure,
  IN OUT VOID               *Buffer
  );

EFI_STATUS
ThresholdTableInit (
  VOID
  );

VOID
DtsS3EntryCallBack (
  IN  EFI_HANDLE                    Handle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT   *Context
  );

#endif
