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

  DigitalThermalSensor.c

Abstract:

  Digital Thermal Sensor (DTS) driver.
  This SMM driver configures and supports the Digital Thermal Sensor features
  for the platform.

--*/

#include "DigitalThermalSensor.h"

#ifdef ECP_FLAG
#include <Protocol/SmmBase/SmmBase.h>
#include <Protocol/SmmIoTrapDispatch.h>
#else
#include <Protocol/SmmBase2.h>
#endif
#include <Protocol/GlobalNvsArea.h>
#ifndef ECP_FLAG
#include <Protocol/SmmIoTrapDispatch2.h>

#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#endif
#include <Library/DTSHookLib.h>
#ifndef ECP_FLAG
#include <Library/DevicePathLib.h>
#endif
#include <Library/CpuIA32.h>
#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#endif
#include <Library/AslUpdateLib.h>
#include <Library/IoLib.h>

#include <PchRegs.h>
#include <CpuPpmLib.h>
#include <CpuRegs.h>
#include <Setup.h>    // AMI_OVERRIDE



#ifdef ECP_FLAG
EFI_SMM_BASE_PROTOCOL     *mSmmBase;
EFI_SMM_SYSTEM_TABLE      *mSmst;
#endif


//
// System configuration (setup) information
//
SETUP_DATA      mSystemConfiguration;    // AMI_OVERRIDE

//
// Globals used by the reference code
//
EFI_GLOBAL_NVS_AREA       *mGlobalNvsAreaPtr;
extern EFI_GUID gEfiSetupVariableGuid;    // AMI_OVERRIDE

//
// Globals used by the Digital Thermal Sensor code
//
BOOLEAN                             mDtsEnabled;
UINT8                               mDtsTjMax;
UINT8                               mDiodeRelativeTemperature;
UINT16                              mAcpiBaseAddr;
BOOLEAN                             mReCalibrationError;
BOOLEAN                             mUpdateDtsInEverySmi;
DTS_VARIABLE                        mDtsVariable;
UINT32                              mFullFamilyModelStep;
UINT8                               mNoOfThresholdRanges;
UINT8                               (*mDtsThresholdTable)[3];


//
// The table below assumes a TjMax of 105 C.  The table is updated as needed for
// products with other TjMax settings.
//
UINT8 mDigitalThermalSensorThresholdTable[DTS_NUMBER_THRESHOLD_RANGES][3] = {
  ///
  /// TJ_MAX = 110                ///< Current Temp.  Low Temp. High Temp.
  ///
  {TJ_MAX-80,100,75},     ///<    <= 30            10       35
  {TJ_MAX-70,85,65},      ///< 30  ~ 39            25       45
  {TJ_MAX-60,75,55},      ///< 40  ~ 49            35       55
  {TJ_MAX-50,65,45},      ///< 50  ~ 59            45       65
  {TJ_MAX-40,55,35},      ///< 60  ~ 69            55       75
  {TJ_MAX-30,45,25},      ///< 70  ~ 79            65       85
  {TJ_MAX-20,35,15},      ///< 80  ~ 89            75       95
  {TJ_MAX-10,25,05},      ///< 90  ~ 99            85       105
  {TJ_MAX-00,15,00}       ///< 100 ~ 109           95       110
};

//
// Function implementations
//
#ifdef ECP_FLAG
EFI_STATUS
DtsSmiCallback(
  IN EFI_HANDLE             SmmImageHandle,
  IN OUT VOID               *CommunicationBuffer,
  IN OUT UINTN              *SourceSize
  )
#else
EFI_STATUS
EFIAPI
DtsSmiCallback(
  IN EFI_HANDLE             SmmImageHandle,
  IN     CONST VOID         *ContextData,        OPTIONAL
  IN OUT VOID               *CommunicationBuffer,    OPTIONAL
  IN OUT UINTN              *SourceSize    OPTIONAL
  )
