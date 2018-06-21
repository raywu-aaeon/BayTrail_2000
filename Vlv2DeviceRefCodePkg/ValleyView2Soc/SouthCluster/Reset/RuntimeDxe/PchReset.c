/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PchReset.c

Abstract:

  Reset Architectural Protocol implementation

--*/
#include "PchReset.h"
#ifdef ECP_FLAG
EFI_GUID gEfiPchExtendedResetProtocolGuid = EFI_PCH_EXTENDED_RESET_PROTOCOL_GUID;
#else
#include <Library/UefiRuntimeServicesTableLib.h>
#include <TianoApi.h>
#endif
//EIP188072 >>
//AMI_OVERRIDE_START >>
#include <SbElinks.h>

// Function Prototypes
typedef VOID (SB_RESET_CALLBACK) (
    IN EFI_RESET_TYPE           ResetType,
    IN EFI_STATUS               ResetStatus,
    IN UINTN                    DataSize,
    IN VOID                     *ResetData OPTIONAL
);

// Function Definitions
extern SB_RESET_CALLBACK SB_RESET_CALLBACK_LIST EndOfList;
SB_RESET_CALLBACK* SbResetCallbackList[] = { SB_RESET_CALLBACK_LIST NULL };
//AMI_OVERRIDE_END >>
//EIP188072 <<

PCH_RESET_INSTANCE  *mPchResetInstance;


STATIC UINT8        mDaysOfMonthInfo[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/**
  Check if it is leap year

  @param[in] Year            year to be check

  @retval True               year is leap year
  @retval FALSE              year is not a leap year
**/
BOOLEAN
IsLeapYear (
  IN UINT16 Year
  )
{
  return (Year % 4 == 0) && ((Year % 100 != 0) || (Year % 400 == 0));
}

/**
  Set System Wakeup Alarm.

  @param[in] WakeAfter       Time offset in seconds to wake from S3

  @retval EFI_SUCCESS        Timer started successfully
**/

STATIC
EFI_STATUS
SetSystemWakeupAlarm (
  IN       UINT32          WakeAfter
  )
{
  EFI_STATUS              Status;
  EFI_TIME                Time;
  EFI_TIME_CAPABILITIES   Capabilities;
  UINT32                  Reminder;
  UINT16                  PmBase;
  UINT8                   DayOfMonth;
  ///
  /// For an instant wake 2 seconds is a safe value
  ///
  if (WakeAfter < 2) {
    WakeAfter = 2;
  }

  Status = EfiGetTime (&Time, &Capabilities);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  Reminder = WakeAfter + (UINT32) Time.Second;
  Time.Second = Reminder % 60;
  Reminder = Reminder / 60;
  Reminder = Reminder + (UINT32) Time.Minute;
  Time.Minute = Reminder % 60;
  Reminder = Reminder / 60;
  Reminder = Reminder + (UINT32) Time.Hour;
  Time.Hour = Reminder % 24;
  Reminder = Reminder / 24;

  if (Reminder > 0) {
    Reminder = Reminder + (UINT32) Time.Day;
    if ((Time.Month == 2) && IsLeapYear (Time.Year)) {
      DayOfMonth = 29;
    } else {
      DayOfMonth = mDaysOfMonthInfo[Time.Month - 1];
    }
    if (Reminder > DayOfMonth) {
      Time.Day = (UINT8)Reminder - DayOfMonth;
      Reminder = 1;
    } else {
      Time.Day = (UINT8)Reminder;
      Reminder = 0;
    }
  }

  if (Reminder > 0) {
    if (Time.Month == 12) {
      Time.Month = 1;
      Time.Year = Time.Year + 1;
    } else {
      Time.Month = Time.Month + 1;
    }
  }

  Status = EfiSetWakeupTime (TRUE, &Time);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PmBase = MmioRead16 (
             MmPciAddress (0,
               DEFAULT_PCI_BUS_NUMBER_PCH,
               PCI_DEVICE_NUMBER_PCH_LPC,
               PCI_FUNCTION_NUMBER_PCH_LPC,
               R_PCH_LPC_ACPI_BASE
             )
           ) & B_PCH_LPC_ACPI_BASE_BAR;

  ///
  /// Clear RTC PM1 status
  ///
  IoWrite16 (PmBase + R_PCH_ACPI_PM1_STS, B_PCH_ACPI_PM1_STS_RTC);

  ///
  /// set RTC_EN bit in PM1_EN to wake up from the alarm
  ///
  IoWrite16 (
    PmBase + R_PCH_ACPI_PM1_EN,
    (IoRead16 (PmBase + R_PCH_ACPI_PM1_EN) | B_PCH_ACPI_PM1_EN_RTC)
    );
  return Status;
}

EFI_STATUS
EFIAPI
InitializePchReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initialize the state information for the Timer Architectural Protocol

Arguments:

  ImageHandle             Image handle of the loaded driver
  SystemTable             Pointer to the System Table

Returns:

  EFI_SUCCESS             Thread can be successfully created
  EFI_OUT_OF_RESOURCES    Cannot allocate protocol data structure
  EFI_DEVICE_ERROR        Cannot create the timer service

--*/
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        ResetHandle;
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
#endif
  EFI_EVENT                         Event;

  DEBUG ((EFI_D_INFO, "InitializePchReset() Start\n"));
  ResetHandle = NULL;

  //
  // Allocate Runtime memory for the PchExtendedReset protocol instance.
  //
  mPchResetInstance = AllocateRuntimePool (sizeof (PCH_RESET_INSTANCE));
  if (mPchResetInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  //
  // Get the desired platform setup policy.
  //
  Status = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **) &PchPlatformPolicy);
  ASSERT_EFI_ERROR (Status);

  //
  // Check whether the CapsuleVariableName is filled.
  //
  if (PchPlatformPolicy->CapsuleVariableName != NULL) {
    //
    // Allocate a runtime space and copy string from CapsuleVariableName
    //
    mPchResetInstance->CapsuleVariableName = AllocateRuntimePool (StrSize (PchPlatformPolicy->CapsuleVariableName));
    CopyMem (
      mPchResetInstance->CapsuleVariableName,
      PchPlatformPolicy->CapsuleVariableName,
      StrSize (PchPlatformPolicy->CapsuleVariableName)
      );
  }
