/*++

Copyright (c)  1999 - 2009, Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Platform.h

Abstract:

  Pinetrail platform specific information

--*/

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "ChipsetAccess.h"
#include "PlatformBaseAddresses.h"
#include <token.h>


//
// Number of P & T states supported
//
#define NPTM_P_STATES_SUPPORTED         16
#define NPTM_T_STATES_SUPPORTED         8

//
// I/O APIC IDs, the code uses math to generate the numbers
// instead of using these defines.
//
#define ICH_IOAPIC                     (1 << 0)
#define ICH_IOAPIC_ID                   0x08

//
// Possible SMBus addresses that will be present
//
#define SMBUS_ADDR_CH_A_1     DIMM1_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_A_2     DIMM2_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_B_1     DIMM3_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_B_2     DIMM4_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_C_1     DIMM5_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_C_2     DIMM6_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_D_1     DIMM7_SMBUS_ADDRESS
#define SMBUS_ADDR_CH_D_2     DIMM8_SMBUS_ADDRESS
#define SMBUS_ADDR_HOST_CLK_BUFFER  0xDC
#define SMBUS_ADDR_ICH_SLAVE        0x44
#define SMBUS_ADDR_HECETA     0x5C
#define SMBUS_ADDR_SMBARP     0xC2
#define SMBUS_ADDR_82573E     0xC6
#define SMBUS_ADDR_CLKCHIP    0xD2
#define SMBUS_ADDR_BRD_REV          0x4E
#define SMBUS_ADDR_DB803            0x82

//
// SMBus addresses that used on this platform
//
#define PLATFORM_SMBUS_RSVD_ADDRESSES { \
  SMBUS_ADDR_CH_A_1, \
  SMBUS_ADDR_CH_A_2, \
  SMBUS_ADDR_HOST_CLK_BUFFER, \
  SMBUS_ADDR_ICH_SLAVE, \
  SMBUS_ADDR_SMBARP, \
  SMBUS_ADDR_CLKCHIP, \
  SMBUS_ADDR_BRD_REV, \
  SMBUS_ADDR_DB803 \
  }
//
// Count of addresses present in PLATFORM_SMBUS_RSVD_ADDRESSES
//
#define PLATFORM_NUM_SMBUS_RSVD_ADDRESSES 8

//
// CMOS usage
//
#define CMOS_CPU_BSP_SELECT         0x10
#define CMOS_CPU_UP_MODE            0x11
#define CMOS_CPU_RATIO_OFFSET       0x12
#define CMOS_CPU_CORE_HT_OFFSET     0x13
#define CMOS_EFI_DEBUG              0x14
#define CMOS_CPU_BIST_OFFSET        0x15
#define CMOS_CPU_VMX_OFFSET         0x16
#define CMOS_ICH_PORT80_OFFSET      0x17
#define CMOS_PLATFORM_DESIGNATOR    0x18      // Second bank CMOS location of Platform ID
#define CMOS_VALIDATION_TEST_BYTE   0x19      // BIT0 - Validation mailbox for UPonDP
#define CMOS_SERIAL_BAUD_RATE       0x1A      // 0=115200; 1=57600; 2=38400; 3=19200; 4=9600
#define CMOS_DCU_MODE_OFFSET        0x1B
#define CMOS_VR11_SET_OFFSET        0x1C
#define CMOS_SBSP_TO_AP_COMM        0x20      // SEC code use ONLY!!!
#define TCG_CMOS_AREA_OFFSET        0x60      // Also Change in Universal\Security\Tpm\PhysicalPresence\Dxe\PhysicalPresence.c &
// Also Change in Platform\IntelEpg\Thurley\Dxe\AcpiTables\Dsdt\Tpm.asi
#define TCG_CMOS_MOR_AREA_OFFSET    (TCG_CMOS_AREA_OFFSET + 0x05) // Also Change in Platform\IntelEpg\Thurley\Dxe\AcpiTables\Dsdt\Tpm.asi

//
// GPIO Index Data Structure
//
typedef struct {
    UINT8   Register;
    UINT32  Value;
} ICH_GPIO_DEV;

//
// CPU Equates
//
#define MAX_THREAD                      2
#define MAX_CORE                        1
#define MAX_DIE                         2
#define MAX_CPU_SOCKET                  1
#define MAX_CPU_NUM                     (MAX_THREAD * MAX_CORE * MAX_DIE * MAX_CPU_SOCKET)

#define MEM64_LEN                       0x00100000000
#define RES_MEM64_36_BASE               0x01000000000 - MEM64_LEN   // 2^36
#define RES_MEM64_36_LIMIT              0x01000000000 - 1           // 2^36
#define RES_MEM64_39_BASE               0x08000000000 - MEM64_LEN   // 2^39
#define RES_MEM64_39_LIMIT              0x08000000000 - 1           // 2^39
#define RES_MEM64_40_BASE               0x10000000000 - MEM64_LEN   // 2^40
#define RES_MEM64_40_LIMIT              0x10000000000 - 1           // 2^40

//
// LT Equates
//
#ifdef LT_FLAG
#define ACM_BASE                      AUTHENTICATED_CODE_BASE_ADDR
#define ACM_SIZE                      UTHENTICATED_CODE_SIZE
#endif

#endif
