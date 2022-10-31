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
  PchInit.h

  @brief
  Header file for PCH Initialization Driver.

**/
#ifndef _PCH_INITIALIZATION_DRIVER_H_
#define _PCH_INITIALIZATION_DRIVER_H_
#define HDA_4683125_WORKAROUND 1

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "EfiScriptLib.h"
#else
#include <Library/UefiLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#endif

#include "PlatformBaseAddresses.h"
#include "../Common/PchUsbCommon.h"
#include "VlvAccess.h"
#include <Library/PchPciExpressHelpersLib.h>
#include <Protocol/PchPlatformPolicy.h>
#ifdef ECP_FLAG
#include <Protocol/BootScriptSave/BootScriptSave.h>
#else
#include <Protocol/BootScriptSave.h>
#endif
#include <Protocol/PchS3Support.h>
#include <Protocol/PchInfo.h>

#include <Library/PchPlatformLib.h>
#ifdef ECP_FLAG
#include <Protocol/ExitPmAuth/ExitPmAuth.h>
#else
#include <Protocol/ExitPmAuth.h>
#endif
#include <Library/PchAslUpdateLib.h>

#define AZALIA_MAX_LOOP_TIME  10
#define AZALIA_WAIT_PERIOD    100
#define AZALIA_MAX_SID_NUMBER 4
#define AZALIA_MAX_SID_MASK   ((1 << AZALIA_MAX_SID_NUMBER) - 1)

typedef struct {
  EFI_PCH_INFO_PROTOCOL PchInfo;
} PCH_INSTANCE_PRIVATE_DATA;

#define IS_PCH_EHCI(DeviceNumber, FunctionNumber) \
    ( \
      (DeviceNumber == PCI_DEVICE_NUMBER_PCH_USB && FunctionNumber == PCI_FUNCTION_NUMBER_PCH_EHCI) \
    )

///
/// Data definitions
///
extern EFI_HANDLE mImageHandle;

///
/// SVID / SID init table entry
///
typedef struct {
  UINT8 DeviceNumber;
  UINT8 FunctionNumber;
  UINT8 SvidRegOffset;
} PCH_SVID_SID_INIT_ENTRY;

///
///  IRQ routing init table entry
///
typedef struct {
  UINTN BusNumber;
  UINTN DeviceNumber;
  UINTN FunctionNumber;
  UINTN Irq;
} PCH_IRQ_INIT_ENTRY;

