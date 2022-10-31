/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SmbiosMemory.c

Abstract:

  This driver will determine memory configuration information from the chipset
  and memory and create SMBIOS memory structures appropriately.

--*/

#include "SmbiosMemory.h"

extern UINT8 SmbiosMemoryStrings[];

MEMORY_MODULE_MANUFACTURE_LIST MemoryModuleManufactureList[] = {
  {0, 0x2c, L"Micron"},
  {0, 0xce, L"Samsung"},
  {1, 0x4f, L"Transcend"},
  {1, 0x98, L"Kingston"},
  {0xff, 0xff, 0}
};

///
/// Even SPD Addresses only as we read Words
///
const UINT8
SpdAddress[] = { 2, 8, 116, 118, 122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 142, 144 };

EFI_STATUS
SmbiosMemoryEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

  Routine Description:

    This driver will determine memory configuration information from the chipset
    and memory and report the memory configuration info to the DataHub.


  Arguments:

    ImageHandle   - Handle for the image of this driver
    SystemTable   - Pointer to the EFI System Table

  Returns:

    EFI_SUCCESS if the data is successfully reported
    EFI_NOT_FOUND if the HOB list could not be located.

--*/
{
  BOOLEAN                         DimmPresent = FALSE;
  CHAR16                          StringBuffer2[64];
  EFI_DATA_HUB_PROTOCOL           *DataHub;
  DDR_ROW_CONFIG                  RowConfArray[MAX_SOCKETS];
  EFI_MEMORY_SUBCLASS_DRIVER_DATA MemorySubClassData;
  EFI_SMBUS_DEVICE_ADDRESS        SmbusSlaveAddress;
  EFI_SMBUS_HC_PROTOCOL           *SmbusController;
  EFI_STATUS                      Status;
  EFI_STRING                      StringBuffer;
  EFI_PHYSICAL_ADDRESS            BaseAddress;
  UINT8                           Index;
  UINT8                           *SmbusBuffer;
  UINT16                          ArrayInstance;
  UINT64                          DimmMemorySize;
  UINT64                          TotalMemorySize;
  UINT8                           Dimm;
  UINT8                           DimmIndex;
  UINTN                           SmbusBufferSize;
  UINTN                           SmbusLength;
  UINTN                           SmbusOffset;
  UINTN                           StringBufferSize;
  UINT8                           NumSlots;
  UINT8                           Freq;
  UINT8                           Type;
//  UINT8                           ChannelMode;
  UINT8                           IndexCounter;
  UINTN                           IdListIndex;
  UINT8                           SerialNumStart;
  UINT8                           PartNumber;
  UINT16                          PrimaryBusBandwidth;
  MEM_INFO_PROTOCOL               *MemInfoHob;
  EFI_GUID                        MemInfoProtocolGuid = MEM_INFO_PROTOCOL_GUID;
  UINT8                           i;

#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
  EFI_HII_DATABASE_PROTOCOL       *HiiDataBase;
  EFI_HII_STRING_PROTOCOL         *HiiString;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_PACKAGE_LIST_HEADER     *PackageList;
  EFI_HII_HANDLE                  StringPackHandle;
#else
  EFI_HII_PROTOCOL                *Hii;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HII_PACKAGES                *PackageList;
#endif
#else
  EFI_HII_DATABASE_PROTOCOL       *HiiDataBase;
  EFI_HII_STRING_PROTOCOL         *HiiString;
  EFI_HII_HANDLE                  StringPackHandle;
  CHAR8                           Language[] = "en-US";
#endif
  UINT8                           spdadressstep;

  STRING_REF                      DimmToDevLocator[] = {
    STRING_TOKEN(STR_MEMORY_SUBCLASS_DEVICE_LOCATOR_0), STRING_TOKEN(STR_MEMORY_SUBCLASS_DEVICE_LOCATOR_1),
    STRING_TOKEN(STR_MEMORY_SUBCLASS_DEVICE_LOCATOR_2), STRING_TOKEN(STR_MEMORY_SUBCLASS_DEVICE_LOCATOR_3),
    STRING_TOKEN(STR_MEMORY_SUBCLASS_DEVICE_LOCATOR_4), STRING_TOKEN(STR_MEMORY_SUBCLASS_DEVICE_LOCATOR_5)
  };

  STRING_REF                      DimmToBankLocator[] = {
    STRING_TOKEN(STR_MEMORY_SUBCLASS_BANK_LOCATOR_0), STRING_TOKEN(STR_MEMORY_SUBCLASS_BANK_LOCATOR_1),
    STRING_TOKEN(STR_MEMORY_SUBCLASS_BANK_LOCATOR_2), STRING_TOKEN(STR_MEMORY_SUBCLASS_BANK_LOCATOR_3)
  };

  EFI_GUID  gEfiMemorySubClassDriverGuid = EFI_MEMORY_SUBCLASS_DRIVER_GUID;

  StringBufferSize = (sizeof (CHAR16)) * 100;
  StringBuffer = AllocateZeroPool(StringBufferSize);
  if (StringBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  SmbusBuffer = NULL;
  SmbusBufferSize = 0x100;
  SmbusBuffer = AllocatePool(SmbusBufferSize);
  if (SmbusBuffer == NULL) {
    (gBS->FreePool) (StringBuffer);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->LocateProtocol (&gEfiDataHubProtocolGuid, NULL, (VOID **) &DataHub);
  ASSERT_EFI_ERROR (Status);

#ifdef ECP_FLAG
  Status = gBS->LocateProtocol (&gEfiSmbusProtocolGuid, NULL, (VOID **) &SmbusController);
#else
  Status = gBS->LocateProtocol (&gEfiSmbusHcProtocolGuid, NULL, (VOID **) &SmbusController);
#endif

  ASSERT_EFI_ERROR (Status);

#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)

  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &HiiDataBase
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHiiStringProtocolGuid,
                  NULL,
                  (VOID **) &HiiString
                  );
  ASSERT_EFI_ERROR (Status);

  ///Create driver handle used by HII database
  Status = CreateHiiDriverHandle (&DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PackageList = PreparePackageList (1, &gEfiMemorySubClassDriverGuid, SmbiosMemoryStrings);
  if (PackageList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = HiiDataBase->NewPackageList (
                          HiiDataBase,
                          PackageList,
                          DriverHandle,
                          &StringPackHandle
                          );
  ASSERT_EFI_ERROR (Status);

  (gBS->FreePool) (PackageList);
#else
  Status = gBS->LocateProtocol (&gEfiHiiProtocolGuid, NULL, (VOID **) &Hii);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Add our default strings to the HII database. They will be modified later.
  ///
  PackageList = PreparePackages (1, &gEfiMemorySubClassDriverGuid, SmbiosMemoryStrings);
  Status      = Hii->NewPack (Hii, PackageList, &HiiHandle);

  (gBS->FreePool) (PackageList);
#endif
#else

  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **) &HiiDataBase
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHiiStringProtocolGuid,
                  NULL,
                  (VOID **) &HiiString
                  );
  ASSERT_EFI_ERROR (Status);

  ///Create driver handle used by HII database
  StringPackHandle = HiiAddPackages (
                       &gEfiMemorySubClassDriverGuid,
                       ImageHandle,
                       SmbiosMemoryStrings,
                       NULL
                       );

  if (StringPackHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
#endif

  //
  // Data for TYPE 16 SMBIOS Structure
  //

  //
  // Create physical array and associated data for all mainboard memory
  //
  ArrayInstance = 1;
  TotalMemorySize = 0;
  MemorySubClassData.Header.Version     = EFI_MEMORY_SUBCLASS_VERSION;
  MemorySubClassData.Header.HeaderSize  = sizeof (EFI_SUBCLASS_TYPE1_HEADER);
  MemorySubClassData.Header.Instance    = ArrayInstance;
  MemorySubClassData.Header.SubInstance = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
  MemorySubClassData.Header.RecordType  = EFI_MEMORY_ARRAY_LOCATION_RECORD_NUMBER;

  MemorySubClassData.Record.ArrayLocationData.MemoryArrayLocation = EfiMemoryArrayLocationSystemBoard;
  MemorySubClassData.Record.ArrayLocationData.MemoryArrayUse      = EfiMemoryArrayUseSystemMemory;


  //
  // These are Chipset specific.
  // These are hard coded as we cannot determine these through SW.
  //

  MemorySubClassData.Record.ArrayLocationData.MemoryErrorCorrection = EfiMemoryErrorCorrectionNone;
  MemorySubClassData.Record.ArrayLocationData.MaximumMemoryCapacity = MAX_MEM_CAPACITY;

  NumSlots = (UINT8)(MAX_SOCKETS);
  MemorySubClassData.Record.ArrayLocationData.NumberMemoryDevices = (UINT16) (NumSlots);

  //
  // Report top level physical array to datahub
  // This will translate into a Type 16 SMBIOS Record
  //
  Status = DataHub->LogData (
                      DataHub,
                      &gEfiMemorySubClassGuid,
                      &gEfiMemorySubClassDriverGuid,
                      EFI_DATA_RECORD_CLASS_DATA,
                      &MemorySubClassData,
                      sizeof (EFI_SUBCLASS_TYPE1_HEADER) +
                      sizeof (EFI_MEMORY_ARRAY_LOCATION_DATA)
                      );
  if (EFI_ERROR (Status)) {
    goto CleanAndExit;
  }

  //
  // Get Memory size parameters for each rank from the chipset registers
  //

  Status = gBS->LocateProtocol (&MemInfoProtocolGuid, NULL, (VOID **) &MemInfoHob);
  ASSERT_EFI_ERROR (Status);
  //
  // We start from a base address of 0 for rank 0. We read the DRAM Row
  // Boundary Registers to find the end address of each rank.
  // Now, the size of each rank can be computed from subtracting the base
  // address of that rank from the end address read from the DRB. The
  // base address variable is then incremented by the size of each rank.
  //

  BaseAddress = 0;

  //
  // Channel 0
  //
  for (DimmIndex = 0; DimmIndex < NumSlots; DimmIndex++) {
    RowConfArray[DimmIndex].BaseAddress = BaseAddress;
    RowConfArray[DimmIndex].RowLength = LShiftU64(MemInfoHob->MemInfoData.dimmSize[DimmIndex], 20);
    BaseAddress += RowConfArray[DimmIndex].RowLength;
    DEBUG ((EFI_D_ERROR, "SMBIOS,MemInfoHob->MemInfoData.dimmSize[DimmIndex] %x \n", MemInfoHob->MemInfoData.dimmSize[DimmIndex]));
  }

  //support only DDR3
  Type = MemInfoHob->MemInfoData.ddrType;
  SerialNumStart  = 122;
  PartNumber      = 128;
  spdadressstep   = 4;

  spdadressstep   = 2;
  //
  // For each socket whether it is populated or not
  // generate Type 17, 19 and 20 structures
  //
  for (Dimm = 0; Dimm < NumSlots; Dimm++) {

    //
    // Generate Memory Device info (Type 17)
    //

    //
    // Read the SPD for this DIMM
    //
    ZeroMem (SmbusBuffer, SmbusBufferSize);
    SmbusSlaveAddress.SmbusDeviceAddress = (DIMM0_SPD_ADDRESS + (Dimm * spdadressstep)) >> 1;
    DimmPresent = TRUE;

    if (((Type == DDRType_LPDDR3) || (Type == DDRType_LPDDR2) || (Type == DDRType_DDR3L)) && (MemInfoHob->MemInfoData.dimmSize[Dimm]!=0)) {
      DimmPresent =  TRUE;
    } else if(Type < DDRType_DDR3All) {

      //
      // Read only needed values to save time
      //
      for (i = 0; i < sizeof SpdAddress; i++) {
        SmbusOffset = SpdAddress[i];
        SmbusLength = 2;
        Status = SmbusController->Execute (
                                    SmbusController,
                                    SmbusSlaveAddress,
                                    SmbusOffset,
                                    EfiSmbusReadWord,
                                    FALSE,
                                    &SmbusLength,
                                    &SmbusBuffer[SmbusOffset]
                                    );
        if (EFI_ERROR (Status)) {
          DimmPresent = FALSE;
          break;
        }
      }

    } else {
      DimmPresent = FALSE;
    }
    ZeroMem(&MemorySubClassData, sizeof (EFI_MEMORY_SUBCLASS_DRIVER_DATA));

    //
    // Use SPD data to generate Device Type info
    //
    MemorySubClassData.Header.Version     = EFI_MEMORY_SUBCLASS_VERSION;
    MemorySubClassData.Header.HeaderSize  = sizeof (EFI_SUBCLASS_TYPE1_HEADER);
    MemorySubClassData.Header.Instance    = ArrayInstance;
    MemorySubClassData.Header.SubInstance = (UINT16)(Dimm + 1);
    MemorySubClassData.Header.RecordType  = EFI_MEMORY_ARRAY_LINK_RECORD_NUMBER;

    MemorySubClassData.Record.ArrayLink.MemoryDeviceLocator               = DimmToDevLocator[Dimm];
    MemorySubClassData.Record.ArrayLink.MemoryBankLocator                 = DimmToBankLocator[Dimm];
    MemorySubClassData.Record.ArrayLink.MemoryAssetTag                    = STRING_TOKEN (STR_MEMORY_SUBCLASS_UNKNOWN);
    MemorySubClassData.Record.ArrayLink.MemoryArrayLink.ProducerName      = gEfiMemorySubClassDriverGuid;
    MemorySubClassData.Record.ArrayLink.MemoryArrayLink.Instance          = ArrayInstance;
    MemorySubClassData.Record.ArrayLink.MemoryArrayLink.SubInstance       = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
    MemorySubClassData.Record.ArrayLink.MemorySubArrayLink.ProducerName   = gEfiMemorySubClassDriverGuid;
    MemorySubClassData.Record.ArrayLink.MemorySubArrayLink.SubInstance    = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
    MemorySubClassData.Record.ArrayLink.MemoryFormFactor                  = EfiMemoryFormFactorSodimm;
    MemorySubClassData.Record.ArrayLink.MemoryType                        = EfiMemoryTypeDdr3;

    if (DimmPresent) {
      //
      // show known manufature name,show ID if the RAM module is unknow
      //
      StrCpy (StringBuffer, L"");
      //
      // calculate index counter
      //
      if(Type < DDRType_DDR3All) { // It's a DDR3
        //
        // get the number of continuation codes
        //
        IndexCounter = (SmbusBuffer[117] & 0x7f);

        //
        // converter memory manufacture ID to string
        //
        for (IdListIndex=0; MemoryModuleManufactureList[IdListIndex].Index!=0xff; IdListIndex++) {
          if(  MemoryModuleManufactureList[IdListIndex].Index == IndexCounter
               && MemoryModuleManufactureList[IdListIndex].ManufactureId == SmbusBuffer[118]) {
            StrCpy (StringBuffer, MemoryModuleManufactureList[IdListIndex].ManufactureName);
            break;
          }
        }

        //
        // use original data if no conversion information in conversion table
        //
        if (!(*StringBuffer))  {
          UnicodeValueToString (StringBuffer2, PREFIX_ZERO, SmbusBuffer[118], 2);
          StrCat (StringBuffer, StringBuffer2);
        }
      } else {
        /* For other dimm do not have SPD */
        StrCpy (StringBuffer, L"");
      }
      MemorySubClassData.Record.ArrayLink.MemoryManufacturer = (STRING_REF)0;
#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
      Status = IfrLibNewString (StringPackHandle,
                                &MemorySubClassData.Record.ArrayLink.MemoryManufacturer,
                                StringBuffer
                                );
      ASSERT_EFI_ERROR (Status);
#else
      Status = Hii->NewString (
                      Hii,
                      NULL,
                      HiiHandle,
                      &MemorySubClassData.Record.ArrayLink.MemoryManufacturer,
                      StringBuffer
                      );
      ASSERT_EFI_ERROR (Status);
#endif
#else
      Status = HiiString->NewString (
                            HiiString,
                            StringPackHandle,
                            &MemorySubClassData.Record.ArrayLink.MemoryManufacturer,
                            Language,
                            NULL,
                            StringBuffer,
                            NULL
                             );
      ASSERT_EFI_ERROR (Status);
#endif

      StrCpy(StringBuffer, L"");
      if(Type < DDRType_DDR3All) { // It's a DDR3
        for (Index = SerialNumStart; Index < SerialNumStart + 4; Index++) {
          UnicodeValueToString (StringBuffer2, PREFIX_ZERO, SmbusBuffer[Index], 2);
          StrCat (StringBuffer, StringBuffer2);
        }
      } else {
        /* For other dimm do not have SPD */
        StrCpy (StringBuffer, L"");
      }
      MemorySubClassData.Record.ArrayLink.MemorySerialNumber = (STRING_REF)0;
#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
      Status = IfrLibNewString (
                 StringPackHandle,
                 &MemorySubClassData.Record.ArrayLink.MemorySerialNumber,
                 StringBuffer
                 );
      ASSERT_EFI_ERROR (Status);
#else
      Status = Hii->NewString (
                      Hii,
                      NULL,
                      HiiHandle,
                      &MemorySubClassData.Record.ArrayLink.MemorySerialNumber,
                      StringBuffer
                      );
      ASSERT_EFI_ERROR (Status);
#endif
#else
      Status = HiiString->NewString (
                            HiiString,
                            StringPackHandle,
                            &MemorySubClassData.Record.ArrayLink.MemorySerialNumber,
                            Language,
                            NULL,
                            StringBuffer,
                            NULL
                            );
      ASSERT_EFI_ERROR (Status);
#endif

      StrCpy(StringBuffer, L"");
      if(Type < DDRType_DDR3All) { // It's a DDR3
        for (Index = PartNumber; Index < PartNumber + 18; Index++) {
          UnicodeSPrint(
            StringBuffer2,
            4,
            L"%c",
            SmbusBuffer[Index]);
          StrCat (StringBuffer, StringBuffer2);
        }
      } else {
        /* For other dimm do not have SPD */
        StrCpy (StringBuffer, L"");
      }
      MemorySubClassData.Record.ArrayLink.MemoryPartNumber = (STRING_REF)0;
#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
      Status = IfrLibNewString (StringPackHandle,
                                &MemorySubClassData.Record.ArrayLink.MemoryPartNumber,
                                StringBuffer
                                );
      ASSERT_EFI_ERROR (Status);
#else
      Status = Hii->NewString (
                      Hii,
                      NULL,
                      HiiHandle,
                      &MemorySubClassData.Record.ArrayLink.MemoryPartNumber,
                      StringBuffer
                      );
      ASSERT_EFI_ERROR (Status);
#endif
#else
      Status = HiiString->NewString (
                            HiiString,
                            StringPackHandle,
                            &MemorySubClassData.Record.ArrayLink.MemoryPartNumber,
                            Language,
                            NULL,
                            StringBuffer,
                            NULL
                            );
      ASSERT_EFI_ERROR (Status);
#endif

      //
      // Chipset Specific
      //
      if(Type < DDRType_DDR3All) { // It's a DDR3

        PrimaryBusBandwidth = 1 << (3 +(SmbusBuffer[8] & 0x07));

        MemorySubClassData.Record.ArrayLink.MemoryDataWidth = PrimaryBusBandwidth;
        MemorySubClassData.Record.ArrayLink.MemoryTotalWidth = PrimaryBusBandwidth;
        //
        // Check for ECC and Parity.
        //
        if ((SmbusBuffer[8] & (BIT4 | BIT3)) == 0x01 ) {
          MemorySubClassData.Record.ArrayLink.MemoryTotalWidth += 8;
        }
        //
        // Calculate the DIMM size
        DimmMemorySize  = RowConfArray[Dimm].RowLength;
        //DimmMemorySize  = RowConfArray[Dimm * 2].RowLength + RowConfArray[(Dimm * 2) + 1].RowLength;
        DEBUG ((EFI_D_ERROR, "SMBIOS,DimmMemorySize %x \n", DimmMemorySize));
      } else { // for LPDDR3 and other
        //MemorySubClassData.Record.ArrayLink.MemoryDataWidth = PrimaryBusBandwidth;
        //MemorySubClassData.Record.ArrayLink.MemoryTotalWidth = PrimaryBusBandwidth;
        // Calculate the DIMM size
        DimmMemorySize  = RowConfArray[Dimm].RowLength;
        //DimmMemorySize  = RowConfArray[Dimm * 2].RowLength + RowConfArray[(Dimm * 2) + 1].RowLength;
        DEBUG ((EFI_D_ERROR, "SMBIOS,DimmMemorySize %x \n", DimmMemorySize));
      }

      TotalMemorySize += DimmMemorySize;

      MemorySubClassData.Record.ArrayLink.MemoryDeviceSize              = DimmMemorySize;
      MemorySubClassData.Record.ArrayLink.MemoryTypeDetail.Synchronous  = 1;
      //temp hardcode to 800MHz
      Freq = MemInfoHob->MemInfoData.ddrFreq;

      switch  (Freq) {
        case FREQ_800:
          MemorySubClassData.Record.ArrayLink.MemorySpeed = 800;
          break;
        case FREQ_1066:
          MemorySubClassData.Record.ArrayLink.MemorySpeed = 1066;
          break;
        case FREQ_1333:
          MemorySubClassData.Record.ArrayLink.MemorySpeed = 1333;
          break;
        case FREQ_1600:
          MemorySubClassData.Record.ArrayLink.MemorySpeed = 1600;
          break;
        default:
          //bugbug: should we assert or otherwise indicate error condition?
          MemorySubClassData.Record.ArrayLink.MemorySpeed = 0;
          break;
      }

      switch  (Type) {
        case DDRType_LPDDR2:
          MemorySubClassData.Record.ArrayLink.MemoryType = EfiMemoryTypeDdr2;
          break;

        case DDRType_DDR3:
        case DDRType_DDR3L:
        case DDRType_DDR3U:
        case DDRType_LPDDR3:
          MemorySubClassData.Record.ArrayLink.MemoryType = EfiMemoryTypeDdr3;
        default:
          //bugbug: should we assert or otherwise indicate error condition?
          MemorySubClassData.Record.ArrayLink.MemoryType = EfiMemoryTypeUnknown;
          break;
      }

    } else {
      StrCpy(StringBuffer, L"");
      MemorySubClassData.Record.ArrayLink.MemoryManufacturer = (STRING_REF)0;
#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
      Status = IfrLibNewString (
                 StringPackHandle,
                 &MemorySubClassData.Record.ArrayLink.MemoryManufacturer,
                 StringBuffer
                 );
      ASSERT_EFI_ERROR (Status);
#else
      Status = Hii->NewString (
                      Hii,
                      NULL,
                      HiiHandle,
                      &MemorySubClassData.Record.ArrayLink.MemoryManufacturer,
                      StringBuffer
                      );
      ASSERT_EFI_ERROR (Status);
#endif
#else
      Status = HiiString->NewString (
                            HiiString,
                            StringPackHandle,
                            &MemorySubClassData.Record.ArrayLink.MemoryManufacturer,
                            Language,
                            NULL,
                            StringBuffer,
                            NULL
                            );
      ASSERT_EFI_ERROR (Status);
#endif

      MemorySubClassData.Record.ArrayLink.MemorySerialNumber = (STRING_REF)0;
#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
      Status = IfrLibNewString (StringPackHandle,
                                &MemorySubClassData.Record.ArrayLink.MemorySerialNumber,
                                StringBuffer
                                );
      ASSERT_EFI_ERROR (Status);
#else
      Status = Hii->NewString (
                      Hii,
                      NULL,
                      HiiHandle,
                      &MemorySubClassData.Record.ArrayLink.MemorySerialNumber,
                      StringBuffer
                      );
      ASSERT_EFI_ERROR (Status);
#endif
#else
      Status = HiiString->NewString (
                            HiiString,
                            StringPackHandle,
                            &MemorySubClassData.Record.ArrayLink.MemorySerialNumber,
                            Language,
                            NULL,
                            StringBuffer,
                            NULL
                            );
      ASSERT_EFI_ERROR (Status);
#endif

      MemorySubClassData.Record.ArrayLink.MemoryPartNumber = (STRING_REF)0;
#ifdef ECP_FLAG
#if (EFI_SPECIFICATION_VERSION > 0x20000)
      Status = IfrLibNewString (StringPackHandle,
                                &MemorySubClassData.Record.ArrayLink.MemoryPartNumber,
                                StringBuffer
                                );
      ASSERT_EFI_ERROR (Status);
#else
      Status = Hii->NewString (
                      Hii,
                      NULL,
                      HiiHandle,
                      &MemorySubClassData.Record.ArrayLink.MemoryPartNumber,
                      StringBuffer
                      );
      ASSERT_EFI_ERROR (Status);
#endif
#else
      Status = HiiString->NewString (
                            HiiString,
                            StringPackHandle,
                            &MemorySubClassData.Record.ArrayLink.MemoryPartNumber,
                            Language,
                            NULL,
                            StringBuffer,
                            NULL
                            );
      ASSERT_EFI_ERROR (Status);
#endif


      DimmMemorySize = 0;
      MemorySubClassData.Record.ArrayLink.MemorySpeed       = 0;
      MemorySubClassData.Record.ArrayLink.MemoryDeviceSize  = 0;
      MemorySubClassData.Record.ArrayLink.MemoryType        = EfiMemoryTypeUnknown;

    }

    //
    // Generate Memory Device info (Type 17)
    //
    Status = DataHub->LogData (
                        DataHub,
                        &gEfiMemorySubClassGuid,
                        &gEfiMemorySubClassDriverGuid,
                        EFI_DATA_RECORD_CLASS_DATA,
                        &MemorySubClassData,
                        sizeof (EFI_SUBCLASS_TYPE1_HEADER) + sizeof (EFI_MEMORY_ARRAY_LINK_DATA)
                        );
    if (EFI_ERROR (Status)) {
      goto CleanAndExit;
    }

    if (DimmPresent) {

      //
      // Generate Memory Device Mapped Address info (Type 20)
      //
      MemorySubClassData.Header.Instance    = ArrayInstance;
      MemorySubClassData.Header.SubInstance = (UINT16) (Dimm + 1);
      MemorySubClassData.Header.RecordType  = EFI_MEMORY_DEVICE_START_ADDRESS_RECORD_NUMBER;

      MemorySubClassData.Record.DeviceStartAddress.MemoryDeviceStartAddress               = RowConfArray[Dimm].BaseAddress;
      //  MemorySubClassData.Record.DeviceStartAddress.MemoryDeviceStartAddress               = RowConfArray[Dimm * 2].BaseAddress;
      MemorySubClassData.Record.DeviceStartAddress.MemoryDeviceEndAddress                 = MemorySubClassData.Record.DeviceStartAddress.MemoryDeviceStartAddress + DimmMemorySize - 1;
      MemorySubClassData.Record.DeviceStartAddress.MemoryDevicePartitionRowPosition       = 0xFF;       // 1 or 2 will be applicable for lock step mode

      MemorySubClassData.Record.DeviceStartAddress.PhysicalMemoryDeviceLink.ProducerName  = gEfiMemorySubClassDriverGuid;
      MemorySubClassData.Record.DeviceStartAddress.PhysicalMemoryDeviceLink.Instance      = ArrayInstance;
      MemorySubClassData.Record.DeviceStartAddress.PhysicalMemoryDeviceLink.SubInstance   = (UINT16)(Dimm + 1);
      MemorySubClassData.Record.DeviceStartAddress.PhysicalMemoryArrayLink.ProducerName   = gEfiMemorySubClassDriverGuid;
      MemorySubClassData.Record.DeviceStartAddress.PhysicalMemoryArrayLink.Instance       = ArrayInstance;
      MemorySubClassData.Record.DeviceStartAddress.PhysicalMemoryArrayLink.SubInstance    = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;

      //
      // Generate Memory Device Mapped Address info (Type 20)
      //
      Status = DataHub->LogData (
                          DataHub,
                          &gEfiMemorySubClassGuid,
                          &gEfiMemorySubClassDriverGuid,
                          EFI_DATA_RECORD_CLASS_DATA,
                          &MemorySubClassData,
                          sizeof (EFI_SUBCLASS_TYPE1_HEADER) + sizeof (EFI_MEMORY_DEVICE_START_ADDRESS_DATA)
                          );
      if (EFI_ERROR (Status)) {
        goto CleanAndExit;
      }
    }
  }

  //
  // Generate Memory Array Mapped Address info (TYPE 19)
  //
  MemorySubClassData.Header.Instance    = ArrayInstance;
  MemorySubClassData.Header.SubInstance = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
  MemorySubClassData.Header.RecordType  = EFI_MEMORY_ARRAY_START_ADDRESS_RECORD_NUMBER;

  MemorySubClassData.Record.ArrayStartAddress.MemoryArrayStartAddress               = 0;
  MemorySubClassData.Record.ArrayStartAddress.MemoryArrayEndAddress                 = TotalMemorySize - 1;
  MemorySubClassData.Record.ArrayStartAddress.PhysicalMemoryArrayLink.ProducerName  = gEfiMemorySubClassDriverGuid;
  MemorySubClassData.Record.ArrayStartAddress.PhysicalMemoryArrayLink.Instance      = ArrayInstance;
  MemorySubClassData.Record.ArrayStartAddress.PhysicalMemoryArrayLink.SubInstance   = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
  MemorySubClassData.Record.ArrayStartAddress.MemoryArrayPartitionWidth             = (UINT16)(NumSlots);

  //
  // Generate Memory Array Mapped Address info (TYPE 19)
  //
  Status = DataHub->LogData (
                      DataHub,
                      &gEfiMemorySubClassGuid,
                      &gEfiMemorySubClassDriverGuid,
                      EFI_DATA_RECORD_CLASS_DATA,
                      &MemorySubClassData,
                      sizeof (EFI_SUBCLASS_TYPE1_HEADER) + sizeof (EFI_MEMORY_ARRAY_START_ADDRESS_DATA)
                      );

CleanAndExit:
  (gBS->FreePool) (SmbusBuffer);
  (gBS->FreePool) (StringBuffer);
  return Status;
}
