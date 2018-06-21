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
//<AMI_FHDR_START>
//
// Name:  AMIReportFvLib.h
//
// Description: Implementation of flexible ROM layout infrastructure support
//
//<AMI_FHDR_END>
//**********************************************************************
EFI_STATUS ReportFV2Pei(
    IN EFI_PEI_SERVICES **PeiServices
);

EFI_STATUS ReportFV2PeiAfterMem(
    IN EFI_PEI_SERVICES **PeiServices
);

EFI_STATUS ReportFV2Dxe(
    IN VOID* RecoveryCapsule OPTIONAL,
    IN EFI_PEI_SERVICES **PeiServices
);

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
