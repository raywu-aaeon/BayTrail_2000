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

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************

#ifndef  _SB_H   //To Avoid this header get compiled twice
#define  _SB_H

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        Sb.h
//
// Description: This header file contains South bridge related structure and
//              constant definitions.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

//----------------------------------------------------------------------------

#define SB_PCI_CFG_ADDRESS(bus,dev,func,reg)    \
 ((UINT64) ( (((UINTN)bus) <<   24) +   (((UINTN)dev)   << 16) + (((UINTN)func) << 8)   +   ((UINTN)reg) ))& 0x00000000ffffffff

//EIP128605 >>
//----------------------------------------------------------------------------
// INTEL PCH PCI Bus Number Equates
//----------------------------------------------------------------------------
#define SB_BUS                  0       // South Bridge Bus Number
#define LPC_BUS                 SB_BUS
#define SMBUS_BUS				0		// SMBUS Bus Number
#define LPSS_SPI_BUS			0		// LPSS SPI Bus Number
#define LPSS_HSUART1_BUS		0		// LPSS HSUART 1 Bus Number
#define LPSS_HSUART0_BUS		0		// LPSS HSUART 0 Bus Number
#define LPSS_PWM1_BUS			0		// LPSS PWM 1 Bus Number
#define LPSS_PWM0_BUS			0		// LPSS PWM 0 Bus Number
#define LPSS_DMAC1_BUS			0		// LPSS DMAC 1 Bus Number 
#define LPSS_DMAC0_BUS			0		// LPSS DMAC 0 Bus Number
#define LPSS_I2C6_BUS			0		// LPSS_I2C 6 Bus Number
#define LPSS_I2C5_BUS			0		// LPSS_I2C 5 Bus Number
#define LPSS_I2C4_BUS			0		// LPSS_I2C 4 Bus Number
#define LPSS_I2C3_BUS			0		// LPSS_I2C 3 Bus Number
#define LPSS_I2C2_BUS			0		// LPSS_I2C 2 Bus Number
#define LPSS_I2C1_BUS			0		// LPSS_I2C 1 Bus Number
#define LPSS_I2C0_BUS			0		// LPSS_I2C 0 Bus Number
#define XHCI_BUS                0       // XHCI Controller Bus Number
#define EHCI_BUS                0       // EHCI Controller Bus Number
#define SCC_SDIO3_BUS			0		// SCC HSI Bus Number
#define OTG_BUS					0		// OTG Bus Number
#define LPE_BUS					0		// LPE Bus Number
#define SCC_SDIO2_BUS			0		// SCC SDIO 2 Bus Number
#define SCC_SDIO1_BUS			0		// SCC SDIO 1 Bus Number
#define SCC_SDIO0_BUS			0		// SCC SDIO 0 Bus Number
#define PCIE_ROOT_P0_BUS		0		// PCIE ROOT PORT0 Bus Number
#define PCIE_ROOT_P1_BUS		0		// PCIE ROOT PORT1 Bus Number
#define PCIE_ROOT_P2_BUS		0		// PCIE ROOT PORT2 Bus Number
#define PCIE_ROOT_P3_BUS		0		// PCIE ROOT PORT3 Bus Number
#define AZALIA_BUS				0		// AZALIA Bus Number
#define SEC_DEVICE_BUS			0		// SEC DEVICE Bus Number
//#define LAN_BUS				0		// Ethernet GBE Controller Bus Number
#define SATA_BUS				0		// SATA Bus Number

