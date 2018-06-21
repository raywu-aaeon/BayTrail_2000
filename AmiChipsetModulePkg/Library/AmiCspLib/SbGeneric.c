//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SbGeneric.c
//
// Description: This file contains south bridge related code that is needed
//              for both PEI & DXE stage.  To avoid code duplication this
//              file is made as a library and linked both in PEI & DXE
//              south bridge FFS.
//
// Notes:       MAKE SURE NO PEI OR DXE SPECIFIC CODE IS NEEDED
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <EFI.h>
#include <token.h>
#include <AmiPeiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#if SB_STALL_PPI_SUPPORT
#include <Library/TimerLib.h>
#endif
#include <PchAccess.h>
//EIP144604 >>
#if defined HPET_PROTOCOL_SUPPORT && HPET_PROTOCOL_SUPPORT == 1
#include <Protocol/SbHpet.h>
#endif
//EIP144604 <<
//EIP176554 >>
#include "Guid/Rtc.h"
#include "Library/AmiChipsetIoLib.h"
//EIP176554 <<

#define MemoryCeilingVariable   L"MemCeil." //EIP169593

//EIP176554 >>
extern VOID CPULib_DisableInterrupt();
extern VOID CPULib_EnableInterrupt();
extern BOOLEAN CPULib_GetInterruptState();

UINT32  gGpe0aEnBackup = 0;
BOOLEAN gSaved = FALSE;
//EIP176554 <<

//EIP167096 >>
//<AMI_PHDR_START>
//--------------------------------------------------------------------------------
// Procedure: GetSpiRegionAddresses
//
// Description: Get the SPI region base, based on the enum type
//
// Input:
//    RegionType              Region type to query for the base address
//    LimitAddress            This address is left shifted by 12 bits to represent
//                            bits 26:12 for the Region 'n' Limit
//    BaseAddress             This address is left shifted by 12 bits to represent
//                            bits 26:12 for the Region 'n' Base
//
// Output:
//    EFI_SUCCESS             Read success
//    EFI_INVALID_PARAMETER   Invalid region type given
//
//--------------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
GetSpiRegionAddresses (
  IN   AMI_PCH_SPI_REGION_TYPE  RegionType,
  OUT  UINT32                   *LimitAddress,
  OUT  UINT32                   *BaseAddress
  )
{
    BOOLEAN               AdjustRange;
    UINTN                 RegionTypeReg = 0;
    UINT32                ReadValue;
    UINT32                RegionRangeBase;

    if (!((RegionType >= AmiUndefinedType) && (RegionType <= AmiTxeType))) {
      return EFI_INVALID_PARAMETER;
    }

    AdjustRange = FALSE;

    if (RegionType == AmiUndefinedType) {
      return EFI_SUCCESS;
    } else if (RegionType == AmiDescriptorType) {
      RegionTypeReg = R_SB_SPI_FREG0_FLASHD;
    } else if (RegionType == AmiBiosType) {
      AdjustRange = TRUE;
      RegionTypeReg = R_SB_SPI_FREG1_BIOS;
    } else if (RegionType == AmiTxeType) {
      RegionTypeReg = R_SB_SPI_FREG2_SEC;
    }

    ReadValue = MmioRead32 (SPI_BASE_ADDRESS + RegionTypeReg);
    RegionRangeBase  = (ReadValue & B_SB_SPI_FREGX_BASE_MASK) << 12;

    if (AdjustRange == TRUE) {
      *LimitAddress += RegionRangeBase;
      *BaseAddress  += RegionRangeBase;
    } else {
      *LimitAddress = ((ReadValue & B_SB_SPI_FREGX_LIMIT_MASK) >> 4) | 0xFFF;
      *BaseAddress  = RegionRangeBase;
    }
    
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: GetTotalFlashRomSize
//
// Description: Get total Flash ROM size by reading SPI Descriptor.
//
// Input: None
//
// Output: UINT32   Flash Rom Size
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetTotalFlashRomSize ( VOID )
{
    UINT32  TotalRomSize = 0;
    UINT32  CtrlData;
    UINT8   NumSpi = 0;

    //
    // Select to Flash Map 0 Register to get the number of flash Component
    //
    CtrlData = MmioRead32((SPI_BASE_ADDRESS + R_SB_SPI_FDOC));
    CtrlData &= (UINT32)(~(B_SB_SPI_FDOC_FDSS_MASK | B_SB_SPI_FDOC_FDSI_MASK));
    CtrlData |= (UINT32)(V_SB_SPI_FDOC_FDSS_FSDM | R_SB_SPI_FDBAR_FLASH_MAP0);
    MmioWrite32((SPI_BASE_ADDRESS + R_SB_SPI_FDOC), CtrlData);

    switch ( MmioRead16 (SPI_BASE_ADDRESS + R_SB_SPI_FDOD) & B_SB_SPI_FDBAR_NC ) {
        case V_SB_SPI_FDBAR_NC_1:
            NumSpi = 1;
            break;
        case V_SB_SPI_FDBAR_NC_2:
            NumSpi = 2;
            break;
        default:
            break;
    }

    if (NumSpi == 0) return TotalRomSize;

    //
    // Select to Flash Components Register to get the Component 1 Density
    //
    CtrlData = MmioRead32((SPI_BASE_ADDRESS + R_SB_SPI_FDOC));
    CtrlData &= (UINT32)(~(B_SB_SPI_FDOC_FDSS_MASK | B_SB_SPI_FDOC_FDSI_MASK));
    CtrlData |= (UINT32)(V_SB_SPI_FDOC_FDSS_COMP | R_SB_SPI_FCBA_FLCOMP);
    MmioWrite32((SPI_BASE_ADDRESS + R_SB_SPI_FDOC), CtrlData);

    //
    //  Get Component 1 Density
    //
    switch ( (UINT8) MmioRead32 (SPI_BASE_ADDRESS + R_SB_SPI_FDOD) & B_SB_SPI_FLCOMP_COMP1_MASK ) {
        case V_SB_SPI_FLCOMP_COMP1_512KB:
            TotalRomSize += (UINT32) (512 << KBShift);
            break;
        case V_SB_SPI_FLCOMP_COMP1_1MB:
            TotalRomSize += (UINT32) (1 << MBShift);
            break;
        case V_SB_SPI_FLCOMP_COMP1_2MB:
            TotalRomSize += (UINT32) (2 << MBShift);
            break;
        case V_SB_SPI_FLCOMP_COMP1_4MB:
            TotalRomSize += (UINT32) (4 << MBShift);
            break;
        case V_SB_SPI_FLCOMP_COMP1_8MB:
            TotalRomSize += (UINT32) (8 << MBShift);
            break;
        case V_SB_SPI_FLCOMP_COMP1_16MB:
            TotalRomSize += (UINT32) (16 << MBShift);
            break;
        default:
            break;
    } // end of switch

    //
    // Get Component 2 Density
    //
    if (NumSpi == 2) {
        switch ( (UINT8) MmioRead32 (SPI_BASE_ADDRESS + R_SB_SPI_FDOD) & B_SB_SPI_FLCOMP_COMP2_MASK ) {
            case V_SB_SPI_FLCOMP_COMP2_512KB:
                TotalRomSize += (UINT32) (512 << KBShift);
                break;
            case V_SB_SPI_FLCOMP_COMP2_1MB:
                TotalRomSize += (UINT32) (1 << MBShift);
                break;
            case V_SB_SPI_FLCOMP_COMP2_2MB:
                TotalRomSize += (UINT32) (2 << MBShift);
                break;
            case V_SB_SPI_FLCOMP_COMP2_4MB:
                TotalRomSize += (UINT32) (4 << MBShift);
                break;
            case V_SB_SPI_FLCOMP_COMP2_8MB:
                TotalRomSize += (UINT32) (8 << MBShift);
                break;
            case V_SB_SPI_FLCOMP_COMP2_16MB:
                TotalRomSize += (UINT32) (16 << MBShift);
                break;
            default:
                break;
        } // end of switch
    }// end of if

    return TotalRomSize;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbFlashProtectedRange
//
// Description: This routine provides H/W read/write-protection of the BIOS
//              region. If chipset supports "SPI Flash Protected Range
//              registers", then program them to protect BIOS region in SPI
//              Flash.
//
// Input:       ProtectedRangeBase      The base address of the protected range.
//              ProtectedRangeLength    The length of the protected range.
//              ReadProtectEnable       Enable read-protection.
//              WriteProtectEnable      Enable write-protection.
//
// Output:      EFI_STATUS
//              EFI_SUCCESS             Set successfully.
//              EFI_OUT_OF_RESOURCES    There is no available register for
//                                      this call.
//              EFI_INVALID_PARAMETER   The parameter of input is invalid
//              EFI_UNSUPPORTED         Chipset or H/W is not supported.
//
// Notes:       Porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbFlashProtectedRange ( VOID )
{
    AMI_SPI_PROTECTED_RANGE_CONIFG  SpiProtectedRange[] = { SPI_PROTECTED_RANGE_0,
                                                            SPI_PROTECTED_RANGE_1,
                                                            SPI_PROTECTED_RANGE_2,
                                                            SPI_PROTECTED_RANGE_3 };
    AMI_PCH_SPI_REGION_TYPE         AmiSpiRegionType;
    BOOLEAN                         WriteProtectionEnable;
    BOOLEAN                         ReadProtectionEnable;
    UINT32                          ProtectedRangeBase;
    UINT32                          ProtectedRangeLimit;
    UINT32                          RegIndex;
    UINT32                          TotalRomSize;
    volatile UINT32                 Value32;

    //
    // Get Total ROM size first
    //
    TotalRomSize = GetTotalFlashRomSize();

    if (TotalRomSize == 0) return EFI_UNSUPPORTED;

    //
    // Program Protected Range Registers per SPI_PROTECTED_RANGE_X tokens
    // Note: PR4 register is RO
    //
    for (RegIndex = 0; RegIndex < 4; RegIndex++) {
      AmiSpiRegionType      = SpiProtectedRange[RegIndex].AmiPchSpiRegionType;
      WriteProtectionEnable = SpiProtectedRange[RegIndex].WriteProtectionEnable;
      ReadProtectionEnable  = SpiProtectedRange[RegIndex].ReadProtectionEnable;
      ProtectedRangeBase    = SpiProtectedRange[RegIndex].ProtectedRangeBase;
      ProtectedRangeLimit   = SpiProtectedRange[RegIndex].ProtectedRangeBase + SpiProtectedRange[RegIndex].ProtectedRangeLength - 1;
      
      if ((WriteProtectionEnable == 0) && (ReadProtectionEnable == 0)) continue;
      
      //
      // Get RangeBase and RangeLimit per SpiRegion
      //
      GetSpiRegionAddresses (AmiSpiRegionType, &ProtectedRangeLimit, &ProtectedRangeBase);
      //
      // Exceed the address of protected range base.
      //
      if (ProtectedRangeBase >= TotalRomSize) continue;
      //
      // Exceed the address of protected range limit.
      //
      if (ProtectedRangeLimit >= TotalRomSize) continue;

      Value32 = *(UINT32 *)(SPI_BASE_ADDRESS + R_SB_SPI_PR0 + (RegIndex * 4));
      if (Value32 == 0) {
        Value32 = (ProtectedRangeBase & 0x1FFF000) >> 12;
        Value32 += ((ProtectedRangeLimit & 0x1FFF000) << 4);
        if (ReadProtectionEnable) Value32 |= B_SB_SPI_PRx_RPE;
        if (WriteProtectionEnable) Value32 |= B_SB_SPI_PRx_WPE;
        *(UINT32 *)(SPI_BASE_ADDRESS + R_SB_SPI_PR0 + (RegIndex * 4)) = Value32;
      } // if register is not use yet
    } // for loop

    return EFI_SUCCESS;
}
//EIP167096 <<

//EIP176554 >>
/**
    Enable or disable Alternate Access Mode.

    @param Switch TRUE: Enable Alternate Access Mode
                  FALSE: Disable Alternate Access Mode

    @retval VOID
**/
VOID SwitchAlternateAccessMode (
    IN BOOLEAN      Switch
)
{
    UINT32          Data32Or;
    UINT32          Data32And;

    Data32Or  = 0;
    Data32And = 0xFFFFFFFF;

    if (Switch == TRUE) {
      ///
      /// Enable Alternate Access Mode
      /// Note: The RTC Index field (including the NMI mask at bit7) is write-only
      /// for normal operation and can only be read in Alt Access Mode.
      ///
      Data32Or  = (UINT32) (B_PCH_ILB_MC_AME);
    }

    if (Switch == FALSE) {
      ///
      /// Disable Alternate Access Mode
      ///
      Data32And = (UINT32) ~(B_PCH_ILB_MC_AME);
    }

    ///
    /// Program Alternate Access Mode Enable bit
    ///
    MmioAndThenOr32 (
      ILB_BASE_ADDRESS + R_PCH_ILB_MC,
      Data32And,
      Data32Or
      );

    ///
    /// Reads back for posted write to take effect
    ///
    MmioRead32(ILB_BASE_ADDRESS + R_PCH_ILB_MC);
}

/**
    Read port 70h.

    @param VOID

    @return Data of port 70h.
**/
UINT8 ReadPort70h (
    VOID
)
{
    UINT8  Port70h;

    SwitchAlternateAccessMode (TRUE);

    Port70h = IoRead8(RTC_INDEX_REG);

    SwitchAlternateAccessMode (FALSE);

    return Port70h;
}

/**
    Enable or disable SB common functions.

    @param SbDevType - The type of SB device, refer to SB_DEV_TYPE
    @param Enable - TRUE: Enable / FALSE: Disable

    @retval VOID

**/
VOID SbEnableDisableFunctions (
    IN SB_DEV_TYPE  SbDevType,
    IN BOOLEAN      Enable
)
{
    UINT32          DisFunc;
    UINT32          DisFunc2;

    if (SbDevType < 64) {
      if (SbDevType < 32) {
        DisFunc   = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS);
        if (Enable) {
          MmioAnd32 (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, (UINT32) ~(1 << SbDevType));
        } else {
          MmioOr32 (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, (UINT32) (1 << SbDevType));
        }
        MmioWrite32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, DisFunc);
      } else {
        SbDevType -= 32;
        DisFunc2  = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS2);
        if (Enable) {
          MmioAnd32 (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS2, (UINT32) ~(1 << SbDevType));
        } else {
          MmioOr32 (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS2, (UINT32) (1 << SbDevType));
        }
        MmioWrite32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS2, DisFunc2); 
      }
    }
}

