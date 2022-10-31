/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/**@file
  Esrt driver that publishes ESRT table to OS. Aslo install EfiEsrtOperationProtocol for other component
  to update ESRT table.

**/

#include "EsrtDxe.h"
#include <Guid/Vlv2Variable.h>

ESRT_OPERATION_PROTOCOL    mEsrtOperationProtocol = {
    EsrtPopulateTable,
    EsrtUpdateTableEntryByGuid,
    EsrtGetFwEntryByGuid
};

CONST UINT16 ESRT_MAX_ENTRY=256;
extern EFI_GUID  gEfiEsrtTableGuid;
extern EFI_GUID  gEfiEsrtOperationProtocolGuid;

EFI_GUID gSystemFirmwareGuid = { 0x3bbdb30d, 0xaa9b, 0x4915, { 0x95, 0x03, 0xe4, 0xb8, 0x2f, 0xb6, 0xb6, 0xe9 }}; 
EFI_GUID gSHBFirmwareGuid = { 0x28442815, 0x3981, 0x2336, { 0x17, 0x15, 0x66, 0x22, 0x59, 0x67, 0x18, 0x97 }}; //SHB Firmware GUID.
EFI_GUID gULPMCFirmwareGuid = { 0x5A1C0D8B, 0x8BB8, 0x4A4F, {0xBC, 0xC6, 0xAC, 0x57, 0x0D, 0x54, 0x32, 0x94 }};
//
// Function implemenations
//
EFI_STATUS
EFIAPI
EsrtDriverEntryPoint(
    IN EFI_HANDLE             ImageHandle,
    IN EFI_SYSTEM_TABLE       *SystemTable
)
/*++

Routine Description:

    Entry point for the DFU Esrt Driver. Will initially creates the ESRT table from local variable
    and publish it. Also expose a series of APIs for UpdateCapsule service to update the table entry.

Arguments:

    ImageHandle       Image handle of this driver.
    SystemTable       Global system service table.

Returns:

    EFI_SUCCESS           Initialization complete.
    EFI_UNSUPPORTED       The chipset is unsupported by this driver.
    EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
    EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
{
    EFI_STATUS                Status;
    EFI_CONFIGURATION_TABLE   *EfiConfigurationTable;
    EFI_SYSTEM_RESOURCE_TABLE *EfiEsrtTable = NULL;
    FW_RES_ENTRY              *FwEntryTable = NULL;
    UINT32                    TotalFwResCount = 0;
    UINT32                    FwEntryIdx = 0;
    BOOLEAN                            MatchFound = FALSE;
    FW_RES_ENTRY_LIST         FwEntryList;
    UINTN                     Size = sizeof(UINT32) + 3*sizeof(FW_RES_ENTRY);	//(CSP20130927B)

    DEBUG ((EFI_D_ERROR, "EsrtDriverEntryPoint Start ..\n"));

    Status = EfiGetSystemConfigurationTable (
              &gEfiEsrtTableGuid,
              (VOID**)&EfiConfigurationTable
              );
    
    if (EFI_ERROR(Status) || EfiConfigurationTable == NULL) {
      DEBUG ((EFI_D_ERROR, "Failed to get EsrtTable ..\n"));
      FwEntryList.NumEntries = 0;
      FwEntryIdx = 0;
    } else {
      EfiEsrtTable = (EFI_SYSTEM_RESOURCE_TABLE *)(UINTN)EfiConfigurationTable;
      FwEntryTable = (FW_RES_ENTRY *)((UINTN)EfiConfigurationTable + sizeof(EFI_SYSTEM_RESOURCE_TABLE));

      TotalFwResCount = EfiEsrtTable->FwResourceCount;

      if (TotalFwResCount < 1) {
        return EFI_DEVICE_ERROR;
      }

      FwEntryList.NumEntries = TotalFwResCount;
    
      for (FwEntryIdx = 0; FwEntryIdx < TotalFwResCount; FwEntryIdx++) {
        FwEntryList.FwEntries[FwEntryIdx].FwClass = FwEntryTable[FwEntryIdx].FwClass;
        FwEntryList.FwEntries[FwEntryIdx].FwType = FwEntryTable[FwEntryIdx].FwType;
        FwEntryList.FwEntries[FwEntryIdx].FwVersion = FwEntryTable[FwEntryIdx].FwVersion;
        FwEntryList.FwEntries[FwEntryIdx].FwLstCompatVersion = FwEntryTable[FwEntryIdx].FwLstCompatVersion;
        FwEntryList.FwEntries[FwEntryIdx].CapsuleFlags = FwEntryTable[FwEntryIdx].CapsuleFlags;
        FwEntryList.FwEntries[FwEntryIdx].LastAttemptVersion = FwEntryTable[FwEntryIdx].LastAttemptVersion;
        FwEntryList.FwEntries[FwEntryIdx].LastAttemptStatus = FwEntryTable[FwEntryIdx].LastAttemptStatus;
      }
    }
    
    DEBUG ((EFI_D_ERROR, "FwEntryIdx = %x ..\n", FwEntryIdx));

    FwEntryList.NumEntries += 1;
    FwEntryList.FwEntries[FwEntryIdx].FwClass = gSystemFirmwareGuid;
    FwEntryList.FwEntries[FwEntryIdx].FwType = 0x01;
    FwEntryList.FwEntries[FwEntryIdx].FwVersion = 0x01;
    FwEntryList.FwEntries[FwEntryIdx].FwLstCompatVersion = 0;
    FwEntryList.FwEntries[FwEntryIdx].CapsuleFlags = 0;
    FwEntryList.FwEntries[FwEntryIdx].LastAttemptVersion = 0x01;
    FwEntryList.FwEntries[FwEntryIdx].LastAttemptStatus = 0;

    Status = gRT->SetVariable (
                    L"FwEntry",
                    &gEfiVlv2VariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    Size,
                    &FwEntryList
                    );

    if (EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }

    Status = EsrtPopulateTable();

    if(EFI_ERROR(Status)) {
        return Status;
    }

    Status = gBS->InstallMultipleProtocolInterfaces(
                 &ImageHandle,
                 &gEfiEsrtOperationProtocolGuid,
                 &mEsrtOperationProtocol,
                 NULL);

    return Status;
}

/**++
Routine Description
*/
EFI_STATUS
EsrtPopulateTable(
)
{
    EFI_STATUS                    Status;
    EFI_CONFIGURATION_TABLE       *EfiConfigurationTable;
    EFI_SYSTEM_RESOURCE_TABLE     *EfiEsrtTable = NULL;
    EFI_PHYSICAL_ADDRESS          PhysicalAddress;
    FW_RES_ENTRY                  *FwEntryTable = NULL;
    FW_RES_ENTRY_LIST             FwEntryList;
    UINTN                         Size;
    UINT32                        FwEntryCounts = 0;
    UINT32                        FwEntryIdx;

    //
    // First publish the Esrt table from local variable.
    // don't assume max 256 entries. Call GetVariable first to get the size of the table.
    //
    Size = sizeof(FW_RES_ENTRY)*ESRT_MAX_ENTRY + sizeof(UINT32);

    Status = gRT->GetVariable (
                    L"FwEntry",
                    &gEfiVlv2VariableGuid,
                    NULL,
                    &Size,
                    &FwEntryList
                    );
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }

    FwEntryCounts = FwEntryList.NumEntries;
    //
    // Get from the variable maximum entries required to set up ESRT table
    //
    if (FwEntryCounts == 0 || FwEntryCounts >ESRT_MAX_ENTRY) {
      //
      // Entries must be between 0 & 256;
      //
      return EFI_UNSUPPORTED;
    }

    Status = EfiGetSystemConfigurationTable (
              &gEfiEsrtTableGuid,
              (VOID**)&EfiConfigurationTable
              );

    if (!EFI_ERROR(Status)) {
      //
      // ESRT table exists. Need to remvoe it first.
      //
      Status = gBS->InstallConfigurationTable (
                      &gEfiEsrtTableGuid,
                      (VOID**)NULL
                      );

      if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
      }
    }

    PhysicalAddress = 0xffffffff;
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiReservedMemoryType,
                    EFI_SIZE_TO_PAGES(sizeof(FW_RES_ENTRY)*FwEntryCounts + sizeof(EFI_SYSTEM_RESOURCE_TABLE)),
                    &PhysicalAddress
                    );
    if (EFI_ERROR(Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Insert Table header first.
    //
    EfiEsrtTable = (EFI_SYSTEM_RESOURCE_TABLE *)(UINTN)PhysicalAddress;
    FwEntryTable = (FW_RES_ENTRY *)((UINTN)PhysicalAddress + sizeof(EFI_SYSTEM_RESOURCE_TABLE));

    EfiEsrtTable->FwResourceCount = FwEntryCounts;
    EfiEsrtTable->FwResourceMax = ESRT_MAX_ENTRY;
    EfiEsrtTable->FwResourceVersion = 0x01;

    //
    // Continue to update FW Resource entries.
    //
    for (FwEntryIdx=0; FwEntryIdx < FwEntryCounts;  FwEntryIdx++) {
        FwEntryTable[FwEntryIdx].FwType = FwEntryList.FwEntries[FwEntryIdx].FwType;
        FwEntryTable[FwEntryIdx].FwVersion = FwEntryList.FwEntries[FwEntryIdx].FwVersion;
        FwEntryTable[FwEntryIdx].FwLstCompatVersion = FwEntryList.FwEntries[FwEntryIdx].FwLstCompatVersion;
        FwEntryTable[FwEntryIdx].CapsuleFlags = FwEntryList.FwEntries[FwEntryIdx].CapsuleFlags;
        FwEntryTable[FwEntryIdx].LastAttemptVersion = FwEntryList.FwEntries[FwEntryIdx].LastAttemptVersion;
        FwEntryTable[FwEntryIdx].LastAttemptStatus = FwEntryList.FwEntries[FwEntryIdx].LastAttemptStatus;
        FwEntryTable[FwEntryIdx].FwClass = FwEntryList.FwEntries[FwEntryIdx].FwClass;
    }

    //
    // Install the table
    //

    Status = gBS->InstallConfigurationTable(&gEfiEsrtTableGuid, (VOID *)EfiEsrtTable);

    if (EFI_ERROR(Status)) {
      //
      // Fail to install the table.
      //
      return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EsrtUpdateTableEntryByGuid(
    IN EFI_GUID       FwEntryGuid,
    IN FW_RES_ENTRY   *FwEntry
)
{
    //
    // This is a two step operation: first update the variable, then publish the table again.
    //
    EFI_STATUS                    Status;
    FW_RES_ENTRY_LIST             FwEntryList;
    UINTN                         Size;
    UINT32                        FwEntryCounts = 0;
    UINT32                        FwEntryIdx;
    BOOLEAN                       MatchFound = FALSE;

    //
    // First publish the Esrt table from local variable.
    //
    Size = sizeof(FW_RES_ENTRY)*ESRT_MAX_ENTRY + sizeof(UINT32);;
    ZeroMem(&FwEntryList, Size);

    Status = gRT->GetVariable (
                    L"FwEntry",
                    &gEfiVlv2VariableGuid,
                    NULL,
                    &Size,
                    &FwEntryList
                    );
    if (EFI_ERROR(Status)) {
      //
      // Failed to find ESRT entries. ESRT tables cannot be published.
      //
      return EFI_UNSUPPORTED;
    }

    FwEntryCounts = FwEntryList.NumEntries;

    //
    // Get from the variable maximum entries required to set up ESRT table
    //
    if (FwEntryCounts == 0 || FwEntryCounts > ESRT_MAX_ENTRY) {
      //
      // Entries must be between 0 & 256;
      //
      return EFI_UNSUPPORTED;
    }

    for (FwEntryIdx =0; FwEntryIdx < FwEntryCounts; FwEntryIdx++) {
      if (!CompareGuid(&(FwEntryList.FwEntries[FwEntryIdx].FwClass), &FwEntryGuid)) {
        continue;
      } else {
        //
        // Match found. Modify it directly or delete it.
        MatchFound = TRUE;
        if (FwEntry!=NULL) {
          //
          // Modify
          //
          CopyMem(&FwEntryList.FwEntries[FwEntryIdx],FwEntry, sizeof(FW_RES_ENTRY));
        } else {
          //
          // Delete request, if it's not the last record, then replace it with last record, and reduce NumEntries by 1.
          //
          if (FwEntryIdx < FwEntryCounts - 1) {
            CopyMem(&FwEntryList.FwEntries[FwEntryIdx], &FwEntryList.FwEntries[FwEntryCounts-1], sizeof(FW_RES_ENTRY));
          }
          FwEntryList.NumEntries -= 1;
        }
        break;
      }
    }

    if (!MatchFound && FwEntry!=NULL) {
      //
      // No match. This is an add request.
      // Append it to the last of the table and then increase NumEntries by 1.
      //
      if (FwEntryCounts < ESRT_MAX_ENTRY) {
        CopyMem(&FwEntryList.FwEntries[FwEntryCounts+1],FwEntry, sizeof(FW_RES_ENTRY));
        FwEntryList.NumEntries += 1;
      } else {
        return EFI_OUT_OF_RESOURCES;
      }
    }

    if (FwEntryList.NumEntries > 0) {
      //
      // Publish the table now.
      //
      Size = sizeof(FW_RES_ENTRY)*FwEntryList.NumEntries+ 4;

      Status = gRT->SetVariable (
                      L"FwEntry",
                      &gEfiVlv2VariableGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                      Size,
                      &FwEntryList
                      );

      if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
      } else {
        Status = EsrtPopulateTable();
        return Status;
      }
    } else {
      return EFI_DEVICE_ERROR;
    }
}

EFI_STATUS
EsrtGetFwEntryByGuid(
    IN EFI_GUID                 FwEntryGuid,
    OUT FW_RES_ENTRY            *FwEntry
)
{
    EFI_STATUS Status;
    EFI_CONFIGURATION_TABLE     *EfiConfigurationTable;
    EFI_SYSTEM_RESOURCE_TABLE   *EfiEsrtTable = NULL;
    FW_RES_ENTRY                *FwEntryTable = NULL;
    UINT32                      TotalFwResCount = 0;
    UINT32                      FwEntryIdx = 0;
    BOOLEAN                     MatchFound = FALSE;

    DEBUG ((EFI_D_ERROR, "EsrtGetFwEntryByGuid Start ..\n"));

    if (FwEntry == NULL) {
      return EFI_DEVICE_ERROR;
    }    
    //
    // The table is retrieved from system configuration table. Never retrieve this from variable.
    //
    Status = EfiGetSystemConfigurationTable (
              &gEfiEsrtTableGuid,
              (VOID**)&EfiConfigurationTable
              );

    if (EFI_ERROR(Status) || EfiConfigurationTable == NULL) {
      DEBUG ((EFI_D_ERROR, "Failed to get EsrtTable ..\n"));
      return Status;
    }

    EfiEsrtTable = (EFI_SYSTEM_RESOURCE_TABLE *)(UINTN)EfiConfigurationTable;
    FwEntryTable = (FW_RES_ENTRY *)((UINTN)EfiConfigurationTable + sizeof(EFI_SYSTEM_RESOURCE_TABLE));

    TotalFwResCount = EfiEsrtTable->FwResourceCount;

    if (TotalFwResCount < 1) {
      return EFI_DEVICE_ERROR;
    }

    for (FwEntryIdx = 0; FwEntryIdx < TotalFwResCount; FwEntryIdx++) {
      if(CompareGuid(&FwEntryGuid, &(FwEntryTable[FwEntryIdx].FwClass))) {
        //
        // Match
        //
        FwEntry->FwClass = FwEntryTable[FwEntryIdx].FwClass;
        FwEntry->FwType = FwEntryTable[FwEntryIdx].FwType;
        FwEntry->FwVersion = FwEntryTable[FwEntryIdx].FwVersion;
        FwEntry->FwLstCompatVersion = FwEntryTable[FwEntryIdx].FwLstCompatVersion;
        FwEntry->CapsuleFlags = FwEntryTable[FwEntryIdx].CapsuleFlags;
        FwEntry->LastAttemptVersion = FwEntryTable[FwEntryIdx].LastAttemptVersion;
        FwEntry->LastAttemptStatus = FwEntryTable[FwEntryIdx].LastAttemptStatus;
        FwEntry->FwClass = FwEntryTable[FwEntryIdx].FwClass;

        MatchFound = TRUE;
        break;
      }
    }

    if (MatchFound) {
      DEBUG ((EFI_D_ERROR, "GUID is found ..\n"));
      return EFI_SUCCESS;
    } else {
      DEBUG ((EFI_D_ERROR, "Failed to find GUID ..\n"));
      return EFI_NOT_FOUND;
    }
}
