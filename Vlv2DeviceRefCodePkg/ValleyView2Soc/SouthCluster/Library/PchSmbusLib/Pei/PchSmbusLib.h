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
  PchSmbusLib.h

  @brief 
  Header file for PCH Smbus PEI Lib implementation. 
  
**/
#ifndef _PCH_SMBUS_PEI_LIBRARY_IMPLEMENTATION_H_

#ifdef ECP_FLAG
#include "EdkIIGlueBase.h"
#include <Library/PeiSmbusLib/PeiSmbusLibInternal.h>
#else
#include <Library/PeiSmbusLibSmbus2Ppi/InternalSmbusLib.h>
#endif

#endif
