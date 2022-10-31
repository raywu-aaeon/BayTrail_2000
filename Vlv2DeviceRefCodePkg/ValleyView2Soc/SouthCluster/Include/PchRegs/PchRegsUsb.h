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
  PchRegsUsb.h

  @brief
  Register names for PCH USB devices.
  
  Conventions:
  
  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, PCH registers are denoted by "_PCH_" in register names
  - Registers / bits that are different between PCH generations are denoted by 
    "_PCH_<generation_name>_" in register/bit names. e.g., "_PCH_VLV_"
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a PCH generation will be just named 
    as "_PCH_" without <generation_name> inserted.

**/
#ifndef _PCH_REGS_USB_H_
#define _PCH_REGS_USB_H_

///
/// USB Definitions
///

typedef enum {
  PchEhci1 = 0,
  PchEhciControllerMax
} PCH_USB20_CONTROLLER_TYPE;

#define PCH_USB_MAX_PHYSICAL_PORTS          4      /// Max Physical Connector EHCI + XHCI, not counting virtual ports like USB-R.
#define PCH_EHCI_MAX_PORTS                  4      /// Counting ports behind RMHs 8 from EHCI-1 and 6 from EHCI-2, not counting EHCI USB-R virtual ports.
#define PCH_HSIC_MAX_PORTS                  2
#define PCH_XHCI_MAX_USB3_PORTS             1

#define PCI_DEVICE_NUMBER_PCH_USB           29
#define PCI_FUNCTION_NUMBER_PCH_EHCI        0

#define R_PCH_USB_VENDOR_ID                 0x00  // Vendor ID
#define V_PCH_USB_VENDOR_ID                 V_PCH_INTEL_VENDOR_ID

#define R_PCH_USB_DEVICE_ID                 0x02  // Device ID
#define V_PCH_USB_DEVICE_ID_0               0x0F34  // EHCI#1

#define R_PCH_EHCI_COMMAND_REGISTER         0x04  // Command
#define B_PCH_EHCI_INTRSTS_0                BIT19  // Interrupt Status(RO)
#define B_PCH_EHCI_COMMAND_INTR_DIS         BIT10 // Interrupt Disable
#define B_PCH_EHCI_COMMAND_FBE              BIT9  // Fast Back to Back Enable: Reserved as '0'
#define B_PCH_EHCI_COMMAND_SERR_EN          BIT8  // SERR# Enable
#define B_PCH_EHCI_COMMAND_WCC              BIT7  // Wait Cycle Enable: Reserved as '0'
#define B_PCH_EHCI_COMMAND_PER              BIT6  // Parity Error Response
#define B_PCH_EHCI_COMMAND_VPS              BIT5  // VGA Pallete Snoop: Reserved as '0'
#define B_PCH_EHCI_COMMAND_PMWE             BIT4  // Post Memory Write Enable: Reserved as '0'
#define B_PCH_EHCI_COMMAND_SCE              BIT3  // Special Cycle Enable: Reserved as '0'
#define B_PCH_EHCI_COMMAND_BME              BIT2  // Bus Master Enable
#define B_PCH_EHCI_COMMAND_MSE              BIT1  // Memory Space Enable
#define B_PCH_EHCI_COMMAND_IOSE             BIT0  // I/O Space Enable: Reserved as '0', R/O

#define R_PCH_EHCI_PCISTS                   0x06  // Device Status
#define B_PCH_EHCI_PCISTS_DPE               BIT15 // Detected Parity Error
#define B_PCH_EHCI_PCISTS_SSE               BIT14 // Signaled System Error
#define B_PCH_EHCI_PCISTS_RMA               BIT13 // Received Master-Abort Status
#define B_PCH_EHCI_PCISTS_RTA               BIT12 // Received Target Abort Status
#define B_PCH_EHCI_PCISTS_STA               BIT11 // Signaled Target-Abort Status
#define B_PCH_EHCI_PCISTS_DEV_STS           (BIT10 | BIT9) // DEVSEL# Timing Status
#define B_PCH_EHCI_PCISTS_DPED              BIT8  // Master Data Parity Error Detected
#define B_PCH_EHCI_PCISTS_FB2BC             BIT7  // Fast Back-to-Back Capable: Reserved as '1'
#define B_PCH_EHCI_PCISTS_UDF               BIT6  // User Definable Features: Reserved as '0'
#define B_PCH_EHCI_PCISTS_66MHZ_CAP         BIT5  // 66 MHz Capable: Reserved as '0'
#define B_PCH_EHCI_PCISTS_CAP_LST           BIT4  // Capabilities List
#define B_PCH_EHCI_PCISTS_INTR_STS          BIT3  // Interrupt Status

#define R_PCH_EHCI_RID                      0x08  // Revision ID
#define B_PCH_EHCI_RID                      0xFF  // Revision ID Mask

#define R_PCH_EHCI_PI                       0x09  // Programming Interface
#define B_PCH_EHCI_PI                       0xFF  // Programming Interface Mask

#define R_PCH_EHCI_SCC                      0x0A  // Sub Class Code
#define B_PCH_EHCI_SCC                      0xFF  // Sub Class Code Mask

#define R_PCH_EHCI_BCC                      0x0B  // Base Class Code
#define B_PCH_EHCI_BCC                      0xFF  // Base Class Code Mask

#define R_PCH_EHCI_MLT                      0x0D  // Master Latency Timer
#define B_PCH_EHCI_MLT                      0xFF  // Master Latency Timer Mask

#define R_PCH_EHCI_HEADTYPE                 0x0E  // Header Type
#define B_PCH_EHCI_HEADTYPE                 0xFF  // Header Type Mask
#define B_PCH_EHCI_HEADTYPE_MFB             BIT7  // Multi-Function Bit
#define B_PCH_EHCI_HEADTYPE_CL              0x7F  // Configuration Layout

#define R_PCH_EHCI_MEM_BASE                 0x10  // Memory Base Address
#define B_PCH_EHCI_MEM_BASE_BAR             0xFFFFFC00 // Base Address
#define V_PCH_EHCI_MEM_LENGTH               0x400 // Memory Space
#define N_PCH_EHCI_MEM_ALIGN                10    // Memory Space Alignment
#define B_PCH_EHCI_MEM_BASE_PREF            BIT3  // Prefetchable
#define B_PCH_EHCI_MEM_BASE_TYPE            (BIT2 | BIT1) // Type
#define B_PCH_EHCI_MEM_BASE_RTE             BIT0  // Resource Type Indicator

#define R_PCH_EHCI_SVID                     0x2C  // USB2 Subsystem Vendor ID
#define B_PCH_EHCI_SVID                     0xFFFF // USB2 Subsystem Vendor ID Mask

#define R_PCH_EHCI_SID                      0x2E  // USB2 Subsystem ID
#define B_PCH_EHCI_SID                      0xFFFF // USB2 Subsystem ID Mask

#define R_PCH_EHCI_CAP_PTR                  0x34  // Capabilities Pointer
#define B_PCH_EHCI_CAP_PTR                  0xFF  // Capabilities Pointer Mask

#define R_PCH_EHCI_INT_LN                   0x3C  // Interrupt Line
#define B_PCH_EHCI_INT_LN                   0xFF  // Interrupt Line Mask

#define R_PCH_EHCI_INT_PN                   0x3D  // Interrupt Pin
#define B_PCH_EHCI_INT_PN                   0xFF  // Interrupt Pin Mask

#define R_PCH_EHCI_IHFCLK                   0x44  // Intel-Specific High Precision Frame Clock
#define B_PCH_EHCI_IHFCLK_MFI               0x3FFF0000 // Frame List Current Index / Frame Number
#define B_PCH_EHCI_IHFCLK_MFB               0x1FFF // Micro-frame Blif (MFB)

#define R_PCH_EHCI_IHFCLKC                  0x48  // Intel-Specific High Precision Frame Clock Capture
#define B_PCH_EHCI_IHFCLKC_CMFI             0x3FFF0000 // Captured Frame List Current Index / Frame Number
#define B_PCH_EHCI_IHFCLKC_CMFB             0x1FFF // Captured Micro-frame Blif (MFB)

#define R_PCH_EHCI_PWR_CAPID                0x50  // PCI Power management Capability ID
#define B_PCH_EHCI_PWR_CAPID                0xFF  // PCI Power management Capability ID Mask

#define R_PCH_EHCI_NXT_PTR1                 0x51  // Next Item Pointer #1
#define B_PCH_EHCI_NXT_PTR1                 0xFF  // Next Item Pointer #1 Mask

#define R_PCH_EHCI_PWR_CAP                  0x52  // Power Management Capabilities
#define B_PCH_EHCI_PWR_CAP_PME_SUP          0xF800  // PME Support
#define B_PCH_EHCI_PWR_CAP_D2_SUP           BIT10 // D2 State Support: not supported
#define B_PCH_EHCI_PWR_CAP_D1_SUP           BIT9  // D1 State Support: not supported
#define B_PCH_EHCI_PWR_CAP_AUX_CUR          (BIT8 | BIT7 | BIT6) // AUX Current
#define B_PCH_EHCI_PWR_CAP_DSI              BIT5  // DSI: Reports 0
#define B_PCH_EHCI_PWR_CAP_PME_CLK          BIT3  // PME Clock: Reports 0
#define B_PCH_EHCI_PWR_CAP_VER              (BIT2 | BIT1 | BIT0) // Version: Reports "010"

#define R_PCH_EHCI_PWR_CNTL_STS             0x54  // Power Management Control / Status
#define B_PCH_EHCI_PWR_CNTL_STS_PME_STS     BIT15 // PME Status
#define B_PCH_EHCI_PWR_CNTL_STS_DATASCL     (BIT14 | BIT13) // Data Scale
#define B_PCH_EHCI_PWR_CNTL_STS_DATASEL     (BIT12 | BIT11 | BIT10 | BIT9) // Data Select
#define B_PCH_EHCI_PWR_CNTL_STS_PME_EN      BIT8  // Power Enable
#define B_PCH_EHCI_PWR_CNTL_STS_PWR_STS     (BIT1 | BIT0) // Power State
#define V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D0  0     // D0 State
#define V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D3  (BIT1 | BIT0) // D3 Hot State

#define R_PCH_EHCI_DBG_CAPID                0x58  // Debug Port Capability ID
#define B_PCH_EHCI_DBG_CAPID                0xFF  // Debug Port Capability ID Mask

#define R_PCH_EHCI_NXT_PTR2                 0x59  // Next Item Pointer #2
#define B_PCH_EHCI_NXT_PTR2                 0xFF  // Next Item Pointer #2 Mask

#define R_PCH_EHCI_DBG_BASE                 0x5A  // Debug Port Base Offset
#define B_PCH_EHCI_DBG_BASE_BAR_NUM         0xE000 // BAR Number
#define B_PCH_EHCI_DBG_BASE_PORT_OFFSET     0x1FFF // Debug Port Offset

