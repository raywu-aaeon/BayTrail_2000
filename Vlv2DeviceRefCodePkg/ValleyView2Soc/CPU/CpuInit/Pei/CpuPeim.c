/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    CpuPeim.c

Abstract:

    EFI 2.0 PEIM to initialize the cache and program for unlock processor

Revision History

--*/

#include "PeiProcessor.h"
#include "Bist.h"

extern PEI_CACHE_PPI                    mCachePpi;

EFI_PEI_PPI_DESCRIPTOR                  mPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gPeiCachePpiGuid,
    &mCachePpi
  }
};

static EFI_PEI_NOTIFY_DESCRIPTOR        mNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiMasterBootModePpiGuid,
    BuildBistHob
  }
};

VOID
InitXmm (
  VOID
  );

extern EFI_STATUS
InitCpuInfo (
  IN EFI_PEI_SERVICES          **PeiServices
  );

EFI_STATUS
PeimInitializeCpu (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES     **PeiServices
#else
  IN CONST EFI_PEI_SERVICES     **PeiServices
#endif
  )
/*++

Routine Description:

  Loads the Processor Microcode & Install the Cache PPI

Arguments:

  PeiServices - General purpose services available to every PEIM.

Returns:

  None

--*/
// GC_TODO:    FfsHeader - add argument and description to function comment
{
  EFI_STATUS  Status;

#ifdef ECP_FLAG
  ((*PeiServices)->PeiReportStatusCode) (
#else
  (*PeiServices)->ReportStatusCode (
#endif
                    PeiServices,
                    EFI_PROGRESS_CODE,                  // Type
                    EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_PEI_INIT,  // Value
                    0,    // Instance
                    NULL, // *CallerId OPTIONAL
                    NULL  // *Data OPTIONAL
                    );
  //
  // Get CPU Info; returns error if CPU isn't Nehalem uarch
  //
  Status = InitCpuInfo ((EFI_PEI_SERVICES**)PeiServices);
  ASSERT_EFI_ERROR (Status);

#ifdef ECP_FLAG
  ((*PeiServices)->PeiReportStatusCode) (
#else
  (*PeiServices)->ReportStatusCode (
#endif
                    PeiServices,
                    EFI_PROGRESS_CODE,                  // Type
                    EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_PEI_STEP1,  // Value
                    0,    // Instance
                    NULL, // *CallerId OPTIONAL
                    NULL  // *Data OPTIONAL
                    );
  //
  // Install PPI
  //
  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiList[0]);
  ASSERT_EFI_ERROR (Status);

  //
  // Install Notify
  //
  Status = (**PeiServices).NotifyPpi (PeiServices, &mNotifyList[0]);
  ASSERT_EFI_ERROR (Status);

  //
  // Try to Init XMM support
  //
  InitXmm ();

  return Status;
}
