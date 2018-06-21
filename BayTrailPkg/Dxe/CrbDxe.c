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
// Revision History
// ----------------
// $Log: $
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        CRBDXE.c
//
// Description: This file contains code for Chipset Reference Board Template
//              initialization in the DXE stage
//
//<AMI_FHDR_END>
//*************************************************************************

//---------------------------------------------------------------------------
// Include(s)
//---------------------------------------------------------------------------

#include <PiDxe.h> //EIP137196
#include <Protocol/CrbInfo.h> //EIP137196
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PciLib.h>
#include <Library/S3PciLib.h>
#include <Library/CrbLib.h> // Optional the library produce by CRB.
#include <Library/BaseMemoryLib.h> //EIP137196

#include <token.h>
#include <AmiDxeLib.h> // Optional. should use Mde Library instead.
#include <AmiCspLib.h> // Optional. should use Mde Library instead.
#include <Setup.h> // Optional. should use Mde Library instead.


// Produced Protocols

// Consumed Protocols
#include <Protocol/S3SaveState.h>

//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)
#define KBC_DATA_PORT   0x60 //EIP177963
// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)
//EIP137196 >>
UINT8
CrbDxeGetChipsetVendorNo (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
);

UINT32
CrbDxeGetCpuId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  	*This
);

UINT32
CrbDxeGetNorthBridgePciId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
);

UINT32
CrbDxeGetSouthBridgePciId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
);

CHAR8*
CrbDxeGetProjectId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
);

UINT16
CrbDxeGetBiosRevision (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
);
//EIP137196 <<
 
//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)
CHAR8 CONST ProjectTag[5] = CONVERT_TO_STRING(CRB_PROJECT_TAG); //EIP137196
EFI_S3_SAVE_STATE_PROTOCOL	*gS3SaveState = NULL; // Optional can use Mde instead.

// GUID Definition(s)

// Protocol Definition(s)
//EIP137196 >>
AMI_EFI_CRBINFO_PROTOCOL	gAmiCrbInforProtocol = {
	CrbDxeGetChipsetVendorNo,
	CrbDxeGetCpuId,
	CrbDxeGetNorthBridgePciId,
	CrbDxeGetSouthBridgePciId,
	CrbDxeGetProjectId,
	CrbDxeGetBiosRevision,
};
//EIP137196 <<

// External Declaration(s)

// Function Definition(s)

//---------------------------------------------------------------------------