/**
    Save or Restore All Hardware SMIs

    @param Save TRUE  : Save all HW SMIs
                FALSE : Restore saved HW SMIs

    @retval VOID

**/
VOID SbSaveRestoreAllHwSmi (
    IN BOOLEAN      Save
)
{
    static UINT16   Pm1EnSave;
    static UINT32   SmiEnSave;
    static UINT32   Gpe0aEnSave;
    static UINT16   AltGpiSmiEnSave;
           UINT32   Data32;
           UINT16   PmBase;

    PmBase    = PcdGet16 (PcdAcpiIoPortBaseAddress);
           
    if (Save) {
      Pm1EnSave = IoRead16 (PmBase + R_PCH_ACPI_PM1_EN);
      SmiEnSave = IoRead32 (PmBase + R_PCH_SMI_EN) & ~(B_PCH_SMI_EN_GBL_SMI | B_PCH_SMI_EN_EOS | B_PCH_SMI_EN_APMC);
      Gpe0aEnSave     = IoRead32 (PmBase + R_PCH_ACPI_GPE0a_EN);
      AltGpiSmiEnSave = IoRead16 (PmBase + R_PCH_ALT_GP_SMI_EN);
    } else {
      IoWrite16 (PmBase + R_PCH_ACPI_PM1_EN, Pm1EnSave);
      Data32 = IoRead32 (PmBase + R_PCH_SMI_EN);
      Data32 &= (B_PCH_SMI_EN_GBL_SMI | B_PCH_SMI_EN_EOS | B_PCH_SMI_EN_APMC);
      IoWrite32 (PmBase + R_PCH_SMI_EN, (Data32 | SmiEnSave));
      IoWrite32 (PmBase + R_PCH_ACPI_GPE0a_EN, Gpe0aEnSave);
      IoWrite16 (PmBase + R_PCH_ALT_GP_SMI_EN, AltGpiSmiEnSave);
    }
}

/**
    Disable all HW SMIs

    @param VOID

    @retval VOID

**/
VOID SbDisableAllHwSmi (
    VOID
)
{
    UINT16 PmBase;
    
    PmBase    = PcdGet16 (PcdAcpiIoPortBaseAddress);

    // Clear PM1_EN
    IoAnd16 (PmBase + R_PCH_ACPI_PM1_EN, 0);
    // Clear SMI_EN except B_PCH_SMI_EN_GBL_SMI, B_PCH_SMI_EN_EOS and B_PCH_SMI_EN_APMC.
    IoAnd32 (PmBase + R_PCH_SMI_EN, (B_PCH_SMI_EN_GBL_SMI | B_PCH_SMI_EN_EOS | B_PCH_SMI_EN_APMC));
    // Clear GPE0_EN.
    IoAnd32 (PmBase + R_PCH_ACPI_GPE0a_EN, 0);
    // Clear ALT_GPI_EN.
    IoAnd16 (PmBase + R_PCH_ALT_GP_SMI_EN, 0);
}

/**
    Check NMI bit

    @param VOID

    @retval TRUE  : NMI is enabled.
            FALSE : NMI is disabled.
**/
BOOLEAN SbCheckNmiStatus (
    VOID
)
{
    if (ReadPort70h() & RTC_NMI_MASK) {
      // NMI is disabled.
      return FALSE;
    } else {
      // NMI is Enabled.
      return TRUE;
    }
}

/**
    Enable / Disable NMI Bit

    @param Enable - TRUE  : Enable NMI
                    FALSE : Disable NMI

    @retval TRUE  : NMI is enabled
            FALSE : NMI is disabled
**/
BOOLEAN SbEnableDisableNmi (
    IN BOOLEAN      Enable
)
{
    BOOLEAN IntState = CPULib_GetInterruptState();
    static UINT8   Data8;

    CPULib_DisableInterrupt();
  
    Data8 = IoRead8 (CMOS_IO_STD_INDEX);
  
    if (Enable) {
      Data8 &= ~RTC_NMI_MASK;
    } else {
      Data8 |= RTC_NMI_MASK;
    }
    IoWrite8 (CMOS_ADDR_PORT, Data8);

    if (IntState) CPULib_EnableInterrupt();

    return SbCheckNmiStatus();
}

/**
    Save / Restore NMI bit

    @param Save TRUE  : Save NMI
                FALSE : Restore NMI

    @retval VOID

**/
VOID SbSaveRestoreNmi (
    IN BOOLEAN      Save
)
{
    static  UINT8   NmiMask;
            BOOLEAN IntState = CPULib_GetInterruptState();
            UINT8   Data8;

    CPULib_DisableInterrupt();

    if (Save) {
      // Save current NMI_EN.
      NmiMask = ReadPort70h() & RTC_NMI_MASK;
    } else {
      Data8 = IoRead8 (CMOS_IO_STD_INDEX) & ~(RTC_NMI_MASK);
      IoWrite8 (CMOS_ADDR_PORT, Data8 | NmiMask);    
    }
    if (IntState) CPULib_EnableInterrupt();
}

/**
    Trigger SW SMI and adding extra I/O instructions around
    the SMI port write reduces the failure rate.

    @param SwSmi - The number of Software SMI

    @retval VOID

**/
VOID SbSwSmiTrigger (
    IN UINT8        SwSmi
)
{
    IoWrite8 (0x81, 0xEF);
    IoWrite8 (0x81, 0xEF);
    IoWrite8 (R_PCH_APM_CNT, SwSmi);
    IoWrite8 (0x81, 0xEF);
    IoWrite8 (0x81, 0xEF);
}

/**
    Trigger SW SMI and return the S/W SMI data from the S/W SMI data port.

    @param SwSmi The number of Software SMI
    @param Data Pointer to the data to be written to the S/W SMI data port.

    @return Data Pointer to the data read from the S/W SMI data port.

**/
VOID SbSwSmiIo (
    IN     UINT8    SwSmi,
    IN OUT UINT8    *Data
)
{
    IoWrite8 (R_PCH_APM_STS, *Data);
    SbSwSmiTrigger (SwSmi);
    *Data = IoRead8 (R_PCH_APM_STS);
}

/**
    Get data from S/W SMI I/O port and return the address of S/W SMI 
    I/O port.

    @param DataValue Pointer to the data buffer for receiving the S/W SMI
                     number read from S/W SMI I/O port.

    @return DataValue Pointer to the data buffer contained the S/W SMI
                      number read from S/W SMI I/O port.

**/
UINT16 SbGetSwSmi (
    IN OUT UINT32   *DataValue
)
{
    *DataValue = (UINT32)(IoRead8(R_PCH_APM_CNT));
    return R_PCH_APM_CNT;
}

/**
    Get Intel TCO2 Status address.(Intel Chipset Only)

    @param AcpiBaseAddr ACPI Base address, it is defined by PM_BASE_ADDRESS
                        normally.

    @return The address of TCO2 status register.
            0xFFFF - not supported.

**/
UINT16 SbGetTco2StsAddress (
    IN UINT16       AcpiBaseAddr
)
{
    return 0xFFFF;
}

/**
    Get Intel TCO2 Control address. (Intel Chipset Only)

    @param AcpiBaseAddr ACPI Base address, it is defined by PM_BASE_ADDRESS
                        normally.

    @retval The address of TCO2 control register.
            0xFFFF - not supported.

**/
UINT16 SbGetTco2CntAddress (
    IN UINT16       AcpiBaseAddr
)
{
    return 0xFFFF;
}

/**
    Set After G3 bit.

    @param Set
           TRUE  = Enable G3 Bit
           FALSE = Disable G3 Bit

    @retval VOID

**/
VOID SetAfterG3Bit (
    IN BOOLEAN      Set
)
{
    if (Set) {
      MmioOr32((PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1), (UINT32) B_PCH_PMC_GEN_PMCON_AFTERG3_EN);
    } else {
      MmioAnd32((PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1), (UINT32) ~B_PCH_PMC_GEN_PMCON_AFTERG3_EN);
    }
}

