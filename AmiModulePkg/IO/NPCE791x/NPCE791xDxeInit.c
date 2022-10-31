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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  <This File's Name>
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include "NPCE791xDxeInit.h"
//#include <Library\SioLibExt.h>
#include <GenericSio.h>

//UINTN		gDriverVersion=0x100000;
//CHAR8		*gSioName="NPCE791x";

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: ClearDevResource
//
// Description:
//  This function will Clear SIO resource
//
// Input:
//  SPIO_DEV* dev
// Output: 
//  NONE
//
//------------------------------------------------------------------------- 
// <AMI_PHDR_END>
VOID ClearDevResource(
    IN  SIO_DEV2    *dev
)
{
    IoWrite8(NPCE791x_CONFIG_INDEX, NPCE791x_LDN_SEL_REGISTER);
    IoWrite8(NPCE791x_CONFIG_DATA, dev->DeviceInfo->Ldn);

    IoWrite8(NPCE791x_CONFIG_INDEX, NPCE791x_BASE1_HI_REGISTER);
    IoWrite8(NPCE791x_CONFIG_DATA, 0);
    IoWrite8(NPCE791x_CONFIG_INDEX, NPCE791x_BASE1_LO_REGISTER);
    IoWrite8(NPCE791x_CONFIG_DATA, 0);
    IoWrite8(NPCE791x_CONFIG_INDEX, NPCE791x_IRQ1_REGISTER);
    IoWrite8(NPCE791x_CONFIG_DATA, 0);

    return;

}

/*EFI_STATUS	Func0(
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
}*/


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: KBC_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS KBC_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;

    EFI_STATUS  Status=EFI_SUCCESS;
    SIO_DEV2    *dev=(SIO_DEV2*)AmiSio;

//-----------------------------
    switch (InitStep) {
		case isGetSetupData:
		case isGetModeData:
		case isPrsSelect:
		case isAfterActivate:
		case isAfterBootScript:
		break;

		case isBeforeActivate:
			AmiSioLibSetLpcDeviceDecoding(PciIo,dev->VlData.DevBase1, dev->DeviceInfo->Uid, dev->DeviceInfo->Type);  // Enable IODecode
		break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
    return Status;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: COM_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS COM_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock)
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS       Status=EFI_SUCCESS;
    SIO_DEV2          *dev=(SIO_DEV2*)AmiSio;
//    UINT8   rv;
//	ACPI_HDR                        *dsdt;
//    EFI_PHYSICAL_ADDRESS            a;

	switch (InitStep)
    {
		case isGetSetupData:
			// Disable IODecode?
			if((!dev->DeviceInfo->Implemented) || (!dev->NvData.DevEnable)){
				AmiSioLibSetLpcDeviceDecoding(PciIo,0, dev->DeviceInfo->Uid, dev->DeviceInfo->Type);
				ClearDevResource(dev);
			}
		break;

		case isGetModeData:
		break;

		case isPrsSelect:
		break;

		case isBeforeActivate:
			// Enable IODecode
			AmiSioLibSetLpcDeviceDecoding(PciIo,dev->VlData.DevBase1, dev->DeviceInfo->Uid, dev->DeviceInfo->Type);

            //Programm Device Mode register here(if NEEDED)use AmiSioProtocol

        break;

        case isAfterActivate:
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    }//switch

    return Status;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: CIR_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS CIR_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS    Status=EFI_SUCCESS;
    SIO_DEV2    *dev=(SIO_DEV2*)AmiSio;


#if   NPCE791x_CIR_PORT_PRESENT
//-----------------------------
    switch (InitStep) {
		case isGetSetupData:
			// Disable IODecode?
			if((!dev->DeviceInfo->Implemented) || (!dev->NvData.DevEnable)){
				AmiSioLibSetLpcDeviceDecoding(PciIo,0, dev->DeviceInfo->Uid, dev->DeviceInfo->Type);
				ClearDevResource(dev);
			}
		break;

		case isGetModeData:
		break;

		case isPrsSelect:
		break;

		case isBeforeActivate:
			// Enable IODecode
			AmiSioLibSetLpcDeviceDecoding(PciIo,dev->VlData.DevBase1, dev->DeviceInfo->Uid, dev->DeviceInfo->Type);
        case isAfterActivate:
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
#endif
    return Status;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: MSWC_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS MSWC_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock)
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS      Status=EFI_SUCCESS;
    SIO_DEV2    *dev=(SIO_DEV2*)AmiSio;
    EFI_S3_SAVE_STATE_PROTOCOL *s3s = dev->Owner->SaveState;