#endif
/*++

Routine Description:

  SMI handler to handle Digital Thermal Sensor CPU Local APIC SMI
  for thermal threshold interrupt

Arguments:

  SmmImageHandle        Image handle returned by the SMM driver.
  CommunicationBuffer   Pointer to the buffer that contains the communication Message
  Source Size           Size of the memory image to be used for handler.

Returns:

  EFI_SUCCESS           Callback Function Executed

--*/
{
  UINTN                       Index;
  DTS_EVENT_TYPE              EventType;

  //
  // If not enabled; return.  (The DTS will be disabled upon S3 entry
  // and will remain disabled until after re-initialized upon wake.)
  //
  if(!mDtsEnabled) {
    return EFI_SUCCESS;
  }

  //
  // We enable the Thermal interrupt on the AP's prior to the event check
  // for the case where the AP has gone through the INIT-SIPI-SIPI sequence
  // and does not have the interrupt enabled.  (This allows the AP thermal
  // interrupt to be re-enabled due to chipset-based SMIs without waiting
  // to receive a DTS event on the BSP.)
  //
  for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
    gSmst->SmmStartupThisAp (DigitalThermalSensorEnableSmi, Index, NULL);
  }

  //
  // Check is this a DTS SMI event or the flag of update DTS temperature and threshold value in every SMI
  //
  if (DigitalThermalSensorEventCheck(&EventType) || mUpdateDtsInEverySmi) {
    //
    // Disable Local APIC SMI before programming the threshold
    //
    RunOnAllLogicalProcessors (DigitalThermalSensorDisableSmi, NULL);

    do {
      //
      // Handle BSP events
      //
      if ((EventType == DtsEventOutOfSpec) &&
          (mGlobalNvsAreaPtr->OperatingSystem == 0)) {
        //
        // Handle critical event by shutting down via EC if ACPI
        // is not enabled.
        //
        PlatformEventOutOfSpec();
      }

      //
      // Set the thermal trip toints as needed.
      // Note:  We only save the highest temperature of each die in
      // the NVS area when more than two logical processors are
      // present as only the highest DTS reading is actually used by
      // the current ASL solution.
      //
      mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature = 0;
      mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature = 0;

      //
      // Set the BSP thermal sensor thresholds
      //
      DigitalThermalSensorSetThreshold (&mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);

      //
      // Set the AP thermal sensor thresholds and update temperatures
      //
      for (Index = 1; Index < gSmst->NumberOfCpus / 2; Index++) {
        gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);
      }

      for (Index = gSmst->NumberOfCpus / 2; Index < gSmst->NumberOfCpus; Index++) {
        gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature);
      }

      //
      // Set SWGPE Status to generate an SCI if we had any events
      //
      if ((EventType != DtsEventNone) || mUpdateDtsInEverySmi) {
        DigitalThermalSensorSetSwGpeSts ();
      }

    } while (DigitalThermalSensorEventCheck(&EventType));
    //
    // Enable Local APIC SMI on all logical processors
    //
    RunOnAllLogicalProcessors(DigitalThermalSensorEnableSmi, NULL);
  }

  return EFI_SUCCESS;
}

#ifdef ECP_FLAG
VOID
DtsIoTrapCallback (
  IN  EFI_HANDLE                                  DispatchHandle,
  IN  EFI_SMM_IO_TRAP_DISPATCH_CALLBACK_CONTEXT   *CallbackContext
  )
#else
EFI_STATUS
EFIAPI
DtsIoTrapCallback (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
#endif
/*++

Routine Description:

  This catches IO trap SMI generated by the ASL code to enable the DTS AP function

Arguments:

  DispatchHandle      Not used
  DispatchContext     Not used

Returns:

  None

--*/
{
  UINTN                       Index;

  //
  // Determine the function desired, passed in the global NVS area
  //
  switch (mGlobalNvsAreaPtr->DigitalThermalSensorSmiFunction) {

      //
      // Enable AP Digital Thermal Sensor function after resume from S3
      //
    case INIT_DTS_FUNCTION_AFTER_S3:

      //
      // Enable the DTS on all logical processors.
      //
      RunOnAllLogicalProcessors (DigitalThermalSensorEnable, NULL);

      //
      // Set the thermal trip toints on all logical processors.
      // Note:  We only save the highest temperature of each die in the NVS area when
      // more than two logical processors are present as only the highest DTS reading
      // is actually used by the current ASL solution.
      //
      mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature = 0;
      mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature = 0;

      DigitalThermalSensorSetThreshold (&mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);

      for (Index = 1; Index < gSmst->NumberOfCpus / 2; Index++) {
        gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);
      }

      for (Index = gSmst->NumberOfCpus / 2; Index < gSmst->NumberOfCpus; Index++) {
        gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature);
      }

      //
      // Set SWGPE Status
      //
      DigitalThermalSensorSetSwGpeSts ();

      //
      // Re-enable the DTS.
      //
      mGlobalNvsAreaPtr->EnableDigitalThermalSensor = 1;
      if (mFullFamilyModelStep >= ( CPUID_FULL_FAMILY_MODEL_PENRYN )) {
        mGlobalNvsAreaPtr->EnableDigitalThermalSensor |= DTS_HYBRID_ENABLE_FOR_PENRYN;
      }

      mUpdateDtsInEverySmi = UPDATE_DTS_EVERY_SMI ;
      mDtsEnabled = TRUE;
      break;

      //
      // Disable update DTS temperature and threshold value in every SMI
      //
    case DISABLE_UPDATE_DTS_EVERY_SMI:
      mUpdateDtsInEverySmi = FALSE ;
      break;

    default:
      break;
  }

  //
  // Store return value
  //
  mGlobalNvsAreaPtr->DigitalThermalSensorSmiFunction = 0;