/**
    Check After G3 bit.

    @param VOID

    @retval TRUE:  G3 bit is set
            FALSE: G3 bit is not set

**/
BOOLEAN SbCheckAfterG3 (
    VOID
)
{
    if (MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & (UINT32) B_PCH_PMC_GEN_PMCON_AFTERG3_EN) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Disable LPC Super I/O Devices

    @param SioType - The type of SIO device, refer to SB_LPC_SIO_TYPE.

    @retval VOID

**/
VOID SbDisableLpcDevices (
    IN SB_LPC_SIO_TYPE  SioType
)
{
}

/**
    Enable EHCI USB SMI

    @param VOID

    @retval VOID

**/
VOID SbEnableEhciSmi (
    VOID
)
{
    UINT16 PmBase;

    PmBase    = PcdGet16 (PcdAcpiIoPortBaseAddress);
    IoOr32 (PmBase + R_PCH_SMI_EN, B_PCH_SMI_EN_LEGACY_USB2);
}

/**
    Disable EHCI USB SMI

    @param VOID

    @retval VOID

**/
VOID SbDisableEhciSmi (
    VOID
)
{
    UINT16 PmBase;
    
    PmBase    = PcdGet16 (PcdAcpiIoPortBaseAddress);
    IoAnd32 (PmBase + R_PCH_SMI_EN, ~B_PCH_SMI_EN_LEGACY_USB2);
}

/**
    Check power button pressed.

    @param VOID

    @retval TRUE:  Pressed
            FALSE: No Pressed

**/
BOOLEAN IsPowerButtonPressed (
    VOID
)
{  
    UINT16 PmBase;

    PmBase    = PcdGet16 (PcdAcpiIoPortBaseAddress);

    if (IoRead16 (PmBase + R_PCH_ACPI_PM1_STS) & B_PCH_ACPI_PM1_STS_PWRBTN) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Enable PCI PME bit.

    @param PciAddress The address got from PCI_LIB_ADDRESS macro in
                      PciLib.h
        #define PCI_LIB_ADDRESS(Bus,Device,Function,Register) \
        (((Register) & 0xfff) | (((Function) & 0x07) << 12) | \
        (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))

    @retval VOID

**/
VOID SbEnablePciPme (
    IN UINTN        PciAddress
)
{
    UINT16          CapPtr;
    UINTN           PcieAddress;
  
    PcieAddress = CSP_PCIE_CFG_ADDRESS(
                    (PciAddress >> 20) & 0xFF,
                    (PciAddress >> 15) & 0x1F,
                    (PciAddress >> 12) & 0x07,
                    0);  
  
    CapPtr = FindCapPtr (PcieAddress, 0x01);
    // CapPtr is found
    if (CapPtr != 0) {
      // Set PMEE.
      MmioOr16 (PcieAddress + CapPtr + 4, BIT08);
    }
}

/**
    Disable PCI PME bit.

    @param PciAddress The address got from PCI_LIB_ADDRESS macro in
                      PciLib.h
        #define PCI_LIB_ADDRESS(Bus,Device,Function,Register) \
        (((Register) & 0xfff) | (((Function) & 0x07) << 12) | \
        (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))

    @retval VOID

**/
VOID SbDisablePciPme (
    IN UINTN        PciAddress
)
{
    UINT16          CapPtr;
    UINTN           PcieAddress;
  
    PcieAddress = CSP_PCIE_CFG_ADDRESS(
                    (PciAddress >> 20) & 0xFF,
                    (PciAddress >> 15) & 0x1F,
                    (PciAddress >> 12) & 0x07,
                    0);  
  
    CapPtr = FindCapPtr (PcieAddress, 0x01);
    // CapPtr is found
    if (CapPtr != 0) {
      // Disable PMEE.
      MmioAnd16 (PcieAddress + CapPtr + 4, ~BIT08);
      // Clear PMES
      MmioWrite16 (PcieAddress + CapPtr + 4, BIT15);
    }
}

/**
    Program the related registers for WOL enabled.

    @param VOID

    @retval VOID

**/
VOID SbEnableWolPmConfg (
    VOID
)
{
}

/**
    Program the related registers for WOL disabled.

    @param VOID

    @retval VOID

**/
VOID SbDisableWolPmConfg (
    VOID
)
{
}

/**
    Get I/O Trap Info

    @param TrappedIoData Pointer to a 32bit data buffer for receiving the
                         data of I/O trap information.

    @return TrappedIoData Pointer to a 32bit data buffer contained the
                          data of I/O trap information.
            I/O Trap address

**/
UINT16 SbGetIoTrapInfo (
    IN OUT UINT32   *TrappedIoData 
)
{
    return  0xFFFF;
}

/**
    Get ACPI Base Address

    @param VOID

    @return The address of ACPI Base.

**/
UINT16 SbGetAcpiBaseAddress (
    VOID
)
{
    return PcdGet16 (PcdAcpiIoPortBaseAddress);
}

/**
    Get PM1 Control Register Offset

    @param VOID

    @return The offset of PM1 Control Register

**/
UINT16 SbGetPm1CntOffset (
    VOID
)
{
    return SbGetAcpiBaseAddress() + R_PCH_ACPI_PM1_CNT;
}

/**
    Get Intel RCBB Address (Intel Chipset Only)

    @param VOID

    @return The address of RCRB

**/
UINT32 SbGetRcrbAddress (
    VOID
)
{
    return PcdGet32 (PcdRcrbBaseAddress);
}

/**
    Check RTC power status

    @param VOID

    @retval TRUE:  Valid
            FALSE: Invalid

**/
BOOLEAN SbIsRtcPwrValid (
    VOID
)
{
    if(MmioRead8(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) {
      return FALSE;
    } else {
      return TRUE;
    }
}

/**
    Get the enable bit setting in GPE0 per Gpe0Type

    @param Gpe0Type	SB_GPE0_TYPE, like TYPE_HOT_PLUG .. etc.

    @retval TRUE = Enabled / FALSE = Disabled

**/
BOOLEAN SbGetGpe0En (
    IN SB_GPE0_TYPE Gpe0Type
)
{
    if (Gpe0Type > 15) {
      return FALSE;
    }
    
    if (IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN) & (UINT32) (1 << (Gpe0Type))) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Set the enable bit in GPE0 per Gpe0Type

    @param Gpe0Type	SB_GPE0_TYPE, like TYPE_HOT_PLUG .. etc.

    @retval VOID

**/
VOID SbSetGpe0En (
    IN SB_GPE0_TYPE Gpe0Type
)
{
    if (Gpe0Type > 15)  return;

    IoOr32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, (UINT32) (1 << (Gpe0Type)));
}

/**
    Reset the enable bit in GPE0 per Gpe0Type

    @param Gpe0Type	SB_GPE0_TYPE, like TYPE_HOT_PLUG .. etc.

    @retval VOID

**/
VOID SbResetGpe0En (
    IN SB_GPE0_TYPE Gpe0Type
)
{
    if (Gpe0Type > 15)  return;
    
    IoAnd32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, (UINT32) ~(1 << (Gpe0Type)));
}

/**
    Save/Restore/ClearAll GPE0 register per Operation

    @param Operation	GPE0_EN_OP_SAVE, GPE0_EN_OP_RESTORE,
                      GPE0_EN_OP_CLEAR_ALL.

    @retval VOID

**/
VOID SbGpe0Operation (
    IN GPE0_EN_OP   Operation
)
{
    UINT32  PchGpe0aEnMask;

    PchGpe0aEnMask = (UINT32) (B_PCH_ACPI_GPE0a_EN_CORE_GPIO | B_PCH_ACPI_GPE0a_EN_SUS_GPIO);
    
    switch (Operation) {
      case GPE0_EN_OP_SAVE:
        if (!gSaved) {
          gGpe0aEnBackup = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN) & ~PchGpe0aEnMask;
          gSaved = TRUE;
        }
        break;
      case GPE0_EN_OP_RESTORE:
        if (gSaved) {
          IoAndThenOr32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, PchGpe0aEnMask, gGpe0aEnBackup);
          gSaved = FALSE;
        }
        break;
      case GPE0_EN_OP_CLEAR_ALL:
        IoAnd32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, PchGpe0aEnMask);
        break;
      default:
        break;
    }
}

/**
    Get the status bit setting in GPE0 per GPE0 Type

    @param Gpe0Type	SB_GPE0_TYPE, like TYPE_HOT_PLUG .. etc.

    @retval TRUE = Set / FALSE= Clear

**/
BOOLEAN SbGetGpe0Sts (
    IN SB_GPE0_TYPE Gpe0Type
)
{
    if (Gpe0Type > 15) {
      return FALSE;
    }
    
    if (IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS) & (UINT32) (1 << (Gpe0Type))) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Clear the status bit in GPE0 per GPE0 Type

    @param Gpe0Type	SB_GPE0_TYPE, like TYPE_HOT_PLUG .. etc.

    @retval VOID 

**/
VOID SbClearGpe0Sts (
    IN SB_GPE0_TYPE Gpe0Type
)
{
    if (Gpe0Type > 15)  return;
    
    IoWrite32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS, (UINT32) (1 << (Gpe0Type)));
}

/**
    Set GPI Pin Enable bit in Gpe0

    @param Gpio - Define the group and Pin# of the GPIO

    @retval VOID

**/
VOID SbSetGpe0GpinEn (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT8   GpioOffset;

    if (Gpio.PinNum > 7) return;

    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 24;
        break;
      case GPIO_SUS:
        GpioOffset = 16;
        break;
      default: 
        return;
    }

    IoOr32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, (UINT32) (1 << (Gpio.PinNum + GpioOffset)));
}

/**
    Get GPI Pin Enable bit setting in Gpe0

    @param Gpio - Define the group and Pin# of the GPIO

    @retval TRUE = Enabled / FALSE= Disabled

**/
BOOLEAN SbGetGpe0GpinEn (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT8   GpioOffset;
    
    if (Gpio.PinNum > 7) return FALSE;
    
    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 24;
        break;
      case GPIO_SUS:
        GpioOffset = 16;
        break;
      default: 
        return FALSE;
    }
    
    if (IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN) & (UINT32) (1 << (Gpio.PinNum + GpioOffset))) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Reset GPI Pin Enable bit in Gpe0

    @param Gpio - Define the group and Pin# of the GPIO

    @retval VOID

**/
VOID SbResetGpe0GpinEn (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT8   GpioOffset;
  
    if (Gpio.PinNum > 7) return;
  
    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 24;
        break;
      case GPIO_SUS:
        GpioOffset = 16;
        break;
      default: 
        return;
    }

    IoAnd32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, (UINT32) ~(1 << (Gpio.PinNum + GpioOffset)));
}

/**
    Get GPI Pin Status bit setting in Gpe0

    @param PinNum - GPIO Pin# (0-7: SUS[0]-SUS[7], 8-15: SC[0]-SC[7])

    @retval VOID

**/
BOOLEAN SbGetGpe0GpinSts (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT8   GpioOffset;
    
    if (Gpio.PinNum > 7) return FALSE;
    
    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 24;
        break;
      case GPIO_SUS:
        GpioOffset = 16;
        break;
      default: 
        return FALSE;
    }
  
    if (IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS) & (UINT32) (1 << (Gpio.PinNum + GpioOffset))) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Clear GPI Pin Status bit in Gpe0

    @param Gpio - Define the group and Pin# of the GPIO

    @retval VOID

**/
VOID SbClearGpe0GpinSts (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT8   GpioOffset;
  
    if (Gpio.PinNum > 7) return;
  
    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 24;
        break;
      case GPIO_SUS:
        GpioOffset = 16;
        break;
      default: 
        return;
    }

    IoWrite32 (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS, (UINT32) (1 << (Gpio.PinNum + GpioOffset)));
}

/**
    Get GPIO Use Setting

    @param Gpio - Define the group and Pin# of the GPIO

    @retval TRUE = GPIO MODE / FALSE= NATIVE MODE

**/
BOOLEAN SbGetGpioUseSel (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT16  GpioBase;
    UINTN   UseRegister;
    UINT32  BitNum;
    UINT32  Buffer32;
    UINT32  Index;
    
    GpioBase = PcdGet16 (PcdGpioPortBaseAddress);

    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return FALSE;
        Index       = Gpio.PinNum/32;
        UseRegister = R_PCH_GPIO_SC_USE_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return FALSE;
        Index       = Gpio.PinNum/32;
        UseRegister = R_PCH_GPIO_SUS_USE_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      default: 
        return FALSE;
    }

    Buffer32 = IoRead32 (GpioBase + UseRegister);
    
    if (Buffer32 & (UINT32) (1 << BitNum)) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Program GPIO Use Setting

    @param Gpio - Define the group and Pin# of the GPIO
    @param GpioMode	- TRUE = GPIO MODE / FALSE= NATIVE MODE
    @param MulFunc - While this pin is a multi-function pin and GpioMode
                     is Native Mode, BIOS will refer to MulFunc to
                     determine which native function this pin will be
                     used.

    @retval VOID

