/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    ProcessorData.h
    
Abstract:

    Header file for CPU Data File
    
Revision History

--*/

#ifndef _PROCESSOR_DATA_H_
#define _PROCESSOR_DATA_H_

#include "CpuDxe.h"

#define EFI_PROCESSOR_TYPE_SIZE 6

typedef struct {
  UINT16  Reserved                        :1;
  UINT16  Unknown                         :1;
  UINT16  Processor64bitCapable           :1;
  UINT16  ProcessorMultiCore              :1;
  UINT16  ProcessorHardwareThread         :1;
  UINT16  ProcessorExecuteProtection      :1;
  UINT16  ProcessorEnhancedVirtualization :1;
  UINT16  ProcessorPowerControl           :1;
  UINT16  Reserved2                       :8;
} EFI_PROCESSOR_CHARACTERISTICS_DATA_EXT;


typedef struct {
  UINT8             Index;
  UINT8             ProcessorFamily;
  EFI_STRING_TOKEN  ProcessorVersionToken;
} EFI_PROCESSOR_VERSION;

typedef struct {
  UINT32  Dword1;
  UINT32  Dword2;
  UINT32  Dword3;
  UINT32  Dword4;
} EFI_QDWORD;

#endif
