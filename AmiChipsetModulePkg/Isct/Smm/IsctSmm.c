//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  IsctSmm.c
//
// Description:	Main implementation source file for the Isct SMM driver
//
//<AMI_FHDR_END>
//**********************************************************************

#include "PchRegs.h"

#include <Library/BaseLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/SmmIoLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSxDispatch.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>

#include <IsctNvsArea.h>
#include <IsctPersistentData.h>

#define R_PCH_RTC_INDEX                           0x70
#define R_PCH_RTC_TARGET                          0x71
#define R_PCH_RTC_EXT_INDEX                       0x72
#define R_PCH_RTC_EXT_TARGET                      0x73
#define R_PCH_RTC_REGA                            0x0A
#define B_PCH_RTC_REGA_UIP                        0x80
#define R_PCH_RTC_REGB                            0x0B
#define B_PCH_RTC_REGB_SET                        0x80
#define B_PCH_RTC_REGB_PIE                        0x40
#define B_PCH_RTC_REGB_AIE                        0x20
#define B_PCH_RTC_REGB_UIE                        0x10
#define B_PCH_RTC_REGB_DM                         0x04
#define B_PCH_RTC_REGB_HOURFORM                   0x02
#define R_PCH_RTC_REGC                            0x0C
#define R_PCH_RTC_REGD                            0x0D


//#include EFI_PROTOCOL_CONSUMER (LoadedImage)
//#include EFI_PROTOCOL_DEPENDENCY (SmmBase)
//#include EFI_PROTOCOL_DEPENDENCY (SmmSxDispatch)
//#include EFI_PROTOCOL_DEPENDENCY (DevicePath)
//#include EFI_PROTOCOL_DEPENDENCY (IsctNvsArea)
//#include EFI_GUID_DEFINITION (IsctPersistentData)

//
// Module global variables
//
//EFI_GUID  gIsctNvsAreaProtocolGuid = ISCT_NVS_AREA_PROTOCOL_GUID;

ISCT_PERSISTENT_DATA              *mIsctData;
ISCT_NVS_AREA                     *mIsctNvs;