**/
VOID SbProgGpioUseSel (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      GpioMode,
    IN UINT8        MulFunc
)
{
    UINT16  GpioBase;
    UINTN   UseRegister;
    UINT32  BitNum;
    UINT32  Buffer32;
    UINT32  Index;
    
    GpioBase = PcdGet16 (PcdGpioPortBaseAddress);
    
    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return;
        Index       = Gpio.PinNum/32;
        UseRegister = R_PCH_GPIO_SC_USE_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return;
        Index       = Gpio.PinNum/32;
        UseRegister = R_PCH_GPIO_SUS_USE_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      default: 
        return;
    }

    Buffer32 = IoRead32 (GpioBase + UseRegister);
    if (GpioMode) {
      Buffer32 |= (UINT32) (1 << BitNum);
    } else {
      Buffer32 &= (UINT32) ~(1 << BitNum);
    }
    IoWrite32 (GpioBase + UseRegister, Buffer32);
}

/**
    Get GPIO I/O Setting

    @param Gpio - Define the group and Pin# of the GPIO

    @retval TRUE : Input Mode / FALSE : Output Mode

**/
BOOLEAN SbGetGpioIoSel (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT16  GpioBase;
    UINTN   IoRegister;
    UINT32  BitNum;
    UINT32  Buffer32;
    UINT32  Index;
    
    GpioBase = PcdGet16 (PcdGpioPortBaseAddress);

    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return FALSE;
        Index       = Gpio.PinNum/32;
        IoRegister  = R_PCH_GPIO_SC_IO_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return FALSE;
        Index       = Gpio.PinNum/32;
        IoRegister  = R_PCH_GPIO_SUS_IO_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      default: 
        return FALSE;
    }
    
    Buffer32 = IoRead32 (GpioBase + IoRegister);
    
    if (Buffer32 & (UINT32) (1 << BitNum)) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Set GPIO I/O Setting

    @param Gpio - Define the group and Pin# of the GPIO
    @param InputMode - TRUE : Input Mode / FALSE : Output Mode

    @retval VOID

**/
VOID SbSetGpioIoSel (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      InputMode
)
{
    UINT16  GpioBase;
    UINTN   IoRegister;
    UINT32  BitNum;
    UINT32  Buffer32;
    UINT32  Index;
    
    GpioBase = PcdGet16 (PcdGpioPortBaseAddress);

    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return;
        Index       = Gpio.PinNum/32;
        IoRegister  = R_PCH_GPIO_SC_IO_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return;
        Index       = Gpio.PinNum/32;
        IoRegister  = R_PCH_GPIO_SUS_IO_SEL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      default: 
        return;
    }

    Buffer32 = IoRead32 (GpioBase + IoRegister);
    if (InputMode) {
      Buffer32 |= (UINT32) (1 << BitNum);
    } else {
      Buffer32 &= (UINT32) ~(1 << BitNum);
    }
    IoWrite32 (GpioBase + IoRegister, Buffer32);
}

/**
    Get GPIO Level Setting

    @param Gpio - Define the group and Pin# of the GPIO

    @retval TRUE: High level / FALSE: Low level

**/
BOOLEAN SbGetGpioLvlSel (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT16  GpioBase;
    UINTN   LvlRegister;
    UINT32  BitNum;
    UINT32  Buffer32;
    UINT32  Index;
    
    GpioBase = PcdGet16 (PcdGpioPortBaseAddress);

    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return FALSE;
        Index       = Gpio.PinNum/32;
        LvlRegister = R_PCH_GPIO_SC_LVL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return FALSE;
        Index       = Gpio.PinNum/32;
        LvlRegister = R_PCH_GPIO_SUS_LVL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      default: 
        return FALSE;
    }

    Buffer32 = IoRead32 (GpioBase + LvlRegister);
    
    if (Buffer32 & (UINT32) (1 << BitNum)) {
      return TRUE;
    } else {
      return FALSE;
    }
}

/**
    Set GPIO Level

    @param Gpio - Define the group and Pin# of the GPIO
    @param High - TRUE: Set to High level / FALSE: Set to Low level

    @retval VOID

**/
VOID SbSetGpioLvlSel (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      High
)
{
    UINT16  GpioBase;
    UINTN   LvlRegister;
    UINT32  BitNum;
    UINT32  Buffer32;
    UINT32  Index;
    
    GpioBase = PcdGet16 (PcdGpioPortBaseAddress);

    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return;
        Index       = Gpio.PinNum/32;
        LvlRegister = R_PCH_GPIO_SC_LVL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return;
        Index       = Gpio.PinNum/32;
        LvlRegister = R_PCH_GPIO_SUS_LVL + (Index * 0x20);
        BitNum      = Gpio.PinNum - (Index * 32);
        break;
      default: 
        return;
    }
    
    Buffer32 = IoRead32 (GpioBase + LvlRegister);
    if (High) {
      Buffer32 |= (UINT32) (1 << BitNum);
    } else {
      Buffer32 &= (UINT32) ~(1 << BitNum);
    }
    IoWrite32 (GpioBase + LvlRegister, Buffer32);
}

/**
    Read Alternate GPI SMI Status Register

    @param VOID

    @return The value read from Alternate GPI SMI Status Register

**/
UINT32 SbReadAltGpiSmiSts (
    VOID
)
{
    UINT16  PmBase;
    
    PmBase = PcdGet16 (PcdAcpiIoPortBaseAddress);
    return IoRead16 (PmBase + R_PCH_ALT_GP_SMI_STS);
}

/**
    Clear Alternate GPI SMI Status

    @param Gpio - Define the group and Pin# of the GPIO

    @retval VOID

**/
VOID SbClearAltGpiSmiSts (
    IN AMI_OEM_GPIO Gpio
)
{
    UINT16  PmBase;
    UINT8   GpioOffset;
  
    if (Gpio.PinNum > 7) return;
  
    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 8;
        break;
      case GPIO_SUS:
        GpioOffset = 0;
        break;
      default: 
        return;
    }

    PmBase = PcdGet16 (PcdAcpiIoPortBaseAddress);
    IoWrite16 (PmBase + R_PCH_ALT_GP_SMI_STS, (1 << (Gpio.PinNum + GpioOffset)));
}

/**
    Program Alternate GPI SMI Register

    @param Gpio - Define the group and Pin# of the GPIO
    @param Set - TRUE: Set / FALSE: Clear corresponding Alternate GPI SMI
                 Enable bit.

    @retval VOID

**/
VOID SbProgramAltGpiSmi (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      Set
)
{
    UINT16  PmBase;
    UINT16  Buffer16;
    UINT8   GpioOffset;

    if (Gpio.PinNum > 7) return;

    PmBase    = PcdGet16 (PcdAcpiIoPortBaseAddress);
    Buffer16  = IoRead16 (PmBase + R_PCH_ALT_GP_SMI_EN);

    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 8;
        break;
      case GPIO_SUS:
        GpioOffset = 0;
        break;
      default: 
        return;
    }

    if (Set) {
      Buffer16 |= (1 << (Gpio.PinNum + GpioOffset));
    } else {
      Buffer16 &= ~(1 << (Gpio.PinNum + GpioOffset));
    }

    IoWrite16 (PmBase + R_PCH_ALT_GP_SMI_EN, Buffer16);
}

/**
    Program GPIO Rout

    @param Gpio - Define the group and Pin# of the GPIO
    @param Mode - NO_EFFECT/SMI_MODE/SCI_MODE/NMI_MODE

    @retval VOID

**/
VOID SbProgramGpioRout (
    IN AMI_OEM_GPIO Gpio,
    IN UINT8        Mode
)
{
    UINT32  GpioRout;
    UINT8   GpioOffset;

    if (Gpio.PinNum > 7) return;

    switch (Gpio.Group) {
      case GPIO_SC:
        GpioOffset = 8;
        break;
      case GPIO_SUS:
        GpioOffset = 0;
        break;
      default: 
        return;
    }

    GpioRout = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT);
    GpioRout &= ~(0x03 << ((Gpio.PinNum + GpioOffset) * 2));
  
    if (Mode == 0) // No effect
      GpioRout |= (0x00 << ((Gpio.PinNum + GpioOffset) * 2));
    if (Mode == 1) // SMI#
      GpioRout |= (0x01 << ((Gpio.PinNum + GpioOffset) * 2));
    if (Mode == 2) // SCI#
      GpioRout |= (0x02 << ((Gpio.PinNum + GpioOffset) * 2));
    if (Mode == 3) // NMI
      GpioRout |= (0x03 << ((Gpio.PinNum + GpioOffset) * 2));

    MmioWrite32(PMC_BASE_ADDRESS + R_PCH_PMC_GPI_ROUT , GpioRout);
}

/**
    Program GPIO Register

    @param Gpio - Define the group and Pin# of the GPIO
    @param RegType - The register type which is going to read
    @param AndData - The value to AND with read value from the GPIO register.
    @param OrData - The value to OR with the result of the AND operation.

    @retval VOID

**/
VOID SbProgramGpioRegister (
    IN AMI_OEM_GPIO   Gpio,
    IN GPIO_REG_TYPE  RegType,
    IN UINT32         AndData,
    IN UINT32         OrData
)
{
    UINTN       IoBaseAddress;

    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return;
        IoBaseAddress = IO_BASE_ADDRESS + (Sc_Gpio_Offset_Xlat_Tbl[Gpio.PinNum] * 0x10) + (RegType * 4);
        break;
      case GPIO_NC:
        if (Gpio.PinNum > 26) return;
        IoBaseAddress = IO_BASE_ADDRESS + 0x1000 + (Nc_Gpio_Offset_Xlat_Tbl[Gpio.PinNum] * 0x10) + (RegType * 4);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return;
        IoBaseAddress = IO_BASE_ADDRESS + 0x2000 + (Sus_Gpio_Offset_Xlat_Tbl[Gpio.PinNum] * 0x10) + (RegType * 4);
        break;
      default: 
        return;
    }

    MmioAndThenOr32(IoBaseAddress, AndData, OrData);
}

/**
    Read GPIO Register

    @param Gpio - Define the group and Pin# of the GPIO
    @param RegType - The register type which is going to read
    @param AndData - The value to AND with read value from the GPIO register.
    @param OrData - The value to OR with the result of the AND operation.

    @retval 0xFFFFFFFF - Failed to read the register
    @retval Others - The data read from the register

**/
UINT32 SbReadGpioRegister (
    IN AMI_OEM_GPIO   Gpio,
    IN GPIO_REG_TYPE  RegType
)
{
    UINTN       IoBaseAddress;
  
    switch (Gpio.Group) {
      case GPIO_SC:
        if (Gpio.PinNum > 101) return 0xFFFFFFFF;
        IoBaseAddress = IO_BASE_ADDRESS + (Sc_Gpio_Offset_Xlat_Tbl[Gpio.PinNum] * 0x10) + (RegType * 4);
        break;
      case GPIO_NC:
        if (Gpio.PinNum > 26) return 0xFFFFFFFF;
        IoBaseAddress = IO_BASE_ADDRESS + 0x1000 + (Nc_Gpio_Offset_Xlat_Tbl[Gpio.PinNum] * 0x10) + (RegType * 4);
        break;
      case GPIO_SUS:
        if (Gpio.PinNum > 43) return 0xFFFFFFFF;
        IoBaseAddress = IO_BASE_ADDRESS + 0x2000 + (Sus_Gpio_Offset_Xlat_Tbl[Gpio.PinNum] * 0x10) + (RegType * 4);
        break;
      default: 
        return 0xFFFFFFFF;
    }

    return MmioRead32(IoBaseAddress);
}

