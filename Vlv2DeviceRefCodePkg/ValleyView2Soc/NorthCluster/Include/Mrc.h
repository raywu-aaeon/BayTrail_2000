/*++

Copyright (c) 2005-2009 Intel Corporation. All rights reserved
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


//#if defined EFI_MEMORY_INIT && EFI_MEMORY_INIT
//#include <Pei.h>
//#endif
#ifdef SIM
#include <string.h>
#endif

#include "Spd.h"
//#include "Types.h"
//#include "Imemory.h"
#include "MchRegs.h"
#include "MMRC_Types.h"

// MRC_VERSION will be written at runtime into byte @ PCI Dev 0 Function 0, 0xF0
#define MRC_VERSION 0x37    //MRC v0.55

#pragma pack(1)

//
// Disabling bitfield type checking warnings.
//
#ifndef __GNUC__
#pragma warning ( disable : 4214 )
#endif

#define VOID void

#define MAX_LOG                             16
#define MRC_DIMM_DETECTION_ERROR            1
#define MRC_MCH_CONFIGURATION_ERROR         2
#define MRC_MCH_WRITE_LEVELING_ERROR        3
#ifdef SIM
#define MSR_XAPIC_BASE     0x01B
#endif

typedef enum {
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
  UINT8  PI[MAXDQ];
  UINT8  VREF[MAXDQ];
} RDTRAINING;


typedef struct {
  UINT8  OneXclk[MAXDQ];
  UINT8  TwoXclk[MAXDQ];
  UINT8  PI[MAXDQ];
  UINT8  DBEn[MAXDQ];
  UINT8  DBSel[MAXDQ];
} DLLSETTING;

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
  //UINT8 goldenBuffer[1024+16];      // Positioned first to enable octword alignment.
  //UINT8 *pGoldenBuffer;
  UINT8   rdvref_values[MMRC_MAX_CHANNELS*MMRC_MAX_RANKS*MMRC_MAX_BYTELANES_PER_MODULE*MMRC_MAX_MODULES];
  UINT16  wdqs_values[MMRC_MAX_CHANNELS*MMRC_MAX_RANKS*MMRC_MAX_BYTELANES_PER_MODULE*MMRC_MAX_MODULES];
  UINT16  wdq_values[MMRC_MAX_CHANNELS*MMRC_MAX_RANKS*MMRC_MAX_BYTELANES_PER_MODULE*MMRC_MAX_MODULES];
  UINT16  rcvn_values[MMRC_MAX_CHANNELS*MMRC_MAX_RANKS*MMRC_MAX_BYTELANES_PER_MODULE*MMRC_MAX_MODULES];
  UINT16  rdqs_values[MMRC_MAX_CHANNELS*MMRC_MAX_RANKS*MMRC_MAX_BYTELANES_PER_MODULE*MMRC_MAX_MODULES];
  UINT8  DQPtrMinus1Select_values[MAX_CHANNELS*MAX_RANKS*MAX_BYTELANES_PER_MODULE*MAX_MODULES];
  UINT8  DQSPtrMinus1Select_values[MAX_CHANNELS*MAX_RANKS*MAX_BYTELANES_PER_MODULE*MAX_MODULES];
  UINT8  RcvnPtrMinus1Select_values[MAX_CHANNELS*MAX_RANKS*MAX_BYTELANES_PER_MODULE*MAX_MODULES];
} TRAINING_SETTING;

//23 + 9 + 40*4 + 40*4 + 2*2 + 16*4 + 40*4 +10*2 + 4 +2 + 2= 608
#define MRC_PARAMS_SAVE_RESTORE_DEF                       \
  UINT8   DdrFreq;                                        \
  UINT8   CoreFreq;                                       \
  UINT8   D_Sides[MAX_SOCKETS];                           \
  UINT8   D_DataWidth[MAX_SOCKETS];                       \
  UINT8   D_Banks[MAX_SOCKETS];                           \
  UINT8   D_RowAddressLines[MAX_SOCKETS];                 \
  UINT8   D_ColumnAddressLines[MAX_SOCKETS];              \
  UINT8   D_BusWidth[MAX_SOCKETS];                        \
  UINT8   D_Size[MAX_SOCKETS];                            \
  UINT8   D_Mirror[MAX_SOCKETS];                          \
  UINT8   DimmConfigChannel;                              \
  UINT8   R_Present[MAX_RANKS];                           \
  UINT8   Tcl;                                            \
  UINT8   DDRType;                                        \
  UINT8   TimingData[MRC_DATA_MAX_TIMING_DATA];           \
  UINT8   D_ECC[MAX_SOCKETS];                             \
  UINT16  SlotMem[MAX_SOCKETS];                           \
  FASTBOOTDATA FastBootData[MAX_SOCKETS];                 \
  UINT32  ScramblerSeed;                                  \
  UINT8   Channel_Present[MAX_CHANNELS];                  \
  UINT8   reserved[7];                                  \
  TRAINING_SETTING  Trained_Value;                                    //DO NOT add or move before this (need 16B aligned) 632+4+20+2+4+2

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
  UINT8   SPDAddressTable[MAX_SOCKETS];
  UINT8   MrcConfigMemProgressCodeBase;
} MRC_OEM_FRAME;


typedef struct _MRC_PARAMETER_FRAME {
  // Always the first line...
  MRC_PARAMS_SAVE_RESTORE_DEF

  MRC_OEM_FRAME  OemMrcData;

  UINT32  ErrorLogIndex;
  UINT32  ErrorLog[MAX_LOG];

  UINT8   BootMode;
  UINT8   TotalDimm;
  UINT8   DimmPresent[MAX_SOCKETS];
  UINT32  EcBase;
  UINT16  SmbusBar;
  UINT8   FastBootEnable;

  UINT8   B0D0F0_RID;                  // B0:D0:F0 Revision Identification Number
  UINT8   CoreFreqCap;
  UINT8   DdrFreqCap;
  UINT8   NumBitDRAMCap;
  UINT8   MaxMemSizeCap;
  UINT8   RankIndex;
  MEMINIT_SPD_DATA  SpdData[MAX_SOCKETS];
  UINT16  C0MemorySize;
  UINT8   WL;
  UINT16  MemoryBaseAddress[MAX_RANKS];
#ifdef SEC_SUPPORT_FLAG
  UINT32  SeCUmaBase;
  UINT32  SeCUmaSize;
#endif
  void              *PrintContextPtr;           // use to save Pei services prt for PEI_DEBUG macro.

  UINT32 storedBunitValue1;
  UINT32 storedBunitValue2;
  UINT32 storedBunitValue3;
  void    (*cbForceODT)(UINT8, UINT8, UINT8);
  void    (*cbJedecCmd)(UINT8, UINT8, UINT8,UINT32);
  UINT8   currentPlatform;
  UINT8   currentConfiguration;
  UINT8   use_staticRcven;
  UINT8   use_restoreValues;
  UINT32  debugLevel;         // 0=Entry/Exit of

  EFI_PEI_SERVICES  **PeiServices;

} MRC_PARAMETER_FRAME;

typedef STATUS (*MRC_TASK_FUNCTION) (MRC_PARAMETER_FRAME  *CurrentMrcData);

typedef struct _MRC_TASK_FUNCTION_DESCRIPTOR {
  UINT8                Checkpoint;
  UINT8                BootModesToExecute;
  MRC_TASK_FUNCTION    TaskFunctionPtr;
} MRC_TASK_FUNCTION_DESCRIPTOR;

STATUS DispatchFunctionDescriptors (
  MRC_TASK_FUNCTION_DESCRIPTOR    *MrcTasks,
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

#pragma pack()

#endif

