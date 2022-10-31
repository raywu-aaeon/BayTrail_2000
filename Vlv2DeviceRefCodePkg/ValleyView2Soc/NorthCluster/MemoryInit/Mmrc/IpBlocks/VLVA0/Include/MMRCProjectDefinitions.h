/*++

Copyright (c) 2005 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  MMRCProjectDefinitions.h

Abstract:

  Includes all hard defines specific to a project. Things like
  the maximum number of channels, ranks, DIMMs, etc, should be included
  here. This file needs to be included by most MMRC components,
  including MMRC.h.

--*/

#define MRS_DEFINED 1
#include <Token.h>	//AMI_OVERRIDE - Create a token to enable\disable MRC debug message. - (P121412A+)

#ifndef _MMRC_PROJECT_DEFINITIONS_H
#define _MMRC_PROJECT_DEFINITIONS_H
#include "MmrcProjectDefinitionsGenerated.h"
//
// Maximum number of DUNITs per CPU socket.
//
#ifndef MAX_DUNITS
#define MAX_DUNITS           2
#endif
//
// Maximum number of SDRAM channels per memory controller
//
#ifndef MAX_CH_PER_DUNIT
#define MAX_CH_PER_DUNIT     1
#endif

//
// Maximum number of DIMM sockets supported by each channel
//
#ifndef MAX_DIMMS_PER_CHANNEL
#define MAX_DIMMS_PER_CHANNEL	1
#endif

//
// Maximum number of ranks per DIMM
//
#ifndef MAX_RANKS_PER_DIMM
#define MAX_RANKS_PER_DIMM   2
#endif

//
// This is used to define the maximum DQS signals, however code
// will use a dynamic variable MaxStrobes which is set to 8 orr
// 9 depending on non-ECC vs ECC DIMM installed.
//
#ifndef MAX_STROBES
#define MAX_STROBES          9//8
#endif

//
// Maximum value of sweep boundary for 2D Eye Diagram in Read/Write training
//
#ifndef MAX_VREF_PI
#define MAX_VREF_PI 63
#endif

#define NUM_ALGOS       	 9
#define NUM_CACHE_ELEMENTS   4

// CMRB Size Definitions
#define TOTAL_CHNS    2
#define TOTAL_MODS    4
#define TOTAL_RNKS    4
#define TOTAL_BLMS    2

//
// Boot Paths
//
#ifndef S0Path
#define S0Path       1
#endif
#ifndef S3Path
#define S3Path       2
#endif
#ifndef S5Path
#define S5Path       4
#endif
#ifndef FBPath
#define FBPath       8
#endif
#ifndef S5
#define S5           16
#endif
#define S7           32
#define S15          64

//
// Bit defs
//
#ifndef BIT0
#define BIT0                 0x00000001
#define BIT1                 0x00000002
#define BIT2                 0x00000004
#define BIT3                 0x00000008
#define BIT4                 0x00000010
#define BIT5                 0x00000020
#define BIT6                 0x00000040
#define BIT7                 0x00000080
#define BIT8                 0x00000100
#define BIT9                 0x00000200
#define BIT10                0x00000400
#define BIT11                0x00000800
#define BIT12                0x00001000
#define BIT13                0x00002000
#define BIT14                0x00004000
#define BIT15                0x00008000
#define BIT16                0x00010000
#define BIT17                0x00020000
#define BIT18                0x00040000
#define BIT19                0x00080000
#define BIT20                0x00100000
#define BIT21                0x00200000
#define BIT22                0x00400000
#define BIT23                0x00800000
#define BIT24                0x01000000
#define BIT25                0x02000000
#define BIT26                0x04000000
#define BIT27                0x08000000
#define BIT28                0x10000000
#define BIT29                0x20000000
#define BIT30                0x40000000
#define BIT31                0x80000000
#endif

#endif // _MMRC_PROJECT_DEFINITIONS_H