#define R_PCH_EHCI_USB_RELNUM               0x60  // Serial Bus Release Number
#define B_PCH_EHCI_USB_RELNUM               0xFF  // Serial Bus Release Number Mask

#define R_PCH_EHCI_FL_ADJ                   0x61  // Frame Length Adjustment
#define B_PCH_EHCI_FL_ADJ                   0x3F  // Frame Length Timing Value

#define R_PCH_EHCI_PWAKE_CAP                0x62  // Port Wake Capability
#define B_PCH_EHCI_PWAKE_CAP_MASK           0x7FE // Port Wake Up Capability Mask
#define B_PCH_EHCI_PWAKE_CAP_PWK_IMP        BIT0  // Port Wake Implemented

#define R_PCH_EHCI_PDO                      0x64  // Port Disable Override
#define B_PCH_EHCI_PDO_DIS_PORT7            BIT7  // USB Port 7 disable
#define B_PCH_EHCI_PDO_DIS_PORT6            BIT6  // USB Port 6 disable
#define B_PCH_EHCI_PDO_DIS_PORT5            BIT5  // USB Port 5 disable
#define B_PCH_EHCI_PDO_DIS_PORT4            BIT4  // USB Port 4 disable
#define B_PCH_EHCI_PDO_DIS_PORT3            BIT3  // USB Port 3 disable
#define B_PCH_EHCI_PDO_DIS_PORT2            BIT2  // USB Port 2 disable
#define B_PCH_EHCI_PDO_DIS_PORT1            BIT1  // USB Port 1 disable
#define B_PCH_EHCI_PDO_DIS_PORT0            BIT0  // USB Port 0 disable

#define R_PCH_EHCI_RMHDEVR                  0x66  // RMH Device Removable Field
#define B_PCH_EHCI_RMHDEVR_DRBM             0x1FE // Device Removable Bit Map

#define R_PCH_EHCI_LEGEXT_CAP               0x68  // USB2 Legacy Support Extended Capability
#define B_PCH_EHCI_LEGEXT_CAP_HCOS          BIT24 // HC OS Owned Semaphore
#define B_PCH_EHCI_LEGEXT_CAP_HCBIOS        BIT16 // HC BIOS Owned Semaphore
#define B_PCH_EHCI_LEGEXT_CAP_NEXT          0x0000FF00 // Next EHCI Capability Pointer
#define B_PCH_EHCI_LEGEXT_CAP_CAPID         0x000000FF // Capability ID

#define R_PCH_EHCI_LEGEXT_CS                0x6C  // USB2 Legacy Support Control / Status
#define B_PCH_EHCI_LEGEXT_CS_SMIBAR         BIT31 // SMI on BAR
#define B_PCH_EHCI_LEGEXT_CS_SMIPCI         BIT30 // SMI on PCI Command
#define B_PCH_EHCI_LEGEXT_CS_SMIOS          BIT29 // SMI on OS Ownership Change
#define B_PCH_EHCI_LEGEXT_CS_SMIAA          BIT21 // SMI on Async Advance
#define B_PCH_EHCI_LEGEXT_CS_SMIHSE         BIT20 // SMI on Host System Error
#define B_PCH_EHCI_LEGEXT_CS_SMIFLR         BIT19 // SMI on Frame List Rollover
#define B_PCH_EHCI_LEGEXT_CS_SMIPCD         BIT18 // SMI on Port Change Detect
#define B_PCH_EHCI_LEGEXT_CS_SMIERR         BIT17 // SMI on USB Error
#define B_PCH_EHCI_LEGEXT_CS_SMICOMP        BIT16 // SMI on USB Complete
#define B_PCH_EHCI_LEGEXT_CS_SMIBAR_EN      BIT15 // SMI on BAR Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIPCI_EN      BIT14 // SMI on PCI Command Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIOS_EN       BIT13 // SMI on OS Ownership Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIAA_EN       BIT5  // SMI on Async Advance Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIHSE_EN      BIT4  // SMI on Host System Error Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIFLR_EN      BIT3  // SMI on Frame List Rollover Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIPCD_EN      BIT2  // SMI on Port Change Enable
#define B_PCH_EHCI_LEGEXT_CS_SMIERR_EN      BIT1  // SMI on USB Error Enable
#define B_PCH_EHCI_LEGEXT_CS_SMICOMP_EN     BIT0  // SMI on USB Complete Enable

#define R_PCH_EHCI_SPCSMI                   0x70  // Intel-Specific USB2 SMI
#define B_PCH_EHCI_SPCSMI_SMIPO             0xFFC00000 // SMI on Port Owner
#define B_PCH_EHCI_SPCSMI_PMCSR             BIT21 // SMI on PMCSR
#define B_PCH_EHCI_SPCSMI_ASYNC             BIT20 // SMI on Async
#define B_PCH_EHCI_SPCSMI_PERIODIC          BIT19 // SMI on Periodic
#define B_PCH_EHCI_SPCSMI_CF                BIT18 // SMI on CF
#define B_PCH_EHCI_SPCSMI_HCHALT            BIT17 // SMI on HCHalted
#define B_PCH_EHCI_SPCSMI_HCRESET           BIT16 // SMI on HCReset
#define B_PCH_EHCI_SPCSMI_PO_EN             0x0000FFC0 // SMI on PortOwner Enable
#define B_PCH_EHCI_SPCSMI_PMCSR_EN          BIT5  // SMI on PMSCR Enable
#define B_PCH_EHCI_SPCSMI_ASYNC_EN          BIT4  // SMI on Async Enable
#define B_PCH_EHCI_SPCSMI_PERIODIC_EN       BIT3  // SMI on Periodic Enable
#define B_PCH_EHCI_SPCSMI_CF_EN             BIT2  // SMI on CF Enable
#define B_PCH_EHCI_SPCSMI_HCHALT_EN         BIT1  // SMI on HCHalted Enable
#define B_PCH_EHCI_SPCSMI_HCRESET_EN        BIT0  // SMI on HCReset Enable

#define R_PCH_EHCI_OCMAP                    0x74  // Over-Current Mapping
#define B_PCH_EHCI_OCMAP_D                  0xFF000000 // OC Mapping D
#define B_PCH_EHCI_OCMAP_C                  0x00FF0000 // OC Mapping C
#define B_PCH_EHCI_OCMAP_B                  0x0000FF00 // OC Mapping B
#define B_PCH_EHCI_OCMAP_A                  0x000000FF // OC Mapping A

#define R_PCH_EHCI_AFEMCTLTM                               0x78  // AFE Misc. Control & Test Mode
#define B_PCH_EHCI_AFEMCTLTM_S0PLLSHUTDOWNRMHDSPWROFF_0    BIT20 //S0PLLSHUTDOWNRMHDSPWROFF
#define B_PCH_EHCI_AFEMCTLTM_S0PLLSHUTDOWNEN_0             BIT28 //S0PLLSHUTDOWNEN_0
#define B_PCH_EHCI_AFEMCTLTM_MOCD                          BIT12 // Mask Over-current Detection
#define B_PCH_EHCI_AFEMCTLTM_UC480GE                       BIT9  // USB Clock 480 Gate Enable
#define B_PCH_EHCI_AFEMCTLTM_BTATS                         (BIT8 | BIT7 | BIT6 | BIT5 | BIT4) // Bus Turn Around Time Select
#define B_PCH_EHCI_AFEMCTLTM_HBPSE                         BIT3  // HS BIAS_EN Port Staggering Enable
#define B_PCH_EHCI_AFEMCTLTM_FLDPSE                        BIT2  // FS / LS DRIVE_EN Port Staggering Enable
#define B_PCH_EHCI_AFEMCTLTM_UACGEODP                      BIT1  // USB AFE Clock Gating Enable on Disconnected Ports
#define B_PCH_EHCI_AFEMCTLTM_UACGEOSP                      BIT0  // USB AFE Clock Gating Enable on Suspended Ports

#define R_PCH_EHCI_ESUBFEN                  0x7A  // EHCI Sub-Feature Enable
#define B_PCH_EHCI_ESUBFEN_S0PSEH           (BIT15 | BIT14 | BIT13)
#define B_PCH_EHCI_ESUBFEN_S0PSEN           BIT12 // S0 PLL Shutdown Enable
#define B_PCH_EHCI_ESUBFEN_S0PSIN           BIT11 // Ignore Noise on S0 PLL Shutdown
#define B_PCH_EHCI_ESUBFEN_S0PSSSS          BIT10 // Allow Serialization of S0 PLL Shutdown Flow with Sx Entry
#define B_PCH_EHCI_ESUBFEN_S0PSRWEN         BIT9  // S0 PLL Shutdown on Remote Wakeup Enable
#define B_PCH_EHCI_ESUBFEN_EUE              BIT8  // EHCI USBR Enable
#define B_PCH_EHCI_ESUBFEN_S0PSCFS          BIT7  // S0 PLL Shutdown on Configure Flag State
#define B_PCH_EHCI_ESUBFEN_S0PSD3S          BIT6  // S0 PLL Shutdown on D3 State
#define B_PCH_EHCI_ESUBFEN_S0PSRDNCS        BIT5  // S0 PLL Shutdown on RMH DS NOT_CONFIGURE State
#define B_PCH_EHCI_ESUBFEN_S0PSRDPOS        BIT4  // S0 PLL Shutdown on RMH DS POWER_OFF State
#define B_PCH_EHCI_ESUBFEN_S0PSRDDCS        BIT3  // S0 PLL Shutdown on RMH DS DISCONNECTED State
#define B_PCH_EHCI_ESUBFEN_S0PSRDDS         BIT2  // S0 PLL Shutdown on RMH DS DISABLED State
#define B_PCH_EHCI_ESUBFEN_S0PSRDSS         BIT1  // S0 PLL Shutdown on RMH DS SUSPENDED State
#define B_PCH_EHCI_ESUBFEN_RMH_DIS          BIT0  // RMH Disable

