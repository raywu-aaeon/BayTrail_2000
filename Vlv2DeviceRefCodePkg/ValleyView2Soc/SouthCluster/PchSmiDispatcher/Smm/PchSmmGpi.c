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
  PchSmmGpi.c

  @brief 
  File to contain all the hardware specific stuff for the Smm Gpi dispatch protocol.

**/
#include "PchSmmHelpers.h"

#define GPI_INIT_ELEMENT(num) { \
    PCH_SMM_NO_FLAGS, \
    { \
      { \
        { \
          ACPI_ADDR_TYPE, R_PCH_ALT_GP_SMI_EN \
        }, \
        S_PCH_ALT_GP_SMI_EN, num, \
      }, \
      NULL_BIT_DESC_INITIALIZER \
    }, \
    { \
      { \
        { \
          ACPI_ADDR_TYPE, R_PCH_ALT_GP_SMI_STS \
        }, \
        S_PCH_ALT_GP_SMI_STS, (num), \
      }, \
    } \
  }

const PCH_SMM_SOURCE_DESC GPI_SOURCE_DESC[NUM_SUPPORTED_GPIS] = {
  GPI_INIT_ELEMENT(0),
  GPI_INIT_ELEMENT(1),
  GPI_INIT_ELEMENT(2),
  GPI_INIT_ELEMENT(3),
  GPI_INIT_ELEMENT(4),
  GPI_INIT_ELEMENT(5),
  GPI_INIT_ELEMENT(6),
  GPI_INIT_ELEMENT(7),
  GPI_INIT_ELEMENT(8),
  GPI_INIT_ELEMENT(9),
  GPI_INIT_ELEMENT(10),
  GPI_INIT_ELEMENT(11),
  GPI_INIT_ELEMENT(12),
  GPI_INIT_ELEMENT(13),
  GPI_INIT_ELEMENT(14),
  GPI_INIT_ELEMENT(15),
};
