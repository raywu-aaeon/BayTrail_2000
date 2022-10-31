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

  ConfigMem.h

Abstract:

  Data defination for Valleyview memory Configuration.

--*/

#ifndef _CONFIGMEM_H_
#define _CONFIGMEM_H_

#include "Mrc.h"
#include "McFunc.h"
#include "MrcFunc.h"

#define ___INTERNAL_CONVERT_TO_STRING___(a) #a
#define CONVERT_TO_STRING(a) ___INTERNAL_CONVERT_TO_STRING___(a)

STATUS
ProgMemoryMappingRegisters (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS
ProgDraDrb (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS
SetDDRInitializationComplete (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS
ClearSelfRefresh (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );


#endif