#define R_PCH_EHCI_EHCSUSCFG                0x7C  // EHC Suspend Well Configuration
#define B_PCH_EHCI_EHCSUSCFG_SLCGE          BIT14 // SB Local Clock Gating Enable
#define B_PCH_EHCI_EHCSUSCFG_ISURAD         BIT13 // IOSF-SB USB2 Register Access Disable
#define B_PCH_EHCI_EHCSUSCFG_ISPWP          BIT12 // IOSF-SB PLL Wake Policy
#define B_PCH_EHCI_EHCSUSCFG_RPIDSF         BIT9  // RMH Periodic IN Data SDC Fix
#define B_PCH_EHCI_EHCSUSCFG_RSCRFD         BIT8  // RMH Suspend Classic RXEN Fix Disable
#define B_PCH_EHCI_EHCSUSCFG_RCPSE          BIT7  // RMH clk240_pp Phase Select Enable
#define B_PCH_EHCI_EHCSUSCFG_RCPS           BIT6  // RMH clk240_pp Phase Select
#define B_PCH_EHCI_EHCSUSCFG_HS_COL_FIX     BIT5  // HS Collision Fix
#define B_PCH_EHCI_EHCSUSCFG_BSHF           BIT4  // Bas Split Handler Fix
#define B_PCH_EHCI_EHCSUSCFG_RPESROED       BIT3  // RMH Port Enable / Suspend / Resume on EOF Disable
#define B_PCH_EHCI_EHCSUSCFG_EEDFHEPFD      BIT2  // EHCI EOR Duration for HS Enable Ports Fix Disable
#define B_PCH_EHCI_EHCSUSCFG_RSSFR          BIT1  // RMH Speed Sample Fix Removal
#define B_PCH_EHCI_EHCSUSCFG_RDCISFR        BIT0  // RMH Disconnect / Connect in Sx Fix Removal

#define R_PCH_EHCI_RMHWKCTL                 0x7E  // RMH Wake Control
#define B_PCH_EHCI_RMHWKCTL_RIEWCS          BIT8  // RMH Inherit EHCI Wake Control Settings
#define B_PCH_EHCI_RMHWKCTL_RUWODR          BIT3  // RMH Upstream Wake on Device Resume
#define B_PCH_EHCI_RMHWKCTL_RUWOOD          BIT2  // RMH Upstream Wake on OC Disable
#define B_PCH_EHCI_RMHWKCTL_RUWODD          BIT1  // RMH Upstream Wake on Disconnect Disable
#define B_PCH_EHCI_RMHWKCTL_RUWOCD          BIT0  // RMH Upstream Wake on Connect Disable

#define R_PCH_EHCI_ACCESS_CNTL              0x80  // Access Control
#define B_PCH_EHCI_ACCESS_CNTL_VMAC         BIT15 // VE MMIO Access Control
#define B_PCH_EHCI_ACCESS_CNTL_RMAC         BIT8  // RMH MMIO Access Control
#define B_PCH_EHCI_ACCESS_CNTL_WRT_RDONLY   BIT0  // WRT RD Only
#define V_PCH_EHCI_ACCESS_CNTL_WRT_RDONLY_E 0x01  // WRT RD Only Enable
#define V_PCH_EHCI_ACCESS_CNTL_WRT_RDONLY_D 0x00  // WRT RD Only Disable

#define R_PCH_EHCI_RMHCTL                   0x82  // RMH Control
#define B_PCH_EHCI_RMHCTL_ERC               BIT7  // Enable RMH Connect
#define B_PCH_EHCI_RMHCTL_ERFR              BIT5  // Enable RMH Fast Reset
#define B_PCH_EHCI_RMHCTL_FRDPSS            (BIT4 | BIT3) // Fast Reset DS PORT Speed Select
#define B_PCH_EHCI_RMHCTL_DRGS              BIT2  // Disable RMH Global Suspend
#define B_PCH_EHCI_RMHCTL_ERFD              BIT1  // Enable RMH Force Disconnect
#define B_PCH_EHCI_RMHCTL_EOD               BIT0  // Enable Opportunistic Disconnect

#define R_PCH_EHCI_EHCIIR1                  0x84  // EHC Configuration 1
#define B_PCH_EHCI_EHCIIR1_DCOQWPRHT1MS     BIT31 // Disable Caching of QH with Poll Rate Higher Than 1ms
#define B_PCH_EHCI_EHCIIR1_PCQPD            BIT30 // Periodic Cache QTD Prefetch Disable
#define B_PCH_EHCI_EHCIIR1_AFP              BIT28 // ADIS Flush Policy
#define B_PCH_EHCI_EHCIIR1_IBSP             BIT27 // Invalidate Bit Set Policy
#define B_PCH_EHCI_EHCIIR1_MRP              (BIT26 | BIT25) // MFP Retry Policy
#define B_PCH_EHCI_EHCIIR1_APRP             (BIT24 | BIT23) // Asynch. Prefetch Retry Policy
#define B_PCH_EHCI_EHCIIR1_CWT              (BIT20 | BIT19) // Cx Writeback Threshold [1:0]
#define V_PCH_EHCI_EHCIIR1_CWT_16E          BIT19 // 16 Entries
#define V_PCH_EHCI_EHCIIR1_CWT_8E           BIT20 // 8 Entries
#define V_PCH_EHCI_EHCIIR1_CWT_4E           (BIT20 | BIT19) // 4 Entries
#define B_PCH_EHCI_EHCIIR1_COWD             BIT18 // Cx Opportunistic Writeback Disable
#define B_PCH_EHCI_EHCIIR1_PPP              (BIT17 | BIT16) // Periodic Prefetch Policy [1:0]
#define B_PCH_EHCI_EHCIIR1_PCH              (BIT15 | BIT14 | BIT13) // Periodic Caching Horizon
#define B_PCH_EHCI_EHCIIR1_CP               (BIT12 | BIT11) // Caching Policy [1:0]
#define B_PCH_EHCI_EHCIIR1_PSCC             (BIT10 | BIT9) // Periodic Schedule Caching Control
#define B_PCH_EHCI_EHCIIR1_DASC             BIT8  // Disable Asynchronous Schedule Caching
#define B_PCH_EHCI_EHCIIR1_PEEF             BIT6  // Prefetch Engine Error Flag
#define B_PCH_EHCI_EHCIIR1_PBPAP            BIT5  // Prefetch Based Pause-Activation Policy
#define B_PCH_EHCI_EHCIIR1_PBPD             BIT4  // Prefetch Based Pause-Scoreboard Enable
#define B_PCH_EHCI_EHCIIR1_SOAE             BIT3  // SERR on Aborts Enable
#define B_PCH_EHCI_EHCIIR1_GLSCO            (BIT2 | BIT1 | BIT0) // Gross Late Start Cut-Off

#define R_PCH_EHCI_EHCIIR2                  0x88  // EHC Configuration 2
#define B_PCH_EHCI_EHCIIR2_MCFP             BIT31 // MFP Cache Full Policy
#define B_PCH_EHCI_EHCIIR2_RBBAFD           BIT30 // RMH Bulk Buffer Arbitration Fix Disable
#define B_PCH_EHCI_EHCIIR2_RTMSFD           BIT29 // RMH TT Missing SOF Fix Disable
#define B_PCH_EHCI_EHCIIR2_RBOIEWSL8PFD     BIT28 // RMH Bytes on INTR Endpoint with SKU < 8 Ports Fix Disable
#define B_PCH_EHCI_EHCIIR2_RCIRFD           BIT27 // RMH CPT Immediate Retry Fix Disable
#define B_PCH_EHCI_EHCIIR2_RIIRFD           BIT26 // RMH IBX Immediate Retry Fix Disable
#define B_PCH_EHCI_EHCIIR2_ROCCFR           BIT25 // RMH OUT Contention Check Fix Removal
#define B_PCH_EHCI_EHCIIR2_PCDOFD           BIT24 // Payload Cache Disable Ordering Fix Disable
#define B_PCH_EHCI_EHCIIR2_AACTPWUIIRS      BIT23 // Allow Asynchronous Cache to Proceed When UE is in RETRY State
#define B_PCH_EHCI_EHCIIR2_RCSFR            BIT22 // RMH clk240_pp Stop Fix Removal
#define B_PCH_EHCI_EHCIIR2_UIAFFD           BIT21 // UE Invalidation Across Frame Fix Disable
#define B_PCH_EHCI_EHCIIR2_EBTIFD           BIT20 // UE Binary Tree Invalidation Fix Disable
#define B_PCH_EHCI_EHCIIR2_UCEOSF           BIT19 // UE Cache Exit on SRAM Full
#define B_PCH_EHCI_EHCIIR2_RSAFR            BIT18 // RMH SOP Arbitration Fix Removal
#define B_PCH_EHCI_EHCIIR2_RCCFR            BIT17 // RMH Contention Check Fix Removal
#define B_PCH_EHCI_EHCIIR2_CFRE             BIT16 // Cache Fast Reply Enable
#define B_PCH_EHCI_EHCIIR2_CRDO             BIT15 // Cache Retry DMA Optimization
#define B_PCH_EHCI_EHCIIR2_MLH              BIT14 // MFP Late Handling
#define B_PCH_EHCI_EHCIIR2_SFP              BIT13 // SFP Late Handling
#define B_PCH_EHCI_EHCIIR2_PLH              BIT12 // Payload Late Handling
#define B_PCH_EHCI_EHCIIR2_HFR              BIT11 // HCRESET Flow Removal
#define B_PCH_EHCI_EHCIIR2_FFR              BIT10 // Fatal Flow Removal
#define B_PCH_EHCI_EHCIIR2_MTSFF            BIT9  // MFC to SFC Flush Flow
#define B_PCH_EHCI_EHCIIR2_PCRE             BIT8  // Periodic Cache Retry Enable
#define B_PCH_EHCI_EHCIIR2_ASQPD            BIT7  // Async. Single QH Prefetch Disable
#define B_PCH_EHCI_EHCIIR2_SOPPWFTPC        BIT6  // Size of Payload Posted Writes From the Payload Cache
#define B_PCH_EHCI_EHCIIR2_DPC              BIT5  // Disable Payload Caching
#define B_PCH_EHCI_EHCIIR2_DPPAMB           BIT4  // Disable Payload Prefetch Across Microframe Boundaries
#define B_PCH_EHCI_EHCIIR2_PMPD             BIT3  // Periodic MicroFrame Pipeline Disable
#define B_PCH_EHCI_EHCIIR2_VCS              BIT2  // Virtual Channel Select
#define B_PCH_EHCI_EHCIIR2_DASA             BIT1  // Descriptor Access Snoop Attribute
#define B_PCH_EHCI_EHCIIR2_DBASA            BIT0  // Data Buffer Access Snoop Attribute

