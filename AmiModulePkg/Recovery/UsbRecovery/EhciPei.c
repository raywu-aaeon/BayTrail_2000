//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:    EhciPei.c
//
// Description:
//  This file is the main source file for the EHCI PEI USB
//  recovery module.  Its entry point at EhciPeiUsbEntryPoint
//  will be executed from the UsbRecoveryInitialize INIT_LIST.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Pei.h>
#include <AmiPeiLib.h>
#include <Token.h>
#include <PeiGetUCtrl.h>
#include <Ppi/Stall.h>
#include "UsbHostController.h"
#include <UsbPeim.h>
#include "EhciPei.h"
#include "usb.h"
#if (PEI_EHCI_SUPPORT == 1)
extern EFI_GUID gPeiUsbHostControllerPpiGuid;

EFI_STATUS EhciPeiBoardInit (
    IN EFI_PEI_SERVICES **PeiServices,
    EFI_PEI_PCI_CFG2_PPI *PciCfgPpi,
    EFI_PEI_STALL_PPI   *StallPpi );

UINT32 gEhciControllerPciTable[] = PEI_EHCI_PCI_BDFS;
UINT16 gEhciControllerCount = \
    sizeof(gEhciControllerPciTable) / sizeof(UINT32);

UINT32 gEhciControllerBaseAddress[] = PEI_EHCI_MEM_BASE_ADDRESSES;

