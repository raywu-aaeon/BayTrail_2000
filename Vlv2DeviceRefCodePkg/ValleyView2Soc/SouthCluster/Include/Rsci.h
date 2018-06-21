/*++
Copyright (c) 1996 - 20013, Intel Corporation.

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.


Module Name:



Abstract:



--*/

#ifndef _RSCI_H
#define _RSCI_H

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>


//
// Definitions
//
#define EFI_ACPI_OEM_RSCI_REVISION                      0x00000000
#define EFI_ACPI_RESET_SHUTDOWN_COMMUNICATION_INTERFACE_SIGNATURE  SIGNATURE_32('R', 'S', 'C', 'I')
//
// RSCI structure
//
//
// Ensure proper structure formats
//

#pragma pack(1)

typedef enum {
 FBR_NONE = 0,
 FBR_WATCHDOG_COUNTER_EXCEEDED,
 FBR_NO_MATCHING_OS,

 FBR_INVALID_OSNIB_CHECKSUM,
} FALL_BACK_POLICY_REASON;

typedef struct {
  FALL_BACK_POLICY_REASON     FallbackPolicyReason;
  UINT8                       FastbootCombo;
  UINT8                       Reserved[2];
} EFI_ACPI_RSCI_INDICATORS;



//
//Android Wake Sources for S4/S5
//
typedef enum {
 WAKE_NOT_APPLICABLE = 0,
 WAKE_BATTERY_INSERTED,
 WAKE_USB_CHARGER_INSERTED,
 WAKE_ACDC_CHARGER_INSERTED,
 WAKE_POWER_BUTTON_PRESSED,
 WAKE_RTC_TIMER,
 WAKE_BATTERY_REACHED_IA_THRESHOLD
} ANDROID_WAKE_SOURCE;

//
//Android Reset Sources
//
typedef enum {
  RESET_NOT_APPLICABLE = 0,
  RESET_OS_INITIATED,
  RESET_FORCED,
  RESET_FW_UPDATE,
  RESET_KERNEL_WATCHDOG,
  RESET_SECURITY_WATCHDOG,
  RESET_SECURITY_INITIATED,
  RESET_PMC_WATCHDOG,
  RESET_EC_WATCHDOG,
  RESET_PLATFORM_WATCHDOG,
} ANDROID_RESET_SOURCE;

typedef enum {
  NOT_APPLICABLE_RESET = 0,
  WARM_RESET = 1,
  COLD_RESET = 2,
  GLOBAL_RESET = 7,
}ANDROID_RESET_TYPE;

//
//Android Shutdown sources
//
typedef enum {
  SHTDWN_NOT_APPLICABLE = 0,
  SHTDWN_POWER_BUTTON_OVERRIDE,
  SHTDWN_BATTERY_REMOVAL,
  SHTDWN_VCRIT,
  SHTDWN_THERMTRIP,
  SHTDWN_PMICTEMP,
  SHTDWN_SYSTEMP,
  SHTDWN_BATTEMP,
  SHTDWN_SYSUVP,
  SHTDWN_SYSOVP,
  SHTDWN_SECURITY_WATCHDOG,
  SHTDWN_SECURITY_INITIATED,
  SHTDWN_PMC_WATCHDOG,
  SHTDWN_EC_WATCHDOG,
  SHTDWN_PLATFORM_WATCHDOG,
  SHTDWN_KERNEL_WATCHDOG
}ANDROID_SHUTDOWN_SOURCE;


typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT8                       WakeSrc;
  UINT8                       Resets;
  UINT8                       ResetType;
  UINT8                       ShuntdownSrc;
  EFI_ACPI_RSCI_INDICATORS    Indicators;
} EFI_ACPI_RESET_SHUTDOWN_COMMUNICATION_INTERFACE;

#pragma pack()

#endif
