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
  PchSmmPowerButton.c

  @brief
  File to contain all the hardware specific stuff for the Smm Power Button dispatch protocol.

**/
#include "PchSmmHelpers.h"

const PCH_SMM_SOURCE_DESC POWER_BUTTON_SOURCE_DESC = {
  PCH_SMM_SCI_EN_DEPENDENT,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_ACPI_PM1_EN
      },
      S_PCH_ACPI_PM1_EN,
      N_PCH_ACPI_PM1_EN_PWRBTN
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_ACPI_PM1_STS
      },
      S_PCH_ACPI_PM1_STS,
      N_PCH_ACPI_PM1_STS_PWRBTN
    }
  }
};

VOID
PowerButtonGetContext (
  IN  DATABASE_RECORD    *Record,
  OUT PCH_SMM_CONTEXT    *Context
  )
/**

  @brief
  Get the power button status.

  @param[in] Record               The pointer to the DATABASE_RECORD.
  @param[in] Context              Calling context from the hardware, will be updated with the current power button status.

  @retval None

**/
{
  UINT16  GenPmCon2;
  UINT32  PmcBase;

  PmcBase = MmioRead32 (
              MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_PCH,
                PCI_DEVICE_NUMBER_PCH_LPC,
                PCI_FUNCTION_NUMBER_PCH_LPC,
                R_PCH_LPC_PMC_BASE
                )
            ) & B_PCH_LPC_PMC_BASE_BAR;

  GenPmCon2 = MmioRead16 (PmcBase + R_PCH_PMC_GEN_PMCON_2);

  if ((GenPmCon2 & B_PCH_PMC_GEN_PMCON_PWRBTN_LVL) == 0) {
    Context->PowerButton.Phase = PowerButtonExit;
  } else {
    Context->PowerButton.Phase = PowerButtonEntry;
  }
}

BOOLEAN
PowerButtonCmpContext (
  IN PCH_SMM_CONTEXT     *Context1,
  IN PCH_SMM_CONTEXT     *Context2
  )
/**

  @brief
  Check whether Power Button status of two contexts match

  @param[in] Context1             Context 1 that includes Power Button status 1
  @param[in] Context2             Context 2 that includes Power Button status 2

  @retval FALSE                   Power Button status match
  @retval TRUE                    Power Button status don't match

**/
{
  return (BOOLEAN) (Context1->PowerButton.Phase == Context2->PowerButton.Phase);
}