//EIP137196 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeGetChipsetVendorNo
//
// Description: Provide the chipset vendor number.
//
// Input:       VOID
//
// Output:      0 - Unknown.
//				1 - Intel.
//				2 - AMD.
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
CrbDxeGetChipsetVendorNo (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
  )
{
	UINT32      Index;
	UINT32      RegisterEax;
	UINT32      RegisterEbx;
	UINT32      RegisterEcx;
	UINT32      RegisterEdx;
	// Intel
	// EBX 756E6547 "uneG"
	// ECX 6C65746E "letn"
	// EDX 49656e69 "Ieni"
	// AMD
	//EBX 68747541 "htuA"
	//ECX 444D4163 "DMAc"
	//EDX 69746E65 "itne"
	CHAR8 CONST Intel[5] = "letnI";
	CHAR8 CONST Amd[3] = "DMA";
	CHAR8 MyString[5];
	UINT8 ChipsetVendorNo = 0;

	Index = 0;
	Index = AsmCpuid(Index, &RegisterEax, &RegisterEbx, &RegisterEcx, &RegisterEdx);
	MyString[4] = ((CHAR8*)&RegisterEdx)[3];
	MyString[3] = ((CHAR8*)&RegisterEcx)[0];
	MyString[2] = ((CHAR8*)&RegisterEcx)[1];
	MyString[1] = ((CHAR8*)&RegisterEcx)[2];
	MyString[0] = ((CHAR8*)&RegisterEcx)[3];

	if (CompareMem(MyString, Intel, 5) == 0) {
		ChipsetVendorNo = 1;
	} else if (CompareMem(MyString, Amd, 3) == 0) {
		ChipsetVendorNo = 2;
	}

	return ChipsetVendorNo;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeGetCpuId
//
// Description: Provide the CPU ID.
//
// Input:
//    AMI_EFI_CRBINFO_PROTOCOL  	*This
//
// Output: EAX : Version Information (Type, Family, Model, and Stepping ID)
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
CrbDxeGetCpuId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  	*This
  )
{
	UINT32	Index = 1;
	UINT32	RegisterEax = -1;
	// EAX : Version Information (Type, Family, Model, and Stepping ID)
	Index = AsmCpuid(Index, &RegisterEax, NULL, NULL, NULL);
	return RegisterEax;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeGetNorthBridgePciId
//
// Description: Provide the PCI DID/VID of the north bridge.
//
// Input:       VOID
//
// Output:      -1 - Undefined.
//				others - PCI DID/VID.
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
CrbDxeGetNorthBridgePciId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
  )
{
	UINT32 PciId = -1;

	if (This->GetChipsetVendorNo(This) == 1) {
		PciId = PciRead32(PCI_LIB_ADDRESS(0, 0, 0, 0));
	} else if (This->GetChipsetVendorNo(This) == 2){
		PciId = PciRead32(PCI_LIB_ADDRESS(0, 0, 0, 0));
	}

	return PciId;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeGetSouthBridgePciId
//
// Description: Provide the PCI DID/VID of the south bridge.
//
// Input:       VOID
//
// Output:      -1 - Undefined.
//				others - PCI DID/VID.
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
CrbDxeGetSouthBridgePciId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
  )
{
	UINT32 PciId = -1;

	if (This->GetChipsetVendorNo(This) == 1) {
		PciId = PciRead32(PCI_LIB_ADDRESS(0, 31, 0, 0));
	} else if (This->GetChipsetVendorNo(This) == 2){
		PciId = PciRead32(PCI_LIB_ADDRESS(0, 20, 0, 0));
	}

	return PciId;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeGetProjectId
//
// Description: Provide the project ID.
//
// Input:       VOID
//
// Output:      NULL - Undefined / error.
//				others - Project ID.
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
CHAR8*
CrbDxeGetProjectId (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
  )
{
	return ProjectTag;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeGetBiosRevision
//
// Description: Provide the BIOS revision.
//
// Input:       VOID
//
// Output:      -1 - Undefined / error.
//				others - BIOS revision.
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
CrbDxeGetBiosRevision (
  IN  CONST AMI_EFI_CRBINFO_PROTOCOL  *This
  )
{
	return (CRB_PROJECT_MAJOR_VERSION << 8) | CRB_PROJECT_MINOR_VERSION;
}
//EIP137196 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbDxeInit
//
// Description: This function is the entry point for CRB DXE driver.
//              This function initializes the CRB in DXE phase.
//
// Input:       ImageHandle - Image handle
//              SystemTable - Pointer to the system table
//
// Output:      EFI_SUCCESS
//
// Notes:       This routine is called very early, prior to SBDXE and NBDXE.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
CrbDxeInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    EFI_STATUS              Status = EFI_SUCCESS;
    
//EIP177963 >>	
    InitAmiLib(ImageHandle,SystemTable);
    
#if KBC_SUPPORT && Recovery_SUPPORT && NPCE791x_SUPPORT
    {
    UINTN	Data,i;
    EFI_BOOT_MODE BootMode;
    
    BootMode = GetBootMode();

     if ( BootMode == BOOT_IN_RECOVERY_MODE ){
        // Clear KBC buffer for Ctrl+home recovery
        for(i=0;i<=0x10000;i++){
            Data = IoRead8(KBC_DATA_PORT);
        }
     }
    }
#endif
//EIP177963 <<
//EIP137196 >>
    Status = gBS->InstallMultipleProtocolInterfaces(
                    &ImageHandle,
                    &gAmiEfiCrbInfoProtocolGuid, &gAmiCrbInforProtocol,
                    NULL
                    );
//EIP137196 <<
    return Status;
}

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