#if NPCE791x_MSWC_PRESENT
//-----------------------------
    switch (InitStep) {
        case isGetSetupData:
        break;

		case isGetModeData:
		break;

        case isPrsSelect:
        break;

        case isBeforeActivate:
        break;

        case isAfterActivate:
            
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
#endif

    return Status;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: SHM_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS SHM_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS  Status = EFI_SUCCESS;

#if NPCE791x_SHM_PRESENT
//-----------------------------
    switch (InitStep) {
        case isGetSetupData:
        break;

		case isGetModeData:
		break;

        case isPrsSelect:
        break;

        case isBeforeActivate:
        break;

        case isAfterActivate:
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
#endif
    return Status;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: PM1_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS PM1_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS    Status = EFI_SUCCESS;

//-----------------------------
#if NPCE791x_PM1_PRESENT
    switch (InitStep) {
        case isGetSetupData:
        break;

		case isGetModeData:
		break;

        case isPrsSelect:
        break;

        case isBeforeActivate:
        break;

        case isAfterActivate:

        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
#endif
//-----------------------------
    return Status;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: PM2_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS PM2_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS  Status=EFI_SUCCESS;

#if NPCE791x_PM2_PRESENT
//-----------------------------
    switch (InitStep) {
        case isGetSetupData:
        break;

		case isGetModeData:
		break;

        case isPrsSelect:
        break;

        case isBeforeActivate:
        break;

        case isAfterActivate:
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
#endif
    return Status;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: PM3_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS PM3_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS    Status=EFI_SUCCESS;

#if NPCE791x_PM3_PRESENT
//-----------------------------
    switch (InitStep) {
        case isGetSetupData:
        break;

		case isGetModeData:
		break;

        case isPrsSelect:
        break;

        case isBeforeActivate:
        break;

        case isAfterActivate:
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
#endif
    return Status;
}
// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: ESHM_Init
//
// Description:
//  This function provide each initial routine in genericsio.c
//
// Input:
//  IN  AMI_SIO_PROTOCOL    *AmiSio - Logical Device's information
//  IN  EFI_PCI_IO_PROTOCOL *PciIo - Read/Write PCI config space
//  IN  SIO_INIT_STEP       InitStep - Initial routine step
//
// Output:
//            EFI_STATUS
//            EFI_SUCCESS - Initial step sucessfully
//            EFI_INVALID_PARAMETER - not find the initial step
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS ESHM_Init(
//    IN    AMI_SIO_PROTOCOL     *AmiSio,
//    IN    EFI_PCI_IO_PROTOCOL  *PciIo,
//    IN    SIO_INIT_STEP        InitStep
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock )
{
    //Update Standard parameter block
    AMI_BOARD_INIT_PARAMETER_BLOCK	*Args=(AMI_BOARD_INIT_PARAMETER_BLOCK*)ParameterBlock;
    SIO_INIT_STEP        			InitStep=(SIO_INIT_STEP)Args->InitStep;
    AMI_SIO_PROTOCOL     			*AmiSio=(AMI_SIO_PROTOCOL*)Args->Param1;
    EFI_PCI_IO_PROTOCOL  			*PciIo=(EFI_PCI_IO_PROTOCOL*)Args->Param2;
    EFI_STATUS    Status=EFI_SUCCESS;

#if   NPCE791x_ESHM_PRESENT
//-----------------------------
    switch (InitStep) {
        case isGetSetupData:
        break;

		case isGetModeData:
		break;

        case isPrsSelect:
        break;

        case isBeforeActivate:
        break;

        case isAfterActivate:
        break;

        case isAfterBootScript:
        break;

        default: Status=EFI_INVALID_PARAMETER;
    } //switch
//-----------------------------
#endif
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