#endif
  //
  // Install protocol interface
  //
  mPchResetInstance->Signature                      = PCH_RESET_SIGNATURE;
  mPchResetInstance->Handle                         = NULL;
  mPchResetInstance->PchExtendedResetProtocol.Reset = PchExtendedReset;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPchResetInstance->Handle,
                  &gEfiPchExtendedResetProtocolGuid,
                  &mPchResetInstance->PchExtendedResetProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  mPchResetInstance->PmcBase = MmioRead32 (
                                 MmPciAddress (0,
                                   DEFAULT_PCI_BUS_NUMBER_PCH,
                                   PCI_DEVICE_NUMBER_PCH_LPC,
                                   PCI_FUNCTION_NUMBER_PCH_LPC,
                                   R_PCH_LPC_PMC_BASE
                                 )
                               ) & B_PCH_LPC_PMC_BASE_BAR;
  ASSERT (mPchResetInstance->PmcBase != 0);

  mPchResetInstance->AcpiBar = MmioRead32 (
                                 MmPciAddress (0,
                                   DEFAULT_PCI_BUS_NUMBER_PCH,
                                   PCI_DEVICE_NUMBER_PCH_LPC,
                                   PCI_FUNCTION_NUMBER_PCH_LPC,
                                   R_PCH_LPC_ACPI_BASE
                                 )
                               ) & B_PCH_LPC_ACPI_BASE_BAR;
  ASSERT (mPchResetInstance->AcpiBar != 0);

  //
  // Make sure the Reset Architectural Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiResetArchProtocolGuid);

  //
  // Hook the runtime service table
  //
  SystemTable->RuntimeServices->ResetSystem = IntelPchResetSystem;

  //
  // Now install the Reset RT AP on a new handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ResetHandle,
                  &gEfiResetArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

//Install a notification to convert the PchReset driver to Virtual Addr
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  PchResetVirtualddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // The Lib Deconstruct will automatically be called when entrypoint return error.
  //
  DEBUG ((EFI_D_INFO, "InitializePchReset() End\n"));

  return Status;
}

