//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//
//*************************************************************************
// $Header:$
//
// $Revision:  $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:    AmiChipsetRuntimeServiceLib.h
//
// Description: 
//
// Notes:
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _AMI_RUNTIME_SERVICE_SMM_LIB_H_
#define _AMI_RUNTIME_SERVICE_SMM_LIB_H_
#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Efi.h>

//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------
typedef EFI_STATUS (SB_RUN_RESET_CALLBACK) (
  IN  EFI_RESET_TYPE  ResetType
);

//-------------------------------------------------------------------------
//Variable, Prototype, and External Declarations
//-------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AmiChipsetResetSystem
//
// Description:
//  This function is the interface for the reset function.
//
// Input:
//  IN EFI_RESET_TYPE ResetType - Type of reset to perform
//  IN EFI_STATUS ResetStatus - System status that caused the reset.  if part of normal operation
//                              then this should be EFI_SUCCESS, Otherwise it should reflect the
//                              state of the system that caused it
//  IN UINTN DataSize - Size in bytes of the data to be logged
//  IN CHAR16 *ResetData - Pointer to the data buffer that is to be logged - OPTIONAL
//
// Output:
//  EFI_DEVICE_ERROR - Even though it should never get that far
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
AmiChipsetResetSystem (
	IN EFI_RESET_TYPE	ResetType,
	IN EFI_STATUS    	ResetStatus,
	IN UINTN         	DataSize,
	IN CHAR16        	*ResetData OPTIONAL
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AmiChipsetGetTime
//
// Description:
//  Return the current date and time
//
// Input:
//    OUT EFI_TIME *Time - Current time filled in EFI_TIME structure
//    OUT EFI_TIME_CAPABILITIES   *Capabilities	- Time capabilities (OPTIONAL)
//
// Output:
//  EFI_SUCCESS		Always
//
// Notes:
//  Here is the control flow of this function:
//  1. Read the original time format 12/24 hours and BCD/binary.
//  2. Set the format to 24 hrs and binary.
//  3. Read the 2 digit year.
//  4. Add either 1900 or 2000, so the year is between 1998 - 2097.
//  5. Read the month, day, hour, minute, second.
//  6. Set the nanosecond to 0.
//  7. Set the time to zone to unspecified.
//  8. Set daylight savings value to 0.
//  9. Restore the original time format.
//  10. Set Capabilities with 1 sec Resolution, 0 Accuracy (Unknown), and False SetsToZero.
//  11. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiChipsetGetTime (
    OUT EFI_TIME                *Time,
    OUT EFI_TIME_CAPABILITIES   *Capabilities OPTIONAL
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AmiChipsetSetTime
//
// Description:
//  Sets the RTC time
//
// Input:
//  IN EFI_TIME *Time - Time to set
//
// Output:
//  EFI_SUCCESS - Time is Set
//  EFI_INVALID_PARAMETER - Time to Set is not valid.
//
// Modified:
//  gTimeZone
//
// Notes:
//  Here is the control flow of this function:
//  1. Read the original time format 12/24 hours and BCD/binary.
//  2. Set the format to 24 hrs and binary.
//  3. Verify the time to set. If it is an invalid time,
//      restore the time format and return EFI_INVALID_PARAMETER.
//  4. Change the 4 digit year to a 2 digit year.
//  5. Stop the RTC time.
//  6. Store time and data on the RTC.
//  7. Read the month, day, hour, minute, second.
//  8. Start the RTC time.
//  9. Restore the original time format.
//  10. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiChipsetSetTime (
    IN EFI_TIME     *Time
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AmiChipsetGetWakeupTime
//
// Description:
//  Read the wake time. Read the status if it is enabled or if the system
//  has woken up.
//
// Input:
//  OUT BOOLEAN *Enabled - Flag indicating the validity of wakeup time
//  OUT BOOLEAN *Pending - Check if wake up time has expired.
//  OUT EFI_TIME *Time - Current wake up time setting
//
// Output:
//  EFI_STATUS
//      EFI_SUCCESS (Always)
//
// Notes:
//  Here is the control flow of this function:
//  1. Read the original time format 12/24 hours and BCD/binary.
//  2. Set the format to 24 hrs and binary.
//  3. Read the status if the wake up time is enabled or if it has expired.
//  4. Set the wakeup time.
//  5. Restore the original time format.
//  6. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiChipsetGetWakeupTime (
    OUT BOOLEAN     *Enabled,
    OUT BOOLEAN     *Pending,
    OUT EFI_TIME    *Time
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AmiChipsetSetWakeupTime
//
// Description:
//  Enable/disable and set wakeup time
//
// Input:
//  IN BOOLEAN Enable - Flag indicating whether to enable/disble the time
//  IN EFI_TIME *Time - Time to set as the wake up time - OPTIONAL
//
// Output:
//  EFI_SUCCESS - Time is Set and/or Enabled/Disabled.
//  EFI_INVALID_PARAMETER - Invalid time or enabling with a NULL Time.
//
// Notes:
//  Here is the control flow of this function:
//  1. Read the original time format 12/24 hours and BCD/binary.
//  2. If Time is not NULL,
//      a. Verify the wakeup time to set. If it is an invalid time,
//          restore the time format and return EFI_INVALID_PARAMETER.
//     	b. Set the wakeup time.
//  3. If Time is NULL and Enable is true, restore original time format
//      and return EFI_INVALID_PARAMETER.
//  4. Enable/Disable wakeup.
//  5. Restore the original time format.
//  6. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiChipsetSetWakeupTime (
    IN BOOLEAN      Enable,
    IN EFI_TIME     *Time OPTIONAL
);

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif //_AMI_RUNTIME_SERVICE_SMM_LIB_H_
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

