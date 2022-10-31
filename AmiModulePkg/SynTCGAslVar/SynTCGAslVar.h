//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2009, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//


//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	SynTCGAslVar.h
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#ifndef __SY_TCG_ASL_VAR_H__
#define __SY_TCG_ASL_VAR_H__
#ifdef __cplusplus
extern "C" {
#endif

EFI_STATUS GetTCGAslUpdateMem(
        UINT8 **pRetriveAddr
    );

EFI_STATUS Update_UserConfirmSts_to_ASL(
    VOID );

EFI_STATUS Update_PPIOP_Rst_to_ASL(
    VOID );

EFI_STATUS Store_MOR_request(
    VOID );

EFI_STATUS Store_PPI_request(
    VOID );

EFI_STATUS SyncTCGAsl_ResetSystem(
    VOID );

EFI_STATUS SyncTCGAsl_ExitBIOS(
    VOID );

EFI_STATUS LoadFile(
	EFI_GUID		*Guid,
	VOID			**Buffer,
	UINTN			*BufferSize
);

#define PPI_INVAILD_SIG     0xFF

#define PPI_Submit_Ofst     0x00
#define PPI_PendOP_Ofst     0x01
#define PPI_CurOP_Ofst      0x02
#define PPI_MOR_Ofst        0x03
#define PPI_OPRst_Ofst      0x04
#define PPI_Confirm_Ofst    0x10

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif

//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2009, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
