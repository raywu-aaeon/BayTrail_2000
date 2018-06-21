//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2017, American Megatrends, Inc.            **
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
// Name:        SbPeiBoard.C
//
// Description: This file contains PEI stage board component code for
//              Template SB
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiPeiLib.h>
#include <Sb.h>
#include <Ppi/CspLibPpi.h>
#include <setup.h>
#include <PchRegs.h>
#include <platformBaseAddresses.h>
#include <PchAccess.h>
#include "CmosMap.h"
#include <Library/SbPolicy.h>
#include <Library/ScPolicyInitPei.h>
#if CMOS_MANAGER_SUPPORT == 1
#include <CmosAccess.h>
#include <SspTokens.h>
#endif //#if CMOS_MANAGER_SUPPORT == 1
#include <Ppi/PchUsbPolicy.h>
#include <Ppi/PchInit.h>
#include <Library/PchPlatformLib.h>    //(EIP120879+)

// Produced PPIs

// GUID Definitions

// Portable Constants

// Function Prototypes

// PPI interface definition
AMI_PEI_PCI_INIT_TABLE_STRUCT stSBH2P_PCIInitTable [] = {
//  { Register, AND Mask, OR Mask },
    {0x00, 0x00, 0x00 }
//  { Register, AND Mask, OR Mask}
};
UINT16  wSBH2P_PCIInitTableSize = sizeof(stSBH2P_PCIInitTable)/sizeof(AMI_PEI_PCI_INIT_TABLE_STRUCT);

// Function Definition
VOID
IchRcrbInit (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN SB_SETUP_DATA                *SystemConfiguration
  )
{
//  UINT8                           LpcRevisionID;
    EFI_BOOT_MODE                   BootMode;

    (*PeiServices)->GetBootMode(PeiServices, &BootMode);

    //
    // If not recovery or flash update boot path. set the BIOS interface lock down bit.
    // It locks the top swap bit and BIOS boot strap bits from being changed.
    //
    if((BootMode != BOOT_IN_RECOVERY_MODE) && (BootMode != BOOT_ON_FLASH_UPDATE)) {
        MmioOr8(RCBA_BASE_ADDRESS + R_PCH_RCRB_GCS, B_PCH_RCRB_GCS_BILD);
    }

    //
    // Disable the Watchdog timer expiration from causing a system reset
    //
    MmioOr8(PMC_BASE_ADDRESS + R_PCH_PMC_PM_CFG, B_PCH_PMC_PM_CFG_NO_REBOOT);

    //
    // Initial RCBA according to the PeiRCBA table
    //
//  LpcRevisionID = PchLpcPciCfg8 (R_PCH_LPC_RID_CC);

    if((BootMode == BOOT_ON_S3_RESUME)) {
        //
        // We are resuming from S3
        // Enable HPET if enabled in Setup
        // ICH Config register Offset 0x3404 bit 7 (Enable) = 1,
        // Bit 1:0 (Mem I/O address) = 0 (0xFED00000)
        //
        MmioOr8(R_PCH_PCH_HPET + R_PCH_PCH_HPET_GCFG, B_PCH_PCH_HPET_GCFG_EN);
    }
}

VOID
PchInitInterrupt(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
)
{
//CSP20130930>>
    UINT32  Buffer32=0;

    if(PchPolicyData->LpssPciModeEnabled == 1) {
      //
      // Configure LPSS Interrupts
      //
      if(PchPolicyData->LpssDma0Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FABCTLP0,
              Buffer32,
              ((UINT32) ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN)),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_A,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssPwm0Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FABCTLP1,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_D,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssPwm1Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FABCTLP2,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_B,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssHsuart0Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FABCTLP3,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_C,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssHsuart1Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FABCTLP4,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_A,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssSpiEnabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FABCTLP5,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_D,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssDma1Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP0,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_A,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C0Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP1,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_C,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C1Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP2,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_D,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C2Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP3,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_B,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C3Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP4,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_A,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C4Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP5,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_C,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C5Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP6,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_D,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
  
      if(PchPolicyData->LpssI2C6Enabled == 1) {
          PchMsgBusAndThenOr32(
              PCH_LPSS_EP_PORT_ID,
              R_PCH_LPSS_FAB2CTLP7,
              Buffer32,
              ~(B_PCH_LPSS_FABXCTLPX_ACPI_INTR_EN | B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS | B_PCH_LPSS_FABXCTLPX_FABCTLIPIN),
              V_PCH_LPSS_FABXCTLPX_FABCTLIPIN_B,
              PCH_LPSS_EP_PRIVATE_READ_OPCODE,
              PCH_LPSS_EP_PRIVATE_WRITE_OPCODE
          );
      }
    }
//CSP20130930<<
    //
    // Program Interrupt routing registers
    //
    //
    // Device 31 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D31IR),
        V_PCH_ILB_DXXIR_IBR_PIRQC    // For SMBUS
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D31IR); // Read Posted Writes Register

    //
    // Device 30 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D30IR),
        V_PCH_ILB_DXXIR_IAR_PIRQD +
        V_PCH_ILB_DXXIR_IBR_PIRQB +
        V_PCH_ILB_DXXIR_ICR_PIRQC +
        V_PCH_ILB_DXXIR_IDR_PIRQA
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D30IR); // Read Posted Writes Register

    //
    // Device 29 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D29IR),
        V_PCH_ILB_DXXIR_IAR_PIRQH    // For EHCI #1
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D29IR); // Read Posted Writes Register

    //
    // Device 28 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D28IR),
        V_PCH_ILB_DXXIR_IAR_PIRQA +  // For PCIe #1
        V_PCH_ILB_DXXIR_IBR_PIRQB +  // For PCIe #2
        V_PCH_ILB_DXXIR_ICR_PIRQC +  // For PCIe #3
        V_PCH_ILB_DXXIR_IDR_PIRQD    // For PCIe #4
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D28IR); // Read Posted Writes Register

    //
    // Device 27 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D27IR),
        V_PCH_ILB_DXXIR_IAR_PIRQG    // For Azalia
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D27IR); // Read Posted Writes Register

    //
    // Device 26 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D26IR),
        V_PCH_ILB_DXXIR_IAR_PIRQF    // For SEC
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D26IR); // Read Posted Writes Register

    //
    // Device 25 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D25IR),
        V_PCH_ILB_DXXIR_IAR_PIRQE    // For GBe
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D25IR); // Read Posted Writes Register

    //
    // Device 24 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D24IR),
        V_PCH_ILB_DXXIR_IAR_PIRQB |  // For LPSS2
        V_PCH_ILB_DXXIR_IBR_PIRQA |
        V_PCH_ILB_DXXIR_ICR_PIRQD |
        V_PCH_ILB_DXXIR_IDR_PIRQC
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D24IR); // Read Posted Writes Register

    //
    // Device 23 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D23IR),
        V_PCH_ILB_DXXIR_IAR_PIRQH   // For HSI
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D23IR); // Read Posted Writes Register

    //
    // Device 22 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D22IR),
        V_PCH_ILB_DXXIR_IAR_PIRQG   // For OTG
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D22IR); // Read Posted Writes Register

    //
    // Device 21 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D21IR),
        V_PCH_ILB_DXXIR_IAR_PIRQF   // For LPE
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D21IR); // Read Posted Writes Register

    //
    // Device 20 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D20IR),
        V_PCH_ILB_DXXIR_IAR_PIRQE   // For xHCI
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D20IR); // Read Posted Writes Register

    //
    // Device 19 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D19IR),
        V_PCH_ILB_DXXIR_IAR_PIRQD   // For SATA
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D19IR); // Read Posted Writes Register

    //
    // Device 18 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D18IR),
        V_PCH_ILB_DXXIR_IAR_PIRQC   // For SDIO #2
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D18IR); // Read Posted Writes Register

    //
    // Device 17 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D17IR),
        V_PCH_ILB_DXXIR_IAR_PIRQB   // For SDIO #1
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D17IR); // Read Posted Writes Register

    //
    // Device 16 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D16IR),
        V_PCH_ILB_DXXIR_IAR_PIRQA   // For SDIO #0
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D16IR); // Read Posted Writes Register

    PEI_TRACE((-1, PeiServices, "PchInitInterrupt () - End\n"));
}

