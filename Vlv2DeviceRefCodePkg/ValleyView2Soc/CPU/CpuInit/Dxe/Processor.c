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

    Processor.c

Abstract:

    Produces CPU data records.



--*/

#include "CpuDxe.h"
#include "Processor.h"
#include "Cache.h"

#ifndef ECP_FLAG
extern EFI_HII_PROTOCOL           *mHii;
#endif
extern EFI_DATA_HUB_PROTOCOL      *mDataHub;
extern EFI_PLATFORM_CPU_PROTOCOL  *mPlatformCpu;
extern EFI_HII_HANDLE             mStringHandle;

EFI_SUBCLASS_TYPE1_HEADER         mCpuDataRecordHeader = {
  EFI_PROCESSOR_SUBCLASS_VERSION,       // Version
  sizeof (EFI_SUBCLASS_TYPE1_HEADER),   // Header Size
  0,                                    // Instance, Initialize later
  EFI_SUBCLASS_INSTANCE_NON_APPLICABLE, // SubInstance
  0                                     // RecordType, Initialize later
};

EFI_STATUS
LogCpuData (
  EFI_DATA_HUB_PROTOCOL      *DataHub,
  UINT8                      *Buffer,
  UINT32                     Size
  );

EFI_STATUS
GetCoreFrequencyList (
  OUT EFI_EXP_BASE10_DATA   **List
  );

EFI_STATUS
GetFsbFrequencyList (
  OUT EFI_EXP_BASE10_DATA   **List
  );

EFI_STATUS
InitializeProcessorData (
  IN  UINTN                         CpuNumber,
  IN  CPU_DATA_FOR_DATAHUB          *CpuDataForDatahub
  )
/*++

Routine Description:
  This function gets called with the processor number and will log all data to data hub
  pertaining to this processor.

Arguments:
  CpuNumber         - Processor Number
  CpuDataForDatahub - Contains CPU data which is collected for data hub
Returns:

    None

--*/

