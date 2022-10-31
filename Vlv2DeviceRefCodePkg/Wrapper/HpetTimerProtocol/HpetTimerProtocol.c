/**
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file 
  HpetTimerProtocol.c

  @brief 
  Timer Architectural Protocol as defined in the DXE CIS

**/
#include "HpetTimerProtocol.h"

///
/// The handle onto which the Timer Architectural Protocol will be installed
///
EFI_HANDLE                mTimerHandle = NULL;

EFI_EVENT                 gHpetLegacyBootEvent;

// Mask used for counter and comparator calculations to adjust for a 32-bit or 64-bit counter.
UINT64                    gCounterMask;
// Cached state of the HPET General Capabilities register managed by this driver.
// Caching the state reduces the number of times the configuration register is read.
volatile HPET_GENERAL_CAPABILITIES_ID_REGISTER   gHpetGeneralCapabilities;
// Cached state of the HPET General Configuration register managed by this driver.
// Caching the state reduces the number of times the configuration register is read.
volatile HPET_GENERAL_CONFIGURATION_REGISTER     gHpetGeneralConfiguration;
// Cached state of the Configuration register for the HPET Timer managed by
// this driver.  Caching the state reduces the number of times the configuration
// register is read.
volatile HPET_TIMER_CONFIGURATION_REGISTER       gTimerConfiguration;

///
/// The Timer Architectural Protocol that this driver produces
///
EFI_TIMER_ARCH_PROTOCOL   mTimer = {
  TimerDriverRegisterHandler,
  TimerDriverSetTimerPeriod,
  TimerDriverGetTimerPeriod,
  TimerDriverGenerateSoftInterrupt
};

///
/// Pointer to the CPU Architectural Protocol instance
///
EFI_CPU_ARCH_PROTOCOL     *mCpu;

///
/// Pointer to the Legacy 8259 Protocol instance
///
EFI_LEGACY_8259_PROTOCOL  *mLegacy8259;

///
/// The notification function to call on every timer interrupt.
/// A bug in the compiler prevents us from initializing this here.
///
volatile EFI_TIMER_NOTIFY mTimerNotifyFunction;

///
/// The current period of the timer interrupt
///
volatile UINT64           mTimerPeriod = 0;

///
/// The time of twice timer interrupt duration
///
volatile UINTN            mPreAcpiTick = 0;

///
/// PMIO BAR Registers
///
UINT16                    AcpiBase;

UINT64 HpetRead (
  IN UINTN          Offset
  )
{
  return MmioRead64(HPET_BASE_ADDRESS + Offset);
}

UINT64 HpetWrite (
  IN UINTN          Offset,
  IN UINT64         Value
  )
{
  MmioWrite64(HPET_BASE_ADDRESS + Offset, Value);
  return HpetRead(Offset);
}

VOID HpetEnable (
  IN BOOLEAN        Enable
  )
{
  gHpetGeneralConfiguration.Bits.MainCounterEnable = Enable ? 1 : 0;
  HpetWrite(HPET_GENERAL_CONFIGURATION_OFFSET, gHpetGeneralConfiguration.Uint64);
}

VOID StopHpetBeforeLagecyBoot (
  IN EFI_EVENT      Event,
  IN VOID           *Context
  )
{
  //
  // Disable HPET and Legacy Replacement Support.
  //
  HpetEnable(FALSE);
  CountTime((HPET_DEFAULT_TICK_DURATION / 10) * 2, PM_BASE_ADDRESS);
  HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, 0);

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
  IoApicDisableIrq(HPET_APIC_INTERRUPT_PIN);
#else
  gHpetGeneralConfiguration.Bits.LegacyRouteEnable = 0;
  HpetEnable(FALSE);
#endif

  gBS->CloseEvent(Event);
}

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
VOID
Irq0InterruptHandler(
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  EFI_TPL OriginalTPL;

  OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  
  //
  // Clear the interrupt flag
  //
  mLegacy8259->EndOfInterrupt(mLegacy8259, SYSTEM_TIMER_IRQ);

  gBS->RestoreTPL (OriginalTPL);
}
#endif

