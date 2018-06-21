//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
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


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbSmmCorePlatformHookLib.c
//
// Description: This file contains code for South Bridge Smm Core Platform Hook
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Library/SmmCorePlatformHookLib.h>
#include <Library/IoLib.h>
#include <token.h>
#include <PchAccess.h>
#include <Sb.h>
#include <Guid/Rtc.h>

#define ENABLE_NMI_BEFORE_SMI_EXIT      0x01
#define DISABLE_NMI_BEFORE_SMI_EXIT     0x02

extern VOID CPULib_DisableInterrupt();
extern VOID CPULib_EnableInterrupt();
extern BOOLEAN CPULib_GetInterruptState();

static volatile UINT8   StoreCMOS;
static volatile UINT8   StoreExtCMOS;
static volatile UINT32  StoreCF8;
static volatile UINT32  AltAcc;
static volatile UINT8   RtcRegA;
  
VOID AlternateAccessMode (
  IN BOOLEAN        Control
    )
{
  UINT32            Data32;
  UINT32            AamBit;

  Data32 = MmioRead32(ILB_BASE_ADDRESS + R_PCH_ILB_MC);

  if (Control == TRUE) {
    //
    // Enable Alternate access mode, let port 70h can be readable.
    //
    Data32 |= (UINT32) (B_PCH_ILB_MC_AME);
    AamBit  = (UINT32) (B_PCH_ILB_MC_AME);
  } else {
    //
    // Disable Alternate access mode.
    //
    Data32 &= (UINT32) ~(B_PCH_ILB_MC_AME);
    AamBit  = 0;
  }

  MmioWrite32 (ILB_BASE_ADDRESS + R_PCH_ILB_MC, Data32);

  //
  // Confirm the bit for Alternate access mode has been updated.
  //
  do {
    Data32 = MmioRead32 (ILB_BASE_ADDRESS + R_PCH_ILB_MC) & (UINT32) (B_PCH_ILB_MC_AME);
  } while (Data32 != AamBit);
}

UINT8 ReadPort70h ( VOID )
{
  UINT8  Port70h;

  AlternateAccessMode (TRUE);

  Port70h = IoRead8(RTC_INDEX_REG);

  AlternateAccessMode (FALSE);

  return Port70h;
}

UINT8 ReadCmos(
  IN UINT8 Index
)
{
  // Read bit 7 (NMI setting).
  UINT8           NMI = 0;
  volatile UINT8  Value;
  BOOLEAN         IntState = CPULib_GetInterruptState();
  UINT8           RtcIndexPort;
  UINT8           RtcDataPort;

  CPULib_DisableInterrupt();

  if (Index < 0x80) {
    // Standard CMOS
    RtcIndexPort  = RTC_INDEX_REG;
    RtcDataPort   = RTC_DATA_REG;
  } else {
    // Upper CMOS
    RtcIndexPort  = CMOS_IO_EXT_INDEX;
    RtcDataPort   = CMOS_IO_EXT_DATA;
  }

  Index &= ~RTC_NMI_MASK;

  if (Index < 0x80) {
    // Save current NMI_EN.
    NMI = ReadPort70h()  & RTC_NMI_MASK;
  }

  IoWrite8(RtcIndexPort, Index | NMI);
  Value = IoRead8(RtcDataPort); // Read register.

  if (IntState) CPULib_EnableInterrupt();

  return (UINT8)Value;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPlatformHookBeforeSmmDispatch
//
// Description: 
//  Performs South Bridge specific tasks before invoking registered SMI 
//  handlers.
//
// Input:       
//
// Output:     
//  The South Bridge platform hook completes successfully.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
SbPlatformHookBeforeSmmDispatch (
  VOID
  )
{
  UINT8 SbCmosMiscFlag = 0;

  //TRACE((-1,"SbPlatformHookBeforeSmmDispatch\n"));

  // Store CF8 (PCI index)
  StoreCF8 = IoRead32(0xcf8);

  // Save Alternate access bit.
  AltAcc = MmioRead32(ILB_BASE_ADDRESS + R_PCH_ILB_MC) & (UINT32) (B_PCH_ILB_MC_AME);

  // Save 0x70
  StoreCMOS = ReadPort70h();

  // Save 0x72
  StoreExtCMOS = IoRead8(CMOS_IO_EXT_INDEX);

#if defined CMOS_MANAGER_SUPPORT && CMOS_MANAGER_SUPPORT
  IoWrite8(CMOS_IO_EXT_INDEX, SB_CMOS_MISC_FLAG_REG);
  IoWrite8(CMOS_IO_EXT_DATA, 0);
#endif

  return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbPlatformHookAfterSmmDispatch
//
// Description: 
//  Performs South Bridge specific tasks after invoking registered SMI 
//  handlers.
//
// Input:       
//
// Output:     
//  EFI_SUCCESS   The South Bridge platform hook completes successfully.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
SbPlatformHookAfterSmmDispatch (
  VOID
  )
{
  UINT8		SbCmosMiscFlag = 0;
  UINT32	Data32;

  //TRACE((-1, "SbPlatformHookAfterSmmDispatch\n"));

  do {
    RtcRegA = ReadCmos(RTC_REG_A_INDEX);
  } while (RtcRegA & 0x80);

#if defined CMOS_MANAGER_SUPPORT && CMOS_MANAGER_SUPPORT
  IoWrite8(CMOS_IO_EXT_INDEX, SB_CMOS_MISC_FLAG_REG);
  SbCmosMiscFlag = IoRead8(CMOS_IO_EXT_DATA);

  if (SbCmosMiscFlag & ENABLE_NMI_BEFORE_SMI_EXIT) {
    StoreCMOS &= 0x7F;          // Enable NMI_EN
  }

  if (SbCmosMiscFlag & DISABLE_NMI_BEFORE_SMI_EXIT) {
    StoreCMOS |= 0x80;           // Disable NMI_EN
  }
#endif

  // Restore 0x70
  IoWrite8(CMOS_ADDR_PORT, StoreCMOS);
  // Restore 0x72
  IoWrite8(CMOS_IO_EXT_INDEX, StoreExtCMOS);

  // Restore Alternate access bit.
  Data32 = MmioRead32 (ILB_BASE_ADDRESS + R_PCH_ILB_MC) & (UINT32) ~(B_PCH_ILB_MC_AME);
  MmioWrite32 (ILB_BASE_ADDRESS + R_PCH_ILB_MC, (Data32 | AltAcc));
  do {
  } while ((MmioRead32 (ILB_BASE_ADDRESS + R_PCH_ILB_MC) & (UINT32) (B_PCH_ILB_MC_AME)) != AltAcc);

  IoWrite32(0xcf8, StoreCF8);    // Restore 0xCF8 (PCI index)

  return EFI_SUCCESS;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