#define PCI_LPC_BASE    (0x8000F800)
#define PCI_LPC_REG(x)  (PCI_LPC_BASE + (x))
//#define PCIEX_BASE_ADDRESS                        0xE0000000
#define PciD31F0RegBase                           PCIEX_BASE_ADDRESS + (UINT32) (31 << 15)
#define V_PCH_ILB_IRQE_UARTIRQEN_IRQ3             BIT3 // UART IRQ3 Enable

//(EIP120879+)>>
//CSP20130712 >>
#if MMC_SUPPORT
EFI_STATUS
ConfigureSoCGpio (
    IN CONST EFI_PEI_SERVICES       **PeiServices,		
    IN SB_SETUP_DATA                *PchPolicyData
  )
{
	UINT8	eMMCSelect;
	PEI_TRACE((-1, PeiServices, "ConfigureSoCGpio------------start\n"));
    if (PchPolicyData->eMMCEnabled== 1) {// Auto detection mode
	    PEI_TRACE((-1, PeiServices, "Auto detection mode------------start\n")); 
     switch (PchStepping()) {
       case PchA0:
       case PchA1:
	  PEI_TRACE((-1, PeiServices, "eMMC 4.41 GPIO Configuration\n"));
	  eMMCSelect = 0;
         break;
       case PchB0:
	//CSP20130910 >>	   
       case PchB1:  
       case PchB2:
	 PEI_TRACE((-1, PeiServices,"eMMC 4.5 GPIO Configuration\n"));
	 eMMCSelect = 1;
         break;
       default:
	 PEI_TRACE((-1, PeiServices, "Unknown Steppting, eMMC 4.5 GPIO Configuration\n"));
	 eMMCSelect = 1;
	 //CSP20130910 <<	   
         break;
     }
    }else if (PchPolicyData->eMMCEnabled == 2) { // eMMC 4.41 
	PEI_TRACE((-1, PeiServices, "eMMC 4.41 GPIO Configuration\n"));
	eMMCSelect = 0;
    } else if (PchPolicyData->eMMCEnabled == 3) { // eMMC 4.5
	 PEI_TRACE((-1, PeiServices,"eMMC 4.5 GPIO Configuration\n"));
	 eMMCSelect = 1;
        
    } else { // eMMC 4.41 controllers
        PEI_TRACE((-1, PeiServices, "eMMC 4.41 GPIO controllers\n"));
        eMMCSelect = 0;
    }
  
    //---------------------------------------------
  /*
  20.1.1  EMMC
  SDMMC1_CLK -         write 0x2003ED01 to IOBASE + 0x03E0
  SDMMC1_CMD -        write 0x2003EC81 to IOBASE + 0x0390
  SDMMC1_D0 -           write 0x2003EC81 to IOBASE + 0x03D0
  SDMMC1_D1 -           write 0x2003EC81 to IOBASE + 0x0400
  SDMMC1_D2 -           write 0x2003EC81 to IOBASE + 0x03B0
  SDMMC1_D3_CD_B - write 0x2003EC81 to IOBASE + 0x0360
  MMC1_D4_SD_WE -   write 0x2003EC81 to IOBASE + 0x0380
  MMC1_D5 -                write 0x2003EC81 to IOBASE + 0x03C0
  MMC1_D6 -                write 0x2003EC81 to IOBASE + 0x0370
  MMC1_D7 -                write 0x2003EC81 to IOBASE + 0x03F0
  MMC1_RESET_B -       write 0x2003ED01 to IOBASE + 0x0330
  */                    
  if (eMMCSelect == 0) {
    MmioWrite32 (IO_BASE_ADDRESS + 0x03E0, 0x2003ED01); //EMMC 4.41
    MmioWrite32 (IO_BASE_ADDRESS + 0x0390, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x03D0, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0400, 0x2003EC81);           
    MmioWrite32 (IO_BASE_ADDRESS + 0x03B0, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0360, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0380, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x03C0, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0370, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x03F0, 0x2003EC81); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0330, 0x2003ED01); 
  }
  
  /*
  eMMC 4.5 controller    
  SDMMC1_CLK -         write 0x2003ED03 to IOBASE + 0x03E0
  SDMMC1_CMD -        write 0x2003EC83 to IOBASE + 0x0390
  SDMMC1_D0 -           write 0x2003EC83 to IOBASE + 0x03D0
  SDMMC1_D1 -           write 0x2003EC83 to IOBASE + 0x0400
  SDMMC1_D2 -           write 0x2003EC83 to IOBASE + 0x03B0
  SDMMC1_D3_CD_B -  write 0x2003EC83 to IOBASE + 0x0360
  MMC1_D4_SD_WE -   write 0x2003EC83 to IOBASE + 0x0380
  MMC1_D5 -                write 0x2003EC83 to IOBASE + 0x03C0
  MMC1_D6 -                write 0x2003EC83 to IOBASE + 0x0370
  MMC1_D7 -                write 0x2003EC83 to IOBASE + 0x03F0
  MMC1_RESET_B -       write 0x2003ED03 to IOBASE + 0x0330
  */                    
  if (eMMCSelect == 1) {
    MmioWrite32 (IO_BASE_ADDRESS + 0x03E0, 0x2003ED03); // EMMC 4.5
    MmioWrite32 (IO_BASE_ADDRESS + 0x0390, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x03D0, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0400, 0x2003EC83);           
    MmioWrite32 (IO_BASE_ADDRESS + 0x03B0, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0360, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0380, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x03C0, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0370, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x03F0, 0x2003EC83); 
    MmioWrite32 (IO_BASE_ADDRESS + 0x0330, 0x2003ED03);   
    
  } 
  PEI_TRACE((-1, PeiServices, "ConfigureSoCGpio------------end\n"));     
  return EFI_SUCCESS;
}
#endif
//CSP20130712 <<
//(EIP120879+)<<

