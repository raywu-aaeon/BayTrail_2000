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

   Cache.h

Abstract:

    Header file for CPU Data File

Revision History

--*/

#ifndef _CACHE_H_
#define _CACHE_H_

#include "CpuDxe.h"

//
// Platform-specific definitions
//
#define EFI_CACHE_DATA_MAXIMUM_LENGTH 0x100

typedef union {
  EFI_CACHE_DATA_RECORD *DataRecord;
  UINT8                 *Raw;
} EFI_CACHE_DATA_RECORD_BUFFER;

typedef struct {
  UINT8                         CacheLevel;
  UINT8                         CacheDescriptor;
  UINT16                        CacheSizeinKB;
  EFI_CACHE_ASSOCIATIVITY_DATA  Associativity;
  EFI_CACHE_TYPE_DATA           Type;
} EFI_CACHE_CONVERTER;

#ifdef ECP_FLAG
typedef enum {
  CacheAssociativityOther        = 0x01,
  CacheAssociativityUnknown      = 0x02,
  CacheAssociativityDirectMapped = 0x03,
  CacheAssociativity2Way         = 0x04,
  CacheAssociativity4Way         = 0x05,
  CacheAssociativityFully        = 0x06,
  CacheAssociativity8Way         = 0x07,
  CacheAssociativity16Way        = 0x08,
  CacheAssociativity12Way        = 0x09,
  CacheAssociativity24Way        = 0x0A,
  CacheAssociativity32Way        = 0x0B,
  CacheAssociativity48Way        = 0x0C,
  CacheAssociativity64Way        = 0x0D,
  CacheAssociativity20Way        = 0x0E
} CACHE_ASSOCIATIVITY_DATA;


typedef enum {
  CacheTypeOther                 = 0x01,
  CacheTypeUnknown               = 0x02,
  CacheTypeInstruction           = 0x03,
  CacheTypeData                  = 0x04,
  CacheTypeUnified               = 0x05
} CACHE_TYPE_DATA;

typedef enum {
  EfiCacheAssociativity6Way      = 0x0E
} EFI_CACHE_ASSOCIATIVITY_DATA_EXT;
#else
typedef enum {
//  EfiCacheAssociativityOther        = 0x01,
//  EfiCacheAssociativityUnknown      = 0x02,
//  EfiCacheAssociativityDirectMapped = 0x03,
//  EfiCacheAssociativity2Way         = 0x04,
//  EfiCacheAssociativity4Way         = 0x05,
//  EfiCacheAssociativityFully        = 0x06,
//  EfiCacheAssociativity8Way         = 0x07,
//  EfiCacheAssociativity16Way        = 0x08,
  EfiCacheAssociativity12Way        = 0x09,
  EfiCacheAssociativity24Way        = 0x0A,
  EfiCacheAssociativity32Way        = 0x0B,
  EfiCacheAssociativity48Way        = 0x0C,
  EfiCacheAssociativity64Way        = 0x0D,
  EfiCacheAssociativity6Way         = 0x0E
} EFI_CACHE_ASSOCIATIVITY_DATA_EXT;
#endif


EFI_STATUS
InitializeCacheData (
  IN  UINTN                           CpuNumber,
  IN  EFI_CPUID_REGISTER              *CacheInformation
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber         - GC_TODO: add argument description
  CacheInformation  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

UINT8
GetCacheIndex (
  UINT8 Descriptor
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Descriptor  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
CollectCacheInfo (
  IN   CPU_DATA_FOR_DATAHUB           *CpuDataForDatahub,
  OUT  EFI_DETAILED_CPU_INFO          *DetailedInfo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuDataForDatahub - GC_TODO: add argument description
  DetailedInfo      - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