#ifndef ECP_FLAG
  return EFI_SUCCESS;
#endif
}

VOID
DtsS3EntryCallBack (
  IN  EFI_HANDLE                    Handle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT   *Context
  )
/*++

Routine Description:

Arguments:

  DispatchHandle    Handle of the callback

  DispatchContext   The dispatch context

Returns:

  EFI_SUCCESS        DTS disabled

--*/
{
  //
  // Clear the Digital Thermal Sensor flag in ACPI NVS.
  //
  mGlobalNvsAreaPtr->EnableDigitalThermalSensor = 0;
  //
  // Clear the enable flag.
  //
  mDtsEnabled = FALSE;

  return;
}

#if 0
static
UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:

  Determine the number of bytes making up a device path

Arguments:

  DevicePath  - pointer to the device path to process

Returns:

  The number of bytes in the device path

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Start;

  if (DevicePath == NULL) {
    return 0;
  }
  //
  // Search for the end of the device path structure
  //
  Start = DevicePath;
  while (!IsDevicePathEnd (DevicePath)) {
    DevicePath = NextDevicePathNode (DevicePath);
  }
  //
  // Compute the size and add back in the size of the end device path structure
  //
  return ((UINTN) DevicePath - (UINTN) Start) + sizeof (EFI_DEVICE_PATH_PROTOCOL);
}

static
EFI_DEVICE_PATH_PROTOCOL *
AppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  )
/*++

Routine Description:

  Append device path Src2 to the end of device path Src1.

Arguments:

  Src1  - A pointer to a device path data structure to append to
  Src2  - A pointer to a device path data structure to append after Src1

Returns:

  A pointer to the new device path is returned.
  NULL is returned if space for the new device path could not be allocated from pool.

Notes:

  It is up to the caller to free the memory used by Src1 and Src2 if they are no longer needed.
  This routine puts the result into boot services memory.

--*/
{
  EFI_STATUS                Status;
  UINTN                     Size;
  UINTN                     Size1;
  UINTN                     Size2;
  EFI_DEVICE_PATH_PROTOCOL  *NewDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *SecondDevicePath;
  //
  // Allocate space for the combined device path. It only has one end node of
  // length EFI_DEVICE_PATH_PROTOCOL
  //
  Size1 = DevicePathSize (Src1);
  Size2 = DevicePathSize (Src2);
  Size  = Size1 + Size2;

  if (Size1 != 0 && Size2 != 0) {
    Size -= sizeof (EFI_DEVICE_PATH_PROTOCOL);
  }
  Status = gBS->AllocatePool (EfiBootServicesData, Size, (VOID **) &NewDevicePath);
  if (EFI_ERROR (Status)) {
    return NULL;
  }
  gBS->CopyMem (NewDevicePath, Src1, Size1);
  //
  // Overwrite Src1 EndNode and do the copy
  //
  if (Size1 != 0) {
    SecondDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ((CHAR8 *) NewDevicePath + (Size1 - sizeof (EFI_DEVICE_PATH_PROTOCOL)));
  } else {
    SecondDevicePath = NewDevicePath;
  }
  gBS->CopyMem (SecondDevicePath, Src2, Size2);
  return NewDevicePath;
}
#endif

EFI_STATUS
ThresholdTableInit (
  VOID
  )