EFI_STATUS
ConfigureLpssAndSccGpio(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
)
{
    /*One time configuration to each GPIO controller PSB_CONF register should be done before starting pad configuration:
    GPIO SCORE -  write 0x01001002 to IOBASE + 0x0700
    GPIO NCORE -  write 0x01001002 to IOBASE + 0x0F00
    GPIO SSUS -    write 0x01001002 to IOBASE + 0x1700
    */
    PEI_TRACE((-1, PeiServices, "ConfigureLpssAndSccGpio------------start\n"));
    /*
    #ifndef AMI_SYSCFG_OVERRIDE
        PchPolicyData->LpssPwm0Enabled = 1;
        PchPolicyData->LpssPwm1Enabled = 1;
        PchPolicyData->LpssHsuart0Enabled = 1;
        PchPolicyData->LpssHsuart1Enabled = 1;
        PchPolicyData->LpssSpiEnabled = 1;
        PchPolicyData->LpssI2C0Enabled = 1;
        PchPolicyData->LpssI2C1Enabled = 1;
        PchPolicyData->LpssI2C2Enabled = 1;
        PchPolicyData->LpssI2C3Enabled = 1;
        PchPolicyData->LpssI2C4Enabled = 1;
        PchPolicyData->LpssI2C5Enabled = 1;
        PchPolicyData->LpssI2C6Enabled = 1;
        PchPolicyData->eMMCEnabled = 1;
        PchPolicyData->SdioEnabled = 1;
        PchPolicyData->SdcardEnabled = 1;
        PchPolicyData->MipiHsi = 1;
    #endif
    */
    /*
    19.1.1  PWM0
    PWM0 - write 0x2003CD01 to IOBASE + 0x00A0
    19.1.2  PWM1
    PWM0 - write 0x2003CD01 to IOBASE + 0x00B0
    */
    if(PchPolicyData->LpssPwm0Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x00A0, 0x2003CD01);
    }
    if(PchPolicyData->LpssPwm1Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x00B0, 0x2003CD01);
    }

    /*
    19.1.3  UART1
    UART1_RXD-L -     write 0x2003CC81 to IOBASE + 0x0020
    UART1_TXD-0 -     write 0x2003CC81 to IOBASE + 0x0010
    UART1_RTS_B-1 - write 0x2003CC81 to IOBASE + 0x0000
    UART1_CTS_B-H - write 0x2003CC81 to IOBASE + 0x0040
    */
    if(PchPolicyData->LpssHsuart0Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0020, 0x2003CC81);  // uart1
        MmioWrite32(IO_BASE_ADDRESS + 0x0010, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0000, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0040, 0x2003CC01);//W/A HSD 4752617 0x2003CC81 
    }

    /*
    19.1.4  UART2
    UART2_RTS_B-1 -  write 0x2003CC81 to IOBASE + 0x0090
    UART2_CTS_B-H - write 0x2003CC81 to IOBASE + 0x0080
    UART2_RXD-H -    write 0x2003CC81 to IOBASE + 0x0060
    UART2_TXD-0 -     write 0x2003CC81 to IOBASE + 0x0070
    */
    if(PchPolicyData->LpssHsuart1Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0090, 0x2003CC81);  // uart2
        MmioWrite32(IO_BASE_ADDRESS + 0x0080, 0x2003CC01);//W/A HSD 4752617 0x2003CC81
        MmioWrite32(IO_BASE_ADDRESS + 0x0060, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0070, 0x2003CC81);
    }

    /*
    19.1.5  SPI
    SPI1_CS0_B - write 0x2003CC81 to IOBASE + 0x0110
    SPI1_CLK -     write 0x2003CD01 to IOBASE + 0x0100
    SPI1_MOSI -   write 0x2003CC81 to IOBASE + 0x0130
    SPI1_MISO -   write 0x2003CC81 to IOBASE + 0x0120
    */
    if(PchPolicyData->LpssSpiEnabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0110, 0x2003CC81);  // SPI
        MmioWrite32(IO_BASE_ADDRESS + 0x0100, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x0130, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0120, 0x2003CC81);
    }

    /*
    19.1.6  I2C0
    I2C0_SDA-OD-O -    write 0x2003CC81 to IOBASE + 0x0210
    I2C0_SCL-OD-O -    write 0x2003CC81 to IOBASE + 0x0200
    */
    if(PchPolicyData->LpssI2C0Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0210, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0200, 0x2003CC81);
    }
    /*
    19.1.7  I2C1
    I2C1_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01F0
    I2C1_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01E0
    */

    if(PchPolicyData->LpssI2C1Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x01F0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x01E0, 0x2003CC81);
    }
    /*
    19.1.8  I2C2
    I2C2_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01D0
    I2C2_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01B0
    */
    if(PchPolicyData->LpssI2C2Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x01D0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x01B0, 0x2003CC81);
    }
    /*
    19.1.9  I2C3
    I2C3_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0190
    I2C3_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01C0
    */
    if(PchPolicyData->LpssI2C3Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0190, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x01C0, 0x2003CC81);
    }
    /*
    19.1.10 I2C4
    I2C4_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01A0
    I2C4_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x0170
    */
    if(PchPolicyData->LpssI2C4Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x01A0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0170, 0x2003CC81);
    }
    /*
    19.1.11 I2C5
    I2C5_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0150
    I2C5_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x0140
    */
    if(PchPolicyData->LpssI2C5Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0150, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0140, 0x2003CC81);
    }
    /*
    19.1.12 I2C6
    I2C6_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0180
    I2C6_SCL-OD-O/I -  write 0x2003CC81 to IOBASE + 0x0160
    */
    if(PchPolicyData->LpssI2C6Enabled == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0180, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0160, 0x2003CC81);
    }

    //---------------------------------------------
    /*
    20.1.1  EMMC
    SDMMC1_CLK -         write 0x2003ED01 to IOBASE + 0x03E0
    SDMMC1_CMD -        write 0x2003EC81 to IOBASE + 0x0390
    SDMMC1_D0 -           write 0x2003EC81 to IOBASE + 0x03D0
    SDMMC1_D1 -           write 0x2003EC81 to IOBASE + 0x0400
    SDMMC1_D2 -           write 0x2003EC81 to IOBASE + 0x03B0
    SDMMC1_D3_CD_B - write 0x2003EC81 to IOBASE + 0x0360
    MMC1_D4_SD_WE -   write 0x2003EC81 to IOBASE + 0x0380
    MMC1_D5 -                write 0x2003EC81 to IOBASE + 0x03C0
    MMC1_D6 -                write 0x2003EC81 to IOBASE + 0x0370
    MMC1_D7 -                write 0x2003EC81 to IOBASE + 0x03F0
    MMC1_RESET_B -       write 0x2003ED01 to IOBASE + 0x0330

    if(PchPolicyData->eMMCEnabled) {    //(CSP20130111A+)
        MmioWrite32(IO_BASE_ADDRESS + 0x03E0, 0x2003ED01);  //EMMC
        MmioWrite32(IO_BASE_ADDRESS + 0x0390, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x03D0, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0400, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x03B0, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0360, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0380, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x03C0, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0370, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x03F0, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0330, 0x2003ED01);
    }
    */
    /*
    20.1.2  SDIO
    SDMMC2_CLK -  write 0x2003ED01 to IOBASE + 0x0320
    SDMMC2_CMD - write 0x2003EC81 to IOBASE + 0x0300
    SDMMC2_D0 -    write 0x2003EC81 to IOBASE + 0x0350
    SDMMC2_D1 -    write 0x2003EC81 to IOBASE + 0x02F0
    SDMMC2_D2 -    write 0x2003EC81 to IOBASE + 0x0340
    SDMMC2_D3_CD_B - write 0x2003EC81 to IOBASE + 0x0310
    */
    if (PchPolicyData->SdioEnabled== 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x0320, 0x2003ED01); //SDIO
        MmioWrite32(IO_BASE_ADDRESS + 0x0300, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0350, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x02F0, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0340, 0x2003EC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0310, 0x2003EC81);
    }

    /*
    20.1.3  SD Card
    SDMMC3_1P8_EN - write 0x2003CD01 to IOBASE + 0x03F0
    SDMMC3_CD_B -    write 0x2003CC81 to IOBASE + 0x03A0
    SDMMC3_CLK -       write 0x2003CD01 to IOBASE + 0x02B0
    SDMMC3_CMD -      write 0x2003CC81 to IOBASE + 0x02C0
    SDMMC3_D0 -         write 0x2003CC81 to IOBASE + 0x02E0
    SDMMC3_D1 -         write 0x2003CC81 to IOBASE + 0x0290
    SDMMC3_D2 -         write 0x2003CC81 to IOBASE + 0x02D0
    SDMMC3_D3 -         write 0x2003CC81 to IOBASE + 0x02A0
    SDMMC3_PWR_EN_B - write 0x2003CC81 to IOBASE + 0x0690
    SDMMC3_WP -            write 0x2003CC82 to IOBASE + 0x0160
    */
    if (PchPolicyData->SdcardEnabled== 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x05F0, 0x2003CD01); //SDCARD
