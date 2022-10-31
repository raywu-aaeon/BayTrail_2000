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
  
  MRC_OemDebugPrint.h
   
Abstract:
  this file the interface to project generic STDOUT.
 	this file connect the MRC_DEBUG_PRINT to project generic STDOUT.
    
--*/

#ifndef _MRC_DEBUG_PRINT_H
#define _MRC_DEBUG_PRINT_H


//
// Required libraries and macros definition
//


//
// define the interface between the MRC and the project debug print macro. 
//
#if defined RMT_SUPPORT && RMT_SUPPORT 
#define MRC_DEBUG_MSG(arg)  PEI_DEBUG(arg)
#else 
#define MRC_DEBUG_MSG(msg)
#endif


//
// use to define the MRC debug print mode. Generally use to define EFI_D_INFO. 
// use only in EFI BIOS. 
//
#define MRC_DEBUG_PRINT_MODE 	EFI_D_INFO

#endif