/*++

Routine Description:

  Performs initialization of the threshold table.

  <TODO> Update this function as necessary for the tables used by the implementation.

Arguments:

  None

Returns:

  EFI_SUCCESS  Threshold tables initialized successfully.

--*/
{
  UINT8   Delta;
  UINTN   i;

  //
  // If the table must be updated, shift the thresholds by the difference between
  // TJ_MAX_100 and DtsTjMax.
  //
  if (mDtsTjMax != TJ_MAX) {
    Delta = TJ_MAX - mDtsTjMax;

    for (i = 0; i < mNoOfThresholdRanges; i++) {
      if (mDtsThresholdTable[i][1] <= mDtsTjMax) {
        mDtsThresholdTable[i][0] = mDtsThresholdTable[i][0] - Delta;
      } else {
        mDtsThresholdTable[i][0] = 0;
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DigitalThermalSensorInit (
  VOID
  )
/*++

Routine Description:

  Perform first time initialization of the Digital Thermal Sensor

Arguments:

  None

Returns:

  EFI_SUCCESS  Init Digital Thermal Sensor successfully

--*/
{
  UINTN               Index;

  //
  // Initialize the DTS threshold table.
  //
  ThresholdTableInit();

  //
  // Set the thermal trip toints on all logical processors.
  // Note:  We only save the highest temperature of each die in the NVS area when
  // more than two logical processors are present as only the highest DTS reading
  // is actually used by the current ASL solution.
  //
  mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature = 0;
  mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature = 0;

  DigitalThermalSensorSetThreshold (&mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);

  for (Index = 1; Index < gSmst->NumberOfCpus / 2; Index++) {
    gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);
  }

  for (Index = gSmst->NumberOfCpus / 2; Index < gSmst->NumberOfCpus; Index++) {
    gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature);
  }

  //
  // Enable the Local APIC SMI on all logical processors
  //
  for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
    gSmst->SmmStartupThisAp (DigitalThermalSensorEnableSmi, Index, NULL);
  }
  DigitalThermalSensorEnableSmi (NULL);

  //
  // Set Digital Thermal Sensor flag in ACPI NVS
  //
  mGlobalNvsAreaPtr->EnableDigitalThermalSensor = 1;
  if (mFullFamilyModelStep >= ( CPUID_FULL_FAMILY_MODEL_PENRYN )) {
    mGlobalNvsAreaPtr->EnableDigitalThermalSensor |= DTS_HYBRID_ENABLE_FOR_PENRYN;
  }
  mUpdateDtsInEverySmi = UPDATE_DTS_EVERY_SMI;
  mDtsEnabled = TRUE;

  return EFI_SUCCESS;
}

VOID
DigitalThermalSensorEnable (
  VOID      *Buffer
  )
/*++

Routine Description:

  Initializes the Thermal Sensor Control MSR

  This function must be AP safe.

Arguments:

  Buffer        Unused.

Returns:

  EFI_SUCCESS   The function completed successfully.

--*/
{
  MSR_REGISTER                MsrData;

  //
  // First, clear our log bits
  //
  MsrData.Qword = AsmReadMsr64 (EFI_MSR_IA32_THERM_STATUS);    // AMI_OVERRIDE
  MsrData.Qword &= ~THERM_STATUS_LOG_MASK;    
  AsmWriteMsr64 (EFI_MSR_IA32_THERM_STATUS, MsrData.Qword);    // AMI_OVERRIDE
#if 0 // EFI_MSR_PIC_SENS_CFG (0x1AA) is not available in Bay-Trail
  //
  // Second, configure the thermal sensor control
  //
  MsrData.Qword = AsmReadMsr64 (EFI_MSR_PIC_SENS_CFG);    // AMI_OVERRIDE
  //
  // Disable the digital LP filter
  //
  MsrData.Qword |= B_BYPASS_FILTER;
  //
  // Only lock interrupts if in CMP mode
  //
  if (gSmst->NumberOfCpus > 1) {
    MsrData.Qword |= B_LOCK_THERMAL_INT;
  }
  //
  // Set sample rate.
  //
  MsrData.Bytes.SecondByte = DTS_SAMPLE_RATE;
  AsmWriteMsr64 (EFI_MSR_PIC_SENS_CFG, MsrData.Qword);    // AMI_OVERRIDE
#endif
  return;
}

VOID
DigitalThermalSensorSetSwGpeSts (
  VOID
  )
/*++

Routine Description:

  Generates a _GPE._L02 SCI to an ACPI OS.

Arguments:

  None

Returns:

  None

--*/
{
  UINT8    GpeCntl;

  //
  // Check SCI enable
  //
  if (((SmmIoRead8 (mAcpiBaseAddr + R_PCH_ACPI_PM1_CNT)) & B_PCH_ACPI_PM1_CNT_SCI_EN) != 0) {
    //
    // Do platform specific things before generate SCI
    //
    PlatformHookBeforeGenerateSCI();

    //
    // Set SWGPE Status
    //
    GpeCntl = SmmIoRead8 (mAcpiBaseAddr + R_ACPI_GPE_CNTL);
    GpeCntl |= B_SWGPE_CTRL;
    SmmIoWrite8 (mAcpiBaseAddr + R_ACPI_GPE_CNTL, GpeCntl);
  }
}


BOOLEAN
DigitalThermalSensorEventCheck (
  DTS_EVENT_TYPE            *EventType
  )
/*++

Routine Description:

  Checks for a Core Thermal Event on any processor

Arguments:

  None

Returns:

  TRUE means this is a DTS Thermal event
  FALSE means this is not a DTS Thermal event.

--*/
{
  UINTN   Index;

  //
  // Clear event status
  //
  *EventType = DtsEventNone;

  //
  // Check BSP thermal event
  //
  DigitalThermalSensorEventCheckMsr (EventType);

  //
  // Check AP thermal event
  //
  for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
    gSmst->SmmStartupThisAp (DigitalThermalSensorEventCheckMsr, Index, EventType);
  }

  //
  // Return TRUE if any logical processor reported an event.
  //
  if (*EventType != DtsEventNone) {
    return TRUE;
  }
  return FALSE;
}

