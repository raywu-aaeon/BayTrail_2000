/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/
/*++

Copyright (c) 2008 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchPciExpressHelpersLibrary.h

Abstract:

  Header file for PCH Pci Express helps library implementation. 
  
--*/
#ifndef _PCH_DXE_RUNTIME_PCI_EXPRESS_LIBRARY_H_
#define _PCH_DXE_RUNTIME_PCI_EXPRESS_LIBRARY_H_

#include "PchAccess.h"
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#if defined(__EDKII_GLUE_BASE_PCI_LIB_CF8__) || defined(__EDKII_GLUE_BASE_PCI_LIB_PCI_EXPRESS__)
#error "Should not use EdkIIGluePciLibCf8 or EdkIIGluePciLibPciExpress with DxeRuntimePciLibPciExpress.\n"
#endif
#else
#include "Library/PciLib.h"
#include "Library/IoLib.h"
#include "Library/DebugLib.h"
#include "Library/PcdLib.h"

#include "PchCommonDefinitions.h"

#include <Uefi.h>
#include <Uefi/UefiBaseType.h>
#include <Library/UefiRuntimeLib.h>
#endif

#endif