//----------------------------------------------------------------------------
// INTEL PCH PCI Device Number Equates
//----------------------------------------------------------------------------
#define SB_DEV                  0x1f    // South Bridge Device Number
#define LPC_DEVICE              SB_DEV
#define SMBUS_DEV				SB_DEV	// SMBUS Device Number
#define LPSS_SPI_DEV			0x1d	// LPSS SPI Device Number
#define LPSS_HSUART_DEV			0x1d	// LPSS HSUART Device Number
#define LPSS_PWM_DEV			0x1d	// LPSS PWM Device Number
#define LPSS_DMAC1_DEV			0x18	// LPSS_DMAC 1 Device Number
#define LPSS_DMAC0_DEV			0x1d	// LPSS_DMAC 0 Device Number
#define LPSS_I2C_DEV			0x18	// LPSS_I2C Device Number
#define XHCI_DEV                0x14    // XHCI Controller Device Number
#define EHCI_DEV                0x1d    // EHCI Controller Device Number
#define SCC_SDIO3_DEV			0x17	// SCC SDI3 Device Number
#define OTG_DEV					0x16	// OTG Device Number
#define LPE_DEV					0x15	// LPE Device Number
#define SCC_SDIO2_DEV			0x12	// SCC SDI2 Device Number
#define SCC_SDIO1_DEV			0x11	// SCC SDI1 Device Number
#define SCC_SDIO0_DEV			0x10	// SCC SDIO Device Number
#define PCIE_ROOT_DEV			0x1c	// PCIE ROOT Device Number
#define AZALIA_DEV				0x1b	// AZALIA Device Number
#define SEC_DEVICE_DEV			0x1a	// SEC DEVICE Device Number
//#define LAN_DEV				0x19	// Ethernet GBE Controller Device Number
#define SATA_DEV				0x13	// SATA Device Number

//----------------------------------------------------------------------------
// INTEL PCH PCI Function Number Equates
//----------------------------------------------------------------------------
#define SB_FUN                  0x00    // South Bridge Function Number
#define LPC_FUNC                SB_FUN	
#define SMBUS_FUNC				0x03	// SMBUS Function Number 
#define LPSS_SPI_FUNC			0x05	// LPSS SPI  Function Number
#define LPSS_HSUART1_FUNC		0x04	// LPSS HSUART 1 Function Number
#define LPSS_HSUART0_FUNC		0x03	// LPSS HSUART 0 Function Number	 
#define LPSS_PWM1_FUNC			0x02	// LPSS PWM 1 Function Number
#define LPSS_PWM0_FUNC			0x01	// LPSS PWM 1 Function Number
#define LPSS_DMAC1_FUNC			0x00	// LPSS_DMAC 1 Function Number
#define LPSS_DMAC0_FUNC			0x00	// LPSS_DMAC 0 Function Number
#define LPSS_I2C6_FUNC			0x07	// LPSS_I2C 6 Function Number
#define LPSS_I2C5_FUNC			0x06	// LPSS_I2C 5 Function Number
#define LPSS_I2C4_FUNC			0x05	// LPSS_I2C 4 Function Number
#define LPSS_I2C3_FUNC			0x04	// LPSS_I2C 3 Function Number
#define LPSS_I2C2_FUNC			0x03	// LPSS_I2C 2 Function Number
#define LPSS_I2C1_FUNC			0x02	// LPSS_I2C 1 Function Number
#define LPSS_I2C0_FUNC			0x01	// LPSS_I2C 0 Function Number
#define XHCI_FUN                0x00    // XHCI Controller Function Number
#define EHCI_FUN                0x00    // EHCI Controller Function Number
#define SCC_SDIO3_FUNC			0x00	// SCC SDI3 Function Number 
#define OTG_FUNC				0x00	// OTG Function Number 
#define LPE_FUNC				0x00	// LPE Function Number
#define SCC_SDIO2_FUNC			0x00	// SCC SDI2 Function Number 
#define SCC_SDIO1_FUNC			0x00	// SCC SDI1 Function Number 
#define SCC_SDIO0_FUNC			0x00	// SCC SDI0 Function Number 
#define PCIE_ROOT_P0_FUNC		0x00	// PCIE ROOT PORT0 Function Number
#define PCIE_ROOT_P1_FUNC		0x01	// PCIE ROOT PORT1 Function Number 
#define PCIE_ROOT_P2_FUNC		0x02	// PCIE ROOT PORT2 Function Number 
#define PCIE_ROOT_P3_FUNC		0x03	// PCIE ROOT PORT3 Function Number
#define AZALIA_FUNC				0x00	// AZALIA Function Number
#define SEC_DEVICE_FUNC			0x00	// SEC DEVICE Function Number
//#define LAN_FUNC				0x00	// Ethernet GBE Controller Function Number
#define SATA_FUNC				0x00	// SATA Function Number
//EIP128605 <<

#define NUM_BITS_IN_ACPI_TIMER      24      // Programmed to 24 not 32
// This is the maximum possible bits in the timer.  Currently this is 32 according to the spec
#define MAX_ACPI_TIMER_BITS         32

