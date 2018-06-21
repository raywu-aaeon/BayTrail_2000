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

  ConfigMemData.c

Abstract:

  Constant and table defination for Valleyview memory configuration.

--*/

#include "ConfigMemData.h"
#include "Spd.h"
#include "MchRegs.h"

UINT16 BunitAddressMap [4] =
{
  BUNIT_BBANKMASK_OFFSET,
  BUNIT_BROWMASK_OFFSET,
  BUNIT_BRANKMASK_OFFSET,
  BUNIT_BTHCTRL_OFFSET
};

#if defined DDR3_SUPPORT && DDR3_SUPPORT
UINT32 DDR3_AddressMapTable_64[5][4]={
//memsize, register
  //bbank,  brow,   brank, rankPickmask
  {0xE,    0x1FFF0, 0x08,  0x0C}, //512
  {0xE,    0x3FFF0, 0x10,  0x18},  //1024
  {0xE,    0x7FFF0, 0x20,  0x30},  //2048
  {0xE,    0xFFFF0, 0x40,  0x60},  //4086
  {0xE,    0xFFFF0, 0x40,  0xC0},  //8192
};

UINT32 DDR3_AddressMapTable_32[5][4]={
//memsize, register
  //bbank,  brow,    brank, rankPickmask
  {0x7,     0x0FFF8, 0x04,  0x06}, //256 (0x100)
  {0x7,     0x1FFF8, 0x08,  0x0c},  //512 (0x200)
  {0x7,     0x3FFF8, 0x10,  0x18},  //1024
  {0x7,     0x7FFF8, 0x20,  0x30},  //2048
  {0x7,     0x7FFF8, 0x20,  0x60},  //4096
};
#endif	//DDR3_SUPPORT

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
UINT32 LPDDR2_AddressMapTable_64[5][4]={
//memsize, register
  //bbank,  brow,    brank, rankPickmask
  {0xE,     0x0FFF8, 0x04,  0x06}, //256 (0x100)
  {0xE,     0x1FFF0, 0x08,  0x0c},  //512 (0x200)
  {0xE,     0x3FFF0, 0x10,  0x18},  //1024
  {0xE,     0x3FFF0, 0x20,  0x30},  //2048
  {0xE,     0x7FFF0, 0x20,  0x60},  //4096
};

UINT32 LPDDR2_AddressMapTable_32[5][4]={
//memsize, register
  //bbank,  brow,    brank, rankPickmask
  {0xE,     0x0FFF8, 0x04,  0x06}, //128 (0x80)
  {0xE,     0x1FFF0, 0x08,  0x0c},  //256 (0x100)
  {0xE,     0x3FFF0, 0x10,  0x18},  //512
  {0xE,     0x3FFF0, 0x20,  0x30},  //1024
  {0xE,     0x7FFF0, 0x20,  0x60},  //2048
};
#endif	//LPDDR3_SUPPORT
