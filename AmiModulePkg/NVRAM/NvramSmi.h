//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//****************************************************************************
//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name: SmiVariable.h
//
// Description: Header file of the SmiVariable eModule.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

//---------------------------------------------------------------------------


#define SMI_SET_VARIABLE        2
#if NVRAM_SMI_FULL_PROTECTION == 1
#define SMI_GET_VARIABLE        1
#define SMI_GET_NEXT_VAR_NAME   3
#define SMI_QUERY_VAR_INFO      4
#endif

#define NVRAM_SMI_GUID \
{0x29C31B9F, 0xD2B9, 0x4900, 0xBD, 0x2A, 0x58, 0x4F, 0x29, 0x12, 0xE3, 0x86}

#define NVAR_SIGNATURE ('N'+('V'<<8)+(('A'+('R'<<8))<<16))//NVAR
typedef struct
{
    UINT32      Signature;
    UINT32      VarAttrib;
    EFI_GUID    VarGuid;
    UINTN       VarSize;
    EFI_STATUS  Status;
    UINT8       Subfunction;
    UINT64      MaxVarStorageSize;
    UINT64      RemVarStorageSize;
    UINT64      MaxVarSize;
    UINT8       VarData;

} SMI_VARIABLE;


//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