#define RESET_PORT              0x0CF9
#define CLEAR_RESET_BITS        0x0F1
#define COLD_RESET              0x02    // Set bit 1 for cold reset
#define RST_CPU                 0x04    // Setting this bit triggers a reset of the CPU
#define FULL_RESET              0x08    // Set bit 3 for full reset

// Define all the Southbridge specific equates and structures in this file
//
// 8259 Hardware definitions
//
#define LEGACY_MODE_BASE_VECTOR_MASTER                    0x08
#define LEGACY_MODE_BASE_VECTOR_SLAVE                     0x10
#define LEGACY_8259_CONTROL_REGISTER_MASTER               0x20
#define LEGACY_8259_MASK_REGISTER_MASTER                  0x21
#define LEGACY_8259_CONTROL_REGISTER_SLAVE                0xA0
#define LEGACY_8259_MASK_REGISTER_SLAVE                   0xA1
#define LEGACY_8259_EDGE_LEVEL_TRIGGERED_REGISTER_MASTER  0x4D0
#define LEGACY_8259_EDGE_LEVEL_TRIGGERED_REGISTER_SLAVE   0x4D1
#define LEGACY_8259_EOI                                   0x20

#define REFRESH_PORT        0x61
#define REFRESH_ON          0x10
#define REFRESH_OFF         0x00

#define LPC_BUS_DEV_FUNC    SB_PCI_CFG_ADDRESS(LPC_BUS, LPC_DEVICE, LPC_FUNC, 0)


#define REG_ACPI_BASE_ADDRESS   0x40
#define REG_ACPI_BASE_ADDRESS1  0x41
#define REG_ACPI_CNTL           0x44
#define ACPI_ENABLE             0x80
#define REG_GPIO_BASE_ADDRESS   0x48
#define REG_GPIO_BASE_ADDRESS1  0x49
#define REG_GPIO_CNTL           0x4c
#define GPIO_ENABLE             0x10

// ACPI I/O register offsets
#define R_PM1_STS               0x00
#define R_SMI_EN                0x30
#define R_SMI_STS               0x34
#define IO_REG_ACPI_TIMER       0x08

#define TCO_BASE PM_BASE_ADDRESS + 0x60

#define  REG_TCO2_STS           0x06
#define   BIT_SECOND_TO_STS       0x0002

#define  REG_TCO1_CNT           0x08
#define   BIT_TCO_TMR_HLT         0x0800
#define   BIT_NMI2SMI_EN          0x0200

#define  R_NMI_SC               0x61
#define   B_NMI_SC_IOCHK_NMI_EN   0x08
#define   B_NMI_SC_PCI_SERR_EN    0x04

#define KBC_IO_DATA             0x60    // Keyboard Controller Data Port
#define KBC_IO_STS              0x64    // Keyboard Controller Status Port

//
// PCI Register Definitions - use SmbusPolicyPpi->PciAddress + offset listed below
//
#define R_COMMAND                     0x04      // PCI Command Register, 16bit
#define   B_IOSE                        0x01    // RW
#define R_PCH_SMBUS_BAR0              0x10  	// The memory bar low
#define R_PCH_SMBUS_BAR1              0x14  	// The memory bar high
#define R_BASE_ADDRESS                0x20      // PCI BAR for SMBus I/O
#define   B_BASE_ADDRESS                0xFFE0  // RW
#define R_HOST_CONFIGURATION          0x40      // SMBus Host Configuration Register
#define   B_HST_EN                      0x01    // RW
#define   B_SMB_SMI_EN                  0x02    // RW
#define   B_I2C_EN                      0x04    // RW


VOID SaveRestoreRegisters(BOOLEAN Save);


#define ONE_SECOND  1000000     // The stall PPI is defined in microseconds
                                // this should be one second in microseconds