//        MmioWrite32 (IO_BASE_ADDRESS + 0x03A0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x02B0, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x02C0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x02E0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0290, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x02D0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x02A0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0690, 0x2003CC81);
//        MmioWrite32 (IO_BASE_ADDRESS + 0x0160, 0x2003CC82);
    }

    /*
    20.1.4  MIPI HSI
    MHSI_ACDATA -    write 0x2003CD01 to IOBASE + 0x04D0
    MHSI_ACFLAG -    write 0x2003CC81 to IOBASE + 0x04F0
    MHSI_ACREADY -  write 0x2003CD01 to IOBASE + 0x0530
    MHSI_ACWAKE -   write 0x2003CD01 to IOBASE + 0x04E0
    MHSI_CADATA -   write 0x2003CD01 to IOBASE + 0x0510
    MHSI_CAFLAG -    write 0x2003CD01 to IOBASE + 0x0500
    MHSI_CAREADY -  write 0x2003CD01 to IOBASE + 0x0520
    MHSI_CAWAKE(#1, SPKR) -             write 0x2003CD02 to IOBASE + 0x0670
    MHSI_CAWAKE(#2, GP_SSP_2_FS) - write 0x2003CD02 to IOBASE + 0x00C0

    if(PchPolicyData->MipiHsi == 1) {
        MmioWrite32(IO_BASE_ADDRESS + 0x04D0, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x04F0, 0x2003CC81);
        MmioWrite32(IO_BASE_ADDRESS + 0x0530, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x04E0, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x0510, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x0500, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x0520, 0x2003CD01);
        MmioWrite32(IO_BASE_ADDRESS + 0x0670, 0x2003CD02);
        MmioWrite32(IO_BASE_ADDRESS + 0x00C0, 0x2003CD02);
    }
    */
    PEI_TRACE((-1, PeiServices, "ConfigureLpssAndSccGpio------------end\n"));

    return EFI_SUCCESS;
}