/**
    Get South Bridge Miscellaneous BIT Status

    @param SbMiscType   Please check SB_MISC_TYPE for the details
    @param BitStatus    The value of the Miscellaneous BIT

    @retval EFI_UNSUPPORTED   This Miscellaneous BIT is not supported
    @retval EFI_SUCCESS       Success to get the value of the Miscellaneous BIT

**/
EFI_STATUS SbGetMiscBitStatus(
    IN SB_MISC_TYPE         SbMiscType,
    IN OUT UINT8            *BitStatus
)
{
    UINT32      Data32;

    Data32 = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);
    
    switch (SbMiscType) {
      case CPU_THERMAL_TRIP_STATUS:
        if (Data32 & (UINT32) B_PCH_PMC_GEN_PMCON_CTS) {
          *BitStatus = 1;
        } else {
          *BitStatus = 0;
        }
        return EFI_SUCCESS;
      case AFTERG3_EN:
        if (Data32 & (UINT32) B_PCH_PMC_GEN_PMCON_AFTERG3_EN) {
          *BitStatus = 1;
        } else {
          *BitStatus = 0;
        }
        return EFI_SUCCESS;
      case PWR_FLR:
        if (Data32 & (UINT32) B_PCH_PMC_GEN_PMCON_PWROK_FLR) {
          *BitStatus = 1;
        } else {
          *BitStatus = 0;
        }
        return EFI_SUCCESS;
      default: 
        break;
    }

    return EFI_UNSUPPORTED;
}

/**
    Program South Bridge Miscellaneous BIT

    @param SbMiscType   Please check SB_MISC_TYPE for the details
    @param Set          True: write 1 to the bit ; FALSE: write 0 to the bit

    @retval EFI_UNSUPPORTED   This Miscellaneous BIT is not supported
    @retval EFI_SUCCESS       Success to program the value of the Miscellaneous BIT

**/
EFI_STATUS SbProgramMiscBit(
    IN SB_MISC_TYPE         SbMiscType,
    IN BOOLEAN              Set
)
{
    UINT32      Data32;

    // B_PCH_PMC_GEN_PMCON_UART_EN               BIT24 // UART Debug Port Enable
    // B_PCH_PMC_GEN_PMCON_DRAM_INIT             BIT23 // DRAM Initialization Scratchpad Bit
    // B_PCH_PMC_GEN_PMCON_MEM_SR                BIT21 // Memory Placed in Self-Refresh
    // B_PCH_PMC_GEN_PMCON_SRS                   BIT20 // System Reset Status
    // B_PCH_PMC_GEN_PMCON_CTS                   BIT19 // CPU Thermal Trip Status
    // B_PCH_PMC_GEN_PMCON_MIN_SLP_S4            BIT18 // Minimum SLP_S4# Assertion Width Violation Status
    // B_PCH_PMC_GEN_PMCON_PWROK_FLR             BIT16 // PWROK Failure
    // B_PCH_PMC_GEN_PMCON_PME_B0_S5_DIS         BIT15 // PME B0 S5 Disable
    // B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR           BIT14 // SUS Well Power Failure
    // B_PCH_PMC_GEN_PMCON_WOL_ENABLE_OVERRIDE   BIT13 // WOL Enable Override
    // B_PCH_PMC_GEN_PMCON_DISABLE_SX_STRETCH    BIT12 // Disable SLP_X Scretching After SUS Well Power Up
    // B_PCH_PMC_GEN_PMCON_SLP_S3_MAW            (BIT11 | BIT10) // SLP_S3# Minimum Assertion Width
    // B_PCH_PMC_GEN_PMCON_GEN_RST_STS           BIT9  // General Reset Status
    // B_PCH_PMC_GEN_PMCON_RTC_RESERVED          BIT8  // RTC Reserved
    // B_PCH_PMC_GEN_PMCON_SWSMI_RTSL            (BIT7 | BIT6)  // SWSMI Rate Select
    // B_PCH_PMC_GEN_PMCON_SLP_S4_MAW            (BIT5 | BIT4) // SLP_S4# Minimum Assertion Width
    // B_PCH_PMC_GEN_PMCON_SLP_S4_ASE            BIT3  // SLP_S4# Assertion Scretch Enable
    // B_PCH_PMC_GEN_PMCON_RTC_PWR_STS           BIT2  // RTC Power Status (Write 0 to clear)
    // B_PCH_PMC_GEN_PMCON_AFTERG3_EN            BIT0  // After G3 State Enable
    Data32 = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & 0xFFE2BDFF;

    switch (SbMiscType) {
      case CPU_THERMAL_TRIP_STATUS:
        if (Set) {
          Data32 |= (UINT32) B_PCH_PMC_GEN_PMCON_CTS;
        }
        break;
      case AFTERG3_EN:
        if (Set) {
          Data32 |= (UINT32) B_PCH_PMC_GEN_PMCON_AFTERG3_EN;
        } else {
          Data32 &= (UINT32) ~B_PCH_PMC_GEN_PMCON_AFTERG3_EN;
        }
        break;
      case PWR_FLR:
        if (Set) {
          Data32 |= (UINT32) B_PCH_PMC_GEN_PMCON_PWROK_FLR;
        }
        break;
      default:
        return EFI_UNSUPPORTED;
    }

    MmioWrite32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, Data32);
    return EFI_SUCCESS;
}

/**
    Check if any USB port is routed to XHCI controller

    @param VOID

    @return TRUE  There is
    @return FALSE There is not

**/
BOOLEAN SbIsXhciRouting ( 
    VOID 
)
{
    UINTN           XhciPciMmBase;
    
    XhciPciMmBase   = CSP_PCIE_CFG_ADDRESS (
                        DEFAULT_PCI_BUS_NUMBER_PCH,
                        PCI_DEVICE_NUMBER_PCH_XHCI,
                        PCI_FUNCTION_NUMBER_PCH_XHCI,
                        0
                        );
    
    if (MmioRead16 (XhciPciMmBase) == 0xFFFF) {
      return FALSE;
    }
    
    if ((MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB2PR) == 0) &&
        (MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB3PR) == 0)) {
      return FALSE;
    }

    return  TRUE;
}

/**
    Get the Mac Address of the LAN inside the chipset

    @param  MacAddress        Pointer to the buffer used to store the Mac Address

    @retval EFI_UNSUPPORTED   This function is not supported
    @retval EFI_SUCCESS       Success to get the Mac Address

**/
EFI_STATUS SbGetChipLanMacAddress ( 
    IN OUT UINT8            *MacAddress
)
{
    return EFI_UNSUPPORTED;
}
//EIP176554 <<

//EIP160150 >>
//EIP164801(-) #if FtRecovery_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsTopSwapOn
//
// Description: This function checks if TopSwap (A16, A17, A18 address lines
//              inversion) is on
//              
// Input:       None
//
// Output:      TRUE - TopSwap is ON
//              FALSE - TopSwap is OFF
//
// Notes:       Intel Chipsets are porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsTopSwapOn (
    VOID
)
{
    volatile UINT32* Address32;

    Address32 = (UINT32 *)(SB_RCBA + R_PCH_RCRB_GCS);

    return ((*Address32) & (UINT32) B_PCH_RCRB_GCS_TS) ? TRUE : FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SetTopSwap
//
// Description: This function sets TopSwap (A16, A17, A18 address lines 
//              inversion) to ON or OFF.
//
// Input:       BOOLEAN On - if TRUE, set TopSwap to ON, if FALSE - set to OFF
//
// Output:      None
//
// Notes:       Intel Chipsets are porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID  SetTopSwap (
    IN BOOLEAN      On
)
{
    volatile UINT32* Address32;

    Address32 = (UINT32 *)(SB_RCBA + R_PCH_RCRB_GCS);

    if (On) {
      *Address32 |= (UINT32) B_PCH_RCRB_GCS_TS;
    } else {
      *Address32 &= (UINT32) ~B_PCH_RCRB_GCS_TS;
    }
}
//EIP164801(-) #endif
//EIP160150 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------------------
// Procedure:   SbLibShutdown
//
// Description: This function Shuts the system down (S5)
//
// Input:       VOID
//
// Output:      VOID
//
//-----------------------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SbLibShutdown (
    VOID
)
{
    UINT16          Value;

    //Check if Sleep SMI is enabled we will disable it.
    Value = IoRead16(PM_BASE_ADDRESS+0x30);
    Value&=(~BIT04);
    IoWrite16(PM_BASE_ADDRESS+0x30,Value);

    //Clear All PM  Statuses
    Value = IoRead16(PM_BASE_ADDRESS);
    IoWrite16(PM_BASE_ADDRESS,Value);

    //Go to S5
    Value = IoRead16(PM_BASE_ADDRESS + 4);
    IoWrite16(PM_BASE_ADDRESS + 4,(UINT16)(Value | (0xf << 10)));
}

//CSP20131223 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------------------
// Procedure:   SbLib_Shutdown
//
// Description: This function Shuts the system down (S5)
//
// Input:       VOID
//
// Output:      VOID
//
//-----------------------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SbLib_Shutdown (
    VOID
)
{
    SbLibShutdown ();
}
//CSP20131223 <<

#if SB_RESET_PPI_SUPPORT

//(EIP127229+)>>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SBLib_ResetSystem
//
// Description: This function is the reset call interface function published
//              by the reset PPI
//
// Input:       IN  EFI_RESET_TYPE ResetType - Type of reset to be generated
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SBLib_ResetSystem (
    IN  EFI_RESET_TYPE  ResetType
)
{
  UINT8          InitialData;
  UINT8          OutputData;
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  UINT8          *CapsuleDataPtr;
  UINTN          CapsuleData;
#endif
  UINT16         Data16;
  UINT32         Data32;

  switch (ResetType) {
  case EfiResetWarm:
    InitialData = V_PCH_RST_CNT_HARDSTARTSTATE;
    OutputData  = V_PCH_RST_CNT_HARDRESET;
    break;
    //
    // For update resets, the reset data is a null-terminated string followed
    // by a VOID * to the capsule descriptors. Get the pointer and set the
    // capsule variable before we do a warm reset. Per the EFI 1.10 spec, the
    // reset data is only valid if ResetStatus != EFI_SUCCESS.
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
    // Then, GPE0_EN should be disabled to avoid any GPI waking up the system from S5
    Data16 = 0;

    IoWrite16 ((UINTN) (PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN), Data16);
    // Clear Sleep SMI Status
    IoWrite16 (PM_BASE_ADDRESS + R_PCH_SMI_STS,
                  (UINT16)(IoRead16 (PM_BASE_ADDRESS + R_PCH_SMI_STS) | B_PCH_SMI_STS_ON_SLP_EN));

    //
    // Clear Sleep Type Enable
    //
//EIP188072 >>
    IoWrite16 (PM_BASE_ADDRESS + R_PCH_SMI_EN,
                  (UINT16)(IoRead16 (PM_BASE_ADDRESS + R_PCH_SMI_EN) & (~B_PCH_SMI_EN_ON_SLP_EN)));
//EIP188072 <<
    
    // Clear Power Button Status
    IoWrite16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_STS, B_PCH_ACPI_PM1_STS_PWRBTN);

    // Secondly, Power Button Status bit must be cleared
    // Write a "1" to bit[8] of power button status register at
    // (ABASE + PM1_STS) to clear this bit
    // Clear it through SMI Status register
    Data16 = B_PCH_SMI_STS_PM1_STS_REG;
    IoWrite16 ((UINTN) (PM_BASE_ADDRESS + R_PCH_SMI_STS), Data16);

    // Finally, transform system into S5 sleep state
    Data32  = IoRead32 ((UINTN) (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT));

    Data32  = (UINT32) ((Data32 &~(B_PCH_ACPI_PM1_CNT_SLP_TYP + B_PCH_ACPI_PM1_CNT_SLP_EN)) | V_PCH_ACPI_PM1_CNT_S5);

    IoWrite32 ((UINTN) (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT), Data32);

    Data32 = Data32 | B_PCH_ACPI_PM1_CNT_SLP_EN;

    IoWrite32 ((UINTN) (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT), Data32);
    //
    // Should not return
    //
    EFI_DEADLOOP();  
    
    return ;

  default:
    return ;
  }
  
  IoWrite8 (
    (UINTN) R_PCH_RST_CNT,
    (UINT8) InitialData
    );

  IoWrite8 (
    (UINTN) R_PCH_RST_CNT,
    (UINT8) OutputData
    );

  // Given we should have reset getting here would be bad
  EFI_DEADLOOP()
}
//(EIP127229+)<<
#endif

