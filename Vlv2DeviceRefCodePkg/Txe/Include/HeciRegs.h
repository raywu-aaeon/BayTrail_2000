/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  HeciRegs.h

Abstract:

  Register Definitions for HECI

--*/
#ifndef _HECI_REGS_H
#define _HECI_REGS_H

#define HECI_BUS                        SEC_BUS
#define HECI_DEV                        SEC_DEVICE_NUMBER
#define HECI_FUN                        HECI_FUNCTION_NUMBER
#define HECI_PCI_ADDR                   (HECI_BUS << 24) | (HECI_DEV << 16) | (HECI_FUN << 8)
#define REVID_MSK                       0x000000FF

#define BRNGUP_HMRFPO_DISABLE_CMD_MASK  0xF0000000
#define BRNGUP_HMRFPO_DISABLE_CMD       0x30000000
#define BRNGUP_HMRFPO_DISABLE_OVR_MASK  0xF0000000
#define BRNGUP_HMRFPO_DISABLE_OVR_RSP   0x30000000

//
// HECI PCI register definition
//
#define R_VENDORID             0x00
#define R_DEVICEID             0x02
#define R_COMMAND              0x04
#define B_BME                  0x04
#define B_MSE                  0x02
#define R_REVID                0x08
#define R_HECIMBAR0            0x10
#define R_HECIMBAR1            0x14
#define R_SEC_FW_STS0          0x40
#define R_SEC_FW_STS1          0x48
#define R_MFS_CORRUPTION_INFO  0x4C
#define R_GEN_STS              0x60 //0x4C
#define R_HIDM                 0xA0

//
// HECIMBAR register definition
//

// BAR1 registers
#define H_ALIVENESS_REQ             0x214C
#define H_ALIVENESS_ACK             0x2044
#define H_SEC_IPC_READINESS         0x2040
#define S_HOST_IPC_READINESS        0x2150
#define H_HHISR                     0x2020
#define S_SEC_IPC_OUTPUT_STATUS     0x2154
#define H_HISR                      0x2060
#define SEC_IPC_OUTPUT_DRBELL       0x2048
#define IPC_OUTPUT_PAYLOAD          0x20c0
// BAR0 registers
#define SEC_IPC_INPUT_DRBELL        0x80400
#define SEC_IPC_INPUT_STS           0x80408
#define SEC_IPC_OUTPUT_STS          0x8040C
#define IPC_INPUT_PAYLOAD           0x80500

// fTPM
#define R_SATT_PTT_CTRL                0x00D0
#define B_PTT_DISABLED                 BIT13
#define R_SATT_PTT_SAP_SIZE            0x00D8
#define R_SATT_PTT_BRG_BA_LSB          0x00D4
#define B_ENTRY_VLD                    BIT0
#define PTT_CMD_BUFFER_OFFSET          0x80
#define PTT_ICR                        0x8c
#define SEC_PTT_SAP_SIZE               0x1000

#define H_CB_WW    0x00
#define H_CSR      0x04
#define SEC_CB_RW  0x08
#define SEC_CSR_HA 0x0C

//
// PCH related registers address
//
#define PCH_ACPI_TIMER_MAX_VALUE  0x1000000 // The timer is 24 bit overflow
//
// HPET Information
//
#define HPET_ADDRESS              0xFED00000

//
// HPET Registers will be used as DWORD index
//
#define HPET_CAPS_REG_LOW       0x00 / 4
#define HPET_CAPS_REG_HIGH      0x04 / 4
#define HPET_GEN_CONFIG_LOW     0x10 / 4
#define HPET_GEN_CONFIG_HIGH    0x14 / 4
#define HPET_INT_STATUS_LOW     0x20 / 4
#define HPET_INT_STATUS_HIGH    0x24 / 4
#define HPET_MAIN_COUNTER_LOW   0xF0 / 4
#define HPET_MAIN_COUNTER_HIGH  0xF4 / 4
//
#define HPET_START              0x01
#define HPET_TICKS_PER_MICRO    14  // 70ns tick so 14.2 ticks per microsecond ish
//
// PEI Timeout values
//
#define PEI_HECI_WAIT_DELAY   50000     // 50ms timeout for IO delay
#define PEI_HECI_INIT_TIMEOUT 10000000  // 10 sec timeout in microseconds
#define PEI_HECI_READ_TIMEOUT 10000000  // 10sec timeout in microseconds
#define PEI_HECI_SEND_TIMEOUT 10000000  // 10sec timeout in microseconds
//
// DXE Timeout values based on HPET
//
#define HECI_WAIT_DELAY      1000      // 1ms timeout for IO delay
#define HECI_INIT_TIMEOUT    15000000  // 15sec timeout in microseconds
#define HECI_READ_TIMEOUT_EX 90000000   // 90sec timeout in microseconds
#define HECI_READ_TIMEOUT    5000000   // 5sec timeout in microseconds
#define HECI_SEND_TIMEOUT    5000000   // 5sec timeout in microseconds
#define HECI_MAX_RETRY       3         // Value based off HECI HPS
#define HECI_MSG_DELAY       2000000   // show warning msg and stay for 2 seconds.
#pragma pack(1)