//
// GC_TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// GC_TODO:    EFI_SUCCESS - add return value to function comment
//
{
  EFI_STATUS                    Status;
  EFI_CPU_DATA_RECORD_BUFFER    RecordBuffer;
  UINT32                        HeaderSize;
  UINT32                        TotalSize;
  EFI_PLATFORM_CPU_INFORMATION  PlatformCpuInfo;
  STRING_REF                    Token;

  mCpuDataRecordHeader.Instance = (UINT16) (CpuNumber + 1);

  HeaderSize                    = sizeof (EFI_SUBCLASS_TYPE1_HEADER);
  RecordBuffer.Raw              = AllocatePool (HeaderSize + EFI_CPU_DATA_MAXIMUM_LENGTH);
  if (RecordBuffer.Raw == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (&PlatformCpuInfo, sizeof (EFI_PLATFORM_CPU_INFORMATION), 0);
  CopyMem (RecordBuffer.Raw, &mCpuDataRecordHeader, HeaderSize);

  //
  // Record Type 1
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType = ProcessorCoreFrequencyRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorCoreFrequency.Value = (UINT16) CpuDataForDatahub->IntendCoreFrequency;
  RecordBuffer.DataRecord->VariableRecord.ProcessorCoreFrequency.Exponent = 6;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_CORE_FREQUENCY_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 2
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType                    = ProcessorFsbFrequencyRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorFsbFrequency.Value     = (UINT16) CpuDataForDatahub->IntendFsbFrequency;
  RecordBuffer.DataRecord->VariableRecord.ProcessorFsbFrequency.Exponent  = 6;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_FSB_FREQUENCY_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 3
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType = ProcessorVersionRecordType;
  if (CpuDataForDatahub->Version.StringValid) {
#ifdef ECP_FLAG
    Token = 0;
#else
    Token   = HiiSetString(mStringHandle, 0, CpuDataForDatahub->Version.BrandString, NULL);
#endif
    if (Token == 0) {
      Token = CpuDataForDatahub->Version.StringRef;
    }
  } else {
    Token = CpuDataForDatahub->Version.StringRef;
  }

  RecordBuffer.DataRecord->VariableRecord.ProcessorVersion  = Token;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_VERSION_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 4
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType          = ProcessorManufacturerRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorManufacturer = CpuDataForDatahub->Manufacturer;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_MANUFACTURER_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 5. (Processor Serial Number: this feature is only available on PIII, not support here)
  //
  //
  // Record Type 6.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType = ProcessorIdRecordType;
  CopyMem (
    &RecordBuffer.DataRecord->VariableRecord.ProcessorId,
    &CpuDataForDatahub->CpuidData,
    sizeof (CpuDataForDatahub->CpuidData)
    );
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_ID_DATA);
  Status    = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 7.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType  = ProcessorTypeRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorType = EfiCentralProcessor;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_TYPE_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 8.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorFamilyRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorFamily = CpuDataForDatahub->Family;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_FAMILY_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type ProcessorFamily2RecordType.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorFamily2RecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorFamily2 = (EFI_PROCESSOR_FAMILY2_DATA) (CpuDataForDatahub->Family);
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_FAMILY2_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type ProcessorCoreCountRecordType.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorCoreCountRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorCoreCount = CpuDataForDatahub->CoreCount;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_CORE_COUNT_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type ProcessorEnabledCoreCountRecordType.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorEnabledCoreCountRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorEnabledCoreCount = CpuDataForDatahub->CoreEnabled;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_ENABLED_CORE_COUNT_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type ProcessorThreadCountRecordType.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorThreadCountRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorThreadCount = CpuDataForDatahub->ThreadCount;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_THREAD_COUNT_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type ProcessorCharacteristicsRecordType.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorCharacteristicsRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorCharacteristics = CpuDataForDatahub->ProcessorCharacteristics;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_CHARACTERISTICS_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 9.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType              = ProcessorVoltageRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorVoltage.Value    = CpuDataForDatahub->Voltage;
  RecordBuffer.DataRecord->VariableRecord.ProcessorVoltage.Exponent = -3;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_VOLTAGE_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 10.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType      = ProcessorApicBaseAddressRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorApicBase = CpuDataForDatahub->ApicBase;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_APIC_BASE_ADDRESS_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 11.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorApicIdRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorApicId = CpuDataForDatahub->ApicID;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_APIC_ID_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 12.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType                = ProcessorApicVersionNumberRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorApicVersionNumber  = CpuDataForDatahub->ApicVersion;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_APIC_VERSION_NUMBER_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 13.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType  = CpuUcodeRevisionDataRecordType;
  RecordBuffer.DataRecord->VariableRecord.CpuUcodeRevisionData.ProcessorMicrocodeType = EfiProcessorIa32Microcode;
  RecordBuffer.DataRecord->VariableRecord.CpuUcodeRevisionData.ProcessorMicrocodeRevisionNumber = CpuDataForDatahub->MicrocodeRevision;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_MICROCODE_REVISION_DATA);
  Status    = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 14.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType    = ProcessorStatusRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorStatus = CpuDataForDatahub->Status;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_STATUS_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 17(0x11). Set in Cache File
  //
  //
  // Record Type 21(0x15). zero based
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType            = ProcessorPackageNumberRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorPackageNumber  = CpuDataForDatahub->Location.Package;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_PACKAGE_NUMBER_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 24(0x18).
  // Health definition in DataHub spec is not the same as BIST format, so add 1 to convert
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType          = ProcessorHealthStatusRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorHealthStatus = CpuDataForDatahub->Health.Uint32 + 1;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_HEALTH_STATUS);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // The following record data comes from platform driver:
  //
  PlatformCpuInfo.StringHandle  = mStringHandle;
  PlatformCpuInfo.ApicID        = CpuDataForDatahub->ApicID;
  Status = mPlatformCpu->GetCpuInfo (
                          mPlatformCpu,
                          &CpuDataForDatahub->Location,
                          &PlatformCpuInfo
                          );
  //
  // Record Type 15.
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType        = ProcessorSocketTypeRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorSocketType = PlatformCpuInfo.SocketType;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_SOCKET_TYPE_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 16(0x10).
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType        = ProcessorSocketNameRecordType;
  //RecordBuffer.DataRecord->VariableRecord.ProcessorSocketName = PlatformCpuInfo.SocketName;
  RecordBuffer.DataRecord->VariableRecord.ProcessorSocketName = STRING_TOKEN(STR_SOCKET_DESIGNATOR);
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_SOCKET_NAME_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 18(0x12).
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType              = ProcessorMaxCoreFrequencyRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorMaxCoreFrequency = PlatformCpuInfo.MaxCoreFrequency;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_MAX_CORE_FREQUENCY_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 19(0x13).
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType      = ProcessorAssetTagRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorAssetTag = PlatformCpuInfo.AssetTag;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_ASSET_TAG_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 20(0x14).
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType              = ProcessorMaxFsbFrequencyRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorMaxFsbFrequency  = PlatformCpuInfo.MaxFsbFrequency;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_MAX_FSB_FREQUENCY_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 22(0x16).
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType                = ProcessorCoreFrequencyListRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorCoreFrequencyList  = PlatformCpuInfo.PlatformCoreFrequencyList;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_CORE_FREQUENCY_LIST_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

  //
  // Record Type 23(0x17).
  //
  RecordBuffer.DataRecord->DataRecordHeader.RecordType              = ProcessorFsbFrequencyListRecordType;
  RecordBuffer.DataRecord->VariableRecord.ProcessorFsbFrequencyList = PlatformCpuInfo.PlatformFsbFrequencyList;
  TotalSize = HeaderSize + sizeof (EFI_PROCESSOR_FSB_FREQUENCY_LIST_DATA);
  Status = LogCpuData (mDataHub, RecordBuffer.Raw, TotalSize);

#ifdef ECP_FLAG
  (gBS->FreePool) (RecordBuffer.Raw);
#else
  gBS->FreePool (RecordBuffer.Raw);
#endif

  return EFI_SUCCESS;
}

EFI_STATUS
LogCpuData (
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
                      &gEfiProcessorSubClassGuid,
                      &gProcessorProducerGuid,
                      EFI_DATA_RECORD_CLASS_DATA,
                      Buffer,
                      Size
                      );
  return Status;

}
