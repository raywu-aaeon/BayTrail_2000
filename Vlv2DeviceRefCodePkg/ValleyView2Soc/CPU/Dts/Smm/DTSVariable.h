/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DTSVariable.h

Abstract:

 Digital Thermal Sensor Driver include file

--*/


#ifndef _DTS_VARIABLE_H_
#define _DTS_VARIABLE_H_

// <NOT FOR REF START>---------------------------------------------------------
//#include "token.h"
// <NOT FOR REF END>-----------------------------------------------------------

#define EFI_DTS_VARIABLE_GUID \
{0x78109c08, 0xa204, 0x41e0, 0xb3, 0xd, 0x11, 0x5b, 0xfe, 0xa8, 0xab, 0x90}

#define DTS_GLOBAL_VARIABLE_NAME  L"DtsGlobalVariable"

EFI_GUID  gEfiDtsVariableGuid = EFI_DTS_VARIABLE_GUID;

//<NOT FOR REF START>-----------------------------------------------------------
//#define LOGICAL_PROCESSORS        NCPU
#define LOGICAL_PROCESSORS        4
//<NOT FOR REF END>-------------------------------------------------------------

#pragma pack(1)

typedef struct _DTS_VARIABLE {
  UINT8   NumberOfProcessors;
  UINT8   DiodeRelativeTemperature;
  UINT8   SlopeCorrectionFactors[LOGICAL_PROCESSORS];
} DTS_VARIABLE;

#pragma pack()


extern EFI_GUID gEfiDtsVariableGuid;

#endif
