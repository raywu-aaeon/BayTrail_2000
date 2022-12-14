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
#include <AmiGenericSio.h>
#include <Setup.h>
#include <Protocol\AmiSio.h>
#include <Protocol\PciIo.h>
#include <Protocol\AmiBoardInitPolicy.h>

EFI_GUID	gSioInitPolicyGuid=AMI_BOARD_SIO_INIT_PROTOCOL_GUID;
UINTN		gDriverVersion=0x100000;
CHAR8		*gSioName="Generic SIO Driver";
EFI_HANDLE	gSioInitHandle=NULL;




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
// Referrals: None
//
// Note:
//  It is recommended to have a separate Initialization Routine for each SIO Device.
//  it will save you some code needed to detect which device is currently selected.
//        case isGetSetupData:
//            SIO implementation uses separate set of NVRAM variables
//            associated with each LogicalDevice who has
//            SIO_DEV2.DeviceInfo->HasSetup property set to true.
//            Current Setup Settings are stored in SIO_DEV2.NvData.
//            If due to different look and fill we need to overwrite standard
//            Setup settings, we can do it here.....
//            =====================================================
//            if(SetupData==NULL){
//                Status=GetSetupData();
//                if(EFI_ERROR(Status)) return Status;
//                }
//            dev->NvData.DevEnable = SetupData->FdcEnable;
//            dev->NvData.DevPrsId  = 0;//SetupData->FdcPrsId;
//            dev->NvData.DevMode   = 0;//SetupData->FdcMode;
//            break;
//
//        case isPrsSelect:
//            If LDN uses non-standard way to determine possible resources(_PRS),
//            or _PRS may wary based  on LD mode. Then here we can get LD mode using
//            SIO_DEV2.NvData.Mode field and get corresponded to the mode _PRS Buffer
//            using GetPrsFromAml() function if ACPISUPPORT is ON. Or set of functions
//            EFI_STATUS SetUartPrs(SIO_DEV2 *Dev);
//            EFI_STATUS SetLptPrs(SIO_DEV2 *Dev, BOOLEAN UseDma);
//            EFI_STATUS SetFdcPrs(SIO_DEV2 *Dev);
//            EFI_STATUS SetPs2kPrs(SIO_DEV2 *Dev);
//            EFI_STATUS SetPs2mPrs(SIO_DEV2 *Dev);
//            EFI_STATUS SetGamePrs(SIO_DEV2 *Dev);
//            EFI_STATUS SetMpu401Prs(SIO_DEV2 *Dev);
//            Defined in AmiGenericSio.h
//
//        case isBeforeActivate:
//            //If any register needs to be initialized, whle enumerating all SIO devices.
//            //Use NEW SbLib_SetLpcDeviceDecoding() function to set Device Decoding Range for
//            //Legacy devices. Implementation in SbGeneric.c, definition in SbCspLib.h
//            //=====================================================
//
//        case isAfterActivate:
//            //Ttis Initialization step is used to programm any runtime registers rsiding in
//            //Decvice's decoded io space like SIO_GPIOs, SIO_PM, HHM registers.
//            //This Programming is needed if device doesnot have or don't need driver to do so.
//            //If there are a spetial driver like could be for HHM which could get THIS device handle
//            //and programm like Terminal Driver for COM ports and Floppy Driver for FDC
//            //nothing needs to be done here
//
//        case isAfterBootScript:
//            //This initialization step is needed to
//            //Use NEW SbLib_SetLpcDeviceDecoding() function to set Device Decoding Range for Legacy devices
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

	Args->Param1=&This->FunctionCount;
	Args->Param2=&gDriverVersion;
	Args->Param2=&gSioName[0];
	Args->Param3=&This->Uid;

	return EFI_SUCCESS;
}


//TODO Now using manualy created; than replace with AUTO generated 
//#include <SIODXEINIT.h>
#include <GSIODXEINIT.h> //Autogenerated GSIO Init Function Definition.
//DEBUG

EFI_STATUS SioDxeInitEntryPoint(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_STATUS				Status;
//	EFI_HANDLE				*hBuff=NULL;
//	UINTN					hCount=0, i;
//-----------------
	InitAmiLib(ImageHandle,SystemTable);

//DEBUG//DEBUG//DEBUG//DEBUG
//	EFI_DEADLOOP()
//DEBUG//DEBUG//DEBUG//DEBUG

	//Now install all Protocol Instances defined in SIOINITSDL.H
    Status=pBS->InstallMultipleProtocolInterfaces(
        &gSioInitHandle,
        &gAmiBoardSioInitProtocolGuid, gSioInitProtocolPtr, NULL
    );
    ASSERT_EFI_ERROR(Status);

    return Status;

}


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
