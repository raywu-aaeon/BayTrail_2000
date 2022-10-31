//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2009, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/ehci.c 105   8/29/12 9:32a Ryanchou $
//
// $Revision: 105 $
//
// $Date: 8/29/12 9:32a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           Ehci.h
//
//  Description:    AMI USB EHCI support
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"

UINT8   EHCI_Start (HC_STRUC*);
UINT8   EHCI_Stop (HC_STRUC*);
UINT8   EHCI_EnumeratePorts (HC_STRUC*);
UINT8   EHCI_DisableInterrupts (HC_STRUC*);
UINT8   EHCI_EnableInterrupts (HC_STRUC*);
UINT8   EHCI_ProcessInterrupt(HC_STRUC*);
UINT8   EHCI_GetRootHubStatus (HC_STRUC*,UINT8);
UINT8   EHCI_DisableRootHub (HC_STRUC*,UINT8);
UINT8   EHCI_EnableRootHub (HC_STRUC*,UINT8);
UINT16  EHCI_ControlTransfer (HC_STRUC*,DEV_INFO*,UINT16,UINT16,UINT16,UINT8*,UINT16);
UINT32  EHCI_BulkTransfer (HC_STRUC*,DEV_INFO*,UINT8,UINT8*,UINT32);
UINT16  EHCI_InterruptTransfer (HC_STRUC*,DEV_INFO*,UINT8*,UINT16);
UINT8   EHCI_DeactivatePolling (HC_STRUC*,DEV_INFO*);
UINT8   EHCI_ActivatePolling (HC_STRUC*,DEV_INFO*);
UINT8   EHCI_DisableKeyRepeat (HC_STRUC*);
UINT8   EHCI_EnableKeyRepeat (HC_STRUC*);
UINT8   EHCI_ResetRootHub (HC_STRUC*,UINT8);
UINT8   EHCI_GlobalSuspend (HC_STRUC*);	//(EIP54018+)

UINT8   EHCIResetHC(HC_STRUC*);
UINT8   EHCIInitializePeriodicSchedule(HC_STRUC*, UINT32);
UINT8   EHCIProgramLegacyRegisters(HC_STRUC*, UINT8);
VOID    EHCIStartAsyncSchedule(HC_STRUC*);
VOID    EHCIStopAsyncSchedule(HC_STRUC*);
UINT8   EHCIStartPeriodicSchedule(HC_STRUC*);
UINT8   EHCIStopPeriodicSchedule(HC_STRUC*);
UINT8   EHCIProcessQH(HC_STRUC*, EHCI_QH*);
VOID    EHCIProcessPeriodicList(HC_STRUC*);
VOID    EHCIInitializeQueueHead (EHCI_QH*);
VOID    EHCISetQTDBufferPointers(EHCI_QTD*, UINT8*, UINT32);
UINT16  EHCIWaitForTransferComplete(HC_STRUC*, EHCI_QH*,DEV_INFO* );
UINT8   EhciAddPeriodicQh(HC_STRUC*,EHCI_QH*);
UINT8   EhciRemovePeriodicQh(HC_STRUC*,EHCI_QH*);
VOID    ProcessOwnerShipChangeSMI(HC_STRUC*);
VOID    ProcessSmiChangeToEHCD(HC_STRUC*);
VOID    ProcessSmiChangeToBIOS(HC_STRUC*);
UINT8   EHCIGetLegacySupportOffset(HC_STRUC*, UINT16);
VOID    EHCIRemoveQHFromAsyncList(HC_STRUC*, EHCI_QH*);
UINT8	EhciPollingTDCallback(HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);
UINT8	EhciRepeatTDCallback(HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);

UINT32  ReadPCIConfig(UINT16, UINT8);
VOID    WordWritePCIConfig(UINT16, UINT8, UINT16);
VOID    DwordWritePCIConfig(UINT16, UINT8, UINT32);

UINT32  DwordReadMem(UINT32, UINT16);
VOID    DwordWriteMem(UINT32, UINT16, UINT32);
VOID    DwordResetMem(UINT32, UINT16, UINT32);
VOID    DwordSetMem(UINT32, UINT16, UINT32);

UINT32	EhciReadPciReg(HC_STRUC*, UINT32);
VOID	EhciWritePciReg(HC_STRUC*, UINT32, UINT32);
UINT32	EhciReadHcMem(HC_STRUC*, UINT32);
VOID	EhciWriteHcMem(HC_STRUC*, UINT32, UINT32);
UINT32	EhciReadOpReg(HC_STRUC*, UINT32);
VOID	EhciWriteOpReg(HC_STRUC*, UINT32, UINT32);
VOID	EhciClearOpReg(HC_STRUC*, UINT32, UINT32);
VOID	EhciSetOpReg(HC_STRUC*, UINT32, UINT32);
UINT32	EhciReadDebugReg(HC_STRUC*, UINT8, UINT32);
VOID*	EhciMemAlloc(HC_STRUC*, UINT16);
VOID	EhciMemFree(HC_STRUC*, VOID*, UINT16);
UINT8	EhciDmaMap(HC_STRUC*, UINT8, UINT8*, UINT32, UINT8**, VOID**);
UINT8	EhciDmaUnmap(HC_STRUC*, VOID*);
BOOLEAN EhciIsHalted(HC_STRUC*);
UINT16  EhciTranslateInterval(UINT8, UINT8);

UINT8   USBCheckPortChange (HC_STRUC*, UINT8, UINT8);
UINT8   USBLogError(UINT16);
UINT8	UsbGetDataToggle(DEV_INFO*,UINT8);
VOID	UsbUpdateDataToggle(DEV_INFO*, UINT8, UINT8);

VOID    USB_InitFrameList (HC_STRUC*, UINT32);
VOID    FixedDelay(UINTN);

VOID*   USB_MemAlloc (UINT16);
UINT8   USB_MemFree (VOID _FAR_ *, UINT16);
UINT8   USB_DisconnectDevice (HC_STRUC*, UINT8, UINT8);
DEV_INFO*   USB_GetDeviceInfoStruc(UINT8, DEV_INFO*, UINT8, HC_STRUC*);
UINT8   USB_StopDevice (HC_STRUC*,  UINT8, UINT8);
UINT8	USB_InstallCallBackFunction (CALLBACK_FUNC);
VOID	USBKeyRepeat(HC_STRUC*, UINT8);

#if USB_DEV_KBD
VOID    USBKBDPeriodicInterruptHandler(HC_STRUC*);
#endif