#define R_PCH_EHCI_EHCIIR3                  0x8C  // EHC Configuration 3
#define B_PCH_EHCI_EHCIIR3_FMHSHKASMSDC     BIT27
#define B_PCH_EHCI_EHCIIR3_TTPMDFP          BIT26
#define B_PCH_EHCI_EHCIIR3_TTISCOMEPDP      BIT25
#define B_PCH_EHCI_EHCIIR3_TTX4DAFP         BIT24
#define B_PCH_EHCI_EHCIIR3_PDAHEE           BIT23 // PBP DMA Active Hysteresis Effect Enable
#define B_PCH_EHCI_EHCIIR3_PDAHV            (BIT22 | BIT21 | BIT20) // PBP DMA Active Hysteresis Value
#define B_PCH_EHCI_EHCIIR3_RACFS            (BIT19 | BIT18) // Read After Cache Flush Select
#define B_PCH_EHCI_EHCIIR3_UTIROXEFD        BIT17 // UH TT Internal Reset on X+4 / EOF1 Fix Disable
#define B_PCH_EHCI_EHCIIR3_TSBBRF           BIT16 // TT STALL Bulk Buffer Release Fix
#define B_PCH_EHCI_EHCIIR3_PPWP             (BIT11 | BIT10 | BIT9 | BIT8) // PBP Pre-Wake Policy
#define B_PCH_EHCI_EHCIIR3_UHHFD            BIT6  // UD HCReset Hung Fix Disable
#define B_PCH_EHCI_EHCIIR3_UEMBEFD          BIT5  // UD ECC Multiple Bit Error Fix Disable
#define B_PCH_EHCI_EHCIIR3_MMIFD            BIT4  // MFC Missing Invalidation Fix Disable
#define B_PCH_EHCI_EHCIIR3_UWP              BIT2  // UD Writeback Policy
#define B_PCH_EHCI_EHCIIR3_UDAP             BIT1  // UD DMA Active Policy
#define B_PCH_EHCI_EHCIIR3_PCEPEDP          BIT0  // Periodic Cache Enable Periodic Engine Done Policy

#define R_PCH_EHCI_FLR_CID                  0x98  // Function Level Reset Capability ID
#define B_PCH_EHCI_FLR_CID                  0xFF  // Function Level Reset Capability ID Mask
#define V_PCH_EHCI_FLR_CID_13               0x13
#define V_PCH_EHCI_FLR_CID_09               0x09

#define R_PCH_EHCI_FLR_NEXT                 0x99  // FLR Next capability Pointer
#define B_PCH_EHCI_FLR_NEXT                 0xFF  // FLR Next capability Pointer Mask

#define R_PCH_EHCI_FLR_CLV                  0x9A  // FLR Capability Length and Version
#define B_PCH_EHCI_FLR_CLV_CAP_SSEL0        BIT9  // FLR Capability
#define B_PCH_EHCI_FLR_CLV_TXP_SSEL0        BIT8  // TXP capability
#define B_PCH_EHCI_FLR_CLV_LNG              0x00FF // Capability Length

#define R_PCH_EHCI_FLR_CTRL                 0x9C  // FLR Control Register
#define B_PCH_EHCI_FLR_CTRL_INITFLR         BIT0  // Initiate FLR

#define R_PCH_EHCI_FLR_STS                  0x9D  // FLR Status Register
#define B_PCH_EHCI_FLR_STS_TXP              BIT0  // Transactions Pending

#define R_PCH_EHCI_DP_CTRLSTS               0xA0  // 1.2.1.21   Debug Port Control/Status Register (DP_CTRLSTS)
#define B_PCH_EHCI_DP_OWNER_CNT             BIT30  // OWNER_CNT (OWNERCNT_0)
#define B_PCH_EHCI_DP_ENABLED_CNT           BIT28  // ENABLED_CNT (ENABLEDCNT_0)


#define R_PCH_EHCI_STM                      0xD0  // Simulation Test Modes
#define B_PCH_EHCI_STM_SHORTFRAME_EN        BIT7  // Short Frame Enable
#define B_PCH_EHCI_STM_SHORTFRAME_SEL       (BIT1 | BIT0) // Short Frame Select

#define R_PCH_EHCI_LBDATA                   0xD4  // Loop-Back Data
#define B_PCH_EHCI_LBDATA_MASK              0x00FFFFFF // Three Bytes of the Loop-Back Data

#define R_PCH_EHCI_MISCTM                   0xD8  // Misc. Test Mode
#define B_PCH_EHCI_MISCTM_UTC               0xFFFF0000 // USB TX CRC
#define B_PCH_EHCI_MISCTM_UHLPS             0x0000E000 // USB HS Loopback Port Select
#define B_PCH_EHCI_MISCTM_UTCPS             0x1C00 // USB TX CRC Port Select
#define B_PCH_EHCI_MISCTM_DRCPORT0SEL       BIT9  // DRC Port0 Select
#define B_PCH_EHCI_MISCTM_DRCBYPASS         BIT8  // DRC Bypass
#define B_PCH_EHCI_MISCTM_UHLE              BIT7  // USB HS Loopback Enable
#define B_PCH_EHCI_MISCTM_DRB               (BIT6 | BIT5 | BIT4) // Discard Rdata Bits
#define B_PCH_EHCI_MISCTM_UTCR              BIT3  // USB TX CRC Reset
#define B_PCH_EHCI_MISCTM_FELPBKBE          BIT1  // Far end Loopback Burst Enable
#define B_PCH_EHCI_MISCTM_FELPBK            BIT0  // Far End Loopback

#define R_PCH_EHCI_CLKGATEEN                0xDC  // Clock Gate Enable
#define B_PCH_EHCI_CLKGATEEN_USB2CDCG       BIT5  // USB2 Cache Dynamic Clock Gating Enable
#define B_PCH_EHCI_CLKGATEEN_USB2CFCG       BIT4  // USB2 Cache Force Clock Gating Enable
#define B_PCH_EHCI_CLKGATEEN_USB2ADCG       BIT2  // USB2 Aggressive Dynamic Clock Gating Enable
#define B_PCH_EHCI_CLKGATEEN_USB2DCG        BIT1  // USB2 Dynamic Clock Gating Enable
#define B_PCH_EHCI_CLKGATEEN_USB2SCG        BIT0  // USB2 Static Clock Gating Enable

#define R_PCH_EHCI_UL                       0xF4  // Unit Latency
#define B_PCH_EHCI_UL_LATPGMEN              BIT31 // LATPGMEN
#define B_PCH_EHCI_UL_TRBT                  (BIT23 | BIT22) // Transmit and Receive Byte Threshold
#define V_PCH_EHCI_UL_TRBT_20               (BIT23 | BIT22) // 20 High-Speed Byte Times
#define V_PCH_EHCI_UL_TRBT_18               BIT23  // 18 High-Speed Byte Times
#define V_PCH_EHCI_UL_TRBT_16               BIT22  // 16 High-Speed Byte Times
#define V_PCH_EHCI_UL_TRBT_12               0      // 12 High-Speed Byte Times
#define B_PCH_EHCI_UL_CUISO                 (BIT21 | BIT20) // Classic USB Interface Signals
#define B_PCH_EHCI_UL_DOE                   BIT19 // Disconnect Offset Enable
#define B_PCH_EHCI_UL_DSO                   (BIT18 | BIT17 | BIT16) // Disconnect Sample Offset
#define B_PCH_EHCI_UL_TDO                   (BIT15 | BIT14 | BIT13 | BIT12) // Timeout Detect Offset
#define B_PCH_EHCI_UL_IPGA                  (BIT11 | BIT10 | BIT9 | BIT8) // Inter Packet Gap Address
#define B_PCH_EHCI_UL_GLSOP                 (BIT7 | BIT6 | BIT5 | BIT4) // Granular Late Start Offset (Periodic)
#define B_PCH_EHCI_UL_GLSOA                 (BIT3 | BIT2 | BIT1 | BIT0) // Granular Late Start Offset (Asynchronous)

#define R_PCH_EHCI_MANID                    0xF8  // Manufacturer's ID
#define B_PCH_EHCI_MAN_ID_DPID              0xF000000 // Dot Portion of Process ID
#define B_PCH_EHCI_MAN_ID_MSID              0xFF0000 // Manufacturing Stepping Identifier
#define B_PCH_EHCI_MAN_ID_MID               0xFF00 // Manufacturing Identifier
#define B_PCH_EHCI_MAN_ID_PPID              0xFF  // Process Portion of Process ID

//
// EHCI MMIO registers
//
#define R_PCH_EHCI_CAPLENGTH                0x00  // Capability Registers Length

#define R_PCH_EHCI_HCIVERSION               0x02  // Host Controller Interface Version
#define B_PCH_EHCI_HCIVERSION_NUM           0xFF  // Host Controller Interface Version Number

#define R_PCH_EHCI_HCSPARAMS                0x04  // Host Controller Structural
#define B_PCH_EHCI_HCSPARAMS_DP_N           (BIT23 | BIT22 | BIT21 | BIT20) // Debug Port Number
#define N_PCH_EHCI_HCSPARAMS_DP_N           20
#define B_PCH_EHCI_HCSPARAMS_P_INDICATOR    BIT16 // Port Indicators
#define B_PCH_EHCI_HCSPARAMS_N_CC           (BIT15 | BIT14 | BIT13 | BIT12) // Number of Companion Controllers
#define N_PCH_EHCI_HCSPARAMS_N_CC           12
#define B_PCH_EHCI_HCSPARAMS_N_PCC          (BIT11 | BIT10 | BIT9 | BIT8) // Number of Ports per Companion Controller
#define N_PCH_EHCI_HCSPARAMS_N_PCC          8
#define B_PCH_EHCI_HCSPARAMS_PPC            BIT4  // Power Port Control
#define B_PCH_EHCI_HCSPARAMS_N_PORTS        (BIT3 | BIT2 | BIT1 | BIT0) // Number of Ports
#define N_PCH_EHCI_HCSPARAMS_N_PORTS        0

#define R_PCH_EHCI_USB2CMD                  0x20  // USB2 Command Register
#define B_PCH_EHCI_USB2CMD_ITC              0xFF0000 // Interrupt Threshold Control
#define B_PCH_EHCI_USB2CMD_ASPE             BIT13 // Asynch Schedule Prefetch Enable Bit
#define B_PCH_EHCI_USB2CMD_PSPE             BIT12 // Prediodic Schedule Prefetch Enable
#define B_PCH_EHCI_USB2CMD_IOAAD            BIT6  // Interrupt on Async. Advance Doorbell
#define B_PCH_EHCI_USB2CMD_ASE              BIT5  // Asynchronous Schedule Enable
#define B_PCH_EHCI_USB2CMD_PSE              BIT4  // Periodic Schedule Enable
#define B_PCH_EHCI_USB2CMD_FLS              (BIT3 | BIT2) // Frame List Size
#define B_PCH_EHCI_USB2CMD_HCRESET          BIT1  // Host Controller Reset
#define B_PCH_EHCI_USB2CMD_RS               BIT0  // Run / Stop

#define R_PCH_EHCI_CONFIGFLAG               0x60  // Configure Flag Register
#define B_PCH_EHCI_CONFIGFLAG               BIT0  // Configure Flag Bit

#define R_PCH_EHCI_PORTSC0                  0x64  // Port 0 Status and Control
#define B_PCH_EHCI_PORTSC0_SUSPEND          BIT7  // Suspend
#define B_PCH_EHCI_PORTSC0_PORT_EN_DIS      BIT2  // Port Enable / Disable

///
/// USB3 (XHCI) related definitions
///
#define PCI_DEVICE_NUMBER_PCH_XHCI          20
#define PCI_FUNCTION_NUMBER_PCH_XHCI        0

