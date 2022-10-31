//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
// $Revision:  $
//
// $Date:  $
//**********************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbAcpiTimerLib.c
//
// Description: This file contains south bridge related TimeLib library
//              instance
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Base.h>
#include <PchAccess.h>
#include <token.h>
#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InternalAcpiGetTimerTick
//
// Description: Internal function to read the current tick counter of ACPI
//
// Input:       None
//
// Output:      The tick counter read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

STATIC
UINT32
InternalAcpiGetTimerTick (
  VOID
  )
{
  return IoRead32 (PcdGet16 (PcdAcpiIoPortBaseAddress) + R_PCH_ACPI_PM1_TMR);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InternalAcpiDelay
//
// Description: Stalls the CPU for at least the given number of ticks.
//              It's invoked by MicroSecondDelay() and NanoSecondDelay().
//
// Input:       Delay     A period of time to delay in ticks
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

STATIC
VOID
InternalAcpiDelay (
  IN      UINT32                    Delay
  )
{
  UINT32                            Ticks;
  UINT32                            Times;

  Times    = Delay >> 22;
  Delay   &= BIT22 - 1;
  do {
    //
    // The target timer count is calculated here
    //
    Ticks    = InternalAcpiGetTimerTick () + Delay;
    Delay    = BIT22;
    //
    // Wait until time out
    // Delay >= 2^23 could not be handled by this function
    // Timer wrap-arounds are handled correctly by this function
    //
    while (((Ticks - InternalAcpiGetTimerTick ()) & BIT23) == 0) {
      CpuPause ();
    }
  } while (Times-- > 0);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MicroSecondDelay
//
// Description: This function delays for the number of micro seconds passed in
//
// Input:       MicroSeconds Number of microseconds(us) to delay
//
// Output:      Value passed in for microseconds delay
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINTN
EFIAPI
MicroSecondDelay (
  IN      UINTN                     Microseconds
  )
{
    UINTN   Ticks;
    UINTN   Counts;
    UINT32  CurrentTick;
    UINT32  OriginalTick;
    UINT32  RemainingTick;

    if (Microseconds == 0) {
      return Microseconds;
    }

    OriginalTick = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR);
    OriginalTick &= (V_PCH_ACPI_PM1_TMR_MAX_VAL - 1);
    CurrentTick = OriginalTick;

    //
    // The timer frequency is 3.579545MHz, so 1 ms corresponds to 3.58 clocks
    //
    Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

    //
    // The loops needed for timer overflow
    //
    Counts = (UINTN)RShiftU64((UINT64)Ticks, 24);

    //
    // Remaining clocks within one loop
    //
    RemainingTick = Ticks & 0xFFFFFF;

    //
    // Do not intend to use TMROF_STS bit of register PM1_STS, because this add extra
    // one I/O operation, and may generate SMI
    //
    while (Counts != 0) {
      CurrentTick = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR) & B_PCH_ACPI_PM1_TMR_VAL;
      if (CurrentTick <= OriginalTick) {
        Counts--;
      }
      OriginalTick = CurrentTick;
    }

    while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
      OriginalTick  = CurrentTick;
      CurrentTick   = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR) & B_PCH_ACPI_PM1_TMR_VAL;
    }
  return Microseconds;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NanoSecondDelay
//
// Description: This function delays for the number of nano seconds passed in
//
// Input:       MicroSeconds Number of nanoseconds(ns) to delay
//
// Output:      Value passed in for nanoseconds delay
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINTN
EFIAPI
NanoSecondDelay (
  IN      UINTN                     NanoSeconds
  )
{
  InternalAcpiDelay (
    (UINT32)DivU64x32 (
              MultU64x32 (
                NanoSeconds,
                V_PCH_ACPI_PM1_TMR_FREQUENCY
                ),
              1000000000u
              )
    );
  return NanoSeconds;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetPerformanceCounter
//
// Description: Retrieves the current value of a 64-bit free running
//              performance counter.
//
// Input:       None
//
// Output:      The current value of the free running performance counter.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT64
EFIAPI
GetPerformanceCounter (
  VOID
  )
{
  return (UINT64)InternalAcpiGetTimerTick ();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetPerformanceCounterProperties
//
// Description: Retrieves the 64-bit frequency in Hz and the range of
//              performance counter values.
//
// Input:       StartValue  The value the performance counter starts with when
//                          it rolls over.
//              EndValue    The value that the performance counter ends with
//                          before it rolls over.
//
// Output:      The frequency in Hz.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT64
EFIAPI
GetPerformanceCounterProperties (
  OUT      UINT64                    *StartValue,  OPTIONAL
  OUT      UINT64                    *EndValue     OPTIONAL
  )
{
  if (StartValue != NULL) {
    *StartValue = 0;
  }

  if (EndValue != NULL) {
    *EndValue = V_PCH_ACPI_PM1_TMR_MAX_VAL - 1;
  }

  return V_PCH_ACPI_PM1_TMR_FREQUENCY;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