extern  USB_GLOBAL_DATA     *gUsbData;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EHCI_FillHCDEntries (HCD_HEADER *fpHCDHeader)
//
// Description: This function fills the host controller driver
//              routine pointers
//
// Parameters:  fpHCDHeader     Ptr to the host controller header structure
//
// Output:      Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_FillHCDEntries (HCD_HEADER *fpHCDHeader)
{
    fpHCDHeader->pfnHCDStart                = EHCI_Start;
    fpHCDHeader->pfnHCDStop                 = EHCI_Stop;
    fpHCDHeader->pfnHCDEnumeratePorts       = EHCI_EnumeratePorts;
    fpHCDHeader->pfnHCDDisableInterrupts    = EHCI_DisableInterrupts;
    fpHCDHeader->pfnHCDEnableInterrupts     = EHCI_EnableInterrupts;
    fpHCDHeader->pfnHCDProcessInterrupt     = EHCI_ProcessInterrupt;
    fpHCDHeader->pfnHCDGetRootHubStatus     = EHCI_GetRootHubStatus;
    fpHCDHeader->pfnHCDDisableRootHub       = EHCI_DisableRootHub;
    fpHCDHeader->pfnHCDEnableRootHub        = EHCI_EnableRootHub;
    fpHCDHeader->pfnHCDControlTransfer      = EHCI_ControlTransfer;
    fpHCDHeader->pfnHCDBulkTransfer         = EHCI_BulkTransfer;
    fpHCDHeader->pfnHCDInterruptTransfer    = EHCI_InterruptTransfer;
    fpHCDHeader->pfnHCDDeactivatePolling    = EHCI_DeactivatePolling;
    fpHCDHeader->pfnHCDActivatePolling      = EHCI_ActivatePolling;
    fpHCDHeader->pfnHCDDisableKeyRepeat     = EHCI_DisableKeyRepeat;
    fpHCDHeader->pfnHCDEnableKeyRepeat      = EHCI_EnableKeyRepeat;
    fpHCDHeader->pfnHCDEnableEndpoints      = USB_EnableEndpointsDummy;
    fpHCDHeader->pfnHCDInitDeviceData       = USB_InitDeviceDataDummy;
    fpHCDHeader->pfnHCDDeinitDeviceData     = USB_DeinitDeviceDataDummy;
	fpHCDHeader->pfnHCDResetRootHub         = EHCI_ResetRootHub;
	fpHCDHeader->pfnHCDClearEndpointState	= 0;	//(EIP54283+)
	fpHCDHeader->pfnHCDGlobalSuspend        = EHCI_GlobalSuspend;	//(EIP54018+)

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EhciIsolateDebugPort
//
// Description: This routine locates EHCI debug port and determines whether
//              or not the debug port is initialized and being used by other
//              agents. If so, the global flag will be set to instruct the
//              EHCI runtime routines about debug port presence and prevent
//              any unwanted reset/reconfiguration of this port.
//
// Parameters:  fpHCStruc   Ptr to the host controller structure
//
// Output:      fpHCStruc->DebugPort is updated if Debug Port is active on
//              this controller; otherwise it will remain 0.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EhciIsolateDebugPort(HC_STRUC *fpHCStruc)
{
    UINT32 HcSParams = fpHCStruc->dHCSParams;   // Host Controller Structural Parameters
    UINT8  DebugPortNo;
    UINT32 NextCap;
	UINT8  DebugPortBarIndex;
    UINT16 DebugPortOffset;

    //
    // Locate debug port by looking at the PCI capabilities
    //
    DebugPortNo = (UINT8)((HcSParams & (EHCI_DEBUG_N)) >> 20);

    //ASSERT(DebugPortNo); // No debug port implemented
    fpHCStruc->DebugPort = 0;
    if (DebugPortNo == 0) return;

    ASSERT(DebugPortNo <= (UINT8)(HcSParams & (EHCI_N_PORTS)));    // Invalid debug port number
    if (DebugPortNo > (UINT8)(HcSParams & (EHCI_N_PORTS))) return;

    //
    // Check whether device implements Capability list that starts at register 0x34
    //
    if (!(EhciReadPciReg(fpHCStruc, 4) & BIT20)) {
        //ASSERT(FALSE);  // Capabilities list is not implemented
        return;
    }

    //
    // Find the beginning of Debug Port registers block
    //
    for (NextCap = EhciReadPciReg(fpHCStruc, 0x34);
            (UINT8)NextCap > 0;
    )
    {
        NextCap = EhciReadPciReg(fpHCStruc, (UINT8)NextCap);
        if ((UINT8)NextCap == 0xA) break;   // Debug port capability found
        NextCap >>= 8;
    }
    if ((UINT8)NextCap == 0) {
        //ASSERT(FALSE);  // Debug capabilities not found
        return;
    }
	DebugPortBarIndex = (UINT8)((NextCap >> 29) - 1);
    DebugPortOffset = (UINT16)((NextCap >> 16) & 0x1FFF);
    ASSERT(DebugPortBarIndex >= 0 && DebugPortBarIndex <= 5); // Wrong BAR
    if (!(DebugPortBarIndex >= 0 && DebugPortBarIndex <= 5)) return;
    //
    // See whether Debug Port is acquired by other software
    //
	if (EhciReadDebugReg(fpHCStruc, DebugPortBarIndex, DebugPortOffset) & BIT28) {
        fpHCStruc->DebugPort = DebugPortNo;
        USB_DEBUG(DEBUG_LEVEL_3, "EHCI HC Debug Port #%d enabled.\n", DebugPortNo);
	}
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EHCI_Start
//
// Description: This API function is called to start a EHCI host controller.
//              The input to the routine is the pointer to the HC structure
//              that defines this host controller
//
// Parameters:  fpHCStruc   Ptr to the host controller structure
//
// Output:      Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_Start (HC_STRUC* fpHCStruc)
{
    UINT32  dTemp;
#if EHCI_ASYNC_BELL_SUPPORT
    EHCI_QH *fpQHAsyncXfer;
#endif
	EHCI_QH     *fpQHRepeat = NULL;
	EHCI_QTD    *fpqTDRepeat = NULL;
	UINT32	i;							//(EIP55960+)
	BOOLEAN	SetPortPower = FALSE;
    UINT16  PortReg;

/*
USB_DEBUG(DEBUG_LEVEL_3, "Enabling MEM/BM for EHCI HC %02X\n", fpHCStruc->wBusDevFuncNum);

    //
    // Enable IO access and Bus Mastering
    //
    WordWritePCIConfig((UINT16)fpHCStruc->wBusDevFuncNum, 4, BIT1 + BIT2);
*/
    //
    // Get memory base address of the HC and store it in the HCStruc
    //
    fpHCStruc->BaseAddress = EhciReadPciReg(fpHCStruc, USB_MEM_BASE_ADDRESS) & 0xFFFFFFF0;

    USB_DEBUG(DEBUG_LEVEL_3, "EHCI HC Mem Addr: %X\n", fpHCStruc->BaseAddress);

    //
    // Get the number of ports supported by the host controller (Offset 4)
    // and store it in HCStruc
    //
    fpHCStruc->dHCSParams = EhciReadHcMem(fpHCStruc, EHCI_HCSPARAMS);
    fpHCStruc->bNumPorts = (UINT8)(fpHCStruc->dHCSParams & EHCI_N_PORTS);
    USB_DEBUG(DEBUG_LEVEL_3, "EHCI HC Number of ports: %d\n", fpHCStruc->bNumPorts);

	EhciIsolateDebugPort(fpHCStruc);

    //
    // Read the Capability Registers Length to find the Offset address for the
    // beginning of the operational registers
    //
    fpHCStruc->bOpRegOffset = (UINT8)EhciReadHcMem(fpHCStruc, EHCI_VERCAPLENGTH);
    USB_DEBUG(DEBUG_LEVEL_3, "EHCI HC Operational Registers Offset: %d\n", fpHCStruc->bOpRegOffset);

    //
    // Read and store the HCCPARAMS value
    //
    fpHCStruc->dHCCParams = EhciReadHcMem(fpHCStruc, EHCI_HCCPARAMS);
    //USB_DEBUG(DEBUG_LEVEL_3, "EHCI HC HCPARAMS: %x\n", gUsbData->dHCCParams);

    //
    // Get PCI register offset for the legacy support in EHCI controller
    // and store it in HC_STRUC
    //
    fpHCStruc->bExtCapPtr = EHCIGetLegacySupportOffset(
                        fpHCStruc,
                        fpHCStruc->wBusDevFuncNum);

#if EHCI_64BIT_DATA_STRUCTURE == 0
    //
    // 64bit data structures are not enabled. So check whether this host controller
    // needs 64bit data structure or not.
    //
    if (fpHCStruc->dHCCParams & EHCI_64BIT_CAP)
    {
        //
        // Engineer has to enable the 64bit capability. Post an error message
        //
        USBLogError(ERRUSB_EHCI_64BIT_DATA_STRUC);
        ASSERT(FALSE);

        //
        // Connect all ports to the classic host controller
        //
        EhciClearOpReg(fpHCStruc, EHCI_CONFIGFLAG, BIT0);
        return  USB_ERROR;
    }
#endif

#if	HIDE_USB_HISPEED_SUPPORT_SETUP_QUESTION == 0
	if ((gUsbData->UsbHiSpeedSupport == 0) && ((fpHCStruc->dHCSParams & EHCI_N_CC) != 0)) {
		EhciClearOpReg(fpHCStruc, EHCI_CONFIGFLAG, BIT0);
		return  USB_ERROR;
	}
#endif

//----------------------------------------------------------------------------
// Note: after this point any access to the operational registers is through
// the macros EHCI_DWORD_READ_MEM and EHCI_DWORD_WRITE_MEM; access to the
// capability registers is through the macro USBPORT_DWORD_READ_MEM and
// there is no macro to write to the registers
//----------------------------------------------------------------------------
										//(EIP55960)>
	if ((EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_HCHALTED) == 0) {
	    // Turn HC off and wait for the Halted bit to get set
	    EhciClearOpReg(fpHCStruc, EHCI_USBCMD, EHCI_RUNSTOP);

		// The Host Controller must halt within 16 micro-frames after 
		// software clears the Run bit. 
		for (i = 0; i < 16; i++) {
			if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_HCHALTED) {
				break;
			}
            FixedDelay(125);	  // 125 us delay
		}
		//while ((DwordReadMem(dMemAddr, EHCI_USBSTS) & EHCI_HCHALTED) == 0) {};
	}
										//<(EIP55960)
// /* EIP#23479
    //
    // Reset the host controller (HC must be halted)
    //
    if (EHCIResetHC(fpHCStruc) == USB_ERROR)
    {
        return  USB_ERROR;  // HC reset error, error log is updated
    }
//*/
    //
    // Get the frame list size from the EHCI command register
    //
    dTemp = EhciReadOpReg(fpHCStruc, EHCI_USBCMD);
    dTemp = (dTemp & (BIT2 + BIT3)) >> 2;

    //
    // Calculate number of elements in the asynchronous list
    // and store the value in the HCStruc
    //
    switch (dTemp)
    {
        case 0: fpHCStruc->wAsyncListSize   = 1024;
                break;
        case 1: fpHCStruc->wAsyncListSize   = 512;
                break;
        case 2: fpHCStruc->wAsyncListSize   = 256;
                break;
        case 3: return  USB_ERROR;

    }

    USB_DEBUG(DEBUG_LEVEL_3, "EHCI AsyncListSize: %d\n", fpHCStruc->wAsyncListSize);

    //
    // Set the max bulk data size
    //
    fpHCStruc->dMaxBulkDataSize = MAX_EHCI_DATA_SIZE;

    //
    // Initialize the frame list pointers
    //
    USB_InitFrameList (fpHCStruc, EHCI_TERMINATE);

    //
    // Write the base address of the Periodic Frame List to the PERIODIC BASE
    // register
    //
    EhciWriteOpReg(fpHCStruc, EHCI_PERIODICLISTBASE, (UINT32)(UINTN)fpHCStruc->fpFrameList);

    //
    // Initialize the periodic schedule
    //
    EHCIInitializePeriodicSchedule(fpHCStruc, (UINT32)fpHCStruc->BaseAddress);

#if EHCI_ASYNC_BELL_SUPPORT
    //
    // Allocate and initialize an queue head for Async transfer
    // Set the QHDummy as Async list head
    //
    fpQHAsyncXfer = EhciMemAlloc (fpHCStruc, GET_MEM_BLK_COUNT_STRUC(EHCI_QH));

    if (!fpQHAsyncXfer) {
        return  USB_ERROR;
    }

    gUsbData->fpQHAsyncXfer        = fpQHAsyncXfer;

    fpQHAsyncXfer->dEndPntCap       = QH_ONE_XFER;
    fpQHAsyncXfer->fpFirstqTD       = 0;
    fpQHAsyncXfer->dAltNextqTDPtr   = EHCI_TERMINATE;
    fpQHAsyncXfer->dNextqTDPtr      = EHCI_TERMINATE;

    //
    // Assume as a high speed device
    //
    dTemp = QH_HIGH_SPEED;  // 10b - High speed

    //
    // Use data toggle from qTD and this QH is the head of the queue
    //
    dTemp |= (QH_USE_QTD_DT | QH_HEAD_OF_LIST | DUMMY_DEVICE_ADDR); // Endpoint is 0

    //
    // dTemp[6:0] = Dev. Addr, dTemp[7] = I bit(0) & dTemp[11:8] = Endpoint (0)
    //
    fpQHAsyncXfer->dEndPntCharac    = dTemp;

    //
    // Set the ASYNCLISTADDR register to point to the QHDummy
    //
    EhciWriteOpReg(fpHCStruc, EHCI_ASYNCLISTADDR, (UINT32)(UINTN)fpQHAsyncXfer);

    //
    // Set next QH pointer to itself (circular link)
    //
    fpQHAsyncXfer->dLinkPointer = (UINT32)(UINTN)fpQHAsyncXfer | EHCI_QUEUE_HEAD;
    fpQHAsyncXfer->bActive = TRUE;
#endif  //  EHCI_ASYNC_BELL_SUPPORT

#if USB_RUNTIME_DRIVER_IN_SMM
	// Check whether no companion host controllers
	if ((fpHCStruc->dHCSParams & EHCI_N_CC) == 0) {
		//
		// Allocate a QH/qTD for QHRepeat/qTDRepeat
		//
		fpQHRepeat = EhciMemAlloc(fpHCStruc,
						GET_MEM_BLK_COUNT(sizeof(EHCI_QH)+sizeof(EHCI_QTD)));

		if (!fpQHRepeat) {
			return	USB_ERROR;	// Memory allocation error
		}
		fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpQHRepeat = fpQHRepeat;
		fpqTDRepeat = (EHCI_QTD*)((UINT32)fpQHRepeat + sizeof(EHCI_QH));
		fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpqTDRepeat = fpqTDRepeat;
//
// Setup QHRepeat and qTDRepeat.  It will run a interrupt transaction to a
// nonexistant dummy device.  This will have the effect of generating
// a periodic interrupt used to generate keyboard repeat.  This QH/qTD
// is normally inactive,  and is only activated when a key is pressed.
//
		//
		// Set the first qTD pointer
		//
		fpQHRepeat->fpFirstqTD = fpqTDRepeat;

		//fpQHRepeat->fpDevInfoPtr = (UINT8*)fpDevInfo;
		fpQHRepeat->dNextqTDPtr = (UINT32)fpqTDRepeat;

		//
		// Intialize the queue head
		//
		fpQHRepeat->dAltNextqTDPtr = EHCI_TERMINATE;
		fpQHRepeat->dLinkPointer = EHCI_TERMINATE;

		//
		// Set max packet size, address, endpoint and high speed
		// Update the AH's endpoint characteristcs field with the data formed
		//
		fpQHRepeat->dEndPntCharac |= ((0x40 << 16) | DUMMY_DEVICE_ADDR |
									QH_HIGH_SPEED);

		//
		// Set a bit in interrupt mask
		//
		fpQHRepeat->dEndPntCap	= (BIT0 | QH_ONE_XFER);
        fpQHRepeat->Interval = 8;

//
// Fill the repeat qTD with relevant information
// The token field will be set so
//	 Direction PID = QTD_IN_TOKEN,
//	 Size = size of the data,
//	 Data Toggle = QTD_DATA0_TOGGLE,
//	 Error Count = QTD_NO_ERRORS,
//	 Status code = QTD_ACTIVE
// The buffer pointers field will point to the fpBuffer buffer
//	 which was before initialized to contain a DeviceRequest struc.
// The dNextqTDPtr field will point to the qTDControlSetup
// The dAltNextqTDPtr field will be set to EHCI_TERMINATE
//
		fpQHRepeat->dTokenReload = ((UINT32)8 << 16) | QTD_IN_TOKEN | QTD_ONE_ERROR;
		fpqTDRepeat->dToken = ((UINT32)8 << 16) | QTD_IN_TOKEN | QTD_ONE_ERROR;

		EHCISetQTDBufferPointers(fpqTDRepeat,
			&fpQHRepeat->aDataBuffer[0], 8);

		//
		// Update next & alternate next qTD pointers
		//
		fpqTDRepeat->dNextqTDPtr = EHCI_TERMINATE;
		fpqTDRepeat->dAltNextqTDPtr = EHCI_TERMINATE;

		//
		// Schedule the QHRepeat to 8ms schedule
		//
        EhciAddPeriodicQh(fpHCStruc,fpQHRepeat);

		fpQHRepeat->bCallBackIndex = USB_InstallCallBackFunction(EhciRepeatTDCallback);
		fpQHRepeat->bActive = FALSE;

		USBKeyRepeat(fpHCStruc, 0);
	}
#endif

    //
    // Clear status register - all R/WC bits
    //
    EhciWriteOpReg(fpHCStruc, EHCI_USBSTS,
            EHCI_USB_INTERRUPT |        // Interrupt
            EHCI_USB_ERROR_INTERRUPT |  // Error interrupt
            EHCI_PORT_CHANGE_DETECT |   // Port Change Detect
            EHCI_FRAME_LIST_ROLLOVER |  // Frame List Rollover
            EHCI_HOST_SYSTEM_ERROR |    // Host System Error
            EHCI_INT_ASYNC_ADVANCE);    // Interrupt on Async Advance
    //
    // Program the HC BIOS owned bit and return the legacy support register offset
    //
    if (fpHCStruc->bExtCapPtr) {
		EHCIProgramLegacyRegisters(fpHCStruc, 1);  // Set HC BIOS owned semaphore

        //
        // Enable USB SMI, SMI on port change and SMI on ownership change
        //
        dTemp = EHCI_SMI + EHCI_PORT_CHANGE_SMI + EHCI_OWNERSHIP_CHANGE_SMI;

		EhciWritePciReg(fpHCStruc, fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG, dTemp);
    }

    //
    // Turn HC on
    //
    EhciSetOpReg(fpHCStruc, EHCI_USBCMD, \
        (EHCI_RUNSTOP | EHCI_PER_SCHED_ENABLE));

    // Wait for halt bit get cleared
    for (i = 0; i < 20; i++) {
        if (!(EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_HCHALTED)) {
            break;
        }
        FixedDelay(100);	  // 100 us delay
    }

    //
    // If the port has the power switch then enable the port. Otherwise 
    // Power for the port is already present. So don't need to enable the power.
    // ( Refer EHCI Spec 2.2.3 HCSPARAMS Structural Parameters Bit 4 (PPC) )
    if (fpHCStruc->dHCSParams & EHCI_PPC) {
        //
        // Enable port power
        //
        for (i = 1, PortReg = EHCI_PORTSC; i <= fpHCStruc->bNumPorts; i++, PortReg += 4) {
            //
            // Skip enabling DebugPort
            //
            if (fpHCStruc->DebugPort && fpHCStruc->DebugPort == i) continue;
    
            if (EhciReadOpReg(fpHCStruc, PortReg) & EHCI_PORTPOWER) {
                continue;
            }
            
            EhciSetOpReg(fpHCStruc, PortReg, EHCI_PORTPOWER);
            SetPortPower = TRUE;
        }
        //
        // Delay till the ports power is stabilised
        //
        if (SetPortPower) {
            FixedDelay(20 * 1000);        // 20 msec delay
        }
    }

    // Set HC flag as running
    fpHCStruc->dHCFlag |= HC_STATE_RUNNING;

    //
    // Disconnect all ports from companion HC (if any) and route them to EHCI
    //
    EhciSetOpReg(fpHCStruc, EHCI_CONFIGFLAG, BIT0);		//(EIP59663-) //(EIP80307+)

    if (fpHCStruc->dHCFlag & HC_STATE_CONTROLLER_WITH_RMH) {
        // Wait for port change detect bit set
        for (i = 0; i < 50; i++) {
            if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_PORT_CHANGE_DETECT) {
                break;
            }
            FixedDelay(100);    // 100 us delay
        }
    } else {
        FixedDelay(100);    // 100 us delay
    }

#if USB_RUNTIME_DRIVER_IN_SMM
    //
    // Register the USB HW SMI handler
    //
    if (!(fpHCStruc->dHCFlag & HC_STATE_EXTERNAL)) {
        UsbInstallHwSmiHandler(fpHCStruc);
    } else {
        USBSB_InstallUsbIntTimerHandler();
    }
#endif

    return  USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIGetLegacySupportOffset
//
// Description: This function returns the PCI register offset for the legacy
//              support in EHCI controller
//
// Input:       fpHCStruc   - HCStruc pointer
//              wPciAddr    - PCI address of the EHCI host controller
//
// Output:      0   If the feature is not present
//              <>0 Legacy support capability offset
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIGetLegacySupportOffset(
    HC_STRUC* fpHCStruc,
    UINT16 wPciAddr)
{
    UINT8   bData = 0;
    UINT32  dData = 0;

#if USB_RUNTIME_DRIVER_IN_SMM

    if (fpHCStruc->dHCFlag & HC_STATE_EXTERNAL) {
        return 0;
    }

    //
    // Get EHCI Extended Capabilities Pointer
    //
    bData = (UINT8)((fpHCStruc->dHCCParams >> 8) & 0xFF);

    if (!bData)
    {
        return 0;   // No extended capabilities are implemented.
    }

    dData = EhciReadPciReg(fpHCStruc, bData);
    if (!((UINT8)dData & 1)) {
        return 0;
    }
#endif
    return bData;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIProgramLegacyRegisters
//
// Description: This function programs the EHCI legacy registers as per the
//              input. Also this routine returns the PCI register offset
//              for the legacy support in EHCI controller
//
// Input:   fpHCStruc   HCStruc pointer
//          bSetReset:
//              0   Reset HC BIOS owned bit
//              1   Set HC BIOS owned bit
//
// Output:  0   If the feature is not present
//          <>0 Legacy support capability offset
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIProgramLegacyRegisters(
    HC_STRUC* fpHCStruc,
    UINT8 bSetReset)
{
    UINT32  dTemp;

    //
    // Check whether EHCI extended capabilities  pointer is present
    //
    if (!fpHCStruc->bExtCapPtr)
    {
        return  0;  // No extended capabilities are implemented.
    }

    //
    // Program 'HC BIOS owned semaphore bit'
    //
    dTemp = EhciReadPciReg(fpHCStruc, fpHCStruc->bExtCapPtr);
    dTemp &= ~BIT16;

    if (bSetReset)
    {
        dTemp |= BIT16;
    }

                                        // (USB_S4_RESUME_ISSUE, EIP#20084)>
    if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)
     dTemp &= ~BIT24;
                                        // <(USB_S4_RESUME_ISSUE, EIP#20084)

    EhciWritePciReg(fpHCStruc, fpHCStruc->bExtCapPtr, dTemp);

    //
    // Reset all enable bits and clear the status
    //
    dTemp = 0xE0000000 | EHCI_OWNERSHIP_CHANGE_SMI;

    EhciWritePciReg(fpHCStruc,
        fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG,
        dTemp);

    return  fpHCStruc->bExtCapPtr;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ClearEECPstatus
//
// Description: This procedure clear EHCI legacy support status.
//
// Input:   fpHCStruc   - HCStruc pointer
//          wSTatus     - Legacy status to clear
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ClearEECPstatus(
    HC_STRUC* fpHCStruc,
    UINT16 wStatus)
{
    UINT32  dTemp;

    if (!fpHCStruc->bExtCapPtr)
    {
        return; // No extended capabilities are implemented.
    }

    //
    // Read control and status register
    //
    dTemp = EhciReadPciReg(fpHCStruc,
                fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG);

    //
    // Keep enable bits and set clear status bit
    //
    dTemp = (dTemp & 0xFFFF) | ((UINT32)wStatus << 16);
    EhciWritePciReg(
        fpHCStruc,
        fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG,
        dTemp);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GetEhciUSBLEGSUP
//
// Description: This routine return USBLEGSUP register content. It could be
//               used to check EHCI semaphore owened by BIOS or OS.
//
// Input:   fpHCStruc   HCStruc pointer
//
// Output:  UINT32  Legacy support extended capability register content.
//                  -1 if no extended capabilities are implemented.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
GetEhciUSBLEGSUP(HC_STRUC* fpHCStruc)
{
    UINT32  dTemp;

    //
    // Check whether EHCI extended capabilities  pointer is present
    //
    if (!fpHCStruc->bExtCapPtr)
    {
        return 0xFFFFFFFF;  // No extended capabilities are implemented.
    }

    //
    // Read Legacy support register
    //
    dTemp = EhciReadPciReg(
                fpHCStruc,
                fpHCStruc->bExtCapPtr);

    return dTemp;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_EnumeratePorts
//
// Description: This function enumerates the HC ports for devices
//
// Input:       fpHCStruc   Host controller's HCStruc structure
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_EnumeratePorts(HC_STRUC* fpHCStruc)
{
    UINT16  wPortCtl    = EHCI_PORTSC;  // Port Status and Control Register (44h)
    UINT8   bHCNumber   = (UINT8)(fpHCStruc->bHCNumber | BIT7);
    UINT8   bPortNum;

    //
    // Enable port power so that new devices can be detected.
    //
    // Check whether enumeration flag is set by us or by somebody else by checking
    // local enum flag.
    //
    if (gUsbData->bEnumFlag == FALSE)
    {
        gUsbData->bIgnoreConnectStsChng    = TRUE;
        gUsbData->bEnumFlag                = TRUE;
                                        //(EIP122174+)>
        do {
            //
            // Clear the EHCI_PCD bit of the interrupt status register EHCI_USBSTS
            //
            EhciWriteOpReg(fpHCStruc, EHCI_USBSTS, EHCI_PORT_CHANGE_DETECT);

            //
            // Check the root hub ports to see if a device is connected.  If so, then
            // call USBCheckPortChange to handle the attachment of a new device.
            //
            for ( bPortNum = 1; bPortNum <= fpHCStruc->bNumPorts; bPortNum++) {
                //
                // Skip DebugPort enumeration
                //
                if (fpHCStruc->DebugPort && fpHCStruc->DebugPort==bPortNum) continue;

                //
                // Process device connect/disconnect
                //
                USBCheckPortChange(fpHCStruc, bHCNumber, bPortNum);
            }
        } while ((EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_PORT_CHANGE_DETECT));
                                        //<(EIP122174+)
        gUsbData->bIgnoreConnectStsChng = FALSE;

        //
        // Reset enumeration flag and enable hub enumeration
        //
        gUsbData->bEnumFlag = FALSE;
    }

    //
    // Enable appropriate interrupts
    //
    EhciWriteOpReg(fpHCStruc, EHCI_USBINTR, EHCI_USBINT_EN | EHCI_PCDINT_EN);
    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCICheckHCStatus
//
// Description: This function checks whether the host controller is still
//              under BIOS
//
// Input:       fpHCStruc   - Host controller's HCStruc structure
//
// Output:      USB_SUCCESS - If the control is with the BIOS
//              USB_ERROR   - If the control is not with the BIOS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCICheckHCStatus(HC_STRUC* fpHCStruc)
{
    //
    // Check whether the controller is still under BIOS control
    // Read the base address of the Periodic Frame List to the PERIODIC BASE
    // register and compare with stored value
    //
    if ((UINTN)fpHCStruc->fpFrameList == 
		(EhciReadOpReg(fpHCStruc, EHCI_PERIODICLISTBASE) & 0xFFFFF000))
    {
        return  USB_SUCCESS;    // Control is with BIOS
    }
    return  USB_ERROR;  // HC is controlled by someone else
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIStop
//
// Description: This function stops the EHCI controller.
//
// Input:       fpHCStruc   Host controller's HCStruc structure
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_Stop (HC_STRUC* fpHCStruc)
{
    UINT8   bPortNum;					//(EIP26685+)
    UINT8   Status;
	UINT8	i;							//(EIP55960+)

    //
    // Check whether the control is with BIOS or not
    //
    if (EHCICheckHCStatus(fpHCStruc) == USB_SUCCESS)    // Controlled by BIOS
    {
#if PCH_EHCI_OWNERSHIP_CHANGE_MECHANISM
		if (fpHCStruc->dHCFlag & HC_STATE_OWNERSHIP_CHANGE_IN_PROGRESS) {
			UINT16  PortReg;
			UINT32	PortSts;
			UINT32	Data32;

			// Disconnect all the devices connected to its ports
			for (bPortNum = 1; bPortNum <= fpHCStruc->bNumPorts; bPortNum++) {
				USB_StopDevice(fpHCStruc, (UINT8)(fpHCStruc->bHCNumber | BIT7), bPortNum);
			}
	
			// Stop the asynchronous schedule
			EHCIStopAsyncSchedule(fpHCStruc);

			// Stop the periodic schedule
			EHCIStopPeriodicSchedule(fpHCStruc);
	
			for (bPortNum = 1; bPortNum <= fpHCStruc->bNumPorts; bPortNum++) {
				PortReg = (UINT16)((bPortNum-1)*4 + EHCI_PORTSC);
				PortSts = EhciReadOpReg(fpHCStruc, PortReg);
	
				if (!(PortSts & EHCI_PORTENABLE)) {
					continue;
				}
				EhciWriteOpReg(fpHCStruc, PortReg, PortSts | EHCI_SUSPEND);
			}
			FixedDelay(250);      // 250 us delay
	
			// Stop the host controller (Reset bit 0)
			EhciClearOpReg(fpHCStruc, EHCI_USBCMD, EHCI_RUNSTOP);
	
			// The Host Controller must halt within 16 micro-frames after 
			// software clears the Run bit. 
			for (i = 0; i < 16; i++) {
				if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_HCHALTED) {
					break;
				}
                FixedDelay(125);	  // 125 us delay
			}
	
			// Clear the SMI enable bits
			if (fpHCStruc->bExtCapPtr) {
				Data32 = EhciReadPciReg(fpHCStruc, 
							fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG);
	
				EhciWritePciReg(fpHCStruc,
					fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG, Data32 & ~(0x3F));
			}
	
			// Clear the USBSTS register bits
			EhciWriteOpReg(fpHCStruc, EHCI_USBSTS, EhciReadOpReg(fpHCStruc, EHCI_USBSTS));
	
			// Clear the Configure Flag bit
			EhciClearOpReg(fpHCStruc, EHCI_CONFIGFLAG, BIT0);
		} else 
#endif
		{
											//(EIP26685+)>
	        //
	        // Disconnect all the devices connected to its ports
	        //
	        for (bPortNum = 1; bPortNum <= fpHCStruc->bNumPorts; bPortNum++)
	        {
	            USB_DisconnectDevice(fpHCStruc,
	                    (UINT8)(fpHCStruc->bHCNumber | BIT7), bPortNum);
	        }
											//<(EIP26685+)

	        if (fpHCStruc->DebugPort == 0) {
	            //
	            // Stop the host controller (Reset bit 0)
	            //
	            EhciClearOpReg(fpHCStruc, EHCI_USBCMD, EHCI_RUNSTOP);
												//(EIP55960)>
				// The Host Controller must halt within 16 micro-frames after 
				// software clears the Run bit. 
				for (i = 0; i < 16; i++) {
	            	if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_HCHALTED) {
						break;
	            	}
                    FixedDelay(125);      // 125 us delay
				}
												//<(EIP55960)
	            //
	            // Reset the host controller
	            //
//  EIP#23479          EHCIResetHC(fpHCStruc); // ERROR CONDITION RETURNED IS NOT TAKEN CARE
	            Status = EHCIResetHC(fpHCStruc);
	            ASSERT(Status == USB_SUCCESS);
	        }
		}
        //
        // Program the HC BIOS owned bit and return the legacy
        // support register offset
        //
        EHCIProgramLegacyRegisters(fpHCStruc, 0);   // Reset HC BIOS owned semaphore
                                                    // ERROR CONDITION IS NOT HANDLED

        //
        // Clear the frame list pointers
        //
        USB_InitFrameList (fpHCStruc, EHCI_TERMINATE);

        //
        // Disable TD schedule and free the data structures
        //
		if (fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpQHRepeat) {
			EhciMemFree(fpHCStruc, fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpQHRepeat,
							GET_MEM_BLK_COUNT(sizeof(EHCI_QH) + sizeof(EHCI_QTD) ));
		}

        //
        // Free the scheduling QHs
        //
        EhciMemFree(fpHCStruc, fpHCStruc->stDescPtrs.fpEHCIDescPtrs->PeriodicQh,        
                        GET_MEM_BLK_COUNT(1 * sizeof(EHCI_QH)));

        //
        // Free descriptor structure
        //
        EhciMemFree(fpHCStruc, fpHCStruc->stDescPtrs.fpEHCIDescPtrs,
                        GET_MEM_BLK_COUNT_STRUC(EHCI_DESC_PTRS));

#if EHCI_ASYNC_BELL_SUPPORT
        //
        // Free the Async transfer QH
        //
        EhciMemFree(fpHCStruc, gUsbData->fpQHAsyncXfer, GET_MEM_BLK_COUNT_STRUC(EHCI_QH));
#endif

		USBKeyRepeat(fpHCStruc, 3);
    }
    else    // not controlled by BIOS
    {
        //
        // Program the HC BIOS owned bit and return the legacy
        // support register offset
        //
        EHCIProgramLegacyRegisters(fpHCStruc, 0);   // Reset HC BIOS owned semaphore
    }

    //
    // Set the HC state to stopped
    //
    fpHCStruc->dHCFlag  &= ~(HC_STATE_RUNNING);

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_DisableInterrupts
//
// Description: This function disables the HC interrupts
//
// Input:   fpHCStruc   Pointer to the HCStruc structure
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_DisableInterrupts (HC_STRUC* fpHCStruc)
{
    //
    // Disable interrupt generation
    //
    EhciClearOpReg(fpHCStruc, EHCI_USBINTR, EHCI_USBINT_EN | EHCI_PCDINT_EN);

    //
    // Stop periodic and asynchoronous schedule
    //
    EHCIStopAsyncSchedule(fpHCStruc);
    EHCIStopPeriodicSchedule(fpHCStruc);

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_EnableInterrupts
//
// Description: This function enables the HC interrupts
//
// Input:   fpHCStruc   Pointer to the HCStruc structure
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_EnableInterrupts (HC_STRUC* fpHCStruc)
{
    //
    // Start periodic and asynchoronous schedule
    //
    EHCIStartAsyncSchedule(fpHCStruc);
    EHCIStartPeriodicSchedule(fpHCStruc);

    //
    // Enable interrupt generation
    //
    EhciSetOpReg(fpHCStruc, EHCI_USBINTR, EHCI_USBINT_EN | EHCI_PCDINT_EN);

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   ProcessRootHubChanges
//
// Description: Root hub change processing code
//
// Parameters:  fpHCStruc   Pointer to the HCStruc structure
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
ProcessRootHubChanges(
    HC_STRUC* fpHCStruc
)
{
    UINT8   bPortNum;

    //
    // Check bEnumFlag before enumerating devices behind root hub
    //
    if ((gUsbData->bEnumFlag) == TRUE) {
        return USB_ERROR;
    }

    //
    // Clear the port change bit of the interrupt status register EHCI_USBSTS
    //
    EhciWriteOpReg(fpHCStruc, EHCI_USBSTS, EHCI_PORT_CHANGE_DETECT);

    //
    // Check all the ports on the root hub for any change in connect status.
    // If the connect status has been changed on either or both of these ports,
    // then call the  routine UsbHubPortChange for each changed port.
    //
    // Set enumeration flag and avoid hub port enumeration
    //
    gUsbData->bEnumFlag    = TRUE;

    for (bPortNum = 1; bPortNum <= fpHCStruc->bNumPorts; bPortNum++) {
        if (fpHCStruc->DebugPort && fpHCStruc->DebugPort==bPortNum) continue;
        //
        // Process device connect/disconnect
        // Note: port connect status is cleared while processing
        // connect/disconnect (EHCIGetRootHubStatus)
        //
        USBCheckPortChange(fpHCStruc, (UINT8)(fpHCStruc->bHCNumber | BIT7), bPortNum);
    }

    //
    // Reset enumeration flag and enable hub enumeration
    //
    gUsbData->bEnumFlag    = FALSE;
    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:    EHCI_ProcessInterrupt
//
// Description: This function is called when the USB interrupt bit is
//              set. This function will parse through the TDs and QHs to
//              find out completed TDs and call their respective call
//              back functions
//
// Parameters:  fpHCStruc   Pointer to the HCStruc structure
//
// Output:      USB_ERROR - Need more Interrupt processing
//              USB_SUCCESS - No interrupts pending
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_ProcessInterrupt(HC_STRUC* fpHCStruc)
{
    UINT32  dSts, dTmp;
    UINT16  wStatus;
                                        //(EIP71067-)>
//#if (EHCI_ASYNC_BELL_SUPPORT==0)
//    EHCI_QH *fpQH;
//#endif
                                        //<(EIP71067-)

    //
    // If EHCI extended capabilities  pointer is present,
    // then service OwnerShipChange SMI
    //
    if (fpHCStruc->bExtCapPtr) {
        //
        // Read control and status register
        //
        dTmp = EhciReadPciReg(
                fpHCStruc,
                fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG);
        wStatus = (UINT16)dTmp;
        wStatus &= (UINT16)(dTmp >> 16);   // "And" enable and status bits
        if (wStatus & EHCI_OWNERSHIP_CHANGE_SMI_STS) {
            ClearEECPstatus(fpHCStruc, wStatus);
            ProcessOwnerShipChangeSMI(fpHCStruc);
            return USB_SUCCESS; // Break from Interrupt process loop
        }
    }

    //
    // Check whether the controller is still under BIOS control
    // Read the base address of the Periodic Frame List to the PERIODIC BASE
    // register and compare with stored value
    //
    if (EHCICheckHCStatus(fpHCStruc) == USB_ERROR) {
        //
        // Control is not with us anymore, we should disable SMI generation
        // and come out.
        //
        if (fpHCStruc->bExtCapPtr) {
            //
            // Read control and status register
            //
            dTmp = EhciReadPciReg(
                        fpHCStruc,
                        fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG);
        
            //
            // Leave only Ownership SMI active.
            //
            dTmp &= 0xE0002000; 
            EhciWritePciReg(
                fpHCStruc,
                fpHCStruc->bExtCapPtr + EHCI_LEGACY_CTRL_STS_REG,
                dTmp);
        }
        return  USB_SUCCESS;
    }

	if (!(fpHCStruc->dHCFlag & HC_STATE_RUNNING)) {
		return USB_SUCCESS;
	}

    while(TRUE){
        //
        // Get the interrupt status
        //
        dSts = EhciReadOpReg(fpHCStruc, EHCI_USBSTS);

        //USB_DEBUG(DEBUG_LEVEL_3, "-->> %x <<--\n", dSts);

		if (dSts & EHCI_HOST_SYSTEM_ERROR) {
			gUsbData->dUSBStateFlag  &= (~USB_FLAG_ENABLE_BEEP_MESSAGE);
			EHCI_Start(fpHCStruc);
			EHCI_EnumeratePorts(fpHCStruc);
			gUsbData->dUSBStateFlag |= USB_FLAG_ENABLE_BEEP_MESSAGE;
			continue;
		}

        if (dSts & EHCI_HCHALTED) {
			// Clear the USBSTS register bits
			EhciWriteOpReg(fpHCStruc, EHCI_USBSTS, EhciReadOpReg(fpHCStruc, EHCI_USBSTS));
            break;
        }
        
        //
        // Check for transaction complete
        //
        if (dSts & EHCI_USB_INTERRUPT) {

            //
            // Clear the interrupt status
            //
            EhciWriteOpReg(fpHCStruc, EHCI_USBSTS, EHCI_USB_INTERRUPT);

            //Section 4.4 Schedule traversal rules.
            //if the periodic schedule is enabled (see Section 4.6) then the host controller must 
            //execute from the periodic schedule before executing from the asynchronous schedule. 
            //It will only execute from the asynchronous schedule after it encounters the end of
            //the periodic schedule.

            //
            // Check and process periodic schedule
            //
            if (dSts & EHCI_PER_SCHED_STATUS) {
                //
                // Check the command register for Async status
                //
                dTmp = EhciReadOpReg(fpHCStruc, EHCI_USBCMD);

                if (dTmp & EHCI_PER_SCHED_ENABLE) {
                    EHCIProcessPeriodicList(fpHCStruc);
                }
            }

            //
            // Check for Asynchronous schedule completion
            //
/*                                        //(EIP71067-)>
            if (dSts & EHCI_ASYNC_SCHED_STATUS) {
                dTmp = DwordReadMem(dMemAddr, EHCI_USBCMD);
                if (dTmp & EHCI_ASYNC_SCHED_ENABLE) {
                    //
                    // Check and process Async. QH
                    //
#if EHCI_ASYNC_BELL_SUPPORT
                    EHCIProcessQH(fpHCStruc, fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpQHControl);
                    EHCIProcessQH(fpHCStruc, fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpQHBulk);
#else
                    //
                    // Get the Async list address
                    //
                    fpQH = (EHCI_QH*)(UINTN)DwordReadMem(dMemAddr, EHCI_ASYNCLISTADDR);
                    if (EHCIProcessQH(fpHCStruc, fpQH) == USB_ERROR) {
                        //continue;
                        //return    USB_SUCCESS;
                    } else {
                        //
                        // Async list processed; stop the Async transfer
                        //
                        EHCIStopAsyncSchedule(fpHCStruc);
                    }
#endif
                }
            }
*/                                      //<(EIP71067-)
            continue;
        }

        //
        // Check PORT_CHANGE_DETECT bit
        //
        if ((dSts & EHCI_PORT_CHANGE_DETECT )) {
            if(ProcessRootHubChanges(fpHCStruc) == USB_SUCCESS) {
                continue;
            }
        }
        break;  // No more statuses to process
    }
    return  USB_SUCCESS;    // Set as interrupt processed
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProcessOwnerShipChangeSMI
//
// Description: This procedure process EHCI OwnerShipChange SMI.
//
// Input:   fpHCStruc   HCStruc pointer
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ProcessOwnerShipChangeSMI(HC_STRUC* fpHCStruc)
{
    UINT32 dTemp = GetEhciUSBLEGSUP(fpHCStruc);
	fpHCStruc->dHCFlag |= HC_STATE_OWNERSHIP_CHANGE_IN_PROGRESS;
    if (dTemp & EHCI_HC_OS) {
        gUsbData->dUSBStateFlag  &= (~USB_FLAG_ENABLE_BEEP_MESSAGE);
        ProcessSmiChangeToEHCD(fpHCStruc);
    }
    else {
        gUsbData->dUSBStateFlag |= USB_FLAG_ENABLE_BEEP_MESSAGE;
        ProcessSmiChangeToBIOS(fpHCStruc);
    }
	fpHCStruc->dHCFlag &= ~(HC_STATE_OWNERSHIP_CHANGE_IN_PROGRESS);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProcessSmiChangeToEHCD
//
// Description: This procedure process OwnerShipChange for BIOS -> EHCD.
//
// Input:   fpHCStruc   HCStruc pointer
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ProcessSmiChangeToEHCD (HC_STRUC* fpHCStruc)
{
    EHCI_Stop(fpHCStruc);    // Stop EHCI legacy
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProcessSmiChangeToBIOS
//
// Description: This procedure process OwnerShipChange for EHCD -> BIOS.
//
// Input:   SI  HCStruc pointer
//
// Output:  None
//
// Modified:    None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ProcessSmiChangeToBIOS (
    HC_STRUC    *fpHCStruc
)
{
    HC_STRUC    *Hc;
    UINT8       Count;
    DEV_INFO    *Device;

    // Stop UHCI devices connected to the companions
    // Core8 executes this under MKF_PCCHECK_PATCH==1 condition, EIP10272
    for (Count = 1; Count < MAX_DEVICES; Count++) {
        Device = &gUsbData->aDevInfoTable[Count];
        Hc = gUsbData->HcTable[Device->bHCNumber - 1];

        if (Hc->bHCType != USB_HC_UHCI) continue;   // Not UHCI

        if ((Device->bFlag & DEV_INFO_VALID_STRUC)==0) continue;    // Not valid

        if ((Hc->wBusDevFuncNum & 0xfff8) !=
            (fpHCStruc->wBusDevFuncNum & 0xfff8)) continue; // Not a companion

        // Found a device connected to UHCI companion controller. Stop it.
        USB_StopDevice(Hc, Device->bHubDeviceNumber, Device->bHubPortNumber);
    }

    EHCI_Start(fpHCStruc);  // Reinitialize EHCI host controller
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_ReleasePortOwner
//
// Description: 
//
// Input:       HcStruc   - Pointer to HCStruc of the host controller
//              PortNum   - Port in the HC whose status is requested
//
// Output:      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_ReleasePortOwner(
    HC_STRUC*	HcStruc,
    UINT8		PortNum
)
{
    UINT16  PortReg = (UINT16)((PortNum-1)*4 + EHCI_PORTSC);
    UINT16  i;

	if ((HcStruc->dHCSParams & EHCI_N_CC) == 0) {
		return USB_SUCCESS;
	}

    if (!(EhciReadOpReg(HcStruc, PortReg) & EHCI_CURRENTCONNECTSTATUS)) {
        return USB_ERROR;
    }

    USB_DEBUG(DEBUG_LEVEL_3, "Release EHCI port %d\n", PortNum);
    EhciSetOpReg(HcStruc, PortReg, EHCI_PORTOWNER);

    // Loop until Full speed device disconnect event process done.
    // This change is done in sync with Core8 except the extra 400mS delay
    for (i = 0; i < 200; i++) {
        if (EhciReadOpReg(HcStruc, PortReg) & EHCI_CONNECTSTATUSCHANGE) {
            break;
        }
        FixedDelay(100); 
    }

    EhciSetOpReg(HcStruc, PortReg, EHCI_CONNECTSTATUSCHANGE);

	return USB_SUCCESS;	
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIGetRootHubStatus
//
// Description: This function returns the port connect status for the
//              root hub port
//
// Input:       fpHCStruc   - Pointer to HCStruc of the host controller
//              bPortNum    - Port in the HC whose status is requested
//
// Output:      Port status flags (see USB_PORT_STAT_XX equates)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_GetRootHubStatus(
    HC_STRUC* fpHCStruc,
    UINT8 bPortNum
)
{
    UINT32  dTmp;
    UINT8   bStatus     = USB_PORT_STAT_DEV_OWNER;
    UINT16  wPortReg    = (UINT16)((bPortNum-1)*4 + EHCI_PORTSC);

    if (fpHCStruc->DebugPort && fpHCStruc->DebugPort==bPortNum) return 0;

    //
    // Read the status of the port
    //
    dTmp    = EhciReadOpReg(fpHCStruc, wPortReg);
	USB_DEBUG(3, "Ehci port[%d] status: %08x\n", bPortNum, dTmp);

	// Detect the high-speed device.
	// In case of low-speed or full-speed change the ownership to a
	// companion 1.1 controller (if any)
	if (dTmp & EHCI_CURRENTCONNECTSTATUS) {
    	// Analyze Line Status
    	if ((dTmp & EHCI_LINE_STATUS) == EHCI_DMINUSBIT) {	// Low speed device connected
    		EHCI_ReleasePortOwner(fpHCStruc, bPortNum);
			dTmp = EhciReadOpReg(fpHCStruc, wPortReg);
    	}
	}

    //
    // Check the connect status change bit
    //
    if (dTmp & EHCI_CONNECTSTATUSCHANGE) {
        //
        // Set connect status change flag
        //
        bStatus |= USB_PORT_STAT_DEV_CONNECT_CHANGED;

        //
        // Wait 20ms for host controller could report accurate port status properly.
        //
        //FixedDelay(gUsbData->UsbTimingPolicy.EhciPortConnect * 1000);   // 20ms delay

        //
        // Read the status of the port
        //
        //dTmp = EhciReadOpReg(fpHCStruc, wPortReg);

		// Clear connect status change
		EhciSetOpReg(fpHCStruc, wPortReg, EHCI_CONNECTSTATUSCHANGE);		//(EIP61030+)
    }

    if (dTmp & EHCI_CURRENTCONNECTSTATUS) {
        bStatus |= USB_PORT_STAT_DEV_CONNECTED;

		if (dTmp & EHCI_PORTENABLE) {
            bStatus |= USB_PORT_STAT_DEV_HISPEED;

            // Patch for CloverTrail
            if (fpHCStruc->Vid == 0x8086 && 
                (fpHCStruc->Did == 0xE006 || fpHCStruc->Did == 0x08F2)) {                
                if ((dTmp & EHCI_LINE_STATUS) == EHCI_DMINUSBIT) {
                    bStatus &= ~USB_PORT_STAT_DEV_HISPEED;
                    bStatus |= USB_PORT_STAT_DEV_LOWSPEED;
                } else  if ((dTmp & EHCI_LINE_STATUS) == EHCI_DPLUSBIT) {
                    bStatus &= ~USB_PORT_STAT_DEV_HISPEED;
                    bStatus |= USB_PORT_STAT_DEV_FULLSPEED;
                }
            }
			bStatus |= USB_PORT_STAT_DEV_ENABLED;
										//(EIP61030+)>
		} else {
			if (gUsbData->bIgnoreConnectStsChng == TRUE) {
				if (!(dTmp & EHCI_CONNECTSTATUSCHANGE)) {
					bStatus |= USB_PORT_STAT_DEV_CONNECT_CHANGED;
				}
			}
										//<(EIP61030+)
		}
	}

    if (dTmp & EHCI_PORTOWNER) {
        bStatus &= ~USB_PORT_STAT_DEV_OWNER;
    }

	return bStatus;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIDisableRootHub
//
// Description: This function disables the EHCI HC Ruoot hub port.
//
// Input:       fpHCStruc   - Pointer to HCStruc of the host controller
//              bPortNum    - Port in the HC to disable
//
// Output:      USB_SUCCESS on success
//              USB_ERROR   on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_DisableRootHub(
    HC_STRUC* fpHCStruc,
    UINT8 bPortNum)
{
										//(EIP58108+)>
    UINT16  PortReg = (UINT16)((bPortNum-1)*4 + EHCI_PORTSC);
	UINT32	i;

    if (fpHCStruc->DebugPort && fpHCStruc->DebugPort==bPortNum) return USB_SUCCESS;

	if (!(EhciReadOpReg(fpHCStruc, PortReg) & EHCI_PORTENABLE)) {
		return USB_SUCCESS;
	}
    EhciClearOpReg(fpHCStruc, PortReg, EHCI_PORTENABLE);

	for (i = 0; i < 100; i++) {
		if ((EhciReadOpReg(fpHCStruc, PortReg) & EHCI_PORTENABLE) == 0) {
			break;
		}
		FixedDelay(100);
	}
										//<(EIP58108+)
    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIEnableRootHub
//
// Description: This function enables the EHCI HC Root hub port.
//
// Input:       fpHCStruc   - Pointer to HCStruc of the host controller
//              bPortNum    - Port in the HC to enable
//
// Output:      USB_SUCCESS on success
//              USB_ERROR   on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_EnableRootHub(
    HC_STRUC* fpHCStruc,
    UINT8 bPortNum)
{
//
// Software can only enable the EHCI root hub ports by port RESET.  HC will
// enable the port only if it is a high speed device
//
    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_ResetRootHub
//
// Description: This function resets the EHCI HC Root hub port.
//
// Input:       HcStruc   - Pointer to HCStruc of the host controller
//              PortNum    - Port in the HC to enable
//
// Output:      USB_SUCCESS on success
//              USB_ERROR   on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_ResetRootHub(
    HC_STRUC*	HcStruc,
    UINT8		PortNum
)
{
    UINT16  PortReg = (UINT16)((PortNum-1)*4 + EHCI_PORTSC);
	UINT32	i;

	// Disable the port if it is enabled
	if (EhciReadOpReg(HcStruc, PortReg) & EHCI_PORTENABLE) {
		EhciClearOpReg(HcStruc, PortReg, EHCI_PORTENABLE);
	
		// There may be a delay in disabling or enabling a port due to other 
		// host controller and bus events.
		for (i = 0; i < 100; i++) {
			if ((EhciReadOpReg(HcStruc, PortReg) & EHCI_PORTENABLE) == 0) {
				break;
			}
			FixedDelay(100);    // 100 us delay
		}
	}

	// Reset the port
	EhciSetOpReg(HcStruc, PortReg, EHCI_PORTRESET);

    if ((HcStruc->dHCFlag & HC_STATE_CONTROLLER_WITH_RMH) && (PortNum == 1)) {
        FixedDelay(3 * 1000);	 // 3 ms delay
    } else {
    	// Wait til port disable is complete (Tdrstr=50ms Ref 7.1.7.5 of USB Spec 2.0)
    	FixedDelay(50 * 1000);	 // 50 ms delay        
    }

	EhciClearOpReg(HcStruc, PortReg, EHCI_PORTRESET);	// Terminate reset

    if (!(EhciReadOpReg(HcStruc, PortReg) & EHCI_CURRENTCONNECTSTATUS)) {
        return USB_ERROR;
    }

	// if the port detects that the attached device is high-speed during reset, 
	// then the host controller must have the port in the enabled state within 2ms 
	// of software writing this bit to a zero.
	for (i = 0; i < 20; i++) {
		if ((EhciReadOpReg(HcStruc, PortReg) & (EHCI_PORTRESET | 
			EHCI_PORTENABLE)) == EHCI_PORTENABLE) {
			break;
		}
        FixedDelay(100);	 // 100 us delay
	}

	if (EhciReadOpReg(HcStruc, PortReg) & EHCI_PORTRESET) {	 // Reset failed
		USBLogError(USB_ERR_PORT_RESET_FAILED);
		return USB_ERROR;
	}

	if (!(EhciReadOpReg(HcStruc, PortReg) & EHCI_PORTENABLE)) {
        if (!(EhciReadOpReg(HcStruc, PortReg) & EHCI_CURRENTCONNECTSTATUS)) {
            return USB_ERROR;
        } else {
		    EHCI_ReleasePortOwner(HcStruc, PortNum);
		    return USB_ERROR;
        }
	}

	FixedDelay(1 * 1000);	 // 1 ms delay

    return USB_SUCCESS;
}

                                        //(EIP54018+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        EHCI_GlobalSuspend
//
// Description: 
//  This function suspend the EHCI HC.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_GlobalSuspend(
    HC_STRUC*	HcStruc
)
{
    UINT16  PortReg;
	UINT32	PortSts;
    UINT8   PortNum;
    UINT8   i;

    for(PortNum = 1; PortNum <= HcStruc->bNumPorts; PortNum++) {
        PortReg = (UINT16)(EHCI_PORTSC + (PortNum - 1) * 4 );
        PortSts = EhciReadOpReg(HcStruc, PortReg );
        USB_DEBUG(DEBUG_LEVEL_3,"EHCI PortSts[%x] %x \n",
                        PortNum, PortSts); 
        // Check if port is disabled or suspended.
        if((PortSts & EHCI_PORTENABLE) && (!(PortSts & EHCI_SUSPEND))) {
            // Suspend if necessary.
            EhciClearOpReg(HcStruc, PortReg,
                            EHCI_WKOC_E | EHCI_WKDSCNNT_E | EHCI_WKCNNT_E);
            EhciSetOpReg(HcStruc, PortReg, EHCI_SUSPEND);
            // Read PortSc until port shows suspended. 
     	    for(i = 0; i < 100; i++) {
                if(EhciReadOpReg(HcStruc, PortReg) & EHCI_SUSPEND) {
                    break;
                }
                FixedDelay(100);      // 100 us delay
	        } 
        }
    }

    // Turn HC off and wait for the Halted bit to get set
    EhciClearOpReg(HcStruc, EHCI_USBCMD, EHCI_RUNSTOP);
    // The Host Controller must halt within 16 micro-frames after 
    // software clears the Run bit. 
	for(i = 0; i < 16; i++) {
        if(EhciReadOpReg(HcStruc, EHCI_USBSTS) & EHCI_HCHALTED) {
            break;
        }
        FixedDelay(125);      // 125 us delay
	}

    HcStruc->dHCFlag &= ~(HC_STATE_RUNNING);
    HcStruc->dHCFlag |= HC_STATE_SUSPEND;

    return USB_SUCCESS;
}
                                        //<(EIP54018+)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_GetHiSpeedHubPortNumber
//
// Description: This function gets the hi-speed hub's device and port number
//              to which this low speed device is connected.  It parses
//              through its parents until it finds the correct device. This
//              information is used for split transaction
//
// Input:   fpDevInfo   - Device info pointer of the device
//
// Output:  UINT16      - Device/port number of the hi-speed hub
//
// Notes:   This low/full speed device may be behind different hubs as
//          shown below. In any case this routine will get the device
//          address of the hub number HISP_A :
//      Notations used:
//          MBPortX     Motherboard USB port
//          HISP_X      Hi-speed hub number X
//          FUSP_X      Full-speed hub number X
//          Device      Low/Full speed device
//      Config 1:
//        MBPortX --> HISP_A --> Device
//      Config 2:
//        MBPortX --> HISP_A --> FUSP_1 --> Device
//      Config 3:
//        MBPortX --> HISP_B --> HISP_A --> Device
//      Config 4:
//        MBPortX --> HISP_A --> FUSP_1 --> HISP_B --> Device
//        In the above configuration the HISP_B will be operated in
//      full speed rather than hi-speed since it is connected to a
//      full speed hub
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
EHCI_GetHiSpeedHubPortNumber(DEV_INFO* fpDevInfo)
{
    DEV_INFO*   fpHubDev = fpDevInfo;
    DEV_INFO*   fpParentHubDev;
    UINT16      wRetCode;

    if( !VALID_DEVINFO( fpDevInfo) )
        return 0;

//
// Get the device info structure for the matching device address
//
    //
    // Get the device number of the immediate hub, then get the device
    // info structure for this device number
    //
    for(;;)
    {
        fpParentHubDev  = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR,
                                0, fpHubDev->bHubDeviceNumber, 0);
        if ( !fpParentHubDev )  // Error. Exit !
        {
            return  0;
        }
        if (((fpParentHubDev->bEndpointSpeed << USB_PORT_STAT_DEV_SPEED_MASK_SHIFT)
             & USB_PORT_STAT_DEV_SPEED_MASK) == 0) break;
        fpHubDev = fpParentHubDev;
    }
    //
    // The first USB 2.0 hub found as fpHubDev to which the low/full speed
    // device is connected
    //
    wRetCode = (UINT16)((fpHubDev->bHubPortNumber << 7) |
                                fpHubDev->bHubDeviceNumber);

    return wRetCode;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EhciExexuteAsyncSchedule
//
// Description: This function insert the requested QH to asynchronous schedule
//				and waits until the QH completes or the transaction time-out.
//
// Input:   HcStruc   - Pointer to HCStruc of the host controller
//          XferQh    - Pointer to the QH which has to be completed
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
EhciExexuteAsyncSchedule(
	HC_STRUC	*HcStruc,
	EHCI_QH 	*XferQh
)
{
	UINT16	Status = USB_SUCCESS;
	UINT32	Count;
    UINT32  TimeOut = gUsbData->wTimeOutValue * 100; // in 10 macrosecond unit

#if EHCI_ASYNC_BELL_SUPPORT
	UINT32	Tmp;

	XferQh->dLinkPointer = EHCI_TERMINATE;
	XferQh->bActive = TRUE;

	//
	// Insert the Control/Bulk QH into the Async list
	//
	Tmp = gUsbData->fpQHAsyncXfer->dLinkPointer;
	gUsbData->fpQHAsyncXfer->dLinkPointer  = (UINT32)XferQh | EHCI_QUEUE_HEAD;
	XferQh->dLinkPointer = Tmp;
#else
	//
	// Set the ASYNCLISTADDR register to point to the Control/Bulk QH
	//
    EhciWriteOpReg(HcStruc, EHCI_ASYNCLISTADDR, (UINT32)(UINTN)XferQh);

	//
	// Set next QH pointer to itself (circular link)
	//
	XferQh->dLinkPointer = (UINT32)((UINTN)XferQh | EHCI_QUEUE_HEAD);
	XferQh->bActive = TRUE;
#endif

	//
    // Now put the Control/Bulk QH into the HC's schedule by
    // setting the Async. schedule enabled field of USBCMD register
    // This will cause the HC to execute the transaction in the next active frame.
    //
    EHCIStartAsyncSchedule(HcStruc);

	// Wait for tansfer complete
	for(Count = 0; !TimeOut || Count < TimeOut; Count++) {
		EHCIProcessQH(HcStruc, XferQh);
		if(XferQh->bActive == FALSE) {
			break;
		}
		FixedDelay(10);  // 10 microsec
	}

#if EHCI_ASYNC_BELL_SUPPORT
    //
    // Disconnect Control/Bulk QH from the Async list
    //
    EHCIRemoveQHFromAsyncList(HcStruc, XferQh);
#else
	//
	// Stop the Async transfer
	//
	EHCIStopAsyncSchedule(HcStruc);
#endif

	if(XferQh->bActive == TRUE) {
		XferQh->bActive = FALSE;
		Status = USB_ERROR;
		USB_DEBUG (DEBUG_LEVEL_3, "EHCI Time-Out\n");
	}
	
	// Service all interrupts
	EHCI_ProcessInterrupt(HcStruc);

	return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:    EHCI_ControlTransfer
//
// Description: This function executes a device request command transaction
//              on the USB. One setup packet is generated containing the
//              device request parameters supplied by the caller.  The setup
//              packet may be followed by data in or data out packets
//              containing data sent from the host to the device
//              or vice-versa. This function will not return until the
//              request either completes successfully or completes in error
//              (due to time out, etc.)
//
// Parameters:  fpHCStruc   Pointer to HCStruc of the host controller
//      pDevInfo    DeviceInfo structure (if available else 0)
//      wRequest    Request type (low byte)
//              Bit 7   : Data direction
//                  0 = Host sending data to device
//                  1 = Device sending data to host
//              Bit 6-5 : Type
//                  00 = Standard USB request
//                  01 = Class specific
//                  10 = Vendor specific
//                  11 = Reserved
//              Bit 4-0 : Recipient
//                  00000 = Device
//                  00001 = Interface
//                  00010 = Endpoint
//                  00100 - 11111 = Reserved
//              Request code, a one byte code describing
//              the actual device request to be executed
//              (ex: Get Configuration, Set Address etc)
//      wIndex      wIndex request parameter (meaning varies)
//      wValue      wValue request parameter (meaning varies)
//      fpBuffer    Buffer containing data to be sent to the
//              device or buffer to be used to receive data
//      wLength     wLength request parameter, number of bytes
//              of data to be transferred in or out
//              of the host controller
//
//
// Output:  Number of bytes actually transferred
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
EHCI_ControlTransfer (
    HC_STRUC    *fpHCStruc,
    DEV_INFO    *fpDevInfo,
    UINT16      wRequest,
    UINT16      wIndex,
    UINT16      wValue,
    UINT8       *fpBuffer,
    UINT16      wLength)
{
    UINT32      dTmp, dTmp1;
    UINT16      wRetCode = 0;   // Initialize with error
    EHCI_QH     *fpQHCtl;
    EHCI_QTD    *fpQTDCtlSetup, *fpQTDCtlData, *fpQTDCtlStatus;
    DEV_REQ     *fpRequest = NULL;
    UINT8       bEndpointSpeed;
	UINT8		*BufPhyAddr = NULL;
	VOID		*BufferMapping = NULL;

	if (EhciIsHalted(fpHCStruc)) {
		return 0;
	}

    if( !VALID_DEVINFO( fpDevInfo) )
        return 0;
    
    gUsbData->dLastCommandStatusExtended = 0;	//(EIP84790+)

//USB_DEBUG(DEBUG_LEVEL_3, "EHCI_ControlTransfer..\n");
    //
    // Build the device request in the data area of the control setup qTD
    //
    fpRequest = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT(sizeof(DEV_REQ)));
	ASSERT(fpRequest);
    if (fpRequest == NULL) {
        return 0;
    }

    fpRequest->wRequestType = wRequest;
    fpRequest->wIndex       = wIndex;
    fpRequest->wValue       = wValue;
    fpRequest->wDataLength  = wLength;

//
// The QH endpoint characteristic field will be set so
//   Function address & Endpoint number = From DeviceInfo structure,
//   Direction = From TD,
//   Speed = DeviceInfo.bEndpointSpeed,
//   Skip = 1, Format = 0,
//   Max packet size  = DeviceInfo.wEndp0MaxPacket
// The dNextqTDPtr field will be set to qTDControlSetup
// The dAltNextqTDPtr field will be set to EHCI_TERMINATE
// The dCurrentqTDPtr field will be set to 0
//
    //
    // Intialize the queue head with null pointers
    //
										//(EIP71067)>
    fpQHCtl = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT((1 * sizeof(EHCI_QH))+
                                             			(3 * sizeof(EHCI_QTD))));
                                        //(EIP83295+)>
    if(!fpQHCtl) {
        return 0;
    }
                                        //<(EIP83295+)
										//(EIP81030)>
    fpQTDCtlSetup = (EHCI_QTD*)((UINTN)fpQHCtl + sizeof(EHCI_QH));
    fpQTDCtlData = (EHCI_QTD*)((UINTN)fpQTDCtlSetup + sizeof(EHCI_QTD));
    fpQTDCtlStatus = (EHCI_QTD*)((UINTN)fpQTDCtlData + sizeof(EHCI_QTD));
										//<(EIP71067)
										//<EIP81030)
    EHCIInitializeQueueHead(fpQHCtl);

    bEndpointSpeed = fpDevInfo->bEndpointSpeed; // 00/01/10 for HI/LO/FULL

    //
    // Assume as a high speed device
    //
    dTmp = QH_HIGH_SPEED;   // 10b - High speed

    //
    // Check for high speed
    //
    if (bEndpointSpeed)         // Low/Full speed device
    {
        dTmp = ((UINT32)bEndpointSpeed & 1) << 12;  // Bit 12 = full/low speed flag
        dTmp |= QH_CONTROL_ENDPOINT;
        //
        // Set the hub address and port number
        // Get the Hispeed hub port number & device number
        //
        dTmp1 = (UINT32)EHCI_GetHiSpeedHubPortNumber(fpDevInfo);
        dTmp1 = (dTmp1 << 16);		// Split complete Xaction
        fpQHCtl->dEndPntCap |= dTmp1;
    }

//USB_DEBUG(DEBUG_LEVEL_3, "Tmp1..%x\n", dTmp);

    //
    // Use data toggle from qTD and this QH is the head of the queue
    //
#if EHCI_ASYNC_BELL_SUPPORT
    dTmp |= QH_USE_QTD_DT;
#else
    dTmp |= QH_USE_QTD_DT;
    // Do not set QH_HEAD_OF_LIST bit on VIA controller
    if (fpHCStruc->Vid != 0x1106) {
        dTmp |= QH_HEAD_OF_LIST;
    }
#endif
    dTmp |= (UINT32)fpDevInfo->bDeviceAddress;
    //
    // dTmp[Bits 6:0] = Dev. Addr
    // dTmp[Bit7] = I bit(0)
    // dTmp[Bits11:8] = Endpoint (0)
    //

    dTmp1 = (UINT32)fpDevInfo->wEndp0MaxPacket;
    dTmp |= (dTmp1 << 16);  // Tmp[Bits26:16] = device's packet size
    fpQHCtl->dEndPntCharac  = dTmp;

    //
    // Fill in various fields in the qTDControlSetup.
    //
    //fpQTDCtlSetup = fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpqTDControlSetup;	//(EIP71067-)

    //
    // The token field will be set so
    //   Direction PID = QTD_SETUP_TOKEN,
    //   Size = size of the data,
    //   Data Toggle = QTD_SETUP_TOGGLE,
    //   Error Count = QTD_THREE_ERRORS,
    //   Status code = QTD_DO_OUT + QTD_ACTIVE
    // The buffer pointers field will point to the aControlSetupData buffer
    //   which was before initialized to contain a DeviceRequest struc.
    // The dNextqTDPtr field will point to the qTDControlData if data will
    //   be sent/received or to the qTDControlStatus if no data is expected.
    // The dAltNextqTDPtr field will be set to EHCI_TERMINATE
    //
    fpQTDCtlSetup->dToken = QTD_SETUP_TOKEN |
                QTD_SETUP_TOGGLE | QTD_IOC_BIT |
                QTD_THREE_ERRORS | QTD_DO_OUT | QTD_ACTIVE |
                (8 << 16);  // Data size

    //
    // Update buffer pointers
    //
    EHCISetQTDBufferPointers(fpQTDCtlSetup, (UINT8*)fpRequest, 8);
    //fpQTDCtlData = fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpqTDControlData;	//(EIP71067-)

    if (wLength)    // br if no data to transfer
    {
        //
        // Fill in various fields in the qTDControlData
        //
        // The token field will be set so
        //   Direction PID = QTD_OUT_TOKEN/QTD_IN_TOKEN,
        //   Size = size of the data,
        //   Data Toggle = QTD_DATA1_TOGGLE,
        //   Error Count = QTD_THREE_ERRORS,
        //   Status code = QTD_DO_OUT(if it is out) + QTD_ACTIVE
        // The buffer pointers field will point to the fpBuffer buffer
        //   which was before initialized to contain a DeviceRequest struc.
        // The dNextqTDPtr field will point to the qTDControlSetup
        // The dAltNextqTDPtr field will be set to EHCI_TERMINATE
        //
        fpQTDCtlData->dToken = QTD_IN_TOKEN |
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_THREE_ERRORS | QTD_ACTIVE;
        if (!(wRequest & BIT7)) // Br if host sending data to device (OUT)
        {
            fpQTDCtlData->dToken    = QTD_OUT_TOKEN |
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_THREE_ERRORS | QTD_DO_OUT | QTD_ACTIVE;
        }

        //
        // Set length
        //
        fpQTDCtlData->dToken |= ((UINT32)wLength << 16);

		EhciDmaMap(fpHCStruc, (UINT8)(wRequest & BIT7), fpBuffer, wLength, 
			&BufPhyAddr, &BufferMapping);

        //
        // Update buffer pointers
        //
        EHCISetQTDBufferPointers(fpQTDCtlData,
                (UINT8*)BufPhyAddr,
                (UINT32)wLength);
    }

    //
    // Fill in various fields in the qTDControlStatus
    //
    //fpQTDCtlStatus = fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpqTDControlStatus;	//(EIP71067-)

    //
    // The token field will be set so
    //   Direction PID = QTD_OUT_TOKEN/QTD_IN_TOKEN,
    //   Size = 0,
    //   Data Toggle = QTD_DATA1_TOGGLE,
    //   Error Count = QTD_THREE_ERRORS,
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
                QTD_THREE_ERRORS | QTD_ACTIVE;
    if(wRequest & BIT7)
    {
        fpQTDCtlStatus->dToken  = QTD_OUT_TOKEN |
                QTD_DATA1_TOGGLE | QTD_IOC_BIT |
                QTD_THREE_ERRORS | QTD_DO_OUT | QTD_ACTIVE;
    }

    EHCISetQTDBufferPointers(fpQTDCtlStatus, NULL, 0);

    //
    // Link the qTD formed now and connect them with the control queue head
    //
    fpQHCtl->fpFirstqTD     = fpQTDCtlSetup;
    fpQHCtl->dNextqTDPtr    = (UINT32)(UINTN)fpQTDCtlSetup;

    if(wLength)
    {
        fpQTDCtlSetup->dNextqTDPtr  = (UINT32)(UINTN)fpQTDCtlData;
        fpQTDCtlData->dNextqTDPtr   = (UINT32)(UINTN)fpQTDCtlStatus;
    }
    else
    {
        fpQTDCtlSetup->dNextqTDPtr  = (UINT32)(UINTN)fpQTDCtlStatus;
    }

    fpQTDCtlStatus->dNextqTDPtr = EHCI_TERMINATE;

	wRetCode = EhciExexuteAsyncSchedule(fpHCStruc, fpQHCtl);
    fpQHCtl->fpFirstqTD     = 0;
    fpQHCtl->dNextqTDPtr    = EHCI_TERMINATE;

	if (wLength) {
		EhciDmaUnmap(fpHCStruc, BufferMapping);
        wLength = wLength - (UINT16)((fpQTDCtlData->dToken & ~(QTD_DATA_TOGGLE)) >> 16);
	}

    //
    // Clear the stalled condition flag
    //
    gUsbData->bLastCommandStatus &= ~USB_CONTROL_STALLED;

    //
    // Check whether the QH stopped or timed out
    //
    if (wRetCode != USB_SUCCESS) {
        gUsbData->dLastCommandStatusExtended |= USB_TRNSFR_TIMEOUT;	//(EIP84790+)
		wLength = 0;                    //(EIP71067)
	}

	if (fpQHCtl->bErrorStatus & QTD_HALTED) {
        //
        // Command stalled set the error bit appropriately
        //
        gUsbData->bLastCommandStatus   |= USB_CONTROL_STALLED;
		wLength = 0;                    //(EIP71067)
    }
                                        //(EIP71067+)>
    EhciMemFree(fpHCStruc, fpQHCtl, GET_MEM_BLK_COUNT((1 * sizeof(EHCI_QH))+
                                           (3 * sizeof(EHCI_QTD))));
                                        //<(EIP71067+)
	EhciMemFree(fpHCStruc, fpRequest, GET_MEM_BLK_COUNT(sizeof(DEV_REQ)));

	return wLength;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:    EHCI_BulkTransfer
//
// Description: This function executes a bulk transaction on the USB. The
//              transfer may be either DATA_IN or DATA_OUT packets containing
//              data sent from the host to the device or vice-versa. This
//              function wil not return until the request either completes
//              successfully or completes with error (due to time out, etc.)
//              NOTE: Make sure that amount of bytes to transfer should not
//              exceed MAX_EHCI_DATA_SIZE
//
// Parameters:  pHCStruc    Pointer to HCStruc of the host controller
//              pDevInfo    DeviceInfo structure (if available else 0)
//              bXferDir    Transfer direction
//                  Bit 7: Data direction
//                          0 Host sending data to device
//                          1 Device sending data to host
//                  Bit 6-0 : Reserved
//              fpBuffer    Buffer containing data to be sent to the
//                          device or buffer to be used to receive data
//                          value in Segment:Offset format
//              dwLength    dwLength request parameter, number of bytes
//                          of data to be transferred in or out
//                          of the host controller
//
// Output:      Amount of data transferred
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
EHCI_BulkTransfer(
    HC_STRUC    *fpHCStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       bXferDir,
    UINT8       *fpBuffer,
    UINT32      dwLength)
{
    UINT16      wMaxPkt;
    UINT8       bEndp, bDatToggle;
    EHCI_QH     *fpQHBulk;
    EHCI_QTD    *fpQTDBulkData;
    UINT32      dTmp, dTmp1;
    UINT16      Status;
    UINT32      dBytesToTransfer, dBytesRemaining;
    UINT32      dBytesTransferred;
	UINT8		*BufPhyAddr = NULL;
	VOID		*BufferMapping = NULL;

	if (EhciIsHalted(fpHCStruc)) {
		return 0;
	}

    if( !VALID_DEVINFO( fpDevInfo) )
        return 0;

    //clear HW source of error
    gUsbData->dLastCommandStatusExtended = 0;


    dBytesRemaining     = dwLength;
    dBytesTransferred   = 0;
		
	//
	// Get Bulk IN/OUT enpoint number, data sync value & max packet size
	//
	if (bXferDir & BIT7)
    {
        wMaxPkt = fpDevInfo->wBulkInMaxPkt;
        bEndp   = fpDevInfo->bBulkInEndpoint;
    }
    else
    {
        wMaxPkt = fpDevInfo->wBulkOutMaxPkt;
        bEndp   = fpDevInfo->bBulkOutEndpoint;
    }
    if( wMaxPkt == 0){
        return 0;
    }
                                        //(EIP71067+)>
    fpQHBulk = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT((1 * sizeof(EHCI_QH))+
                                              			 (1 * sizeof(EHCI_QTD))));
                                        //(EIP83295+)>
    if(!fpQHBulk) {
        return 0;
    }
                                        //<(EIP83295+)
    fpQTDBulkData = (EHCI_QTD*)((UINTN)fpQHBulk + sizeof(EHCI_QH));	//(EIP81030)
                                        //<(EIP71067+)
	EhciDmaMap(fpHCStruc, bXferDir, fpBuffer, dwLength, &BufPhyAddr, &BufferMapping);

    while (dBytesRemaining) {
        dBytesToTransfer = 
             (dBytesRemaining < HIGHSPEED_MAX_BULK_DATA_SIZE)?
                    dBytesRemaining : HIGHSPEED_MAX_BULK_DATA_SIZE;
		
		//
		// Get data toggle value
		//
    	bDatToggle = UsbGetDataToggle(fpDevInfo, bEndp | bXferDir);

        //
        // Set the QH's dNextqTDPtr field to bulk data qTD and dAltqTDPtr field to
        // EHCI_TERMINATE. Also set QH's link pointer to itself
        //
                                        //(EIP71067-)>
        //fpQHBulk        = fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpQHBulk;
        //fpQTDBulkData   = fpHCStruc->stDescPtrs.fpEHCIDescPtrs->fpqTDBulkData;
                                        //<(EIP71067-)
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
        // Device address & Endpoint
        //
        dTmp    = (UINT32)(fpDevInfo->bDeviceAddress | (bEndp << 8));
    
        //
        // Set max packet size
        //
        dTmp    = dTmp | ((UINT32)wMaxPkt << 16);
    
        //
        // Set the data toggle control
        //
    #if EHCI_ASYNC_BELL_SUPPORT
        dTmp    |= QH_USE_QTD_DT;
    #else
        dTmp |= QH_USE_QTD_DT;
        // Do not set QH_HEAD_OF_LIST bit on VIA controller
        if (fpHCStruc->Vid != 0x1106) {
            dTmp |= QH_HEAD_OF_LIST;
        }
    #endif
    
        //
        // Set the device speed
        // Reset the device speed bits
        //
        dTmp1   = (UINT32)fpDevInfo->bEndpointSpeed;    // 00/01/10 for HI/LO/FULL
    
        //
        // Assume as a high speed device
        //
        dTmp    |= QH_HIGH_SPEED;   // 10b - High speed
    
        //
        // Check for high speed
        //
        if (dTmp1)
        {
            dTmp1   = (dTmp1 & 1) << 12;    // Bit 12 = full/low speed flag
            dTmp    &= ~(QH_ENDPOINT_SPEED);
            dTmp    |= dTmp1;
            //
            // Set the hub address and port number
            //
            dTmp1   = (UINT32)EHCI_GetHiSpeedHubPortNumber(fpDevInfo);
            dTmp1   = (dTmp1 << 16);    	// Hispeed hub port number & device number
            fpQHBulk->dEndPntCap    |= dTmp1;   // Split complete Xaction
        }
    
        //
        // Update the endpoint characteristcs field with the data formed
        //
        fpQHBulk->dEndPntCharac = dTmp;
    
        //
        // Fill the bulk data qTD with relevant information
        // The token field will be set so
        //   Direction PID = QTD_OUT_TOKEN/QTD_IN_TOKEN,
        //   Size = size of the data,
        //   Data Toggle = bDatToggle,
        //   Error Count = QTD_THREE_ERRORS,
        //   Status code = QTD_DO_OUT(if it is out) + QTD_ACTIVE
        // The buffer pointers field will point to the fpBuffer buffer
        //   which was before initialized to contain a DeviceRequest struc.
        // The dNextqTDPtr field will point to the qTDControlSetup
        // The dAltNextqTDPtr field will be set to EHCI_TERMINATE
        //
        if (bXferDir & BIT7)
        {
            fpQTDBulkData->dToken   = QTD_IN_TOKEN |
                    QTD_IOC_BIT |
                    QTD_THREE_ERRORS | QTD_ACTIVE;
        }
        else
        {
            fpQTDBulkData->dToken   = QTD_OUT_TOKEN |
                    QTD_IOC_BIT |
                    QTD_THREE_ERRORS | QTD_DO_OUT | QTD_ACTIVE;
        }
    
        //
        // Set the data toggle depending on the bDatToggle value
        //
        fpQTDBulkData->dToken   |= (UINT32)bDatToggle << 31;
    
        //
        // Set length
        //
        fpQTDBulkData->dToken   |= (dBytesToTransfer << 16);
    
        //
        // Update buffer pointers
        //
        EHCISetQTDBufferPointers(fpQTDBulkData, BufPhyAddr, dBytesToTransfer);
    
        //
        // Update next & alternate next qTD pointers
        //
        fpQTDBulkData->dNextqTDPtr      = EHCI_TERMINATE;
        fpQTDBulkData->dAltNextqTDPtr   = EHCI_TERMINATE;
    
        fpQHBulk->bActive   = TRUE;
    
    
        //
        // Set bulk condition as not stalled
        //
        gUsbData->bLastCommandStatus &= ~(USB_BULK_STALLED + USB_BULK_TIMEDOUT);
    
        //
        // Now wait for bulk transaction to be complete
        // the EHCIProcessInterrupt will set its active flag to FALSE.
        // Now wait for the bulk transfer to complete
        //
		Status = EhciExexuteAsyncSchedule(fpHCStruc, fpQHBulk);

		fpQHBulk->fpFirstqTD	= 0;
		fpQHBulk->dNextqTDPtr	= EHCI_TERMINATE;

		if(Status != USB_SUCCESS) {
			//
            // Set time out status
            //
            gUsbData->bLastCommandStatus |= USB_BULK_TIMEDOUT;
            gUsbData->dLastCommandStatusExtended  |= USB_TRNSFR_TIMEOUT;
			break;
		}

		if (fpQHBulk->bErrorStatus & QTD_HALTED) {
			//
			// Stall condition
			//
            gUsbData->bLastCommandStatus   &= ~(USB_BULK_TIMEDOUT);
            gUsbData->bLastCommandStatus   |= USB_BULK_STALLED;
            gUsbData->dLastCommandStatusExtended  |= USB_TRSFR_STALLED;
			break;
		}

        //
        // Update the data toggle value into the mass info structure
        //
        bDatToggle =
            (UINT8)(((fpQHBulk->dToken & QH_DATA_TOGGLE) >> 31) & 1);
		UsbUpdateDataToggle(fpDevInfo, bEndp | bXferDir, bDatToggle);

        //
        // Get the size of data transferred
        //
        dTmp = (fpQTDBulkData->dToken & ~(QTD_DATA_TOGGLE)) >> 16;
        dTmp = (dTmp)? dBytesToTransfer-dTmp : dBytesToTransfer;

        if (!dTmp) {
            break;
        }

        //
        // Adjust loop variables
        //
        dBytesRemaining = dBytesRemaining - dTmp;
        dBytesTransferred += dTmp;

        if (dTmp < dBytesToTransfer) {
            break;
        }

        BufPhyAddr += dTmp;
    }

	EhciDmaUnmap(fpHCStruc, BufferMapping);
                                        //(EIP71067+)>
    EhciMemFree(fpHCStruc, fpQHBulk, GET_MEM_BLK_COUNT( (1 * sizeof(EHCI_QH)) +
                                             			(1 * sizeof(EHCI_QTD))));
                                        //<(EIP71067+)
    return  dBytesTransferred;

}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:    EHCI_InterruptTransfer
//
// Description: This function executes an interrupt transaction on the USB.
//              The data transfer direction is always DATA_IN. This
//              function wil not return until the request either completes
//              successfully or completes in error (due to time out, etc.)
//
// Parameters:  fpHCStruc   Pointer to HCStruc of the host controller
//              fpDevInfo   DeviceInfo structure (if available else 0)
//              fpBuffer    Buffer containing data to be sent to the
//                          device or buffer to be used to receive data
//              wLength     wLength request parameter, number of bytes
//                          of data to be transferred in
//
// Output:      Number of bytes transferred
//
//
// Notes:       DO NOT TOUCH THE LINK POINTER OF THE TDInterruptData. It is
//              statically allocated and linked with other items in the
//              1ms schedule
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
EHCI_InterruptTransfer (
    HC_STRUC    *fpHCStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       *fpBuffer,
    UINT16      wLength
)
{

    UINT8       bDatToggle;
    EHCI_QH     *fpQHInt;
    UINT32      dTmp, dTmp1;
    EHCI_QTD    *fpqTDIntData;
	UINT32		Count;
	UINT32		Timeout;
	UINT32		BytesTransferred;
	UINT8		*BufPhyAddr = NULL;
	VOID		*BufferMapping = NULL;

	if (EhciIsHalted(fpHCStruc)) {
		return 0;
	}

	gUsbData->dLastCommandStatusExtended = 0;

	bDatToggle = UsbGetDataToggle(fpDevInfo, fpDevInfo->bIntEndpoint);

    //
    // Get the QHInterrupt pointer
    //
    fpQHInt = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT((1 * sizeof(EHCI_QH))+
                                             			(1 * sizeof(EHCI_QTD))));
	if (fpQHInt == NULL) {
		return 0;
	}

	fpqTDIntData = (EHCI_QTD*)((UINTN)fpQHInt + sizeof(EHCI_QH));

	//
	// Intialize the queue head
	//
	EHCIInitializeQueueHead(fpQHInt);

	//
	// Set the first qTD pointer
	//
	fpQHInt->fpFirstqTD	= fpqTDIntData;
	fpQHInt->dNextqTDPtr	= (UINT32)(UINTN)fpqTDIntData;
	fpQHInt->dLinkPointer	= EHCI_TERMINATE;

    //
    // Get Device address & Endpoint
    //
    dTmp = (UINT32)fpDevInfo->bDeviceAddress;
    dTmp |= (UINT32)(fpDevInfo->bIntEndpoint & 0xF) << 8;

    //
    // Set max packet size
    //
    dTmp |= (UINT32)(fpDevInfo->wIntMaxPkt) << 16;

    //
    // Set the device speed, reset the device speed bits
    //
    dTmp1 = (UINT32)fpDevInfo->bEndpointSpeed;  // 00/01/10 for HI/LO/FULL

    //
    // Assume as a high speed device
    //
    dTmp |= QH_HIGH_SPEED;  // 10b - High speed

    //
    // Check for high speed
    //
    if (dTmp1)
    {
        dTmp1 = (dTmp1 & 1) << 12;  // Bit 12 = full/low speed flag
        dTmp &= ~(QH_ENDPOINT_SPEED);
        dTmp |= dTmp1;
        //
        // Set the hub address and port number
        //
        dTmp1 = (UINT32)EHCI_GetHiSpeedHubPortNumber(fpDevInfo);
        dTmp1 = (dTmp1 << 16) | BIT10 | BIT11 | BIT12;
        fpQHInt->dEndPntCap |= dTmp1;   // Split complete Xaction
    }
    //
    // Update the endpoint characteristcs field with the data formed
    //
    fpQHInt->dEndPntCharac = dTmp;
    fpQHInt->dEndPntCap |= (BIT0 | QH_ONE_XFER);    // Interrupt schedule mask
    fpQHInt->Interval = EhciTranslateInterval(fpDevInfo->bEndpointSpeed, 
                            fpDevInfo->bPollInterval);

    //
    // Set the data toggle depending on the bDatToggle value
    //
    fpQHInt->dToken |= (UINT32)bDatToggle << 31;

//
// Fill the interrupt data qTD with relevant information
// The token field will be set so
//   Direction PID = QTD_IN_TOKEN,
//   Size = size of the data,
//   Data Toggle = bDatToggle,
//   Error Count = QTD_THREE_ERRORS,
//   Status code = QTD_ACTIVE
// The buffer pointers field will point to the EDX
//   which was before initialized to contain a DeviceRequest struc.
// The dNextqTDPtr field will point to the qTDControlSetup
// The dAltNextqTDPtr field will be set to EHCI_TERMINATE
//
    fpqTDIntData->dToken = QTD_IOC_BIT | QTD_THREE_ERRORS | QTD_ACTIVE;
	if (fpDevInfo->bIntEndpoint & BIT7) {
		fpqTDIntData->dToken |= QTD_IN_TOKEN;
	} else {
		fpqTDIntData->dToken |= QTD_OUT_TOKEN;
	}

    //
    // Set length
    //
    fpqTDIntData->dToken |= (UINT32)wLength << 16;
	
	EhciDmaMap(fpHCStruc, fpDevInfo->bIntEndpoint & BIT7, fpBuffer, wLength, 
		&BufPhyAddr, &BufferMapping);
    //
    // Update buffer pointers
    //
    EHCISetQTDBufferPointers(fpqTDIntData, BufPhyAddr, (UINT32)wLength);

    //
    // Update next & alternate next qTD pointers
    //
    fpqTDIntData->dNextqTDPtr = EHCI_TERMINATE;
    fpqTDIntData->dAltNextqTDPtr = EHCI_TERMINATE;

    //
    // Schedule the QHInterrupt to 1msec schedule
    //
    EhciAddPeriodicQh(fpHCStruc,fpQHInt);

	// Set the QH as active
	fpQHInt->bActive = TRUE;

    //
    // Now wait for interrupt transaction to be complete;
    // the EHCIProcessInterrupt will set its active flag to FALSE.
    //
	Timeout = gUsbData->wTimeOutValue * 100; // makes it number of 10 macrosecond units

	for (Count = 0; Timeout == 0 || Count < Timeout; Count++) {
		if (!(fpqTDIntData->dToken & QTD_ACTIVE)) {
			break;
		}
		FixedDelay(10);  // 60 microsec
	}
    //Status = EHCIWaitForTransferComplete(fpHCStruc, fpQHInt, fpDevInfo);

	// Remove the QH from periodic schedule
    EhciRemovePeriodicQh(fpHCStruc,(EHCI_QH*)fpQHInt);

    //
    // Check whether the QH stopped or timed out
    //
    BytesTransferred = 0;
	if (fpqTDIntData->dToken & QTD_ACTIVE) {
		USB_DEBUG (DEBUG_LEVEL_3, "EHCI Time-Out\n");
		gUsbData->dLastCommandStatusExtended |= USB_TRNSFR_TIMEOUT;
	} else if (fpqTDIntData->dToken & QTD_HALTED) {
		gUsbData->dLastCommandStatusExtended  |= USB_TRSFR_STALLED;
	} else {
		BytesTransferred = (UINT16)(wLength - ((fpqTDIntData->dToken &
								~(QTD_DATA_TOGGLE)) >> 16));
		//
		// Update the data toggle value into the mass info structure
		//
		bDatToggle = (UINT8)(((fpQHInt->dToken & QH_DATA_TOGGLE) >> 31) & 1);
		UsbUpdateDataToggle(fpDevInfo, fpDevInfo->bIntEndpoint, bDatToggle);
	}

	EhciDmaUnmap(fpHCStruc, BufferMapping);

	// Free the allocated QH and qTD
    EhciMemFree(fpHCStruc, fpQHInt, GET_MEM_BLK_COUNT((1 * sizeof(EHCI_QH)) +
                                            		  (1 * sizeof(EHCI_QTD))));

	// Service all interrupts
	EHCI_ProcessInterrupt(fpHCStruc);

    return (UINT16)BytesTransferred;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_DeactivatePolling
//
// Description: This function de-activates the polling QH for the requested
//              device. The device may be a USB keyboard or USB hub
//
// Input:       fpHCStruc   - Pointer to the HC structure
//              fpDevInfo   - Pointer to the device information structure
//
// Output:      USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_DeactivatePolling(
    HC_STRUC* fpHCStruc,
    DEV_INFO* fpDevInfo)
{
    UINT8   *fpPollED;

	if (EhciIsHalted(fpHCStruc)) {
		return USB_ERROR;
	}

    //
    // Get a pointer to the device's QH from the poll QH pointer and remove
    // the polling ED from the schedule
    //
    fpPollED    = fpDevInfo->fpPollEDPtr;
	if(!fpPollED) return USB_ERROR;

	((EHCI_QH*)fpPollED)->bActive = FALSE;

    EhciRemovePeriodicQh(fpHCStruc,(EHCI_QH*)fpPollED); 

	UsbUpdateDataToggle(fpDevInfo, fpDevInfo->bIntEndpoint, 
		(UINT8)((((EHCI_QH*)fpPollED)->dToken & QH_DATA_TOGGLE) >> 31));

    EhciMemFree(fpHCStruc, fpPollED, GET_MEM_BLK_COUNT(sizeof(EHCI_QH)+sizeof(EHCI_QTD)));
    fpDevInfo->fpPollEDPtr  = NULL;
	fpDevInfo->fpPollTDPtr	= NULL;

	if(fpDevInfo->fpPollDataBuffer) {
		EhciMemFree(fpHCStruc, fpDevInfo->fpPollDataBuffer, 
						GET_MEM_BLK_COUNT(fpDevInfo->wIntMaxPkt)); 
    	fpDevInfo->fpPollDataBuffer = 0;
	}

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCI_ActivatePolling
//
// Description: This function activates the polling QH for the requested
//              device. The device may be a USB keyboard or USB hub
//
// Input:       fpHCStruc   - Pointer to the HC structure
//              fpDevInfo   - Pointer to the device information structure
//
// Output:      USB_ERROR on error, USB_SUCCESS on success
//
// Notes:       For the keyboard device this routine allocates TDRepeat
//              also, if it is not already allocated. This routine allocate
//              a polling TD and schedule it to 8ms schedule for keyboards
//              and to 1024ms schedule for hubs.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_ActivatePolling(
    HC_STRUC* fpHCStruc,
    DEV_INFO* fpDevInfo)
{
    EHCI_QH     *fpPollED; 
    EHCI_QTD    *fpPollTD;
    UINT32      dTmp, dTmp1;

    USB_DEBUG (DEBUG_LEVEL_3, "EHCI_AP dev type %d\n", fpDevInfo->bDeviceType);

	if (EhciIsHalted(fpHCStruc)) {
		return USB_ERROR;
	}

    //
    // Allocate a QH/qTD for polling QH & qTD
    //
    fpPollED = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT(
                                    sizeof(EHCI_QH)+sizeof(EHCI_QTD)));
    if (!fpPollED)
    {
        return  USB_ERROR;  // Memory allocation error
    }

    //
    // Save the pointers in DeviceInfo structure
    //
    fpDevInfo->fpPollEDPtr  = (UINT8*)fpPollED;
    fpPollTD    = (EHCI_QTD*)((UINTN)fpPollED + sizeof(EHCI_QH));
    fpDevInfo->fpPollTDPtr  = (UINT8*)fpPollTD;

//
// Setup the polling QH
// Set the QH's dNextqTDPtr field to polling qTD and dAltqTDPtr field to
// EHCI_TERMINATE
//

    fpPollED->fpFirstqTD    = fpPollTD;
    fpPollED->dNextqTDPtr   = (UINT32)(UINTN)fpPollTD;

    //
    // Intialize the queue head
    //
    fpPollED->dAltNextqTDPtr    = EHCI_TERMINATE;
    fpPollED->dLinkPointer      = EHCI_TERMINATE;

    //
    // Set the device info pointer in the QH
    //
    fpPollED->fpDevInfoPtr  = (UINT8*)fpDevInfo;

    //
    // Get Device address & Endpoint
    //
    dTmp = ((UINT32)fpDevInfo->bDeviceAddress)  |
                    ((UINT32)(fpDevInfo->bIntEndpoint & 0xF) << 8);

    dTmp |= ((UINT32)fpDevInfo->wIntMaxPkt) << 16; // Set max packet size	//(EIP54782)

    dTmp1   = (UINT32)fpDevInfo->bEndpointSpeed;    // 00/01/10 for HI/LO/FULL

    //
    // Assume as a high speed device
    //
    dTmp |= QH_HIGH_SPEED;  // 10b - High speed

    //
    // Check for high speed
    //
    if (dTmp1)
    {
        dTmp1 = (dTmp1 & 1) << 12;  // Bit 12 = full/low speed flag
        dTmp &= ~(QH_ENDPOINT_SPEED);
        dTmp |= dTmp1;
        //
        // Set the hub address and port number
        //
        dTmp1 = (UINT32)EHCI_GetHiSpeedHubPortNumber(fpDevInfo);
        dTmp1 = (dTmp1 << 16) | (BIT10 + BIT11 + BIT12);    // Split complete Xaction
        fpPollED->dEndPntCap    |= dTmp1;
    }

    //
    // Update the endpoint characteristcs field with the data formed
    //
    fpPollED->dEndPntCharac = dTmp;

    //
    // Set a bit in interrupt mask
    //
    fpPollED->dEndPntCap    |= (BIT0 + QH_ONE_XFER);
    fpPollED->Interval = EhciTranslateInterval(fpDevInfo->bEndpointSpeed, 
                                fpDevInfo->bPollInterval);

    //
    // Set the data toggle
    //
	fpPollED->dToken |= (UINT32)(UsbGetDataToggle(fpDevInfo, fpDevInfo->bIntEndpoint) << 31);

//
// Fill the polling qTD with relevant information
// The token field will be set so
//   Direction PID = QTD_IN_TOKEN,
//   Size = size of the data,
//   Data Toggle = QTD_DATA0_TOGGLE,
//   Error Count = QTD_THREE_ERRORS,
//   Status code = QTD_ACTIVE
// The buffer pointers field will point to the fpBuffer buffer
//   which was before initialized to contain a DeviceRequest struc.
// The dNextqTDPtr field will point to the qTDControlSetup
// The dAltNextqTDPtr field will be set to EHCI_TERMINATE
//
    fpPollTD->dToken = QTD_IN_TOKEN |
                    QTD_IOC_BIT |
                    QTD_THREE_ERRORS |
                    QTD_ACTIVE;
    //
    // Set length
    //
    fpPollTD->dToken |= (UINT32)fpDevInfo->wIntMaxPkt << 16;
    fpDevInfo->fpPollDataBuffer = EhciMemAlloc(fpHCStruc, 
									GET_MEM_BLK_COUNT(fpDevInfo->wIntMaxPkt)); 
	ASSERT(fpDevInfo->fpPollDataBuffer);

    //
    // Update buffer pointers
    //
    EHCISetQTDBufferPointers(fpPollTD,
                fpDevInfo->fpPollDataBuffer, fpDevInfo->wIntMaxPkt);		//(EIP54782)
                //&fpPollED->aDataBuffer[0], sizeof(fpPollED->aDataBuffer));
    //
    // Update next & alternate next qTD pointers
    //
    fpPollTD->dNextqTDPtr = EHCI_TERMINATE;
    fpPollTD->dAltNextqTDPtr = EHCI_TERMINATE;

    EhciAddPeriodicQh(fpHCStruc,fpPollED); 

	fpPollED->bCallBackIndex = USB_InstallCallBackFunction(EhciPollingTDCallback);
	fpPollED->bActive = TRUE;

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIDisableKeyRepeat
//
// Description: This function disables the keyboard repeat rate logic by
//              enabling the repeat TD
//
// Input:       fpHCStruc   - Pointer to the HCStruc structure
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_DisableKeyRepeat (
	HC_STRUC	*HcStruc
)
{
	EHCI_QH		*RepeatQh = HcStruc->stDescPtrs.fpEHCIDescPtrs->fpQHRepeat;

	if (RepeatQh) {
		RepeatQh->dTokenReload = ((UINT32)8 << 16) | QTD_IN_TOKEN | QTD_ONE_ERROR;
		RepeatQh->bActive = FALSE;
	}
    return  USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:    EHCI_EnableKeyRepeat
//
// Description: This function disables the keyboard repeat rate logic by
//              enabling the repeat TD
//
// Parameters:  fpHCStruc   Pointer to the HCStruc structure
//
// Output:      USB_SUCCESS or USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCI_EnableKeyRepeat (
	HC_STRUC* HcStruc
)
{
	EHCI_QH		*RepeatQh = HcStruc->stDescPtrs.fpEHCIDescPtrs->fpQHRepeat;
	EHCI_QTD    *RepeatQtd;

	if (RepeatQh == NULL) {
		return USB_SUCCESS;
	}

	if ((RepeatQh->dTokenReload & QTD_ACTIVE) == 0) {
		RepeatQtd = HcStruc->stDescPtrs.fpEHCIDescPtrs->fpqTDRepeat;
		RepeatQh->dTokenReload = ((UINT32)8 << 16) | QTD_IN_TOKEN | 
								QTD_ONE_ERROR | QTD_IOC_BIT | QTD_ACTIVE;

		// Update buffer pointers
		EHCISetQTDBufferPointers(RepeatQtd,
						&RepeatQh->aDataBuffer[0], 8);
		
		// Re-init the QH pointers
		RepeatQh->dCurqTDPtr = 0;
		RepeatQh->dAltNextqTDPtr = EHCI_TERMINATE;
		RepeatQh->dNextqTDPtr = (UINT32)(UINTN)RepeatQtd;
		
		//
		// Restart the qTD
		//
		RepeatQh->dToken = 0;
		RepeatQtd->dToken = RepeatQh->dTokenReload;

		RepeatQh->bActive = TRUE;
	}
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIResetHC
//
// Description: This function resets the EHCI controller
//
// Input:   Pointer to the HCStruc structure
//
// Output:  USB_SUCCESS     HC successfully reset
//          USB_ERROR       Error, error log is updated
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIResetHC(HC_STRUC* fpHCStruc)
{
    UINT8   count;

    if (fpHCStruc->DebugPort) return USB_SUCCESS;
    //
    // Check HC is halted: attempting to reset an actively running HC will
    // result in undefined behavior.
    //
    if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_HCHALTED)
    {
        //
        // Issue reset
        //
        EhciSetOpReg(fpHCStruc, EHCI_USBCMD, EHCI_HCRESET);

        //
        // EHCI_HCRESET bit is set to zero by the Host Controller when the reset
        // process is complete.
        //
        for (count = 0; count < 500; count++) {
            if (!(EhciReadOpReg(fpHCStruc, EHCI_USBCMD) & EHCI_HCRESET)) {
                return  USB_SUCCESS;
            }
            FixedDelay(100);     // 100 us delay
        }
    }

    //
    // Error - HC reset failed
    //
    USBLogError(USB_ERR_HC_RESET_FAILED);

    return  USB_ERROR;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIInitializesPeriodicSchedule
//
// Description: This function initializes the periodic schedules for the
//              EHCI host controller
//
// Input:       fpHCStruc   - HCStruc for the controller
//              dMemAddr    - Membase address
//
// Output:      USB_ERROR   On error
//              USB_SUCCESS On success
//
// Notes:   This routine creates 8ms and 32ms schedules
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIInitializePeriodicSchedule(
    HC_STRUC* fpHCStruc,
    UINT32 dMemBase)
{
    UINT8           *fpPtr;
    EHCI_DESC_PTRS  *fpDescPtr;
    //
    // Allocate descriptor structure and fill it in HCStruc
    //
    fpDescPtr = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT_STRUC(EHCI_DESC_PTRS));
    if (fpDescPtr == NULL) {
        USB_DEBUG(DEBUG_LEVEL_4, "EHCI Descriptor struc alloc failed. %d \n",
                                    GET_MEM_BLK_COUNT_STRUC(EHCI_DESC_PTRS));
        return USB_ERROR;
    }

    //
    // Save the value in the HC struc
    //
    fpHCStruc->stDescPtrs.fpEHCIDescPtrs = fpDescPtr;

    // Allocate QH/qTD for PeriodicQh
    fpPtr = EhciMemAlloc(fpHCStruc, GET_MEM_BLK_COUNT((1 * sizeof(EHCI_QH))+
													  (0 * sizeof(EHCI_QTD))));
    if (fpPtr == NULL) {
        USB_DEBUG(DEBUG_LEVEL_4, "Schedule of EHCI QH alloc failed.\n");
        return USB_ERROR;
    }
	
    // Save the 1 QH in appropriate location
    fpDescPtr->PeriodicQh = (EHCI_QH*)fpPtr;

    EHCIInitializeQueueHead(fpDescPtr->PeriodicQh);
    fpDescPtr->PeriodicQh->dNextqTDPtr = EHCI_TERMINATE;
    fpDescPtr->PeriodicQh->dAltNextqTDPtr = EHCI_TERMINATE;
    fpDescPtr->PeriodicQh->dEndPntCharac = ((UINT32)0x40 << 16) + QH_HIGH_SPEED;
    fpDescPtr->PeriodicQh->dLinkPointer      = EHCI_TERMINATE;
    fpDescPtr->PeriodicQh->dToken = QTD_HALTED;
    fpDescPtr->PeriodicQh->Interval = 1;

    EhciAddPeriodicQh(fpHCStruc, fpDescPtr->PeriodicQh);

    return  USB_SUCCESS;
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIInitializeQueueHead
//
// Description: This function initializes the queue head with default values
//
// Input:       fpQH    Pointer to queue head
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCIInitializeQueueHead(EHCI_QH *fpQH)
{
    fpQH->dNextqTDPtr       = 1;
    fpQH->dAltNextqTDPtr    = 1;
    fpQH->dCurqTDPtr        = 0;

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
//
// Procedure:   EHCIStartPeriodicSchedule
//
// Description: This function starts the periodic schedule for the
//              EHCI host controller
//
// Input:       Pointer to HCStruc
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIStartPeriodicSchedule(HC_STRUC* fpHCStruc)
{
    UINT16  i;
    //
    // Start periodic schedule
    //
    EhciSetOpReg(fpHCStruc, EHCI_USBCMD, EHCI_PER_SCHED_ENABLE);

    //
    // Make sure the HC started the schedules
    //
    for (i = 0; i < 1000; i++) {
        if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_PER_SCHED_STATUS) {
            break;
        }
        FixedDelay(10);
    }
    if (!(EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_PER_SCHED_STATUS)) {
        return USB_ERROR;
    }
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIStopPeriodicSchedule
//
// Description: This function stops the periodic schedule for the
//              EHCI USB host controller
//
// Input:       HCStruc for the controller
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIStopPeriodicSchedule(HC_STRUC* fpHCStruc)
{
    UINT16  i;
    //
    // Stop periodic schedule
    //
    EhciClearOpReg(fpHCStruc, EHCI_USBCMD, EHCI_PER_SCHED_ENABLE);

    //
    // Make sure the HC stopped the schedules
    //
    for (i = 0; i < 1000; i++) {
        if (!(EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_PER_SCHED_STATUS)) {
            break;
        }
        FixedDelay(10);
    }
    if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_PER_SCHED_STATUS) {
        return USB_ERROR;
    }
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIStartAsyncSchedule
//
// Description: This function starts the asynchronous schedule
//
// Input:       Pointer to HCStruc
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCIStartAsyncSchedule(HC_STRUC* fpHCStruc)
{
    //
    // Start the Async schedule
    //
    EhciSetOpReg(fpHCStruc, EHCI_USBCMD, EHCI_ASYNC_SCHED_ENABLE);

    //
    // Make sure the HC started the async. execution
    //
    for (;;)
    {
        if (EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_ASYNC_SCHED_STATUS)
        {
            break;
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIStopAsyncSchedule
//
// Description: This function stops the asynchronous transfer and sets the
//              asynchronous pointer to null
//
// Input:       Pointer to HCStruc
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCIStopAsyncSchedule(HC_STRUC* fpHCStruc)
{
	if (!(EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_ASYNC_SCHED_STATUS)) {
		return;
	}

    //
    // Stop periodic schedule
    //
    EhciClearOpReg(fpHCStruc, EHCI_USBCMD, EHCI_ASYNC_SCHED_ENABLE);

    //
    // Make sure the HC stopped the async. execution
    //
    for (;;)
    {
        if (!(EhciReadOpReg(fpHCStruc, EHCI_USBSTS) & EHCI_ASYNC_SCHED_STATUS))
        {
            break;
        }
    }
}


#if EHCI_ASYNC_BELL_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIRemoveQHFromAsyncList
//
// Description: This function stops the asynchronous transfer and sets the
//      asynchronous pointer to null
//
// Input:   Pointer to HCStruc
//          Pointer to the Queue head that has to be removed
//          from the asynchronous schedule
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCIRemoveQHFromAsyncList(
    HC_STRUC    *fpHCStruc,
    EHCI_QH     *fpQH
)
{
    //
    // Stop the Async transfer
    //
    EHCIStopAsyncSchedule(fpHCStruc);

    //
    // Remove the queue head from the Async list
    //
    gUsbData->fpQHAsyncXfer->dLinkPointer  = fpQH->dLinkPointer;

    //
    // Pointer is advanced. The queue head is totally removed from the list!
    //
}
#endif


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIAddPeriodicQH
//
// Description: This function adds a QH to the frame list
//
// Input:   Pointer to the QH to be added
//          Absolute pointer to the frame list
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EhciAddPeriodicQh(
    HC_STRUC    *HcStruc,
    EHCI_QH     *NewQh
)
{
    UINT16      Index;
    UINT32      *PrevPtr;
    UINT32      LinkPtr;
    EHCI_QH     *Qh;

    if (NewQh == NULL || NewQh->Interval == 0) {
        return USB_ERROR;
    }

    for (Index = 0; Index < HcStruc->wAsyncListSize; Index += NewQh->Interval) {
        PrevPtr = &HcStruc->fpFrameList[Index];
        LinkPtr = *PrevPtr; 
        while (!(LinkPtr & EHCI_TERMINATE)){
            Qh = (EHCI_QH*)(LinkPtr & EHCI_POINTER_MASK);
            if (Qh->Interval <= NewQh->Interval) {
                break;
            }
            PrevPtr = &Qh->dLinkPointer;
            LinkPtr = *PrevPtr; 
        }

        if (!(LinkPtr & EHCI_TERMINATE) && (Qh == NewQh)) {
            continue;
        }
        NewQh->dLinkPointer = *PrevPtr;
        *PrevPtr = (UINT32)((UINTN)NewQh | EHCI_QUEUE_HEAD);
    }

    return USB_SUCCESS;
} 


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EhciRemovePeriodicQh
//
// Description: This function removes a QH from the frame list
//
// Input:   Pointer to the QH to be added
//          Absolute pointer to the frame list
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EhciRemovePeriodicQh (
    HC_STRUC    *HcStruc,
    EHCI_QH     *RetiredQh
)
{
    UINT16      Index;
    UINT32      *PrevPtr;
    UINT32      LinkPtr;
    EHCI_QH     *Qh;

    if (RetiredQh == NULL || RetiredQh->Interval == 0) {
        return USB_ERROR;
    }

    EHCIStopPeriodicSchedule(HcStruc);

    for (Index = 0; Index < HcStruc->wAsyncListSize; Index += RetiredQh->Interval) {
        PrevPtr = &HcStruc->fpFrameList[Index];
        LinkPtr = *PrevPtr; 
 
        while (!(LinkPtr & EHCI_TERMINATE)){
            Qh = (EHCI_QH*)(LinkPtr & EHCI_POINTER_MASK);
            if (Qh == RetiredQh) {
                break;
            }
            PrevPtr = &Qh->dLinkPointer;
            LinkPtr = *PrevPtr; 
        }

        if (LinkPtr & EHCI_TERMINATE) {
            continue;
        }
        *PrevPtr = RetiredQh->dLinkPointer;
    }

    RetiredQh->dLinkPointer = EHCI_TERMINATE;

    EHCIStartPeriodicSchedule(HcStruc);

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
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
    wBufSize = (UINT16)((dSize > MAX_EHCI_DATA_SIZE) ? MAX_EHCI_DATA_SIZE : dSize);

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
// Procedure:   EHCIWaitForTransferComplete
//
// Description: This function waits until the requested QH completes or
//              the transaction time-out
//
// Input:   fpHCStruc   - Pointer to HCStruc of the host controller
//          fpQH        - Pointer to the QH which has to be completed
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
EHCIWaitForTransferComplete(
    HC_STRUC    *fpHCStruc,
    EHCI_QH     *fpQH,
    DEV_INFO*   fpDevInfo

)
{
    UINT32 dCount ;
    UINT32 countLimit = gUsbData->wTimeOutValue * 100; // makes it number of macrosecond units
	UINT16 Status = USB_ERROR;

    //
    // Check status change loop iteration
    //
    for(dCount = 0; !countLimit || dCount < countLimit; dCount++)
    {
        EHCIProcessQH(fpHCStruc, fpQH);
        if(fpQH->bActive == FALSE) {
            Status = USB_SUCCESS;
			break;
        }
        FixedDelay(10);  // 60 microsec
    }

	if(fpQH->bActive == TRUE) {
		// Set the QH as in-active
		fpQH->bActive = FALSE;
		Status = USB_ERROR;
    	USB_DEBUG (DEBUG_LEVEL_3, "EHCI Time-Out\n");
	}

	// Service all interrupts
	EHCI_ProcessInterrupt(fpHCStruc);
	
    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIProcessQH
//
// Description: This function whether all the TD's in the QH is completed
//
// Input:       fpQH    - Pointer to the QH which has to be completed
//
// Output:      USB_ERROR   On error
//              USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EHCIProcessQH(
	HC_STRUC	*fpHCStruc,
	EHCI_QH		*fpQH
)
{
    EHCI_QTD *fpQTD = fpQH->fpFirstqTD;

    for (;;) {
        //
        // Check whether the qTD is active, if so. Exit!
        //
        if (fpQTD->dToken & QTD_ACTIVE) {
            return  USB_ERROR;
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
			break;
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
            break;
        }

        //
        // More qTDs are in the list. Process next qTD
        //
        fpQTD = (EHCI_QTD*)(UINTN)fpQTD->dNextqTDPtr;
    }

	if (fpQH->bCallBackIndex) {
		if (gUsbData->aCallBackFunctionTable[fpQH->bCallBackIndex-1]) {
			(*gUsbData->aCallBackFunctionTable[fpQH->bCallBackIndex-1])
							(fpHCStruc, (DEV_INFO*)fpQH->fpDevInfoPtr, 
							(UINT8*)fpQH, NULL);
		}
	}

	return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIProcessPeriodicList
//
// Description: This function parses through the periodic list to find
//              completed qTD.  If a qTD is done it will call its associated
//              call back function (in device info structure) and restarts
//              the polling qTD
//
// Input:   fpHCStruc   - Pointer to HCStruc of the host controller
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
EHCIProcessPeriodicList (
	HC_STRUC	*HcStruc
)
{
	UINT32		LinkPtr;
    EHCI_QH     *Qh = NULL;

    //
    // Get the first entry in the periodic list. This QH list will link to all
    // the periodic QH's
    //

	LinkPtr = HcStruc->fpFrameList[0];
	while (!(LinkPtr & EHCI_TERMINATE)) {
		Qh = (EHCI_QH*)(LinkPtr & EHCI_POINTER_MASK);

		// Check whether this QH is actived
		if (Qh->bActive == TRUE) {			
			EHCIProcessQH(HcStruc, Qh);
		}

		LinkPtr = Qh->dLinkPointer;
	}
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EhciPollingTDCallback
//
// Description: This function is called when a polling TD from the TD pool 
//				completes an interrupt transaction to its assigned device.
//  			This routine should process any data in the TD's data buffer, 
//				handle any errors, and then copy the TD's CSReloadValue field 
//				into its control status field to put the TD back into service.
//
// Input:       HcStruc   Pointer to the HCStruc structure
//				DevInfo   NULL (pDevInfo is not valid)
//              QueueHead Pointer to the QH that completed
//  			Buffer    Not used
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EhciPollingTDCallback(
    HC_STRUC	*HcStruc,
    DEV_INFO	*DevInfo,
    UINT8		*QueueHead,
    UINT8		*Buffer
)
{
	EHCI_QH*	PollQh = (EHCI_QH*)QueueHead;
	EHCI_QTD    *PollQtd = PollQh->fpFirstqTD;

	// Update datat toggle value
	UsbUpdateDataToggle(DevInfo, DevInfo->bIntEndpoint, 
		(UINT8)((PollQh->dToken & QH_DATA_TOGGLE) >> 31));

	if (!(PollQh->dToken & QTD_STATUS_FIELD)) {
		if(gUsbData->aCallBackFunctionTable[DevInfo->bCallBackIndex-1]) {
			(*gUsbData->aCallBackFunctionTable[DevInfo->bCallBackIndex-1])
					(HcStruc, DevInfo, (UINT8*)PollQtd, DevInfo->fpPollDataBuffer);
		}
	}

    // Keep the PID code bit in the qTD
    PollQtd->dToken &= QTD_DIRECTION_PID;
    
    //
    // Set length
    //
	PollQtd->dToken |= (UINT32)DevInfo->wIntMaxPkt << 16;

    //
    // Update buffer pointers
    //
	EHCISetQTDBufferPointers(PollQtd,
					DevInfo->fpPollDataBuffer, DevInfo->wIntMaxPkt);

    //
    // Re-init the QH pointers
    //
    PollQh->dToken &= QH_DATA_TOGGLE; 
    PollQh->dCurqTDPtr = 0;
    PollQh->dAltNextqTDPtr = EHCI_TERMINATE;
    PollQh->dBufferPtr0 = 0;
    PollQh->dBufferPtr1 = 0;
    PollQh->dBufferPtr2 = 0;
    PollQh->dBufferPtr3 = 0;
    PollQh->dBufferPtr4 = 0;
    //
    // Update next & alternate next qTD pointers
    //
    PollQtd->dNextqTDPtr = EHCI_TERMINATE;
    PollQtd->dAltNextqTDPtr = EHCI_TERMINATE;
    PollQh->dNextqTDPtr = (UINT32)(UINTN)PollQtd;

    //
    // Restart the qTD
    //
    PollQtd->dToken |= (QTD_IOC_BIT | QTD_THREE_ERRORS | QTD_ACTIVE);
	PollQh->bActive = TRUE;
	
    return  USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EHCIRepeatTDCallback
//
// Description: This function is called when qTdRepeat completes
//              a transaction.  This qTD runs a dummy interrupt transaction
//              to a non-existant device address for the purpose of
//              generating a periodic timeout interrupt which in turn
//              is used to generate keyboard repeat or update LED status.
//
// Input:       HcStruc   Pointer to the HCStruc structure
//				DevInfo   NULL (pDevInfo is not valid)
//              QueueHead Pointer to the QH that completed
//  			Buffer    Not used
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
EhciRepeatTDCallback(
	HC_STRUC	*HcStruc,
	DEV_INFO	*DevInfo,
	UINT8		*QueueHead,
	UINT8		*Buffer
)
{
	EHCI_QH		*RepeatQh = (EHCI_QH*)QueueHead;
	EHCI_QTD	*RepeatQtd = RepeatQh->fpFirstqTD;

#if USB_DEV_KBD
	USBKBDPeriodicInterruptHandler(HcStruc);
#endif

	if (RepeatQh->dTokenReload & QTD_ACTIVE) {
		//
		// Update buffer pointers
		//
		EHCISetQTDBufferPointers(RepeatQtd,
						&RepeatQh->aDataBuffer[0], 8);

		//
		// Re-init the QH pointers
		//
        RepeatQh->dToken &= QH_DATA_TOGGLE;
        RepeatQh->dCurqTDPtr = 0;
        RepeatQh->dAltNextqTDPtr = EHCI_TERMINATE;
        RepeatQh->dBufferPtr0 = 0;
        RepeatQh->dBufferPtr1 = 0;
        RepeatQh->dBufferPtr2 = 0;
        RepeatQh->dBufferPtr3 = 0;
        RepeatQh->dBufferPtr4 = 0;
		RepeatQh->dNextqTDPtr = (UINT32)(UINTN)RepeatQtd;

	    //
	    // Restart the qTD
	    //
		RepeatQh->dToken = 0;
		RepeatQtd->dToken = RepeatQh->dTokenReload;
		
		RepeatQh->bActive = TRUE;
	}

    return  USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EhciIsHalted
//
// Description: This function check whether HC is halted.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
EhciIsHalted (
	HC_STRUC	*HcStruc
)
{
	return (EhciReadOpReg(HcStruc, EHCI_USBSTS) & EHCI_HCHALTED) == EHCI_HCHALTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   EhciTranslateInterval
//
// Description: This function calculates the polling rate in frames unit.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
EhciTranslateInterval(
    UINT8   Speed,
    UINT8   Interval
)
{
    UINT8   BitCount = 0;
    UINT16  PollingRate = 0;

    if (Speed == USB_PORT_STAT_DEV_HISPEED) {
        // For high-speed interrupt endpoints, the Interval value must be 
        // from 1 to 16
        ASSERT(Interval >= 1 && Interval <= 16);
 
        PollingRate = (1 << (Interval - 1)) >> 3;
        return PollingRate != 0 ? PollingRate : 1;
    }

    // For full-/low-speed interrupt endpoints, the Interval value should 
    // be from 1 to 255
    ASSERT(Interval >= 1 && Interval <= 255);

    for (BitCount = 0; Interval != 0; BitCount++) {
        Interval >>= 1;
    }
    return (1 << (BitCount - 1));
}

//==========================================================================

UINT32
EhciReadPciReg(
	HC_STRUC	*HcStruc,
	UINT32		Offset
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	return ReadPCIConfig(HcStruc->wBusDevFuncNum, Offset);
#else
	EFI_STATUS	Status;
	UINT32		Data = 0;

	Status = HcStruc->PciIo->Pci.Read (
			 HcStruc->PciIo,
			 EfiPciIoWidthUint32,
			 Offset,
			 1,
			 &Data);
	ASSERT_EFI_ERROR(Status);
	return Data;
#endif
}


VOID
EhciWritePciReg(
	HC_STRUC	*HcStruc,
	UINT32		Offset,
	UINT32		Data
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	DwordWritePCIConfig(HcStruc->wBusDevFuncNum, Offset, Data);
	return;
#else
	EFI_STATUS	Status;

	Status = HcStruc->PciIo->Pci.Write (
			 HcStruc->PciIo,
			 EfiPciIoWidthUint32,
			 Offset,
			 1,
			 &Data);
	ASSERT_EFI_ERROR(Status);
	return;
#endif
}

UINT32
EhciReadHcMem(
	HC_STRUC	*HcStruc,
	UINT32		Offset
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	return DwordReadMem((UINT32)HcStruc->BaseAddress, Offset);
#else
	EFI_STATUS	Status;
	UINT32		Data = 0;

	Status = HcStruc->PciIo->Mem.Read (
			 HcStruc->PciIo,
			 EfiPciIoWidthUint32,
			 0,
			 Offset,
			 1,
			 &Data);
	ASSERT_EFI_ERROR(Status);
	return Data;
#endif
}

VOID
EhciWriteHcMem(
	HC_STRUC	*HcStruc,
	UINT32		Offset,
	UINT32		Data
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	DwordWriteMem((UINT32)HcStruc->BaseAddress, Offset, Data);
	return;
#else
	EFI_STATUS	Status;

	Status = HcStruc->PciIo->Mem.Write (
			 HcStruc->PciIo,
			 EfiPciIoWidthUint32,
			 0,
			 Offset,
			 1,
			 &Data);
	ASSERT_EFI_ERROR(Status);
	return;
#endif
}

UINT32
EhciReadOpReg(
	HC_STRUC	*HcStruc,
	UINT32		Offset
)
{
	return EhciReadHcMem(HcStruc, HcStruc->bOpRegOffset + Offset);
}

VOID
EhciWriteOpReg(
	HC_STRUC	*HcStruc,
	UINT32		Offset,
	UINT32		Data
)
{
	EhciWriteHcMem(HcStruc, HcStruc->bOpRegOffset + Offset, Data);
	return;
}

VOID
EhciSetOpReg(
	HC_STRUC	*HcStruc,
	UINT32		Offset,
	UINT32		Bit
)
{
	UINT32	Data;

	Data = EhciReadOpReg(HcStruc, Offset) | Bit;
	EhciWriteOpReg(HcStruc, Offset, Data);
	return;
}

VOID
EhciClearOpReg(
	HC_STRUC	*HcStruc,
	UINT32		Offset,
	UINT32		Bit
)
{
	UINT32	Data;

	Data = EhciReadOpReg(HcStruc, Offset) & ~Bit;
	EhciWriteOpReg(HcStruc, Offset, Data);
	return;
}

UINT32
EhciReadDebugReg(
	HC_STRUC	*HcStruc,
	UINT8		BarIndex,
	UINT32		Offset
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	UINT8  BarOffset[6] = {0x10, 0x14, 0x18, 0x1C, 0x20, 0x24};
	UINT32 DebugPortsMem;

    DebugPortsMem = EhciReadPciReg(HcStruc, BarOffset[BarIndex]);
	return DwordReadMem(DebugPortsMem, Offset);
#else
	EFI_STATUS	Status;
	UINT32		Data = 0;

	Status = HcStruc->PciIo->Mem.Read (
			 HcStruc->PciIo,
			 EfiPciIoWidthUint32,
			 BarIndex,
			 Offset,
			 1,
			 &Data);
	ASSERT_EFI_ERROR(Status);
	return Data;
#endif
}

VOID*
EhciMemAlloc(
	HC_STRUC	*HcStruc,
	UINT16		NumBlks
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	return USB_MemAlloc(NumBlks);
#else
	UINT32	SavedMemPages = gUsbData->MemPages;
	UINT8	*SavedMemPool = gUsbData->fpMemBlockStart;
	UINT32	SavedMemBlkStsBytes = gUsbData->MemBlkStsBytes;
	UINT32	*SavedMemBlsSts = gUsbData->aMemBlkSts;
	VOID*	MemPtr;

	gUsbData->MemPages = HcStruc->MemPoolPages;
	gUsbData->fpMemBlockStart = HcStruc->MemPool;
	gUsbData->MemBlkStsBytes = HcStruc->MemBlkStsBytes;
	gUsbData->aMemBlkSts = HcStruc->MemBlkSts;

	MemPtr = USB_MemAlloc(NumBlks);

	gUsbData->MemPages = SavedMemPages;
	gUsbData->fpMemBlockStart = SavedMemPool;
	gUsbData->MemBlkStsBytes = SavedMemBlkStsBytes;
	gUsbData->aMemBlkSts = SavedMemBlsSts;

	return MemPtr;
#endif
}

VOID
EhciMemFree(
	HC_STRUC	*HcStruc,
	VOID*		MemPtr,
	UINT16		NumBlks
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	USB_MemFree(MemPtr, NumBlks);
#else
	UINT32	SavedMemPages = gUsbData->MemPages;
	UINT8	*SavedMemPool = gUsbData->fpMemBlockStart;
	UINT32	SavedMemBlkStsBytes = gUsbData->MemBlkStsBytes;
	UINT32	*SavedMemBlsSts = gUsbData->aMemBlkSts;

	gUsbData->MemPages = HcStruc->MemPoolPages;
	gUsbData->fpMemBlockStart = HcStruc->MemPool;
	gUsbData->MemBlkStsBytes = HcStruc->MemBlkStsBytes;
	gUsbData->aMemBlkSts = HcStruc->MemBlkSts;

	USB_MemFree(MemPtr, NumBlks);

	gUsbData->MemPages = SavedMemPages;
	gUsbData->fpMemBlockStart = SavedMemPool;
	gUsbData->MemBlkStsBytes = SavedMemBlkStsBytes;
	gUsbData->aMemBlkSts = SavedMemBlsSts;

	return;
#endif
}

UINT8
EhciDmaMap(
	HC_STRUC	*HcStruc,
	UINT8		Direction,
	UINT8		*BufferAddr,
	UINT32		BufferSize,
	UINT8		**PhyAddr,
	VOID		**Mapping
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
	*PhyAddr = BufferAddr;
#else
	EFI_PCI_IO_PROTOCOL_OPERATION	Operation;
	EFI_PHYSICAL_ADDRESS	Addr;
	EFI_STATUS	Status;
	UINTN		Bytes = BufferSize;

	if (Direction & BIT7) {
		Operation = EfiPciIoOperationBusMasterWrite;
	} else {
		Operation = EfiPciIoOperationBusMasterRead;
	}

	Status = HcStruc->PciIo->Map (
				HcStruc->PciIo,
				Operation,
				BufferAddr,
				&Bytes,
				&Addr,
				Mapping
				);
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status) || Bytes != BufferSize) {
		return USB_ERROR;
	}

	*PhyAddr = (UINT8*)Addr;
#endif
	return USB_SUCCESS;
}

UINT8
EhciDmaUnmap(
	HC_STRUC	*HcStruc,
	VOID		*Mapping
)
{
#if !USB_RUNTIME_DRIVER_IN_SMM

	EFI_STATUS	Status;

	Status = HcStruc->PciIo->Unmap(HcStruc->PciIo, Mapping);
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return USB_ERROR;
	}
#endif
	return USB_SUCCESS;
}


//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2009, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
