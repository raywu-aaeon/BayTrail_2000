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

  MemoryInit.h

Abstract:

  Framework PEIM to initialize memory on a Memory Controller.

--*/

#ifndef _MEMORY_INIT_H_
#define _MEMORY_INIT_H_

#include "../Src32/Mrc.h"
#include "../Src32/McFunc.h"
#include "../Src32/OemHooks.h"
#include "../Mmrc/IpBlocks/VLVA0/Include/MMRCProjectLibraries.h"
#include "MRC_OemDebugPrint.h"

#include <Ppi/VlvMmioPolicy.h>

#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#include <Ppi/Variable/Variable.h>
#else
#include <Library/DebugLib.h>
#endif
#ifdef ECP_FLAG
//
// MRC macro
//
#define MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, Code) \
        (*PeiServices)->PeiReportStatusCode(PeiServices, EFI_PROGRESS_CODE, Code, 0, NULL, NULL)
#define MRC_PEI_REPORT_ERROR_CODE(PeiServices, Code, Severity)\
        (*PeiServices)->PeiReportStatusCode(PeiServices, EFI_ERROR_CODE|(Severity), Code, 0, NULL, NULL)
#else
//
// MRC macro
//
#define MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, Code) \
        (*PeiServices)->ReportStatusCode(PeiServices, EFI_PROGRESS_CODE, Code, 0, NULL, NULL)                                   
#define MRC_PEI_REPORT_ERROR_CODE(PeiServices, Code, Severity)\
        (*PeiServices)->ReportStatusCode(PeiServices, EFI_ERROR_CODE|(Severity), Code, 0, NULL, NULL)
#endif

//
// MRC Variable Attributes
//
#define MEM_DET_COMMON_MEM_ATTR \
          (EFI_RESOURCE_ATTRIBUTE_PRESENT                 | \
           EFI_RESOURCE_ATTRIBUTE_INITIALIZED             | \
           EFI_RESOURCE_ATTRIBUTE_TESTED                  | \
           EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)

#define MEM_DET_COMMON_MEM_ATTR1 \
          (EFI_RESOURCE_ATTRIBUTE_PRESENT                 | \
           EFI_RESOURCE_ATTRIBUTE_INITIALIZED             | \
           EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE             | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
           EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE)


EFI_STATUS
InstallEfiMemory (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                  **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES            **PeiServices,
#endif
  IN  EFI_BOOT_MODE               BootMode,
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

EFI_STATUS
InstallS3Memory (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                 **PeiServices
#else
  IN CONST EFI_PEI_SERVICES           **PeiServices
#endif
  );


EFI_STATUS
SetOemMrcData (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                  **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES            **PeiServices,
#endif
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

EFI_STATUS
MrcParamsRestore (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                 **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES           **PeiServices,
#endif
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

EFI_STATUS
MrcParamsSave (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                 **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES           **PeiServices,
#endif
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

STATUS
DetectDimms (
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

STATUS
FillDimmsParam (
  MRC_PARAMETER_FRAME 				*CurrentMrcData,
  MRC_DRAM_INPUT 					Dram_Input
  );

STATUS
ConfigureMemory (
  MRC_PARAMETER_FRAME             *CurrentMrcData
  );

#endif