///
/// XHCI PCI Config Space registers
///
#define R_PCH_XHCI_VENDOR_ID                0x00  // Vendor ID
#define B_PCH_XHCI_VENDOR_ID                0xFFFF

#define R_PCH_XHCI_DEVICE_ID                0x02  // Device ID
#define B_PCH_XHCI_DEVICE_ID                0xFFFF

#define R_PCH_XHCI_COMMAND_REGISTER         0x04  // Command
#define B_PCH_XHCI_COMMAND_ID               BIT10 // Interrupt Disable
#define B_PCH_XHCI_COMMAND_FBE              BIT9  // Fast Back to Back Enable
#define B_PCH_XHCI_COMMAND_SERR             BIT8  // SERR# Enable
#define B_PCH_XHCI_COMMAND_WCC              BIT7  // Wait Cycle Control
#define B_PCH_XHCI_COMMAND_PER              BIT6  // Parity Error Response
#define B_PCH_XHCI_COMMAND_VPS              BIT5  // VGA Palette Snoop
#define B_PCH_XHCI_COMMAND_MWI              BIT4  // Memory Write Invalidate
#define B_PCH_XHCI_COMMAND_SCE              BIT3  // Special Cycle Enable
#define B_PCH_XHCI_COMMAND_BME              BIT2  // Bus Master Enable
#define B_PCH_XHCI_COMMAND_MSE              BIT1  // Memory Space Enable

#define R_PCH_XHCI_MEM_BASE                 0x10  // Memory Base Address
#define B_PCH_XHCI_MEM_BASE_BA              0xFFFFFFFFFFFF0000 // Base Address
#define V_PCH_XHCI_MEM_LENGTH               0x10000 // 64 KB of Memory Length
#define N_PCH_XHCI_MEM_ALIGN                16    // Memory Space Alignment
#define B_PCH_XHCI_MEM_BASE_PREF            BIT3  // Prefetchable
#define B_PCH_XHCI_MEM_BASE_TYPE            (BIT2 | BIT1) // Type
#define B_PCH_XHCI_MEM_BASE_RTE             BIT0  // Resource Type Indicator

#define R_PCH_XHCI_SVID                     0x2C
#define B_PCH_XHCI_SVID                     0xFFFF

#define R_PCH_XHCI_SID                      0x2E
#define B_PCH_XHCI_SID                      0xFFFF

#define R_PCH_XHCI_XHCC1                    0x40
#define B_PCH_XHCI_XHCC1_ACCTRL             BIT31
#define B_PCH_XHCI_XHCC1_RMTASERR           BIT24
#define B_PCH_XHCI_XHCC1_URD                BIT23
#define B_PCH_XHCI_XHCC1_URRE               BIT22
#define B_PCH_XHCI_XHCC1_IIL1E              (BIT21 | BIT20 | BIT19)
#define V_PCH_XHCI_XHCC1_IIL1E_DIS          0
#define V_PCH_XHCI_XHCC1_IIL1E_32           (BIT19)
#define V_PCH_XHCI_XHCC1_IIL1E_64           (BIT20)
#define V_PCH_XHCI_XHCC1_IIL1E_128          (BIT20 | BIT19)
#define V_PCH_XHCI_XHCC1_IIL1E_256          (BIT21)
#define V_PCH_XHCI_XHCC1_IIL1E_512          (BIT21 | BIT19)
#define V_PCH_XHCI_XHCC1_IIL1E_1024         (BIT21 | BIT20)
#define V_PCH_XHCI_XHCC1_IIL1E_131072       (BIT21 | BIT20 | BIT19)
#define B_PCH_XHCI_XHCC1_XHCIL1E            BIT18 // XHC Initiated L1 Enable
#define B_PCH_XHCI_XHCC1_D3IL1E             BIT17 // D3 Initiated L1 Enable
#define B_PCH_XHCI_XHCC1_UNPPA              (BIT16 | BIT15 | BIT14 | BIT13 | BIT12) // Periodic Complete Pre Wake Time
#define B_PCH_XHCI_XHCC1_SWAXHCI            BIT11 // SW Assisted xHC Idle
#define B_PCH_XHCI_XHCC1_L23HRAWC           (BIT10 | BIT9 | BIT8) // L23 to Host Reset Acknowledge Wait Count
#define V_PCH_XHCI_XHCC1_L23HRAWC_DIS       0
#define V_PCH_XHCI_XHCC1_L23HRAWC_128       (BIT8)
#define V_PCH_XHCI_XHCC1_L23HRAWC_256       (BIT9)
#define V_PCH_XHCI_XHCC1_L23HRAWC_512       (BIT9 | BIT8)
#define V_PCH_XHCI_XHCC1_L23HRAWC_1024      (BIT10)
#define V_PCH_XHCI_XHCC1_L23HRAWC_2048      (BIT10 | BIT8)
#define V_PCH_XHCI_XHCC1_L23HRAWC_4096      (BIT10 | BIT9)
#define V_PCH_XHCI_XHCC1_L23HRAWC_131072    (BIT10 | BIT9 | BIT8)
#define B_PCH_XHCI_XHCC1_UTAGCP             (BIT7 | BIT6) // Upstream Type Arbiter Grant Count Posted
#define B_PCH_XHCI_XHCC1_UDAGCNP            (BIT5 | BIT4) // Upstream Type Arbiter Grant Count Non Posted
#define B_PCH_XHCI_XHCC1_UDAGCCP            (BIT3 | BIT2) // Upstream Type Arbiter Grant Count Completion
#define B_PCH_XHCI_XHCC1_UDAGC              (BIT1 | BIT0) // Upstream Type Arbiter Grant Count

#define R_PCH_XHCI_XHCC2                    0x44  // XHC System Bus Configuration 2
#define B_PCH_XHCI_XHCC2_OCCFDONE           BIT31 // OC Configuration Done
#define B_PCH_XHCI_XHCC2_DREQBCC            BIT25 // DMA Request Boundary Crossing Control
#define B_PCH_XHCI_XHCC2_IDMARRSC           (BIT24 | BIT23 | BIT22) // IDMA Read Request Size Control
#define B_PCH_XHCI_XHCC2_XHCUPRDROE         BIT21 // XHC Upstream Read Relaxed Ordering Enable
#define B_PCH_XHCI_XHCC2_IOSFSRAD           BIT20 // IOSF Sideband Register Access Disable
#define B_PCH_XHCI_XHCC2_UNPPA              0xFC000 // Upstream Non-Posted Pre-Allocation
#define B_PCH_XHCI_XHCC2_SWAXHCIP           (BIT13 | BIT12) // SW Assisted xHC Idle Policy
#define B_PCH_XHCI_XHCC2_RAWDD              BIT11 // MMIO Read After MMIO Write Delay Disable
#define B_PCH_XHCI_XHCC2_WAWDE              BIT10 // MMIO Write After MMIO Write Delay Enable
#define B_PCH_XHCI_XHCC2_SWACXIHB           (BIT9 | BIT8) // SW Assisted Cx Inhibit
#define B_PCH_XHCI_XHCC2_SWADMIL1IHB        (BIT7 | BIT6) // SW Assisted DMI L1 Inhibit
#define B_PCH_XHCI_XHCC2_L1FP2CGWC          (BIT5 | BIT4 | BIT3) // L1 Force P2 clock Gating Wait Count
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_DIS      0
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_128      (BIT3)
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_256      (BIT4)
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_512      (BIT4 | BIT3)
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_1024     (BIT5)
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_2048     (BIT5 | BIT3)
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_4096     (BIT5 | BIT4)
#define V_PCH_XHCI_XHCC2_L1FP2CGWC_131072   (BIT5 | BIT4 | BIT3)
#define B_PCH_XHCI_XHCC2_RDREQSZCTRL        (BIT2 | BIT1 | BIT0) // Read Request Size Control
#define V_PCH_XHCI_XHCC2_RDREQSZCTRL_128    0
#define V_PCH_XHCI_XHCC2_RDREQSZCTRL_256    (BIT0)
#define V_PCH_XHCI_XHCC2_RDREQSZCTRL_512    (BIT1)
#define V_PCH_XHCI_XHCC2_RDREQSZCTRL_64     (BIT2 | BIT1 | BIT0)

#define R_PCH_XHCI_XHCLKGTEN                0x50  // Clock Gating
#define B_PCH_XHCI_XHCLKGTEN_NUEFBCGPS      BIT28 // Naking USB2.0 EPs for Backbone Clock Gating and PLL Shutdown
#define B_PCH_XHCI_XHCLKGTEN_SRAMPGTEN      BIT27 // SRAM Power Gate Enable
#define B_PCH_XHCI_XHCLKGTEN_SSLSE          BIT26 // SS Link PLL Shutdown Enable
#define B_PCH_XHCI_XHCLKGTEN_USB2PLLSE      BIT25 // USB2 PLL Shutdown Enable
#define B_PCH_XHCI_XHCLKGTEN_IOSFSTCGE      BIT24 // IOSF Sideband Trunk Clock Gating Enable
#define B_PCH_XHCI_XHCLKGTEN_HSTCGE         (BIT23 | BIT22 | BIT21 | BIT20) // HS Backbone PXP Trunk Clock Gate Enable
#define B_PCH_XHCI_XHCLKGTEN_SSTCGE         (BIT19 | BIT18 | BIT17 | BIT16) // SS Backbone PXP Trunk Clock Gate Enable
#define B_PCH_XHCI_XHCLKGTEN_XHCIGEU3S      BIT15 // XHC Ignore_EU3S
#define B_PCH_XHCI_XHCLKGTEN_XHCFTCLKSE     BIT14 // XHC Frame Timer Clock Shutdown Enable
#define B_PCH_XHCI_XHCLKGTEN_XHCBBTCGIPISO  BIT13 // XHC Backbone PXP Trunk Clock Gate In Presence of ISOCH EP
#define B_PCH_XHCI_XHCLKGTEN_XHCHSTCGU2NRWE BIT12 // XHC HS Backbone PXP Trunk Clock Gate U2 non RWE
#define B_PCH_XHCI_XHCLKGTEN_XHCUSB2PLLSDLE (BIT11 | BIT10) // XHC USB2 PLL Shutdown Lx Enable
#define B_PCH_XHCI_XHCLKGTEN_HSPLLSUE       (BIT9 | BIT8) // HS Backbone PXP PLL Shutdown Ux Enable
#define B_PCH_XHCI_XHCLKGTEN_SSPLLSUE       (BIT7 | BIT6 | BIT5) // SS backbone PXP PLL Shutdown Ux Enable
#define B_PCH_XHCI_XHCLKGTEN_XHCBLCGE       BIT4  // XHC Backbone Local Clock Gating Enable
#define B_PCH_XHCI_XHCLKGTEN_HSLTCGE        BIT3  // HS Link Trunk Clock Gating Enable
#define B_PCH_XHCI_XHCLKGTEN_SSLTCGE        BIT2  // SS Link Trunk Clock Gating Enable
#define B_PCH_XHCI_XHCLKGTEN_IOSFBTCGE      BIT1  // IOSF Backbone Trunk Clock Gating Enable
#define B_PCH_XHCI_XHCLKGTEN_IOSFGBLCGE     BIT0  // IOSF Gasket Backbone Local Clock Gating Enable