VOID
UARTInit(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
)
{
    //UINT32        Data32;
    if(0) {  // for fix cr4 issue
        //if (PchPolicyData->UartInterface == 0){
        //
        // Program and enable PMC Base.
        //
        IoWrite32(0xCF8,  PCI_LPC_REG(R_PCH_LPC_PMC_BASE));
        IoWrite32(0xCFC, (PMC_BASE_ADDRESS | B_PCH_LPC_PMC_BASE_EN));

        if((PchPolicyData->PcuUart1 == 1) &&
                (PchPolicyData->LpssHsuart0Enabled == 0)) {
            //
            // Enable COM1 for debug message output.
            //
            MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, BIT24);

            //Enable internal UART3 port(COM1)
            MmioOr8(ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) V_PCH_ILB_IRQE_UARTIRQEN_IRQ3);
            MmioOr32(IO_BASE_ADDRESS + 0x0520, 0x01);  // UART3_RXD-L
            MmioOr32(IO_BASE_ADDRESS + 0x0530, 0x01);  // UART3_TXD-0
            MmioOr8(PciD31F0RegBase + R_PCH_LPC_UART_CTRL, (UINT8) B_PCH_LPC_UART_CTRL_COM1_EN);
        } else {
            //Disable UART3(COM1)
            MmioAnd8(ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) ~V_PCH_ILB_IRQE_UARTIRQEN_IRQ3);
            MmioAnd32(IO_BASE_ADDRESS + 0x0520, ~(UINT32)0x07);
            MmioAnd32(IO_BASE_ADDRESS + 0x0530, ~(UINT32)0x07);
            MmioAnd8(PciD31F0RegBase + R_PCH_LPC_UART_CTRL, (UINT8) ~B_PCH_LPC_UART_CTRL_COM1_EN);

            if(PchPolicyData->LpssHsuart0Enabled == 1) {
                //
                //Valleyview BIOS Specification Vol2,17.2
                //LPSS_UART1 ¡LC set each pad PAD_CONF0.Func_Pin_Mux to function 1:
                //
                MmioAnd8(IO_BASE_ADDRESS + 0x0090, (UINT8)~0x07);
                MmioOr8(IO_BASE_ADDRESS + 0x0090, 0x01);
                MmioAnd8(IO_BASE_ADDRESS + 0x00D0, (UINT8)~0x07);
                MmioOr8(IO_BASE_ADDRESS + 0x00D0, 0x01);

                //
                //Valleyview BIOS Specification Vol2,22.1.4
                //
                //MmioWrite32(IO_BASE_ADDRESS + 0x0090, 0x0000CD29);
                //MmioWrite32(IO_BASE_ADDRESS + 0x00D0, 0x0000CD29);
                //MmioWrite32(IO_BASE_ADDRESS + 0x0110, 0x0000CCA9);
                //MmioWrite32(IO_BASE_ADDRESS + 0x0150, 0x0000CCA9);
            }
        }

        PEI_TRACE((-1, PeiServices, "EnableInternalUart\n"));
    } else {
        // If SIO UART interface selected
        //Disable internal UART port(COM1)
        if(0) {
            ; // For fix CR4 issue
            MmioAnd8(ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) ~V_PCH_ILB_IRQE_UARTIRQEN_IRQ3);
            MmioAnd8(IO_BASE_ADDRESS + 0x0090, (UINT8)~0x07);
            MmioAnd8(IO_BASE_ADDRESS + 0x00D0, (UINT8)~0x07);
            MmioAnd8(PciD31F0RegBase + R_PCH_LPC_UART_CTRL, (UINT8) ~B_PCH_LPC_UART_CTRL_COM1_EN);

            //MmioAnd8 (ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) ~V_PCH_ILB_IRQE_UARTIRQEN_IRQ4);
            //MmioAnd8 (IO_BASE_ADDRESS + 0x0110, (UINT8)~0x07);
            //MmioAnd8 (IO_BASE_ADDRESS + 0x0150, (UINT8)~0x07);
            //MmioOr8 (PciD31F0RegBase + R_PCH_LPC_UART_CTRL, (UINT8) ~B_PCH_LPC_UART_CTRL_COM2_EN);
        }
    }
}

VOID
WriteCmosBank1Byte(
    IN UINT8                     Address,
    IN UINT8                     Data
)
{
    IoWrite8(R_PCH_RTC_EXT_INDEX, Address);
    IoWrite8(R_PCH_RTC_EXT_TARGET, Data);
}

VOID
ClearSmiAndWake(
    VOID
)
/*++

Routine Description:

  Clear any SMI status or wake status left over from boot.

Arguments:

Returns:

  None.

--*/
{
    UINT16  Pm1Sts;
//  UINT32  Pm1Cnt;
    UINT32  Gpe0Sts;
    UINT32  SmiSts;

    //
    // Read the ACPI registers
    //
    Pm1Sts  = IoRead16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS);