//----------------------------------------------------------------------------
// INTEL ValleyView SPI Registers (Porting Required)
//----------------------------------------------------------------------------
// Note: The following definitions are used by SPIFlash.c
#define R_RCRB_SPI_PREOP              0x94        // Prefix Opcode Configuration
#define R_RCRB_SPI_OPTYPE             0x96        // Opcode Type Configuration
#define R_RCRB_SPI_OPMENU             0x98        // Opcode Menu Configuration
//EIP167096 >>
#define R_SB_SPI_FREG0_FLASHD         0x54        // Flash Region 0 (Flash Descriptor) (32bits)
#define R_SB_SPI_FREG1_BIOS           0x58        // Flash Region 1 (BIOS) (32bits)
#define R_SB_SPI_FREG2_SEC            0x5C        // Flash Region 2 (SEC) (32bits)
#define B_SB_SPI_FREGX_LIMIT_MASK     0x1FFF0000  // Size, [28:16] here represents limit[24:12]
#define B_SB_SPI_FREGX_BASE_MASK      0x00001FFF  // Base, [12:0]  here represents base [24:12]
#define R_SB_SPI_PR0                  0x74        // Protected Region 0 Register
#define B_SB_SPI_PRx_WPE              BIT31       // Write Protection Enable
#define B_SB_SPI_PRx_RPE              BIT15       // Read Protection Enable
#define R_SB_SPI_FDOC                 0xB0        // Flash Descriptor Observability Control Register (32 bits)
#define B_SB_SPI_FDOC_FDSS_MASK       (BIT14 | BIT13 | BIT12) // Flash Descriptor Section Select
#define V_SB_SPI_FDOC_FDSS_FSDM       0x0000      // Flash Signature and Descriptor Map
#define V_SB_SPI_FDOC_FDSS_COMP       0x1000      // Component
#define B_SB_SPI_FDOC_FDSI_MASK       0x0FFC      // Flash Descriptor Section Index
#define R_SB_SPI_FDOD                 0xB4        // Flash Descriptor Observability Data Register (32 bits)
#define R_SB_SPI_FDBAR_FLASH_MAP0     0x04        // Flash MAP 0
#define B_SB_SPI_FDBAR_NC             0x00000300  // Number Of Components
#define V_SB_SPI_FDBAR_NC_2           0x00000100
#define V_SB_SPI_FDBAR_NC_1           0x00000000
#define R_SB_SPI_FCBA_FLCOMP          0x00        // Flash Components Register
#define B_SB_SPI_FLCOMP_COMP2_MASK    0x38        // Flash Component 2 Density
#define V_SB_SPI_FLCOMP_COMP2_512KB   0x00
#define V_SB_SPI_FLCOMP_COMP2_1MB     0x08
#define V_SB_SPI_FLCOMP_COMP2_2MB     0x10
#define V_SB_SPI_FLCOMP_COMP2_4MB     0x18
#define V_SB_SPI_FLCOMP_COMP2_8MB     0x20
#define V_SB_SPI_FLCOMP_COMP2_16MB    0x28
#define B_SB_SPI_FLCOMP_COMP1_MASK    0x07        // Flash Component 1 Density
#define V_SB_SPI_FLCOMP_COMP1_512KB   0x00
#define V_SB_SPI_FLCOMP_COMP1_1MB     0x01
#define V_SB_SPI_FLCOMP_COMP1_2MB     0x02
#define V_SB_SPI_FLCOMP_COMP1_4MB     0x03
#define V_SB_SPI_FLCOMP_COMP1_8MB     0x04
#define V_SB_SPI_FLCOMP_COMP1_16MB    0x05
//EIP167096 <<
//----------------------------------------------------------------------------
// Misc. I/O Registers
//----------------------------------------------------------------------------
#define PCI_DEBUG_PORT          0x80
#define CMOS_ADDR_PORT          0x70
#define CMOS_DATA_PORT          0x71
#define CMOS_IO_EXT_INDEX       0x72    // CMOS I/O Extended Index Port
#define CMOS_IO_EXT_DATA        0x73    // CMOS I/O Extended Data Port
#define IO_DELAY_PORT           0xed    // Use for I/O delay

#endif  // #ifndef  _SB_H_