///
/// Worker Functions
///
VOID
SetPitCount (
  IN UINT16  Count
  )
/**

  @brief 
  Sets the counter value for Timer #0 in a legacy 8254 timer.

  @param[in] Count                The 16-bit counter value to program into Timer #0 of the legacy 8254 timer.

  @retval None

**/
{
  UINT8 Data;
  ///
  /// 0x36 = Read/Write counter LSB then MSB, Mode3 square wave output from this timer.
  /// Check register Counter Access Ports Register(0x40/41/42 for counter0/1/2) in PCH B0D31F0
  /// check Counter Operating Mode 0~5 at 8254 Timer function description in LPC in EDS.
  ///
  Data = 0x36;
  IoWrite8 (TIMER_CONTROL_PORT, Data);
  IoWrite8 (TIMER0_COUNT_PORT, (UINT8) Count);
  IoWrite8 (TIMER0_COUNT_PORT, (UINT8) (Count >> 8));
}

UINT32
GetAcpiTick (
  VOID
  )
/**

  @brief 
  Get the current ACPI counter's value

  @param[in] None

  @retval UINT32                  The value of the counter

**/
{
  UINT32  Tick;

  Tick = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_TMR)) & B_PCH_ACPI_PM1_TMR_VAL;
  return Tick;

}

UINT64
MeasureTimeLost (
  IN UINT64             TimePeriod
  )
/**

  @brief 
  Measure the 8254 timer interrupt use the ACPI time counter

  @param[in] TimePeriod           The current period of the timer interrupt

  @retval UINT64                  The real system time pass between the sequence 8254 timer 
                                  interrupt

**/
{
  UINT32  CurrentTick;
  UINT32  EndTick;
  UINT64  LostTime;

  CurrentTick = GetAcpiTick ();
  EndTick     = CurrentTick;

  if (CurrentTick < mPreAcpiTick) {
    EndTick = CurrentTick + 0x1000000;
  }
  ///
  /// The calculation of the lost system time should be very accurate, we use
  /// the shift calcu to make sure the value's accurate:
  /// the origenal formula is:
  ///                      (EndTick - mPreAcpiTick) * 10,000,000
  ///      LostTime = -----------------------------------------------
  ///                   (3,579,545 Hz / 1,193,182 Hz) * 1,193,182 Hz
  ///
  /// Note: the 3,579,545 Hz is the ACPI timer's clock;
  ///       the 1,193,182 Hz is the 8254 timer's clock;
  ///
  LostTime = RShiftU64 (
              MultU64x32 ((UINT64) (EndTick - mPreAcpiTick),
              46869689) + 0x00FFFFFF,
              24
              );

  if (LostTime != 0) {
    mPreAcpiTick = CurrentTick;
  }

#if (_SIMIC_ || _SLE_HYB_ || _SLE_COMP_ || VP_FLAG)
  //
  // Simulators/emulators ACPI timer run much slower than real.
  //
  return LShiftU64 (LostTime, 6);
#else
  return LostTime;
#endif
}