//  Pm1Cnt  = IoRead32 (ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT);
    Gpe0Sts = IoRead32(ACPI_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS);
    SmiSts  = IoRead32(ACPI_BASE_ADDRESS + R_PCH_SMI_STS);

    //
    // Register Wake up reason for S4.  This information is used to notify
    // WinXp of wake up reason because S4 wake up path doesn't keep SCI.
    // This is important for Viiv(Quick resume) platform.
    //

    //
    // First Clear CMOS S4 Wake up flag.
    //
    CmosWriteByte(SB_CMOS_S4_WAKEUP_FLAG_ADDRESS, 0x00);

    //
    // Check wake up reason and set CMOS accordingly.  Currently checks
    // Power button, USB, PS/2.
    // Note : PS/2 wake up is using GPI13 (IO_PME).  This must be changed depending
    // on board design.
    //
    if((Pm1Sts & B_PCH_ACPI_PM1_STS_PWRBTN) || (Gpe0Sts & (B_PCH_ACPI_GPE0a_STS_CORE_GPIO | B_PCH_ACPI_GPE0a_STS_SUS_GPIO))) {
        CmosWriteByte(SB_CMOS_S4_WAKEUP_FLAG_ADDRESS, 0x01);
    }

    //
    // Clear any SMI or wake state from the boot
    //
    Pm1Sts = (B_PCH_ACPI_PM1_STS_PRBTNOR | B_PCH_ACPI_PM1_STS_PWRBTN);

    Gpe0Sts |=
        (
            B_PCH_ACPI_GPE0a_STS_CORE_GPIO |
            B_PCH_ACPI_GPE0a_STS_SUS_GPIO |
            B_PCH_ACPI_GPE0a_STS_PME_B0 |
            B_PCH_ACPI_GPE0a_STS_BATLOW |
            B_PCH_ACPI_GPE0a_STS_PCI_EXP |
            B_PCH_ACPI_GPE0a_STS_GUNIT_SCI |
            B_PCH_ACPI_GPE0a_STS_PUNIT_SCI |
            B_PCH_ACPI_GPE0a_STS_SWGPE |
            B_PCH_ACPI_GPE0a_STS_HOT_PLUG
        );

    SmiSts |=
        (
            BIT16 |
            B_PCH_SMI_STS_PERIODIC |
            B_PCH_SMI_STS_TCO |
            B_PCH_SMI_STS_SWSMI_TMR |
            B_PCH_SMI_STS_APM |
            B_PCH_SMI_STS_ON_SLP_EN |
            B_PCH_SMI_STS_BIOS
        );

    //
    // Write them back
    //
    IoWrite16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS, Pm1Sts);
//  IoWrite32 (ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, Pm1Cnt);
    IoWrite32(ACPI_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS, Gpe0Sts);
    IoWrite32(ACPI_BASE_ADDRESS + R_PCH_SMI_STS, SmiSts);
}

VOID
ClearPowerState()
{
    UINT8   Data8;
    UINT16  Data16;
    UINT32  Data32;

    //
    // Check for PowerState option for AC power loss and program the chipset
    //

    //
    // Clear PWROK (Set to Clear)
    //
    MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, B_PCH_PMC_GEN_PMCON_PWROK_FLR);

    //
    // Clear Power Failure Bit (Set to Clear)
    //
    MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR);

    //
    // Clear the GPE and PM enable
    //
    IoWrite32(ACPI_BASE_ADDRESS , (UINT32) 0x00); //EIP145306
    IoWrite32(ACPI_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, (UINT32) 0x00);

    //
    // Halt the TCO timer
    //
    Data16 = IoRead16(ACPI_BASE_ADDRESS + R_PCH_TCO_CNT);
    Data16 |= B_PCH_TCO_CNT_TMR_HLT;
    IoWrite16(ACPI_BASE_ADDRESS + R_PCH_TCO_CNT, Data16);

#if defined(LPC_WA_ENABLE_LPC_CLKRUN) && LPC_WA_ENABLE_LPC_CLKRUN
    //
    // Enable LPC CLKRUN# to extend LPC circuit life
    //
    Data8 = MmioRead8(ILB_BASE_ADDRESS + R_PCH_ILB_LPCC);
    MmioOr8 (ILB_BASE_ADDRESS + R_PCH_ILB_LPCC, B_PCH_ILB_LPCC_CLKRUN_EN);
#endif

  //
  // if NMI_NOW_STS is set 
  // NMI NOW bit is "Write '1' to clear"
  //
  Data8 = MmioRead8(ILB_BASE_ADDRESS + R_PCH_ILB_GNMI);
  if ((Data8 & B_PCH_ILB_GNMI_NMINS) == B_PCH_ILB_GNMI_NMINS) {
    MmioOr8 (ILB_BASE_ADDRESS + R_PCH_ILB_GNMI, B_PCH_ILB_GNMI_NMIN);
  }
    MmioWrite32 (PMC_BASE_ADDRESS + R_PCH_PMC_S0IX_WAKE_STS, 0xffffffff);    

    //
    // Before we clear the TO status bit here we need to save the results in a CMOS bit for later use.
    //
    Data32 = IoRead32(ACPI_BASE_ADDRESS + R_PCH_TCO_STS);
    if((Data32 & B_PCH_TCO_STS_SECOND_TO) == B_PCH_TCO_STS_SECOND_TO) {
#if (defined(HW_WATCHDOG_TIMER_SUPPORT) && (HW_WATCHDOG_TIMER_SUPPORT != 0))
        CmosValue = CmosReadByte(SB_EFI_CMOS_PERFORMANCE_FLAGS);
        CmosWriteByte(SB_EFI_CMOS_PERFORMANCE_FLAGS, CmosValue | B_CMOS_TCO_WDT_RESET);
#endif
    }
    //
    // Now clear the TO status bit (Write '1' to clear)
    //
    IoWrite32(ACPI_BASE_ADDRESS + R_PCH_TCO_STS, (UINT32)(Data32 | B_PCH_TCO_STS_SECOND_TO));

}