/****************** REGISTER EQUATES ****************************************************/

//
// SEC_CSR_HA - SEC Control Status Host Access
//
typedef union {
  UINT32  ul;
  struct {
    UINT32  SEC_IE_HRA : 1;    // 0 - SEC Interrupt Enable (Host Read Access)
    UINT32  SEC_IS_HRA : 1;    // 1 - SEC Interrupt Status (Host Read Access)
    UINT32  SEC_IG_HRA : 1;    // 2 - SEC Interrupt Generate (Host Read Access)
    UINT32  SEC_RDY_HRA : 1;   // 3 - SEC Ready (Host Read Access)
    UINT32  SEC_RST_HRA : 1;   // 4 - SEC Reset (Host Read Access)
    UINT32  Reserved : 3;     // 7:5
    UINT32  SEC_CBRP_HRA : 8;  // 15:8 - SEC CB Read Pointer (Host Read Access)
    UINT32  SEC_CBWP_HRA : 8;  // 23:16 - SEC CB Write Pointer (Host Read Access)
    UINT32  SEC_CBD_HRA : 8;   // 31:24 - SEC Circular Buffer Depth (Host Read Access)
  } r;
} HECI_SEC_CONTROL_REGISTER;

//
// H_CSR - Host Control Status
//
typedef union {
  UINT32  ul;
  struct {
    UINT32  H_IE : 1;     // 0 - Host Interrupt Enable SEC
    UINT32  H_IS : 1;     // 1 - Host Interrupt Status SEC
    UINT32  H_IG : 1;     // 2 - Host Interrupt Generate
    UINT32  H_RDY : 1;    // 3 - Host Ready
    UINT32  H_RST : 1;    // 4 - Host Reset
    UINT32  Reserved : 3; // 7:5
    UINT32  H_CBRP : 8;   // 15:8 - Host CB Read Pointer
    UINT32  H_CBWP : 8;   // 23:16 - Host CB Write Pointer
    UINT32  H_CBD : 8;    // 31:24 - Host Circular Buffer Depth
  } r;
} HECI_HOST_CONTROL_REGISTER;


//
// SICR19 - SICR_HOST_ALIVENESS_REQ
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 H_ALIVE_REQ : 1;    // 0    - Aliveness Requested
    UINT32 Reserved : 31;      // 1:31 - Reseved
  } r;
} SICR_HOST_ALIVENESS_REQ;


//
// HICR1 - HICR_HOST_ALIVENESS_RESP
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 H_ACK: 1;   // 0    - Aliveness ack
    UINT32 Reserved: 31;   // 1:31 - Reseved
  } r;
} HICR_HOST_ALIVENESS_RESP;


//
// SICR20 - SICR_HOST_IPC_READINESS
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 HOST_RDY: 1; // 0    - Host Ready
    UINT32 SEC_RDY: 1; // 1    - SeC Ready
    UINT32 RDY_CLR: 1; // 2    - Ready Clear
    UINT32 Reserved: 29;   // 3:31 - Reseved
  } r;
} SICR_HOST_IPC_READINESS;


//
// HICR0 - HICR_HOST_IPC_READINESS
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 HOST_RDY: 1; // 0    - Host Ready
    UINT32 SEC_RDY: 1; // 1    - SeC Ready
    UINT32 RDY_CLR: 1; // 2    - Ready Clear
    UINT32 Reserved: 29;   // 3:31 - Reseved
  } r;
} HICR_SEC_IPC_READINESS;


//
// HHISR - Host High-level Interrupt Status Register.
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 INT_BAR0_STS: 1; // 0    - Host Ready
    UINT32 INT_BAR1_STS: 1; // 1    - SeC Ready
    UINT32 RSVD_31_2: 30;  // 2:31 - Reseved
  } r;
} HHISR;


//
// SICR_SEC_IPC_OUTPUT_STATUS
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 IPC_OUTPUT_READY: 1; // 0    - Host Ready
    UINT32 RSVD_31_2: 31;  // 1:31 - Reseved
  } r;
} SICR_SEC_IPC_OUTPUT_STATUS;


//
// SEC_IPC_INPUT_STATUS
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 IPC_INPUT_READY: 1; // 0    - Host Ready
    UINT32 RSVD_31_2: 31;  // 1:31 - Reseved
  } r;
} SEC_IPC_INPUT_STATUS;



//
// SEC_IPC_INPUT_DOORBELL
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 IPC_INPUT_DOORBELL: 1;  // 0    - Door bell from host
    UINT32 RSVD_31_2: 31;  // 1:31 - Reseved
  } r;
} SEC_IPC_INPUT_DOORBELL;


