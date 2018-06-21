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
// Name:        SbCspLib.h
//
// Description: 
//
// Notes:       
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __SBLIB_H__
#define __SBLIB_H__

#include <Pei.h>
#include <PciBus.h>  //EIP176554
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/AmiSio.h>
#include <Ppi/ReadOnlyVariable2.h>

#if     CSM_SUPPORT
#include <Protocol/LegacyInterrupt.h>
#endif

#if ACPI_SUPPORT
#include <Protocol/S3SaveState.h>
#endif

#ifndef AMI_S3_SAVE_PROTOCOL
#define AMI_S3_SAVE_PROTOCOL      EFI_S3_SAVE_STATE_PROTOCOL
#define AMI_S3_SAVE_PROTOCOL_GUID &gEfiS3SaveStateProtocolGuid
#endif

//EIP167096 >>
#define  KBShift  10
#define  MBShift  20

#define ALIGNMENT_4KB     0x1000       

typedef enum {
    AmiUndefinedType        = 0,
    AmiDescriptorType       = 1,
    AmiBiosType             = 2,
    AmiTxeType              = 3
} AMI_PCH_SPI_REGION_TYPE;

typedef struct _AMI_SPI_PROTECTED_RANGE_CONIFG{
    AMI_PCH_SPI_REGION_TYPE  AmiPchSpiRegionType; 
    BOOLEAN                  WriteProtectionEnable;
    BOOLEAN                  ReadProtectionEnable;
    UINT32                   ProtectedRangeBase;
    UINT32                   ProtectedRangeLength;
} AMI_SPI_PROTECTED_RANGE_CONIFG;
//EIP167096 <<

