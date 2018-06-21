//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
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
// $Header: /Alaska/Tools/template.c 6     1/13/10 2:13p Felixp $
//
// $Revision: 6 $
//
// $Date: 1/13/10 2:13p $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/Tools/template.c $
// 
// 6     1/13/10 2:13p Felixp
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  SioInitPolicy.c
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************

#include <AmiDxeLib.h>
#include <Token.h>
#include <PciBus.h>
#include <PciHostBridge.h>
#include <Setup.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/AmiBoardInfo2.h>
#include <Protocol/AmiBoardInitPolicy.h>

UINTN		gDriverVersion=PCI_BUS_VER_COMBINED;
CHAR8		*gPciName="AMI PCI Bus Driver";
UINTN		gPciPlatformId=0x1;
EFI_HANDLE	gPciInitHandle=NULL;




//-------------------------------------------------------------------------
// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: Func0()
//
// Description:
//  Default Init Function - FUNC 0. Must present always. 
//  initialization(Assigning and Programming IO/IRQ/DMA resources),
//  First time it will be called before Activating the device,
//  If device requires some additional initialization like
//      - programming SIO device registers except IO1, IO2, IRQ1, IRQ2, DMA1 DMA2
//  Second time After Installing AmiSioProtocol, and DevicePath Protocol of SIO Device.
//  If device requires some additional initialization like
//      - if programming of some runtime registers like SIO_GPIO, SIO_PM SIO_HHM is needed
//      - implementation of some additional setup questions
//  do it here
//  NOTE#1 Once SIO_INIT function invoked SIO Logical device already selected
//  NOTE#2 If Device Does not require any additional initialization just set
//     InitRoutine field to NULL in SioDevLst[] Table.
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output: None
//  EFI_STATUS
//  EFI_SUCCESS - Initial step sucessfully
//  EFI_INVALID_PARAMETER - not find the initial step
//
// Modified: Nothing
//
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS	Func0(
		AMI_BOARD_INIT_PROTOCOL		*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock
)
{
	AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
//----------------------


	if((This==NULL) || (*Function != 0) || (Args->InitStep != 0) || (ParameterBlock==NULL)) return EFI_INVALID_PARAMETER;

    if(Args->Signature != AMI_PCI_PARAM_SIG) return EFI_INVALID_PARAMETER;

    Args->Param1=&This->FunctionCount;
	Args->Param2=&gDriverVersion;
	Args->Param2=&gPciName[0];
	Args->Param3=&gPciPlatformId;

	return EFI_SUCCESS;
}


//Now using AUTO generated 
#include <PCIDXEINIT.h>

EFI_STATUS PciDxeInitEntryPoint(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_STATUS				Status;
//-----------------
	InitAmiLib(ImageHandle,SystemTable);

	//Now install all Protocol Instances defined in SIOINITSDL.H
    Status=pBS->InstallMultipleProtocolInterfaces(
        &gPciInitHandle,
        &gAmiBoardPciInitProtocolGuid, gPciInitProtocolPtr, NULL
        //&gF81866InitPolicyGuid, &gF81866InitProtocol, NULL
    );
    ASSERT_EFI_ERROR(Status);

    return Status;

}
//============================================================
//---- DO Not modify this code!
//============================================================


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
