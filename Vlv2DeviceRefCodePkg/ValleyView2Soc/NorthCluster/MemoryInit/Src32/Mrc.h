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

  Mrc.h

Abstract:

  This file include external MRC data for Valleyview.

--*/

#ifndef _MRC_H_
#define _MRC_H_



#ifndef _MSC_EXTENSIONS
#include <string.h>
#endif

#include "Spd.h"
#include "Types.h"
#include "Imemory.h"
#include "MchRegs.h"
#include "../Mmrc/IpBlocks/VLVA0/Include/MMRC.h"

// MRC_VERSION will be written at runtime into byte @ PCI Dev 0 Function 0, 0xF0
#define MRC_VERSION 0x64	//MRC v1.00
#define DEBUG_MSG_MRC_VERSION "v1.00"

#pragma pack(1)

//
// Disabling bitfield type checking warnings.
//
#ifndef __GNUC__
#pragma warning ( disable : 4214 )
#endif

#define VOID void

#define RNK2RNK_SHARING_DISABLED 0

enum {
  MRC_DATA_TRAS = 0,
  MRC_DATA_TRP,
  MRC_DATA_TRCD,
  MRC_DATA_TWR,
  MRC_DATA_TRFC,
  MRC_DATA_TWTR,
  MRC_DATA_TRRD,
  MRC_DATA_TRTP,
  MRC_DATA_TFAW,
  MRC_DATA_MAX_TIMING_DATA
};

typedef struct _MEMINIT_SPD_DATA {
  UINT8   SpdPresent;
  UINT8   Buffer[MAX_SPD_ADDR+1];
} MEMINIT_SPD_DATA;

typedef struct {
  UINT8 ManuIDlo;                             
  UINT8 ManuIDhi;                             
  UINT8 ManuLoc;                             
  UINT8 ManuDateLO;                          
  UINT8 ManuDateHI;                          
  UINT8 SerialNumber1;                       
  UINT8 SerialNumber2;                       
  UINT8 SerialNumber3;                             
  UINT8 SerialNumber4;
  UINT8 TotalDimm; 
} FASTBOOTDATA;

typedef struct {
  UINT8   Enabled;
  UINT8   DimmConfigChannel;
  UINT8   DimmSides[MAX_SLOTS];
  UINT8   DimmDataWidth[MAX_SLOTS];
  UINT8   DimmBanks[MAX_SLOTS];
  UINT8   DimmRowAddressLines[MAX_SLOTS];
  UINT8   DimmColumnAddressLines[MAX_SLOTS];
  UINT8   DimmBusWidth[MAX_SLOTS];
  UINT8   DimmSize[MAX_SLOTS];
  UINT8   DimmMirror[MAX_SLOTS];
  UINT8   RankPresent[RANKS_PER_CHANNEL];
  UINT8   DimmPresent[MAX_SLOTS];
  UINT8   DimmECC[MAX_SLOTS];
  UINT16  SlotMem[MAX_SLOTS];
  FASTBOOTDATA FastBootData[MAX_SLOTS];                          
} CHANNEL;

// Because of Pointer length is different between dxe and pei in X64 BIOS. 
// We need move data struct that contain pointer data to last. 
//23 + 9 + 40*4 + 40*4 + 2*2 + 16*4 + 40*4 +10*2 + 4 +2 + 2= 608
#define MRC_PARAMS_SAVE_RESTORE_DEF     \
  UINT8   DdrFreq;                                      \
  UINT8   CoreFreq;                                     \
  UINT8   Tcl;                                          \
  UINT8   WL;                                           \
  UINT8   DDRType;     					                \
  UINT32  ScramblerSeed;                                \
  UINT8   TimingData[MRC_DATA_MAX_TIMING_DATA];         \
  CHANNEL Channel[MAX_CHANNELS_TOTAL];                  \
  MMRC_DATA ModMrcData;									\
  BOOLEAN EccEnabled;


// Do not add or change this structure.
typedef struct {
  MRC_PARAMS_SAVE_RESTORE_DEF
} MRC_PARAMS_SAVE_RESTORE;

typedef struct {
  // Always the first line...
  MRC_PARAMS_SAVE_RESTORE_DEF
  UINT8   MrcParamsValidFlag;

  UINT16  MmioSize;
  UINT16  TsegSize;
  UINT8   SPDAddressTable[MAX_CHANNELS_TOTAL][MAX_SLOTS];
  UINT8   MrcConfigMemProgressCodeBase;
} MRC_OEM_FRAME;
 

typedef struct _MRC_PARAMETER_FRAME {
  // Always the first line...
  MRC_PARAMS_SAVE_RESTORE_DEF

  MRC_OEM_FRAME  OemMrcData;

  UINT8   BootMode;
  UINT8   TotalDimm[MAX_CHANNELS_TOTAL];
  UINT32  EcBase;
  UINT16  SmbusBar;
  UINT8   FastBootEnable;
  UINT8   DualChannelEnable;
  UINT8   DdrFreqCap;
  UINT8   NumBitDRAMCap;
  UINT8   MaxMemSizeCap;
  UINT8   RankIndex;
  MEMINIT_SPD_DATA  SpdData[MAX_CHANNELS_TOTAL*MAX_SLOTS];
  UINT16  C0MemorySize;
  BOOLEAN PcdCpuIEDEnabled;
  UINT32  IedSize;
  UINT8   MemoryDown;
#ifdef SEC_SUPPORT_FLAG
  UINT32  SeCUmaBase;
  UINT32  SeCUmaSize;
#ifdef FTPM_ENABLE
  UINT32  SeCfTPMUmaSize;
  UINT32  SeCfTPMUmaBase;
#endif
#endif
#if defined (RTIT_Support) && (RTIT_Support == 1)
  EFI_PHYSICAL_ADDRESS  RtitBase;
  UINT32  RtitSize;
#endif
  void              *PrintContextPtr;           // use to save Pei services prt for PEI_DEBUG macro.

  UINT32 storedBunitValue1;
  UINT32 storedBunitValue2;
  UINT32 storedBunitValue3;   

  void	(*DunitMsgBus32Write)(UINT8, UINT16, UINT32);
  UINT32 (*DunitMsgBus32Read)(UINT8, UINT16);
  void	(*DunitInitCommand)(UINT8, UINT32);
  void	(*DunitWakeCommand)();
  UINT8	currentPlatform;
  UINT8	currentConfiguration;
  UINT8	currentPlatformDesign;
  UINT8	override_cpu_ODT_value;
  UINT8	override_dram_ODT_value;
  UINT8	override_dram_RTT_NOM;
  UINT8 overrideAutoSelfRefreshASR;
	UINT32	debugLevel;			// 0=Entry/Exit of 


  UINT8 SiRevisionID;
} MRC_PARAMETER_FRAME;


typedef STATUS (*MRC_TASK_FUNCTION) (MRC_PARAMETER_FRAME  *CurrentMrcData, UINT8 Channel);
typedef STATUS (*MRC_DETECTDIMM_TASK_FUNCTION) (MRC_PARAMETER_FRAME  *CurrentMrcData);

typedef struct _MRC_TASK_FUNCTION_DESCRIPTOR {
  UINT8                Checkpoint;
  UINT8                BootModesToExecute;
  MRC_TASK_FUNCTION    TaskFunctionPtr;
  UINT8                Channel;
} MRC_TASK_FUNCTION_DESCRIPTOR;

STATUS DispatchFunctionDescriptors (
  MRC_TASK_FUNCTION_DESCRIPTOR    *MrcTasks,
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

#pragma pack()

#endif

