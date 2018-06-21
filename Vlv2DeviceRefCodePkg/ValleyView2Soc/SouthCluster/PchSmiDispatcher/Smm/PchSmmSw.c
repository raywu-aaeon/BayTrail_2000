/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file
  PchSmmSw.c

  @brief
  File to contain all the hardware specific stuff for the Smm Sw dispatch protocol.

**/
#include "PchSmmHelpers.h"
#include "PlatformBaseAddresses.h"

const PCH_SMM_SOURCE_DESC SW_SOURCE_DESC = {
  PCH_SMM_NO_FLAGS,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_SMI_EN
      },
      S_PCH_SMI_EN,
      N_PCH_SMI_EN_APMC
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_SMI_STS
      },
      S_PCH_SMI_STS,
      N_PCH_SMI_STS_APM
    }
  }
};

VOID
SwGetContext (
  IN  DATABASE_RECORD    *Record,
  OUT PCH_SMM_CONTEXT    *Context
  )
/**

  @brief
  Get the Software Smi value

  @param[in] Record               No use
  @param[in] Context              The context that includes Software Smi value to be filled

  @retval None

**/
{
  UINT8 ApmCnt;
  //
  // Workaround to allow SW SMI# generation
  // Sighting 4633070
  //
  BOOLEAN LUsb;
  UINT32  Data32;

  //
  // Workaround to allow SW SMI# generation
  // Sighting 4633070
  //


  Data32 = IoRead32(AcpiBase + R_PCH_SMI_EN);
  if ((Data32 & B_PCH_SMI_EN_LEGACY_USB2) == B_PCH_SMI_EN_LEGACY_USB2) {
    LUsb = 1;
    IoWrite32 (AcpiBase + R_PCH_SMI_EN, (UINT32) (Data32 & (UINT32)~B_PCH_SMI_EN_LEGACY_USB2));
  } else {
    LUsb = 0;
  }
  ApmCnt                      = IoRead8 ((UINTN) R_PCH_APM_CNT);

  //
  // Workaround to allow SW SMI# generation
  // Sighting 4633070
  //
  if (LUsb) {
    IoWrite32(AcpiBase + R_PCH_SMI_EN, (UINT32) (IoRead32(AcpiBase + R_PCH_SMI_EN) | B_PCH_SMI_EN_LEGACY_USB2));
  }

  Context->Sw.SwSmiInputValue = ApmCnt;
}

BOOLEAN
SwCmpContext (
  IN PCH_SMM_CONTEXT     *Context1,
  IN PCH_SMM_CONTEXT     *Context2
  )
/**

  @brief
  Check whether software SMI value of two contexts match

  @param[in] Context1             Context 1 that includes software SMI value 1
  @param[in] Context2             Context 2 that includes software SMI value 2

  @retval FALSE                   Software SMI value match
  @retval TRUE                    Software SMI value don't match

**/
{
  return (BOOLEAN) (Context1->Sw.SwSmiInputValue == Context2->Sw.SwSmiInputValue);
}


