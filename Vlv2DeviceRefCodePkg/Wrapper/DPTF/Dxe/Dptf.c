#/** @file
#    
#  Driver for DPTF Module
#  
#  Copyright (c) 2006 - 2012, Intel Corporation. <BR>
#  All rights reserved. This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#  
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#  
#**/

#include "Dptf.h"
#include "VlvAccess.h"
#include "token.h" //AMI_OVERRIDE - EIP144300 I2C port 3 IC_COMP_VERSION register not being set.
//#include "CpuIA32.h"
#include <Library/UefiLib.h>
#include <IndustryStandard/Acpi30.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiSupport.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/VlvPlatformPolicy.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/GlobalNvsArea.h>
#include <Library/DriverLib.h>

#include <Library/I2cLib.h>


EFI_GLOBAL_NVS_AREA_PROTOCOL *mGlobalNvsArea;
extern EFI_GUID               gEfiGlobalNvsAreaProtocolGuid;
extern EFI_GUID               gDptfAcpiTableGuid;
EFI_ACPI_DESCRIPTION_HEADER     *mCpuDptfTable  = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mSoCDptfTable  = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mLpmDptfTable  = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mDptfTable     = NULL;

DPTF_DRIVER_DATA  gDptfDrvData;

typedef struct _LPAT_ENTRY {
  UINT16 TemperatureInKelvin;
  UINT16 RawTemperature;
} LPAT_ENTRY;

LPAT_ENTRY LinearApproxTable[] = {
  // LPAT table for a real sensor would be a 1:1 mapping in LPAT table.
  //Temp, Raw Value
    {2531,   977},
    {2581,   961},
    {2631,   941},
    {2681,   917},
    {2731,   887},
    {2781,   853},
    {2831,   813},
    {2881,   769},
    {2931,   720},
    {2981,   669},
    {3031,   615},
    {3081,   561},
    {3131,   508},
    {3181,   456},
    {3231,   407},
    {3281,   357},
    {3331,   315},
    {3381,   277},
    {3431,   243},
    {3481,   212},
    {3531,   186},
    {3581,   162},
    {3631,   140},
    {3731,   107}
};

VOID
CheckHotAndShutdown();

VOID
CheckAndEnableFailSafe();

VOID
SocThermInit ();


/**
  Uinstalls a SSDT ACPI Table

  @retval EFI_SUCCESS             Operation completed successfully.
  @retval other                   Some error occurred when executing this function.

**/
EFI_STATUS
UninstallSSDTAcpiTable(
  UINT64  OemTableId
  )
{
  EFI_STATUS                    Status;
  UINTN                         Count = 0;
  UINTN                         Handle;
  EFI_ACPI_DESCRIPTION_HEADER   *Table;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_SUPPORT_PROTOCOL     *mAcpiSupport = NULL;

  //
  // Locate ACPI Support protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, &mAcpiSupport);
  ASSERT_EFI_ERROR (Status);
  
  if (!EFI_ERROR(Status)){
    do {
      Status = mAcpiSupport->GetAcpiTable (mAcpiSupport, Count, &Table, &Version, &Handle);
      if (EFI_ERROR(Status)) {
        break;
      }
      //
      // Check if this is a DPTF SSDT table. If so, uninstall the table
      //
      if (Table->Signature == EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE && 
          Table->OemTableId == OemTableId) {
         DEBUG((EFI_D_INFO, "DPTF SSDT Table found.Uninstalling it\n"));
         Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, NULL, TRUE, Version, &Handle);
         ASSERT_EFI_ERROR(Status);
         Status = mAcpiSupport->PublishTables(
                                  mAcpiSupport,
                                  EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0
                                  );
         ASSERT_EFI_ERROR(Status);
         break;
        }
      Count++;
    } while (1);
  }
  
  return Status;
}

/**
  Uninstalls DPTF ACPI Tables

  @retval EFI_SUCCESS             Operation completed successfully.
  @retval other                   Some error occurred when executing this function.

**/

EFI_STATUS
UnInstallDptfAcpiTables(
  VOID
)
{
  EFI_STATUS Status;

  Status = UninstallSSDTAcpiTable(DPTF_ACPI_CPU_TABLE_SIGNATURE);
  Status = UninstallSSDTAcpiTable(DPTF_ACPI_SOC_TABLE_SIGNATURE);
  Status = UninstallSSDTAcpiTable(DPTF_ACPI_LPM_TABLE_SIGNATURE);
  Status = UninstallSSDTAcpiTable(DPTF_ACPI_GEN_TABLE_SIGNATURE);
  
  return Status;
}

/**
  Installs DPTF ACPI Tables

  @retval EFI_SUCCESS             Operation completed successfully.
  @retval other                   Some error occurred when executing this function.

**/