BOOLEAN EHCIDbg_PortUsed = FALSE;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciGetOperationalRegisterBase
//
// Description: 
//      This function uses ControllerIndex and the global PCI_BUS_DEV_FUNCTION
//      array to access a particular controller's PCI configuration space in 
//      order to obtain the Operational Register base address.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  PEI Sevices table pointer
//      IN UINT16 ControllerIndex
//                  --  Index of the controller in the global
//                      PCI_BUS_DEV_FUNCTION array
//
// Output: 
//      UINT32 (Return Value)
//                  = Base address for this controller's operational
//                      registers.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 EhciGetOperationalRegisterBase (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN UINT16               ControllerIndex )
{
    UINT32 EhciBaseAddress;

    (*PeiServices)->PciCfg->Read(
        PeiServices,
        (*PeiServices)->PciCfg,
        EfiPeiPciCfgWidthUint32,
        gEhciControllerPciTable[ControllerIndex] + EHCI_BASE_ADDR_REG,
        &EhciBaseAddress
    );

    return EhciBaseAddress;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   Update_EHCIDbg_PortUsed
//
// Description: 
//          Check debugport of a controlller is in use or not.   
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  PEI Sevices table pointer
//      IN UINT16 ControllerIndex
//                  --  Index of the controller in the global
//                      PCI_BUS_DEV_FUNCTION array
//
// Output :
//
// Note : EHCIDbg_PortUsed used to maintain the debugport usage of 
//        CURRENT processing controller.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID Update_EHCIDbg_PortUsed(EFI_PEI_SERVICES    **PeiServices,UINT8 Index)
{
    UINT8   CmdReg = 0;
    UINT16  DebugBase = 0;
    UINT32  EhciBaseMem = 0;

//
// Read Command register and check MMIO enabled or not
//
    (*PeiServices)->PciCfg->Read( PeiServices, (*PeiServices)->PciCfg,
            EfiPeiPciCfgWidthUint8,
            gEhciControllerPciTable[Index] + EHCI_CMD_REGISTER,
            &CmdReg );

    if(!(CmdReg & 2)){ //MMIO is disabled
        EHCIDbg_PortUsed =  FALSE;
        return;
    }
//
// Retrieve the debugport base offset address
//
    (*PeiServices)->PciCfg->Read( PeiServices, (*PeiServices)->PciCfg,
            EfiPeiPciCfgWidthUint16,
            gEhciControllerPciTable[Index] + DEBUG_BASE,
            &DebugBase );

    DebugBase = DebugBase & 0x1FFF; // Bar number hardwired to 001b

    EhciBaseMem = EhciGetOperationalRegisterBase(PeiServices,Index);

//
// Check ENABLED_CNT bit of DEBUG status register
//
    if ((*(volatile UINT32*)(UINTN)(EhciBaseMem + DebugBase)) & BIT28){
            EHCIDbg_PortUsed = TRUE;
    }else{
            EHCIDbg_PortUsed = FALSE;
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciPeiUsbEntryPoint
//
// Description: 
//      This is the entry point into the EHCI controller initialization
//      subsystem.
//
// Input:
//      IN EFI_FFS_FILE_HEADER *FfsHeader
//                  --  EFI_FFS_FILE_HEADER pointer
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciPeiUsbEntryPoint (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    UINT8                Index;
    UINTN                MemPages;
    EFI_STATUS           Status;
    EFI_PHYSICAL_ADDRESS TempPtr;
    PEI_EHCI_DEV         *EhciDevPtr;
    EFI_PEI_STALL_PPI    *StallPpi;
    UINT8                ByteData;

    //-------------------------------------------
    // Initialize the EFI_PEI_STALL_PPI interface
    //-------------------------------------------
    Status = (**PeiServices).LocatePpi( PeiServices, &gEfiPeiStallPpiGuid,
        0, NULL, &StallPpi );
    if ( EFI_ERROR( Status ) ) {
        return EFI_UNSUPPORTED;
    }

    //-----------------------------------------
    // board specific initialization to program
    // PCI bridges and enable MMIO
    //-----------------------------------------

    Status = EhciPeiBoardInit(
        PeiServices,
        (*PeiServices)->PciCfg,
        StallPpi );
    if ( EFI_ERROR( Status ) ) {
        return EFI_UNSUPPORTED;
    }

    //----------------------------------------------------------
    // Allocate EHCI DEVICE OBJECT that holds all necessary
    // information for the Host Controller operational registers
    // for each controller.  Initialze the controller and setup
    // data structures for ready for operation
    //----------------------------------------------------------

    for (Index = 0; Index < gEhciControllerCount; Index++) {
    

    if (!(EhciGetOperationalRegisterBase( PeiServices, Index ))) {
        // Program PCI BAR and command register
        (*PeiServices)->PciCfg->Write( PeiServices, (*PeiServices)->PciCfg,
                EfiPeiPciCfgWidthUint32,
                gEhciControllerPciTable[Index] + EHCI_BASE_ADDR_REG,
                &gEhciControllerBaseAddress[Index] );
    }

        ByteData = 6;   // Enable MMIO and BusMaster
        (*PeiServices)->PciCfg->Write( PeiServices, (*PeiServices)->PciCfg,
                EfiPeiPciCfgWidthUint8,
                gEhciControllerPciTable[Index] + EHCI_CMD_REGISTER,
                &ByteData );


        Update_EHCIDbg_PortUsed(PeiServices,Index);

        // PAGESIZE = 0x1000
        MemPages = sizeof (PEI_EHCI_DEV) / 0x1000 + 1;
        Status = (**PeiServices).AllocatePages(
            PeiServices,
            EfiBootServicesData,
            MemPages,
            &TempPtr
                 );
        if (EFI_ERROR(Status)) {
            return EFI_OUT_OF_RESOURCES;
        }
        (**PeiServices).SetMem((VOID*)TempPtr, sizeof (PEI_EHCI_DEV), 0);

        EhciDevPtr = (PEI_EHCI_DEV *) ( (UINTN) TempPtr );
        EhciDevPtr->Signature = PEI_EHCI_DEV_SIGNATURE;
        EhciDevPtr->PeiServices = PeiServices;
        EhciDevPtr->CpuIoPpi = (*PeiServices)->CpuIo;
        EhciDevPtr->StallPpi = StallPpi;
        EhciDevPtr->PciCfgPpi = (*PeiServices)->PciCfg;
        
        EhciDevPtr->UsbHostControllerBaseAddress =
            EhciGetOperationalRegisterBase( PeiServices, Index );

        //Initialize the EHCI Controller for operation

        EhciInitHC( PeiServices, EhciDevPtr );

        //Setup PPI entry point
        EhciDevPtr->UsbHostControllerPpi.ControlTransfer = EhciHcControlTransfer;
        EhciDevPtr->UsbHostControllerPpi.BulkTransfer = EhciHcBulkTransfer;
        EhciDevPtr->UsbHostControllerPpi.GetRootHubPortNumber = EhciHcGetRootHubPortNumber;
        EhciDevPtr->UsbHostControllerPpi.GetRootHubPortStatus = EhciHcGetRootHubPortStatus;
        EhciDevPtr->UsbHostControllerPpi.SetRootHubPortFeature = EhciHcSetRootHubPortFeature;
        EhciDevPtr->UsbHostControllerPpi.ClearRootHubPortFeature = EhciHcClearRootHubPortFeature;
        EhciDevPtr->UsbHostControllerPpi.Reset = EhciHcReset;
                
        EhciDevPtr->UsbHostControllerPpi.DebugPortUsed = EHCIDbg_PortUsed;
        EhciDevPtr->UsbHostControllerPpi.PreConfigureDevice = NULL;
        EhciDevPtr->UsbHostControllerPpi.EnableEndpoints = NULL;

        EhciDevPtr->PpiDescriptor.Flags = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
        EhciDevPtr->PpiDescriptor.Guid = &gPeiUsbHostControllerPpiGuid;
        EhciDevPtr->PpiDescriptor.Ppi = &EhciDevPtr->UsbHostControllerPpi;

        //Now is the time to install the PPI
        Status = (**PeiServices).InstallPpi( PeiServices, &EhciDevPtr->PpiDescriptor );
        if (EFI_ERROR(Status)) {
            return EFI_NOT_FOUND;
        }

    }

    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciInitHC
//
// Description: EHCI controller initialization.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_EHCI_DEV *EhciDevPtr
//                  --  PEI_EHCI_DEV pointer
//      IN UINT8 Index
//                  --  Index of this controller in the global 
//                      PCI_BUS_DEV_FUNCTION array
//
// Output: 
//      VOID (Return Value)
//
// Note:
//      EhciDevPtr->UsbHostControllerBaseAddress is is initialized
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EhciInitHC (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_EHCI_DEV     *EhciDevPtr)
{
    UINT8                   *pPtr;
    UINTN                   MemPages;
    EFI_STATUS              Status;
    EFI_PHYSICAL_ADDRESS    TempPtr;
    EHCI_HC_CAP_REG         *EhciHcCapReg =
        (EHCI_HC_CAP_REG *)(UINTN)EhciDevPtr->UsbHostControllerBaseAddress;
    EHCI_HC_OPER_REG        *EhciHcOpReg;
    EHCI_DESC_PTRS          *pstEHCIDescPtrs;


    // Allocate memory for control transfers and bulk transfers
    // Allocate a block of memory and define/initialize 
    // Setup Control and Bulk EDs/TDs
    pstEHCIDescPtrs = &(EhciDevPtr->stEHCIDescPtrs);

    MemPages = (( 4 * sizeof (EHCI_QTD) ) + (2 * sizeof(EHCI_QH) )) / 0x1000 + 1;
    Status = (**PeiServices).AllocatePages(PeiServices,
                                           EfiBootServicesData,
                                           MemPages,
                                           &TempPtr);
    ASSERT_PEI_ERROR(PeiServices, Status);

    pPtr = (UINT8 *) ( (UINTN) TempPtr );
    if (pPtr == NULL) {
        return;
    }
    MemSet( pPtr, 4 * sizeof (EHCI_QTD) + 2 * sizeof(EHCI_QH), 0 );

    pstEHCIDescPtrs->fpQHControl = (EHCI_QH*) pPtr;
    pPtr += sizeof (EHCI_QH);
    pstEHCIDescPtrs->fpqTDControlSetup = (EHCI_QTD*) pPtr;
    pPtr += sizeof (EHCI_QTD);
    pstEHCIDescPtrs->fpqTDControlData = (EHCI_QTD*) pPtr;
    pPtr += sizeof (EHCI_QTD);
    pstEHCIDescPtrs->fpqTDControlStatus = (EHCI_QTD*) pPtr;
    pPtr += sizeof (EHCI_QTD);
    pstEHCIDescPtrs->fpQHBulk = (EHCI_QH*) pPtr;
    pPtr += sizeof (EHCI_QH);
    pstEHCIDescPtrs->fpqTDBulkData = (EHCI_QTD*) pPtr;

    // Store number of downstream ports into PEI_EHCI_DEV struct
    EhciDevPtr->bNumPorts = (UINT8)(EhciHcCapReg->HcsParams.Field.NumberOfPorts);

    // Read the Capability Registers Length to find the Offset address for the
    // beginning of the operational registers
    EhciHcOpReg = (EHCI_HC_OPER_REG *)((UINTN)EhciHcCapReg + EhciHcCapReg->CapLength);
    EhciDevPtr->EhciHcOpReg = EhciHcOpReg;

    EhciDevPtr->HcStatus = 0;

    // Do a Host Controller reset first
    if (!EHCIDbg_PortUsed) {
        EhciReset(PeiServices, EhciDevPtr);
    }

    // Do not do any periodic schedule related initialization

    // Clear status register - all R/WC bits
    EhciHcOpReg->UsbSts.AllBits = 
           (EHCI_USB_INTERRUPT |        // Interrupt
            EHCI_USB_ERROR_INTERRUPT |  // Error interrupt
            EHCI_PORT_CHANGE_DETECT |   // Port Change Detect
            EHCI_FRAME_LIST_ROLLOVER |  // Frame List Rollover
            EHCI_HOST_SYSTEM_ERROR |    // Host System Error
            EHCI_INT_ASYNC_ADVANCE);    // Interrupt on Async Advance

    // Turn HC on
    EhciHcOpReg->UsbCmd.AllBits = EHCI_RUNSTOP | EHCI_INTTHRESHOLD;

    // Disconnect all ports from companion HC (if any) and route them to EHCI
    EhciHcOpReg->ConfigFlag = 1;

    EHCI_FIXED_DELAY_MS(EhciDevPtr, 100);   // 100msec delay to stabilize

}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciReset
//
// Description: 
//      This function performs a software reset of the host controller.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_EHCI_DEV *EhciDevPtr
//                  --  PEI_EHCI_DEV pointer
//
// Output: 
//      VOID (Return Value)
//
// Notes:
//      It is assumed that all necessary operational register data has been 
//      saved prior to calling this function.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
EhciReset (
    EFI_PEI_SERVICES **PeiServices,
    PEI_EHCI_DEV     *EhciDevPtr
)
{
    UINT32  UsbCmd;
    UINT8   Count;

    // Check HC is halted: attempting to reset an actively running HC will
    // result in undefined behavior.
    if (EhciDevPtr->EhciHcOpReg->UsbSts.Field.HCHalted == 0)
    {
        PEI_TRACE ((EFI_D_ERROR, PeiServices, "EHCI HC RESET ERROR: HC is running.\n"));
        EhciDevPtr->HcStatus = EHCI_RESET_ERROR;
        return;
    }

    // Issue reset
    UsbCmd = EhciDevPtr->EhciHcOpReg->UsbCmd.AllBits | EHCI_HCRESET;
    EhciDevPtr->EhciHcOpReg->UsbCmd.AllBits = UsbCmd;
    
    // EHCI_HCRESET bit is set to zero by the Host Controller when the reset
    // process is complete.
    for (Count = 0; Count < 10; Count++)
    {
        EHCI_FIXED_DELAY_MS( EhciDevPtr, 10);   // 10msec delay
        if (EhciDevPtr->EhciHcOpReg->UsbCmd.Field.HcReset == 0)
        {
            return;
        }
    }

    PEI_TRACE ((EFI_D_ERROR, PeiServices, "EHCI HC RESET ERROR: timeout.\n"));
    EhciDevPtr->HcStatus = EHCI_RESET_ERROR;

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EhciReleasePortOwner
//
// Description: 
//
// Input:   
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EhciReleasePortOwner (
    EFI_PEI_SERVICES **PeiServices,
    PEI_EHCI_DEV     *EhciDevPtr,
    UINT8            PortNumber
)
{
    EHCI_PORTSC	*PortScReg = &EhciDevPtr->EhciHcOpReg->PortSC[PortNumber - 1];
    UINT8		i = 0;

    // Clear connect status change bit
    PortScReg->Field.ConnectStatusChange = 1;

    // Release port owner
    PortScReg->Field.PortOwner = 1;
    for (i = 0; i < 20; i++) {
        if (PortScReg->Field.ConnectStatusChange != 0) {
            break;
        }
        EHCI_FIXED_DELAY_MS( EhciDevPtr, 1);   // 1 msec delay
    }

    PortScReg->Field.ConnectStatusChange = 1;	
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciHcGetRootHubPortStatus
//
// Description: 
//      This function obtains the port status and port change status for
//      a port specified by PortNumber and updates the EFI_USB_PORT_STATUS 
//      data structure as specified the the PortStatus pointer parameter.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_USB_HOST_CONTROLLER_PPI *This
//                  --  PEI_USB_HOST_CONTROLLER_PPI pointer
//      IN UINT8 PortNumber
//                  --  Port number for which status is requested
//      OUT EFI_USB_PORT_STATUS *PortStatus
//                  --  EFI_USB_PORT_STATUS pointer's data is updated
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciHcGetRootHubPortStatus (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *This,
    IN UINT8                       PortNumber,
    OUT EFI_USB_PORT_STATUS        *PortStatus )
{
    PEI_EHCI_DEV    *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS( This );
    EHCI_PORTSC     *PortScReg = &EhciDevPtr->EhciHcOpReg->PortSC[PortNumber - 1];


    if (PortNumber > EhciDevPtr->bNumPorts) {
        return EFI_INVALID_PARAMETER;
    }

    //PEI_TRACE((-1, PeiServices, "EHCI port[%d] status: %08x\n", PortNumber, PortScReg->AllBits));
    PortStatus->PortStatus = 0;
    PortStatus->PortChangeStatus = 0;

    if (PortScReg->Field.CurrentConnectStatus != 0 && 
        PortScReg->Field.PortEnable == 0 &&
        PortScReg->Field.LineStatus == 0x1) {
        EhciReleasePortOwner(PeiServices, EhciDevPtr, PortNumber);
    }

    if (PortScReg->Field.CurrentConnectStatus != 0) {
        PortStatus->PortStatus |= USB_PORT_STAT_CONNECTION;
        if (PortScReg->Field.PortEnable != 0) {
            PortStatus->PortStatus |= USB_PORT_STAT_ENABLE;
            PortStatus->PortStatus |= USB_PORT_STAT_HIGH_SPEED;
        }
    }
    if (PortScReg->Field.ConnectStatusChange != 0) {
        PortStatus->PortChangeStatus |= USB_PORT_STAT_C_CONNECTION;
    }
    if (PortScReg->Field.PortEnableChange != 0) {
        PortStatus->PortChangeStatus |= USB_PORT_STAT_C_ENABLE;
    }
    if (PortScReg->Field.OvercurrentActive != 0) {
        PortStatus->PortStatus |= USB_PORT_STAT_OVERCURRENT;
    }
    if (PortScReg->Field.OvercurrentChange != 0) {
        PortStatus->PortChangeStatus |= USB_PORT_STAT_C_OVERCURRENT;
    }
    if (PortScReg->Field.Suspend != 0) {
        PortStatus->PortStatus |= USB_PORT_STAT_SUSPEND;
    }
    if (PortScReg->Field.PortReset != 0) {
        PortStatus->PortStatus |= USB_PORT_STAT_RESET;
    }
    if (PortScReg->Field.PortPower != 0) {
        PortStatus->PortStatus |= USB_PORT_STAT_POWER;
    }
    if (PortScReg->Field.PortOwner != 1) {
        PortStatus->PortStatus |= USB_PORT_STAT_OWNER;
    }
    if (EhciDevPtr->PortResetStatusChangeMap & (1 << (PortNumber - 1))) {
        PortStatus->PortChangeStatus |= USB_PORT_STAT_C_RESET;
    }

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciHcGetRootHubPortNumber
//
// Description: 
//      This function returns the number of downstream ports as specified 
//      in the HcRhDescriptorA operational register.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_USB_HOST_CONTROLLER_PPI *This
//                  --  PEI_USB_HOST_CONTROLLER_PPI pointer
//      OUT UINT8 *PortNumber
//                  --  Number of downstream ports
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciHcGetRootHubPortNumber (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *This,
    OUT UINT8                      *PortNumber )
{

    PEI_EHCI_DEV *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS( This );
        
    if (PortNumber == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    
    *PortNumber = EhciDevPtr->bNumPorts;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciHcSetRootHubPortFeature
//
// Description: 
//      This function sets port feature as specified by
//      PortFeature for the port specified by PortNumber.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_USB_HOST_CONTROLLER_PPI *This
//                  --  PEI_USB_HOST_CONTROLLER_PPI pointer
//      IN UINT8 PortNumber
//                  --  Port number whose feature is to be set
//      IN EFI_USB_PORT_FEATURE PortFeature
//                  --  Feature to set
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciHcSetRootHubPortFeature (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *This,
    IN UINT8                       PortNumber,
    IN EFI_USB_PORT_FEATURE        PortFeature )
{
    PEI_EHCI_DEV *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS( This ); 
    EHCI_PORTSC	*PortScReg = &EhciDevPtr->EhciHcOpReg->PortSC[PortNumber - 1];
    UINT32		PortSc = 0;
    UINT8		i = 0;

    if (PortNumber > EhciDevPtr->bNumPorts) {
        return EFI_INVALID_PARAMETER;
    }

    PortSc = PortScReg->AllBits;
    PortSc &= ~(EHCI_CONNECTSTATUSCHANGE | EHCI_PORTENABLESTATUSCHANGE | EHCI_OVERCURRENTCAHGE);

    switch (PortFeature) {
        case EfiUsbPortEnable:
            break;

        case EfiUsbPortSuspend:
            PortSc |= EHCI_SUSPEND;
            break;

        case EfiUsbPortReset:
            PortSc = PortScReg->AllBits;
            PortSc &= ~(EHCI_CONNECTSTATUSCHANGE | EHCI_PORTENABLESTATUSCHANGE | EHCI_OVERCURRENTCAHGE);
            PortScReg->AllBits = (PortSc & ~EHCI_PORTENABLE) | EHCI_PORTRESET;
            EHCI_FIXED_DELAY_MS( EhciDevPtr, 50);   // 50 msec delay

            PortSc = PortScReg->AllBits;
            PortSc &= ~(EHCI_PORTRESET | EHCI_CONNECTSTATUSCHANGE | EHCI_PORTENABLESTATUSCHANGE | 
                            EHCI_OVERCURRENTCAHGE);
            PortScReg->AllBits = PortSc;

            for (i = 0; i < 10; i++) {
                if (PortScReg->Field.PortReset == 0) {
                    break;
                }
                EHCI_FIXED_DELAY_MS( EhciDevPtr, 1);   // 50 msec delay
            }

            if (PortScReg->Field.PortEnable != 0) {
                EhciDevPtr->PortResetStatusChangeMap |= (1 << (PortNumber - 1));
            } else {
                EhciReleasePortOwner(PeiServices, EhciDevPtr, PortNumber);
            }
            //break;
            return EFI_SUCCESS;

        case EfiUsbPortPower:
            PortSc |= EHCI_PORTPOWER;
            break; 

        default:
            return EFI_INVALID_PARAMETER;
    }
    PortScReg->AllBits = PortSc;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciHcClearRootHubPortFeature
//
// Description: 
//      This function clears an EHCI specification port feature as specified
//      by PortFeature for the port specified by PortNumber.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_USB_HOST_CONTROLLER_PPI *This
//                  --  PEI_USB_HOST_CONTROLLER_PPI pointer
//      IN UINT8 PortNumber
//                  --  Port number whose feature is to be set
//      IN EFI_USB_PORT_FEATURE PortFeature
//                  --  Feature to set
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciHcClearRootHubPortFeature (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *This,
    IN UINT8                       PortNumber,
    IN EFI_USB_PORT_FEATURE        PortFeature )
{
    PEI_EHCI_DEV *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS( This );
    UINT32 PortSc = 0;

    if (PortNumber > EhciDevPtr->bNumPorts) {
        return EFI_INVALID_PARAMETER;
    }

    PortSc = EhciDevPtr->EhciHcOpReg->PortSC[PortNumber - 1].AllBits;
    PortSc &= ~(EHCI_CONNECTSTATUSCHANGE | EHCI_PORTENABLESTATUSCHANGE | EHCI_OVERCURRENTCAHGE);

    switch (PortFeature) {
        case EfiUsbPortEnable:
            PortSc &= ~EHCI_PORTENABLE;
            break;

        case EfiUsbPortSuspend:
            PortSc |= EHCI_FORCEPORTRESUME;
            break;

        case EfiUsbPortReset:
            PortSc &= ~EHCI_PORTRESET;
            break;

        case EfiUsbPortPower:
            PortSc &= ~EHCI_PORTPOWER;
            break; 

        case EfiUsbPortOwner:
            //PortSc |= EHCI_PORTOWNER;
            EhciReleasePortOwner(PeiServices, EhciDevPtr, PortNumber);
            //break;
            return EFI_SUCCESS;

        case EfiUsbPortConnectChange:
            PortSc |= EHCI_CONNECTSTATUSCHANGE;
            break;

        case EfiUsbPortEnableChange:
            PortSc |= EHCI_PORTENABLESTATUSCHANGE;
            break;

        case EfiUsbPortSuspendChange:
            break;

        case EfiUsbPortOverCurrentChange:
            PortSc |= EHCI_OVERCURRENTCAHGE;
            break;

        case EfiUsbPortResetChange:
            EhciDevPtr->PortResetStatusChangeMap &= ~(1 << (PortNumber - 1));
            //break;
            return EFI_SUCCESS;

        default:
            return EFI_INVALID_PARAMETER;
    }
    EhciDevPtr->EhciHcOpReg->PortSC[PortNumber - 1].AllBits = PortSc;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EhciHcReset
//
// Description: This routine resets the Host Controler and its USB bus 
//              according to the attributes
// Input:   EFI_PEI_SERVICES            **PeiServices,
//          PEI_USB_HOST_CONTROLLER_PPI	*This,
//          UINT16                      Attributes
//
// Output:
//     EFI_SUCCESS - The reset operation succeeded.
//     EFI_UNSUPPORTED - The type of reset specified by Attributes is not
//	                 currently supported.
//     EFI_ACCESS_DENIED - Reset operation is rejected due to the debug 
//	                   port being configured and active; only
//	                   EFI_USB_HC_RESET_GLOBAL_WITH_DEBUG or 
//	                   EFI_USB_HC_RESET_HOST_WITH_DEBUG reset Attributes
//	                   can be used to perform reset operation for this 
//	                   host controller.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EhciHcReset (
    IN EFI_PEI_SERVICES             	**PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI	*This,
    IN UINT16                           Attributes
)
{    
    UINTN               i;
    PEI_EHCI_DEV        *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS(This);	
    EHCI_HC_OPER_REG    *EhciHcOpReg = EhciDevPtr->EhciHcOpReg;
    
    switch (Attributes) {
        case EFI_USB_HC_RESET_GLOBAL:   
            // Stop periodic scheduling
            EhciHcOpReg->UsbCmd.AllBits &= ~EHCI_PER_SCHED_ENABLE;
            for (i = 0; i < 1000; i++) {
                EHCI_FIXED_DELAY_US(EhciDevPtr, 10);    // 10 us
                if (EhciHcOpReg->UsbSts.Field.PeriodicSheduleStatus == 0) {
                    break;
                }
            }

            // Check HC is runnning, if so STOP before proceeding	
            if (EhciHcOpReg->UsbSts.Field.HCHalted == 0) {
                EhciHcOpReg->UsbCmd.AllBits &= ~(EHCI_RUNSTOP);
                for (i = 0; i < 16; i++) {
                    EHCI_FIXED_DELAY_US(EhciDevPtr, 125);   // 125 us
                    if (EhciHcOpReg->UsbSts.Field.HCHalted) {
                        break;
                    }
                }
            }

            if (EhciDevPtr->UsbHostControllerPpi.DebugPortUsed) {
                return EFI_ACCESS_DENIED;
            }
            
            // Reset Host Controller
            EhciReset(PeiServices, EhciDevPtr);

            break;
        default:
            return EFI_UNSUPPORTED;
    }

    return EFI_SUCCESS;    
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EHCIInitializeQueueHead
//
// Description: This function initializes the queue head with default values
//
// Input:       fpQH    Pointer to queue head
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCIInitializeQueueHead(EHCI_QH *fpQH)
{
    fpQH->dNextqTDPtr       = 1;
    fpQH->dAltNextqTDPtr    = 1;
    fpQH->dCurqTDPtr        = 1;

    fpQH->dEndPntCap        = QH_ONE_XFER;
    fpQH->dToken            = 0;
    fpQH->dEndPntCharac     = 0;
    fpQH->dBufferPtr0       = 0;
    fpQH->dBufferPtr1       = 0;
    fpQH->dBufferPtr2       = 0;
    fpQH->dBufferPtr3       = 0;
    fpQH->dBufferPtr4       = 0;
    fpQH->bErrorStatus      = 0;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EHCISetQTDBufferPointers
//
// Description: This function will set the 5 buffer pointer in the qTD
//              appropriately depending upon the input size
//
// Input:   fpQtd   - Pointer to the qTD
//          fpBuf   - 32bit absolute buffer pointer
//          wSize   - Amount of data to be transferred
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCISetQTDBufferPointers(
    EHCI_QTD    *fpQtd,
    UINT8       *fpBuf,
    UINT32      dSize
)
{
    UINT16      wBufSize;
    UINT8       *fpBuffer   = fpBuf;
    UINT32      *fpBufferPtr;
    UINT16      w4KRemainder;

    //
    // Fill the buffer pointers with 0s
    //
    fpQtd->dBufferPtr0      = 0;
    fpQtd->dBufferPtr1      = 0;
    fpQtd->dBufferPtr2      = 0;
    fpQtd->dBufferPtr3      = 0;
    fpQtd->dBufferPtr4      = 0;
    fpQtd->dAltNextqTDPtr   = 1;

    //
    // If size to transfer is 0 skip updating pointers
    //
    if (!dSize)
    {
        return;
    }

    //
    // Make sure the amount of data to be xferred is 16K or less
    //
    wBufSize = (UINT16)((dSize > PEI_MAX_EHCI_DATA_SIZE) ? PEI_MAX_EHCI_DATA_SIZE : dSize);

    fpBufferPtr = &fpQtd->dBufferPtr0;

    for (;;)
    {
        *fpBufferPtr = (UINT32)(UINTN)fpBuffer;
        //
        // Calculate the number of bytes that can be transferred using current
        // buffer pointer
        //
        w4KRemainder = (UINT16)((((UINT32)((UINTN)fpBuffer+0x1000)) & 0xFFFFF000) -
                                        (UINT32)(UINTN)fpBuffer);

        //
        // Check whether all the bytes can be accomadated in the current buffer
        //
        if (w4KRemainder >= wBufSize)
        {
            break;  // Yes. Current Buffer is sufficient for the rest of data
        }

        //
        // We have more data to transfer - adjust data and store it in the next pointer
        //
        wBufSize = (UINT16)(wBufSize - w4KRemainder);   // Amount of data remaining
        fpBuffer = fpBuffer + w4KRemainder;     // Adjust buffer (4K bound)
        fpBufferPtr++;                          // Next buffer pointer
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIStartAsyncSchedule
//
// Description: This function starts the asynchronous schedule
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EHCIStartAsyncSchedule(
    EHCI_HC_OPER_REG    *HcOpReg
)
{
    // Start the Async schedule
    HcOpReg->UsbCmd.AllBits |= EHCI_ASYNC_SCHED_ENABLE;

    // Make sure the HC started the async. execution
    for (;HcOpReg->UsbSts.Field.AsyncSheduleStatus == 0;) {}
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIStopAsyncSchedule
//
// Description: This function stops the asynchronous transfer and sets the
//              asynchronous pointer to null
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EHCIStopAsyncSchedule(
    EHCI_HC_OPER_REG    *HcOpReg
)
{
    // Stop the Async schedule
    HcOpReg->UsbCmd.AllBits &= ~EHCI_ASYNC_SCHED_ENABLE;

    // Make sure the HC stopped the async. execution
    for (;HcOpReg->UsbSts.Field.AsyncSheduleStatus != 0;) {}
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EHCIProcessQH
//
// Description: This function whether all the TD's in the QH is completed
//
// Input:       fpQH    - Pointer to the QH which has to be completed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EHCIProcessQH(
    EHCI_QH *fpQH
)
{
    EHCI_QTD *fpQTD = fpQH->fpFirstqTD;

    for (;;) {
        //
        // Check whether the qTD is active, if so. Exit!
        //
        if (fpQTD->dToken & QTD_ACTIVE) {
            return; // Not complete
        }

        //
        // Check for halt condition, if halted - exit
        //
        if (fpQTD->dToken & QTD_HALTED) {
            //
            // Set the QH halted status
            //
            fpQH->bErrorStatus = QTD_HALTED;
            //
            // Set the QH as in-active
            //
            fpQH->bActive = FALSE;

            return; // Complete
        }
        //
        // qTD is not active and not halted. That is it is completed successfully
        // Check whether this qTD is the last one in the list
        //
        if (fpQTD->dNextqTDPtr & EHCI_TERMINATE) {
            //
            // Set the QH as in-active
            //
            fpQH->bActive = FALSE;
            return; //  Complete
        }
        //
        // More qTDs are in the list. Process next qTD
        //
        fpQTD = (EHCI_QTD*)(UINTN)fpQTD->dNextqTDPtr;
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIWaitForAsyncTransferComplete
//
// Description: This function waits for asynchronous transfer completion.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EHCIWaitForAsyncTransferComplete (
    EFI_PEI_SERVICES    **PeiServices,
    PEI_EHCI_DEV        *EhciDevPtr,
    EHCI_QH             *QHead,
    UINT32              TimeOut
)
{
    UINT32              Count;
//    UINT32              TimeOut = 15000; // 5 sec

    for (Count = 0; Count < TimeOut; Count++) {
        EHCIProcessQH(QHead);

        if (QHead->bActive == FALSE) {
            return;
        }
        EHCI_FIXED_DELAY_MS(EhciDevPtr, 1);   // 1 msec delay
    }

    PEI_TRACE((EFI_D_ERROR, PeiServices, "EHCI Time-Out:\n"));
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciHcControlTransfer
//
// Description: 
//      This function intiates a USB control transfer and waits on it to 
//      complete.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_USB_HOST_CONTROLLER_PPI *This
//                  --  PEI_USB_HOST_CONTROLLER_PPI pointer
//      IN UINT8 bDeviceAddress
//                  --  USB address of the device for which the control 
//                      transfer is to be issued
//      IN UINT8 DeviceSpeed
//                  --  Not used
//      IN UINT8 MaximumPacketLength
//                  --  Maximum number of bytes that can be sent to or 
//                      received from the endpoint in a single data packet
//      IN EFI_USB_DEVICE_REQUEST *Request
//                  --  EFI_USB_DEVICE_REQUEST pointer
//      IN EFI_USB_DATA_DIRECTION TransferDirection
//                  --  Direction of transfer
//      OPTIONAL IN OUT VOID *DataBuffer        
//                  --  Pointer to source or destination buffer
//      OPTIONAL IN OUT UINTN *DataLength       
//                  --  Length of buffer
//      IN UINTN TimeOut
//                  --  Not used
//      OUT UINT32 *TransferResult
//                  --  Not used
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciHcControlTransfer (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *This,
    IN UINT8                       bDeviceAddress,
    IN UINT8                       DeviceSpeed,
    IN UINT8                       MaximumPacketLength,
    IN UINT16                      TransactionTranslator    OPTIONAL,
    IN EFI_USB_DEVICE_REQUEST      *Request,
    IN EFI_USB_DATA_DIRECTION      TransferDirection,
    IN OUT VOID *DataBuffer        OPTIONAL,
    IN OUT UINTN *DataLength       OPTIONAL,
    IN UINTN                       TimeOut,
    OUT UINT32                     *TransferResult )
{
    UINT16            WordRequest;
    EFI_STATUS        Status = EFI_SUCCESS;


    PEI_EHCI_DEV        *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS( This );
    EHCI_HC_OPER_REG    *HcOpReg = EhciDevPtr->EhciHcOpReg;
    EHCI_DESC_PTRS      *DescPtrs = &EhciDevPtr->stEHCIDescPtrs;
    UINT16              wLength = (DataLength != NULL)? (UINT16) *DataLength : 0;
    EHCI_QH             *fpQHCtl;
    EHCI_QTD            *fpQTDCtlSetup, *fpQTDCtlData, *fpQTDCtlStatus;
    UINT32              dTmp, dTmp1;

    *TransferResult = EFI_USB_NOERROR;

    WordRequest = (Request->Request << 8) | Request->RequestType;

    //
    // Intialize the queue head with null pointers
    //
    fpQHCtl = DescPtrs->fpQHControl;
    EHCIInitializeQueueHead(fpQHCtl);

    //
    // Assume as a high speed device
    //
    dTmp = QH_HIGH_SPEED;   // 10b - High speed

    if (DeviceSpeed != USB_HIGH_SPEED_DEVICE && TransactionTranslator != 0)
    {
        // Low/Full speed device OR device is connected to a root port
        // DeviceSpeed = 1 (low) or 2 (full)
        dTmp = (UINT32)((DeviceSpeed & 1) << 12);    // Bit 12 = full/low speed flag
        dTmp |= QH_CONTROL_ENDPOINT;
        //
        // Set the hub address and port number
        //
        dTmp1 = (TransactionTranslator << 16) | (BIT10+BIT11+BIT12);    // Split complete Xaction
        fpQHCtl->dEndPntCap |= dTmp1;
    }

    dTmp |= (QH_USE_QTD_DT | QH_HEAD_OF_LIST);
    dTmp |= (UINT32)bDeviceAddress;

    //
    // dTmp[Bits 6:0] = Dev. Addr
    // dTmp[Bit7] = I bit(0)
    // dTmp[Bits11:8] = Endpoint (0)
    //
    dTmp1 = MaximumPacketLength;
    dTmp |= (dTmp1 << 16);  // Tmp[Bits26:16] = device's packet size
    fpQHCtl->dEndPntCharac = dTmp;

    //
    // Fill in various fields in the qTDControlSetup.
    //
    fpQTDCtlSetup = DescPtrs->fpqTDControlSetup;

    //
    // The token field will be set so
    //   Direction PID = QTD_SETUP_TOKEN,
    //   Size = size of the data,
    //   Data Toggle = QTD_SETUP_TOGGLE,
    //   Error Count = QTD_NO_ERRORS,
    //   Status code = QTD_DO_OUT + QTD_ACTIVE
    // The buffer pointers field will point to the aControlSetupData buffer
    //   which was before initialized to contain a DeviceRequest struc.
    // The dNextqTDPtr field will point to the qTDControlData if data will
    //   be sent/received or to the qTDControlStatus if no data is expected.
    // The dAltNextqTDPtr field will be set to EHCI_TERMINATE
    //
    fpQTDCtlSetup->dToken = QTD_SETUP_TOKEN | QTD_SETUP_TOGGLE | QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_DO_OUT | QTD_ACTIVE | 
                (8 << 16);  // Data size

    //
    // Update buffer pointers
    //
    EHCISetQTDBufferPointers(fpQTDCtlSetup,
            (UINT8*)Request,
            8);
    fpQTDCtlData = DescPtrs->fpqTDControlData;

    if (wLength != 0 && DataBuffer != NULL)    // br if no data to transfer
    {
        //
        // Fill in various fields in the qTDControlData
        //
        // The token field will be set so
        //   Direction PID = QTD_OUT_TOKEN/QTD_IN_TOKEN,
        //   Size = size of the data,
        //   Data Toggle = QTD_DATA1_TOGGLE,
        //   Error Count = QTD_NO_ERRORS,
        //   Status code = QTD_DO_OUT(if it is out) + QTD_ACTIVE
        // The buffer pointers field will point to the fpBuffer buffer
        //   which was before initialized to contain a DeviceRequest struc.
        // The dNextqTDPtr field will point to the qTDControlSetup
        // The dAltNextqTDPtr field will be set to EHCI_TERMINATE
        //
        fpQTDCtlData->dToken = QTD_IN_TOKEN |
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_ACTIVE;
        if ((WordRequest & BIT7) == 0) // Br if host sending data to device (OUT)
        {
            fpQTDCtlData->dToken    = QTD_OUT_TOKEN |
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_DO_OUT | QTD_ACTIVE;
        }

        //
        // Set length
        //
        fpQTDCtlData->dToken |= ((UINT32)wLength << 16);

        //
        // Update buffer pointers
        //
        EHCISetQTDBufferPointers(fpQTDCtlData,
                (UINT8*)DataBuffer,
                (UINT32)wLength);
    }

    //
    // Fill in various fields in the qTDControlStatus
    //
    fpQTDCtlStatus = DescPtrs->fpqTDControlStatus;

    //
    // The token field will be set so
    //   Direction PID = QTD_OUT_TOKEN/QTD_IN_TOKEN,
    //   Size = 0,
    //   Data Toggle = QTD_DATA1_TOGGLE,
    //   Error Count = QTD_NO_ERRORS,
    //   Status code = QTD_DO_OUT(if it is out) + QTD_ACTIVE
    // The buffer pointers field will be 0
    // The dNextqTDPtr field will set to EHCI_TERMINATE
    // The dAltNextqTDPtr field will be set to EHCI_TERMINATE
    //
    // For OUT control transfer status should be IN and
    // for IN cotrol transfer, status should be OUT
    //
    fpQTDCtlStatus->dToken = QTD_IN_TOKEN | 
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_ACTIVE;
    if((WordRequest & BIT7) != 0)
    {
        fpQTDCtlStatus->dToken  = QTD_OUT_TOKEN |
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_DO_OUT | QTD_ACTIVE;
    }

    EHCISetQTDBufferPointers(fpQTDCtlStatus, NULL, 0);

    //
    // Link the qTD formed now and connect them with the control queue head
    //
    fpQHCtl->fpFirstqTD     = fpQTDCtlSetup;
    fpQHCtl->dNextqTDPtr    = (UINT32)(UINTN)fpQTDCtlSetup;

    if (wLength != 0)
    {
        fpQTDCtlSetup->dNextqTDPtr  = (UINT32)(UINTN)fpQTDCtlData;
        fpQTDCtlData->dNextqTDPtr   = (UINT32)(UINTN)fpQTDCtlStatus;
    }
    else
    {
        fpQTDCtlSetup->dNextqTDPtr  = (UINT32)(UINTN)fpQTDCtlStatus;
    }

    fpQTDCtlStatus->dNextqTDPtr = EHCI_TERMINATE;

    //
    // Set the ASYNCLISTADDR register to point to the QHControl
    //
    HcOpReg->AsyncListAddr = (UINT32)(UINTN)fpQHCtl;

    //
    // Set next QH pointer to itself (circular link)
    //
    fpQHCtl->dLinkPointer = (UINT32)((UINTN)fpQHCtl | EHCI_QUEUE_HEAD);
    fpQHCtl->bActive = TRUE;

    //
    // Now put the control setup, data and status into the HC's schedule by
    // setting the Async. schedule enabled field of USBCMD register
    // This will cause the HC to execute the transaction in the next active frame.
    //
    EHCIStartAsyncSchedule(HcOpReg);

    EHCIWaitForAsyncTransferComplete(PeiServices, EhciDevPtr, fpQHCtl, TimeOut);

    //
    // Stop the Async transfer
    //
    EHCIStopAsyncSchedule(HcOpReg);

    //
    // Check whether the QH stopped or timed out
    //
    if (fpQHCtl->bActive == TRUE) {

        fpQHCtl->bActive = FALSE;
        Status = EFI_TIMEOUT;
        *TransferResult |= EFI_USB_ERR_TIMEOUT;
    }

    //
    // Check for the stall condition
    //
    if (fpQHCtl->bErrorStatus & QTD_HALTED) {
        //
        // Command stalled set the error bit appropriately
        //
        Status = EFI_DEVICE_ERROR;
        *TransferResult |= EFI_USB_ERR_STALL;
    }

    fpQHCtl->fpFirstqTD     = 0;
    fpQHCtl->dNextqTDPtr    = EHCI_TERMINATE;

    return  Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciHcBulkTransfer
//
// Description: 
//      This function intiates a USB bulk transfer and waits on it to 
//      complete.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  EFI_PEI_SERVICES pointer
//      IN PEI_USB_HOST_CONTROLLER_PPI *This
//                  --  PEI_USB_HOST_CONTROLLER_PPI pointer
//      IN UINT8 DeviceAddress
//                  --  USB address of the device for which the control 
//                      transfer is to be issued
//      IN UINT8 EndPointAddress
//                  --  Particular endpoint for the device
//      IN UINT8 MaximumPacketLength
//                  --  Maximum number of bytes that can be sent to or 
//                      received from the endpoint in a single data packet
//      OPTIONAL IN OUT VOID *DataBuffer        
//                  --  Pointer to source or destination buffer
//      OPTIONAL IN OUT UINTN *DataLength       
//                  --  Length of buffer
//      IN OUT UINT8 *DataToggle
//                  --  Used to update the control/status DataToggle field
//                      of the Transfer Descriptor
//      IN UINTN TimeOut
//                  --  Not used
//      OUT UINT32 *TransferResult
//                  --  Not used
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EhciHcBulkTransfer (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *This,
    IN UINT8                       DeviceAddress,
    IN UINT8                       EndPointAddress,
    IN UINT8                       DeviceSpeed,
    IN UINT16                      MaximumPacketLength,
    IN UINT16                      TransactionTranslator    OPTIONAL,
    IN OUT VOID                    *DataBuffer,
    IN OUT UINTN                   *DataLength,
    IN OUT UINT8                   *DataToggle,
    IN UINTN                       TimeOut,
    OUT UINT32                     *TransferResult )
{
    PEI_EHCI_DEV        *EhciDevPtr = PEI_RECOVERY_USB_EHCI_DEV_FROM_THIS( This );
    EHCI_HC_OPER_REG    *HcOpReg = EhciDevPtr->EhciHcOpReg;
    EHCI_DESC_PTRS      *DescPtrs = &EhciDevPtr->stEHCIDescPtrs;
    EFI_STATUS          Status = EFI_SUCCESS;
    UINT32              dBytesToTransfer;
    UINT32              dTmp, dTmp1;
    EHCI_QH             *fpQHBulk;
    EHCI_QTD            *fpQTDBulkData;

    *TransferResult = EFI_USB_NOERROR;

    //PEI_TRACE((EFI_D_ERROR, PeiServices, "bulk..%x, device address %d\n", *DataLength, DeviceAddress));

    dBytesToTransfer =
        (*DataLength < PEI_MAX_EHCI_DATA_SIZE)? (UINT32)*DataLength : PEI_MAX_EHCI_DATA_SIZE;

    //
    // Set the QH's dNextqTDPtr field to bulk data qTD and dAltqTDPtr field to
    // EHCI_TERMINATE. Also set QH's link pointer to itself
    //
    fpQHBulk = DescPtrs->fpQHBulk;
    fpQTDBulkData = DescPtrs->fpqTDBulkData;

    //
    // Intialize the queue head
    //
    EHCIInitializeQueueHead(fpQHBulk);

    //
    // Set the first qTD pointer
    //
    fpQHBulk->fpFirstqTD    = fpQTDBulkData;
    fpQHBulk->dNextqTDPtr   = (UINT32)(UINTN)fpQTDBulkData;
    fpQHBulk->dLinkPointer  = (UINT32)((UINTN)fpQHBulk | EHCI_QUEUE_HEAD);

    //
    // Device address, Endpoint, max packet size, data toggle control
    //
    dTmp = (UINT32)(DeviceAddress | (EndPointAddress << 8));
    dTmp |= ((UINT32)MaximumPacketLength << 16);
    dTmp |= (QH_USE_QTD_DT | QH_HEAD_OF_LIST);

    //
    // Assume as a high speed device
    //
    dTmp |= QH_HIGH_SPEED;   // 10b - High speed

    if (DeviceSpeed != USB_HIGH_SPEED_DEVICE && TransactionTranslator != 0)
    {
        // Note: low speed bulk endpoints are not supported
        dTmp1 = BIT12;    // Bit 12 = full speed flag
        dTmp &= ~(QH_ENDPOINT_SPEED);
        dTmp |= dTmp1;
            //
            // Set the hub address and port number
            //
        dTmp1   = (TransactionTranslator << 16) | BIT14;    // Hispeed hub port number & device number
        fpQHBulk->dEndPntCap |= dTmp1;   // Split complete Xaction
    }

    //
    // Update the endpoint characteristcs field with the data formed
    //
    fpQHBulk->dEndPntCharac = dTmp;

    //
    // Fill the bulk data qTD with relevant information
    //
    if ((EndPointAddress & BIT7) != 0)
    {
        fpQTDBulkData->dToken   = QTD_IN_TOKEN |
                QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_ACTIVE;
    }
    else
    {
        fpQTDBulkData->dToken   = QTD_OUT_TOKEN |
                QTD_IOC_BIT |
                QTD_NO_ERRORS | QTD_DO_OUT | QTD_ACTIVE;
    }

    //
    // Set the data toggle depending on the DatToggle value
    //
    fpQTDBulkData->dToken |= ((UINT32)(*DataToggle)) << 31;

    //
    // Set length
    //
    fpQTDBulkData->dToken |= (dBytesToTransfer << 16);

    //
    // Update buffer pointers
    //
    EHCISetQTDBufferPointers(
        fpQTDBulkData, (UINT8*)DataBuffer, dBytesToTransfer);

    //
    // Update next & alternate next qTD pointers
    //
    fpQTDBulkData->dNextqTDPtr      = EHCI_TERMINATE;
    fpQTDBulkData->dAltNextqTDPtr   = EHCI_TERMINATE;

    //
    // Set the ASYNCLISTADDR register to point to the QHBulk
    //
    HcOpReg->AsyncListAddr = (UINT32)(UINTN)fpQHBulk;

    fpQHBulk->bActive   = TRUE;

    //
    // Now put the bulk QH into the HC's schedule by
    // setting the Async. schedule enabled field of USBCMD register
    // This will cause the HC to execute the transaction in the next active frame.
    //
    EHCIStartAsyncSchedule(HcOpReg);

    EHCIWaitForAsyncTransferComplete(PeiServices, EhciDevPtr, fpQHBulk, TimeOut);

    //
    // Stop the Async transfer
    //
    EHCIStopAsyncSchedule(HcOpReg);

    dTmp = 0;    // Return value

    //
    // Check whether the QH stopped or timed out
    //
    if (fpQHBulk->bActive == TRUE) {

        fpQHBulk->bActive = FALSE;

        //
        // Set time out status
        //
        Status = EFI_TIMEOUT;
        *TransferResult |= EFI_USB_ERR_TIMEOUT;
    }

    //
    // Check for the error conditions - if possible recover from them
    //
    if (fpQHBulk->bErrorStatus & QTD_HALTED) {
        //
        // Indicate Stall condition
        //
        Status = EFI_DEVICE_ERROR;
        *TransferResult |= EFI_USB_ERR_STALL;
    }

    if (Status == EFI_SUCCESS) {
        *DataToggle = (UINT8)(((fpQHBulk->dToken & QH_DATA_TOGGLE) >> 31) & 1);
        //
        // Get the size of data transferred
        //
        dTmp = (fpQTDBulkData->dToken & ~(QTD_DATA_TOGGLE)) >> 16;
        dTmp = (dTmp != 0)? dBytesToTransfer-dTmp : dBytesToTransfer;
    }

    fpQHBulk->fpFirstqTD = 0;
    fpQHBulk->dNextqTDPtr = EHCI_TERMINATE;

    *DataLength = (UINTN)dTmp;
    return  Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordReadMem
//
// Description: This routine reads a DWORD from the specified Memory Address
//
// Input:   dBaseAddr   - Memory address to read
//          bOffset     - Offset of dBaseAddr
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
DwordReadMem(UINT32 dBaseAddr, UINT16 wOffset)
{
    return *(volatile UINT32*)(UINTN)(dBaseAddr+wOffset);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordWriteMem
//
// Description: This routine writes a DWORD to a specified Memory Address
//
// Input:   dBaseAddr   - Memory address to write
//          bOffset     - Offset of dBaseAddr
//          dValue      - Data to write
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
DwordWriteMem(UINT32 dBaseAddr, UINT16 wOffset, UINT32 dValue)
{
    *(volatile UINT32*)(UINTN)(dBaseAddr+wOffset) = dValue;
}

#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