#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
VOID
CapsuleReset (
  IN UINTN   CapsuleDataPtr
  )
/*++

Routine Description:

  If need be, do any special reset required for capsules. For this
  implementation where we're called from the ResetSystem() api,
  just set our capsule variable and return to let the caller
  do a soft reset.

Arguments:

  CapsuleDataPtr          pointer to the capsule block descriptors

Returns:

  None

--*/
{
  UINT32  Data32;
  UINT32  Eflags;
  UINT16  AcpiBase;

  //
  // Check the CapsuleVariableName, asset if it is not filled.
  //
  ASSERT (mPchResetInstance->CapsuleVariableName != NULL);

  //
  // This implementation assumes we're using a variable for the capsule
  // data pointer.
  //
  EfiSetVariable (
    mPchResetInstance->CapsuleVariableName,
    &gEfiCapsuleVendorGuid, // capsule variable guid
    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,  // attributes
    sizeof (UINTN),           // data size
    (VOID *) &CapsuleDataPtr  // data
    );
  //
  // Read the PM1_CNT register
  //
  AcpiBase = MmioRead16 (
               MmPciAddress (0,
                 DEFAULT_PCI_BUS_NUMBER_PCH,
                 PCI_DEVICE_NUMBER_PCH_LPC,
                 PCI_FUNCTION_NUMBER_PCH_LPC,
                 R_PCH_LPC_ACPI_BASE
               )
             ) & B_PCH_LPC_ACPI_BASE_BAR;
  ASSERT (AcpiBase != 0);

  Data32  = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT));

  Data32  = (UINT32) ((Data32 &~(B_PCH_ACPI_PM1_CNT_SLP_TYP + B_PCH_ACPI_PM1_CNT_SLP_EN)) | V_PCH_ACPI_PM1_CNT_S3);

  Eflags  = (UINT32) AsmReadEflags ();

  if ((Eflags & 0x200)) {
    DisableInterrupts ();
  }

  AsmWbinvd ();
  AsmWriteCr0 (AsmReadCr0 () | 0x060000000);

  IoWrite32 (
    (UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT),
    (UINT32) Data32
    );

  Data32 = Data32 | B_PCH_ACPI_PM1_CNT_SLP_EN;

  IoWrite32 (
    (UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT),
    (UINT32) Data32
    );

  if ((Eflags & 0x200)) {
    EnableInterrupts ();
  }
  //
  // Should not return
  //
  EFI_DEADLOOP ();
}
#else
VOID
CapsuleReset (
  )
/*++

Routine Description:

  If need be, do any special reset required for capsules. For this
  implementation where we're called from the ResetSystem() api,
  just set our capsule variable and return to let the caller
  do a soft reset.

Arguments:

  None

Returns:

  None

--*/
{
  EFI_STATUS     Status;
  UINTN          CapsuleDataPtr;
  UINTN          Size;
  UINT32  Data32;
  UINT32  Eflags;
  UINT16  AcpiBase;

  ///
  /// Check if there are pending capsules to process
  ///
  Size = sizeof (CapsuleDataPtr);
  Status = EfiGetVariable (
            EFI_CAPSULE_VARIABLE_NAME,
            &gEfiCapsuleVendorGuid,
            NULL,
            &Size,
            (VOID *) &CapsuleDataPtr
            );

  if (Status == EFI_SUCCESS) {
    ///
    /// Wake up system 2 seconds after putting system into S3 to complete the reset operation.
    ///
    SetSystemWakeupAlarm (2);

    //
    // Read the PM1_CNT register
    //
    AcpiBase = MmioRead16 (
                 MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_PCH,
                   PCI_DEVICE_NUMBER_PCH_LPC,
                   PCI_FUNCTION_NUMBER_PCH_LPC,
                   R_PCH_LPC_ACPI_BASE
                 )
               ) & B_PCH_LPC_ACPI_BASE_BAR;
    ASSERT (AcpiBase != 0);
  
    Data32  = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT));
  
    Data32  = (UINT32) ((Data32 &~(B_PCH_ACPI_PM1_CNT_SLP_TYP + B_PCH_ACPI_PM1_CNT_SLP_EN)) | V_PCH_ACPI_PM1_CNT_S3);
  
    Eflags  = (UINT32) AsmReadEflags ();
  
    if ((Eflags & 0x200)) {
      DisableInterrupts ();
    }
  
    AsmWbinvd ();
    AsmWriteCr0 (AsmReadCr0 () | 0x060000000);
  
    IoWrite32 (
      (UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT),
      (UINT32) Data32
      );
  
    Data32 = Data32 | B_PCH_ACPI_PM1_CNT_SLP_EN;
  
    IoWrite32 (
      (UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT),
      (UINT32) Data32
      );
  
    if ((Eflags & 0x200)) {
      EnableInterrupts ();
    }
    //
    // Should not return
    //
    CpuDeadLoop ();
  }
}
#endif

