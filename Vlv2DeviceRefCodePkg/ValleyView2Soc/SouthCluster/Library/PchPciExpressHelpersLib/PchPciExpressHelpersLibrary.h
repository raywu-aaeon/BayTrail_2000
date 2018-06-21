/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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
  PchPciExpressHelpersLibrary.h

  @brief
  Header file for PCH Pci Express helps library implementation.
  
**/
#ifndef _PCH_PCI_EXPRESS_HELPERS_LIBRARY_H_
#define _PCH_PCI_EXPRESS_HELPERS_LIBRARY_H_

#include "PchAccess.h"
#ifdef ECP_FLAG
#include "EdkIIGlueBase.h"
#else
#include "Library/PciLib.h"
#include "Library/IoLib.h"
#include "Library/DebugLib.h"
#include "Library/PcdLib.h"

#include "PchCommonDefinitions.h"
#endif

#include <Protocol/PchPlatformPolicy.h>

#ifdef ECP_FLAG
#include <pci.h>
#include <Pci23.h>
#include <pci22.h>
#else
#include <Uefi/UefiBaseType.h>

#include <IndustryStandard/Pci.h>
#include <IndustryStandard/Pci23.h>
#include <IndustryStandard/Pci22.h>
#endif

#define LTR_VALUE_MASK (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7 + BIT8 + BIT9)
#define LTR_SCALE_MASK (BIT10 + BIT11 + BIT12)

///
/// LTR related macros
///
#define LTR_LATENCY_VALUE(x)           (x & LTR_VALUE_MASK)
#define LTR_MULTIPLIER_INDEX(x)        ((UINT32)(x & LTR_SCALE_MASK) >> 10)

#endif
