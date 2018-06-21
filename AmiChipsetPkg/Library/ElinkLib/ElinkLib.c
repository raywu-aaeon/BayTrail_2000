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
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        ElinkLib.c
//
// Description: This file contains the functions used for AMI Elink
//
//<AMI_FHDR_END>
//*************************************************************************

//-------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------

#include <Library/BaseLib.h>
#include <Uefi/UefiBaseType.h>
#include <Library/ElinkLib.h>
#include <Library/MemoryAllocationLib.h>

//-------------------------------------------------------------------------
// Variable and External Declaration(s)
//-------------------------------------------------------------------------
// Variable Declaration(s)
extern AMI_ELINK_TABLE mAmiElinkTable[];
extern AMI_HOOK_TABLE  mAmiHookTable[];
extern UINTN mAmiElinkTableNum;


UINT32 
ElinkGet (
  IN UINTN            TokenNumber
  )
{
  UINT32              Index;

  for (Index = 0; Index < mAmiElinkTableNum; Index++) {
    if (mAmiElinkTable[Index].TokenNumber == TokenNumber) {
      return (UINT32) mAmiHookTable[Index].ElinkPtr;
    }
  }
  return 0;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ElinkAdd
//
// Description: This function compares ParentName with the name published in
//              gAmiElinkTable, and add the Function to the corresponding
//              elink while matches.
//
// Input:       *ParentName - The name of the parent elink
//              Function    - The function which is going to be added to the
//                            parent elink.
//
// Output:      EFI_INVALID_PARAMETER - mAmiElinkTableNum is zero
//              EFI_OUT_OF_RESOURCES  - Failed to add elink
//              EFI_NOT_FOUND         - Can not find matched name in
//                                      gAmiElinkTable
//              EFI_SUCCESS           - Find matched name in gAmiElinkTable
//                                      and is successful to add elink
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
ElinkAdd (
  IN UINTN                TokenNumber,
  IN AMI_HOOK             Function
  )
{
  AMI_HOOK_LINK           *AmiHookLink;
  UINT32                   Index;

  for (Index = 0; Index < mAmiElinkTableNum; Index++) {
    if (mAmiElinkTable[Index].TokenNumber == 0) {
      AmiHookLink = AllocateZeroPool (sizeof(AMI_HOOK_LINK));
      if (AmiHookLink == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      mAmiElinkTable[Index].TokenNumber = TokenNumber;
      mAmiHookTable[Index].ElinkPtr = (UINTN) AmiHookLink;
      break;
    } else {
      if (mAmiElinkTable[Index].TokenNumber == TokenNumber) {
        AmiHookLink = (AMI_HOOK_LINK *) mAmiHookTable[Index].ElinkPtr;
        break;
      }
    }
  }

  for (Index = 0; Index < ELINK_ARRAY_NUM; Index++) {
    if (AmiHookLink->ElinkArray[Index] == NULL) {
      AmiHookLink->ElinkArray[Index] = Function;
      return EFI_SUCCESS;
    }
  }

  return EFI_OUT_OF_RESOURCES;
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