VOID
DigitalThermalSensorEventCheckMsr (
  IN  VOID        *Buffer
  )
/*++

Routine Description:

  Checks for a Core Thermal Event by reading MSR.

  This function must be MP safe.

Arguments:

  Buffer    Pointer to DTS_EVENT_TYPE


Returns:

  None

--*/
{
  MSR_REGISTER                MsrData;
  DTS_EVENT_TYPE              *EventType;

  //
  // Cast to enhance readability.
  //
  EventType = (DTS_EVENT_TYPE*)Buffer;

  //
  // If any processor has already been flagged as Out-Of-Spec,
  // just return.
  //
  if(*EventType != DtsEventOutOfSpec) {
    //
    // Read thermal status
    //
    MsrData.Qword = AsmReadMsr64 (EFI_MSR_IA32_THERM_STATUS);    // AMI_OVERRIDE

    //
    // Check for Out-Of-Spec status.
    //
    if (MsrData.Qword & B_OUT_OF_SPEC_STATUS_LOG) {
      *EventType = DtsEventOutOfSpec;

    //
    // Check thresholds.
    //
    } else if (MsrData.Qword & (B_THERMAL_THRESHOLD_1_STATUS_LOG | B_THERMAL_THRESHOLD_2_STATUS_LOG)) {
      *EventType = DtsEventThreshold;
    }
  }
}

VOID
DigitalThermalSensorSetThreshold (
  VOID          *Buffer
  )
/*++

Routine Description:

  Read the temperature and reconfigure the thresholds.
  This function must be AP safe.

Arguments:

  Buffer        Pointer to UINT8 to update with the current temperature

Returns:

  EFI_SUCCESS   Digital Thermal Sensor threshold programmed successfully

--*/
{
  INT8                  ThresholdEntry;
  MSR_REGISTER          MsrData;
  UINT8                 Temperature;

  //
  // Read the temperature
  //
  MsrData.Qword = AsmReadMsr64 (EFI_MSR_IA32_THERM_STATUS);    // AMI_OVERRIDE

  //
  // If Out-Of-Spec, return the critical shutdown temperature.
  //
  if (MsrData.Qword & B_OUT_OF_SPEC_STATUS) {
    *((UINT8*) Buffer) = DTS_CRITICAL_TEMPERATURE;
    return;
  } else if (MsrData.Qword & B_READING_VALID) {
    //
    // Find the DTS temperature.
    //
    Temperature = mDtsTjMax - (MsrData.Bytes.ThirdByte & OFFSET_MASK);
    //
    // We only update the temperature if it is above the current temperature.
    //
    if (Temperature > *((UINT8*) Buffer)) {
      *((UINT8*) Buffer) = Temperature;
    }

    //
    // Compare the current temperature to the Digital Thermal Sensor Threshold Table until
    // a matching Value is found.
    //
    ThresholdEntry = 0;
    while((Temperature > mDtsThresholdTable[ThresholdEntry][0]) &&
          (ThresholdEntry < (mNoOfThresholdRanges - 1))) {
      ThresholdEntry++;
    }

    //
    // Update the threshold values
    //
    MsrData.Qword = AsmReadMsr64 (EFI_MSR_IA32_THERM_INTERRUPT);      // AMI_OVERRIDE
    //
    // Low temp is threshold #2
    //
    MsrData.Bytes.ThirdByte  = mDtsThresholdTable[ThresholdEntry][1];
    //
    // High temp is threshold #1
    //
    MsrData.Bytes.SecondByte = mDtsThresholdTable[ThresholdEntry][2];

    //
    // Enable interrupts
    //
    MsrData.Qword |= TH1_ENABLE;
    MsrData.Qword |= TH2_ENABLE;

    //
    // If the high temp is at TJ_MAX (offset == 0)
    // We disable the int to avoid generating a large number of SMI because of TM1/TM2
    // causing many threshold crossings
    //
    if (MsrData.Bytes.SecondByte == 0x80) {
      MsrData.Qword &= ~TH1_ENABLE;
    }
    AsmWriteMsr64 (EFI_MSR_IA32_THERM_INTERRUPT, MsrData.Qword);    // AMI_OVERRIDE
  }

  //
  //  Clear the threshold log bits
  //
  MsrData.Qword = AsmReadMsr64 (EFI_MSR_IA32_THERM_STATUS);    // AMI_OVERRIDE
  MsrData.Qword &= ~THERM_STATUS_THRESHOLD_LOG_MASK;
  AsmWriteMsr64 (EFI_MSR_IA32_THERM_STATUS, MsrData.Qword);    // AMI_OVERRIDE

  return;
}

