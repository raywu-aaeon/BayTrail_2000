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

    File name:    MmrcProjectDefinitionsGenerated.h
    Date:             10/3/2013
    Input File: ValleyviewA0_04_17_2013_VLV_B0_MRC_0p9.mdb
    Revision:     v1.00-ww08

Abstract:

    THIS FILE IS AUTO-GENERATED BY THE MMRC TOOL. DO NOT CHANGE THIS CODE.

    If edits are needed in this file, they must be done via the MMRC tool.

    Includes all hard defines specific to a project. Things like
    the maximum number of channels, ranks, DIMMs, etc, should be included
    here. This file needs to be included by most MMRC components,
    including Mmrc.h.

--*/

#ifndef _MMRCPROJECTDEFINITIONSGENERATED_H
#define _MMRCPROJECTDEFINITIONSGENERATED_H

//
// Maximum number of SDRAM channels supported by each CPU
//
#ifndef MAX_CHANNELS
#define MAX_CHANNELS                 2
#endif
//
// Maximum number of DIMM sockets supported by each channel
//
#ifndef MAX_DIMMS
#define MAX_DIMMS                        1
#endif
//
// Maximum number of ranks supported by each memory controller
//
#ifndef MAX_RANKS
#define MAX_RANKS                        2
#endif

#ifndef MAX_DQ_MODULES
#define	MAX_DQ_MODULES             4
#endif
#ifndef MAX_BYTELANES_PER_DQ_MODULE
#define	MAX_BYTELANES_PER_DQ_MODULE	2
#endif

//
// PFCT Size Definitions
//
#define NUM_PLAT        9
#define NUM_FREQ        7
#define NUM_CONF        8
#define NUM_TYPE        3


#endif // _MMRCPROJECTDEFINITIONSGENERATED_H

