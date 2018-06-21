/*++

Copyright (c)  1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  CacheData.c

Abstract:

  Processor Cache data records.

--*/

#include "Cache.h"
#include <IndustryStandard/SmBios.h>

extern EFI_DATA_HUB_PROTOCOL      *mDataHub;
extern EFI_SUBCLASS_TYPE1_HEADER   mCpuDataRecordHeader;

typedef struct {
  UINT16                        CacheSizeinKB;
  CACHE_ASSOCIATIVITY_DATA      Associativity;
  CACHE_TYPE_DATA               SystemCacheType;
  //
  // Can extend the structure here.
  //
} CPU_CACHE_DATA;

//
// Bit field definitions for return registers of CPUID EAX = 4
//
// EAX
#define CPU_CACHE_TYPE_MASK                0x1F
#define CPU_CACHE_LEVEL_MASK               0xE0
#define CPU_CACHE_LEVEL_SHIFT              5
// EBX
#define CPU_CACHE_LINESIZE_MASK            0xFFF
#define CPU_CACHE_PARTITIONS_MASK          0x3FF000
#define CPU_CACHE_PARTITIONS_SHIFT         12
#define CPU_CACHE_WAYS_MASK                0xFFC00000
#define CPU_CACHE_WAYS_SHIFT               22

#define CPU_CACHE_L1        1
#define CPU_CACHE_L2        2
#define CPU_CACHE_L3        3
#define CPU_CACHE_L4        4
#define CPU_CACHE_LMAX      CPU_CACHE_L4

EFI_CACHE_CONVERTER       mCacheConverter[]  = {
  {
    1,
    0x09,
    32,
    EfiCacheAssociativity4Way,
    EfiCacheTypeInstruction
  },
  {
    1,
    0x0D,
    16,
    EfiCacheAssociativity4Way,
    EfiCacheTypeData
  },
  {
    1,
    0x0E,
    24,
    EfiCacheAssociativity6Way,
    EfiCacheTypeData
  },
  {
    2,
    0x21,
    256,
    EfiCacheAssociativity8Way,
    EfiCacheTypeUnified
  },
  {
    1,
    0x2C,
    32,
    EfiCacheAssociativity8Way,
    EfiCacheTypeData
  },
  {
    1,
    0x30,
    32,
    EfiCacheAssociativity8Way,
    EfiCacheTypeInstruction
  },
  {
    2,
    0x3F,
    256,
    EfiCacheAssociativity4Way,
    EfiCacheTypeData
  },
  {
    2,
    0x40,
    0,
    EfiCacheAssociativityUnknown,
    EfiCacheTypeUnknown
  },
  {
    2,
    0x80,
    512,
    EfiCacheAssociativity8Way,
    EfiCacheTypeData
  },
  {
    3,
    0xD0,
    512,
    EfiCacheAssociativity4Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xD1,
    1024,
    EfiCacheAssociativity4Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xD2,
    2048,
    EfiCacheAssociativity4Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xD6,
    1024,
    EfiCacheAssociativity8Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xD7,
    2048,
    EfiCacheAssociativity8Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xD8,
    4096,
    EfiCacheAssociativity8Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xDC,
    1536,
    EfiCacheAssociativity12Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xDD,
    3072,
    EfiCacheAssociativity12Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xDE,
    6144,
    EfiCacheAssociativity12Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xE2,
    2048,
    EfiCacheAssociativity16Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xE3,
    4096,
    EfiCacheAssociativity16Way,
    EfiCacheTypeUnified
  },
  {
    3,
    0xE4,
    8192,
    EfiCacheAssociativity16Way,
    EfiCacheTypeUnified
  },
  {
    0,
    0xFF,
    0,
    0,
    0
  }
};

UINT8                     mCacheInstance[EFI_CACHE_LMAX] = { 0, 0, 0, 0 };

EFI_SUBCLASS_TYPE1_HEADER mCacheDataRecordHeader = {
  EFI_CACHE_SUBCLASS_VERSION,         // Version
  sizeof (EFI_SUBCLASS_TYPE1_HEADER), // Header Size
  0,                                  // Instance, Initialize later
  0,                                  // SubInstance, Initialize later to Cache Level
  0                                   // RecordType, Initialize later
};

