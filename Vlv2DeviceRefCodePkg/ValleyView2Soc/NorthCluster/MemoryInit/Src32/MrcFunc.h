/*++

Copyright (c) 1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

	MrcFunc.h
	
Abstract: 

	This file the include all the external MRC Function.

--*/

#ifndef _MRCFUNC_H_
#define _MRCFUNC_H_

#include "Mrc.h"

#ifndef _MSC_EXTENSIONS 
#include <string.h>
#else
#pragma intrinsic(memcpy)
#pragma intrinsic(memset)
#endif

UINT8
BitScanForward8 (
  UINT8                   Input
  );

UINT8
BitScanReverse8 (
  UINT8                   Input
  );

STATUS
FillInputStructure (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS
FillOutputStructure (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

#endif