//
// HICR_SEC_IPC_OUTPUT_DOORBELL
//
typedef union {
  UINT32 ul;
  struct {
    UINT32 IPC_OUTPUT_DOORBELL: 1; // 0    - Door bell from SeC
    UINT32 RSVD_31_2: 31;  // 1:31 - Reseved
  } r;
} HICR_SEC_IPC_OUTPUT_DOORBELL;




//
// FWS
//
typedef union {
  UINT32  ul;
  struct {
    UINT32  CurrentState : 4;         // 0:3 - Current State
    UINT32  ManufacturingMode : 1;    // 4 Manufacturing Mode
    UINT32  FptBad : 1;               // 5 FPT(Flash Partition Table ) Bad
    UINT32  SeCOperationState : 3;     // 6:8 - SEC Operation State
    UINT32  FwInitComplete : 1;       // 9
    UINT32  FtBupLdFlr : 1;           // 10 - This bit is set when firmware is not able to load BRINGUP from the fault tolerant (FT) code.
    UINT32  FwUpdateInprogress : 1;   // 11
    UINT32  ErrorCode : 4;            // 12:15 - Error Code
    UINT32  SeCOperationMode : 4;      // 16:19 - Management Engine Current Operation Mode
    UINT32  Reserved2 : 4;            // 20:23
    UINT32  SeCBootOptionsPresent : 1; // 24 - If this bit is set, an Boot Options is present
    UINT32  AckData : 3;              // 25:27 Ack Data
    UINT32  BiosMessageAck : 4;       // 28:31 BIOS Message Ack
  } r;
} HECI_FWS_REGISTER;

//
// FWS1
//
typedef union {
  UINT32  ul;
  struct {
    UINT32  BIST_IN_PROG : 1;            // 0 Indicates whether BIST is in progress
    UINT32  RSVD_5_1 : 5;                // 5:1 - Reserved
    UINT32  MFS_CORRUPT : 1;             // 6 MFS corrupted
    UINT32  RSVD_15_7 : 9;               // 15:7 - Reserved
    UINT32  EXT_STAT_DAT_CURR_STATE : 8; // 23:16 - The interpretation of this field's value depends on the value of field INF_PROG_CODE
    UINT32  CURR_PM_EV : 4;              // 27:24 - Status of current SeC power management event
    UINT32  INF_PROG_CODE : 4;           // 31:28 - Identifies the infrastructure progress code
  } r;
} HECI_FWS1_REGISTER;

//
//  MFS_CORRUPTION_INFO
//
typedef union {
  UINT32  ul;
  struct {
    UINT32  Status : 4;                 // 3:0 Indicates whether BIST is in progress
    UINT32  Reserved : 12;              // 15:4 - Reserved
    UINT32  FileID : 4;                 // 19:16 - Reserved
    UINT32  LineNumber : 12;            // 31:20 - The interpretation of this field's value depends on the value of field INF_PROG_CODE
  } r;
} MFS_CORRUPTION_INFO;


//
// SEC Current State Values
//
#define SEC_STATE_RESET        0x00
#define SEC_STATE_INIT         0x01
#define SEC_STATE_RECOVERY     0x02
#define SEC_STATE_NORMAL       0x05
#define SEC_STATE_DISABLE_WAIT 0x06
#define SEC_STATE_TRANSITION   0x07
#define SEC_STATE_INVALID_CPU  0x08
//
// SEC Firmware FwInitComplete
//
#define SEC_FIRMWARE_COMPLETED   0x01
#define SEC_FIRMWARE_INCOMPLETED 0x00

//
// SEC Boot Options Present
//
#define SEC_BOOT_OPTIONS_PRESENT     0x01
#define SEC_BOOT_OPTIONS_NOT_PRESENT 0x00

//
// SEC Operation State Values
//
#define SEC_OPERATION_STATE_PREBOOT  0x00
#define SEC_OPERATION_STATE_M0_UMA   0x01
#define SEC_OPERATION_STATE_M3       0x04
#define SEC_OPERATION_STATE_M0       0x05
#define SEC_OPERATION_STATE_BRINGUP  0x06
#define SEC_OPERATION_STATE_M0_ERROR 0x07

//
// SEC Error Code Values
//
#define SEC_ERROR_CODE_NO_ERROR      0x00
#define SEC_ERROR_CODE_UNKNOWN       0x01
#define SEC_ERROR_CODE_IMAGE_FAILURE 0x03
#define SEC_ERROR_CODE_DEBUG_FAILURE 0x04

//
// Management Engine Current Operation Mode
//
#define SEC_OPERATION_MODE_NORMAL            0x00

#define SEC_OPERATION_MODE_ALT_DISABLED      0x02
#define SEC_OPERATION_MODE_SOFT_TEMP_DISABLE 0x03
#define SEC_OPERATION_MODE_SECOVR_JMPR       0x04
#define SEC_OPERATION_MODE_SECOVR_HECI_MSG   0x05
#pragma pack()

#endif // HECI_REGS_H
