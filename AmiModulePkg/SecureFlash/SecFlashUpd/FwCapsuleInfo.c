//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: FwCapsuleInfo.c
//
// Description:   File contains hook FwCapsuleInfo() which is called from  Recovery.LoadRecoveryCapsule
//                Hook use dot update the default Recovery File name and Size if defaults are 
//                overridden in FlashUpd EFI Var. 
//----------------------------------------------------------------------
//<AMI_FHDR_END>
// Module specific Includes
#include "Efi.h"
#include "Pei.h"
#include "token.h"
#include <AmiPeiLib.h>
#include <Hob.h>
#include <Ppi/ReadOnlyVariable.h>
#include <FlashUpd.h>

//------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------
extern EFI_GUID gPeiReadOnlyVariablePpiGuid;

const char *RecoveryCapFileName = CONVERT_TO_STRING(FWCAPSULE_FILE_NAME);
const UINTN RecoveryCapImageSize = FWCAPSULE_IMAGE_SIZE; // 8k should be enough to cover Sec Capsule Hdr overhead

AMI_FLASH_UPDATE_BLOCK gFlashUpdDesc = {FlRecovery, 1<<FV_MAIN, CONVERT_TO_STRING(FWCAPSULE_FILE_NAME), FWCAPSULE_IMAGE_SIZE,0 };

//------------------------------------------------------------------------
// Local prototypes
//------------------------------------------------------------------------
EFI_STATUS
FwCapsuleInfo (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN OUT VOID          **pCapsuleName,
  IN OUT UINTN         *pCapsuleSize,
  OUT   BOOLEAN        *ExtendedVerification
);
//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:    FwCapsuleInfo
//
// Description:  Updates the Recovery File name and size if defaults are
//               overriden in FlashUpd EFI Var.
//               Called from Recovery LoadRecoveryCapsule.
//
//    pCapsuleName  Pointer to the variable containing a Recovery File name
//    pCapsuleSize  Pointer to the size of recovery image capsule, in bytes.
//    ExtendedVerification Indicates to Recovery module whether Fw Capsule
//                  Recovery path will perform image size check.
//
// Output:
//    EFI_SUCCESS
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
FwCapsuleInfo (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN OUT VOID          **pCapsuleName,
  IN OUT UINTN         *pCapsuleSize,
  OUT   BOOLEAN        *ExtendedVerification
){
    EFI_STATUS          Status;
    UINTN               Size;
    EFI_PEI_READ_ONLY_VARIABLE_PPI  *ReadOnlyVariable = NULL;
    EFI_GUID gFlashUpdGuid  = FLASH_UPDATE_GUID;
    UINT32 AlternativeBiosLimit = 0;  //<EIP153486+>

    if(!pCapsuleName && !pCapsuleSize && !ExtendedVerification )
        return EFI_UNSUPPORTED;

    if(ExtendedVerification != NULL)
        *ExtendedVerification = TRUE;

    if(pCapsuleSize != NULL)
    {
        *pCapsuleSize = RecoveryCapImageSize;

        if(pCapsuleName != NULL)
        {
            // Secure Recovery image file name
            *pCapsuleName = (VOID*)RecoveryCapFileName;

//<EIP153486+> >>>
            AlternativeBiosLimit = *(UINT32*)0xE00D0050;
            AlternativeBiosLimit = AlternativeBiosLimit >> 0x13;
            if( AlternativeBiosLimit )
            {
                AlternativeBiosLimit = AlternativeBiosLimit << 0x0C;
                AlternativeBiosLimit |= 0xFF0;
                if( (*(UINT32*)(AlternativeBiosLimit+FULL_FLASH_BASE_ADDRESS) != 0xFFFFFFFF) )
                {
                    PEI_TRACE((-1, PeiServices, "FOTA DID NOT FINISH!\n"));
                    *pCapsuleName = (VOID*)CONVERT_TO_STRING(firmware.bin);
                }
            }
//<EIP153486+> <<<

        // Detect if we are in Flash Update mode and set some recovery global variables
        // Read "FlashOp" Variable to update global RecoveryFileName, Size
            Status = (*PeiServices)->LocatePpi( PeiServices,
                                        &gPeiReadOnlyVariablePpiGuid,
                                        0,
                                        NULL,
                                        &ReadOnlyVariable );
        //    ASSERT_PEI_ERROR (PeiServices, Status);
           if(EFI_ERROR(Status))
               return Status;

            Size = sizeof(AMI_FLASH_UPDATE_BLOCK);
            Status = ReadOnlyVariable->PeiGetVariable( PeiServices,
                                        FLASH_UPDATE_VAR,
                                        &gFlashUpdGuid,
                                        NULL,
                                        &Size,
                                        &gFlashUpdDesc );
            if(!EFI_ERROR(Status))
            {
                if(gFlashUpdDesc.FlashOpType == FlRecovery && gFlashUpdDesc.FwImage.AmiRomFileName[0] != 0)
                    *pCapsuleName = (VOID*)gFlashUpdDesc.FwImage.AmiRomFileName;

                *pCapsuleSize = gFlashUpdDesc.ImageSize;
                Status = (*PeiServices)->SetBootMode(PeiServices, BOOT_ON_FLASH_UPDATE);
            }
            PEI_TRACE((-1, PeiServices, "FW Capsule update %r\nImage Name %s, Size %x\n", Status, *pCapsuleName, *pCapsuleSize));
        }
    }
    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