//EIP188072 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SBLib_ExtResetSystem
//
// Description: This function is the extended reset call interface function
//              provided by SB.
//
// Input:       ResetType - The extended type of reset to be generated
//
// Output:      SYSTEM RESET
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SBLib_ExtResetSystem (
  IN SB_EXT_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL  
)
{
  UINT8          InitialData, OutputData;
  
  switch (ResetType) {
    case SbResetFull:
    case SbResetGlobal:
    case SbResetGlobalWithEc:
      if ((ResetType == SbResetGlobal) || (ResetType == SbResetGlobalWithEc)) {
        MmioOr32 ( PMC_BASE_ADDRESS+ R_PCH_PMC_PMIR, (UINT32) (B_PCH_PMC_PMIR_CF9GR) );
      }
	    
      InitialData = V_PCH_RST_CNT_HARDSTARTSTATE;
      OutputData  = V_PCH_RST_CNT_HARDRESET;
      break;

    default:
      return;
  }

  IoWrite8 (
    (UINTN) R_PCH_RST_CNT,
    (UINT8) InitialData
    );

  IoWrite8 (
    (UINTN) R_PCH_RST_CNT,
    (UINT8) OutputData
    );

  // Given we should have reset getting here would be bad
  EFI_DEADLOOP()
}
//EIP188072 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbIsDefaultConfigMode
//
// Description: This function determines if the system should boot with the
//              default configuration. 
//
// Input:       EFI_PEI_SERVICES - Pointer to the PEI services table
//              EFI_PEI_READ_ONLY_VARIABLE2_PPI - Pointer to the Read 
//                                                Variable#2 PPI
//              (The pointer can be used to read and enumerate existing NVRAM
//               variables)
//
// Output:      TRUE - Firmware will boot with default configuration.
//
// Notes:       1. If boot with default configuration is detected, default
//                 values for NVRAM variables are used.
//              2. Normally we have to check RTC power status or CMOS clear
//                 jumper status to determine whether the system should boot
//                 with the default configuration.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN SbIsDefaultConfigMode (
    IN EFI_PEI_SERVICES                 **PeiServices,
    IN EFI_PEI_READ_ONLY_VARIABLE2_PPI  *ReadVariablePpi )
{
//EIP169593 >>
    UINTN             VariableSize;
    EFI_STATUS        Status;
    UINT32            MemoryCeiling;
    EFI_GUID          AmiGlobalVariableGuid = AMI_GLOBAL_VARIABLE_GUID; 
  
    VariableSize = sizeof(MemoryCeiling);
    Status = ReadVariablePpi->GetVariable (
                                ReadVariablePpi,
                                MemoryCeilingVariable,
                                &AmiGlobalVariableGuid,
                                NULL,
                                &VariableSize,
                                &MemoryCeiling
                                );
    if(EFI_ERROR(Status)) {
      return FALSE;
    } else {
#if IVI_PF_ENABLE
        return FALSE;
#else
        return (MmioRead8 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) ? TRUE : FALSE; //EIP139414
#endif
    }
//EIP169593 <<
}

#if SB_STALL_PPI_SUPPORT

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CountTime
//
// Description: This function delays for the number of micro seconds passed in
//
// Input:       IN  UINTN DelayTime - Number of microseconds(us) to delay
//              IN  UINT16 BaseAddr - The I/O base address of the ACPI registers
//
// Output:      EFI_STATUS based on errors that occurred while waiting for
//              time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CountTime (
    IN  UINTN   DelayTime,
    IN  UINT16  BaseAddr // only needs to be 16 bit for I/O address
)
{
  MicroSecondDelay (DelayTime);
  return EFI_SUCCESS;
}

#endif


#if CMOS_MANAGER_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// 
// Procedure:   ReadWriteCmosBank2
//
// Description: This function is used to access addresses in the CMOS
//              register range (0x80-0xff), for PEI and DXE boot phases.
//
// Input:       **PeiServices       - PEI Services table pointer
//                                    (NULL in DXE phase)
//              AccessType          - ReadType or WriteType to specify the
//                                    type of access
//              CmosRegister        - The CMOS register to access
//              *CmosParameterValue - Pointer to the data variable to be
//                                    accessed
//
// Output:      EFI_STATUS (return value) 
//                  EFI_SUCCESS - The access operation was successfull.
//                  Otherwise   - A valid EFI error code is returned.
//
// Modified:    None
//
// Referrals:   IoRead8, IoWrite8
//
// Notes:       This function is used when a translation from logical address
//              to index port value is required.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS ReadWriteCmosBank2 (
    IN EFI_PEI_SERVICES         **PeiServices,  // NULL in DXE phase
    IN CMOS_ACCESS_TYPE         AccessType,
    IN UINT16                   CmosRegister,
    IN OUT UINT8                *CmosParameterValue )
{
/**** PORTING REQUIRED ****
    if ((CmosRegister < 0x80) || (CmosRegister > 0xff))
        return EFI_INVALID_PARAMETER;

    // Some chipsets require tranlation from the logical CMOS address to a
    // physical CMOS index port value. However, other chipsets do not require
    // a translation and the index/data port can be directly used for 
    // accessing the second bank.

    IoWrite8( CMOS_IO_EXT_INDEX, (UINT8)CmosRegister );

    if (AccessType == ReadType) {
        *CmosParameterValue = IoRead8( CMOS_IO_EXT_DATA );
    } else {
        IoWrite8( CMOS_IO_EXT_DATA, *CmosParameterValue );
    }
****/
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// 
// Procedure:   SbGetRtcPowerStatus
//
// Description: This function is checked CMOS battery is good or not.
//
// Input:       **PeiServices - PEI Services table pointer (NULL in DXE phase)
//
// Output:      BOOLEAN  
//                  TRUE  - The CMOS is battery is good.
//                  FALSE - The CMOS is battery is bad.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN SbGetRtcPowerStatus (
    IN EFI_PEI_SERVICES     **PeiServices )
{
	if (MmioRead8 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) {
		return FALSE;
	} else {
		return TRUE;
	}
}

#endif  // #if CMOS_MANAGER_SUPPORT