EFI_STATUS
LogCacheData (
  EFI_DATA_HUB_PROTOCOL      *DataHub,
  UINT8                      *Buffer,
  UINT32                     Size
  );

EFI_STATUS
LogCpuData (
  EFI_DATA_HUB_PROTOCOL      *DataHub,
  UINT8                      *Buffer,
  UINT32                     Size
  );

EFI_STATUS
InitializeCacheData (
  IN  UINTN                            CpuNumber,
  IN  EFI_CPUID_REGISTER               *CacheInformation
  )
/*++

Routine Description:
  This function gets called with the processor number and will log all cache data to data hub
  pertaining to this processor.

Arguments:
  CpuNumber - Processor Number
  CacheInfo - Cache information get from cpuid instruction

Returns:

    None

--*/
// GC_TODO:    CacheInformation - add argument and description to function comment
// GC_TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                    Status;
  UINT32                        HeaderSize;
  UINT32                        TotalSize;
  EFI_CACHE_DATA_RECORD_BUFFER  RecordBuffer;
  UINT8                         Index1;
  UINT8                         CacheLevel;
  UINT32                        NumberOfDeterministicCacheParameters;
  UINT32                        RegValue;
  CPU_CACHE_DATA                CacheData[CPU_CACHE_LMAX];
  UINT32                        Ways;
  UINT32                        Partitions;
  UINT32                        LineSize;
  UINT32                        Sets;
  CACHE_TYPE_DATA               SystemCacheType;
  CACHE_ASSOCIATIVITY_DATA      Associativity;

  UINT32                        RegisterEax;
  UINT32                        RegisterEbx;
  UINT32                        RegisterEcx;
  UINT32                        RegisterEdx;

  mCacheDataRecordHeader.Instance = (UINT16) (CpuNumber + 1);

  HeaderSize                      = sizeof (EFI_SUBCLASS_TYPE1_HEADER);
  RecordBuffer.Raw                = AllocatePool (HeaderSize + EFI_CACHE_DATA_MAXIMUM_LENGTH);
  if (RecordBuffer.Raw == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get the number of deterministic cache parameter CPUID leafs
  //
  NumberOfDeterministicCacheParameters = 0;
  do {
    AsmCpuidEx (4, NumberOfDeterministicCacheParameters++, &RegValue, NULL, NULL, NULL);
  } while ((RegValue & 0x0f) != 0);


  for (Index1 = 0; Index1 < NumberOfDeterministicCacheParameters; Index1++) {
    AsmCpuidEx(4, Index1, &RegisterEax, &RegisterEbx, &RegisterEcx, &RegisterEdx);

    if ((RegisterEax & CPU_CACHE_TYPE_MASK) == 0) {
      //break;
      continue;
    }

    switch (RegisterEax & CPU_CACHE_TYPE_MASK) {
      case 1:
        SystemCacheType = CacheTypeData;
        break;
      case 2:
        SystemCacheType = CacheTypeInstruction;
        break;
      case 3:
        SystemCacheType = CacheTypeUnified;
        break;
      default:
        SystemCacheType = CacheTypeUnknown;
    }

    Ways = ((RegisterEbx & CPU_CACHE_WAYS_MASK) >> CPU_CACHE_WAYS_SHIFT) + 1;
    Partitions = ((RegisterEbx & CPU_CACHE_PARTITIONS_MASK) >> CPU_CACHE_PARTITIONS_SHIFT) + 1;
    LineSize = (RegisterEbx & CPU_CACHE_LINESIZE_MASK) + 1;
    Sets = RegisterEcx + 1;

    switch (Ways) {
      case 2:
        Associativity = CacheAssociativity2Way;
        break;
      case 4:
        Associativity = CacheAssociativity4Way;
        break;
      case 8:
        Associativity = CacheAssociativity8Way;
        break;
      case 12:
        Associativity = CacheAssociativity12Way;
        break;
      case 16:
        Associativity = CacheAssociativity16Way;
        break;
      case 24:
        Associativity = CacheAssociativity24Way;
        break;
      case 32:
        Associativity = CacheAssociativity32Way;
        break;
      case 48:
        Associativity = CacheAssociativity48Way;
        break;
      case 64:
        Associativity = CacheAssociativity64Way;
        break;
      default:
        Associativity = CacheAssociativityFully;
        break;
    }

    CacheLevel = (UINT8) ((RegisterEax & CPU_CACHE_LEVEL_MASK) >> CPU_CACHE_LEVEL_SHIFT); // 1 based
    ASSERT (CacheLevel >= 1 && CacheLevel <= CPU_CACHE_LMAX);
    CacheData[CacheLevel - 1].CacheSizeinKB = (UINT16) ((Ways * Partitions * LineSize * Sets) / 1024);
    CacheData[CacheLevel - 1].SystemCacheType = SystemCacheType;
    CacheData[CacheLevel - 1].Associativity = Associativity;

#ifdef ECP_FLAG
    (gBS->CopyMem) (RecordBuffer.Raw, &mCacheDataRecordHeader, HeaderSize);
#else
    gBS->CopyMem (RecordBuffer.Raw, &mCacheDataRecordHeader, HeaderSize);
#endif

    mCacheInstance[CacheLevel-1]++;
    RecordBuffer.DataRecord->DataRecordHeader.Instance    = mCacheInstance[CacheLevel-1];
    RecordBuffer.DataRecord->DataRecordHeader.SubInstance = CacheLevel;

    //
    //  Record Type 1
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType       = CacheSizeRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_SIZE_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheSize.Value    = CacheData[CacheLevel - 1].CacheSizeinKB;
    RecordBuffer.DataRecord->VariableRecord.CacheSize.Exponent = 10;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 2
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType              = MaximumSizeCacheRecordType;
    TotalSize = HeaderSize + sizeof (EFI_MAXIMUM_CACHE_SIZE_DATA);
    RecordBuffer.DataRecord->VariableRecord.MaximumCacheSize.Value    = CacheData[CacheLevel - 1].CacheSizeinKB;
    RecordBuffer.DataRecord->VariableRecord.MaximumCacheSize.Exponent = 10;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 3
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType        = CacheSpeedRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_SPEED_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheSpeed.Exponent = 0;
    RecordBuffer.DataRecord->VariableRecord.CacheSpeed.Value    = 0;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 4
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType  = CacheSocketRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_SOCKET_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheSocket   = STRING_TOKEN (STR_UNKNOWN);
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 5
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType  = CacheSramTypeRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_SRAM_TYPE_DATA);
#ifdef ECP_FLAG
    (gBS->SetMem) (
#else
    gBS->SetMem (
#endif
          &RecordBuffer.DataRecord->VariableRecord.CacheSramType,
          sizeof (EFI_CACHE_SRAM_TYPE_DATA),
          0
          );
    RecordBuffer.DataRecord->VariableRecord.CacheSramType.Synchronous = TRUE;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 6, since record same as Type 5
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType  = CacheInstalledSramTypeRecordType;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 7
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType    = CacheErrorTypeRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_ERROR_TYPE_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheErrorType  = EfiCacheErrorSingleBit;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 8
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType  = CacheTypeRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_TYPE_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheType     = SystemCacheType;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 9
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType        = CacheAssociativityRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_ASSOCIATIVITY_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheAssociativity  = CacheData[CacheLevel - 1].Associativity;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    //  Record Type 10
    //
    RecordBuffer.DataRecord->DataRecordHeader.RecordType                = CacheConfigRecordType;
    TotalSize = HeaderSize + sizeof (EFI_CACHE_CONFIGURATION_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.Level           = CacheLevel;
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.Socketed        = EFI_CACHE_NOT_SOCKETED;
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.Reserved2       = 0;
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.Location        = EfiCacheInternal;
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.Enable          = EFI_CACHE_ENABLED;
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.OperationalMode = EfiCacheWriteBack;
    RecordBuffer.DataRecord->VariableRecord.CacheConfig.Reserved1       = 0;
    Status = LogCacheData (mDataHub, RecordBuffer.Raw, TotalSize);

    //
    // Cache Association. Processor Record Type 17
    //
    TotalSize = HeaderSize + sizeof (EFI_CACHE_ASSOCIATION_DATA);
    RecordBuffer.DataRecord->VariableRecord.CacheAssociation.ProducerName = gProcessorProducerGuid;
    //
    // RecordBuffer.DataRecord->VariableRecord.CacheAssociation.ProducerInstance = (UINT16)Instance;
    //
    RecordBuffer.DataRecord->VariableRecord.CacheAssociation.Instance    = RecordBuffer.DataRecord->DataRecordHeader.Instance;
    RecordBuffer.DataRecord->VariableRecord.CacheAssociation.SubInstance = RecordBuffer.DataRecord->DataRecordHeader.SubInstance;
#ifdef ECP_FLAG
    (gBS->CopyMem) (RecordBuffer.Raw, &mCpuDataRecordHeader, HeaderSize);
#else
    gBS->CopyMem (RecordBuffer.Raw, &mCpuDataRecordHeader, HeaderSize);
#endif
    RecordBuffer.DataRecord->DataRecordHeader.RecordType  = CacheAssociationRecordType;
    Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);
  }


#ifdef ECP_FLAG
  (gBS->FreePool) (RecordBuffer.Raw);
#else
  gBS->FreePool (RecordBuffer.Raw);
#endif
  return EFI_SUCCESS;
}

