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
// $Header:  $
//
// $Revision:  $
//
// $Date: $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name:        SmiHandlerPorting.c
//
// Description: This file contains SMM Child Dispatcher porting functions
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiSmm.h>
#include <Protocol/SmmCpu.h>
#include "SmmChildDispatch.h"

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
extern EFI_SMM_CPU_PROTOCOL  *gEfiSmmCpuProtocol;
#endif

//*************** All purpose SMI Porting hooks *************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ClearAllSmi
//
// Description: This function clears all SMI's
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ClearAllSmi(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DisableAllSmi
//
// Description: This function disables all SMI's
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID DisableAllSmi(VOID)
{
//Porting required
//Disable All smi, which can be caused by SouthBridge
}

//*************** SW SMI Handler Porting hooks ***************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SwSmiEnable
//
// Description: This function enables SW SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SwSmiEnable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SwSmiDisable
//
// Description: This function disables SW SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SwSmiDisable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SwSmiClear
//
// Description: This function clears SW SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SwSmiClear(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SwSmiDetect
//
// Description: This function detects SW SMI event
//
// Input:       UINT16 *Type - pointer to store SW SMI number
//
// Output:      TRUE - SW SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SwSmiDetect(UINT16 *Type)
{
//Porting required
//If Software SMI occured return TRUE, and value of SMI store in Type variable
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetEAX
//
// Description: This function returns EAX saved value from CPU that caused SW SMI
//
// Input:       None
//
// Output:      EAX saved value
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINTN GetEAX(VOID)
{
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
//Porting required for different CPU
    EFI_STATUS              Status;
    EFI_GUID                SwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;
    SW_SMI_CPU_TRIGGER      *SwSmiCpuTrigger;
    UINTN                   Cpu = pSmst->CurrentlyExecutingCpu - 1; // CPU #
    UINT16                  i;
    UINT32                  RegEAX;

    for(i = 0; i < pSmst->NumberOfTableEntries; i++) {
        if(guidcmp(&(pSmst->SmmConfigurationTable[i].VendorGuid), \
                   &SwSmiCpuTriggerGuid) == 0)
            break;
    }

    // If found table, check for the CPU that caused the software Smi.
    if(i != pSmst->NumberOfTableEntries) {
        SwSmiCpuTrigger = pSmst->SmmConfigurationTable[i].VendorTable;
        Cpu = SwSmiCpuTrigger->Cpu;
    }

    Status = gEfiSmmCpuProtocol->ReadSaveState(\
             gEfiSmmCpuProtocol, \
             4, \
             EFI_SMM_SAVE_STATE_REGISTER_RAX, \
             Cpu, \
             &RegEAX);
    return RegEAX;
#else
//Porting required for different CPU
    UINT16                  i;
    EFI_GUID                SwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;
    SW_SMI_CPU_TRIGGER      *SwSmiCpuTrigger;
    UINTN                   Cpu = pSmst->CurrentlyExecutingCpu - 1; //default cpu #
    EFI_SMM_CPU_SAVE_STATE  *CpuSaveState;

    for(i = 0; i < pSmst->NumberOfTableEntries; i++) {
        if(guidcmp(&(pSmst->SmmConfigurationTable[i].VendorGuid), &SwSmiCpuTriggerGuid) == 0)
            break;
    }

    //If found table, check for the CPU that caused the software Smi.
    if(i != pSmst->NumberOfTableEntries) {
        SwSmiCpuTrigger = pSmst->SmmConfigurationTable[i].VendorTable;
        Cpu = SwSmiCpuTrigger->Cpu;
    }

    CpuSaveState = pSmst->CpuSaveState;
    return CpuSaveState[Cpu].Ia32SaveState.EAX;
#endif
}

//*************** SX SMI Handler Porting hooks ***************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SxSmiEnable
//
// Description: This function enables SX SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SxSmiEnable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SxSmiDisable
//
// Description: This function disables SX SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SxSmiDisable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SxSmiClear
//
// Description: This function clears SX SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SxSmiClear(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SxSmiDetect
//
// Description: This function detects SX SMI event
//
// Input:       UINT16 *Type - pointer to store value of Sleep type
//
// Output:      TRUE - SX SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SxSmiDetect(UINT16 *Type)
{
//Porting required
//If Sleep SMI occured return TRUE, and value of Sleep type store in Type variable
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PutToSleep
//
// Description: Disable Smi sleep and put to sleep.
//
// Input:       *Context - Pointer to Sleep SMI context
//
// Output:      None
//
// Referrals:   SxSmiDisable
//
// Notes:       Here is the control flow of this function:
//              1. Disable Smi sleep.
//              2. Set to go to sleep if you want to sleep in SMI. otherwise
//                 set IORestart to 0xFF in CPU SMM dump area.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID PutToSleep(
    IN AMI_SMM_SX_CONTEXT  *SxContext)
{
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    SxSmiDisable(); // Disable sleep SMI.

    // Set to sleep.
    IoWrite16(PM_BASE_ADDRESS + 4, IoRead16(PM_BASE_ADDRESS + 4) | 0x2000);
#else
#if !ACPI_SLEEP_IN_SMM
    EFI_SMM_CPU_SAVE_STATE  *pCpuSaveState = pSmst->CpuSaveState;
    UINTN                   Cpu = pSmst->CurrentlyExecutingCpu - 1;
    UINT32                  CacheFlush;
#endif

    SxSmiDisable(); // Disable sleep SMI.

#if ACPI_SLEEP_IN_SMM
    IoWrite16(PM_BASE_ADDRESS + 4, 0x2000); // Set to sleep.
#else
    CacheFlush = pCpuSaveState[Cpu].Ia32SaveState.IORestart;
    pCpuSaveState[Cpu].Ia32SaveState.IORestart = 0xff;
#endif
#endif
}

//*************** Periodic timer SMI Handler Porting hooks ***************************
UINT64 SupportedIntervals[] = {
//Porting required - put all available intervals here (in nanoseconds)

    0       //terminator record
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerSmiEnable
//
// Description: This function enables Periodic timer SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TimerSmiEnable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerSmiDisable
//
// Description: This function disables Periodic timer SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TimerSmiDisable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerSmiClear
//
// Description: This function clears Periodic timer SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TimerSmiClear(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerSmiDetect
//
// Description: This function detects Periodic timer SMI event
//
// Input:       UINT16 *Type - added for compatibility, not used
//
// Output:      TRUE - Periodic timer SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN TimerSmiDetect(UINT16 *Type)
{
//Porting required - return TRUE if Timer SMI detected, Type ignored
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   TimerSetInterval
//
// Description: This function programs Periodic timer to given interval
//
// Input:       UINT64 Interval - interval to program
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TimerSetInterval(UINT64 Interval)
{
//Porting required
}

//*************** Usb SMI Handler Porting hooks ***************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbSmiSet
//
// Description: This function enables/disables USB SMI based on given Controller type
//
// Input:       UINT16 ControllerType - USB controller type variable
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UsbSmiSet(UINT16 ControllerType)
{
//Porting required
//This function implements logic as follows:
//Two lowest bits of ControllerType:
// 00 - both USB controllers smi are disabled
// 01 - UHCI/OHCI enabled, EHCI - disabled
// 10 - UHCI/OHCI disabled, EHCI - enabled
// 11 - both USB controllers smi are enabled
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbSmiClear
//
// Description: This function clears USB SMI based on given Controller type
//
// Input:       UINT16 ControllerType - USB controller type variable
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UsbSmiClear(UINT16 ControllerType)
{
//Porting required
//clear UHCI/OHCI SMI if 1, EHCI - if 2 or both if 3
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbSmiDetect
//
// Description: This function detects USB SMI event
//
// Input:       UINT16 *Type - pointer to store USB controller type, source of event
//
// Output:      TRUE - USB SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN UsbSmiDetect(UINT16 *Type)
{
//Porting required
//If interrupt occured Set *Type to 1 for UHCI/OHCI, 2 for EHCI or 3 for both and return TRUE
//Otherwise return FALSE and set *Type to zero
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetControllerType
//
// Description: This function returns USB controller type, based on given device path
//
// Input:       EFI_DEVICE_PATH_PROTOCOL *Device - pointer USB device path protocol
//
// Output:      UINT16 - USB controller type
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 GetControllerType(EFI_DEVICE_PATH_PROTOCOL *Device)
{
//Porting required
//return 1 for USB1.1 (UHCI or OHCI controllers) and 2 for USB2.0 (EHCI controller)
//return 0 if there are no matches - it will indicate an error
    return 0;
}

//*************** GPI SMI Handler Porting hooks ***************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GpiSmiSet
//
// Description: This function enables/disables GPI SMI based on given bit field
//
// Input:       UINT16 Type - GPI enabled bit field
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID GpiSmiSet(UINT16 Type)
{
//Porting required
//All Gpis which correspondent bit in Type set to 1 should be enabled, all others - disabled
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GpiSmiSet
//
// Description: This function clears GPI SMI based on given bit field
//
// Input:       UINT16 Type - GPI enabled bit field
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID GpiSmiClear(UINT16 Type)
{
//Porting required
//All Gpis which correspondent bit in Type set to 1 should be cleared
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GpiSmiDetect
//
// Description: This function detects GPI SMI event
//
// Input:       UINT16 *Type - pointer to store source of GPI SMI
//
// Output:      TRUE - GPI SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN GpiSmiDetect(UINT16 *Type)
{
//Porting required
    return FALSE;
}

//*************** Standby button SMI Handler Porting hooks *****************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SButtonSmiEnable
//
// Description: This function enables Standby button SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SButtonSmiEnable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SButtonSmiDisable
//
// Description: This function disables Standby button SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SButtonSmiDisable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SButtonSmiClear
//
// Description: This function clears Standby button SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SButtonSmiClear(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SButtonSmiDetect
//
// Description: This function detects Standby button SMI event
//
// Input:       UINT16 *Type - pointer to store value of Standby button phase
//
// Output:      TRUE - Standby button SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SButtonSmiDetect(UINT16 *Type)
{
//Porting required
    return FALSE;
}

//*************** Power button SMI Handler Porting hooks *****************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PButtonSmiEnable
//
// Description: This function enables Power button SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PButtonSmiEnable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PButtonSmiDisable
//
// Description: This function disables Power button SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PButtonSmiDisable(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PButtonSmiClear
//
// Description: This function clears Power button SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PButtonSmiClear(VOID)
{
//Porting required
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PButtonSmiDetect
//
// Description: This function detects Power button SMI event
//
// Input:       UINT16 *Type - pointer to store value of Power button phase
//
// Output:      TRUE - Standby button SMI occured, FALSE otherwise
//
// Notes:       Porting required
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN PButtonSmiDetect(UINT16 *Type)
{
//Porting required
    return FALSE;
}

//---------------------------------------------------------------------------
//                     I/O Trap SMI Handler Porting hooks
//---------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoTrapSmiSet
//
// Description: This function sets I/O Trap functon based on given the
//              context
//
// Input:       IoTrapContext - Pointer to the context that I/O trap register
//                              will be enabled.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IoTrapSmiSet(
    IN EFI_SMM_IO_TRAP_REGISTER_CONTEXT    *IoTrapContext)
{
    /*
        // Porting required if needed.
        UINT32 IoTrapAddr = RCRB_MMIO_IO_TRAP_0; // 0x1E80
        UINT32 i;
        UINT32 Buffer32 = 0;

        // Find an available I/O trap register
        for (i = 0; i < MAX_SUPPORTED_IOTRAP_REGS; i++) {
            if ((READ_MEM32_RCRB(IoTrapAddr) & 1) == 0) break;
            IoTrapAddr += 8;
        }

        IoTrapContext->TrapRegIndex = i;

        if (IoTrapContext->IoTrapContext.Length < 4) IoTrapContext->IoTrapContext.Length = 4;
        Buffer32 = IoTrapContext->IoTrapContext.Length;
        for (i = 0; Buffer32 != 1; Buffer32 >>= 1, i++);
        if (IoTrapContext->IoTrapContext.Length > (1 << i)) i++;

        IoTrapContext->IoTrapContext.Length = 1 << i; // Length is always 2^n

        Buffer32 = IoTrapContext->IoTrapContext.Address & 0xfffc;
        Buffer32 |= ((IoTrapContext->IoTrapContext.Length - 1) & 0xfffc) << 16;
        WRITE_MEM32_RCRB(IoTrapAddr, Buffer32);

        Buffer32 = 0xf0;
        if (IoTrapContext->TrapWidth == AccessWord) Buffer32 = 0x03;
        if (IoTrapContext->TrapWidth == AccessDWord) Buffer32 = 0x0f;

        if (IoTrapContext->IoTrapContext.TrapOpType == ReadWriteIoCycle) {
            Buffer32 |= (1 << 17); // Both Read/Write Cycles.
        } else {
            if (IoTrapContext->IoTrapContext.TrapOpType == ReadIoCycle)
                Buffer32 |= (1 << 16); // Read Cycle Only
        }

        WRITE_MEM32_RCRB(IoTrapAddr + 4, Buffer32);
        SET_MEM32_RCRB(IoTrapAddr, 1); // Enable Trap and SMI.
    */
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoTrapSmiReset
//
// Description: This function resets I/O Trap functon based on given the
//              context
//
// Input:       IoTrapContext - Pointer to the context that I/O trap register
//                              will be disabled.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IoTrapSmiReset(
    IN EFI_SMM_IO_TRAP_REGISTER_CONTEXT    *IoTrapContext)
{
    // Porting required if needed.
//   UINT32 IoTrapAddr = RCRB_MMIO_IO_TRAP_0 + IoTrapContext->TrapRegIndex * 8;

//    WRITE_MEM32_RCRB(IoTrapAddr, 0);
//    WRITE_MEM32_RCRB(IoTrapAddr + 4, 0);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoTrapSmiEnable
//
// Description: This function enables I/O Trap SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IoTrapSmiEnable(VOID)
{
    // Porting required if needed.
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoTrapSmiDisable
//
// Description: This function disables I/O Trap SMI
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IoTrapSmiDisable(VOID)
{
    // Porting required if needed.
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoTrapSmiClear
//
// Description: This function clears all I/O Trap SMI status.
//
// Input:       None
//
// Output:      None
//
// Notes:       Porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IoTrapSmiClear(VOID)
{
    // Porting required
//    SET_MEM32_RCRB(RCRB_MMIO_TRSR, 0); // 0x1E00
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoTrapSmiDetect
//
// Description: This function detects I/O Trap SMI event.
//
// Input:       *IoTrapContext - Pointer to EFI_SMM_IO_TRAP_DISPATCH_CONTEXT
//
// Output:      TRUE - I/O Trap SMI occured, the SMI context IoTrapContext
//                     should be updated according to the traped H/W
//                     information.
//
// Notes:       Porting required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IoTrapSmiDetect(
    OUT EFI_SMM_IO_TRAP_REGISTER_CONTEXT    *IoTrapContext)
{
    /*
        UINT32      IoTrapStatus;
        UINT32      Buffer32;

        // Porting required
        IoTrapStatus = READ_MEM32_RCRB(RCRB_MMIO_TRSR) & 15; // 0x1E00

        if (IoTrapStatus) {

    //        IoTrapContext->TrapRegIndex = 0;

    //        while (IoTrapStatus != 1) {
    //            IoTrapStatus >>= 1;
    //            IoTrapContext->TrapRegIndex++;
    //        }

            Buffer32 = READ_MEM32_RCRB(RCRB_MMIO_TRCR); // 0x1E10
    //        IoTrapContext->TrapAddress = Buffer32 & 0xfffc;
            IoTrapContext->TrapOpType = (Buffer32 & 0x1000000) ? WriteIoCycle : ReadIoCycle;

    //        if (IoTrapContext->TrapOpType == WriteIoCycle)
    //            IoTrapContext->TrapData = READ_MEM32_RCRB(RCRB_MMIO_TWDR);

    //        return TRUE;
    //    }
    */
    return FALSE;
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