#define R_PCH_XHCI_USB_RELNUM               0x60
#define B_PCH_XHCI_USB_RELNUM               0xFF

#define R_PCH_XHCI_FL_ADJ                   0x61
#define B_PCH_XHCI_FL_ADJ                   0x3F

#define R_PCH_XHCI_PWR_CAPID                0x70
#define B_PCH_XHCI_PWR_CAPID                0xFF

#define R_PCH_XHCI_NXT_PTR1                 0x71
#define B_PCH_XHCI_NXT_PTR1                 0xFF

#define R_PCH_XHCI_PWR_CAP                  0x72
#define B_PCH_XHCI_PWR_CAP_PME_SUP          0xF800
#define B_PCH_XHCI_PWR_CAP_D2_SUP           BIT10
#define B_PCH_XHCI_PWR_CAP_D1_SUP           BIT9
#define B_PCH_XHCI_PWR_CAP_AUX_CUR          (BIT8 | BIT7 | BIT6)
#define B_PCH_XHCI_PWR_CAP_DSI              BIT5
#define B_PCH_XHCI_PWR_CAP_PME_CLK          BIT3
#define B_PCH_XHCI_PWR_CAP_VER              (BIT2 | BIT1 | BIT0)

#define R_PCH_XHCI_PWR_CNTL_STS             0x74
#define B_PCH_XHCI_PWR_CNTL_STS_PME_STS     BIT15
#define B_PCH_XHCI_PWR_CNTL_STS_DATASCL     (BIT14 | BIT13)
#define B_PCH_XHCI_PWR_CNTL_STS_DATASEL     (BIT12 | BIT11 | BIT10 | BIT9)
#define B_PCH_XHCI_PWR_CNTL_STS_PME_EN      BIT8
#define B_PCH_XHCI_PWR_CNTL_STS_PWR_STS     (BIT1 | BIT0)
#define V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3  (BIT1 | BIT0)

#define R_PCH_XHCI_HSCFG1                   0xA0  // High Speed Configuration 1
#define R_PCH_XHCI_HSCFG2                   0xA4  // High Speed Configuration 2
#define B_PCH_XHCI_HSCFG2_HSAAIM            BIT15  // HS ASYNC Active IN Mask (HSAAIM)
#define B_PCH_XHCI_HSCFG2_HSOAAPEPM         BIT14  // HS OUT ASYNC Active Polling EP Mask (HSOAAPEPM)
#define B_PCH_XHCI_HSCFG2_HSIAAPEPM         BIT13  // HS IN ASYNC Active Polling EP Mask (HSIAAPEPM)
#define B_PCH_XHCI_HSCFG2_HSIIPAPC          (BIT12 | BIT11)  // HS INTR IN Periodic Active Policy Control (HSIIPAPC)
#define B_PCH_XHCI_HSCFG2_HSIIPANEPT        (BIT10 | BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4)  // HS INTR IN Periodic Active Num of EP Threshold(HSIIPANEPT)
#define B_PCH_XHCI_HSCFG2_HSIIPASIT         (BIT3 | BIT2 | BIT1 | BIT0)                        // HS INTR IN Periodic Active Service Interval Threshold (HSIIPASIT)
#define R_PCH_XHCI_SSCFG1                   0xB0  // SuperSpeed Configuration 1
#define B_PCH_XHCI_XHCLKGTEN_NUEFBCGPS      BIT28 // Naking USB2.0 EPs for Backbone Clock Gating and PLL Shutdown
#define B_PCH_XHCI_SSCFG1_LFPS_PMU3EN       (BIT21 | BIT20 | BIT19 | BIT18)  // LFPS Power Management in U3 Enable
#define B_PCH_XHCI_SSCFG1_LFPS_PMEN         BIT17  // LFPS Power Management Enable
#define B_PCH_XHCI_SSCFG1_USB3PHY_PGENU2    BIT14  // USB3 PHY Power Gate Enable for U2
#define B_PCH_XHCI_SSCFG1_USB3PHY_PGENS_OTU2 BIT13  // USB3 PHY Power Gate Enable for states other than U2
#define R_PCH_XHCI_U2OCM1                   0xC0  // XHCI USB2 Overcurrent Pin Mapping 1
#define B_PCH_XHCI_U2OCM1_OC2_MAPPING       0x00000F00
#define B_PCH_XHCI_U2OCM1_OC1_MAPPING       0x0000000F

#define R_PCH_XHCI_U3OCM1                   0xC8  // XHCI USB3 Overcurrent Pin Mapping 1
#define B_PCH_XHCI_U3OCM1_OC4_MAPPING       BIT24
#define B_PCH_XHCI_U3OCM1_OC3_MAPPING       BIT16
#define B_PCH_XHCI_U3OCM1_OC2_MAPPING       BIT8
#define B_PCH_XHCI_U3OCM1_OC1_MAPPING       BIT0

#define R_PCH_XHCI_U3OCM2                   0xCC  // XHCI USB3 Overcurrent Pin Mapping 2
#define B_PCH_XHCI_U3OCM2_OC8_MAPPING       BIT24
#define B_PCH_XHCI_U3OCM2_OC7_MAPPING       BIT16
#define B_PCH_XHCI_U3OCM2_OC6_MAPPING       BIT8
#define B_PCH_XHCI_U3OCM2_OC5_MAPPING       BIT0

#define R_PCH_XHCI_USB2PR                   0xD0  // USB2 Port Routing
#define B_PCH_XHCI_USB2PR_USB2HCSEL         0x3F  // USB2 HC Selector

#define R_PCH_XHCI_USB2PRM                  0xD4  // USB2 Port Routing Mask
#define B_PCH_XHCI_USB2PR_USB2HCSELM        0x3F // USB2 HC Selector Mask

#define R_PCH_XHCI_USB3PR                   0xD8  // USB3 Port Routing
#define B_PCH_XHCI_USB3PR_USB3SSEN          0x01  // USB3 SS Enable

#define R_PCH_XHCI_USB3PRM                  0xDC  // USB3 Port Routing Mask
#define B_PCH_XHCI_USB3PR_USB3SSENM         0x01  // USB3 SS Enable Mask

#define R_PCH_XHCI_FUS                      0xE0  // Fuse and Strap
#define B_PCH_XHCI_FUS_USH_DEVID            (BIT13 | BIT12 | BIT11) // USH Device ID
#define B_PCH_XHCI_FUS_XDD_EN               BIT10 // Debug Device Enable
#define B_PCH_XHCI_FUS_SRAMPWRGTDIS         BIT9  // SRAM Power Gating Disable
#define B_PCH_XHCI_FUS_USB2PLLSDIS          BIT8  // USB2 PLL Shutdown Disable
#define B_PCH_XHCI_FUS_USBIOPMDIS           BIT7  // USB I/O Power Management Disable
#define B_PCH_XHCI_FUS_XHCDCGDIS            BIT6  // XHC Dynamic Clock Gating Disable
#define B_PCH_XHCI_FUS_USBR                 BIT5  // USBr Disable
#define V_PCH_XHCI_FUS_USBR_EN              0
#define V_PCH_XHCI_FUS_USBR_DIS             BIT5
#define B_PCH_XHCI_FUS_SSPRTCNT             (BIT4 | BIT3) // SS Port Count
#define V_PCH_XHCI_FUS_SSPRTCNT_00B         0
#define V_PCH_XHCI_FUS_SSPRTCNT_01B         (BIT3)
#define V_PCH_XHCI_FUS_SSPRTCNT_10B         (BIT4)
#define V_PCH_XHCI_FUS_SSPRTCNT_11B         (BIT4 | BIT3)
#define B_PCH_XHCI_FUS_HSPRTCNT             (BIT2 | BIT1) // HS Port Count
#define V_PCH_XHCI_FUS_HSPRTCNT_00B         0
#define V_PCH_XHCI_FUS_HSPRTCNT_01B         (BIT1)
#define V_PCH_XHCI_FUS_HSPRTCNT_10B         (BIT2)
#define V_PCH_XHCI_FUS_HSPRTCNT_11B         (BIT2 | BIT1)
#define V_PCH_XHCI_FUS_SSPRTCNT_00B_CNT     6
#define V_PCH_XHCI_FUS_SSPRTCNT_01B_CNT     4
#define V_PCH_XHCI_FUS_SSPRTCNT_10B_CNT     2
#define V_PCH_XHCI_FUS_SSPRTCNT_11B_CNT     0
#define V_PCH_XHCI_FUS_SSPRTCNT_00B_MASK    0x3F
#define V_PCH_XHCI_FUS_SSPRTCNT_01B_MASK    0x0F
#define V_PCH_XHCI_FUS_SSPRTCNT_10B_MASK    0x03
#define V_PCH_XHCI_FUS_SSPRTCNT_11B_MASK    0x00
#define V_PCH_XHCI_FUS_HSPRTCNT_00B_CNT     14
#define V_PCH_XHCI_FUS_HSPRTCNT_01B_CNT     12
#define V_PCH_XHCI_FUS_HSPRTCNT_10B_CNT     10
#define V_PCH_XHCI_FUS_HSPRTCNT_11B_CNT     8
#define V_PCH_XHCI_FUS_HSPRTCNT_00B_MASK    0x3FFF
#define V_PCH_XHCI_FUS_HSPRTCNT_01B_MASK    0x0FFF
#define V_PCH_XHCI_FUS_HSPRTCNT_10B_MASK    0x03FF
#define V_PCH_XHCI_FUS_HSPRTCNT_11B_MASK    0x00FF
#define B_PCH_XHCI_FUS_XHCFD                BIT0  // XHCI Function Disable

#define R_PCH_XHCI_USB2PDO                  0xE4  // USB2 Port Disable Override
#define B_PCH_XHCI_USB2PDO_MASK             0x1FF
#define B_PCH_XHCI_USB2PDO_DIS_PORT0        BIT0

#define R_PCH_XHCI_USB3PDO                  0xE8  // USB3 Port Disable Override
#define B_PCH_XHCI_USB3PDO_MASK             0x0F
#define B_PCH_XHCI_USB3PDO_DIS_PORT0        BIT0

///
/// xHCI MMIO registers
///

///
/// 0x00 - 0x1F - Capability Registers
///
#define R_PCH_XHCI_CAPLENGTH                0x00  // Capability Registers Length

#define R_PCH_XHCI_HCIVERSION               0x02  // Host Controller Interface Version Number