// Begin Generic RTC SMM library porting hooks

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadCmos
//
// Description: This function reads one byte from CMOS register addressed by Index
//
// Input:       UINT8 Index
//
// Output:      UINT8 - read value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 ReadCmos(
    IN UINT8 Index
)
{
    UINT8       NMI = IoRead8(0x70) & 0x80;   //Read bit 7 (NMI setting).
    UINT8       volatile Value;
    UINT8       Reg;
//------------------
    if(Index<0x80) Reg=0x70;
    else Reg=0x72;

    IoWrite8(Reg, Index | NMI);
    Value = IoRead8(Reg+1);               //Read register.

    return (UINT8)Value;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WriteCmos
//
// Description: This function writes value to CMOS register addressed by Index
//
// Input:       UINT8 Index - CMOS register index
//              UINT8 Value - value to write
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID WriteCmos(
    IN UINT8 Index, 
    IN UINT8 Value
)
{
    UINT8       NMI = IoRead8(0x70) & 0x80;   //Read bit 7 (NMI setting).
    UINT8       Reg;
//------------------
    if(Index<0x80) Reg=0x70;
    else Reg=0x72;

    IoWrite8(Reg, Index | NMI);
    IoWrite8(Reg+1, Value);                  //Write Register.
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbLib_GetSmiState
//
// Description: This function returns SMI state
//              
// Input:       None
//
// Output:      TRUE - SMI enabled, FALSE - SMI disabled
//
// Note:        This function must work at runtime. Do not use boot time
//              services/protocols.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SbLib_GetSmiState(
    VOID
)
{
#if SMM_SUPPORT
    UINT32  SmiCtl = IoRead32 (PM_BASE_ADDRESS + 0x30) ;
    return ((SmiCtl & BIT00)!= 0);
#else
    return FALSE;
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbLib_SmiDisable
//
// Description: This function disables SMI
//              
// Input:       None
//
// Output:      None
//
// Notes:       This function should be used ONLY in critical parts of code
//              This function must work at runtime. Do not use boot time services/protocols
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SbLib_SmiDisable(
    VOID
)
{
#if SMM_SUPPORT
    RESET_IO32_PM(R_PCH_SMI_EN, 3); // 0x30
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbLib_SmiEnable
//
// Description: This function enables SMI
//              
// Input:       None
//
// Output:      None
//
// Notes:       This function should be used ONLY in critical parts of code
//              This function must work at runtime. Do not use boot time services/protocols
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SbLib_SmiEnable(
    VOID
)
{
#if SMM_SUPPORT
    SET_IO32_PM(R_PCH_SMI_EN, 2); // 0x30
    SET_IO32_PM(R_PCH_SMI_EN, 1); // 0x30
#endif
}

//---------------------------------------------------------------------------
#if SMM_SUPPORT
//---------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbSmmSaveRestoreStates
//
// Description: This hook is called in the very SMI entry and exit.
//              Save/Restore chipset data if needed.
//
// Input:       Save - TRUE = Save / FALSE = Restore
//
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbSmmSaveRestoreStates (
    IN BOOLEAN                      Save )
{
    //
    // Have done in SbPlatformHookBeforeSmmDispatch() and
    // SbPlatformHookAfterSmmDispatch.
    // There should no need to recall this fucntion in the very
    // SMI entry and exit.
    //
    return EFI_SUCCESS;
}

//---------------------------------------------------------------------------
#endif  // END OF SMM Related Porting Hooks
//---------------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CspLibCheckPowerLoss
//
// Description: This function is PM Specific function to check and Report to
//              the System Status Code - CMOS Battery and Power Supply Power
//              loss/failure. Also it responsible of clearing PM Power Loss
//              Statuses
//
// Input:       None
//
// Output:      True  - RTC Power is lost
//              False - RTC Power is well
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CspLibCheckPowerLoss (
    VOID
)
{
    UINT8       Buffer8;
    BOOLEAN     RtcLostPower = FALSE;

    Buffer8 = MmioRead8 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);

    if (Buffer8 & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) {
      RtcLostPower = TRUE;
      Buffer8 &= ~B_PCH_PMC_GEN_PMCON_RTC_PWR_STS;
      MmioWrite8 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, Buffer8);
    }

    return RtcLostPower;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbLibSetLpcDeviceDecoding
//
// Description: This function sets LPC Bridge Device Decoding
//              
// Input:       IN EFI_PCI_IO_PROTOCOL *LpcPciIo - Pointer to LPC PCI IO Protocol
//              IN UINT16 Base - I/O base address, if Base is 0 means disabled the
//                               decode of the device 
//              IN UINT8 DevUid - The device Unique ID
//              IN SIO_DEV_TYPE Type - Device Type, please refer to AMISIO.h
//
// Output:      EFI_STATUS
//                  EFI_SUCCESS - Set successfully.
//                  EFI_UNSUPPORTED - There is not proper Device Decoding 
//                                    register for the device UID.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbLibSetLpcDeviceDecoding (
    IN EFI_PCI_IO_PROTOCOL      *LpcPciIo,
    IN UINT16                   Base,
    IN UINT8                    DevUid,
    IN SIO_DEV_TYPE             Type )
{
    EFI_STATUS              Status = EFI_UNSUPPORTED;

/** Porting Required

    UINT16                  ComRange[] = { 0x3f8, 0x2f8, 0x220, 0x228,
                                           0x238, 0x2e8, 0x338, 0x3e8, 0};
    UINT16                  LptRange[] = { 0x378, 0x278, 0x3bc, 0};
    UINT16                  FpcRange[] = { 0x3f0, 0x370, 0};
    UINT16                  IoRangeMask16 = 0xffff;
    UINT16                  IoRangeSet16 = 0;
    UINT16                  IoEnMask16 = 0xffff;
    UINT16                  IoEnSet16 = 0;
    UINT8                   i;    

    switch (Type) {
        // FDC Address Range
        case (dsFDC) :
            if (Base == 0) IoEnMask16 &= ~BIT03;
            else {
                for (i = 0; (FpcRange[i] != 0) && (FpcRange[i] != Base); i++);
                if (FpcRange[i]) {
                    IoEnSet16 |= BIT03;
                    IoRangeMask16 &= ~BIT12;
                    IoRangeSet16 |= (i << 12);
                }
                else return EFI_UNSUPPORTED;
            }
            break;

        // LPT Address Range
        case (dsLPT) :
            if (Base == 0) IoEnMask16 &= ~BIT02;
            else {
                for (i = 0; (LptRange[i] != 0) && (LptRange[i] != Base); i++);
                if (LptRange[i]) {
                    IoEnSet16 |= BIT02;
                    IoRangeMask16 &= ~(BIT09 | BIT08);
                    IoRangeSet16 |= (i << 8);
                } else return EFI_UNSUPPORTED;
            }
            break;

        // ComA Address Range
        case (dsUART) :
            if (Base == 0) {
                if (DevUid) IoEnMask16 &= ~BIT01;
                else IoEnMask16 &= ~BIT00;
            } else {
                for (i = 0; (ComRange[i] != 0) && (ComRange[i] != Base); i++);
                if (ComRange[i]) {
                    if (DevUid) {
                        IoEnSet16 |= BIT01;
                        IoRangeMask16 &= ~(BIT06 | BIT05 | BIT04);
                        IoRangeSet16 |= (i << 4);
                    } else {
                        IoEnSet16 |= BIT00;
                        IoRangeMask16 &= ~(BIT02 | BIT01 | BIT00);
                        IoRangeSet16 |= i;
                    }
                } else return EFI_UNSUPPORTED;
            }
            break;

        // KBC Address Enable
        case (dsPS2K) :
        case (dsPS2M) :
        case (dsPS2CK) :
        case (dsPS2CM) :
            if (Base == 0) IoEnMask16 &= ~BIT10;
            else IoEnSet16 |= BIT10;
            break;

        // Game Port Address Enable
        case (dsGAME) :
            if (Base == 0) IoEnMask16 &= ~(BIT09 | BIT08);
            else {
                if (Base == 0x200) {
                    IoEnSet16 |= BIT08;
                } else {
                    if (Base == 0x208) IoEnSet16 |= BIT09;
                    else return EFI_UNSUPPORTED;
                }
            }
            break;

        // LPC CFG Address Enable
        case (0xff) :
            if (Base == 0x2e) IoEnSet16 |= BIT12;
            else {
                if (Base == 0x4e) IoEnSet16 |= BIT13;
                else {
                    if (Base == 0x62) IoEnSet16 |= BIT11;
                    else {
                    if (Base) SbLibSetLpcGenericDecoding( LpcPciIo,
                                                           Base ,
                                                           4,
                                                           TRUE );
                        else return EFI_UNSUPPORTED;
                    }
                }
            }
            break;

        default :
            return EFI_UNSUPPORTED;
    }

    if(LpcPciIo == NULL)
    {
      MmPci16(0, 0x1F, 0, ICH_REG_LPC_IODEC) &= ~(IoRangeMask16);  // 0x80
      MmPci16(0, 0x1F, 0, ICH_REG_LPC_IODEC) |= IoRangeSet16;

      MmPci16(0, 0x1F, 0, ICH_REG_LPC_EN) &= ~(IoEnMask16);  // 0x82
      MmPci16(0, 0x1F, 0, ICH_REG_LPC_EN) |= IoEnSet16;
    }
    else
    {
        UINT16 Data16;
    //-----------------
        Status = LpcPciIo->Pci.Read(LpcPciIo, EfiPciIoWidthUint16, ICH_REG_LPC_IODEC, 1, &Data16); 
        Data16 &= ~(IoRangeMask16);
        Data16 |= IoRangeSet16;
        Status = LpcPciIo->Pci.Write(LpcPciIo, EfiPciIoWidthUint16, ICH_REG_LPC_IODEC, 1, &Data16); 

        Status = LpcPciIo->Pci.Read(LpcPciIo, EfiPciIoWidthUint16, ICH_REG_LPC_EN, 1, &Data16); 
        Data16 &= ~(IoEnMask16);
        Data16 |= IoEnSet16;
        Status = LpcPciIo->Pci.Write(LpcPciIo, EfiPciIoWidthUint16, ICH_REG_LPC_EN, 1, &Data16); 
    }
Porting End **/

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbLibSetLpcGenericDecoding
//
// Description: This function set LPC Bridge Generic Decoding
//              
// Input:       IN EFI_PCI_IO_PROTOCOL *LpcPciIo - Pointer to LPC PCI IO Protocol
//              IN UINT16 Base - I/O base address
//              IN UINT16 Length - I/O Length
//              IN BOOLEAN Enabled - Enable/Disable the generic decode range register
//
// Output:      EFI_STATUS
//                  EFI_SUCCESS - Set successfully.
//                  EFI_UNSUPPORTED - This function is not implemented or the
//                                    Length more than the maximum supported
//                                    size of generic range decoding.
//                  EFI_INVALID_PARAMETER - the Input parameter is invalid.
//                  EFI_OUT_OF_RESOURCES - There is not available Generic
//                                         Decoding Register.
//                  EFI_NOT_FOUND - the generic decode range will be disabled
//                                  is not found.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbLibSetLpcGenericDecoding (
    IN EFI_PCI_IO_PROTOCOL      *LpcPciIo,
    IN UINT16                   Base,
    IN UINT16                   Length,
    IN BOOLEAN                  Enable )
{
/** Porting Required
    UINT32                  IoGenDecode32;
    UINT16                  IoGenDecIndex;
    UINT16                  Buffer16;
    UINT8                   Bsf8 = 0;
    UINT8                   Bsr8 = 0;

    if (Length > 0x100) return EFI_UNSUPPORTED;

    if (Length == 0) return EFI_INVALID_PARAMETER;

    if (Length < 4) Length = 4;

    // Read I/O Generic Decodes Register.
    for (IoGenDecIndex = 0; IoGenDecIndex < 4; IoGenDecIndex++) {
        IoGenDecode32 = MmPci32(0, 0x1F, 0, (ICH_REG_LPC_GEN1_DEC + IoGenDecIndex * 4));
        if (Enable) {
            if ((IoGenDecode32 & 1) == 0) break;
        } else {
            if (((IoGenDecode32 & 0xfffc) == Base) && (IoGenDecode32 & 1)) {
                IoGenDecode32 = 0; // Disable & clear the base/mask fields
                break;
            }
        }
    }

    if (Enable) {
        if (IoGenDecIndex == 4) return EFI_OUT_OF_RESOURCES;

        Buffer16 = Length;
        while ((Buffer16 % 2) == 0) {
            Buffer16 /= 2;
            Bsf8++;
        }

        while (Length) {
            Length >>= 1;
            Bsr8++;
        }

        if (Bsf8 == (Bsr8 - 1)) Bsr8--;
    
        Length = (1 << Bsr8) - 1 ;

        Base &= (~Length);

        IoGenDecode32 = Base | (UINT32)((Length >> 2) << 18) | 1;

    } else {
        if (IoGenDecIndex == 4) return EFI_NOT_FOUND;
    }

    if(LpcPciIo == NULL)
        MmPci32(0, 0x1F, 0, (ICH_REG_LPC_GEN1_DEC + IoGenDecIndex * 4)) = IoGenDecode32; // 0x84
    else
        Status = LpcPciIo->Pci.Write(LpcPciIo, 
                                     EfiPciIoWidthUint32, 
                                     (ICH_REG_LPC_GEN1_DEC + IoGenDecIndex * 4), 
                                     1, 
                                     &IoGenDecode32); 
Porting End **/

    return EFI_SUCCESS;
}

//EIP127537 >> 
#if EOP_USB_PER_PORT_CTRL

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   SbUsbPortsControlHook
//
// Description:	This function provides a feature that allows the user to
//              enable/disable USB Ports.
//
// Input:       IN  UsbPortsCtrlValue
//                  0 = port is always disabled
//                  1 = port is always enabled
//                  Note:
//                  Bit0 - Bit3: USB 2.0 Port 0 - 3
//                  Bit4 : USB 3.0 Port 0
//
// Return Value:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbUsbPortsControlHook (
//EIP160754 >>
    IN  UINT32        UsbPortsCtrlValue,
    IN  UINT8         UsbXhciMode
//EIP160754 <<
)
{
    UINT16          RegData16;
    UINTN           EhciPciMmBase;
    UINTN           XhciPciMmBase;
    BOOLEAN         EhciController;
    UINT32          XhciUsb2Pdo;
    UINT32          XhciUsb3Pdo;
    UINT32          Index;
  
    ///
    /// Open the Per-Port Disable Control Override
    ///
    RegData16 = IoRead16 ((UINTN) ((UINT64) (PM_BASE_ADDRESS + R_PCH_UPRWC)));
    RegData16 |= B_PCH_UPRWC_WR_EN;
    IoWrite16 ((UINTN) ((UINT64) (PM_BASE_ADDRESS + R_PCH_UPRWC)), RegData16);
    
    EhciPciMmBase   = CSP_PCIE_CFG_ADDRESS (
                        DEFAULT_PCI_BUS_NUMBER_PCH,
                        PCI_DEVICE_NUMBER_PCH_USB,
                        PCI_FUNCTION_NUMBER_PCH_EHCI,
                        0
                        );

    XhciPciMmBase   = CSP_PCIE_CFG_ADDRESS (
                        DEFAULT_PCI_BUS_NUMBER_PCH,
                        PCI_DEVICE_NUMBER_PCH_XHCI,
                        PCI_FUNCTION_NUMBER_PCH_XHCI,
                        0
                        );

//EIP160754 >>
    if (MmioRead32 (EhciPciMmBase) != 0xFFFFFFFF) 
    {
      EhciController = TRUE;
    } 
    else 
    {
      EhciController = FALSE;
      XhciUsb2Pdo = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB2PDO) & B_PCH_XHCI_USB2PDO_MASK;
    }

    for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) 
    {
      if (UsbPortsCtrlValue & (1 << Index)) 
      {
        if (UsbXhciMode == 2 || UsbXhciMode == 3) //AUTO or SMART AUTO
        {
          MmioAnd8 (EhciPciMmBase + R_PCH_EHCI_PDO, (UINT8) ~(B_PCH_EHCI_PDO_DIS_PORT0 << Index));

          ///
          /// XHCI PDO for HS
          ///
          XhciUsb2Pdo &= (UINT32)~(B_PCH_XHCI_USB2PDO_DIS_PORT0 << Index);
        }
        else
        {
          if (EhciController == TRUE) 
          {
            MmioAnd8 (EhciPciMmBase + R_PCH_EHCI_PDO, (UINT8) ~(B_PCH_EHCI_PDO_DIS_PORT0 << Index));
          } 
          else 
          {
            ///
            /// XHCI PDO for HS
            ///
            XhciUsb2Pdo &= (UINT32)~(B_PCH_XHCI_USB2PDO_DIS_PORT0 << Index);
          }            
        }
      } 
      else 
      {
        if (UsbXhciMode == 2 || UsbXhciMode == 3) //AUTO or SMART AUTO
        {
          MmioOr8 (EhciPciMmBase + R_PCH_EHCI_PDO, (UINT8) (B_PCH_EHCI_PDO_DIS_PORT0 << Index));

          ///
          /// XHCI PDO for HS
          ///
          XhciUsb2Pdo |= (UINT32) (B_PCH_XHCI_USB2PDO_DIS_PORT0 << Index);            
        }
        else
        {
          if (EhciController == TRUE) 
          {
            MmioOr8 (EhciPciMmBase + R_PCH_EHCI_PDO, (UINT8) (B_PCH_EHCI_PDO_DIS_PORT0 << Index));
          } 
          else 
          {
            ///
            /// XHCI PDO for HS
            ///
            XhciUsb2Pdo |= (UINT32) (B_PCH_XHCI_USB2PDO_DIS_PORT0 << Index);            
          }          
        }
      }
    }
    
    if (UsbXhciMode == 2 || UsbXhciMode == 3) //AUTO or SMART AUTO
    {
      ///
      /// XHCI PDO for SS
      ///
      XhciUsb3Pdo = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB3PDO) & B_PCH_XHCI_USB3PDO_MASK;
      for (Index = 0; Index < PCH_XHCI_MAX_USB3_PORTS; Index++) 
      {
        if (UsbPortsCtrlValue & (1 << (Index + PCH_USB_MAX_PHYSICAL_PORTS))) 
        {
          XhciUsb3Pdo &= (UINT32)~(B_PCH_XHCI_USB3PDO_DIS_PORT0 << Index);
        } 
        else 
        {
          XhciUsb3Pdo |= (UINT32) (B_PCH_XHCI_USB3PDO_DIS_PORT0 << Index);            
        }
      }
      ///
      /// USB2PDO and USB3PDO are Write-Once registers and bits in them are in the SUS Well.
      ///
      MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_USB2PDO, XhciUsb2Pdo);
      MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_USB3PDO, XhciUsb3Pdo);
    }
    else
    {
      ///
      /// XHCI PDO for SS
      ///
      if (EhciController == FALSE) 
      {
        XhciUsb3Pdo = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB3PDO) & B_PCH_XHCI_USB3PDO_MASK;
        for (Index = 0; Index < PCH_XHCI_MAX_USB3_PORTS; Index++) 
        {
          if (UsbPortsCtrlValue & (1 << (Index + PCH_USB_MAX_PHYSICAL_PORTS))) 
          {
            XhciUsb3Pdo &= (UINT32)~(B_PCH_XHCI_USB3PDO_DIS_PORT0 << Index);
          } 
          else 
          {
            XhciUsb3Pdo |= (UINT32) (B_PCH_XHCI_USB3PDO_DIS_PORT0 << Index);            
          }
        }
        ///
        /// USB2PDO and USB3PDO are Write-Once registers and bits in them are in the SUS Well.
        ///
        MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_USB2PDO, XhciUsb2Pdo);
        MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_USB3PDO, XhciUsb3Pdo);
      }  