//(EIP131491+)>>
//----------------------------------------------------------------------------
// INTEL PCH SATA Controller Registers
//----------------------------------------------------------------------------
#define SATA_REG_DEVID          0x02    // Device ID Reg.
#define SATA_REG_PCICMD         0x04    // Command Register
#define SATA_REG_RID            0x08    // Revision ID Reg.
#define SATA_REG_PCIPI          0x09    // Programming Interface Register
#define SATA_REG_MLT            0x0d    // Primary Master Latnecy Timer Reg.
#define SATA_REG_PCMD_BAR       0x10    // Primary Command Block Base Address Register
#define SATA_REG_PCNL_BAR       0x14    // Primary Control Block Base Address Register
#define SATA_REG_SCMD_BAR       0x18    // Secondary Command Block Base Address Register
#define SATA_REG_SCNL_BAR       0x1C    // Secondary Control Block Base Address Register
#define SATA_REG_BM_BASE        0x20    // Bus Master Base Address Register
#define SATA_REG_ABAR           0x24    // AHCI Base Address Register
#define SATA_REG_SVID           0x2C    // Sub-Vendor/SubSystem IDs register
#define SATA_REG_INTR_LN        0x3C    // Interrupt Line Register
#define SATA_REG_IDETIM         0x40    // Primary & Secondary drive timings register
#define SATA_REG_SIDETIM        0x44    // Slave Primary & Secondary drive timings register
#define SATA_REG_SDMACTL        0x48    // Synchronous DMA Control register
#define SATA_REG_SDMATIM        0x4A    // Synchronous DMA Timing register
#define SATA_REG_IDE_CONFIG     0x54    // IDE I/O Configuration register
#define SATA_REG_PID            0x70    // PCI Power Management Capability ID register
#define SATA_REG_PC             0x72    // PCI Power Management Capability register
#define SATA_REG_PMCS           0x74    // PCI Power Management Control & Status register
#define SATA_REG_MSICI          0x80    // Message Signaled Interrupt Identifiers register
#define SATA_REG_MSIMC          0x82    // Message Signaled Interrupt Message Control register
#define SATA_REG_MSIMA          0x84    // Message Signaled Interrupt Message Address register
#define SATA_REG_MSIMD          0x88    // Message Signaled Interrupt Message Data register
#define SATA_REG_MAP            0x90    // Address Map register
#define SATA_REG_PCS            0x92    // Port Status & Control register
#define SATA_REG_SIR            0x94    // Initialization register
#define SATA_REG_SIRI           0xA0    // S-ATA Register Index register
#define SATA_REG_STRD           0xA4    // S-ATA Register Data register
//(EIP131491+)<<

//(P052813A+)>>
//----------------------------------------------------------------------------
// INTEL PCH GP I/O Register Equates
//----------------------------------------------------------------------------
#define GP_IOREG_USE_SEL        0x00    // GPIO Use Select register
#define GP_IOREG_IO_SEL         0x04    // GPIO Input/Output select
#define GP_IOREG_GP_LVL         0x08    // GPIO Level for Input/Ouput
#define GP_IOREG_TPE            0x0C    // GPIO Trigger Positive Edge Enable
#define GP_IOREG_TNE            0x10    // GPIO Trigger Negative Enable
#define GP_IOREG_TS             0x14    // GPIO Trigger Status
#define GP_IOREG_WE             0x18    // GPIO Wake Enable
//(P052813A+)<<

