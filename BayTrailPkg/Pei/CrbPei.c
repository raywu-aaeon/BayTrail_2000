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
// Name:        CrbPei.c
//
// Description: This file contains code for Chipset Reference Board
//              Template initialization in the PEI stage
//
//<AMI_FHDR_END>
//*************************************************************************

//---------------------------------------------------------------------------
// Include(s)
//---------------------------------------------------------------------------
#include <PiPei.h>
#include <Ppi/CrbInfo.h> //EIP137196
#include <Library/PeiServicesLib.h> //EIP137196
#include <Library/PciLib.h>
#include <Library/BaseMemoryLib.h> //EIP137196

#include <Token.h>
#include <AmiPeiLib.h> // Optional. should use Mde Library instead.
#include <AmiCspLib.h> // Optional. should use Mde Library instead.
#include <Setup.h> // Optional. should use Mde Library instead.
#include <Sb.h> //EIP137196
#include <ppi\NBPPI.h>
#include <ppi\SBPPI.h>

#include <Library/SbPolicy.h>
#include <AaeonCommonLib.h>
//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)
//EIP137196 >>
UINT8
CrbPeiGetChipsetVendorNo (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
);

UINT32
CrbPeiGetCpuId (
  IN  CONST AMI_PEI_CRBINFO_PPI  	*This
);

UINT32
CrbPeiGetNorthBridgePciId (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
);

UINT32
CrbPeiGetSouthBridgePciId (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
);

CHAR8*
CrbPeiGetProjectId (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
);

UINT16
CrbPeiGetBiosRevision (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
);
//EIP137196 <<
//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)
CHAR8 CONST ProjectTag[5] = CONVERT_TO_STRING(CRB_PROJECT_TAG); //EIP137196

AMI_GPIO_INIT_TABLE_STRUCT CrbGpioTable [] = {
//    {13, GPIO_NC_OFFSET + IS_GPO + GPIO_PULL_DOWN},      //Program NCore GPIO_13 (It is a sample.)
//    {01, GPIO_SC_OFFSET + GPIO_FUNC2 + GPIO_PULL_DOWN},  //Program SCore GPIO_01 (It is a sample.)
    #include <OemGpio.h> // For 2000
    {0xffff, 0xffff}, // End of the table.
};

// GUID Definition(s)

// PPI Definition(s)
//EIP137196 >>
AMI_PEI_CRBINFO_PPI	gAmiCrbInforPpi = {
	CrbPeiGetChipsetVendorNo,
	CrbPeiGetCpuId,
	CrbPeiGetNorthBridgePciId,
	CrbPeiGetSouthBridgePciId,
	CrbPeiGetProjectId,
	CrbPeiGetBiosRevision,
};

static EFI_PEI_PPI_DESCRIPTOR gCrbPpiList[] =  {
		{
        (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
		&gAmiPeiCrbInfoPpiGuid,
		&gAmiCrbInforPpi
		}
	};
//EIP137196 <<

AMI_GPIO_INIT_PPI CrbGpioInitPpi = {
    IO_BASE_ADDRESS,
    CrbGpioTable,
    FALSE
};

AMI_SB_PCI_SSID_TABLE_STRUCT CrbSbSsidTable[] = {
    CRB_SB_PCI_DEVICES_SSID_TABLE
};

AMI_NB_PCI_SSID_TABLE_STRUCT CrbNbSsidTable[] = {
    CRB_NB_PCI_DEVICES_SSID_TABLE
};

static AMI_PEI_SB_CUSTOM_PPI CrbSbCustomPpi = {
    &CrbGpioInitPpi,
    CrbSbSsidTable
};

static AMI_PEI_NB_CUSTOM_PPI CrbNbCustomPpi = {
    CrbNbSsidTable
};

// PPI that are installed
static EFI_PEI_PPI_DESCRIPTOR CrbCustomPpi[] = {
    { EFI_PEI_PPI_DESCRIPTOR_PPI , \
      &gAmiPeiNbCustomPpiGuid, &CrbNbCustomPpi },
    { EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST, \
      &gAmiPeiSbCustomPpiGuid, &CrbSbCustomPpi },
};

// PPI that are installed

// PPI that are notified

// External Declaration(s)

// Function Definition(s)

//---------------------------------------------------------------------------

//EIP137196 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbPeiGetChipsetVendorNo
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
CrbPeiGetChipsetVendorNo (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
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
// Procedure:   CrbPeiGetCpuId
//
// Description: Provide the CPU ID.
//
// Input:
//    AMI_PEI_CRBINFO_PPI  	*This
//
// Output: EAX : Version Information (Type, Family, Model, and Stepping ID)
//
// Notes:       .
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
CrbPeiGetCpuId (
  IN  CONST AMI_PEI_CRBINFO_PPI  	*This
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
// Procedure:   CrbPeiGetNorthBridgePciId
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
CrbPeiGetNorthBridgePciId (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
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
// Procedure:   CrbPeiGetSouthBridgePciId
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
CrbPeiGetSouthBridgePciId (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
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
// Procedure:   CrbPeiGetProjectId
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
CrbPeiGetProjectId (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
  )
{
	return ProjectTag;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbPeiGetBiosRevision
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
CrbPeiGetBiosRevision (
  IN  CONST AMI_PEI_CRBINFO_PPI  *This
  )
{
	return (CRB_PROJECT_MAJOR_VERSION << 8) | CRB_PROJECT_MINOR_VERSION;
}
//EIP137196 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CrbPeiInit
//
// Description: This function is the entry point for CRB PEIM.
//              It initializes the chipset CRB in PEI phase.
//
// Input:       FfsHeader   - Pointer to the FFS file header.
//              PeiServices - Pointer to the PEI services table.
//
// Output:      EFI_SUCCESS
//
// Notes:       This routine is called very early, prior to SBPEI and NBPEI.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
CrbPeiInit (
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
    EFI_STATUS                  Status = EFI_SUCCESS;
    SB_SETUP_DATA               PchPolicyData;

    Status = PeiServicesInstallPpi(gCrbPpiList); //EIP137196

#if CRB_CUSTOM_PPI_SUPPORT
    Status = (*PeiServices)->InstallPpi( PeiServices, CrbCustomPpi );
    ASSERT_PEI_ERROR( PeiServices, Status );
#endif

    GetSbSetupData((VOID *)PeiServices, &PchPolicyData, TRUE);

    #if defined(F81866_SUPPORT) && (F81866_SUPPORT == 1)
	if ( PchPolicyData.WdtEnabled )
    {
        F81866EnableWdt(PchPolicyData.WdtUnit, PchPolicyData.WdtTimer);	
    }
    else
    {
        F81866DisableWdt();
    }
    #endif

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
