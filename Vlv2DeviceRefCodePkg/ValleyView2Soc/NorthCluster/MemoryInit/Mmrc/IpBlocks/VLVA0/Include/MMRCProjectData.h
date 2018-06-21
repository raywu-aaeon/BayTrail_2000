#ifndef _MMRCPROJECTDATA_H_
#define _MMRCPROJECTDATA_H_
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

  MMRCProjectData.h

Abstract:

  Internal and external data structures, Macros, and enumerations.

--*/
#include "MMRCLibraries.h"
#include "MMRCProjectDefinitions.h"
#include "MmrcProjectDataGenerated.h"


#ifndef PCIEX_BASE_ADDRESS
//
// PCIe Base Address
//
#define PCIEX_BASE_ADDRESS  0xE0000000
//
// Smbus Address
//
#define SMA                 0xEFA0
//
// PMC Memory Space
//
#define PBASE               0xFED03000
//
// Smbus Memory Space
//
#define SMBMBAR             0xFED04000
//
// IO Controllers Memory Space
//
#define IOBASE              0xFED06000
//
// iLB Memory Space Address
//
#define IBASE               0xFED08000
//
// M-Phys Memory Space
//
#define MPBASE              0xFEF00000
//
// RCRB Memory Space
//
#define RCBA                0xFED1C000
//
// ACPI IO Base
//
#define ABASE               0x400
//
// GPIO IO Base
//
#define GBASE               0x500
#endif

#define PRINT_FUNCTION_INFO     
#define PRINT_FUNCTION_INFO_MAX 

#ifndef _MRCPROJECTDATA_H_
#define _MRCPROJECTDATA_H_

#define DONT_CARE            0xFF
#endif

#define TOTAL_FINISH_BL         ((1 << MAX_STROBES) - 1)

typedef int status_t;

#define TASK_FUNCTION_DESC_DONE 0xFF, 0xFF, (STATUS (*)(MMRC_DATA *, UINT8)) 0xFFFFFFFF, 0xFF
#define PHYINIT_LIST_DONE      ((UINT8 *) 0xFFFFFFFF)

#endif