VOID
EFIAPI
IntelPchResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN VOID             *ResetData OPTIONAL
  )
/*++

Routine Description:

  Reset the system.

Arguments:

  ResetType             Warm or cold
  ResetStatus           Possible cause of reset
  DataSize              Size of ResetData in bytes
  ResetData             Optional Unicode string

Returns:

  Does not return if the reset takes place.
  EFI_INVALID_PARAMETER   If ResetType is invalid.

--*/
{
  UINT8          InitialData;
  UINT8          OutputData;
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  UINT8          *CapsuleDataPtr;
  UINTN          CapsuleData;
#endif
  UINT16         AcpiBase;
  UINT16         Data16;
  UINT32         Data32;

//  DEBUG ((EFI_D_INFO, "IntelPchResetSystem() Start\n"));
//EIP188072 >>
//AMI_OVERRIDE_START >>
{
  UINT32                  Index;

  for (Index = 0; SbResetCallbackList[Index] != NULL; Index++) {
    SbResetCallbackList[Index](ResetType, ResetStatus, DataSize, ResetData);
  }
}
//EIP188072 <<
  switch (ResetType) {
    case EfiResetWarm:
      CapsuleReset ();
      InitialData = V_PCH_RST_CNT_HARDSTARTSTATE;
      OutputData  = V_PCH_RST_CNT_HARDRESET;
      break;
      //
      // For update resets, the reset data is a null-terminated string followed
      // by a VOID * to the capsule descriptors. Get the pointer and set the
      // capsule variable before we do a warm reset. Per the EFI 1.10 spec, the
      // reset data is only valid if ResetStatus != EFI_SUCCESS.
      //
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))

    case EfiResetUpdate:
      if ((ResetStatus != EFI_SUCCESS) && (ResetData != NULL)) {
        CapsuleDataPtr = (UINT8 *) ResetData;
        while (*(UINT16 *) CapsuleDataPtr != 0) {
          CapsuleDataPtr += sizeof (UINT16);
        }

        CapsuleDataPtr += sizeof (UINT16);
        CapsuleData = *(UINTN *) CapsuleDataPtr;
        //
        // If CapsuleReset() returns, then do a soft reset (default)
        //
        CapsuleReset (CapsuleData);
      }

      InitialData = V_PCH_RST_CNT_HARDSTARTSTATE;
      OutputData  = V_PCH_RST_CNT_HARDRESET;
      break;
#endif

    case EfiResetCold:
      InitialData = V_PCH_RST_CNT_HARDSTARTSTATE;
      OutputData  = V_PCH_RST_CNT_HARDRESET;
      break;

    case EfiResetShutdown:
      AcpiBase = (UINT16)mPchResetInstance->AcpiBar;
      //
      // Then, GPE0_EN should be disabled to avoid any GPI waking up the system from S5
      //
      Data16 = 0;

      IoWrite16 ((UINTN) (AcpiBase + R_PCH_ACPI_GPE0a_EN), Data16);
      //
      // Clear Sleep SMI Status
      //
      IoWrite16 (AcpiBase + R_PCH_SMI_STS,
                 (UINT16)(IoRead16 (AcpiBase + R_PCH_SMI_STS) | B_PCH_SMI_STS_ON_SLP_EN));
      //
      // Clear Sleep Type Enable
      //