VOID
PchPlatformLpcInit(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
)
{
    EFI_BOOT_MODE BootMode;
    UINT8         Data8;
    UINT16                Data16;

    (*PeiServices)->GetBootMode(PeiServices, &BootMode);

    if((BootMode != BOOT_ON_S3_RESUME)) {

        //
        // Clear all pending SMI. On S3 clear power button enable so it wll not generate an SMI
        //
        ClearSmiAndWake();
    }

    ClearPowerState();

    //
    // Update BSP Selection
    //

    CmosWriteByte(SB_CMOS_CPU_BSP_SELECT, PchPolicyData->BspSelection);


    //
    // Update Processor Flex Ratio
    //
    CmosWriteByte(SB_CMOS_CPU_RATIO_OFFSET, PchPolicyData->ProcessorFlexibleRatio);

    //
    // Update CPU BIST
    //
    CmosWriteByte(SB_CMOS_CPU_BIST_OFFSET, PchPolicyData->ProcessorBistEnable);

    //
    // Update CPU VMX
    //
    CmosWriteByte(SB_CMOS_CPU_VMX_OFFSET, PchPolicyData->ProcessorVmxEnable);

    //
    // Update Core number & HT enable
    //
    Data8 = PchPolicyData->ActiveProcessorCores + (PchPolicyData->ProcessorHyperThreadingDisable << 7);
    CmosWriteByte(SB_CMOS_CPU_CORE_HT_OFFSET, Data8);

    //
    // BIOS Decode Enable
    // This register effects the BIOS decode regardless of whether the BIOS is resident on LPC or SPI.
    // Since Mount Olive uses 8Mb part decode for 8Mb range.
    //
    /*  LpcPciCfg16(R_ICH_LPC_FWH_BIOS_DEC) = (
                        B_ICH_LPC_FWH_BIOS_DEC_F8 |
                        B_ICH_LPC_FWH_BIOS_DEC_F0 |
                        B_ICH_LPC_FWH_BIOS_DEC_E8 |
                        B_ICH_LPC_FWH_BIOS_DEC_E0 |
                        B_ICH_LPC_FWH_BIOS_DEC_D8 |
                        B_ICH_LPC_FWH_BIOS_DEC_D0 |
                        B_ICH_LPC_FWH_BIOS_DEC_C8 |
                        B_ICH_LPC_FWH_BIOS_DEC_C0
                        B_ICH_LPC_FWH_BIOS_LEG_F |
                        B_ICH_LPC_FWH_BIOS_LEG_E
                        );
    */
    //
    // Need to set and clear SET bit (RTC_REGB Bit 7) as requested by the ICH EDS
    // early in POST after each power up directly after coin-cell battery insertion.
    // This is to avoid the UIP bit (RTC_REGA Bit 7) from stuck at "1".
    // The UIP bit status may be polled by software (i.e ME FW) during POST.
    //
	//EIP129481 >>
    /* Remove it since it has been done in PchMiscEarlyInit().
    if(MmioRead8(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) {
        // Set and clear SET bit in RTC_REGB
        Data8 = CmosReadByte(SB_PCH_RTC_REGISTERB);
        Data8 |= B_PCH_RTC_REGISTERB_SET;
        CmosWriteByte(SB_PCH_RTC_REGISTERB, Data8);

        Data8 &= (~B_PCH_RTC_REGISTERB_SET);
        CmosWriteByte(SB_PCH_RTC_REGISTERB, Data8);

        CmosWriteByte(SB_PCH_RTC_REGISTERA, 0x00);
    }
    */
	//EIP129481 <<
    //
    // Disable SERR NMI and IOCHK# NMI in port 61
    //
    Data8 = IoRead8(R_PCH_NMI_SC);
    IoWrite8(R_PCH_NMI_SC, (UINT8)(Data8 | B_PCH_NMI_SC_PCI_SERR_EN | B_PCH_NMI_SC_IOCHK_NMI_EN));

    //
    // Enable Bus Master, I/O, Mem, and SERR on LPC bridge
    //
    Data16 = PchLpcPciCfg16(R_PCH_LPC_COMMAND);
    MmioWrite16(
        MmPciAddress(0,
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     PCI_DEVICE_NUMBER_PCH_LPC,
                     PCI_FUNCTION_NUMBER_PCH_LPC,
                     R_PCH_LPC_COMMAND
                    ),
        (Data16 |
         B_PCH_LPC_COMMAND_IOSE |
         B_PCH_LPC_COMMAND_MSE |
         B_PCH_LPC_COMMAND_BME |
         B_PCH_LPC_COMMAND_SERR_EN)
    );

    //
    // Set Stretch S4 to 1-2s per marketing request.
    // Note: This register is powered by RTC well.
    //
    MmioAndThenOr8(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1 , (UINT8)(~B_PCH_PMC_GEN_PMCON_SLP_S4_MAW), (UINT8)(B_PCH_PMC_GEN_PMCON_SLP_S4_ASE | V_PCH_PMC_GEN_PMCON_SLP_S4_MAW_4S));

}