VOID
TimerInterruptHandler (
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
/**

  @brief 
  8254 Timer #0 Interrupt Handler

  @param[in] InterruptType        The type of interrupt that occured
  @param[in] SystemContext        A pointer to the system context when the interrupt occured

  @retval None

**/
{
  EFI_TPL OriginalTPL;
  static volatile UINT32  StoreCF8;
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) && defined(HPET_INTERRUPT_TRIGGER) && (HPET_INTERRUPT_TRIGGER == 1)
  static volatile UINT64  HpetGenIntSts;
#endif

  //
  // Store CF8 (PCI index)
  //
  StoreCF8 = IoRead32(0xcf8);

  OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
  IoApicEoi(GetHpetApicPin());
#if defined(HPET_INTERRUPT_TRIGGER) && (HPET_INTERRUPT_TRIGGER != 0)
  HpetGenIntSts = HpetRead(HPET_GENERAL_INTERRUPT_STATUS_OFFSET);
  HpetWrite (HPET_GENERAL_INTERRUPT_STATUS_OFFSET, Shl64(BIT0, HPET_OFFSET));
#endif
#else
  mLegacy8259->EndOfInterrupt (mLegacy8259, Efi8259Irq0);
#endif

  if (mTimerNotifyFunction) {
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0) && defined(HPET_INTERRUPT_TRIGGER) && (HPET_INTERRUPT_TRIGGER == 1)
    if (HpetGenIntSts & Shl64(BIT0, HPET_OFFSET)) {
      mNotifyFunction (mProgrammedTimerValue);
    }
#else
    ///
    /// If we have the timer interrupt miss, then we use
    /// the platform ACPI time counter to retrieve the time lost
    ///
    mTimerNotifyFunction (MeasureTimeLost (mTimerPeriod));
#endif
  }

  gBS->RestoreTPL (OriginalTPL);
  
  //
  // Restore 0xCF8 (PCI index)
  //
  IoWrite32(0xcf8, StoreCF8);
}

EFI_STATUS
EFIAPI
TimerDriverRegisterHandler (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN EFI_TIMER_NOTIFY         NotifyFunction
  )