///
/// Function Prototype
///
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
;
VOID
HDA_4683125_WA (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
;

EFI_STATUS
ConfigureMiscPm (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINT32                              PmcBase,
  UINT16                              GpioBase
  )
/**

  @brief
  Configure miscellaneous power management settings

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] PmcBase              PmcBase value of this PCH device
  @param[in] GpioBase             GPIO base address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureAdditionalPm (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Configure additional power management settings

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ProgramDeepSx (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINT32                              RootComplexBar
  )
/**

  @brief
  Configure deep Sx programming

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureS0ix (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              PmcBase
  )
/**

  @brief
  Configure S0ix settings

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] PmcBase              PmcBase value of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
;
VOID
ConfigureAcpiHwRed (
  IN  UINT16                                    AcpiBase
  )
/**

  @brief
  Configure for platforms with Acpi Hardware Reduced Mode enabled

  @param[in] AcpiBase             The ACPI I/O Base address of the PCH

  @retval EFI_SUCCESS             None

**/
;

EFI_STATUS
ConfigureMiscItems (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN      UINT32                              PmcBase,
  IN      UINT32                              IlbBase,
  IN OUT  UINT32                              *FuncDisableReg
  )
/**

  @brief
  Perform miscellany PCH initialization

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] PmcBase              PmcBase value of this PCH device
  @param[in] IlbBase              IlbBase value of this PCH device
  @param[in] FuncDisableReg       The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureLan (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                              PmcBase,
  IN     UINT32                              SpiBase,
  IN OUT UINT32                              *FuncDisableReg
  )
/**

  @brief
  Initialize LAN device. Reference: PCH BIOS Spec Rev 0.5.0,

  ** NOTE:
  in PchInit PEIM. Platform PEI code is responsible for calling PCH Init PPI
    - (BUC register setting is also done in the PCH Init PPI)

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] PmcBase              PmcBase address of this PCH device
  @param[in] SpiBase              SpiBase address of this PCH device
  @param[in] FuncDisableReg       The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureOtgAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  Hide PCI config space of OTG device and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureOtg (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure OTG device.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
;

EFI_STATUS
ConfigureLpeAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  Hide PCI config space of LPE device and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
;

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
;

EFI_STATUS
ConfigureLpioAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  Hide PCI config space of LPIO devices and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
InitializeLpe (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/*++

Routine Description:

  Initialize LPE devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
;

EFI_STATUS
ConfigureLpssAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  );

EFI_STATUS
ConfigureLpss (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure LPSS devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
;

EFI_STATUS
ConfigureSccAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  Hide PCI config space of SCC devices and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
;


EFI_STATUS
InitializeLpss (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/*++

Routine Description:

  Initialize LPSS devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
;

EFI_STATUS
ConfigureScc (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure SCC devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
;

EFI_STATUS
InitializeScc (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/*++

Routine Description:

  Initialize SCC devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
;

EFI_STATUS
ConfigureUsb (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  UINT32                              *FuncDisableReg
  )
/**

  @brief
  Configures PCH USB controller

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] FuncDisableReg       Function Disable Register

  @retval EFI_INVALID_PARAMETER   The parameter of PchPlatformPolicy is invalid
  @retval EFI_SUCCESS             The function completed successfully

**/
;
EFI_STATUS
ConfigureSata (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  UINT32                              *FuncDisableReg,
  IN      UINT16                              GpioBase
  )
/**

  @brief
  Configures PCH Sata Controller

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] FuncDisableReg       Function Disable Register
  @param[in] GpioBase             GPIO base address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureClockGating (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              RootComplexBar,
  IN  UINT32                              PmcBase,
  IN  UINT32                              SpiBase,
  IN  UINT32                              FuncDisableReg
  )
/**

  @brief
  Perform Clock Gating programming
  Enables clock gating in various PCH interfaces and the registers must be restored during S3 resume.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] PmcBase              PmcBase value of this PCH device
  @param[in] SpiBase              SpiBase value of this PCH device
  @param[in] FuncDisableReg       The Function Disable Register

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
ConfigureIoApic (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              IlbBase
  )
/**

  @brief
  Configure IoApic Controler

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] IlbBase              IlbBase address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
PchInitSingleRootPort (
  IN  UINT8                                     RootPort,
  IN  UINT8                                     RootPortFunction,
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL          *PchPlatformPolicy,
  IN  UINT16                                    AcpiBase,
  IN  UINT32                                    RootComplexBar,
  IN  UINT32                                    PmcBase
  )
/**

  @brief
  Perform Root Port Initialization.

  @param[in] RootPort             The root port to be initialized (zero based)
  @param[in] RootPortFunction     The PCI function number of the root port
  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol
  @param[in] AcpiBase             The ACPI I/O Base address of the PCH
  @param[in] RootComplexBar       RCBA of the PCH
  @param[in] PmcBase              PmcBase of the PCH

  @retval EFI_SUCCESS             Device found. The root port must be enabled.
  @retval EFI_NOT_FOUND           No device is found on the root port. It may be disabled.
  @exception EFI_UNSUPPORTED      Unsupported operation.

**/
;

EFI_STATUS
PchInitRootPorts (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL      *PchPlatformPolicy,
  IN      UINT32                                RootComplexBar,
  IN      UINT32                                PmcBase,
  IN      UINT16                                AcpiBase,
  IN OUT  UINT32                                *FuncDisableReg
  )
/**

  @brief
  Perform Initialization of the Downstream Root Ports.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol
  @param[in] RootComplexBar       RCBA of the PCH
  @param[in] PmcBase              PmcBase of the PCH
  @param[in] AcpiBase             The ACPI I/O Base address of the PCH
  @param[in] FuncDisableReg       The function disable register. IN / OUT parameter.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_INVALID_PARAMETER   The PCIe Root Port Number of D28:F0 is not found
                                  or invalid

**/
;

EFI_STATUS
PcieEnableClockGating (
  IN  UINT8                                     BusNumber,
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL          *PchPlatformPolicy,
  IN  UINT32                                    RpEnableMask,
  IN  UINT32                                    RpHiddenMask
  )
/**

  @brief
  This is the function to enable the clock gating for PCI Express ports.

  @param[in] BusNumber            The Bus Number of the PCH device
  @param[in] PchPlatformPolicy    PCH Platform Policy protocol
  @param[in] RpEnableMask         Bit Mask indicating the enabled root ports
  @param[in] RpHiddenMask         Bit Mask indicating the root ports used for other > x1 root ports

  @retval EFI_SUCCESS             Successfully completed.

**/
;

EFI_STATUS
SetInitRootPortDownstreamS3Item (
  IN  UINT8                           RootPortBus,
  IN  UINT8                           RootPortDevice,
  IN  UINT8                           RootPortFunc,
  IN  UINT8                           TempBusNumberMin,
  IN  UINT8                           TempBusNumberMax
  )
/**

  @brief
  Set an Init Root Port Downstream devices S3 dispatch item, this function may assert if any error happend

  @param[in] RootPortBus          Pci Bus Number of the root port
  @param[in] RootPortDevice       Pci Device Number of the root port
  @param[in] RootPortFunc         Pci Function Number of the root port
  @param[in] TempBusNumberMin     Minimal temp bus number that can be assigned to the root port (as secondary
                                  bus number) and its down stream switches
  @param[in] TempBusNumberMax     Maximal temp bus number that can be assigned to the root port (as subordinate
                                  bus number) and its down stream switches

  @retval EFI_SUCCESS             The function completed successfully

**/
;

VOID
PchDumpPlatformProtocol (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Dump whole DXE_PCH_PLATFORM_POLICY_PROTOCOL and serial out.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval None

**/
;

VOID
ConfigureXhciAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Configures ports of the PCH USB3 (xHCI) controller
  just before OS boot.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_INVALID_PARAMETER   The parameter of PchPlatformPolicy is invalid
  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
PmTimerStallS3Item (
  IN   UINT32                    DelayTime
  );

//AMI OVERRIDE - EIP132398 Update code for S3 resume issues on EMMC, I2C and LPE devices.>>
VOID
S3BootScriptPchMsgBus32Write (
    UINT32    PortId,
    UINT32    Register,
    UINT32    Dbuff,
    UINT32    ReadOpCode,
    UINT32    WriteOpCode
  );
//AMI OVERRIDE - EIP132398 Update code for S3 resume issues on EMMC, I2C and LPE devices.<<
#endif