//EIP128605 >>
//----------------------------------------------------------------------------
// INTEL PCH PCI Bus/Device/Function/Register Number Macros
//----------------------------------------------------------------------------
//SMBUS
#define SMBUS_REG(Reg)          CSP_PCIE_CFG_ADDRESS(SMBUS_BUS, SMBUS_DEV, SMBUS_FUNC, Reg)
//LPC
#define SB_REG(Reg)             CSP_PCIE_CFG_ADDRESS(LPC_BUS, LPC_DEVICE, LPC_FUNC, Reg)
//LPSS SPI
#define LPSS_SPI_REG(Reg)       CSP_PCIE_CFG_ADDRESS(LPSS_SPI_BUS, LPSS_SPI_DEV, LPSS_SPI_FUNC, Reg)
//LPSS HSUART 1
#define LPSS_HSUART1_REG(Reg)   CSP_PCIE_CFG_ADDRESS(LPSS_HSUART1_BUS, LPSS_HSUART_DEV, LPSS_HSUART1_FUNC, Reg)
//LPSS HSUART 0
#define LPSS_HSUART0_REG(Reg)   CSP_PCIE_CFG_ADDRESS(LPSS_HSUART0_BUS, LPSS_HSUART_DEV, LPSS_HSUART0_FUNC, Reg)
//LPSS PWM 1
#define LPSS_PWM1_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_PWM1_BUS, LPSS_PWM_DEV, LPSS_PWM1_FUNC, Reg)
//LPSS PWM 0
#define LPSS_PWM0_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_PWM0_BUS, LPSS_PWM_DEV, LPSS_PWM0_FUNC, Reg)
//LPSS DMAC1
#define LPSS_DMAC1_REG(Reg)     CSP_PCIE_CFG_ADDRESS(LPSS_DMAC1_BUS, LPSS_DMAC1_DEV, LPSS_DMAC1_FUNC, Reg)
//LPSS DMAC0
#define LPSS_DMAC0_REG(Reg)     CSP_PCIE_CFG_ADDRESS(LPSS_DMAC0_BUS, LPSS_DMAC0_DEV, LPSS_DMAC0_FUNC, Reg)
//LPSS_I2C 6
#define LPSS_I2C6_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C6_BUS, LPSS_I2C_DEV, LPSS_I2C6_FUNC, Reg)
//LPSS_I2C 5
#define LPSS_I2C5_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C5_BUS, LPSS_I2C_DEV, LPSS_I2C5_FUNC, Reg)
//LPSS_I2C 4
#define LPSS_I2C4_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C4_BUS, LPSS_I2C_DEV, LPSS_I2C4_FUNC, Reg)
//LPSS_I2C 3
#define LPSS_I2C3_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C3_BUS, LPSS_I2C_DEV, LPSS_I2C3_FUNC, Reg)
//LPSS_I2C 2
#define LPSS_I2C2_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C2_BUS, LPSS_I2C_DEV, LPSS_I2C2_FUNC, Reg)
//LPSS_I2C 1
#define LPSS_I2C1_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C1_BUS, LPSS_I2C_DEV, LPSS_I2C1_FUNC, Reg)
//LPSS_I2C 0
#define LPSS_I2C0_REG(Reg)      CSP_PCIE_CFG_ADDRESS(LPSS_I2C0_BUS, LPSS_I2C_DEV, LPSS_I2C0_FUNC, Reg)
//USB EHCI
#define USB_EHCI_REG(Reg)       CSP_PCIE_CFG_ADDRESS(EHCI_BUS, EHCI_DEV, EHCI_FUN, Reg)
//USB XHCI
#define USB_XHCI_REG(Reg)       CSP_PCIE_CFG_ADDRESS(XHCI_BUS, XHCI_DEV, XHCI_FUN, Reg)
//SCC HSI
#define SCC_SDIO3_REG(Reg)      CSP_PCIE_CFG_ADDRESS(SCC_SDIO3_BUS, SCC_SDIO3_DEV, SCC_SDIO3_FUNC, Reg)
//OTG
#define OTG_REG(Reg)            CSP_PCIE_CFG_ADDRESS(OTG_BUS, OTG_DEV, OTG_FUNC, Reg)
//LPE
#define LPE_REG(Reg)            CSP_PCIE_CFG_ADDRESS(LPE_BUS, LPE_DEV, LPE_FUNC, Reg)
//SCC SDIO 2
#define SCC_SDIO2_REG(Reg)      CSP_PCIE_CFG_ADDRESS(SCC_SDIO2_BUS, SCC_SDIO2_DEV, SCC_SDIO2_FUNC, Reg)
//SCC SDIO 1
#define SCC_SDIO1_REG(Reg)      CSP_PCIE_CFG_ADDRESS(SCC_SDIO1_BUS, SCC_SDIO1_DEV, SCC_SDIO1_FUNC, Reg)
//SCC SDIO 0
#define SCC_SDIO0_REG(Reg)      CSP_PCIE_CFG_ADDRESS(SCC_SDIO0_BUS, SCC_SDIO0_DEV, SCC_SDIO0_FUNC, Reg)
//PCIE ROOT PORT0
#define PCIE_ROOT_P0_REG(Reg)   CSP_PCIE_CFG_ADDRESS(PCIE_ROOT_P0_BUS, PCIE_ROOT_DEV, PCIE_ROOT_P0_FUNC, Reg)
//PCIE ROOT PORT1
#define PCIE_ROOT_P1_REG(Reg)   CSP_PCIE_CFG_ADDRESS(PCIE_ROOT_P1_BUS, PCIE_ROOT_DEV, PCIE_ROOT_P1_FUNC, Reg)
//PCIE ROOT PORT2
#define PCIE_ROOT_P2_REG(Reg)   CSP_PCIE_CFG_ADDRESS(PCIE_ROOT_P2_BUS, PCIE_ROOT_DEV, PCIE_ROOT_P2_FUNC, Reg)
//PCIE ROOT PORT3
#define PCIE_ROOT_P3_REG(Reg)   CSP_PCIE_CFG_ADDRESS(PCIE_ROOT_P3_BUS, PCIE_ROOT_DEV, PCIE_ROOT_P3_FUNC, Reg)
//AZALIA
#define AZALIA_REG(Reg)         CSP_PCIE_CFG_ADDRESS(AZALIA_BUS, AZALIA_DEV, AZALIA_FUNC, Reg)
//SEC DEVICE
#define SEC_DEVICE_REG(Reg)     CSP_PCIE_CFG_ADDRESS(SEC_DEVICE_BUS, SEC_DEVICE_DEV, SEC_DEVICE_FUNC, Reg)
//LAN
//#define LAN_REG(Reg)            CSP_PCIE_CFG_ADDRESS(LAN_BUS, LAN_DEV, LAN_FUNC, Reg)
//SATA
#define SATA_REG(Reg)           CSP_PCIE_CFG_ADDRESS(SATA_BUS, SATA_DEV, SATA_FUNC, Reg)
//EIP128605 <<