EFI_STATUS
InstallDptfAcpiTables(
  VOID
)
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  EFI_FV_FILETYPE               FileType;
  UINT32                        FvStatus;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINTN                         Size;
  UINTN                         i;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol = NULL;
  INTN                          Instance;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_COMMON_HEADER        *CurrentTable;
  EFI_ACPI_DESCRIPTION_HEADER   *TempTable;
  UINTN                         AcpiTableHandle;
  EFI_ACPI_SUPPORT_PROTOCOL     *mAcpiSupport = NULL;
  BOOLEAN                       bDptf=FALSE;
  BOOLEAN                       bCpuDptf=FALSE;
  BOOLEAN                       bSoCDptf=FALSE;
  BOOLEAN                       bLpmDptf=FALSE;

  // Locate protocol.
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, &mAcpiSupport);
  ASSERT_EFI_ERROR (Status);
  
  // There is little chance we can't find an FV protocol
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  // Looking for FV with ACPI storage file
  for (i = 0; i < NumberOfHandles; i++) {

    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    Status = gBS->HandleProtocol (
                  HandleBuffer[i],
                  &gEfiFirmwareVolume2ProtocolGuid,
                  (VOID **)&FwVol
                  );
    ASSERT_EFI_ERROR (Status);

    // See if it has the ACPI storage file
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                    FwVol,
                    &gDptfAcpiTableGuid,
                    NULL,
                    &Size,
                    &FileType,
                    &Attributes,
                    &FvStatus
                    );

    // If we found it, then we are done
    if (Status == EFI_SUCCESS) {
      break;
    }
  }

  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  // Free any allocated buffers
  gBS->FreePool (HandleBuffer);

  // Sanity check that we found our data file
  ASSERT (FwVol != NULL);
  if( FwVol == NULL ) {
    return EFI_SUCCESS;
  }
  // By default, a table belongs in all ACPI table versions published.
  Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  // Read tables from the storage file.
  Instance = 0;
  CurrentTable = NULL;
  while (Status == EFI_SUCCESS) {
    Status = FwVol->ReadSection (
                      FwVol,
                      &gDptfAcpiTableGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID **)&CurrentTable,
                      &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      // Check the table ID to modify the table
      switch (((EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable)->OemTableId) {

      case DPTF_ACPI_CPU_TABLE_SIGNATURE:
        bCpuDptf = TRUE;
        mCpuDptfTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
        break;

      case DPTF_ACPI_SOC_TABLE_SIGNATURE:
        bSoCDptf = TRUE;
        mSoCDptfTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
        break;

      case DPTF_ACPI_LPM_TABLE_SIGNATURE:
//        bLpmDptf = TRUE;
        mLpmDptfTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
        break;

      case DPTF_ACPI_GEN_TABLE_SIGNATURE:
        bDptf = TRUE;
        mDptfTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
        break;

      default:
        break;
      }

      // Increment the instance
      Instance++;
      CurrentTable = NULL;
    }
  }
  if(bCpuDptf) {
    Status = gBS->AllocatePool (EfiReservedMemoryType, mCpuDptfTable->Length, &TempTable);
    ASSERT_EFI_ERROR (Status);
    CopyMem (TempTable, mCpuDptfTable, mCpuDptfTable->Length);
    gBS->FreePool (mCpuDptfTable);
    mCpuDptfTable = TempTable;
    AcpiChecksum (mCpuDptfTable, mCpuDptfTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));
    AcpiTableHandle = 0;
    Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mCpuDptfTable, TRUE, Version, &AcpiTableHandle);
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (mCpuDptfTable);
  }
  
  if(bSoCDptf) {
    Status = gBS->AllocatePool (EfiReservedMemoryType, mSoCDptfTable->Length, &TempTable);
    ASSERT_EFI_ERROR (Status);
    CopyMem (TempTable, mSoCDptfTable, mSoCDptfTable->Length);
    gBS->FreePool (mSoCDptfTable);
    mSoCDptfTable = TempTable;
    AcpiChecksum (mSoCDptfTable, mSoCDptfTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));
    AcpiTableHandle = 0;
    Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mSoCDptfTable, TRUE, Version, &AcpiTableHandle);
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (mSoCDptfTable);
  }
  
  if(bLpmDptf) {
    Status = gBS->AllocatePool (EfiReservedMemoryType, mLpmDptfTable->Length, &TempTable);
    ASSERT_EFI_ERROR (Status);
    CopyMem (TempTable, mLpmDptfTable, mLpmDptfTable->Length);
    gBS->FreePool (mLpmDptfTable);
    mLpmDptfTable = TempTable;
    AcpiChecksum (mLpmDptfTable, mLpmDptfTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));
    AcpiTableHandle = 0;
    Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mLpmDptfTable, TRUE, Version, &AcpiTableHandle);
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (mLpmDptfTable);
  }
  
  if(bDptf) {
    Status = gBS->AllocatePool (EfiReservedMemoryType, mDptfTable->Length, &TempTable);
    ASSERT_EFI_ERROR (Status);
    CopyMem (TempTable, mDptfTable, mDptfTable->Length);
    gBS->FreePool (mDptfTable);
    mDptfTable = TempTable;
    AcpiChecksum (mDptfTable, mDptfTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));
    AcpiTableHandle = 0;
    Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mDptfTable, TRUE, Version, &AcpiTableHandle);
    ASSERT_EFI_ERROR (Status);
    gBS->FreePool (mDptfTable);
  }

  // Publish all ACPI Tables
  Status = mAcpiSupport->PublishTables (mAcpiSupport, Version);
  ASSERT_EFI_ERROR (Status);
  
  return Status;
}