#define R_PCH_XHCI_HCSPARAMS1               0x04  // Structural Parameters 1
#define B_PCH_XHCI_HCSPARAMS1_MAXPORTS      0xFF000000 // Number of Ports
#define B_PCH_XHCI_HCSPARAMS1_MAXINTRS      0x7FF00 // Number of Interrupters
#define B_PCH_XHCI_HCSPARAMS1_MAXSLOTS      0xFF  // Number of Device Slots

#define R_PCH_XHCI_HCSPARAMS2               0x08  // Structural Parameters 2
#define B_PCH_XHCI_HCSPARAMS2_MSB           0xF8000000 // Max Scratchpad Buffers
#define B_PCH_XHCI_HCSPARAMS2_ERSTMAX       0xF0  // Event Ring Segment Table Max
#define B_PCH_XHCI_HCSPARAMS2_IST           0x0F  // Isochronous Scheduling Threshold

#define R_PCH_XHCI_HCSPARAMS3               0x0C  // Structural Parameters 3
#define B_PCH_XHCI_HCSPARAMS3_U2DEL         0xFFFF0000 // U2 Device Exit Latency
#define B_PCH_XHCI_HCSPARAMS3_U1DEL         0x000000FF // U1 Device Exit Latency

#define R_PCH_XHCI_HCCPARAMS                0x10  // Capability Parameters
#define B_PCH_XHCI_HCCPARAMS_XECP           0xFFFF0000 // xHCI Extended Capabilities Pointer
#define B_PCH_XHCI_HCCPARAMS_MAXPSASIZE     (BIT15 | BIT14 | BIT13 | BIT12) // Maximum Primary Stream Array Size
#define B_PCH_XHCI_HCCPARAMS_PAE            BIT8  // Parst All Event Data
#define B_PCH_XHCI_HCCPARAMS_NSS            BIT7  // No Secondary SID Support
#define B_PCH_XHCI_HCCPARAMS_LTC            BIT6  // Latency Tolerance Messaging Capability
#define B_PCH_XHCI_HCCPARAMS_LHRC           BIT5  // Light HC Reset Capability
#define B_PCH_XHCI_HCCPARAMS_PIND           BIT4  // Port Indicators
#define B_PCH_XHCI_HCCPARAMS_PPC            BIT3  // Port Power Control
#define B_PCH_XHCI_HCCPARAMS_CSZ            BIT2  // Context Size
#define B_PCH_XHCI_HCCPARAMS_BNC            BIT1  // BW Negotiation Capability
#define B_PCH_XHCI_HCCPARAMS_AC64           BIT0  // 64-bit Addressing Capability

#define R_PCH_XHCI_DBOFF                    0x14  // Doorbell Offset
#define B_PCH_XHCI_DBOFF_DBAO               0xFFFFFFFC // Doorbell Array Offset

#define R_PCH_XHCI_RTSOFF                   0x18  // Runtime Register Space Offset
#define B_PCH_XHCI_RTSOFF_RTRSO             0xFFFFFFE0 // Runtime Register Space Offset

///
/// 0x80 - 0xBF - Operational Registers
///
#define R_PCH_XHCI_USBCMD                   0x80  // USB Command
#define B_PCH_XHCI_USBCMD_EU3S              BIT11 // Enable U3 MFINDEX Stop
#define B_PCH_XHCI_USBCMD_EWE               BIT10 // Enable Wrap Event
#define B_PCH_XHCI_USBCMD_CRS               BIT9  // Controller Restore State
#define B_PCH_XHCI_USBCMD_CSS               BIT8  // Controller Save State
#define B_PCH_XHCI_USBCMD_LHCRST            BIT7  // Light Host Controller Reset
#define B_PCH_XHCI_USBCMD_HSEE              BIT3  // Host System Error Enable
#define B_PCH_XHCI_USBCMD_INTE              BIT2  // Interrupter Enable
#define B_PCH_XHCI_USBCMD_HCRST             BIT1  // Host Controller Reset
#define B_PCH_XHCI_USBCMD_RS                BIT0  // Run/Stop

#define R_PCH_XHCI_USBSTS                   0x84  // USB Status
#define B_PCH_XHCI_USBSTS_HCE               BIT12 // Host Controller Error
#define B_PCH_XHCI_USBSTS_CNR               BIT11 // Controller Not Ready
#define B_PCH_XHCI_USBSTS_SRE               BIT10 // Save / Restore Error
#define B_PCH_XHCI_USBSTS_RSS               BIT9  // Restore State Status
#define B_PCH_XHCI_USBSTS_SSS               BIT8  // Save State Status
#define B_PCH_XHCI_USBSTS_PCD               BIT4  // Port Change Detect
#define B_PCH_XHCI_USBSTS_EINT              BIT3  // Event Interrupt
#define B_PCH_XHCI_USBSTS_HSE               BIT2  // Host System Error
#define B_PCH_XHCI_USBSTS_HCH               BIT0  // HC Halted

///
/// 0x480 - 0x5CF - Port Status and Control Registers
///
#define R_PCH_XHCI_PORTSC01USB2             0x480
#define R_PCH_XHCI_PORTSC02USB2             0x490
#define R_PCH_XHCI_PORTSC03USB2             0x4A0
#define R_PCH_XHCI_PORTSC04USB2             0x4B0
#define R_PCH_XHCI_PORTSC05USB2             0x4C0
#define R_PCH_XHCI_PORTSC06USB2             0x4D0
#define B_PCH_XHCI_PORTSCXUSB2_WPR          BIT31 // Warm Port Reset
#define B_PCH_XHCI_PORTSCXUSB2_DR           BIT30 // Device Removable
#define B_PCH_XHCI_PORTSCXUSB2_WOE          BIT27 // Wake on Over-Current Enable
#define B_PCH_XHCI_PORTSCXUSB2_WDE          BIT26 // Wake on Disconnect Enable
#define B_PCH_XHCI_PORTSCXUSB2_WCE          BIT25 // Wake on Connect Enable
#define B_PCH_XHCI_PORTSCXUSB2_CAS          BIT24 // Cold Attach Status
#define B_PCH_XHCI_PORTSCXUSB2_CEC          BIT23 // Port Config Error Change
#define B_PCH_XHCI_PORTSCXUSB2_PLC          BIT22 // Port Link State Change
#define B_PCH_XHCI_PORTSCXUSB2_PRC          BIT21 // Port Reset Change
#define B_PCH_XHCI_PORTSCXUSB2_OCC          BIT20 // Over-current Change
#define B_PCH_XHCI_PORTSCXUSB2_WRC          BIT19 // Warm Port Reset Change
#define B_PCH_XHCI_PORTSCXUSB2_PEC          BIT18 // Port Enabled Disabled Change
#define B_PCH_XHCI_PORTSCXUSB2_CSC          BIT17 // Connect Status Change
#define B_PCH_XHCI_PORTSCXUSB2_LWS          BIT16 // Port Link State Write Strobe
#define B_PCH_XHCI_PORTSCXUSB2_PIC          (BIT15 | BIT14) // Port Indicator Control
#define B_PCH_XHCI_PORTSCXUSB2_PS           (BIT13 | BIT12 | BIT11 | BIT10) // Port Speed
#define B_PCH_XHCI_PORTSCXUSB2_PP           BIT9  // Port Power
#define B_PCH_XHCI_PORTSCXUSB2_PLS          (BIT8 | BIT7 | BIT6 | BIT5) // Port Link State
#define B_PCH_XHCI_PORTSCXUSB2_PR           BIT4  // Port Reset
#define B_PCH_XHCI_PORTSCXUSB2_OCA          BIT3  // Over-Current Active
#define B_PCH_XHCI_PORTSCXUSB2_PED          BIT1  // Port Enabled Disabled
#define B_PCH_XHCI_PORTSCXUSB2_CCS          BIT0  // Current Connect Status

#define R_PCH_XHCI_PORTSC1USB3              0x4E0
#define B_PCH_XHCI_PORTSCXUSB3_WPR          BIT31  /// Warm Port Reset
#define B_PCH_XHCI_PORTSCXUSB3_CEC          BIT23  /// Port Config Error Change
#define B_PCH_XHCI_PORTSCXUSB3_PLC          BIT22  /// Port Link State Change
#define B_PCH_XHCI_PORTSCXUSB3_PRC          BIT21  /// Port Reset Change
#define B_PCH_XHCI_PORTSCXUSB3_OCC          BIT20  /// Over-current Chang
#define B_PCH_XHCI_PORTSCXUSB3_WRC          BIT19  /// Warm Port Reset Change
#define B_PCH_XHCI_PORTSCXUSB3_PEC          BIT18  /// Port Enabled Disabled Change
#define B_PCH_XHCI_PORTSCXUSB3_CSC          BIT17  /// Connect Status Change
#define B_PCH_XHCI_PORTSCXUSB3_PR           BIT4   /// Port Reset
#define B_PCH_XHCI_PORTSCXUSB3_PED          BIT1   /// Port Enabled / Disabled

#define R_PCH_XHCI_XECP_SUPP_USB2_2                                         0x8008  // XECP_SUPP_USB2_2 
#define R_PCH_XHCI_XECP_CMDM_CTRL_REG1                                      0x8058  // XECP_CMDM_CTRL_REG1 - Command Manager Control 1
#define R_PCH_XHCI_XECP_CMDM_CTRL_REG3                                      0x8060  // XECP_CMDM_CTRL_REG3 - Command Manager Control 3
#define R_PCH_XHCI_HOST_CTRL_SCH_REG                                        0x8094  // HOST_CTRL_SCH_REG - Host Control Scheduler
#define R_PCH_XHCI_AUX_CTRL_REG1                                            0x80E0  // AUX_CTRL_REG1 - AUX Power Management Control
#define R_PCH_XHCI_USB2_LINK_MGR_CTRL_REG1                                  0x80F0  // USB2_LINK_MGR_CTRL_REG1 - USB2 Port Link Control 1, 2, 3, 4
#define R_PCH_XHCI_USB2_LINK_MGR_CTRL_REG1_CONTROL4                         0x80FC //USB2_LINK_MGR_CTRL_REG1 - USB2 Port Link Control 1, 2, 3, 4
#define R_PCH_XHCI_HOST_CTRL_TRM_REG2                                       0x8110  // HOST_CTRL_TRM_REG2 - Host Controller Transfer Manager Control 2
#define R_PCH_XHCI_HOST_IF_PWR_CTRL_REG0                                    0x8140  // HOST_IF_PWR_CTRL_REG0 - Power Scheduler Control 0
#define R_PCH_XHCI_HOST_IF_PWR_CTRL_REG1                                    0x8144  // HOST_IF_PWR_CTRL_REG1 - Power Scheduler Control 1
#define R_PCH_XHCI_HOST_IF_PWR_CTRL_REG1_HSII                               BIT8  // HS Interrupt IN Alarm (HSII)
#define R_PCH_XHCI_AUX_CTRL_REG2                                            0x8154  // AUX_CTRL_REG2 - Aux PM Control Register 2
#define R_PCH_XHCI_USB2_PHY_POWER_MANAGEMENT_CONTROL                        0x8164  // USB2 PHY Power Management Control
#define R_PCH_XHCI_AUX_CLOCK_CTRL_REG                                       0x816C  // xHCI Aux Clock Control Register
#define R_PCH_XHCI_LATENCY_TOLERANCE_PARAMETERS_LTV_CONTROL                 0x8174  // xHCI Latency Tolerance Parameters - LTV Control
#define R_PCH_XHCI_LATENCY_TOLERANCE_PARAMETERS_HIGH_IDLE_TIME_CONTROL      0x817C  // xHC Latency Tolerance Parameters - High Idle Time Control
#define R_PCH_XHCI_LATENCY_TOLERANCE_PARAMETERS_MEDIUM_IDLE_TIME_CONTROL    0x8180  // xHC Latency Tolerance Parameters - Medium Idle Time Control
#define R_PCH_XHCI_LATENCY_TOLERANCE_PARAMETERS_LOW_IDLE_TIME_CONTROL       0x8184  // xHC Latency Tolerance Parameters - Low Idle Time Control
#define R_PCH_XHCI_HOST_CONTROLLER_MISC_REG                                 0x8188  // Host Controller Misc Reg
#define R_PCH_XHCI_THROTT                                                   0x8198  // THROTT -XHCI Throttle Control (WPT Only)
///
/// USB3 OTG PCI Config Space Registers
///
#define PCI_DEVICE_NUMBER_PCH_OTG           22
#define PCI_FUNCTION_NUMBER_PCH_OTG         0

