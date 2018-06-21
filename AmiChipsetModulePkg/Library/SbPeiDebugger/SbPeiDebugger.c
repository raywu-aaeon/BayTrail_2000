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
// Name:        SbPeiDebugger.C
//
// Description: This file contains PEI stage debugger code for SB template
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>
// Files included from EDKII
#include <Sb.h>
#include <token.h>
#include <AmiChipsetIoLib.h>
#include <PchUsbCommon.h>
#include <Library/PciLib.h>

#define PCI_PMCSR               0x8000
#define PWR_MGT_CAP_ID          1
#define DBG_PRT_CAP_ID          0x0a
#define TIMER_CONTROL_PORT      0x43
#define TIMER0_COUNT_PORT       0x40

typedef union _PEI_DBG_PORT_INFO  PEI_DBG_PORT_INFO;
typedef EFI_STATUS (PEI_INIT_FUNCTION) (IN OUT PEI_DBG_PORT_INFO   *DebugPort);

typedef struct _PEI_DBG_COM_PORT{
  UINT16        COMBaseAddr;
  UINT8       SIO_COM_LDN;
}PEI_DBG_COM_PORT;

typedef struct _PEI_DBG_USB_PORT{
  UINT32        USBBaseAddr;
  UINT32        USBDebugPortStartAddr;
  UINT16        MemoryMappedIoSpaceSize;
  UINT8       PciBusNumber;
  UINT8       PciDeviceNumber;
  UINT8       PciFunctionNumber;
  UINT8       PciBAROffset;
  PEI_INIT_FUNCTION *InitUSBEHCI;
}PEI_DBG_USB_PORT;

typedef union _PEI_DBG_PORT_INFO{
  PEI_DBG_COM_PORT  SerialPort;
  PEI_DBG_USB_PORT  UsbDebugPort;
}PEI_DBG_PORT_INFO;

// Function Prototypes
VOID   Pei8259WriteMask (IN UINT16 Mask, IN UINT16 EdgeLevel);
VOID   Pei8259SetVectorBase (IN UINT8 MasterBase, IN UINT8 SlaveBase);
VOID   Program8254Timer0 (IN UINT16 Count);

EFI_STATUS
InitPchUsb (
    VOID
  )
/*++

Routine Description:


Arguments:


Returns:


--*/
{
  EFI_STATUS      Status;
  PCH_USB_CONFIG  *UsbConfig;
  UINTN           EhciBaseAddr[] = PEI_EHCI_MEM_BASE_ADDRESSES;
  UINT32          FuncDisableReg;
  UINTN           PciD31F0RegBase;
  UINT32          PmcBase;
  UINT32          RootComplexBar;

  UsbConfig = NULL;
  PciD31F0RegBase = SB_REG (0);
  PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  RootComplexBar  = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_RCBA) & B_PCH_LPC_RCBA_BAR;
  FuncDisableReg  = MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS);

  Status = CommonUsbInit (
            UsbConfig,
            (UINT32) EhciBaseAddr[0],
            (UINT32) 0,
            0,
            RootComplexBar,
            &FuncDisableReg,
            1
            );
  
  MmioWrite32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS), (UINT32) (FuncDisableReg));
  ///
  /// Reads back for posted write to take effect
  ///
  MmioRead32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS));

  return Status;
}

