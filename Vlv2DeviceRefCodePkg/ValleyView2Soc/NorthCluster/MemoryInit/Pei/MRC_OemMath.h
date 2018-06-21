
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  
  MRC_OemMath.h
   
Abstract:
  this file the interface to project generic MATH.
 	
    
--*/

#ifndef _MRC_MATH_H
#define _MRC_MATH_H


//
// Required libraries and macros definition
//


//
// define the interface between the MRC and the project debug print macro. 
//

#define MRC_LShiftU64(Op, Count)  LShiftU64(Op, Count)
 
#define MRC_RShiftU64(Op, Count)  RShiftU64(Op, Count)


#endif
