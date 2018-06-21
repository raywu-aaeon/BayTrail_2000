/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/
/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  MobileProcessorDefinitions.h

Abstract:

--*/

#ifndef _EXTENDED_CPU_FUNCS_H_
#define _EXTENDED_CPU_FUNCS_H_

#define EFI_CPUID_EXT_FAMILY                    0x0FF00000
#define EFI_CPUID_EXT_MODEL                     0x000F0000

#define EFI_CPUID_DOTHAN                        0x00D0
#define EFI_CPUID_YONAH                         0x00E0
#define EFI_CPUID_MEROM                         0x00F0
#define EFI_CPUID_MILLVILLE                     0x0060
#define EFI_CPUID_SILVERTHORN                   0x00C0

#define EFI_CPUID_PENRYN                                0x00010670
#define   EFI_CPUID_FAMILY_MODEL_EXPANDED_LSW           0x00000670
#define   EFI_CPUID_FAMILY_MODEL_EXPANDED_MSW           0x00000001
#define EFI_CPUID_FAMILY_MODEL_DOTHAN                   0x6D0
#define EFI_CPUID_FAMILY_MODEL_YONAH                    0x6E0
#define EFI_CPUID_FAMILY_MODEL_MEROM                    0x6F0
#define EFI_CPUID_FAMILY_MODEL_MILLVILLE                0x10660
#define   EFI_CPUID_FAMILY_MODEL_MILLVILLE_LSW          0x0660
#define   EFI_CPUID_FAMILY_MODEL_MILLVILLE_MSW          0x0001
#define EFI_CPUID_FAMILY_MODEL_SILVERTHORNE             0x6C0
#define   EFI_CPUID_FAMILY_MODEL_SILVERTHORNE_LSW       0x06C0
#define   EFI_CPUID_FAMILY_MODEL_SILVERTHORNE_MSW       0x0001

#define EFI_MSR_BBL_CR_CTL3                     0x11E
#define EFI_MSR_PIC_SENS_CFG                    0x1AA

#endif