/**
  Reads BIOS setup variable to determine if DPTF is enabled in BIOS Setup.

  @param[in, out] bDptfEnabled  TRUE, if DPTF is enabled in BIOS setup else FALSE  

  @retval EFI_SUCCESS             Operation completed successfully.
  @retval EFI_INVALID_PARAMETER   Input parameter is invalid
  @retval other                   Some error occurred when executing this function.

**/
EFI_STATUS
DPTFEnabledInBIOS(
  IN OUT  BOOLEAN *bDptfEnabled
)
{
  EFI_STATUS  Status  = EFI_SUCCESS;

  if (bDptfEnabled == NULL){
    return EFI_INVALID_PARAMETER;
  }

  *bDptfEnabled = gDptfDrvData.bDptfEnabled;
  
  return Status;
}

/**
  Ready to Boot Event notification handler.

  Sequence of OS boot events is measured in this event notification handler.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
OnReadyToBoot (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS  Status;
  BOOLEAN     bDptfEnabled = TRUE;

    
    //
    // Check if system is hot and shutdown if so
    //
//    CheckHotAndShutdown();

    //
    // program the critical trip points for system sensors into PMIC
    // if temperature exceeds critical value, PMIC will shutdown
    // 
#if (ENBDT_PF_ENABLE == 0) && (BYTI_PF_ENABLE == 0)
    // CriticalThermalTripPoint is written to EC in InitializeEcSmm, so not needed here.
    CheckAndEnableFailSafe();
#endif
    //
    // Check if DPTF is enabled in BIOS setup option
    //
    Status = DPTFEnabledInBIOS(&bDptfEnabled);
    
    if (!bDptfEnabled) {
      //
      // DPTF is disabled in BIOS Setup
      // Uninstall DPTF ACPI tables
      Status = UnInstallDptfAcpiTables();
    }
    else {
      //
      // Update Global NVS data
      //
      Status = gBS->LocateProtocol (&gEfiGlobalNvsAreaProtocolGuid, NULL, (VOID **) &mGlobalNvsArea);
      ASSERT_EFI_ERROR (Status);
  
      mGlobalNvsArea->Area->CriticalThermalTripPoint         = (UINT8)gDptfDrvData.CpuParticipantCriticalTemperature;
      mGlobalNvsArea->Area->PassiveThermalTripPoint          = (UINT8)gDptfDrvData.CpuParticipantPassiveTemperature;
      mGlobalNvsArea->Area->DptfGenericCriticalTemperature0        = gDptfDrvData.GenParticipant0CriticalTemperature;
      mGlobalNvsArea->Area->DptfGenericPassiveTemperature0         = gDptfDrvData.GenParticipant0PassiveTemperature;
      mGlobalNvsArea->Area->DptfGenericCriticalTemperature1        = gDptfDrvData.GenParticipant1CriticalTemperature;
      mGlobalNvsArea->Area->DptfGenericPassiveTemperature1         = gDptfDrvData.GenParticipant1PassiveTemperature;
      mGlobalNvsArea->Area->DptfGenericCriticalTemperature2        = gDptfDrvData.GenParticipant2CriticalTemperature;
      mGlobalNvsArea->Area->DptfGenericPassiveTemperature2         = gDptfDrvData.GenParticipant2PassiveTemperature;
      mGlobalNvsArea->Area->DptfGenericCriticalTemperature3        = gDptfDrvData.GenParticipant3CriticalTemperature;
      mGlobalNvsArea->Area->DptfGenericPassiveTemperature3         = gDptfDrvData.GenParticipant3PassiveTemperature;
      mGlobalNvsArea->Area->DptfGenericCriticalTemperature4        = gDptfDrvData.GenParticipant4CriticalTemperature;
      mGlobalNvsArea->Area->DptfGenericPassiveTemperature4         = gDptfDrvData.GenParticipant4PassiveTemperature;      
      mGlobalNvsArea->Area->CLpmSetting                            = gDptfDrvData.ClpmSetting;
      mGlobalNvsArea->Area->DptfCriticalThreshold0                 = gDptfDrvData.CriticalThreshold0ForScu;
      mGlobalNvsArea->Area->DptfCriticalThreshold1                 = gDptfDrvData.CriticalThreshold1ForScu;
      mGlobalNvsArea->Area->DptfCriticalThreshold2                 = gDptfDrvData.CriticalThreshold2ForScu;
      mGlobalNvsArea->Area->DptfCriticalThreshold3                 = gDptfDrvData.CriticalThreshold3ForScu;
      mGlobalNvsArea->Area->DptfCriticalThreshold4                 = gDptfDrvData.CriticalThreshold4ForScu;      
      mGlobalNvsArea->Area->DptfSuperDbg                           = gDptfDrvData.SuperDebug;
      mGlobalNvsArea->Area->LPOEnable                              = gDptfDrvData.LPOEnable;
      mGlobalNvsArea->Area->LPOStartPState                         = gDptfDrvData.LPOStartPState;
      mGlobalNvsArea->Area->LPOStepSize                            = gDptfDrvData.LPOStepSize;
      mGlobalNvsArea->Area->LPOPowerControlSetting                 = gDptfDrvData.LPOPowerControlSetting;
      mGlobalNvsArea->Area->LPOPerformanceControlSetting           = gDptfDrvData.LPOPerformanceControlSetting;
      mGlobalNvsArea->Area->DppmEnabled                            = (gDptfDrvData.bDppmEnabled == TRUE)? 0x1 : 0x0;
   
    //CSP20130719>>  
      //
      // DPTF is enabled in BIOS Setup
      // install DPTF ACPI tables
      Status = InstallDptfAcpiTables();
    }
    //CSP20130719<<

  gBS->CloseEvent(Event);
}

INT16
ReadTemperature(THERISTOR_REG_DEF *pSysThermRegs)
{
  UINT32  Index         = 0;
  UINT16  RawTemp       = 0;
  UINT16  TempIn10K     = 0;
  INT16   TempInCelsius = 0;

  if(pSysThermRegs != NULL) {
    RawTemp = pSysThermRegs->STR;
    //
    // Convert Raw Temperature to Degree Celsius
    //
    for (Index = 0; Index < (sizeof(LinearApproxTable)/sizeof(LPAT_ENTRY)-1); Index++) {
      if (LinearApproxTable[Index].RawTemperature >= RawTemp && LinearApproxTable[Index+1].RawTemperature <= RawTemp) {
        //
        // Raw temperature read out from thermistor lies between these entries
        //
        TempIn10K = LinearApproxTable[Index+1].TemperatureInKelvin + (RawTemp - LinearApproxTable[Index+1].RawTemperature) * (((LinearApproxTable[Index+1].TemperatureInKelvin-LinearApproxTable[Index].TemperatureInKelvin)/(LinearApproxTable[Index].RawTemperature-LinearApproxTable[Index+1].RawTemperature)));
        TempInCelsius = TempIn10K - 2732;
        TempInCelsius /= 10;
        break;
      }
    }
  }
  return TempInCelsius;
}

VOID
CheckHotAndShutdown()
{
  EFI_STATUS  Status;
  BOOLEAN     bDptfEnabled = TRUE;
  INT16       Temp = 0;
  BOOLEAN     bShutdown = FALSE;

  Status = DPTFEnabledInBIOS(&bDptfEnabled);
  if (bDptfEnabled) {
      Temp = ReadTemperature((THERISTOR_REG_DEF *)((UINTN)A0_SYS_THERM0_BASE_ADDRESS));
      if (Temp >= gDptfDrvData.GenParticipant0CriticalTemperature) {
        bShutdown = TRUE;
        goto chk_shutdown;
      }

      Temp = ReadTemperature((THERISTOR_REG_DEF *)((UINTN)A0_SYS_THERM1_BASE_ADDRESS));
      if (Temp >= gDptfDrvData.GenParticipant1CriticalTemperature) {
        bShutdown = TRUE;
        goto chk_shutdown;
      }

      Temp = ReadTemperature((THERISTOR_REG_DEF *)((UINTN)A0_SYS_THERM2_BASE_ADDRESS));
      if (Temp >= gDptfDrvData.GenParticipant2CriticalTemperature) {
        bShutdown = TRUE;
        goto chk_shutdown;
      }

      Temp = ReadTemperature((THERISTOR_REG_DEF *)((UINTN)A0_SYS_THERM3_BASE_ADDRESS));
      if (Temp >= gDptfDrvData.GenParticipant3CriticalTemperature) {
        bShutdown = TRUE;
        goto chk_shutdown;
      }
  }

chk_shutdown:
  if(bShutdown) {
    //
    // One of generic sensor (thermistors) temperature exceeded critical.
    // Shutdown the system
    //
    Print(L"One or more system thermistors temperature exceeds critical temperature.\nSystem will shutdown in %d seconds...\n", DPTF_SHUTDOWN_DELAY);
    gBS->Stall(DPTF_SHUTDOWN_DELAY*1000*1000);
    gRT->ResetSystem(
          EfiResetShutdown,
          EFI_SUCCESS,
          0,
          NULL);
  }
}

INT16
GetRawTempValue(INT16 TempInCelsius)
{
    UINT32  Index        = 0;
    INT16   TempIn10K    = 0;
    INT16   RawTempValue = 0;
    INT16   LastEntry = sizeof(LinearApproxTable)/sizeof(LPAT_ENTRY) - 1;
    
    TempIn10K = (TempInCelsius*10+2731);

    //
    // Return maximum Raw value if temperature is above the maximum.
    //
    if (TempIn10K >= LinearApproxTable[LastEntry].TemperatureInKelvin)
    	return LinearApproxTable[LastEntry].RawTemperature;
    	
    //
    // Return minimum Raw value if temperature is below the minimum.
    //
    if (TempIn10K <= LinearApproxTable[0].TemperatureInKelvin)
    	return LinearApproxTable[0].RawTemperature;
    	
    //
    // Convert Degree Celsius to Raw data
    //
    for (Index = 0; Index < (sizeof(LinearApproxTable)/sizeof(LPAT_ENTRY)-1); Index++) {
      if (LinearApproxTable[Index].TemperatureInKelvin <= TempIn10K && LinearApproxTable[Index+1].TemperatureInKelvin >= TempIn10K) {
        //
        // Temperature lies between these entries
        //
        RawTempValue = LinearApproxTable[Index].RawTemperature - ((TempIn10K - LinearApproxTable[Index].TemperatureInKelvin) * (LinearApproxTable[Index].RawTemperature - LinearApproxTable[Index+1].RawTemperature)/(LinearApproxTable[Index+1].TemperatureInKelvin - LinearApproxTable[Index].TemperatureInKelvin));
        break;
      }
    }
  return RawTempValue;
}

EFI_STATUS
InitializeDriverData()
{
  EFI_STATUS                Status;

  DXE_VLV_PLATFORM_POLICY_PROTOCOL*  pPlatformPolicy;

  Status = gBS->LocateProtocol (
                  &gDxeVlvPlatformPolicyGuid,
                  NULL,
                  &pPlatformPolicy);

  if(EFI_ERROR(Status) || (pPlatformPolicy->DptfSettings.ProcCriticalTemp == 0 && pPlatformPolicy->DptfSettings.EnableDptf == 0)) {
    // Setup options are invalid. Restore defaults
    DEBUG((EFI_D_WARN, "DPTF Setup options are invalid. Restoring defaults "));

    gDptfDrvData.bDptfEnabled                       = 0x1;
    gDptfDrvData.CpuParticipantCriticalTemperature  = PCT_DEFAULT;
    gDptfDrvData.CpuParticipantPassiveTemperature   = PPT_DEFAULT;
    gDptfDrvData.GenParticipant0CriticalTemperature = GCT0_DEFAULT;
    gDptfDrvData.GenParticipant0PassiveTemperature  = GPT0_DEFAULT;
    gDptfDrvData.GenParticipant1CriticalTemperature = GCT1_DEFAULT;
    gDptfDrvData.GenParticipant1PassiveTemperature  = GPT1_DEFAULT;
    gDptfDrvData.GenParticipant2CriticalTemperature = GCT2_DEFAULT;
    gDptfDrvData.GenParticipant2PassiveTemperature  = GPT2_DEFAULT;
    gDptfDrvData.GenParticipant3CriticalTemperature = GCT3_DEFAULT;
    gDptfDrvData.GenParticipant3PassiveTemperature  = GPT3_DEFAULT;
    gDptfDrvData.GenParticipant4CriticalTemperature = GCT4_DEFAULT;
    gDptfDrvData.GenParticipant4PassiveTemperature  = GPT4_DEFAULT;
    
    gDptfDrvData.ClpmSetting                        = 3;
    // critical trip point is 5C above BIOS setup critical value
    // reuse CriticalThresholdxForScu to set the critical trip point for PMIC sensors
    gDptfDrvData.CriticalThreshold0ForScu            =  (gDptfDrvData.GenParticipant0CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant0CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold1ForScu            =  (gDptfDrvData.GenParticipant1CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant1CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold2ForScu            =  (gDptfDrvData.GenParticipant2CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant2CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold3ForScu            =  (gDptfDrvData.GenParticipant3CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant3CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold4ForScu            =  (gDptfDrvData.GenParticipant4CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant4CriticalTemperature + 5) : GetRawTempValue(60);    
    gDptfDrvData.SuperDebug                         =  0x0;
    //CLPO Defaults
    gDptfDrvData.LPOEnable                          = CLPO_DEFAULT_ENABLE;
    gDptfDrvData.LPOStartPState                     = CLPO_DEFAULT_START_PSTATE;             
    gDptfDrvData.LPOStepSize                        = CLPO_DEFAULT_STEP_SIZE;                
    gDptfDrvData.LPOPowerControlSetting             = CLPO_DEFAULT_PWR_CTRL_SETTING;     
    gDptfDrvData.LPOPerformanceControlSetting       = CLPO_DEFAULT_PERF_CTRL_SETTING;
    gDptfDrvData.bDppmEnabled                       = DPPM_ENABLE_DEFAULT;
    }
    else {
    gDptfDrvData.SdpProfile                         = pPlatformPolicy->DptfSettings.SdpProfile;
    gDptfDrvData.bDptfEnabled                       = (pPlatformPolicy->DptfSettings.EnableDptf == 0x1)? TRUE: FALSE;
    gDptfDrvData.CpuParticipantCriticalTemperature  =  pPlatformPolicy->DptfSettings.ProcCriticalTemp;
    gDptfDrvData.CpuParticipantPassiveTemperature   =  pPlatformPolicy->DptfSettings.ProcPassiveTemp;
    gDptfDrvData.GenParticipant0CriticalTemperature  =  pPlatformPolicy->DptfSettings.GenericCriticalTemp0;
    gDptfDrvData.GenParticipant0PassiveTemperature   =  pPlatformPolicy->DptfSettings.GenericPassiveTemp0;
    gDptfDrvData.GenParticipant1CriticalTemperature  =  pPlatformPolicy->DptfSettings.GenericCriticalTemp1;
    gDptfDrvData.GenParticipant1PassiveTemperature   =  pPlatformPolicy->DptfSettings.GenericPassiveTemp1;
    gDptfDrvData.GenParticipant2CriticalTemperature  =  pPlatformPolicy->DptfSettings.GenericCriticalTemp2;
    gDptfDrvData.GenParticipant2PassiveTemperature   =  pPlatformPolicy->DptfSettings.GenericPassiveTemp2;
    gDptfDrvData.GenParticipant3CriticalTemperature  =  pPlatformPolicy->DptfSettings.GenericCriticalTemp3;
    gDptfDrvData.GenParticipant3PassiveTemperature   =  pPlatformPolicy->DptfSettings.GenericPassiveTemp3;
    gDptfDrvData.GenParticipant4CriticalTemperature  =  pPlatformPolicy->DptfSettings.GenericCriticalTemp4;
    gDptfDrvData.GenParticipant4PassiveTemperature   =  pPlatformPolicy->DptfSettings.GenericPassiveTemp4;    
    gDptfDrvData.ClpmSetting                        =  pPlatformPolicy->DptfSettings.Clpm;
    // critical trip point is 5C above BIOS setup critical value
    // reuse CriticalThresholdxForScu to set the critical trip point for PMIC sensors
    gDptfDrvData.CriticalThreshold0ForScu            =  (gDptfDrvData.GenParticipant0CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant0CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold1ForScu            =  (gDptfDrvData.GenParticipant1CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant1CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold2ForScu            =  (gDptfDrvData.GenParticipant2CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant2CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold3ForScu            =  (gDptfDrvData.GenParticipant3CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant3CriticalTemperature + 5) : GetRawTempValue(60);
    gDptfDrvData.CriticalThreshold4ForScu            =  (gDptfDrvData.GenParticipant4CriticalTemperature >= 60) ? GetRawTempValue(gDptfDrvData.GenParticipant4CriticalTemperature + 5) : GetRawTempValue(60);    
    gDptfDrvData.SuperDebug                         = pPlatformPolicy->DptfSettings.SuperDebug;
    gDptfDrvData.LPOEnable                          = pPlatformPolicy->DptfSettings.LPOEnable;                   
    gDptfDrvData.LPOStartPState                     = pPlatformPolicy->DptfSettings.LPOStartPState;              
    gDptfDrvData.LPOStepSize                        = pPlatformPolicy->DptfSettings.LPOStepSize;                 
    gDptfDrvData.LPOPowerControlSetting             = pPlatformPolicy->DptfSettings.LPOPowerControlSetting;      
    gDptfDrvData.LPOPerformanceControlSetting       = pPlatformPolicy->DptfSettings.LPOPerformanceControlSetting;
    gDptfDrvData.bDppmEnabled                       = (pPlatformPolicy->DptfSettings.EnableDppm == 0x1)? TRUE: FALSE;
  }

  Status = EFI_SUCCESS;

  gDptfDrvData.pSysTherm0BaseAddress = (THERISTOR_REG_DEF *) ((UINTN)A0_SYS_THERM0_BASE_ADDRESS);
  gDptfDrvData.pSysTherm1BaseAddress = (THERISTOR_REG_DEF *) ((UINTN)A0_SYS_THERM1_BASE_ADDRESS);
  gDptfDrvData.pSysTherm2BaseAddress = (THERISTOR_REG_DEF *) ((UINTN)A0_SYS_THERM2_BASE_ADDRESS);
  gDptfDrvData.pSysTherm3BaseAddress = (THERISTOR_REG_DEF *) ((UINTN)A0_SYS_THERM3_BASE_ADDRESS);

  DEBUG((EFI_D_INFO, "Dumping DPTF settings in dptf driver...\n"));
  DEBUG((EFI_D_INFO, "DPTFEnabled = %d\n", gDptfDrvData.bDptfEnabled));
  DEBUG((EFI_D_INFO, "CpuParticipantCriticalTemperature = %d\n", gDptfDrvData.CpuParticipantCriticalTemperature));
  DEBUG((EFI_D_INFO, "CpuParticipantPassiveTemperature = %d\n", gDptfDrvData.CpuParticipantPassiveTemperature));  
  DEBUG((EFI_D_INFO, "GenParticipant0CriticalTemperature = %d\n", gDptfDrvData.GenParticipant0CriticalTemperature));  
  DEBUG((EFI_D_INFO, "GenParticipant0PassiveTemperature = %d\n", gDptfDrvData.GenParticipant0PassiveTemperature));    
  DEBUG((EFI_D_INFO, "GenParticipant1CriticalTemperature = %d\n", gDptfDrvData.GenParticipant1CriticalTemperature));  
  DEBUG((EFI_D_INFO, "GenParticipant1PassiveTemperature = %d\n", gDptfDrvData.GenParticipant1PassiveTemperature));      
  DEBUG((EFI_D_INFO, "GenParticipant2CriticalTemperature = %d\n", gDptfDrvData.GenParticipant2CriticalTemperature));  
  DEBUG((EFI_D_INFO, "GenParticipant2PassiveTemperature = %d\n", gDptfDrvData.GenParticipant2PassiveTemperature));
  DEBUG((EFI_D_INFO, "GenParticipant3CriticalTemperature = %d\n", gDptfDrvData.GenParticipant3CriticalTemperature));  
  DEBUG((EFI_D_INFO, "GenParticipant3PassiveTemperature = %d\n", gDptfDrvData.GenParticipant3PassiveTemperature));
  DEBUG((EFI_D_INFO, "GenParticipant4CriticalTemperature = %d\n", gDptfDrvData.GenParticipant4CriticalTemperature));  
  DEBUG((EFI_D_INFO, "GenParticipant4PassiveTemperature = %d\n", gDptfDrvData.GenParticipant4PassiveTemperature));      

  DEBUG((EFI_D_INFO, "ClpmSetting = %d\n", gDptfDrvData.ClpmSetting));        
  DEBUG((EFI_D_INFO, "SuperDebug = %d\n", gDptfDrvData.SuperDebug));
  DEBUG((EFI_D_INFO, "LPOEnable = %d\n", gDptfDrvData.LPOEnable));
  DEBUG((EFI_D_INFO, "LPOStartPState = %d\n", gDptfDrvData.LPOStartPState));  
  DEBUG((EFI_D_INFO, "LPOStepSize = %d\n", gDptfDrvData.LPOStepSize));        
  DEBUG((EFI_D_INFO, "LPOPowerControlSetting = %d\n", gDptfDrvData.LPOPowerControlSetting));
  DEBUG((EFI_D_INFO, "LPOPerformanceControlSetting = %d\n", gDptfDrvData.LPOPerformanceControlSetting));  
  DEBUG((EFI_D_INFO, "bDppmEnabled = %d\n", gDptfDrvData.bDppmEnabled));
  DEBUG((EFI_D_INFO, "SdpProfile = %d\n", gDptfDrvData.SdpProfile));
  return Status;
}

VOID
CheckAndEnableFailSafe(
  )
{
  UINT8 Value=0, Vendor=0;
  EFI_STATUS rc=0;

  Value = (UINT8)(gDptfDrvData.CriticalThreshold0ForScu >> 1);
  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, PMIC_SYS0_THRMCRIT, 1, &Value);
  if (EFI_SUCCESS != rc)  {
      DEBUG ((DEBUG_ERROR, "DPTF:Failed to write sys0 thrmcrit.\r\n"));  
	  rc = EFI_ABORTED;
	  goto _exit;
  }
  
  Value = (UINT8)(gDptfDrvData.CriticalThreshold1ForScu >> 1);
  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, PMIC_SYS1_THRMCRIT, 1, &Value);
  if (EFI_SUCCESS != rc)  {
      DEBUG ((DEBUG_ERROR, "DPTF:Failed to write sys1 thrmcrit.\r\n"));    	
	  rc = EFI_ABORTED;
	  goto _exit;
  }

  Value = (UINT8)(gDptfDrvData.CriticalThreshold2ForScu >> 1);
  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, PMIC_SYS2_THRMCRIT, 1, &Value);
  if (EFI_SUCCESS != rc)  {
      DEBUG ((DEBUG_ERROR, "DPTF:Failed to write sys2 thrmcrit.\r\n"));    	
	  rc = EFI_ABORTED;
	  goto _exit;
  }

  rc = ByteReadI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, PMIC_ID0, 1, &Vendor);
  if (EFI_SUCCESS != rc)  {
      DEBUG ((DEBUG_ERROR, "DPTF:Failed to read PMIC Vendor type.\r\n"));    	
	  rc = EFI_ABORTED;
	  goto _exit;
  }

  Value = (Vendor == PMIC_VENDOR_ROHM ? 0x7 : 0x38);
  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, PMIC_TS_CRIT_ENABLE, 1, &Value);
  if (EFI_SUCCESS != rc)  {
      DEBUG ((DEBUG_ERROR, "DPTF:Failed to write ts_crit_enable.\r\n"));    	
	  rc = EFI_ABORTED;
	  goto _exit;
  }

_exit:
   return;
}

/**
  Entry Point for this driver.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
DptfDriverEntry(
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
)
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;


  {
    char* beg;
    char* end;
    unsigned int offset;

    end = (char*)(&(mGlobalNvsArea->Area->SdpProfile));

    beg =  (char*)(&(mGlobalNvsArea->Area->OperatingSystem));

    offset = (unsigned int)(end - beg);

    DEBUG ((EFI_D_INFO, "DptfDriverEntry beg=%x end=%x offset=%d\n",
          beg, end, offset));
  }


  Status = InitializeDriverData();
  ASSERT_EFI_ERROR(Status);

  // SOC thermal initialization
  SocThermInit();

  if (!EFI_ERROR(Status)) {

    // Register for OnReadyToBoot event
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               OnReadyToBoot,
               NULL, 
               &Event
               );
  }

  return EFI_SUCCESS;
}

VOID
SocThermInit ()
{
  UINT8 AUX3 = 0, TjMax = 90;
#define MSR_CPU_THERM_TEMPERATURE       0x1a2

  TjMax = RShiftU64(AsmReadMsr64(MSR_CPU_THERM_TEMPERATURE), 16) & 0xFF;
  if (TjMax == 127) TjMax = 90;

  AUX3 = (UINT8)(gDptfDrvData.CpuParticipantCriticalTemperature > TjMax ? TjMax : gDptfDrvData.CpuParticipantCriticalTemperature);
  AUX3 = TjMax - AUX3;

  MsgBus32Write(VLV_PUNIT, PUNIT_PTMC, 0x00030708);
  MsgBus32Write(VLV_PUNIT, PUNIT_GFXT, 0x0000C000);
  MsgBus32Write(VLV_PUNIT, PUNIT_VEDT, 0x00000004);
  MsgBus32Write(VLV_PUNIT, PUNIT_ISPT, 0x00000004);
  // Program PTPS according to the DTS critical temperature 
  //MsgBus32Write(VLV_PUNIT, PUNIT_PTPS, 0x00000000);
  MsgBus32Write(VLV_PUNIT, PUNIT_PTPS, AUX3 << 24);
  MsgBus32Write(VLV_PUNIT, PUNIT_TE_AUX3, 0x00061029);
  MsgBus32Write(VLV_PUNIT, PUNIT_TTE_VRIccMax, 0x00061029);
  MsgBus32Write(VLV_PUNIT, PUNIT_TTE_VRHot, 0x00061029);
  MsgBus32Write(VLV_PUNIT, PUNIT_TTE_XXPROCHOT, 0x00061029);
  MsgBus32Write(VLV_PUNIT, PUNIT_TTE_SLM0, 0x00001029);
  MsgBus32Write(VLV_PUNIT, PUNIT_TTE_SLM1, 0x00001029);

  // Set up registers for energy management and dynamic power limiting
  if (gDptfDrvData.SdpProfile == 4) {
    MsgBus32Write(VLV_PUNIT, PUNIT_SOC_POWER_BUDGET, 0x00000A00);  // CPU_POWER_BUDGET_CONTROL, ratio 10 = 1333mhz for 2.5w fanless.
  } else if (gDptfDrvData.SdpProfile == 1) {
    MsgBus32Write(VLV_PUNIT, PUNIT_SOC_POWER_BUDGET, 0x00001200);  // CPU_POWER_BUDGET_CONTROL, ratio 18 = 1500mhz for top bin pentium.
  } else if ((gDptfDrvData.SdpProfile == 2) || (gDptfDrvData.SdpProfile == 3)) {
    MsgBus32Write(VLV_PUNIT, PUNIT_SOC_POWER_BUDGET, 0x00000B00);  // CPU_POWER_BUDGET_CONTROL, ratio 11 = 1466mhz for mid and entry celeron.
  } else {
    MsgBus32Write(VLV_PUNIT, PUNIT_SOC_POWER_BUDGET, 0x00000000);  // CPU_POWER_BUDGET_CONTROL, cpu floor = hfm.
  }

  MsgBus32Write(VLV_PUNIT, PUNIT_SOC_ENERGY_CREDIT, 0x00000002);  // SOC_POWER_BUDGET_CONTROL, value provided by architecture.

}
