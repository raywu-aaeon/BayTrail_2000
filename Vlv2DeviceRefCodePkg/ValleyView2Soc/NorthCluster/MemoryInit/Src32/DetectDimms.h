/*++

Copyright (c) 2005-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DetectDimms.h

Abstract:

  This file include all the external DetectDimm.

--*/

#ifndef _DETECTDIMMS_H_
#define _DETECTDIMMS_H_

#include "Mrc.h"
#include "McFunc.h"
#include "MrcFunc.h"

#define PRINT_SPD_TABLE	0

typedef enum _SMBUS_OPERATION {
  SMBUS_READ_BYTE,
  SMBUS_WRITE_BYTE,
  SMBUS_READ_BLOCK,
  SMBUS_WRITE_BLOCK
} SMBUS_OPERATION;

//
// SysCtl SMBus address and block size
//
#define VF_SC_SMBUS_ADDRESS        	0x60
#define VF_SC_BYTE_LEN            	1
#define VF_SC_BLOCK_LEN            	4
#define VF_SC_SMBUS_WRCMD           1
#define VF_SC_SMBUS_RDCMD           0
//
// SysCtl registers offset
//
#define VF_SC_REG_SODIMM_CONFIG    36    

//
// SysCtl SODIMM Config register bit definition
//
#define VF_SC_SODIMM_NORMAL        	0  // 1.5V
#define VF_SC_SODIMM_LOW       		1  // 1.35V
#define VF_SC_SODIMM_ULTRA_LOW 		2  //1.25V

//
// SMBus I/O Registers
//
#define R_PCH_SMBUS_HSTS                   	0x00  // Host Status Register R/W
#define B_PCH_SMBUS_HBSY          			0x01
#define R_PCH_SMBUS_HCTL                   	0x02  // Host Control Register R/W
	#define B_PCH_SMBUS_START                  	BIT6  // Start
	#define B_PCH_SMBUS_DERR          			0x04
	#define B_PCH_SMBUS_BERR          			0x08
	#define B_PCH_SMBUS_IUS           			0x40
	#define B_PCH_SMBUS_BYTE_DONE_STS 			0x80
	#define B_PCH_SMBUS_HSTS_ALL      			0xFF
	#define V_PCH_SMBUS_SMB_CMD_BYTE_DATA      	0x08  // Byte Data
	#define V_PCH_SMBUS_SMB_CMD_BLOCK          	0x14  // Block


#define R_PCH_SMBUS_HCMD                   	0x03  // Host Command Register R/W

#define R_PCH_SMBUS_TSA                    	0x04  // Transmit Slave Address Register R/W
	#define B_PCH_SMBUS_RW_SEL_READ            	0x01  // Read

#define R_PCH_SMBUS_HD0                    	0x05  // Data 0 Register R/W
#define R_PCH_SMBUS_HD1                    	0x06  // Data 1 Register R/W
#define R_PCH_SMBUS_HBD                    	0x07  // Host Block Data Register R/W

#define R_PCH_SMBUS_AUXS                   	0x0C  // Auxiliary Status Register R/WC
	#define B_PCH_SMBUS_CRCE                   	BIT0  // CRC Error

#define R_PCH_SMBUS_AUXC                   	0x0D  // Auxiliary Control Register R/W
	#define B_PCH_SMBUS_E32B                   	BIT1  // Enable 32-byte Buffer
	#define B_PCH_SMBUS_AAC                    	BIT0  // Automatically Append CRC

#define BUS_TRIES                 3       // How many times to retry on Bus Errors

#pragma pack(1)

STATUS
FindTrasTrpTrcd (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );

STATUS
CalcDimmConfig (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );

STATUS
ReadSpdFromSmbus (
  UINT32  SmbusBase,
  UINT8   SpdAddress,
  UINT8   Offset,
  UINTN   Count,
  UINT8   *Buffer
  );

STATUS
GetSpdData (
  MRC_PARAMETER_FRAME   *CurrentMrcData,
  UINT8 Channel,
  UINT8 *SpdTable,
  UINT8 TableLen
  );

STATUS
IdentifyDimms (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );

STATUS
FindTclTacTClk (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );

typedef struct {
	UINT8	TimingDataIndex;
	UINT8	MaxMCHVal;
	UINT8	DDR3_MinMCHVal;
	UINT8	DDR3_LowSPDByte;
	UINT8	DDR3_HighSPDByte;
	UINT8	DDR3_HighSPDByteMask;
} STRUCT_TIMING_DATA;

STATUS
MrcSmbusExec (
  UINT16 SmbusBase,
  UINT8 SlvAddr,
  UINT8 Operation,
  UINT8 Offset,
  UINT8 *Length,
  UINT8 *Buffer
  );

#pragma pack()

#endif
