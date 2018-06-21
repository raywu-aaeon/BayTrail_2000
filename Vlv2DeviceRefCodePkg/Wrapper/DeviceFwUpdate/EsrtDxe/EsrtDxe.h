/*++

This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor.  This file may be modified by the user, subject to  
the additional terms of the license agreement               

--*/

/*++
Copyright (c)  2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
  
Module Name:

 EsrtDxe.h

Abstract:

  
--*/
#ifndef _DXE_ESRT_H_
#define _DXE_ESRT_H_

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Library/Esrt.h>



EFI_STATUS
EsrtPopulateTable(
);

EFI_STATUS
EsrtUpdateTableEntryByGuid(
    IN EFI_GUID FwEntryGuid,
    IN FW_RES_ENTRY *FwEntry
);

EFI_STATUS
EsrtGetFwEntryByGuid(
    IN EFI_GUID FwEntryGuid,
    OUT FW_RES_ENTRY *FwEntry
);

#endif