VOID
DigitalThermalSensorEnableSmi (
  VOID          *Buffer
  )
/*++

Routine Description:

  Enables the Thermal Interrupt in the core Local APIC.

Arguments:

  None

Returns:

  EFI_SUCCESS    Enable Local APIC to generate a SMI successfully

--*/
{
  UINT32                ApicThermalValue;

  //
  // Configure the Local APIC to generate an SMI on Thermal events.  First,
  // Clear BIT16, BIT10-BIT8, BIT7-BIT0.  Then, set BIT9 (delivery mode).
  // Don't enable the interrupt if it's already enabled
  //

  ApicThermalValue = *(UINT32*) (UINTN) LOCAL_APIC_THERMAL_DEF;

  if ((ApicThermalValue & (B_INTERRUPT_MASK | B_DELIVERY_MODE | B_VECTOR)) != V_MODE_SMI) {
    ApicThermalValue = (ApicThermalValue & !(B_INTERRUPT_MASK | B_DELIVERY_MODE | B_VECTOR)) | V_MODE_SMI;
    *(UINT32*) (UINTN) LOCAL_APIC_THERMAL_DEF = ApicThermalValue;
  }

  return;
}

VOID
DigitalThermalSensorDisableSmi (
  VOID          *Buffer
  )
/*++

Routine Description:

  Disables the Thermal Interrupt in the core Local APIC.

Arguments:

  None

Returns:

  EFI_SUCCESS    Disable Local APIC to generate a SMI successfully

--*/
{
  UINT32                ApicThermalValue;

  //
  // Disable Local APIC thermal entry
  //
  ApicThermalValue = *(UINT32*) (UINTN) LOCAL_APIC_THERMAL_DEF;
  // Following descriptions were from SSE BIOS
  // We set the interrupt mode at the same time as the interrupt is disabled to
  // avoid the "Received Illegal Vector" being set in the Error Status Register.
  //  and eax, 0FFFEF800h
  //  or  eax, 000010200h     ; Clear Mask, Set Delivery
  ApicThermalValue = (ApicThermalValue & !(B_INTERRUPT_MASK | B_DELIVERY_MODE | B_VECTOR)) | (B_INTERRUPT_MASK | V_MODE_SMI);
  *(UINT32*) (UINTN) LOCAL_APIC_THERMAL_DEF = ApicThermalValue;

  return;
}

STATIC
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE   Procedure,
  IN OUT VOID               *Buffer
  )
/*++

Routine Description:

  Runs the specified procedure on all logical processors, passing in the
  parameter buffer to the procedure.

Arguments:

  Procedure     The function to be run.
  Buffer        Pointer to a parameter buffer.

Returns:

  None

--*/
{
  UINTN                       Index;

  //
  // Run the procedure on all logical processors.
  //
  (*Procedure) (Buffer);
  for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
    gSmst->SmmStartupThisAp (Procedure, Index, Buffer);
  }

  return EFI_SUCCESS;
}

VOID
DtsSwSmiCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:
  SMI handler to update DTS temperatures

  Dispatched on reads from APM port with value 0xD0

Returns:
  Nothing

--*/
{
  UINTN                       Index;

  //
  // Determine the function desired, passed in the global NVS area
  //
  switch (mGlobalNvsAreaPtr->DigitalThermalSensorSmiFunction) {

  //
  // Enable AP Digital Thermal Sensor function after resume from S3
  //
    case INIT_DTS_FUNCTION_AFTER_S3:

    //
    // Enable the DTS on all logical processors.
    //
      RunOnAllLogicalProcessors (DigitalThermalSensorEnable, NULL);


      //
      // Set the thermal trip toints on all logical processors.
      // Note:  We only save the highest temperature of each die in the NVS area when
      // more than two logical processors are present as only the highest DTS reading
      // is actually used by the current ASL solution.
      //
      mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature = 0;
      mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature = 0;

      DigitalThermalSensorSetThreshold (&mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);

      for (Index = 1; Index < gSmst->NumberOfCpus / 2; Index++) {
        gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->BspDigitalThermalSensorTemperature);
      }

      for (Index = gSmst->NumberOfCpus / 2; Index < gSmst->NumberOfCpus; Index++) {
        gSmst->SmmStartupThisAp (DigitalThermalSensorSetThreshold, Index, &mGlobalNvsAreaPtr->ApDigitalThermalSensorTemperature);
      }

      //
      // Set SWGPE Status
      //
      DigitalThermalSensorSetSwGpeSts ();

      //
      // Re-enable the DTS.
      //
      mGlobalNvsAreaPtr->EnableDigitalThermalSensor = 1;
      if (mFullFamilyModelStep >= ( CPUID_FULL_FAMILY_MODEL_PENRYN )) {
        mGlobalNvsAreaPtr->EnableDigitalThermalSensor |= DTS_HYBRID_ENABLE_FOR_PENRYN;
      }

      mUpdateDtsInEverySmi = UPDATE_DTS_EVERY_SMI ;
      mDtsEnabled = TRUE;
      break;

      //
      // Disable update DTS temperature and threshold value in every SMI
      //
    case DISABLE_UPDATE_DTS_EVERY_SMI:
      mUpdateDtsInEverySmi = FALSE ;
      break;

    default:
      break;
  }

  //
  // Store return value
  //
  mGlobalNvsAreaPtr->DigitalThermalSensorSmiFunction = 0;
}