//----------------------------------------------------------------------------
// INTEL PCH PCI Bus/Device/Function Number Macros
//----------------------------------------------------------------------------
#define SMBUS_BUS_DEV_FUN           	SMBUS_REG(0)
#define SB_BUS_DEV_FUN		 		    SB_REG(0)
#define LPSS_SPI_BUS_DEV_FUN		    LPSS_SPI_REG(0)
#define LPSS_HSUART1_BUS_DEV_FUN	  	LPSS_HSUART1_REG(0)
#define LPSS_HSUART0_BUS_DEV_FUN	  	LPSS_HSUART0_REG(0)
#define LPSS_PWM1_BUS_DEV_FUN		    LPSS_PWM1_REG(0)
#define LPSS_PWM0_BUS_DEV_FUN		    LPSS_PWM0_REG(0)
#define LPSS_DMAC1_BUS_DEV_FUN		  	LPSS_DMAC1_REG(0)
#define LPSS_DMAC0_BUS_DEV_FUN		  	LPSS_DMAC0_REG(0)
#define LPSS_I2C6_BUS_DEV_FUN		    LPSS_I2C6_REG(0)
#define LPSS_I2C5_BUS_DEV_FUN		    LPSS_I2C5_REG(0)
#define LPSS_I2C4_BUS_DEV_FUN		    LPSS_I2C4_REG(0)
#define LPSS_I2C3_BUS_DEV_FUN		    LPSS_I2C3_REG(0)
#define LPSS_I2C2_BUS_DEV_FUN		    LPSS_I2C2_REG(0)
#define LPSS_I2C1_BUS_DEV_FUN		    LPSS_I2C1_REG(0)
#define LPSS_I2C0_BUS_DEV_FUN		    LPSS_I2C0_REG(0)
#define USB_EHCI_BUS_DEV_FUN		    USB_EHCI_REG(0)
#define USB_XHCI_BUS_DEV_FUN		    USB_XHCI_REG(0)
#define SCC_SDIO3_BUS_DEV_FUN       	SCC_SDIO3_REG(0)
#define OTG_BUS_DEV_FUN				    OTG_REG(0)
#define LPE_BUS_DEV_FUN				    LPE_REG(0)
#define SCC_SDIO2_BUS_DEV_FUN		    SCC_SDIO2_REG(0)
#define SCC_SDIO1_BUS_DEV_FUN		    SCC_SDIO1_REG(0)
#define SCC_SDIO0_BUS_DEV_FUN		    SCC_SDIO0_REG(0)
#define PCIE_ROOT_P0_BUS_DEV_FUN    	PCIE_ROOT_P0_REG(0)
#define PCIE_ROOT_P1_BUS_DEV_FUN    	PCIE_ROOT_P1_REG(0)
#define PCIE_ROOT_P2_BUS_DEV_FUN    	PCIE_ROOT_P2_REG(0)
#define PCIE_ROOT_P3_BUS_DEV_FUN    	PCIE_ROOT_P3_REG(0)
#define AZALIA_BUS_DEV_FUN          	AZALIA_REG(0)
#define SEC_DEVICE_BUS_DEV_FUN      	SEC_DEVICE_REG(0)
//#define LAN_BUS_DEV_FUN             	LAN_REG(0)
#define SATA_BUS_DEV_FUN            	SATA_REG(0)

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