UINT8
GetCacheIndex (
  UINT8       Descriptor
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Descriptor  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT8 Index;
  Index = 0;

  while (mCacheConverter[Index].CacheDescriptor != 0xFF) {
    if (mCacheConverter[Index].CacheDescriptor == Descriptor) {
      break;
    }

    Index++;
  }

  return Index;
}

EFI_STATUS
LogCacheData (
  EFI_DATA_HUB_PROTOCOL      *DataHub,
  UINT8                      *Buffer,
  UINT32                     Size
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  DataHub - GC_TODO: add argument description
  Buffer  - GC_TODO: add argument description
  Size    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  EFI_STATUS  Status;

  Status = DataHub->LogData (
                      DataHub,
                      &gEfiCacheSubClassGuid,
                      &gProcessorProducerGuid,
                      EFI_DATA_RECORD_CLASS_DATA,
                      Buffer,
                      Size
                      );
  return Status;

}

EFI_STATUS
CollectCacheInfo (
  IN   CPU_DATA_FOR_DATAHUB           *CpuDataForDatahub,
  OUT  EFI_DETAILED_CPU_INFO          *DetailedInfo
  )
/*++

Routine Description:

  This function is called by the platform specific cpu protocol to collect 
  cache size information for every processors installed on the system.

Arguments:

  CpuDataForDatahub - Pointer to the processor specific data structure provided by the caller.
  DetailedInfo      - Pointer to caller-allocated buffer, returned with cache size on the processor.
  
Returns:
  
  EFI_SUCCESS  - Cache size information is returned.

--*/
{
  UINT8   Index1;
  UINT8   Index2;
  UINT8   Descriptor;
  UINT8   DescriptorIndex;
  UINT8   CacheLevel;
  UINT32  Data32;
  UINT32  *CacheInfo;

  //
  // Intialize the cache size data structure.
  //
  for (Index1 = 0; Index1 < EFI_CACHE_LMAX; Index1++) {
    DetailedInfo->CacheSize[Index1].Value     = 0;
    DetailedInfo->CacheSize[Index1].Exponent  = 10;
  }
  //
  // Set the cache size by parsing the processor cache descriptors.
  //
  CpuDataForDatahub->CacheInformation[0].RegEax &= 0xFFFFFF00;
  CacheInfo = (UINT32 *) &(CpuDataForDatahub->CacheInformation);
  for (Index1 = 0; Index1 < 4; Index1++) {
    Data32 = CacheInfo[Index1];
    if ((Data32 & 0x80000000) != 0) {
      continue;
    }

    for (Index2 = 0; Index2 < 4; Index2++, Data32 >>= 8) {
      Descriptor      = (UINT8) (Data32 & 0xFF);
      DescriptorIndex = GetCacheIndex (Descriptor);
      if (DescriptorIndex < ((sizeof (mCacheConverter) / sizeof (EFI_CACHE_CONVERTER)))) {
        if (mCacheConverter[DescriptorIndex].CacheDescriptor != 0xFF) {
          CacheLevel = mCacheConverter[DescriptorIndex].CacheLevel;
          DetailedInfo->CacheSize[CacheLevel - 1].Value = DetailedInfo->CacheSize[CacheLevel - 1].Value + mCacheConverter[DescriptorIndex].CacheSizeinKB;
        }
      }
    }
  }

  return EFI_SUCCESS;
}