#ifdef __cplusplus
extern "C" {
#endif

#if     CSM_SUPPORT
EFI_STATUS SbGenInitializeRouterRegisters(
    IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRBIo
);

UINT8 SBGen_GetPIRQIndex(
    IN UINT8 PIRQRegister
);

EFI_STATUS SbGenReadPirq(
    IN EFI_LEGACY_INTERRUPT_PROTOCOL    *This,
    IN UINT8                            PirqNumber,
    OUT UINT8                           *PirqData 
);

EFI_STATUS SbGenWritePirq(
    IN EFI_LEGACY_INTERRUPT_PROTOCOL    *This,
    IN UINT8                            PirqNumber,
    IN UINT8                            PirqData
);
#endif

VOID SbLibShutdown(
    VOID
);

#if SB_RESET_PPI_SUPPORT
VOID SBLib_ResetSystem(
    IN  EFI_RESET_TYPE  ResetType
);
#endif

#if SB_STALL_PPI_SUPPORT
EFI_STATUS CountTime(
    IN  UINTN   DelayTime,
    IN  UINT16  BaseAddr            // only needs to be 16 bit for I/O address
);
#endif

EFI_STATUS SbLibSetLpcDeviceDecoding(
    IN EFI_PCI_IO_PROTOCOL      *LpcPciIo,
    IN UINT16                   Base,
    IN UINT8                    DevUid,
    IN SIO_DEV_TYPE             Type
);

EFI_STATUS SbLibSetLpcGenericDecoding(
    IN EFI_PCI_IO_PROTOCOL      *LpcPciIo,
    IN UINT16                   Base,
    IN UINT16                   Length,
    IN BOOLEAN                  Enable
);

BOOLEAN SbIsDefaultConfigMode (
    IN EFI_PEI_SERVICES                 **PeiServices,
    IN EFI_PEI_READ_ONLY_VARIABLE2_PPI  *ReadVariablePpi
);

UINT8 ReadCmos(
    IN UINT8 Index
);

VOID WriteCmos(
    IN UINT8 Index, 
    IN UINT8 Value
);

BOOLEAN SbLib_GetSmiState(
    VOID
);

VOID SbLib_SmiDisable(
    VOID
);

VOID SbLib_SmiEnable(
    VOID
);

BOOLEAN CspLibCheckPowerLoss (
    VOID
);

#if SMM_SUPPORT
EFI_STATUS SbSmmSaveRestoreStates (
    IN BOOLEAN                      Save
);
#endif

#if BIOS_LOCK_ENABLE
VOID BiosLockEnableSmiFlashHook (
    IN UINT8                        SwSmiNum,
    IN OUT UINT64                   Buffer
);
#endif

//EIP160150 >>
//EIP164801(-) #if FtRecovery_SUPPORT
BOOLEAN IsTopSwapOn(
    VOID
);

VOID  SetTopSwap(
    IN BOOLEAN                      On
);
//EIP164801(-) #endif
//EIP160150 <<

//EIP167096 >>
EFI_STATUS SbFlashProtectedRange  (
    VOID
);
//EIP167096 <<

//EIP176554 >>
//---------------------------------------------------------------------------
// OEM Generic Functions Support
//---------------------------------------------------------------------------
//EIP188072 >>
typedef enum {
    SbResetFull = 3,
    SbResetGlobal,
    SbResetGlobalWithEc
} SB_EXT_RESET_TYPE;
//EIP188072 <<

typedef enum {
  SB_DEV_LPSS1_DMA1 = 0,
  SB_DEV_LPSS1_PWM1,
  SB_DEV_LPSS1_PWM2,
  SB_DEV_LPSS1_HSUART1,
  SB_DEV_LPSS1_HSUART2,
  SB_DEV_LPSS1_SPI1,
  SB_DEV_LPSS1_SPI2,
  SB_DEV_LPSS1_SPI3,
  SB_DEV_EMMC,
  SB_DEV_SDIO,
  SB_DEV_SDCARD,
  SB_DEV_EMMC_45,
  SB_DEV_AZALIA,
  SB_DEV_LPE = 13,
  SB_DEV_OTG,
  SB_DEV_XHCI,
  SB_DEV_SATA1 = 17,
  SB_DEV_EHCI1,
  SB_DEV_SEC,
  SB_DEV_PCI_EX_PORT1,
  SB_DEV_PCI_EX_PORT2,
  SB_DEV_PCI_EX_PORT3,
  SB_DEV_PCI_EX_PORT4,
  SB_DEV_LPSS2_DMA2,
  SB_DEV_LPSS2_I2C1,
  SB_DEV_LPSS2_I2C2,
  SB_DEV_LPSS2_I2C3,
  SB_DEV_LPSS2_I2C4,
  SB_DEV_LPSS2_I2C5,
  SB_DEV_LPSS2_I2C6,
  SB_DEV_LPSS2_I2C7,
  SB_DEV_SMBUS,
  SB_DEV_FUNCTION_0 = 64,
  SB_DEV_ADSP,
  SB_DEV_EHCI2,
  SB_DEV_LPC_BRIDGE,
  SB_DEV_PCI_EX_PORT5,
  SB_DEV_PCI_EX_PORT6,
  SB_DEV_PCI_EX_PORT7,
  SB_DEV_PCI_EX_PORT8,
  SB_DEV_THERMAL,
  SB_DEV_SATA2,
} SB_DEV_TYPE;

typedef enum {
  SbComA,
  SbComB,
  SbLpt,
  SbFdd,
  SbGameL,
  SbGameH,
  SbKbc,
  SbMc,
  SbCnf1,	// 0x2E & 0x2F
  SbCnf2	// 0x4E & 0x4F
} SB_LPC_SIO_TYPE;

typedef enum {
  TYPE_HOT_PLUG = 1,
  TYPE_SWGPE,
  TYPE_PCI_EXP = 9,
  TYPE_BATLOW,
  TYPE_PME_B0 = 13,
  TYPE_PME = 16,
  TYPE_ME_SCI,
  TYPE_GP27,
  TYPE_TCOSCI,
  TYPE_RI,
  TYPE_WADT
} SB_GPE0_TYPE;

typedef enum {
  GPE0_EN_OP_SAVE,
  GPE0_EN_OP_RESTORE,
  GPE0_EN_OP_CLEAR_ALL
} GPE0_EN_OP;

typedef enum {
  CPU_THERMAL_TRIP_STATUS,
  AFTERG3_EN,
  PWR_FLR
} SB_MISC_TYPE;

typedef enum {
  PCONF0,
  PCONF1,
  PADVAL
} GPIO_REG_TYPE;

typedef enum {
  GPIO_NC,
  GPIO_SC,
  GPIO_SUS,
  GPIO_Normal
} AMI_GPIO_GROUP_TYPE;

typedef struct _AMI_OEM_GPIO {
  AMI_GPIO_GROUP_TYPE   Group;
  UINT8                 PinNum;
} AMI_OEM_GPIO;

static UINTN Nc_Gpio_Offset_Xlat_Tbl[] = {
    0x13,    // GPIONC_000 (HV_DDI0_HPD)
    0x12,    // GPIONC_001 (HV_DDI0_DDC_SDA)
    0x11,    // GPIONC_002 (HV_DDI0_DDC_SCL)
    0x14,    // GPIONC_003 (PANEL0_VDDEN)
    0x15,    // GPIONC_004 (PANEL0_BKLTEN)
    0x16,    // GPIONC_005 (PANEL0_BKLTCTL)
    0x18,    // GPIONC_006 (HV_DDI1_HPD)
    0x19,    // GPIONC_007 (HV_DDI1_DDC_SDA)
    0x17,    // GPIONC_008 (HV_DDI1_DDC_SCL)
    0x10,    // GPIONC_009 (PANEL1_VDDEN)
    0x0e,    // GPIONC_010 (PANEL1_BKLTEN)
    0x0f,    // GPIONC_011 (PANEL1_BKLTCTL)
    0x0c,    // GPIONC_012 (GP_INTD_DSI_TE1)
    0x1a,    // GPIONC_013 (HV_DDI2_DDC_SDA)
    0x1b,    // GPIONC_014 (HV_DDI2_DDC_SCL)
    0x01,    // GPIONC_015 (GP_CAMERASB00)
    0x04,    // GPIONC_016 (GP_CAMERASB01)
    0x08,    // GPIONC_017 (GP_CAMERASB02)
    0x0b,    // GPIONC_018 (GP_CAMERASB03)
    0x00,    // GPIONC_019 (GP_CAMERASB04)
    0x03,    // GPIONC_020 (GP_CAMERASB05)
    0x06,    // GPIONC_021 (GP_CAMERASB06)
    0x0a,    // GPIONC_022 (GP_CAMERASB07)
    0x0d,    // GPIONC_023 (GP_CAMERASB08)
    0x02,    // GPIONC_024 (GP_CAMERASB09)
    0x05,    // GPIONC_025 (GP_CAMERASB10)
    0x09     // GPIONC_026 (GP_CAMERASB11)
};

static UINTN Sc_Gpio_Offset_Xlat_Tbl[] = {
    0x55,    // GPIOSC_000 (SATA_GP0)
    0x59,    // GPIOSC_001 (SATA_GP1)
    0x5d,    // GPIOSC_002 (SATA_LEDN)
    0x60,    // GPIOSC_003 (PCIE_CLKREQ0B)
    0x63,    // GPIOSC_004 (PCIE_CLKREQ1B)
    0x66,    // GPIOSC_005 (PCIE_CLKREQ2B)
    0x62,    // GPIOSC_006 (PCIE_CLKREQ3B)
    0x65,    // GPIOSC_007 (PCIE_CLKREQ4B)
    0x22,    // GPIOSC_008 (HDA_RSTB)
    0x25,    // GPIOSC_009 (HDA_SYNC)
    0x24,    // GPIOSC_010 (HDA_CLK)
    0x26,    // GPIOSC_011 (HDA_SDO)
    0x27,    // GPIOSC_012 (HDA_SDI0)
    0x23,    // GPIOSC_013 (HDA_SDI1)
    0x28,    // GPIOSC_014 (HDA_DOCKRSTB)
    0x54,    // GPIOSC_015 (HDA_DOCKENB)
    0x3e,    // GPIOSC_016 (SDMMC1_CLK)
    0x3d,    // GPIOSC_017 (SDMMC1_D0)
    0x40,    // GPIOSC_018 (SDMMC1_D1)
    0x3b,    // GPIOSC_019 (SDMMC1_D2)
    0x36,    // GPIOSC_020 (SDMMC1_D3_CD_B)
    0x38,    // GPIOSC_021 (MMC1_D4_SD_WE)
    0x3c,    // GPIOSC_022 (MMC1_D5)
    0x37,    // GPIOSC_023 (MMC1_D6)
    0x3f,    // GPIOSC_024 (MMC1_D7)
    0x39,    // GPIOSC_025 (SDMMC1_CMD)
    0x33,    // GPIOSC_026 (MMC1_RESET_B)
    0x32,    // GPIOSC_027 (SDMMC2_CLK)
    0x35,    // GPIOSC_028 (SDMMC2_D0)
    0x2f,    // GPIOSC_029 (SDMMC2_D1)
    0x34,    // GPIOSC_030 (SDMMC2_D2)
    0x31,    // GPIOSC_031 (SDMMC2_D3_CD_B)
    0x30,    // GPIOSC_032 (SDMMC2_CMD)
    0x2b,    // GPIOSC_033 (SDMMC3_CLK)
    0x2e,    // GPIOSC_034 (SDMMC3_D0)
    0x29,    // GPIOSC_035 (SDMMC3_D1)
    0x2d,    // GPIOSC_036 (SDMMC3_D2)
    0x2a,    // GPIOSC_037 (SDMMC3_D3)
    0x3a,    // GPIOSC_038 (SDMMC3_CD_B)
    0x2c,    // GPIOSC_039 (SDMMC3_CMD)
    0x5f,    // GPIOSC_040 (SDMMC3_1P8_EN)
    0x69,    // GPIOSC_041 (SDMMC3_PWR_EN_B)
    0x46,    // GPIOSC_042 (LPC_AD0)
    0x44,    // GPIOSC_043 (LPC_AD1)
    0x43,    // GPIOSC_044 (LPC_AD2)
    0x42,    // GPIOSC_045 (LPC_AD3)
    0x45,    // GPIOSC_046 (LPC_FRAMEB)
    0x47,    // GPIOSC_047 (LPC_CLKOUT0)
    0x41,    // GPIOSC_048 (LPC_CLKOUT1)
    0x48,    // GPIOSC_049 (LPC_CLKRUNB)
    0x56,    // GPIOSC_050 (ILB_SERIRQ)
    0x5a,    // GPIOSC_051 (SMB_DATA)
    0x58,    // GPIOSC_052 (SMB_CLK)
    0x5c,    // GPIOSC_053 (SMB_ALERTB)
    0x67,    // GPIOSC_054 (SPKR)
    0x4d,    // GPIOSC_055 (MHSI_ACDATA)
    0x4f,    // GPIOSC_056 (MHSI_ACFLAG)
    0x53,    // GPIOSC_057 (MHSI_ACREADY)
    0x4e,    // GPIOSC_058 (MHSI_ACWAKE)
    0x51,    // GPIOSC_059 (MHSI_CADATA)
    0x50,    // GPIOSC_060 (MHSI_CAFLAG)
    0x52,    // GPIOSC_061 (MHSI_CAREADY)
    0x0d,    // GPIOSC_062 (GP_SSP_2_CLK)
    0x0c,    // GPIOSC_063 (GP_SSP_2_FS)
    0x0f,    // GPIOSC_064 (GP_SSP_2_RXD)
    0x0e,    // GPIOSC_065 (GP_SSP_2_TXD)
    0x11,    // GPIOSC_066 (SPI1_CS0_B)
    0x12,    // GPIOSC_067 (SPI1_MISO)
    0x13,    // GPIOSC_068 (SPI1_MOSI)
    0x10,    // GPIOSC_069 (SPI1_CLK)
    0x02,    // GPIOSC_070 (UART1_RXD)
    0x01,    // GPIOSC_071 (UART1_TXD)
    0x00,    // GPIOSC_072 (UART1_RTS_B)
    0x04,    // GPIOSC_073 (UART1_CTS_B)
    0x06,    // GPIOSC_074 (UART2_RXD)
    0x07,    // GPIOSC_075 (UART2_TXD)
    0x09,    // GPIOSC_076 (UART2_RTS_B)
    0x08,    // GPIOSC_077 (UART2_CTS_B)
    0x21,    // GPIOSC_078 (I2C0_SDA)
    0x20,    // GPIOSC_079 (I2C0_SCL)
    0x1f,    // GPIOSC_080 (I2C1_SDA)
    0x1e,    // GPIOSC_081 (I2C1_SCL)
    0x1d,    // GPIOSC_082 (I2C2_SDA)
    0x1b,    // GPIOSC_083 (I2C2_SCL)
    0x19,    // GPIOSC_084 (I2C3_SDA)
    0x1c,    // GPIOSC_085 (I2C3_SCL)
    0x1a,    // GPIOSC_086 (I2C4_SDA)
    0x17,    // GPIOSC_087 (I2C4_SCL)
    0x15,    // GPIOSC_088 (I2C5_SDA)
    0x14,    // GPIOSC_089 (I2C5_SCL)
    0x18,    // GPIOSC_090 (I2C6_SDA)
    0x16,    // GPIOSC_091 (I2C6_SCL)
    0x05,    // GPIOSC_092 (I2C_NFC_SDA)
    0x03,    // GPIOSC_093 (I2C_NFC_SCL)
    0x0a,    // GPIOSC_094 (PWM0)
    0x0b,    // GPIOSC_095 (PWM1)
    0x6a,    // GPIOSC_096 (PLT_CLK0)
    0x57,    // GPIOSC_097 (PLT_CLK1)
    0x5b,    // GPIOSC_098 (PLT_CLK2)
    0x68,    // GPIOSC_099 (PLT_CLK3)
    0x61,    // GPIOSC_100 (PLT_CLK4)
    0x64     // GPIOSC_101 (PLT_CLK5)
};

static UINTN Sus_Gpio_Offset_Xlat_Tbl[] = {
    0x1d,    // GPIOSUS_000 (GPIO_SUS0)
    0x21,    // GPIOSUS_001 (GPIO_SUS1)
    0x1e,    // GPIOSUS_002 (GPIO_SUS2)
    0x1f,    // GPIOSUS_003 (GPIO_SUS3)
    0x20,    // GPIOSUS_004 (GPIO_SUS4)
    0x22,    // GPIOSUS_005 (GPIO_SUS5)
    0x24,    // GPIOSUS_006 (GPIO_SUS6)
    0x23,    // GPIOSUS_007 (GPIO_SUS7)
    0x26,    // GPIOSUS_008 (SEC_GPIO_SUS8)
    0x25,    // GPIOSUS_009 (SEC_GPIO_SUS9)
    0x12,    // GPIOSUS_010 (SEC_GPIO_SUS10)
    0x07,    // GPIOSUS_011 (SUSPWRDNACK)
    0x0b,    // GPIOSUS_012 (PMU_SUSCLK)
    0x14,    // GPIOSUS_013 (PMU_SLP_S0IX_B)
    0x11,    // GPIOSUS_014 (PMU_SLP_LAN_B)
    0x01,    // GPIOSUS_015 (PMU_WAKE_B)
    0x08,    // GPIOSUS_016 (PMU_PWRBTN_B)
    0x0a,    // GPIOSUS_017 (PMU_WAKE_LAN_B)
    0x13,    // GPIOSUS_018 (SUS_STAT_B)
    0x0c,    // GPIOSUS_019 (USB_OC0_B)
    0x00,    // GPIOSUS_020 (USB_OC1_B)
    0x02,    // GPIOSUS_021 (SPI_CS1_B)
    0x17,    // GPIOSUS_022 (GPIO_DFX0)
    0x27,    // GPIOSUS_023 (GPIO_DFX1)
    0x1c,    // GPIOSUS_024 (GPIO_DFX2)
    0x1b,    // GPIOSUS_025 (GPIO_DFX3)
    0x16,    // GPIOSUS_026 (GPIO_DFX4)
    0x15,    // GPIOSUS_027 (GPIO_DFX5)
    0x18,    // GPIOSUS_028 (GPIO_DFX6)
    0x19,    // GPIOSUS_029 (GPIO_DFX7)
    0x1a,    // GPIOSUS_030 (GPIO_DFX8)
    0x33,    // GPIOSUS_031 (USB_ULPI_0_CLK)
    0x38,    // GPIOSUS_032 (USB_ULPI_0_DATA0)
    0x36,    // GPIOSUS_033 (USB_ULPI_0_DATA1)
    0x31,    // GPIOSUS_034 (USB_ULPI_0_DATA2)
    0x37,    // GPIOSUS_035 (USB_ULPI_0_DATA3)
    0x30,    // GPIOSUS_036 (USB_ULPI_0_DATA4)
    0x39,    // GPIOSUS_037 (USB_ULPI_0_DATA5)
    0x32,    // GPIOSUS_038 (USB_ULPI_0_DATA6)
    0x3a,    // GPIOSUS_039 (USB_ULPI_0_DATA7)
    0x34,    // GPIOSUS_040 (USB_ULPI_0_DIR)
    0x35,    // GPIOSUS_041 (USB_ULPI_0_NXT)
    0x3b,    // GPIOSUS_042 (USB_ULPI_0_STP)
    0x28     // GPIOSUS_043 (USB_ULPI_0_REFCLK)
};

//EIP188072 >>
VOID SBLib_ExtResetSystem (
  IN SB_EXT_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL  
);
//EIP188072 <<

VOID SbEnableDisableFunctions (
    IN SB_DEV_TYPE  SbDevType,
    IN BOOLEAN      Enable
);

VOID SbSaveRestoreAllHwSmi (
    IN BOOLEAN      Save
);

VOID SbDisableAllHwSmi (
    VOID
);

BOOLEAN SbCheckNmiStatus (
    VOID
);

BOOLEAN SbEnableDisableNmi (
    IN BOOLEAN      Enable
);

VOID SbSaveRestoreNmi (
    IN BOOLEAN      Save
);

VOID SbSwSmiTrigger (
    IN UINT8        SwSmi
);

VOID SbSwSmiIo (
    IN     UINT8    SwSmi,
    IN OUT UINT8    *Data
);

UINT16 SbGetSwSmi (
    IN OUT UINT32   *DataValue
);

UINT16 SbGetTco2StsAddress (
    IN UINT16       AcpiBaseAddr
);

UINT16 SbGetTco2CntAddress (
    IN UINT16       AcpiBaseAddr
);

VOID SetAfterG3Bit (
    IN BOOLEAN      Set
);

BOOLEAN SbCheckAfterG3 (
    VOID
);

VOID SbDisableLpcDevices (
    IN SB_LPC_SIO_TYPE  SioType
);

VOID SbEnableEhciSmi (
    VOID
);

VOID SbDisableEhciSmi (
    VOID
);

BOOLEAN IsPowerButtonPressed (
    VOID
);

VOID SbEnablePciPme (
    IN UINTN        PciAddress
);

VOID SbDisablePciPme (
    IN UINTN        PciAddress
);

VOID SbEnableWolPmConfg (
    VOID
);

VOID SbDisableWolPmConfg (
    VOID
);

UINT16 SbGetIoTrapInfo (
    IN OUT UINT32   *TrappedIoData 
);

UINT16 SbGetAcpiBaseAddress (
    VOID
);

UINT16 SbGetPm1CntOffset (
    VOID
);

UINT32 SbGetRcrbAddress (
    VOID
);

BOOLEAN SbIsRtcPwrValid (
    VOID
);

BOOLEAN SbGetGpe0En (
    IN SB_GPE0_TYPE Gpe0Type
);

VOID SbSetGpe0En (
    IN SB_GPE0_TYPE Gpe0Type
);

VOID SbResetGpe0En (
    IN SB_GPE0_TYPE Gpe0Type
);

VOID SbGpe0Operation (
    IN GPE0_EN_OP   Operation
);

BOOLEAN SbGetGpe0Sts (
    IN SB_GPE0_TYPE Gpe0Type
);

VOID SbClearGpe0Sts (
    IN SB_GPE0_TYPE Gpe0Type
);

VOID SbSetGpe0GpinEn (
    IN AMI_OEM_GPIO Gpio
);

BOOLEAN SbGetGpe0GpinEn (
    IN AMI_OEM_GPIO Gpio
);

VOID SbResetGpe0GpinEn (
    IN AMI_OEM_GPIO Gpio
);

BOOLEAN SbGetGpe0GpinSts (
    IN AMI_OEM_GPIO Gpio
);

VOID SbClearGpe0GpinSts (
    IN AMI_OEM_GPIO Gpio
);

BOOLEAN SbGetGpioUseSel (
    IN AMI_OEM_GPIO Gpio
);

VOID SbProgGpioUseSel (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      GpioMode,
    IN UINT8        MulFunc
);

BOOLEAN SbGetGpioIoSel (
    IN AMI_OEM_GPIO Gpio
);

VOID SbSetGpioIoSel (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      InputMode
);

BOOLEAN SbGetGpioLvlSel (
    IN AMI_OEM_GPIO Gpio
);

VOID SbSetGpioLvlSel (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      High
);

UINT32 SbReadAltGpiSmiSts (
    VOID
);

VOID SbClearAltGpiSmiSts (
    IN AMI_OEM_GPIO Gpio
);

VOID SbProgramAltGpiSmi (
    IN AMI_OEM_GPIO Gpio,
    IN BOOLEAN      Set
);

VOID SbProgramGpioRout (
    IN AMI_OEM_GPIO Gpio,
    IN UINT8        Mode
);

VOID SbProgramGpioRegister (
    IN AMI_OEM_GPIO   Gpio,
    IN GPIO_REG_TYPE  RegType,
    IN UINT32         AndData,
    IN UINT32         OrData
);

UINT32 SbReadGpioRegister (
    IN AMI_OEM_GPIO   Gpio,
    IN GPIO_REG_TYPE  RegType
);

EFI_STATUS SbGetMiscBitStatus(
    IN SB_MISC_TYPE         SbMiscType,
    IN OUT UINT8            *BitStatus
);

EFI_STATUS SbProgramMiscBit(
    IN SB_MISC_TYPE         SbMiscType,
    IN BOOLEAN              Set
);

BOOLEAN SbIsXhciRouting ( 
    VOID 
);

EFI_STATUS SbGetChipLanMacAddress ( 
    IN OUT UINT8            *MacAddress
);
//EIP176554 <<

//---------------------------------------------------------------------------
// Standard I/O Macros, No Porting Required.
//---------------------------------------------------------------------------
#define ReadIo8(IoAddr)           IoRead8(IoAddr)
#define READ_IO8(IoAddr)          IoRead8(IoAddr)
#define WriteIo8(IoAddr, bVal)    IoWrite8(IoAddr, bVal)
#define WRITE_IO8(IoAddr, bVal)   IoWrite8(IoAddr, bVal)
#define SET_IO8(IoAddr, bSet)     IoWrite8(IoAddr, IoRead8(IoAddr) | (bSet))
#define RESET_IO8(IoAddr, bRst)   IoWrite8(IoAddr, IoRead8(IoAddr) & ~(bRst))
#define RW_IO8(Bx, Set, Rst)      IoWrite8(Bx, IoRead8(Bx) & ~(Rst) | (Set))
#define ReadIo16(IoAddr)          IoRead16(IoAddr)
#define READ_IO16(IoAddr)         IoRead16(IoAddr)
#define WriteIo16(IoAddr, wVal)   IoWrite16(IoAddr, wVal)
#define WRITE_IO16(IoAddr, wVal)  IoWrite16(IoAddr, wVal)
#define SET_IO16(IoAddr, wSet)    IoWrite16(IoAddr, IoRead16(IoAddr) | (wSet))
#define RESET_IO16(IoAddr, Rst)   IoWrite16(IoAddr, IoRead16(IoAddr) & ~(Rst))
#define RW_IO16(Bx, Set, Rst)     IoWrite16(Bx, IoRead16(Bx) & ~(Rst) | (Set))
#define ReadIo32(IoAddr)          IoRead32(IoAddr)
#define READ_IO32(IoAddr)         IoRead32(IoAddr)
#define WriteIo32(IoAddr, dVal)   IoWrite32(IoAddr, dVal)
#define WRITE_IO32(IoAddr, dVal)  IoWrite32(IoAddr, dVal)
#define SET_IO32(IoAddr, dSet)    IoWrite32(IoAddr, IoRead32(IoAddr) | (dSet))
#define RESET_IO32(IoAddr, Rst)   IoWrite32(IoAddr, IoRead32(IoAddr) & ~(Rst))
#define RW_IO32(Bx, Set, Rst)     IoWrite32(Bx, IoRead32(Bx) & ~(Rst) | (Set))

#define WRITE_IO8_S3(mBtScSv, IoAddr16, bValue) \
                                    WriteIo8S3(mBtScSv, IoAddr16, bValue)
#define SET_IO8_S3(mBtScSv, IoAddr16, bSet) \
                                    RwIo8S3(mBtScSv, IoAddr16, bSet, 0)
#define RESET_IO8_S3(mBtScSv, IoAddr16, bReset) \
                                    RwIo8S3(mBtScSv, IoAddr16, 0, bReset) 
#define RW_IO8_S3(mBtScSv, IoAddr16, bSet, bReset) \
                                    RwIo8S3(mBtScSv, IoAddr16, bSet, bReset)
#define WRITE_IO16_S3(mBtScSv, IoAddr16, wValue) \
                                    WriteIo16S3(mBtScSv, IoAddr16, wValue)
#define SET_IO16_S3(mBtScSv, IoAddr16, wSet) \
                                    RwIo16S3(mBtScSv, IoAddr16, wSet, 0)
#define RESET_IO16_S3(mBtScSv, IoAddr16, wReset) \
                                    RwIo16S3(mBtScSv, IoAddr16, 0, wReset) 
#define RW_IO16_S3(mBtScSv, IoAddr16, wSet, wReset) \
                                    RwIo16S3(mBtScSv, IoAddr16, wSet, wReset)
#define WRITE_IO32_S3(mBtScSv, IoAddr16, dValue) \
                                    WriteIo32S3(mBtScSv, IoAddr16, dValue)
#define SET_IO32_S3(mBtScSv, IoAddr16, dSet) \
                                    RwIo32S3(mBtScSv, IoAddr16, dSet, 0)
#define RESET_IO32_S3(mBtScSv, IoAddr16, dReset) \
                                    RwIo32S3(mBtScSv, IoAddr16, 0, dReset) 
#define RW_IO32_S3(mBtScSv, IoAddr16, dSet, dReset) \
                                    RwIo32S3(mBtScSv, IoAddr16, dSet, dReset)

//---------------------------------------------------------------------------
// Chipset PCI Macros, Porting Required.
//---------------------------------------------------------------------------

#define READ_PCI8_SB(Rx)          READ_PCI8(SB_BUS, SB_DEV, SB_FUN, Rx)
#define WRITE_PCI8_SB(Rx, Val)    WRITE_PCI8(SB_BUS, SB_DEV, SB_FUN, Rx, Val)
#define SET_PCI8_SB(Rx, Set)      SET_PCI8(SB_BUS, SB_DEV, SB_FUN, Rx, Set)
#define RESET_PCI8_SB(Rx, Rst)    RESET_PCI8(SB_BUS, SB_DEV, SB_FUN, Rx, Rst)
#define RW_PCI8_SB(Rx, St, Rt)    RW_PCI8(SB_BUS, SB_DEV, SB_FUN, Rx, St, Rt)
#define READ_PCI16_SB(Rx)         READ_PCI16(SB_BUS, SB_DEV, SB_FUN, Rx)
#define WRITE_PCI16_SB(Rx, Val)   WRITE_PCI16(SB_BUS, SB_DEV, SB_FUN, Rx, Val)
#define SET_PCI16_SB(Rx, Set)     SET_PCI16(SB_BUS, SB_DEV, SB_FUN, Rx, Set)
#define RESET_PCI16_SB(Rx, Rst)   RESET_PCI16(SB_BUS, SB_DEV, SB_FUN, Rx, Rst)
#define RW_PCI16_SB(Rx, St, Rt)   RW_PCI16(SB_BUS, SB_DEV, SB_FUN, Rx, St, Rt)
#define READ_PCI32_SB(Rx)         READ_PCI32(SB_BUS, SB_DEV, SB_FUN, Rx)
#define WRITE_PCI32_SB(Rx, Val)   WRITE_PCI32(SB_BUS, SB_DEV, SB_FUN, Rx, Val)
#define SET_PCI32_SB(Rx, Set)     SET_PCI32(SB_BUS, SB_DEV, SB_FUN, Rx, Set)
#define RESET_PCI32_SB(Rx, Rst)   RESET_PCI32(SB_BUS, SB_DEV, SB_FUN, Rx, Rst)
#define RW_PCI32_SB(Rx, St, Rt)   RW_PCI32(SB_BUS, SB_DEV, SB_FUN, Rx, St, Rt)

//---------------------------------------------------------------------------
//EIP131059 >>
#define READ_PCI8_SATA(Rx)       READ_PCI8(SATA_BUS, SATA_DEV, SATA_FUNC, Rx)
#define WRITE_PCI8_SATA(Rx, Vx)  WRITE_PCI8(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Vx)
#define SET_PCI8_SATA(Rx, Set)   SET_PCI8(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Set)
#define RESET_PCI8_SATA(Rx, Rt)  RESET_PCI8(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Rt)
#define RW_PCI8_SATA(Rx,St,Rt)   RW_PCI8(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, St, Rt)
#define READ_PCI16_SATA(Rx)      READ_PCI16(SATA_BUS, SATA_DEV, SATA_FUNC, Rx)
#define WRITE_PCI16_SATA(Rx, Vx) WRITE_PCI16(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Vx)
#define SET_PCI16_SATA(Rx, Set)  SET_PCI16(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Set)
#define RESET_PCI16_SATA(Rx, Rt) RESET_PCI16(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Rt)
#define RW_PCI16_SATA(Rx,St,Rt)  RW_PCI16(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, St,Rt)
#define READ_PCI32_SATA(Rx)      READ_PCI32(SATA_BUS, SATA_DEV, SATA_FUNC, Rx)
#define WRITE_PCI32_SATA(Rx, Vx) WRITE_PCI32(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Vx)
#define SET_PCI32_SATA(Rx, Set)  SET_PCI32(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Set)
#define RESET_PCI32_SATA(Rx, Rt) RESET_PCI32(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, Rt)
#define RW_PCI32_SATA(Rx,St,Rt)  RW_PCI32(SATA_BUS, SATA_DEV, SATA_FUNC, Rx, St,Rt)
//EIP131059 <<
//---------------------------------------------------------------------------

#define READ_PCI8_EHCI(Rx)       READ_PCI8(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx)
#define WRITE_PCI8_EHCI(Rx, Vx)  WRITE_PCI8(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Vx)
#define SET_PCI8_EHCI(Rx, Set)   SET_PCI8(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Set)
#define RESET_PCI8_EHCI(Rx, Rt)  RESET_PCI8(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Rt)
#define RW_PCI8_EHCI(Rx,St,Rt)   RW_PCI8(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, St, Rt)
#define READ_PCI16_EHCI(Rx)      READ_PCI16(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx)
#define WRITE_PCI16_EHCI(Rx, Vx) WRITE_PCI16(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Vx)
#define SET_PCI16_EHCI(Rx, Set)  SET_PCI16(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Set)
#define RESET_PCI16_EHCI(Rx, Rt) RESET_PCI16(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Rt)
#define RW_PCI16_EHCI(Rx,St,Rt)  RW_PCI16(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, St,Rt)
#define READ_PCI32_EHCI(Rx)      READ_PCI32(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx)
#define WRITE_PCI32_EHCI(Rx, Vx) WRITE_PCI32(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Vx)
#define SET_PCI32_EHCI(Rx, Set)  SET_PCI32(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Set)
#define RESET_PCI32_EHCI(Rx, Rt) RESET_PCI32(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, Rt)
#define RW_PCI32_EHCI(Rx,St,Rt)  RW_PCI32(EHCI_BUS, EHCI_DEV, EHCI_FUN, Rx, St,Rt)

//---------------------------------------------------------------------------
// Chipset MMIO Macros, Porting Required.
//---------------------------------------------------------------------------

#define READ_MEM8_RCRB(wReg)        READ_MEM8(SB_RCBA | wReg)
#define WRITE_MEM8_RCRB(wReg, bVal) WRITE_MEM8(SB_RCBA | wReg,bVal)
#define SET_MEM8_RCRB(wReg, Set)    RW_MEM8(SB_RCBA | wReg, Set, 0)
#define RESET_MEM8_RCRB(wReg, Rst)  RW_MEM8(SB_RCBA | wReg,0,Rst)
#define RW_MEM8_RCRB(wReg,Set,Rst)  RW_MEM8(SB_RCBA|wReg,Set,Rst)
#define READ_MEM16_RCRB(wReg)       READ_MEM16(SB_RCBA | wReg)
#define WRITE_MEM16_RCRB(wReg,Val)  WRITE_MEM16(SB_RCBA|wReg,Val)
#define SET_MEM16_RCRB(wReg, Set)   RW_MEM16(SB_RCBA|wReg, Set,0)
#define RESET_MEM16_RCRB(wReg, Rst) RW_MEM16(SB_RCBA|wReg, 0,Rst)
#define RW_MEM16_RCRB(Reg,Set,Rst)  RW_MEM16(SB_RCBA|Reg,Set,Rst)
#define READ_MEM32_RCRB(wReg)       READ_MEM32(SB_RCBA | wReg)
#define WRITE_MEM32_RCRB(wReg,Val)  WRITE_MEM32(SB_RCBA|wReg,Val)
#define SET_MEM32_RCRB(wReg,Set)    RW_MEM32(SB_RCBA|wReg, Set,0)
#define RESET_MEM32_RCRB(wReg,Rst)  RW_MEM32(SB_RCBA|wReg,0,Rst)
#define RW_MEM32_RCRB(Reg,Set,Rst)  RW_MEM32(SB_RCBA|Reg,Set,Rst)

//---------------------------------------------------------------------------
#define WRITE_MEM8_RCRB_S3(mBoot, wReg, bVal) \
                         WRITE_MEM8_S3(mBoot, SB_RCBA|wReg, bVal)
#define SET_MEM8_RCRB_S3(mBoot, wReg, Set) \
                         SET_MEM8_S3(mBoot, SB_RCBA|wReg, Set)
#define RESET_MEM8_RCRB_S3(mBoot, wReg, Rst) \
                         RESET_MEM8_S3(mBoot, SB_RCBA|wReg, Rst)
#define RW_MEM8_RCRB_S3(mBoot, wReg, Set, Rst) \
                         RW_MEM8_S3(mBoot, SB_RCBA|wReg, Set,Rst)
#define WRITE_MEM16_RCRB_S3(mBoot, wReg, wVal) \
                         WRITE_MEM16_S3(mBoot, SB_RCBA|wReg,wVal)
#define SET_MEM16_RCRB_S3(mBoot, wReg, Set) \
                         SET_MEM16_S3(mBoot, SB_RCBA|wReg, Set)
#define RESET_MEM16_RCRB_S3(mBoot, wReg, Rst) \
                         RESET_MEM16_S3(mBoot, SB_RCBA|wReg, Rst)
#define RW_MEM16_RCRB_S3(mBoot, wReg, Set, Rst) \
                         RW_MEM16_S3(mBoot,SB_RCBA|wReg, Set,Rst)
#define WRITE_MEM32_RCRB_S3(mBoot, wReg, dVal) \
                         WRITE_MEM32_S3(mBoot, SB_RCBA|wReg,dVal)
#define SET_MEM32_RCRB_S3(mBoot, wReg, Set) \
                         SET_MEM32_S3(mBoot, SB_RCBA|wReg, Set)
#define RESET_MEM32_RCRB_S3(mBoot, wReg, Rst) \
                         RESET_MEM32_S3(mBoot, SB_RCBA|wReg, Rst)
#define RW_MEM32_RCRB_S3(mBoot, wReg, Set, Rst) \
                         RW_MEM32_S3(mBoot,SB_RCBA|wReg, Set,Rst)

//---------------------------------------------------------------------------
// Chipset I/O Macros, Porting Required.
//---------------------------------------------------------------------------

#define READ_IO8_PM(bReg)           READ_IO8(PM_BASE_ADDRESS+bReg)
#define WRITE_IO8_PM(bReg, bVal)    WRITE_IO8(PM_BASE_ADDRESS+bReg, bVal)
#define SET_IO8_PM(bReg, Set)       SET_IO8(PM_BASE_ADDRESS+bReg, Set)
#define RESET_IO8_PM(bReg, Reset)   RESET_IO8(PM_BASE_ADDRESS+bReg, Reset)
#define RW_IO8_PM(bReg, Set, Rst)   RW_IO8(PM_BASE_ADDRESS+bReg, Set, Rst)
#define READ_IO16_PM(bReg)          READ_IO16(PM_BASE_ADDRESS+bReg)
#define WRITE_IO16_PM(bReg, wVal)   WRITE_IO16(PM_BASE_ADDRESS+bReg, wVal)
#define SET_IO16_PM(bReg, Set)      SET_IO16(PM_BASE_ADDRESS+bReg, Set)
#define RESET_IO16_PM(bReg, Reset)  RESET_IO16(PM_BASE_ADDRESS+bReg, Reset)
#define RW_IO16_PM(bReg, Set, Rst)  RW_IO16(PM_BASE_ADDRESS+bReg, Set, Rst)
#define READ_IO32_PM(bReg)          READ_IO32(PM_BASE_ADDRESS+bReg)
#define WRITE_IO32_PM(bReg, dVal)   WRITE_IO32(PM_BASE_ADDRESS+bReg, dVal)
#define SET_IO32_PM(bReg, Set)      SET_IO32(PM_BASE_ADDRESS+bReg, Set)
#define RESET_IO32_PM(bReg, Reset)  RESET_IO32(PM_BASE_ADDRESS+bReg, Reset)
#define RW_IO32_PM(bReg, Set, Rst)  RW_IO32(PM_BASE_ADDRESS+bReg, Set, Rst)

//---------------------------------------------------------------------------

#define READ_IO8_TCO(bReg)          READ_IO8(TCO_BASE_ADDRESS+bReg)
#define WRITE_IO8_TCO(bReg, bVal)   WRITE_IO8(TCO_BASE_ADDRESS+bReg, bVal)
#define SET_IO8_TCO(bReg, Set)      SET_IO8(TCO_BASE_ADDRESS+bReg, Set)
#define RESET_IO8_TCO(bReg, Reset)  RESET_IO8(TCO_BASE_ADDRESS+bReg, Reset)
#define RW_IO8_TCO(bReg, Set, Rst)  RW_IO8(TCO_BASE_ADDRESS+bReg, Set, Rst)
#define READ_IO16_TCO(bReg)         READ_IO16(TCO_BASE_ADDRESS+bReg)
#define WRITE_IO16_TCO(bReg, wVal)  WRITE_IO16(TCO_BASE_ADDRESS+bReg, wVal)
#define SET_IO16_TCO(bReg, Set)     SET_IO16(TCO_BASE_ADDRESS+bReg, Set)
#define RESET_IO16_TCO(bReg, Reset) RESET_IO16(TCO_BASE_ADDRESS+bReg, Reset)
#define RW_IO16_TCO(bReg, Set, Rst) RW_IO16(TCO_BASE_ADDRESS+bReg, Set, Rst)
#define READ_IO32_TCO(bReg)         READ_IO32(TCO_BASE_ADDRESS+bReg)
#define WRITE_IO32_TCO(bReg, dVal)  WRITE_IO32(TCO_BASE_ADDRESS+bReg, dVal)
#define SET_IO32_TCO(bReg, Set)     SET_IO32(TCO_BASE_ADDRESS+bReg, Set)
#define RESET_IO32_TCO(bReg, Reset) RESET_IO32(TCO_BASE_ADDRESS+bReg, Reset)
#define RW_IO32_TCO(bReg, Set, Rst) RW_IO32(TCO_BASE_ADDRESS+bReg, Set, Rst)

//---------------------------------------------------------------------------

#define WRITE_IO8_PM_S3(mBoot, bReg, bVal) \
                            WRITE_IO8_S3(mBoot, PM_BASE_ADDRESS+bReg, bVal)
#define RW_IO8_PM_S3(mBoot, bReg, Set, Reset) \
                            RW_IO8_S3(mBoot, PM_BASE_ADDRESS+bReg, Set, Reset)
#define SET_IO8_PM_S3(mBoot, bReg, Set) \
                            RW_IO8_S3(mBoot, PM_BASE_ADDRESS+bReg, Set, 0)
#define RESET_IO8_PM_S3(mBoot, bReg, Reset) \
                            RW_IO8_S3(mBoot, PM_BASE_ADDRESS+bReg, 0, Reset)

#define WRITE_IO16_PM_S3(mBoot, bReg, bVal) \
                            WRITE_IO16_S3(mBoot, PM_BASE_ADDRESS+bReg, bVal)
#define RW_IO16_PM_S3(mBoot, bReg, Set, Rst) \
                            RW_IO16_S3(mBoot, PM_BASE_ADDRESS+bReg, Set, Rst)
#define SET_IO16_PM_S3(mBoot, bReg, Set) \
                            RW_IO16_S3(mBoot, PM_BASE_ADDRESS+bReg, Set, 0)
#define RESET_IO16_PM_S3(mBoot, bReg, Reset) \
                            RW_IO16_S3(mBoot, PM_BASE_ADDRESS+bReg, 0, Reset)

#define WRITE_IO32_PM_S3(mBoot, bReg, bVal) \
                            WRITE_IO32_S3(mBoot, PM_BASE_ADDRESS+bReg, bVal)
#define RW_IO32_PM_S3(mBoot, bReg, Set, Rst) \
                            RW_IO32_S3(mBoot, PM_BASE_ADDRESS+bReg, Set, Rst)
#define SET_IO32_PM_S3(mBoot, bReg, Set) \
                            RW_IO32_S3(mBoot, PM_BASE_ADDRESS+bReg, Set, 0)
#define RESET_IO32_PM_S3(mBoot, bReg, Reset) \
                            RW_IO32_S3(mBoot, PM_BASE_ADDRESS+bReg, 0, Reset)

//---------------------------------------------------------------------------

#define WRITE_IO8_TCO_S3(mBoot, bReg, bVal) \
                            WRITE_IO8_S3(mBoot, TCO_BASE_ADDRESS+bReg, bVal)
#define RW_IO8_TCO_S3(mBoot, bReg, Set, Rst) \
                            RW_IO8_S3(mBoot, TCO_BASE_ADDRESS+bReg, Set, Rst)
#define SET_IO8_TCO_S3(mBoot, bReg, Set) \
                            RW_IO8_S3(mBoot, TCO_BASE_ADDRESS+bReg, Set, 0)
#define RESET_IO8_TCO_S3(mBoot, bReg, Reset) \
                            RW_IO8_S3(mBoot, TCO_BASE_ADDRESS+bReg, 0, Reset)

#define WRITE_IO16_TCO_S3(mBoot, bReg, bVal) \
                            WRITE_IO16_S3(mBoot, TCO_BASE_ADDRESS+bReg, bVal)
#define RW_IO16_TCO_S3(mBoot, bReg, Set, Rst) \
                            RW_IO16_S3(mBoot, TCO_BASE_ADDRESS+bReg, Set, Rst)
#define SET_IO16_TCO_S3(mBoot, bReg, Set) \
                            RW_IO16_S3(mBoot, TCO_BASE_ADDRESS+bReg, Set, 0)
#define RESET_IO16_TCO_S3(mBoot, bReg, Reset) \
                            RW_IO16_S3(mBoot, TCO_BASE_ADDRESS+bReg, 0, Reset)

#define WRITE_IO32_TCO_S3(mBoot, bReg, bVal) \
                            WRITE_IO32_S3(mBoot, TCO_BASE_ADDRESS+bReg, bVal)
#define RW_IO32_TCO_S3(mBoot, bReg, Set, Rst) \
                            RW_IO32_S3(mBoot, TCO_BASE_ADDRESS+bReg, Set, Rst)
#define SET_IO32_TCO_S3(mBoot, bReg, Set) \
                            RW_IO32_S3(mBoot, TCO_BASE_ADDRESS+bReg, Set, 0)
#define RESET_IO32_TCO_S3(mBoot, bReg, Reset) \
                            RW_IO32_S3(mBoot, TCO_BASE_ADDRESS+bReg, 0, Reset)

//---------------------------------------------------------------------------

#define READ_IO8_RTC(bReg)          ReadIo8IdxData(CMOS_ADDR_PORT, bReg)
#define WRITE_IO8_RTC(bReg, bVal)   WriteIo8IdxData(CMOS_ADDR_PORT, bReg, bVal)
#define RW_IO8_RTC(bReg, Set, Rst)  RwIo8IdxData(CMOS_ADDR_PORT, bReg, Set, Rst)
#define SET_IO8_RTC(bReg, Set)      RwIo8IdxData(CMOS_ADDR_PORT, bReg, Set, 0)
#define RESET_IO8_RTC(bReg, Reset)  RwIo8IdxData(CMOS_ADDR_PORT, bReg, 0, Reset)

//---------------------------------------------------------------------------

#define WRITE_IO8_RTC_S3(mBoot, bReg, bVal) \
                            WriteIo8IdxDataS3(mBoot, CMOS_ADDR_PORT, bReg, bVal)
#define RW_IO8_RTC_S3(mBoot, bReg, Set, Rst) \
                            RwIo8IdxDataS3(mBoot, CMOS_ADDR_PORT, bReg, Set,Rst)
#define SET_IO8_RTC_S3(mBoot, bReg, Set) \
                            RwIo8IdxDataS3(mBoot, CMOS_ADDR_PORT, bReg, Set, 0)
#define RESET_IO8_RTC_S3(mBoot, bReg, Rst) \
                            RwIo8IdxDataS3(mBoot, CMOS_ADDR_PORT, bReg, 0, Rst)

//---------------------------------------------------------------------------
//  CMOS Manager Support
//
//  Southbridge should implement functions to support access to additional 
//  CMOS banks that exist beyond the first 128 bytes.
//---------------------------------------------------------------------------

#if CMOS_MANAGER_SUPPORT
#include <CmosAccess.h>

EFI_STATUS ReadWriteCmosBank2 (
    IN EFI_PEI_SERVICES             **PeiServices,  // NULL in DXE phase
    IN CMOS_ACCESS_TYPE             AccessType,
    IN UINT16                       CmosRegister,
    IN OUT UINT8                    *CmosParameterValue
);

BOOLEAN SbGetRtcPowerStatus (
    IN EFI_PEI_SERVICES             **PeiServices  // NULL in DXE phase
);

#endif  // #if CMOS_MANAGER_SUPPORT

#ifdef __cplusplus
}
#endif
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