#define R_PCH_OTG_DEVVENDID                 0x00  // Vendor ID
#define V_PCH_USB_DEVVENDID_VID             V_PCH_INTEL_VENDOR_ID

#define R_PCH_OTG_STSCMD                    0x04  // Command Status
#define B_PCH_OTG_STSCMD_INTR_DIS           BIT10 // Interrupt Disable
#define B_PCH_OTG_STSCMD_BME                BIT2  // Bus Master Enable
#define B_PCH_OTG_STSCMD_MSE                BIT1  // Memory Space Enable

#define R_PCH_OTG_BAR0                      0x10  // BAR 0
#define B_PCH_OTG_BAR0_BA                   0xFFE00000 // Base Address
#define V_PCH_OTG_BAR0_SIZE                 0x200000
#define N_PCH_OTG_BAR0_ALIGNMENT            21
#define B_PCH_OTG_BAR0_PREF                 BIT3  // Prefetchable
#define B_PCH_OTG_BAR0_ADDRNG               (BIT2 | BIT1) // Address Range
#define B_PCH_OTG_BAR0_SPTYP                BIT0  // Space Type (Memory)

#define R_PCH_OTG_BAR1                      0x14  // BAR 1
#define B_PCH_OTG_BAR1_BA                   0xFFFFF000 // Base Address
#define B_PCH_OTG_BAR1_PREF                 BIT3  // Prefetchable
#define B_PCH_OTG_BAR1_ADDRNG               (BIT2 | BIT1) // Address Range
#define B_PCH_OTG_BAR1_SPTYP                BIT0  // Space Type (Memory)
#define V_PCH_OTG_BAR1_SIZE                 (1 << 12)

#define R_PCH_OTG_SSID                      0x2C  // Sub System ID
#define B_PCH_OTG_SSID_SID                  0xFFFF0000 // Sub System ID
#define B_PCH_OTG_SSID_SVID                 0x0000FFFF // Sub System Vendor ID

#define R_PCH_OTG_PMECTLSTS                 0x84  // PME Control Status
#define B_PCH_OTG_PMECTLSTS_POWERSTATE      (BIT1 | BIT0) // Power State

///
/// USB3 OTG Private Space
///
#define PCH_OTG_PORT_ID                    0x59  // OTG Private Space PortID
#define PCH_OTG_PRIVATE_READ_OPCODE        0x06  // CUnit to OTG Private Space Read Opcode
#define PCH_OTG_PRIVATE_WRITE_OPCODE       0x07  // CUnit to OTG Private Space Write Opcode

#define R_PCH_OTG_PCICFGCTR1               0x500 // PCI Configuration Control 1
#define B_PCH_OTG_PCICFGCTR1_IPIN1         (BIT11 | BIT10 | BIT9 | BIT8) // Interrupt Pin
#define B_PCH_OTG_PCICFGCTR1_B1D1          BIT7  // BAR 1 Disable
#define B_PCH_OTG_PCICFGCTR1_PS            0x7C  // PME Support
#define B_PCH_OTG_PCICFGCTR1_ACPI_INT_EN1  BIT1  // ACPI Interrupt Enable
#define B_PCH_OTG_PCICFGCTR1_PCI_CFG_DIS1  BIT0  // PCI Configuration Space Disable

///
/// IOSF_SPXB Private Space
///
#define IOSF_SPXB_PORT_ID                  0x49  // IOSF_SPXB Private Space PortID
#define IOSF_SPXB_PRIVATE_READ_OPCODE      0x06  // IOSF_SPXB Private Space Read Opcode
#define IOSF_SPXB_PRIVATE_WRITE_OPCODE     0x07  // IOSF_SPXB Private Space Write Opcode

#define R_IOSF_SPXB_CH1CTRL                0x40  // CH1CTRL
#define B_IOSF_SPXB_TCX1                  (BIT3 | BIT2 | BIT1 | BIT0) // TCX1
#define B_IOSF_SPXB_VCX1_ENABLE           (BIT4) // VCX1_ENABLE

#define R_IOSF_SPXB_PORTCTL0              0x00  // PORTCTL0
#define B_IOSF_SPXB_PORT0_DEVICE_BW_TYPE  (BIT1 | BIT0) // PORT0_DEVICE_BW_TYPE

#define R_IOSF_SPXB_PORTCTL1              0x04  // PORTCTL1
#define B_IOSF_SPXB_PORT1_DEVICE_BW_TYPE  (BIT1 | BIT0) // PORT1_DEVICE_BW_TYPE

#define R_IOSF_SPXB_PORTCTL2              0x8  // PORTCTL2
#define B_IOSF_SPXB_PORT2_DEVICE_BW_TYPE  (BIT1 | BIT0) // PORT2_DEVICE_BW_TYPE

#define R_IOSF_SPXB_PMCTL                 0x84  // PMCTL
#define B_IOSF_SPXB_PMCTL_CLK_GATE_EN     (BIT1) // IOSFCLK_GATE_EN
#define B_IOSF_SPXB_PMCTL_CLK_US_GATE_EN  (BIT2) // SPXBCLK_US_GATE_EN
#define B_IOSF_SPXB_PMCTL_CLK_DS_GATE_EN  (BIT3) // SPXBCLK_DS_GATE_EN
#define B_IOSF_SPXB_PMCTL_IOSFSB_GATE_EN  (BIT4) // IOSFSB_GATE_EN
#define B_IOSF_SPXB_PMCTL_SB_TK_GT_EN     (BIT7) // IOSFSB_TRUNK_GATE_EN

#define R_IOSF_SPXB_BDGDBGCTL             0x118  // BDGDBGCTL

#define R_IOSF_SPXB_BDGCTL                0x11C  // BDGCTL
#define B_IOSF_SPXB_IOSF_PASS_THROUGH     (BIT0) //SPXB_IOSF_PASS_THROUGH
#define B_IOSF_SPXB_PASS_THROUGH          (BIT1) //IOSF_SPXB_PASS_THROUGH
#define B_IOSF_SPXB_GNT_CTR_RESET_CLK     (BIT3 | BIT2) //GNT_CTR_RESET_CLK

#define PCH_USH_PHY_PORT_ID                0x61  // CDN PHY  FOR XHCI
#define PCH_USH_PHY_READ_OPCODE            0x06  // Read Opcode
#define PCH_USH_PHY_WRITE_OPCODE           0x07  // Write Opcode

#define R_PCH_CDN_U1_POWER_STATE_DEFINITION               0x02
#define B_PCH_CDN_TX_EN                                   (BIT2)

#define R_PCH_CDN_VCO_START_CALIBRATION_START_POINT       0x56
#define B_PCH_CDN_VCO_START_CALIBRATION_START_POINT_VALUE 0x1F

#define R_PCH_CDN_PLL_CONTROL                             0x3C2
#define R_PCH_CDN_PLL_VCO_CALIBRATION_TRIM_CODE           (BIT6 | BIT5 | BIT4)

//USB2 PHY (USB2SHIP) side band registers
#define PCH_USB2_PHY_PORT_ID                              0x43  // USB2SHIP Private Space PortID
#define PCH_USB2_PHY_READ_OPCODE                          0x06  // Read Opcode
#define PCH_USB2_PHY_WRITE_OPCODE                         0x07  // Write Opcode

#define R_PCH_USB2_PHY_USB2_PER_PORT_LANE0                          0x4100
#define R_PCH_USB2_PHY_USB2_PER_PORT_LANE1                          0x4200
#define R_PCH_USB2_PHY_USB2_PER_PORT_LANE2                          0x4300
#define R_PCH_USB2_PHY_USB2_PER_PORT_LANE3                          0x4400
#define R_PCH_USB2_PHY_USB2_PERPORT_RCOMP_HS_PULLUP_REGISTER_LANE0  0x4122
#define R_PCH_USB2_PHY_USB2_PERPORT_RCOMP_HS_PULLUP_REGISTER_LANE1  0x4222
#define R_PCH_USB2_PHY_USB2_PERPORT_RCOMP_HS_PULLUP_REGISTER_LANE2  0x4322
#define R_PCH_USB2_PHY_USB2_PERPORT_RCOMP_HS_PULLUP_REGISTER_LANE3  0x4422

#define R_PCH_USB2_PHY_USB2_COMPBG                        0x7F04
#define R_PCH_USB2_PHY_USB2_COMPBG2                       0x7F14

#define R_PCH_OTG_GEN_REGRW1                              0xA0
#define B_PCH_OTG_ULPIPHY_REFCLK_DISABLE                  BIT17

//memory space registers 
#define R_PCH_OTG_VCO_START_CALIBRATION_START_POINT       0x1000AC
#define B_PCH_OTG_VCO_START_CALIBRATION_START_POINT_VALUE 0x1F

#define R_PCH_OTG_U1_POWER_STATE_DEFINITION               0x100004
#define B_PCH_OTG_U1_POWER_STATE_DEFINITION_TX_EN         (BIT2)

#define R_PCH_OTG_PLL_CONTROL                             0x100784
#define R_PCH_OTG_PLL_VCO_CALIBRATION_TRIM_CODE           (BIT6 | BIT5 | BIT4)
#endif