EFI_STATUS
InitPchUsb(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
)
/*++

Routine Description:


Arguments:


Returns:


--*/
{
    EFI_STATUS              Status;
    EFI_PEI_PPI_DESCRIPTOR  *PeiPchUsbPolicyPpiDesc;
    PCH_USB_POLICY_PPI      *PeiPchUsbPolicyPpi;
    PCH_USB_CONFIG          *UsbConfig;
    UINT8                   PortIndex;
    UINTN                   EhciBaseAddr[] = PEI_EHCI_MEM_BASE_ADDRESSES;
    PCH_INIT_PPI            *PchInitPpi;

    //
    // Allocate descriptor and PPI structures.  Since these are dynamically updated
    // we cannot do a global variable PPI.
    //
    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(EFI_PEI_PPI_DESCRIPTOR), &PeiPchUsbPolicyPpiDesc);
    ASSERT_PEI_ERROR(PeiServices, Status);
    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(PCH_USB_POLICY_PPI), &PeiPchUsbPolicyPpi);
    ASSERT_PEI_ERROR(PeiServices, Status);
    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(PCH_USB_CONFIG), &UsbConfig);
    ASSERT_PEI_ERROR(PeiServices, Status);

    //
    // Initiate PCH USB policy.
    //
    PeiPchUsbPolicyPpi->Revision        = PCH_USB_POLICY_PPI_REVISION_1;
	// EIP126330 EIP126249 >>
    UsbConfig->Usb20Settings[0].Enable  = PCH_DEVICE_ENABLE;
    UsbConfig->UsbPerPortCtl            = PCH_DEVICE_DISABLE;
	// EIP126330 EIP126249 <<
    for(PortIndex = 0; PortIndex < PCH_USB_MAX_PHYSICAL_PORTS; PortIndex++) {
    	UsbConfig->PortSettings[PortIndex].Enable	= PchPolicyData->PchUsbPort[PortIndex];
        UsbConfig->PortSettings[PortIndex].Dock		= PchPolicyData->PchUsbDock[PortIndex];
        UsbConfig->PortSettings[PortIndex].Panel	= PchPolicyData->PchUsbPanel[PortIndex];
    }

    UsbConfig->Ehci1Usbr                = PchPolicyData->Ehci1Usbr;

    for(PortIndex = 0; PortIndex < PCH_USB_MAX_PHYSICAL_PORTS; PortIndex++) {
        UsbConfig->Usb20OverCurrentPins[PortIndex]  = PchPolicyData->Usb20OverCurrentPins[PortIndex];
        UsbConfig->Usb20PortLength[PortIndex]       = PchPolicyData->Usb20PortLength[PortIndex];
    }

    UsbConfig->Usb30Settings.XhciStreams  = PchPolicyData->PchUsb30Streams;
    UsbConfig->Usb30Settings.Mode         = PCH_XHCI_MODE_OFF; 	// EIP126330 EIP126249
    UsbConfig->UsbOtgSettings.Enable      = PchPolicyData->PchUsbOtg;
    UsbConfig->Usb30OverCurrentPins[0]    = PchPolicyData->Usb30OverCurrentPins[0];
    UsbConfig->UsbXhciLpmSupport          = PchPolicyData->UsbXhciLpmSupport;
    
    switch(UsbConfig->Usb30Settings.Mode) {
    case 0: // Disabled
        UsbConfig->Usb30Settings.PreBootSupport = 0;
        break;
    case 1: // Enabled
        UsbConfig->Usb30Settings.PreBootSupport = 1;
        break;
    case 2: // Auto
        UsbConfig->Usb30Settings.PreBootSupport = 0;
        break;
    case 3: // Smart Auto
        UsbConfig->Usb30Settings.PreBootSupport = 1;
        break;
    default:
        UsbConfig->Usb30Settings.PreBootSupport  = PchPolicyData->PchUsbPreBootSupport; //CSP20130723_C
        break;
    }

    PeiPchUsbPolicyPpi->Mode            = EHCI_MODE;
    PeiPchUsbPolicyPpi->EhciMemBaseAddr = EhciBaseAddr[0];
    PeiPchUsbPolicyPpi->EhciMemLength   = (UINT32) 0x400 * PchEhciControllerMax;
    PeiPchUsbPolicyPpi->UsbConfig       = UsbConfig;
    PeiPchUsbPolicyPpiDesc->Flags       = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    PeiPchUsbPolicyPpiDesc->Guid        = &gPchUsbPolicyPpiGuid;
    PeiPchUsbPolicyPpiDesc->Ppi         = PeiPchUsbPolicyPpi;

    //
    // Install PCH USB Policy PPI
    //
    Status = (**PeiServices).InstallPpi(PeiServices, PeiPchUsbPolicyPpiDesc);
    ASSERT_PEI_ERROR(PeiServices, Status);

    //
    // Locate PchInit Ppi
    //
    Status = (*PeiServices)->LocatePpi(PeiServices, &gPchInitPpiGuid, 0, NULL, &PchInitPpi);
    ASSERT_PEI_ERROR(PeiServices, Status);

    //
    // Performing USB initialization
    //
    Status = PchInitPpi->UsbInit(PeiServices);

    return Status;
}

EFI_STATUS
PlatformPchInit(
    IN CONST EFI_PEI_SERVICES       **PeiServices,
    IN SB_SETUP_DATA                *PchPolicyData
)
{
    //EFI_STATUS                  Status; // EIP132001 (-)
    //EFI_BOOT_MODE               BootMode; // EIP132001 (-)

    // PCH Policy Initialization based on Setup variable.
    ScPolicyInitPei(PeiServices, PchPolicyData);

    // Program CRID
    IchRcrbInit(PeiServices, PchPolicyData);

    PchInitInterrupt(PeiServices, PchPolicyData);

    UARTInit(PeiServices, PchPolicyData);

    // EIP132001 (-) >>
    //
    // Initialize USB Controller before entering recovery mode.
    //
    //Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);
    //if(!EFI_ERROR(Status)) {
    //    if(BootMode == BOOT_IN_RECOVERY_MODE) {
    //        PEI_TRACE((-1, PeiServices, "Recovery Mode!\n"));
    //        InitPchUsb(PeiServices, PchPolicyData);
    //    }
    //}
    // EIP132001 (-) <<

    PchPlatformLpcInit(PeiServices, PchPolicyData);
    //
    // Soc specific GPIO setting
    //
	
//CSP20130712 >>
#if MMC_SUPPORT
    ConfigureSoCGpio(PeiServices, PchPolicyData);   //(EIP120879+)
#endif
//CSP20130712 <<

    return EFI_SUCCESS;
}


//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2017, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