//EIP160754 <<	            
    }
    ///
    /// Close the Per-Port Disable Control Override
    ///
    RegData16 &= (~B_PCH_UPRWC_WR_EN);
    IoWrite16 ((UINTN) ((UINT64) (PM_BASE_ADDRESS + R_PCH_UPRWC)), RegData16);
    
    return  EFI_SUCCESS;
}
#endif
//EIP127537 <<

#if CSM_SUPPORT == 1

#define MAX_PIRQS               8  // Porting Required.
UINT8   RRegs[MAX_PIRQS] = { R_PCH_ILB_PIRQA_ROUT, \
                             R_PCH_ILB_PIRQB_ROUT, \
                             R_PCH_ILB_PIRQC_ROUT, \
                             R_PCH_ILB_PIRQD_ROUT, \
                             R_PCH_ILB_PIRQE_ROUT, \
                             R_PCH_ILB_PIRQF_ROUT, \
                             R_PCH_ILB_PIRQG_ROUT, \
                             R_PCH_ILB_PIRQH_ROUT }; // Porting required

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SBGen_GetPIRQIndex
//
// Description: This function returns the 0 based PIRQ index (PIRQ0, 1 etc)
//              based on the PIRQ register number specified in the routing
//              table.
//
// Input:       PIRQRegister - Register number of the PIR
//
// Output:      An 8Bit Index for RRegs table, its range is 0 - (MAX_PIRQ -1)
//              if PIRQRegister is invalid, then 0xff will be returned.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 SBGen_GetPIRQIndex (
    IN UINT8        PIRQRegister )
{
  UINT8   rrIndx = 0;

  while ((rrIndx < MAX_PIRQS) && (RRegs[rrIndx] != PIRQRegister)) rrIndx++;

  if (rrIndx == MAX_PIRQS) return 0xff;

  return rrIndx;
}

#endif

#if defined(HPET_APIC_INTERRUPT_MODE) && (HPET_APIC_INTERRUPT_MODE != 0)
//----------------------------------------------------------------------------
// Generic IO APIC routine.
//----------------------------------------------------------------------------
UINT8 gBspLocalApicID = 0;
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicBase
//
// Description: This routine returns a structure pointer to the I/O APIC.
//
// Input:       None
//
// Output:      IO_APIC structure.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
struct IO_APIC* IoApicBase( VOID )
{
	static UINT32 IoApicAddr = 0;
	if (IoApicAddr == 0 || IoApicAddr == -1) {
        // This value may need to read from chipset registers.
		IoApicAddr = APCB;
	}

	return (struct IO_APIC*)IoApicAddr;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicWrite
//
// Description: This function writes a 32bits data to the register of
//              I/O APIC.
//
// Input:       UINT8  Reg - The register offset to be written.
//              UINT32 Value - A 32bits data will be written to the register
//                             of I/O APIC.
//
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IoApicWrite (
    IN UINT8        Reg,
    IN UINT32       Value )
{
	struct IO_APIC *IoApicStruct = IoApicBase();

	MMIO_WRITE8((UINT64)&IoApicStruct->Index, Reg);
	MMIO_WRITE32((UINT64)&IoApicStruct->Data, Value);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicRead
//
// Description: This function reads a 32bits data from the register of
//              I/O APIC.
//
// Input:       UINT8 Reg - the register offset to be read.
//
// Output:      UINT32 - A 32bits data read from the register of I/O APIC.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 IoApicRead (
    IN UINT8        Reg )
{
	struct IO_APIC *IoApicStruct = IoApicBase();

	MMIO_WRITE8((UINT64)&IoApicStruct->Index, Reg);
	return MMIO_READ32((UINT64)&IoApicStruct->Data);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicEnableIrq
//
// Description: This function enables the specific interrupt pin of I/O APIC.
//
// Input:       UINT8   Irq - The pin number of I/O APIC
//              BOOLEAN LevelTriggered - Trigger mechanism (level or edge)
//                                       for this INT pin.
//              BOOLEAN Polarity - Specifies the polarity of the INT pin.
//                                 (Active High or Active Low)
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IoApicEnableIrq (
    IN UINT8        Irq,
    IN BOOLEAN      LevelTriggered,
    IN BOOLEAN      Polarity )
{
	IO_APIC_ROUTE_ENTRY	ApicEntry;
	union ENTRY_UNION			Eu = {{0, 0}};

	ApicEntry.DestMode = 0; // 0: physical
	ApicEntry.Mask = 0;		 // 0 : enable
	ApicEntry.Dest = gBspLocalApicID; // suppose the BSP handle interrupt.
	ApicEntry.DeliveryMode = 0;      // 000: FIXED
	ApicEntry.Polarity = (Polarity) ? 1 : 0;
	ApicEntry.Trigger = (LevelTriggered) ? 1 : 0;
	ApicEntry.Vector = MASTER_INTERRUPT_BASE + Irq;

	Eu.Entry = ApicEntry;
	IoApicWrite(IO_APIC_REDIR_TABLE_HIGH + 2 * Irq, Eu.W2);
	IoApicWrite(IO_APIC_REDIR_TABLE_LOW + 2 * Irq, Eu.W1);
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicDisableIrq
//
// Description: This function disables the specific interrupt pin of I/O APIC.
//
// Input:       UINT8 Irq - The pin number of I/O APIC
//
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IoApicDisableIrq (
    IN UINT8        Irq )
{
	union ENTRY_UNION Eu = {{0, 0}};

	Eu.W1 = IoApicRead(IO_APIC_REDIR_TABLE_LOW + 2 * Irq);
	Eu.W2 = IoApicRead(IO_APIC_REDIR_TABLE_HIGH + 2 * Irq);
	Eu.Entry.Mask = 1;
	IoApicWrite(IO_APIC_REDIR_TABLE_LOW + 2 * Irq, Eu.W1);
	IoApicWrite(IO_APIC_REDIR_TABLE_HIGH + 2 * Irq, Eu.W2);
	IoApicEoi(Irq);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicMaskIrq
//
// Description: This routine masks the specific interrupt pin of I/O APIC.
//
// Input:       UINT8 Irq - The pin number of I/O APIC
//
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IoApicMaskIrq (
    IN UINT8        Irq )
{
	union ENTRY_UNION Eu = {{0, 0}};

	Eu.W1 = IoApicRead(IO_APIC_REDIR_TABLE_LOW + 2 * Irq);
	Eu.Entry.Mask = 1;
	IoApicWrite(IO_APIC_REDIR_TABLE_LOW + 2 * Irq, Eu.W1);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicUnmaskIrq
//
// Description: This routine unmasks the specific interrupt pin of I/O APIC.
//
// Input:       UINT8 Irq - The pin number of I/O APIC
//
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IoApicUnmaskIrq (
    IN UINT8        Irq )
{
	union ENTRY_UNION Eu = {{0, 0}};

	Eu.W1 = IoApicRead(IO_APIC_REDIR_TABLE_LOW + 2 * Irq);
	Eu.Entry.Mask = 0;
	IoApicWrite(IO_APIC_REDIR_TABLE_LOW + 2 * Irq, Eu.W1);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoApicEoi
//
// Description: This routine issues an EOI to the specific pin of I/O APIC.
//
// Input:       UINT8 Irq - The pin number of I/O APIC
//
// Output:      EFI_SUCCESS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IoApicEoi (
    IN UINT8        Irq )
{
	EFI_STATUS 	Status = EFI_SUCCESS;
	UINT32		Vector = MASTER_INTERRUPT_BASE + Irq;
	struct IO_APIC *IoApicStruct = IoApicBase();
	UINT32		Isr = 0;
	union ENTRY_UNION Eu = {{0, 0}};

	do {
		MMIO_WRITE32((UINT64)&IoApicStruct->Eoi, Vector);
		Eu.W1 = IoApicRead(IO_APIC_REDIR_TABLE_LOW + 2 * Irq);
	} while (Eu.Entry.Irr);

	do {
		MMIO_WRITE32(LOCAL_APIC_BASE + APIC_EOI_REGISTER, Vector);
		Isr = MMIO_READ32(LOCAL_APIC_BASE + ISR_REG (Vector));
	} while (Isr & ISR_BIT(Vector));

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetHpetApicPin
//
// Description: This routine gets the pin number of I/O APIC for HPET.
//
// Input:       None
//
// Output:      UINT8 Irq - The pin number of I/O APIC for HPET.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 GetHpetApicPin (VOID)
{
	EFI_STATUS 	Status = EFI_SUCCESS;
	UINT8		Irq = 0;

    volatile HPET_TIMER_CONFIGURATION_REGISTER   TimerConfiguration;

    TimerConfiguration.Uint64 = MMIO_READ64( HPET_BASE_ADDRESS + HPET_TIMER_CONFIGURATION_OFFSET + HPET_OFFSET * HPET_TIMER_STRIDE );
	Irq = TimerConfiguration.Bits.InterruptRoute;

	return Irq;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsHpetApicEnable
//
// Description: This routine checks the pin of I/O APIC for HPET is enabled or
//              not.
//
// Input:       None
//
// Output:      TRUE  - The pin of I/O APIC for HPET is enabled 
//              FALSE - The pin of I/O APIC for HPET is disabled 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsHpetApicEnable (VOID)
{
	union ENTRY_UNION Eu = {{0, 0}};
	UINT8 Irq;

	Irq = GetHpetApicPin();

	Eu.W1 = IoApicRead(IO_APIC_REDIR_TABLE_LOW + 2 * Irq);

	return (Eu.Entry.Mask) ? FALSE : TRUE;
}
#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
