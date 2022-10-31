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
