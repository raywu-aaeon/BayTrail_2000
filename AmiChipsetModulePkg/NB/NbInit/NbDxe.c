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

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        NbDxe.C
//
// Description: This file contains code for Template Northbridge initialization
//              in the DXE stage
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <VlvAccess.h>	// (P051313A+)
#include <Efi.h>
#include <Token.h>
#include <DXE.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Library/UefiLib.h>	// (P051313A+)
#include <Nb.h>
#include <Library/NbPolicy.h>
//EIP201856>>
#include <PciBus.h>
#if CSM_SUPPORT
#include <Protocol/LegacyBiosExt.h>
#endif

VOID		*mCsmOpromPolicyRegs;
//EIP201856<<

// Produced Protocols

// GUID Definitions

// Portable Constants

// Function Prototypes

// Function Definition
// (P051313A+)>>
VOID ReadyToBootFunction(
    EFI_EVENT                 Event,
    VOID                      *Context
);
// (P051313A+)<<

//EIP201856>>
#if defined CSM_SUPPORT && CSM_SUPPORT == 1
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   VgaOpRomCallback
//
// Description: Determine if execute this PCI VGA option Rom.
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID VgaOpRomCallback (
	IN EFI_EVENT    Event,
	IN VOID         *Context
)
{
    EFI_STATUS                  Status;
    EFI_HANDLE                  Handle;
    UINTN                       Size = sizeof(EFI_HANDLE);
    CSM_PLATFORM_POLICY_DATA    *CsmOpromPolicyData;
    EFI_PCI_IO_PROTOCOL         *PciIo;
    PCI_DEV_INFO                *Device;

    TRACE((-1, "NbDxe: VgaOpRomCallback() Entry\n"));

    Status = pBS->LocateHandle(ByRegisterNotify, NULL, mCsmOpromPolicyRegs, &Size, &Handle);
    if (EFI_ERROR(Status)) return;

    Status = pBS->HandleProtocol(Handle, &gOpromStartEndProtocolGuid, &CsmOpromPolicyData);
    if (EFI_ERROR(Status)) return;

    if(CsmOpromPolicyData == NULL) return; //post-process OpROM callback
    if(CsmOpromPolicyData->PciIo == NULL) return; // OEM Service ROM

    PciIo = CsmOpromPolicyData->PciIo;
    Device = (PCI_DEV_INFO*)PciIo;
    if ((Device->Type == tPciDevice) && \
    	(Device->Class.BaseClassCode == PCI_CL_DISPLAY) && \
    	(Device->Address.Addr.Bus != 0)) {
    	CsmOpromPolicyData->ExecuteThisRom = FALSE;  //this attritube default is TRUE
    	TRACE((-1, "NbDxe: ExecuteThisRom is setted FALSE.\n"));
    }

    TRACE((-1, "NbDxe: VgaOpRomCallback() Exit\n"));
}
#endif // CSM_SUPPORT
//EIP201856<<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbDxeInit
//
// Description: This function is the entry point for this DXE. This function
//              initializes the chipset NB before PCI Bus enumeration.
//
// Input:       ImageHandle - Image handle
//              SystemTable - Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
NbDxeInit(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS          Status = EFI_SUCCESS;
//EIP201856>>
    NB_SETUP_DATA       VlvPolicyData;
    EFI_EVENT		CsmOpromPolicyEvent = NULL;
//EIP201856<<
    EFI_EVENT           ReadyToBootEvent;

    InitAmiLib(ImageHandle, SystemTable);

    //Report Progress code
    PROGRESS_CODE(DXE_NB_INIT);

//EIP201856>>
    // Get the value of the NB Setup data.
    GetNbSetupData((VOID*)pRS, &VlvPolicyData, FALSE);
//EIP201856<<

    Status = NbDxeBoardInit(ImageHandle, SystemTable);

//EIP201856>>
#if defined CSM_SUPPORT && CSM_SUPPORT == 1
    if (VlvPolicyData.PrimaryDisplay == 0) {
        Status = RegisterProtocolCallback (
       			&gOpromStartEndProtocolGuid,
            		VgaOpRomCallback,
            		NULL,
            		&CsmOpromPolicyEvent,
            		&mCsmOpromPolicyRegs
            		);
    }
#endif // CSM_SUPPORT
//EIP201856<<
        
    // (P051313A+)>>
    // Create a ReadyToBoot Event
    Status = EfiCreateEventReadyToBootEx (
                                      TPL_CALLBACK,
                                      ReadyToBootFunction,
                                      NULL,
                                      &ReadyToBootEvent
                                      );
    // (P051313A+)<<

    if (Status != EFI_SUCCESS) {
      //Report Error code
      ERROR_CODE (DXE_NB_ERROR, EFI_ERROR_MAJOR);
    }
    return Status;
}

// (P051313A+)>>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: ReadyToBootFunction
//
// Description: The function called for Ready To Boot.
//
// Input:       Event   - Watchdog event
//              Context - Context pointer
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ReadyToBootFunction(
    EFI_EVENT                 Event,
    VOID                      *Context
)
{
		UINT32 	Data = 0xffffffff;
		UINT32 	ZeroData = 0;

//EIP141429 >>
		if((E_F_SEGMENT_LOCK == 2) || (E_F_SEGMENT_LOCK == 3)) {
			MsgBus32And(VLV_BUNIT, BUNIT_BMISC, ZeroData, (UINT32) ~B_BMISC_RFSDRAM );
		} else
			MsgBus32Or(VLV_BUNIT, BUNIT_BMISC, Data, (UINT32) B_BMISC_RFSDRAM );

		if((E_F_SEGMENT_LOCK == 1) || (E_F_SEGMENT_LOCK == 3)) {
			MsgBus32And(VLV_BUNIT, BUNIT_BMISC, ZeroData, (UINT32) ~B_BMISC_RESDRAM );
		} else
			MsgBus32Or(VLV_BUNIT, BUNIT_BMISC, Data, (UINT32) B_BMISC_RESDRAM );
//EIP141429 <<

    return;
}
// (P051313A+)<<

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