EFI_STATUS
InstallDigitalThermalSensor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Digital Thermal Sensor (DTS) SMM driver entry point function.

Arguments:

  ImageHandle   Image handle for this driver image
  SystemTable   Pointer to the EFI System Table

Returns:

  EFI_SUCCESS   Driver initialization completed successfully

--*/
{
  EFI_STATUS                              Status;
  UINT32                                  Attributes;
  UINTN                                   DataSize;

  MSR_REGISTER                            MsrData;
  EFI_CPUID_REGISTER                      CpuidRegisters;
  EFI_GLOBAL_NVS_AREA_PROTOCOL            *GlobalNvsAreaProtocol;

#if 0 // Bay-Trail has no IO Trap Available, use SW SMI 0xD0 instead
  EFI_SMM_IO_TRAP_REGISTER_CONTEXT        IchIoTrapContext;
  EFI_SMM_IO_TRAP_DISPATCH2_PROTOCOL      *IchIoTrap;
  EFI_HANDLE                              IchIoTrapHandle;
#endif

  EFI_SMM_SX_DISPATCH_CONTEXT             SxDispatchContext;
  EFI_SMM_SX_DISPATCH_PROTOCOL            *SxDispatchProtocol;
  EFI_HANDLE                              SxDispatchHandle;
  EFI_HANDLE                              Handle;

  EFI_SMM_SW_DISPATCH_PROTOCOL            *SwDispatch;
  EFI_SMM_SW_DISPATCH_CONTEXT             SwContext;

  //
  // Locate setup variable
  //
  Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  DataSize = sizeof (SETUP_DATA);    // AMI_OVERRIDE
  Status = gST->RuntimeServices->GetVariable (
                                   PLATFORM_SETUP_VARIABLE_NAME,
                                   &gEfiSetupVariableGuid,
                                   &Attributes,
                                   &DataSize,
                                   &mSystemConfiguration
                                   );
  ASSERT_EFI_ERROR (Status);

  //
  // Check if DTS enabled in setup and supported by the CPU:
  // Yonah A1 or later, Merom or Millville.
  //
//  EfiCpuid (EFI_CPUID_VERSION_INFO, &CpuidRegisters);
  AsmCpuid (EFI_CPUID_VERSION_INFO, &CpuidRegisters.RegEax, &CpuidRegisters.RegEbx, &CpuidRegisters.RegEcx, &CpuidRegisters.RegEdx);      // AMI_OVERRIDE
  mFullFamilyModelStep = CpuidRegisters.RegEax & CPUID_FULL_FAMILY_MODEL_STEPPING;

  if (!mSystemConfiguration.EnableDigitalThermalSensor) {
    DEBUG ((EFI_D_ERROR, "DTS not enabled/supported, so driver not loaded into SMM\n"));
    return EFI_SUCCESS;
  }

#ifdef ECP_FLAG
  //
  // Find the SMM base protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmmBaseProtocolGuid, NULL, (VOID **) &mSmmBase);
  ASSERT_EFI_ERROR (Status);
#endif

  //
  // Verify the code supports the number of processors present.
  //
  ASSERT (gSmst->NumberOfCpus <= LOGICAL_PROCESSORS);

  //
  // Get the ACPI Base Address
  //
  mAcpiBaseAddr = PchLpcPciCfg16 (R_PCH_LPC_ACPI_BASE) & B_PCH_LPC_PMC_BASE_BAR;

  //
  // Locate our shared data area
  //
  Status = gBS->LocateProtocol (&gEfiGlobalNvsAreaProtocolGuid, NULL, (VOID **) &GlobalNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);
  mGlobalNvsAreaPtr = GlobalNvsAreaProtocol->Area;

  //
  // Locate Platform specific data area, or prepare platform services
  //
  InitializeDtsHookLib();

#if 0 // Bay-Trail has no IO Trap Available, use SW SMI 0xD0 instead
  //
  // Initialize ASL manipulation library
  //
  InitializeAslUpdateLib ();

  //
  // Locate the ICH Trap dispatch protocol
  //
#ifdef ECP_FLAG
  Status = gBS->LocateProtocol (&gEfiSmmIoTrapDispatchProtocolGuid, NULL, (VOID **) &IchIoTrap);
#else
  Status = gBS->LocateProtocol (&gEfiSmmIoTrapDispatch2ProtocolGuid, NULL, (VOID **) &IchIoTrap);
#endif
  ASSERT_EFI_ERROR (Status);

  IchIoTrapContext.Type     = ReadWriteTrap;
  IchIoTrapContext.Length   = 4;
  IchIoTrapContext.Address  = 0;
#ifdef ECP_FLAG
  IchIoTrapContext.Context  = NULL;
#endif

  Status = IchIoTrap->Register (
                        IchIoTrap,
                        DtsIoTrapCallback,
                        &IchIoTrapContext,
                        &IchIoTrapHandle
                        );
  ASSERT_EFI_ERROR (Status);

  //
  // Update two ASL items.
  // 1: Operating Region for DTS IO Trap.
  // 2: Resource Consumption in LPC Device.
  //
  ASSERT (IchIoTrapContext.Length <= (UINT8) (-1));
  Status = UpdateAslCode ((SIGNATURE_32 ('I', 'O', '_', 'D')), IchIoTrapContext.Address, (UINT8) IchIoTrapContext.Length);
  ASSERT_EFI_ERROR (Status);
#endif

  //
  // Register a callback function to handle Digital Thermal Sensor SMIs.
  //
#ifdef ECP_FLAG
  Status = mSmmBase->RegisterCallback (mSmmBase, ImageHandle, DtsSmiCallback, FALSE, FALSE);
#else
  Status = gSmst->SmiHandlerRegister (DtsSmiCallback, NULL,  &Handle);
#endif
  ASSERT_EFI_ERROR (Status);

  //
  //  Get the Sw dispatch protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmmSwDispatchProtocolGuid,
                  NULL,
                  (VOID **) &SwDispatch
                  );
  ASSERT_EFI_ERROR(Status);

  //
  // Register DTS SW SMI 0xD0 handler
  //
  SwContext.SwSmiInputValue = 0xD0;
  Status = SwDispatch->Register (
                         SwDispatch,
                         DtsSwSmiCallback,
                         &SwContext,
                         &Handle
                         );
  ASSERT_EFI_ERROR(Status);


  //
  // Locate the Sx Dispatch Protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmmSxDispatchProtocolGuid,
                  NULL,
                  (VOID **) &SxDispatchProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register the callback for S3 entry
  //
  SxDispatchContext.Type  = SxS3;
  SxDispatchContext.Phase = SxEntry;
  Status = SxDispatchProtocol->Register (
                                 SxDispatchProtocol,
                                 DtsS3EntryCallBack,
                                 &SxDispatchContext,
                                 &SxDispatchHandle
                                 );
  ASSERT_EFI_ERROR (Status);

  //
  // Set the TjMax value used by the processor.  (This value is used for absolute
  // value temperature calculations.)
  //
  ///
  /// Get the TCC Activation Temperature and use it for TjMax.
  ///
    MsrData.Qword         = AsmReadMsr64 (EFI_MSR_CPU_THERM_TEMPERATURE);    // AMI_OVERRIDE

  mDtsTjMax             = (MsrData.Bytes.ThirdByte);
  mDtsThresholdTable    = mDigitalThermalSensorThresholdTable;
  mNoOfThresholdRanges  = DTS_NUMBER_THRESHOLD_RANGES;

  //
  // Enable the DTS on all logical processors.
  //
  RunOnAllLogicalProcessors (DigitalThermalSensorEnable, NULL);

  //
  // Locate DTS variables
  //
  Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  DataSize = sizeof (DTS_VARIABLE);
  Status = gST->RuntimeServices->GetVariable (
                                   DTS_GLOBAL_VARIABLE_NAME,
                                   &gEfiDtsVariableGuid,
                                   NULL,
                                   &DataSize,
                                   &mDtsVariable
                                   );

  //
  // If unable to locate DTS variables, flag the variables as invalid.
  //
  if (EFI_ERROR (Status)) {
    mDtsVariable.NumberOfProcessors         = 0;
  }

  //
  // Initialize Digital Thermal Sensor Function in POST
  //
  DigitalThermalSensorInit ();

  return EFI_SUCCESS;
}
