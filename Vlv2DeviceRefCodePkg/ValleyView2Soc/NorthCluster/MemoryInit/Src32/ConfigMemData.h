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

  ConfigMemData.h

Abstract:

  This file contains extended defination of ConfigMemData.c
  

--*/

#ifndef _CONFIGMEMDATA_H_
#define _CONFIGMEMDATA_H_

#include "Mrc.h"
#include "McFunc.h"
#include "MrcFunc.h"

#pragma pack(1)

#if defined DDR3_SUPPORT && DDR3_SUPPORT
extern UINT32 DDR3_AddressMapTable_64[5][4];
extern UINT32 DDR3_AddressMapTable_32[5][4];
#endif	//DDR3_SUPPORT
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
extern UINT32 LPDDR2_AddressMapTable_64[5][4];
extern UINT32 LPDDR2_AddressMapTable_32[5][4];
#endif	//LPDDR3_SUPPORT
extern UINT16 BunitAddressMap [4];


#pragma pack()

#endif