/**

  @brief 
  This function registers the handler NotifyFunction so it is called every time 
  the timer interrupt fires.  It also passes the amount of time since the last 
  handler call to the NotifyFunction.  If NotifyFunction is NULL, then the 
  handler is unregistered.  If the handler is registered, then EFI_SUCCESS is 
  returned.  If the CPU does not support registering a timer interrupt handler, 
  then EFI_UNSUPPORTED is returned.  If an attempt is made to register a handler 
  when a handler is already registered, then EFI_ALREADY_STARTED is returned.  
  If an attempt is made to unregister a handler when a handler is not registered, 
  then EFI_INVALID_PARAMETER is returned.  If an error occurs attempting to 
  register the NotifyFunction with the timer interrupt, then EFI_DEVICE_ERROR 
  is returned.

  @param[in] This                 The EFI_TIMER_ARCH_PROTOCOL instance.
  @param[in] NotifyFunction       The function to call when a timer interrupt fires.  This 
                                  function executes at TPL_HIGH_LEVEL.  The DXE Core will 
                                  register a handler for the timer interrupt, so it can know 
                                  how much time has passed.  This information is used to 
                                  signal timer based events.  NULL will unregister the handler.

  @retval EFI_SUCCESS             The timer handler was registered.
  @exception EFI_UNSUPPORTED      The CPU does not support registering a timer interrupt handler
  @retval EFI_ALREADY_STARTED     NotifyFunction is not NULL, and a handler is already registered.
  @retval EFI_INVALID_PARAMETER   NotifyFunction is NULL, and a handler was not previously registered.

**/
{
  ///
  /// If an attempt is made to unregister a handler when a handler is not registered, 
  /// then EFI_INVALID_PARAMETER is returned.
  ///
  if (mTimerNotifyFunction == NULL && NotifyFunction == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ///
  /// If an attempt is made to register a handler 
  /// when a handler is already registered, then EFI_ALREADY_STARTED is returned.
  ///
  if (mTimerNotifyFunction != NULL && NotifyFunction != NULL) {
    return EFI_ALREADY_STARTED;
  }
  ///
  /// If the CPU does not support registering a timer interrupt handler, then EFI_UNSUPPORTED is returned.
  ///
  if (mCpu == NULL || mLegacy8259 == NULL) {
    return EFI_UNSUPPORTED;
  }

  if (NotifyFunction == NULL) {
    ///
    /// If NotifyFunction is NULL, then the handler is unregistered.
    ///
    mTimerNotifyFunction = NULL;
  } else {
    mTimerNotifyFunction = NotifyFunction;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
TimerDriverSetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                   TimerPeriod
  )
/**

  @brief 
  This function adjusts the period of timer interrupts to the value specified 
  by TimerPeriod.  If the timer period is updated, then the selected timer 
  period is stored in EFI_TIMER.TimerPeriod, and EFI_SUCCESS is returned.  If 
  the timer hardware is not programmable, then EFI_UNSUPPORTED is returned.  
  If an error occurs while attempting to update the timer period, then the 
  timer hardware will be put back in its state prior to this call, and 
  EFI_DEVICE_ERROR is returned.  If TimerPeriod is 0, then the timer interrupt 
  is disabled.  This is not the same as disabling the CPU's interrupts.  
  Instead, it must either turn off the timer hardware, or it must adjust the 
  interrupt controller so that a CPU interrupt is not generated when the timer 
  interrupt fires. 

  @param[in] This                 The EFI_TIMER_ARCH_PROTOCOL instance.
  @param[in] TimerPeriod          The rate to program the timer interrupt in 100 nS units.  If 
                                  the timer hardware is not programmable, then EFI_UNSUPPORTED is 
                                  returned. If the timer is programmable, then the timer period 
                                  will be rounded up to the nearest timer period that is supported 
                                  by the timer hardware.  If TimerPeriod is set to 0, then the 
                                  timer interrupts will be disabled.

  @retval EFI_SUCCESS             The timer period was changed.

**/
{
  UINT64  TimerCount;

  //
  // Disable HPET timer when adjusting the timer period
  //
  HpetEnable(FALSE);

  ///
  ///  The basic clock is 1.19318 MHz or 0.119318 ticks per 100 ns.
  ///  TimerPeriod * 0.119318 = 8254 timer divisor. Using integer arithmetic
  ///  TimerCount = (TimerPeriod * 119318)/1000000.
  ///
  ///  Round up to next highest integer. This guarantees that the timer is
  ///  equal to or slightly longer than the requested time.
  ///  TimerCount = ((TimerPeriod * 119318) + 500000)/1000000
  ///
  /// Note that a TimerCount of 0 is equivalent to a count of 65,536
  ///
  /// Since TimerCount is limited to 16 bits for IA32, TimerPeriod is limited
  /// to 20 bits.
  ///
  if (TimerPeriod == 0) {
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
    IoApicDisableIrq(HPET_APIC_INTERRUPT_PIN);
#endif
    ///
    /// Disable timer interrupt for a TimerPeriod of 0
    ///
    mLegacy8259->DisableIrq (mLegacy8259, Efi8259Irq0);
  } else {
    //
    // Convert TimerPeriod to femtoseconds and divide by the number if
    // femtoseconds per tick of the HPET counter to determine the number
    // of HPET counter ticks in TimerPeriod 100 ns units.
    //
    TimerCount = DivU64x32 (
                  MultU64x32 (TimerPeriod, 100000000),
                  gHpetGeneralCapabilities.Bits.CounterClockPeriod
                  );
    //
    // Reset Main Counter
    //
    HpetWrite(HPET_MAIN_COUNTER_OFFSET, 0);
    //
    // ValueSetEnable must be set if the timer is set to periodic mode.
    //
    gTimerConfiguration.Bits.ValueSetEnable = 1;
    HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);
    //
    // Clear ValueSetEnable bit.
    //
    gTimerConfiguration.Bits.ValueSetEnable = 0;
    HpetWrite(HPET_TIMER_COMPARATOR_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, TimerCount);
    
    //
    // Now enable the interrupt
    //
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
    IoApicEnableIrq(HPET_APIC_INTERRUPT_PIN, HPET_INTERRUPT_TRIGGER, (HPET_INTERRUPT_POLARITY == 0) ? TRUE : FALSE);
#endif

    ///
    /// Enable timer interrupt
    ///
    mLegacy8259->EnableIrq (mLegacy8259, Efi8259Irq0, FALSE);
    
    //
    // Enable HPET Interrupt Generation
    //
    gTimerConfiguration.Bits.InterruptEnable = 1;
    HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);

    //
    // Enable the HPET counter once new timer period has been established
    // The HPET counter should run even if the HPET Timer interrupts are
    // disabled.  This is used to account for time passed while the interrupt
    // is disabled.
    //
    HpetEnable(TRUE);
  }
  ///
  /// Save the new timer period
  ///
  mTimerPeriod = TimerPeriod;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
TimerDriverGetTimerPeriod (
  IN EFI_TIMER_ARCH_PROTOCOL   *This,
  OUT UINT64                   *TimerPeriod
  )
/**

  @brief 
  This function retrieves the period of timer interrupts in 100 ns units, 
  returns that value in TimerPeriod, and returns EFI_SUCCESS.  If TimerPeriod 
  is NULL, then EFI_INVALID_PARAMETER is returned.  If a TimerPeriod of 0 is 
  returned, then the timer is currently disabled.

  @param[in] This                 The EFI_TIMER_ARCH_PROTOCOL instance.
  @param[in] TimerPeriod          A pointer to the timer period to retrieve in 100 ns units. 
                                  If 0 is returned, then the timer is currently disabled.

  @retval EFI_SUCCESS             The timer period was returned in TimerPeriod.
  @retval EFI_INVALID_PARAMETER   TimerPeriod is NULL.

**/
{
  if (TimerPeriod == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *TimerPeriod = mTimerPeriod;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
TimerDriverGenerateSoftInterrupt (
  IN EFI_TIMER_ARCH_PROTOCOL  *This
  )
/**

  @brief 
  This function generates a soft timer interrupt. If the platform does not support soft 
  timer interrupts, then EFI_UNSUPPORTED is returned. Otherwise, EFI_SUCCESS is returned. 
  If a handler has been registered through the EFI_TIMER_ARCH_PROTOCOL.RegisterHandler() 
  service, then a soft timer interrupt will be generated. If the timer interrupt is 
  enabled when this service is called, then the registered handler will be invoked. The 
  registered handler should not be able to distinguish a hardware-generated timer 
  interrupt from a software-generated timer interrupt.

  @param[in] This                 The EFI_TIMER_ARCH_PROTOCOL instance.

  @retval EFI_SUCCESS             The soft timer interrupt was generated.

**/
{
  EFI_STATUS  Status;
  UINT16      IRQMask;
  EFI_TPL     OriginalTPL;

  DEBUG ((EFI_D_INFO, "TimerDriverGenerateSoftInterrupt() Start\n"));

  ///
  /// If the timer interrupt is enabled, then the registered handler will be invoked.
  ///
  Status = mLegacy8259->GetMask (mLegacy8259, NULL, NULL, &IRQMask, NULL);
  ASSERT_EFI_ERROR (Status);
  if ((IRQMask & 0x1) == 0) {
    ///
    /// Invoke the registered handler
    ///
    OriginalTPL = gBS->RaiseTPL (TPL_HIGH_LEVEL);

    if (mTimerNotifyFunction) {
      ///
      /// We use the platform ACPI time counter to determine
      /// the amount of time that has passed
      ///
      mTimerNotifyFunction (MeasureTimeLost (mTimerPeriod));
    }

    gBS->RestoreTPL (OriginalTPL);
  }

  DEBUG ((EFI_D_INFO, "TimerDriverGenerateSoftInterrupt() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HpetTimerDriverInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/**

  @brief 
  Initialize the Timer Architectural Protocol driver

  @param[in] ImageHandle          ImageHandle of the loaded driver
  @param[in] SystemTable          Pointer to the System Table

  @retval EFI_SUCCESS             Timer Architectural Protocol created
  @retval Other                   Failed

**/
{
  EFI_STATUS  Status;
  UINT32      TimerVector;
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
  UINT32      Irq0TimerVector;
#endif
  UINT8       Value;

  DEBUG ((EFI_D_INFO, "HpetTimerDriverInitialize() Start\n"));

  ///
  /// Initialize the pointer to our notify function.
  ///
  mTimerNotifyFunction  = NULL;
  mCpu                  = NULL;
  mLegacy8259           = NULL;

  ///
  /// Make sure the Timer Architectural Protocol is not already installed in the system
  ///
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiTimerArchProtocolGuid);

  ///
  /// Find the CPU architectural protocol.  ASSERT if not found.
  ///
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &mCpu);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Find the Legacy8259 protocol.  ASSERT if not found.
  ///
  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &mLegacy8259);
  ASSERT_EFI_ERROR (Status);

  AcpiBase = MmioRead16 (
               MmPciAddress (0,
                 DEFAULT_PCI_BUS_NUMBER_PCH,
                 PCI_DEVICE_NUMBER_PCH_LPC,
                 PCI_FUNCTION_NUMBER_PCH_LPC,
                 R_PCH_LPC_ACPI_BASE
               )
             ) & B_PCH_LPC_ACPI_BASE_BAR;
  ASSERT (AcpiBase != 0);
  
  //
  // Retrieve HPET Capabilities and Configuration Information
  //
  gHpetGeneralCapabilities.Uint64  = HpetRead(HPET_GENERAL_CAPABILITIES_ID_OFFSET);
  gHpetGeneralConfiguration.Uint64 = HpetRead(HPET_GENERAL_CONFIGURATION_OFFSET);

  //
  // If Revision is not valid, then ASSERT() and unload the driver because the HPET
  // device is not present.
  //
  if(gHpetGeneralCapabilities.Uint64 == 0 || gHpetGeneralCapabilities.Uint64 == 0xFFFFFFFFFFFFFFFF) {
      DEBUG ((EFI_D_INFO, "HPET device is not present.  Unload HPET driver.\n"));
      return EFI_DEVICE_ERROR;
  }

  HpetEnable(FALSE);

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE == 0)
  //
  // Enable Legacy Interrupt
  //
  gHpetGeneralConfiguration.Bits.LegacyRouteEnable = 1;
#endif

  ///
  /// Force the timer to be disabled
  ///
  Status = TimerDriverSetTimerPeriod (&mTimer, 0);
  ASSERT_EFI_ERROR (Status);

  //
  // Configure the selected HPET Timer (Timer#0), clear InterruptEnable to keep
  // interrupts disabled until full init is complete
  // Enable PeriodicInterruptEnable to use perioidic mode
  // Configure as a 32-bit counter
  gTimerConfiguration.Uint64 = HpetRead(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE);
  gTimerConfiguration.Bits.InterruptEnable         = 0;
  gTimerConfiguration.Bits.PeriodicInterruptEnable = 1;
  gTimerConfiguration.Bits.CounterSizeEnable       = 1;
  gTimerConfiguration.Bits.LevelTriggeredInterrupt = 0;
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
  gTimerConfiguration.Bits.InterruptRoute = HPET_APIC_INTERRUPT_PIN;
  gTimerConfiguration.Bits.LevelTriggeredInterrupt = HPET_INTERRUPT_TRIGGER;
#endif
  HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);

  //
  // Read the HPET Timer Capabilities and Configuration register back again.
  // CounterSizeEnable will be read back as a 0 if it is a 32-bit only timer
  //
  gTimerConfiguration.Uint64 = HpetRead(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE);
#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
  //
  // If the interrupt pin isn't supported by the particular timer, then the value read back won't match that is written.
  //
  if(gTimerConfiguration.Bits.InterruptRoute != HPET_APIC_INTERRUPT_PIN) {
    ASSERT_EFI_ERROR(EFI_UNSUPPORTED);
    return EFI_UNSUPPORTED;
  }
#endif

  if((gTimerConfiguration.Bits.CounterSizeEnable == 1) && (sizeof(UINTN) == sizeof(UINT64))) {
    //
    // 64-bit BIOS can use 64-bit HPET timer
    //
    gCounterMask = 0xffffffffffffffff;
    //
    // Set timer back to 64-bit
    //
    gTimerConfiguration.Bits.CounterSizeEnable = 0;
    HpetWrite(HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE, gTimerConfiguration.Uint64);
  } else {
    gCounterMask = 0x00000000ffffffff;
  }

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
  TimerVector = MASTER_INTERRUPT_BASE + HPET_APIC_INTERRUPT_PIN;

  Status      = mLegacy8259->GetVector (mLegacy8259, Efi8259Irq0, (UINT8 *) &Irq0TimerVector);
  ASSERT_EFI_ERROR(Status);

  Status      = mCpu->RegisterInterruptHandler (mCpu, Irq0TimerVector, Irq0InterruptHandler);
  ASSERT_EFI_ERROR(Status);
#else
  ///
  /// Get the interrupt vector number corresponding to IRQ0 from the 8259 driver
  ///
  TimerVector = 0;
  Status      = mLegacy8259->GetVector (mLegacy8259, Efi8259Irq0, (UINT8 *) &TimerVector);
  ASSERT_EFI_ERROR (Status);
#endif

  ///
  /// Install interrupt handler for 8254 Timer #0 (ISA IRQ0)
  ///
  Status = mCpu->RegisterInterruptHandler (mCpu, TimerVector, TimerInterruptHandler);
  ASSERT_EFI_ERROR (Status);

  //
  // Init default for Timer 0
  //
  IoWrite8(TIMER_CONTROL_PORT, 0x36);
  IoWrite8(TIMER0_COUNT_PORT, 0);
  IoWrite8(TIMER0_COUNT_PORT, 0);
  //
  // Add boot script programming
  //
  Value = 0x36;
  S3BootScriptSaveIoWrite (   
    EfiBootScriptWidthUint8,
    (UINTN) (TIMER_CONTROL_PORT),
    1,
    &Value
    );
  Value = 0x0;
  S3BootScriptSaveIoWrite (   
    EfiBootScriptWidthUint8,
    (UINTN) (TIMER0_COUNT_PORT),
    1,
    &Value
    );
  S3BootScriptSaveIoWrite (   
    EfiBootScriptWidthUint8,
    (UINTN) (TIMER0_COUNT_PORT),
    1,
    &Value
    );
  
  //
  // The default value of 10000 100 ns units is the same as 1 ms.
  //
  Status = TimerDriverSetTimerPeriod (&mTimer, HPET_DEFAULT_TICK_DURATION);
  Status = EfiCreateEventLegacyBootEx (
              TPL_CALLBACK,
              StopHpetBeforeLagecyBoot,
              NULL,
              &gHpetLegacyBootEvent
              );
  ASSERT_EFI_ERROR (Status);

  //
  // Program Timer 1 to pass certain customer's test
  //
  IoWrite8(TIMER_CONTROL_PORT, 0x54);
  IoWrite8(TIMER1_COUNT_PORT, 0x12);
  
  //
  // Add boot script programming
  //
  Value = 0x54;
  S3BootScriptSaveIoWrite (   
    EfiBootScriptWidthUint8,
    (UINTN) (TIMER_CONTROL_PORT),
    1,
    &Value
    );
  Value = 0x12;
  S3BootScriptSaveIoWrite (   
    EfiBootScriptWidthUint8,
    (UINTN) (TIMER1_COUNT_PORT),
    1,
    &Value
    );

  ///
  /// Begin the ACPI timer counter
  ///
  mPreAcpiTick = GetAcpiTick ();

  ///
  /// Install the Timer Architectural Protocol onto a new handle
  ///
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mTimerHandle,
                  &gEfiTimerArchProtocolGuid,
                  &mTimer,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "HpetTimerDriverInitialize() End\n"));

  return Status;
}