// AMI_OVERRIDE - Capsule supported. (EIP127538+) >>
//    IoWrite16 (AcpiBase + R_PCH_SMI_EN,
//                  (UINT16)(IoRead16 (AcpiBase + R_PCH_SMI_EN) & (~B_PCH_SMI_EN_ON_SLP_EN)));
// AMI_OVERRIDE - Capsule supported. (EIP127538+) <<
      //
      // Clear Power Button Status
      //
      IoWrite16(AcpiBase + R_PCH_ACPI_PM1_STS, B_PCH_ACPI_PM1_STS_PWRBTN);

      //
      // Secondly, Power Button Status bit must be cleared
      //
      // Write a "1" to bit[8] of power button status register at
      // (ABASE + PM1_STS) to clear this bit
      // Clear it through SMI Status register
      //
      Data16 = B_PCH_SMI_STS_PM1_STS_REG;
      IoWrite16 ((UINTN) (AcpiBase + R_PCH_SMI_STS), Data16);

      //
      // Finally, transform system into S5 sleep state
      //
      Data32  = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT));

      Data32  = (UINT32) ((Data32 &~(B_PCH_ACPI_PM1_CNT_SLP_TYP + B_PCH_ACPI_PM1_CNT_SLP_EN)) | V_PCH_ACPI_PM1_CNT_S5);

      IoWrite32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT), Data32);

      Data32 = Data32 | B_PCH_ACPI_PM1_CNT_SLP_EN;

      IoWrite32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT), Data32);
      //
      // Should not return
      //
      CpuDeadLoop();

      return ;

    default:
      return ;
  }

//  DEBUG ((EFI_D_INFO, "IntelPchResetSystem() End\n"));

  IoWrite8 (
    (UINTN) R_PCH_RST_CNT,
    (UINT8) InitialData
    );

  IoWrite8 (
    (UINTN) R_PCH_RST_CNT,
    (UINT8) OutputData
    );

  //
  // Given we should have reset getting here would be bad
  //

  CpuDeadLoop();

  ASSERT (FALSE);
}

EFI_STATUS
EFIAPI
PchExtendedReset (
  IN     EFI_PCH_EXTENDED_RESET_PROTOCOL   *This,
  IN     PCH_EXTENDED_RESET_TYPES          PchExtendedResetTypes
  )
/*++

Routine Description:

  Execute Pch Extended Reset from the host controller.

Arguments:

  This                    Pointer to the EFI_PCH_EXTENDED_RESET_PROTOCOL instance.
  PchExtendedResetTypes   Pch Extended Reset Types which includes PowerCycle, Globalreset.

Returns:

  EFI_SUCCESS             Successfully completed.
  EFI_INVALID_PARAMETER   If ResetType is invalid.

--*/
{
  DEBUG ((EFI_D_INFO, "PchExtendedReset() Start\n"));
  //
  // Check if the parameters are valid.
  //
  if ((PchExtendedResetTypes.PowerCycle == 0) && (PchExtendedResetTypes.GlobalReset == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (PchExtendedResetTypes.GlobalReset == 1) {
    MmioOr32 (mPchResetInstance->PmcBase + R_PCH_PMC_PMIR, (UINT32) (B_PCH_PMC_PMIR_CF9GR));
  }

  DEBUG ((EFI_D_INFO, "PchExtendedReset() End\n"));

  IoWrite8 ((UINTN) R_PCH_RST_CNT, (UINT8) V_PCH_RST_CNT_HARDRESET);

  //
  // Waiting for system reset
  //
  CpuDeadLoop();


  return EFI_SUCCESS;
}

VOID
PchResetVirtualddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Fixup internal data pointers so that the services can be called in virtual mode.

Arguments:

  Event                   The event registered.
  Context                 Event context. Not used in this event handler.

Returns:

  None

--*/
{
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mPchResetInstance->PchExtendedResetProtocol.Reset));
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mPchResetInstance->CapsuleVariableName));
#endif
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mPchResetInstance->AcpiBar));
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mPchResetInstance->PmcBase));
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mPchResetInstance));
}
