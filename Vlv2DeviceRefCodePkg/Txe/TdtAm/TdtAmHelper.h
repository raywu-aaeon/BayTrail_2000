/*++

Copyright (c) 2004-2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTAm.h
  
Abstract:

  TDT authetication module for using TDT DXE driver.
  This driver uses the TDT protocol, HECI Protocol and TDT Platform Policy to implement Theft Deterrence Technology AM module.
  
--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/

#ifndef _TDTAM_HELPER_H_
#define _TDTAM_HELPER_H_


#include "Uefi.h"


#define PEM_LINE_SIZE               64
#define PEM_INPUT_LINE_SIZE         48
#define PEM_UNITS_PER_LINE          (PEM_INPUT_LINE_SIZE / 3)
#define PEM_DECODED_LINE_SIZE       48
#define PEM_INVALID_CHAR            255
#define PEM_PAD_CHAR                64

const char *PEMCodes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-!=";
const char *LineBreak = " \n\t";



#endif //_TDTAM_HELPER_H_