STATIC UINT8 mDaysOfMonthInfo[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define R_PCH_RTC_INDEX_ALT       0x74
#define R_PCH_RTC_TARGET_ALT      0x75
#define R_PCH_RTC_EXT_INDEX_ALT   0x76
#define R_PCH_RTC_EXT_TARGET_ALT  0x77

#define R_PCH_RTC_REGC            0x0C
#define B_PCH_RTC_REGC_AF         0x20

#define RTC_INDEX_MASK            0x7F
#define RTC_BANK_SIZE             0x80

#define R_PCH_RTC_SECOND          0x00
#define R_PCH_RTC_ALARM_SECOND    0x01
#define R_PCH_RTC_MINUTE          0x02
#define R_PCH_RTC_ALARM_MINUTE    0x03
#define R_PCH_RTC_HOUR            0x04
#define R_PCH_RTC_ALARM_HOUR      0x05
#define R_PCH_RTC_DAY_OF_WEEK     0x06
#define R_PCH_RTC_DAY_OF_MONTH    0x07
#define R_PCH_RTC_MONTH           0x08
#define R_PCH_RTC_YEAR            0x09

#define DAY_IN_SEC                (24 * 60 * 60)


#pragma pack(1)
typedef struct {
    UINT16  Year;
    UINT8   Month;
    UINT8   Date;
    UINT8   Hour;
    UINT8   Minute;
    UINT8   Second;
} RTC_TIME;
#pragma pack()


UINT8
RtcRead (
    IN      UINT8        Location
)
/*++

Routine Description:

  Read specific RTC/CMOS RAM

Arguments:

  Location        Point to RTC/CMOS RAM offset for read

Returns:

  The data of specific location in RTC/CMOS RAM.

--*/
{
    UINT8 RtcIndexPort;
    UINT8 RtcDataPort;

    //
    // CMOS access registers (using alternative access not to handle NMI bit)
    //
    if (Location < RTC_BANK_SIZE) {
        //
        // First bank
        //
        RtcIndexPort  = R_PCH_RTC_INDEX_ALT;
        RtcDataPort   = R_PCH_RTC_TARGET_ALT;
    } else {
        //
        // Second bank
        //
        RtcIndexPort  = R_PCH_RTC_EXT_INDEX_ALT;
        RtcDataPort   = R_PCH_RTC_EXT_TARGET_ALT;
    }

    IoWrite8 (RtcIndexPort, Location & RTC_INDEX_MASK);
    return IoRead8 (RtcDataPort);
}

BOOLEAN
RtcIsAlarmEnabled (
    VOID
)
/*++

Routine Description:

  Check if RTC Alarm has been enabled.

Arguments:

  None

Returns:

  TRUE      RTC Alarm is enabled
  FALSE     RTC Alarm is not enabled

--*/
{
    return (RtcRead (R_PCH_RTC_REGB) & B_PCH_RTC_REGB_AIE) != 0;
}

STATIC
VOID
RtcWaitEndOfUpdate (
    VOID
)
/*++

Routine Description:

  Wait for updating RTC process finished.

Arguments:

  None

Returns:

  None

--*/
{
    while (RtcRead (R_PCH_RTC_REGA) & B_PCH_RTC_REGA_UIP) {
    }
}

EFI_STATUS
RtcGetAlarm (
    OUT      RTC_TIME        *tm
)
/*++

Routine Description:

  Get current RTC Alarm time.

Arguments:

  tm                  A structure which will be updated with current RTC Alarm time

Returns:

  EFI_NOT_STARTED      RTC Alarm has not been enabled yet.
  EFI_SUCCESS          RTC Alarm enabled and RTC_TIME structure contain current Alarm time setting.

--*/
{
    ASSERT (tm != NULL);
    if (!RtcIsAlarmEnabled ()) {
        return EFI_NOT_STARTED;
    }

    RtcWaitEndOfUpdate ();
    tm->Second  = BcdToDecimal8 (RtcRead (R_PCH_RTC_ALARM_SECOND));
    tm->Minute  = BcdToDecimal8 (RtcRead (R_PCH_RTC_ALARM_MINUTE));
    tm->Hour    = BcdToDecimal8 (RtcRead (R_PCH_RTC_ALARM_HOUR));
    tm->Date    = BcdToDecimal8 (RtcRead (R_PCH_RTC_REGD) & 0x3F);
    tm->Month   = 0;
    tm->Year    = 0;
    return EFI_SUCCESS;
}

VOID
RtcWrite (
    IN      UINT8        Location,
    IN      UINT8        Value
)
/*++

Routine Description:

  Write specific RTC/CMOS RAM

Arguments:

  Location        Point to RTC/CMOS RAM offset for write
  Value           The data that will be written to RTC/CMOS RAM

Returns:

  None

--*/
{
    UINT8 RtcIndexPort;
    UINT8 RtcDataPort;

    //
    // CMOS access registers (using alternative access not to handle NMI bit)
    //
    if (Location < RTC_BANK_SIZE) {
        //
        // First bank
        //
        RtcIndexPort  = R_PCH_RTC_INDEX_ALT;
        RtcDataPort   = R_PCH_RTC_TARGET_ALT;
    } else {
        //
        // Second bank
        //
        RtcIndexPort  = R_PCH_RTC_EXT_INDEX_ALT;
        RtcDataPort   = R_PCH_RTC_EXT_TARGET_ALT;
    }

    IoWrite8 (RtcIndexPort, Location & RTC_INDEX_MASK);
    IoWrite8 (RtcDataPort, Value);
}

EFI_STATUS
RtcSetAlarm (
    IN      RTC_TIME        *tm
)
/*++

Routine Description:

  Set RTC Alarm with specific time

Arguments:

  tm             A time interval structure which will be used to setup an RTC Alarm

Returns:

  EFI_SUCCESS     RTC Alarm has been enabled with specific time interval

--*/
{
    UINT8 RegB;

    ASSERT (tm != NULL);
    //EFI_DEADLOOP();
    RegB = RtcRead (R_PCH_RTC_REGB);

    RtcWaitEndOfUpdate ();

    //
    // Inhibit update cycle
    //
    RtcWrite (R_PCH_RTC_REGB, RegB | B_PCH_RTC_REGB_SET);

    RtcWrite (R_PCH_RTC_ALARM_SECOND, DecimalToBcd8 (tm->Second));
    RtcWrite (R_PCH_RTC_ALARM_MINUTE, DecimalToBcd8 (tm->Minute));
    RtcWrite (R_PCH_RTC_ALARM_HOUR, DecimalToBcd8 (tm->Hour));
    RtcWrite (R_PCH_RTC_REGD, DecimalToBcd8 (tm->Date));

    //
    // Allow update cycle and enable wake alarm
    //
    RegB &= ~B_PCH_RTC_REGB_SET;
    RtcWrite (R_PCH_RTC_REGB, RegB | B_PCH_RTC_REGB_AIE);

    return EFI_SUCCESS;
}

EFI_STATUS
RtcGetTime (
    OUT      RTC_TIME        *tm
)
/*++

Routine Description:

  Get current RTC time

Arguments:

  tm             RTC time structure including Second, Minute and Hour, Date, Month, Year.

Returns:

  EFI_SUCCESS     Operation successfully and RTC_TIME structure contained current time.

--*/
{
    ASSERT (tm != NULL);
    RtcWaitEndOfUpdate ();
    tm->Second  = BcdToDecimal8 (RtcRead (R_PCH_RTC_SECOND));
    tm->Minute  = BcdToDecimal8 (RtcRead (R_PCH_RTC_MINUTE));
    tm->Hour    = BcdToDecimal8 (RtcRead (R_PCH_RTC_HOUR));
    tm->Date    = BcdToDecimal8 (RtcRead (R_PCH_RTC_DAY_OF_MONTH));
    tm->Month   = BcdToDecimal8 (RtcRead (R_PCH_RTC_MONTH));
    tm->Year    = (UINT16)BcdToDecimal8 (RtcRead (R_PCH_RTC_YEAR)) + 2000;
    return EFI_SUCCESS;
}

STATIC
UINT32
TimeToSeconds (
    IN      RTC_TIME        *tm
)
/*++

Routine Description:

  Convert RTC_TIME structure data to seconds

Arguments:

  tm          A time data structure including second, minute and hour fields.

Returns:

  A number of seconds converted from given RTC_TIME structure data.

--*/
{
    ASSERT (tm->Hour < 24);
    ASSERT (tm->Minute < 60);
    ASSERT (tm->Second < 60);
    return ((tm->Hour * 60) + tm->Minute) * 60 + tm->Second;
}

STATIC
VOID
SecondsToTime (
    OUT      RTC_TIME        *tm,
    IN       UINT32          Seconds
)
/*++

Routine Description:

  Convert seconds to RTC_TIME structure data

Arguments:

  tm              A time data structure which will be updated with converted value.
  Seconds         Total seconds that will be converted into RTC_TIME

Returns:

  None

--*/
{
    tm->Second = Seconds % 60;
    Seconds /= 60;
    tm->Minute = Seconds % 60;
    Seconds /= 60;
    tm->Hour = Seconds % 24;
    tm->Date = 0;
}

BOOLEAN
IsLeapYear (
    IN UINT16 Year
)
/*++

Routine Description:

  Check if it is leap year

Arguments:

  Year            year to be check

Returns:

  True    year is leap year
  FALSE   year is not a leap year

--*/
{
    return (Year%4 == 0) && ((Year%100 != 0) || (Year%400 == 0));
}

UINT8
DaysOfMonth (
    IN UINT16 Year,
    IN UINT8  Month
)
/*++

Routine Description:

  Get days of the month

Arguments:

  Year            Year number
  Month           Month number, January is 1, Feburary is 2, ... December is 12.

Returns:

  Days  Number of day of the Month of the Year

--*/
{
    UINT8 Days;
    if (Month < 1 || Month > 12) {
        return 0;
    }
    Days = mDaysOfMonthInfo[Month-1];
    if (Month == 2) {
        Days += IsLeapYear(Year);
    }
    return (Days);
}

BOOLEAN
IsOver2Days (
    IN      RTC_TIME        *tm1,
    IN      RTC_TIME        *tm2
)
/*++

Routine Description:

  check if tm2 is after 2 days of tm1

Arguments:

  tm1             First time to compare
  tm2             Second time to compare

Returns:

  True  tm2 is 2 days after tm1
  FALSE tm2 is not 2 days after tm1

--*/
{
    BOOLEAN RetVal;
    RetVal = TRUE;
    if (tm2->Date > tm1->Date) {
        if (tm2->Date - tm1->Date == 1) {
            RetVal = FALSE;;
        }
    } else if ((DaysOfMonth (tm1->Year, tm1->Month) == tm1->Date) && (tm2->Date == 1)) {
        RetVal = FALSE;;
    }
    return RetVal;
}

EFI_STATUS
GetISCTTime (
    IN       UINT32          ISCTRtcDurationTime,
    OUT      RTC_TIME        *tm
)
{
    ASSERT (tm != NULL);
    ISCTRtcDurationTime &= ~BIT31;
    tm->Second  = (UINT8)(ISCTRtcDurationTime & 0x3F);
    tm->Minute  = (UINT8)((ISCTRtcDurationTime >> 6) & 0x3F);
    tm->Hour    = (UINT8)((ISCTRtcDurationTime >> 12) & 0x1F);
    tm->Date    = (UINT8)((ISCTRtcDurationTime >> 17) & 0x1F);
    tm->Month   = (UINT8)((ISCTRtcDurationTime >> 22) & 0x0F);
    tm->Year    = (UINT8)((ISCTRtcDurationTime >> 26) & 0x1F) + 2000;
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
IsctSxEntryCallback (
    IN  UINT8                    SleepState
)
/*++

Routine Description:

  ISCT S3 entry callback SMI handler

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:

  EFI_SUCCESS - Function executed successfully

--*/
{
    EFI_STATUS  Status;
    RTC_TIME    rtc_tm;
    RTC_TIME    wake_tm;
    RTC_TIME    alarm_tm;
    RTC_TIME    Isct_Actual_tm;
    UINT32      IsctDuration;
    UINT32      CurrentTime;
    UINT32      AlarmTime;
    UINT32      WakeTime;
    UINT16      PmBase;
    UINT8       RegB;
    BOOLEAN     UseIsctTimer;
    BOOLEAN     Over2Days;

    // Check RtcDurationTime value.
    if (mIsctNvs->RtcDurationTime == 0) {
        mIsctNvs->IsctOverWrite = 0;
        return EFI_SUCCESS;
    }
    UseIsctTimer = FALSE;
    Over2Days = FALSE;
    mIsctNvs->IsctOverWrite = 0;
    IsctDuration = mIsctNvs->RtcDurationTime;

    // Make sure RTC is in BCD and 24h format
    RegB = RtcRead (R_PCH_RTC_REGB);
    RegB |= B_PCH_RTC_REGB_HOURFORM;
    RegB &= ~B_PCH_RTC_REGB_DM;
    RtcWrite (R_PCH_RTC_REGB, RegB);

    // Get RTC Timer and convert RTC_TIME to seconds
    Status = RtcGetTime (&rtc_tm);
    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    CurrentTime = TimeToSeconds (&rtc_tm);
    if ( (IsctDuration & BIT31) == BIT31 ) {     //Actual time
        GetISCTTime(IsctDuration, &Isct_Actual_tm);
        IsctDuration = TimeToSeconds (&Isct_Actual_tm);
        IsctDuration -= CurrentTime;
    }
    Status = RtcGetAlarm (&wake_tm);
    if (Status == EFI_SUCCESS) {
        AlarmTime = TimeToSeconds (&wake_tm);
        
        // When OS set alarm date to zero,
        // that would mean the alarm date is today or next day depending alarm time,
        // and the alarm will happen in 24 hour.
        if (wake_tm.Date != 0 && wake_tm.Date != rtc_tm.Date) {
          
            // OS Wake-up time is over 1 day
            AlarmTime += DAY_IN_SEC;
            if (IsOver2Days (&rtc_tm, &wake_tm)) {
              
                // OS Wake-up time is over 2 day
                UseIsctTimer = TRUE;
                Over2Days = TRUE;
            }
        } else if (AlarmTime < CurrentTime && wake_tm.Date == 0) {
          
            // When alarm time behind current time and alarm date is zero,
            // OS set the alarm for next day
            AlarmTime += DAY_IN_SEC;
        }

        if ((IsctDuration <= (AlarmTime - CurrentTime)) && (Over2Days == FALSE)) {
            UseIsctTimer = TRUE;
        }
    } else {
        UseIsctTimer = TRUE;
    }

    // if ISCT Timer <= OS RTC alarm timer, then overwrite RTC alarm by ISCT timer
    if (UseIsctTimer == TRUE) {
        WakeTime = CurrentTime + IsctDuration;
        SecondsToTime (&alarm_tm, WakeTime);
        Status = RtcSetAlarm (&alarm_tm);
        ASSERT_EFI_ERROR (Status);

        PmBase = (UINT16) (PciRead32 (
                               PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH,
                                                PCI_DEVICE_NUMBER_PCH_LPC,
                                                PCI_FUNCTION_NUMBER_PCH_LPC,
                                                R_PCH_LPC_ACPI_BASE)
                           ) & B_PCH_LPC_ACPI_BASE_BAR);

        // Clear RTC PM1 status
        IoWrite16 (PmBase + R_PCH_ACPI_PM1_STS, B_PCH_ACPI_PM1_STS_RTC);

        // set RTC_EN bit in PM1_EN to wake up from the alarm
        IoWrite16 (
            PmBase + R_PCH_ACPI_PM1_EN,
            (IoRead16 (PmBase + R_PCH_ACPI_PM1_EN) | B_PCH_ACPI_PM1_EN_RTC)
        );

        mIsctNvs->IsctOverWrite = 1;
    }
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
IsctS3EntryCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

  ISCT S3 entry callback SMI handler

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:

  EFI_SUCCESS - Function executed successfully
  EFI_ABORTED - An error occurred.

--*/
{
  EFI_STATUS  Status;

  Status = IsctSxEntryCallback(0x3);
  if(Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_ABORTED;
  }
}

STATIC
EFI_STATUS
IsctS4EntryCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

  ISCT S4 entry callback SMI handler

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:

  EFI_SUCCESS - Function executed successfully
  EFI_ABORTED - An error occurred.

--*/
{
  EFI_STATUS  Status;

  Status = IsctSxEntryCallback(0x4);
  if(Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return EFI_ABORTED;
  }
}

EFI_STATUS
InstallIsctSmm (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
/*++

Routine Description:

  Isct SMM driver entry point function.

Arguments:

  ImageHandle   - image handle for this driver image
  SystemTable   - pointer to the EFI System Table

Returns:

  EFI_SUCCESS   - driver initialization completed successfully

--*/
{
    EFI_STATUS                        Status;
    EFI_HANDLE                        DispatchHandle;
    EFI_SMM_SX_DISPATCH_PROTOCOL      *SxDispatchProtocol;
    EFI_SMM_SX_DISPATCH_CONTEXT       EntryDispatchContext;
    ISCT_NVS_AREA_PROTOCOL            *IsctNvsAreaProtocol;

    DEBUG ((EFI_D_INFO, "IsctSmm Entry Point- Install\n"));

    //
    // Located ISCT Nvs Protocol
    //
    Status = gBS->LocateProtocol (
                 &gIsctNvsAreaProtocolGuid,
                 NULL,
                 &IsctNvsAreaProtocol
             );
    DEBUG((EFI_D_INFO, "(ISCT SMM) Located ISCT Nvs protocol Status = %r\n", Status));

    if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "No ISCT Nvs protocol available\n"));
        return EFI_SUCCESS;
    }


    if (IsctNvsAreaProtocol->Area->IsctEnabled == 0) {
        DEBUG ((EFI_D_INFO, "ISCT is Disbaled \n"));
        return EFI_SUCCESS;
    }
    
    if (IsctNvsAreaProtocol->Area->IsctRTCTimerSupport == 0) {
        DEBUG ((EFI_D_INFO, "ISCT Timer is not for RTC timer. \n"));
        return EFI_SUCCESS;
    }
    
    //
    // Assign NvsPointer to Global Module Variable
    //
    mIsctData = IsctNvsAreaProtocol->IsctData;
    mIsctNvs = IsctNvsAreaProtocol->Area;

    //
    // Loacted SxDispatchProtocol
    //
    Status = gBS->LocateProtocol (
                 &gEfiSmmSxDispatchProtocolGuid,
                 NULL,
                 &SxDispatchProtocol
             );
    if ( EFI_ERROR(Status) ) {
        DEBUG((EFI_D_INFO, "(ISCT SMM) Located SxDispatchProtocol protocol Status = %r\n", Status));
        return Status;
    }

    //
    // Register S3 entry phase call back function
    //
    EntryDispatchContext.Type   = SxS3;
    EntryDispatchContext.Phase  = SxEntry;
    Status = SxDispatchProtocol->Register (
                 SxDispatchProtocol,
                 IsctS3EntryCallback,
                 &EntryDispatchContext,
                 &DispatchHandle
             );
    DEBUG((EFI_D_INFO, "(ISCT SMM) Register IsctS3EntryCallback Status = %r\n", Status));
    if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_INFO, "IsctS3EntryCallback failed to load.\n"));
        ASSERT_EFI_ERROR (Status);
        return Status;
    }

    //
    // Register S4 entry phase call back function
    //
    EntryDispatchContext.Type   = SxS4;
    EntryDispatchContext.Phase  = SxEntry;
    Status = SxDispatchProtocol->Register (
                 SxDispatchProtocol,
                 IsctS4EntryCallback,
                 &EntryDispatchContext,
                 &DispatchHandle
             );
    DEBUG((EFI_D_INFO, "(ISCT SMM) Register IsctS4EntryCallback Status = %r\n", Status));
    if (EFI_ERROR (Status)) {
        DEBUG((EFI_D_INFO, "IsctS4EntryCallback failed to load.\n"));
        ASSERT_EFI_ERROR (Status);
        return Status;
    }

    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