#if USB_DEBUG_TRANSPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SBPEIDBG_InitUsbEhci
//
// Description: This eLink function is used to initialize the EHCI controller
//              debug port for USB PEI Debugging. It also fills the transport
//              interface structure with appropriate information
//
// Input:       IN OUT PEI_DBG_PORT_INFO *DebugPort - Debug transport interface structure
//
// Output:      EFI_STATUS
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SBPEIDBG_InitUsbEhci (
    IN OUT PEI_DBG_PORT_INFO    *DebugPort
)
{
  UINT8               PwrMgtCapReg;
  UINT8               DbgPrtCapReg;
  UINT16              CmdReg;
  UINT16              DbgPrtBaseOffset;
  UINT32              EhciBaseAddress = 0;
  UINT32               PciAddress;
  UINT32               EhciBaseAddr[] = PEI_EHCI_MEM_BASE_ADDRESSES;

  InitPchUsb();
  
  // Init USB 2.0 Controller & Enable Debug Port
  if (DebugPort->UsbDebugPort.USBBaseAddr)
      EhciBaseAddress = DebugPort->UsbDebugPort.USBBaseAddr;
  else
      EhciBaseAddress = EhciBaseAddr[0];

  // First disable the EHCI device by programming PCI command register

  PciAddress = CSP_PCIE_CFG_ADDRESS (
                DEBUG_EHCI_BUS_NUMBER,
                DEBUG_EHCI_DEVICE_NUMBER,
                DEBUG_EHCI_FUNCTION_NUMBER,
                0
                );

  CmdReg = MmioRead16 (PciAddress + R_PCH_EHCI_COMMAND_REGISTER);
  if (CmdReg & 7) {
    MmioWrite16 (PciAddress + R_PCH_EHCI_COMMAND_REGISTER, 0);
  }

  // Assign MMIO base address register in appropriate PCI register

  MmioWrite32 (PciAddress + R_PCH_EHCI_MEM_BASE, EhciBaseAddress);

  // Set the Power state of the device to D0
  PwrMgtCapReg = FindCapPtr (USB_EHCI_REG(0), PWR_MGT_CAP_ID);
  MmioWrite16 (PciAddress + PwrMgtCapReg + 4, PCI_PMCSR);

  // Enable SiS EHCI register & make it Bus master
  CmdReg |= 0x06;
  MmioWrite16 (PciAddress + R_PCH_EHCI_COMMAND_REGISTER, CmdReg);

  // Locate the Debug port register Interface location
  DbgPrtCapReg = FindCapPtr (USB_EHCI_REG(0), DBG_PRT_CAP_ID);

  // 0x58
  DbgPrtBaseOffset = MmioRead16 (PciAddress + DbgPrtCapReg + 2) & 0x1fff;
  // 0x20a0
  if (DebugPort->UsbDebugPort.USBBaseAddr == 0)
  {
      DebugPort->UsbDebugPort.USBBaseAddr = EHCI_MMIO_BASE_ADDRESS;
      DebugPort->UsbDebugPort.MemoryMappedIoSpaceSize = EHCI_MMIO_SIZE;
  }
  DebugPort->UsbDebugPort.USBDebugPortStartAddr = EhciBaseAddress + DbgPrtBaseOffset;
  DebugPort->UsbDebugPort.PciBusNumber          = DEBUG_EHCI_BUS_NUMBER ;
  DebugPort->UsbDebugPort.PciDeviceNumber       = DEBUG_EHCI_DEVICE_NUMBER;
  DebugPort->UsbDebugPort.PciFunctionNumber     = DEBUG_EHCI_FUNCTION_NUMBER;
  DebugPort->UsbDebugPort.PciBAROffset          = R_PCH_EHCI_MEM_BASE;
  DebugPort->UsbDebugPort.InitUSBEHCI           = SBPEIDBG_InitUsbEhci;
  IoWrite8 (0x80, MmioRead8 (EhciBaseAddress));

  return EFI_SUCCESS;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EnableSerialIrq
//
// Description: This function programs the SB register to enable the serial IRQ
//
// Input:       None
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID EnableSerialIrq (VOID)
{
  UINT8   RegData8;
  UINT16  RegData16;
  
  RegData16  = MmioRead16 (ILB_BASE_ADDRESS + R_PCH_ILB_OIC) & (UINT16) ~(B_PCH_ILB_OIC_SIRQEN);
  RegData8   = MmioRead8  (ILB_BASE_ADDRESS + R_PCH_ILB_SERIRQ_CNT);
  
  ///
  /// Set the SERIRQ logic to continuous mode
  ///
  RegData16 |= (UINT16) B_PCH_ILB_OIC_SIRQEN;
  RegData8  |= (UINT8)  B_PCH_ILB_SERIRQ_CNT_SIRQMD;
  
  MmioWrite16 (ILB_BASE_ADDRESS + R_PCH_ILB_OIC, RegData16);
  MmioWrite8 (ILB_BASE_ADDRESS + R_PCH_ILB_SERIRQ_CNT, RegData8);
  PchPmTimerStall (1000);
  MmioAnd8 (ILB_BASE_ADDRESS + R_PCH_ILB_SERIRQ_CNT, (UINT8) ~B_PCH_ILB_SERIRQ_CNT_SIRQMD);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EnableLpcIoDecode
//
// Description: This function programs the SB register to enable the LPC IO
//              Decoding ranges to enable the programming of SIO and Serial Port.
//
// Input:       None
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID EnableLpcIoDecode (
    VOID
)
{
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProgramAcpiBaseToDisableTco
//
// Description: This function programs the SB register to disable
//              the TCO timer. If this timer is not disabled the system
//              will shutdown or reset as soon as the timer is expired
//
// Input:       None
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramAcpiBaseToDisableTco (
    VOID
)
{
    // Halt the TCO timer TCOBASE+8 = TCO1 control register
    IoWrite16(PM_BASE_ADDRESS + R_PCH_TCO_CNT, IoRead16(PM_BASE_ADDRESS + R_PCH_TCO_CNT) | B_PCH_TCO_CNT_TMR_HLT); //TCOBASE+8 = TCO1 control register
    IoWrite16(PM_BASE_ADDRESS + R_PCH_TCO_STS, IoRead16(PM_BASE_ADDRESS + R_PCH_TCO_STS) | B_PCH_TCO_STS_SECOND_TO);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProgramClockGenerator
//
// Description: This function programs the onboard clock generator.
//
// Input:       None
//
// Output:      None
//
// Notes:       PORTING REQUIRED if needed.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramClockGenerator (VOID)
{

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SBPEIDBG_Initialize
//
// Description: This eLink function is used to initialize the South Bridge
//              for PEI Debugger support
//
// Input:       IN OUT PEI_DBG_PORT_INFO *DebugPort - Debug transport interface structure
//
// Output:      EFI_STATUS
//
// Notes:       Normally no PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SBPEIDBG_Initialize (
    IN OUT PEI_DBG_PORT_INFO    *DebugPort
)
{
    //
    // Program the TCO IO to avoid rebooting of the hardware
    //
    ProgramAcpiBaseToDisableTco();
    //
    // Programm Clock Generator
    //
    ProgramClockGenerator();
    //
    // Init 8259 Controller
    //
    Pei8259SetVectorBase (LEGACY_MODE_BASE_VECTOR_MASTER, LEGACY_MODE_BASE_VECTOR_SLAVE );
    //
    // Set all 8259 Interrupts to edge triggered and disabled
    //
    Pei8259WriteMask (0xffff , 0x0000);
    EnableLpcIoDecode ();
    EnableSerialIrq ();
    Program8254Timer0 (0);
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: FindCapPtrDbg
//
// Description: This function searches the PCI address space for the PCI
//              device specified for a particular capability ID and returns
//              the offset in the PCI address space if one found
//
// Input:  Bus      PCI Bus number
//         DevFunc  PCI Device and function number
//         CapId    Capability ID to look for
//
// Output:  Capability ID location if one found
//          Otherwise returns 0
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
FindCapPtrDbg(
 IN UINT8 Bus,
 IN UINT8 DevFunc,
 IN UINT8 CapId
)
{
  UINTN     PcieAddress;
  UINT8     Dev;
  UINT8     Func;

  Dev = (DevFunc >> 3) & 0x1F;
  Func = DevFunc & 0x07;
  PcieAddress = CSP_PCIE_CFG_ADDRESS(Bus, Dev, Func, 0);
  return FindCapPtr (PcieAddress, CapId);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: Pei8259WriteMask
//
// Description: Writes PC 8259 interrupt Controller mask register
//
// Input:       IN UINT16 Mask - Mask to write
//              IN UINT16 EdgeLevel - Edge/level trigger to be programmed
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
Pei8259WriteMask (
    IN UINT16 Mask, 
    IN UINT16 EdgeLevel)
{
    IoWrite8 (LEGACY_8259_MASK_REGISTER_MASTER, (UINT8)Mask);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_SLAVE,  (UINT8)(Mask >> 8));
    IoWrite8 (LEGACY_8259_EDGE_LEVEL_TRIGGERED_REGISTER_MASTER, (UINT8)EdgeLevel);
    IoWrite8 (LEGACY_8259_EDGE_LEVEL_TRIGGERED_REGISTER_SLAVE,  (UINT8)(EdgeLevel >> 8));
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: Pei8259SetVectorBase
//
// Description: Set up the 8259 interrupt controller master and slave base values
//
// Input:       IN UINT8 MasterBase - Master base to be programmed
//              IN UINT8 SlaveBase - Slave base to be programmed
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
Pei8259SetVectorBase (
    IN UINT8 MasterBase, 
    IN UINT8 SlaveBase
)
{
    UINT8           Mask;
    
    // Initialize Slave interrupt controller.
    Mask = IoRead8 (LEGACY_8259_MASK_REGISTER_SLAVE);
    IoWrite8 (LEGACY_8259_CONTROL_REGISTER_SLAVE, 0x11);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_SLAVE,    SlaveBase);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_SLAVE,    0x02); 
    IoWrite8 (LEGACY_8259_MASK_REGISTER_SLAVE,    0x01);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_SLAVE,    Mask);

    // Initialize Master interrupt controller.
    Mask = IoRead8 (LEGACY_8259_MASK_REGISTER_MASTER);
    IoWrite8 (LEGACY_8259_CONTROL_REGISTER_MASTER, 0x11);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_MASTER,    MasterBase);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_MASTER,    0x04);
    IoWrite8 (LEGACY_8259_MASK_REGISTER_MASTER,    0x01); 
    IoWrite8 (LEGACY_8259_MASK_REGISTER_MASTER,    Mask); 

    IoWrite8 (LEGACY_8259_CONTROL_REGISTER_SLAVE,  LEGACY_8259_EOI);
    IoWrite8 (LEGACY_8259_CONTROL_REGISTER_MASTER, LEGACY_8259_EOI);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Program8254Timer0
//
// Description: Programs the 8254 Timer channel 0
//
// Input:       IN UINT16 Count - Timer tick count
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID Program8254Timer0(
    IN UINT16 Count
)
{
    UINT8   LCntbyte,HCntbyte;

    LCntbyte = (UINT8)Count;
    HCntbyte = (UINT8)(Count >> 8);

    //Write the timer control port to select timer 0 and set to mode 3
    IoWrite8(TIMER_CONTROL_PORT, 0x36);

    //Write the Counter 0 with initial count value
    IoWrite8(TIMER0_COUNT_PORT, LCntbyte);
    IoWrite8(TIMER0_COUNT_PORT, HCntbyte);

    //Enable the IRQ0 interrupt for this timer 0 in USB Xport module
    //......
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetLegCapStrucAddr
//
// Description: This function tries to find the specific capabilities
//              ID structure address.
//
// Input:       Bus       - The PCI Bus number.
//              Dev       - The PCI Device number.
//              Fun       - The PCI Function number.
//              FindCapId - the specific legacy capabilities ID will be
//                          found.
//
// Output:      EFI_STATUS
//                  EFI_SUCCESS   - Found the legacy capabilities structure
//                                  successfully, the input CapPtr8 will
//                                  have the structure address.
//                  EFI_NOT_FOUND - Not found the extended capabilities
//                                  structure. 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
/*
EFI_STATUS GetLegCapStrucAddr (
    IN UINT8                Bus,
    IN UINT8                Dev,
    IN UINT8                Fun,
    IN UINT8                FindCapId,
    OUT UINT8               *CapPtr8 )
{
    UINT8           Buffer8;
    
    PciRead32 (PciAddress + R_PCH_EHCI_COMMAND_REGISTER);
    if (PciRead32 (PciAddress) != 0xffffffff) {    	
        if (PciRead16(PciAddress, PCI_PRIMARY_STATUS_OFFSET) & 0x10) {
            *CapPtr8 = ((PciRead8(PciAddress, PCI_HEADER_TYPE_OFFSET) & 0x7f) == 2) ? \
                                                                  0x14 : 0x34;
            *CapPtr8 = PciRead8(PciAddress, *CapPtr8);
            if (*CapPtr8 == 0) return EFI_NOT_FOUND;
            Buffer8 = PciRead8(PciAddress, *CapPtr8);
            while (Buffer8 != 0) {
                if (Buffer8 == FindCapId) return EFI_SUCCESS;
                Buffer8 = *CapPtr8 + 1;
                *CapPtr8 = PciRead8(PciAddress, Buffer8);
                if (*CapPtr8 == 0) break;
                Buffer8 = PciRead8(PciAddress, *CapPtr8);
            }
        }
    }

    return EFI_NOT_FOUND;
}
*/

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
