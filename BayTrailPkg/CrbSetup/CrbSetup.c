//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
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
// $Header: /Alaska/BIN/Modules/CRB Board/CRBSetup.c 3     2/19/09 10:02p Abelwu $
//
// $Revision: 3 $
//
// $Date: 2/19/09 10:02p $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/BIN/Modules/CRB Board/CRBSetup.c $
// 
// 3     2/19/09 10:02p Abelwu
// Updated for AMI Coding Standard v0.99
// 
// 2     6/04/08 6:04a Abelwu
// Updated the header of the source file.
// 
// 1     6/03/08 2:40a Abelwu
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        CRBSetup.c
//
// Description: Chipset Reference Board Setup Routines
//
//<AMI_FHDR_END>
//*************************************************************************

//---------------------------------------------------------------------------
// Include(s)
//---------------------------------------------------------------------------

#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Setup.h>
#include <Protocol\PciIo.h>
#include <Protocol\DevicePath.h>
#include <Protocol\PciRootBridgeIo.h>
#include <PchRegs.h>
#include "token.h"
#include <Library/KscLib.h>

//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)

#define PciD31F0RegBase                           PCIEX_BASE_ADDRESS + (UINT32) (31 << 15)

// GUID Definition(s)

// Protocol Definition(s)

// External Declaration(s)

// Function Definition(s)

//---------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitCrbStrings
//
// Description: Initializes Demo Board Setup String.
//
// Input:       HiiHandle - Handle to HII database
//              Class     - Indicates the setup class
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitCrbStrings (
    IN EFI_HII_HANDLE   HiiHandle,
    IN UINT16           Class )
{
    EFI_STATUS              Status = EFI_SUCCESS;
    UINT8                   RevId;
#if CRB_EC_SUPPORT
    UINT8                   KscVerHigh;
    UINT8                   KscVerLow;
#endif

    if (Class == MAIN_FORM_SET_CLASS) {
//EIP136991 >>
        RevId = MmioRead8 (PciD31F0RegBase + R_PCH_LPC_RID_CC);
        //
        // Get SoC Stepping
        //
        switch (RevId) {
            case V_PCH_LPC_RID_0:
            case V_PCH_LPC_RID_1:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"A0 Stepping", NULL);
                break;
            case V_PCH_LPC_RID_2:
            case V_PCH_LPC_RID_3:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"A1 Stepping", NULL);
                break;
            case V_PCH_LPC_RID_4:
            case V_PCH_LPC_RID_5:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"B0 Stepping", NULL);
                break;
            case V_PCH_LPC_RID_6:
            case V_PCH_LPC_RID_7:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"B1 Stepping", NULL);
                break;
            case V_PCH_LPC_RID_8:
            case V_PCH_LPC_RID_9:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"B2 Stepping", NULL);
                break;
            case V_PCH_LPC_RID_A:
            case V_PCH_LPC_RID_B:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"B3 Stepping", NULL);
                break;
            case V_PCH_LPC_RID_C:
            case V_PCH_LPC_RID_D:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"C0 Stepping", NULL);
                break;
            case 0x11:
            case 0x12:
                InitString(HiiHandle, STRING_TOKEN(STR_SOC_STEPPING_VALUE), L"D0 Stepping", NULL);
                break;
            default:
              break;
        }

#if CRB_EC_SUPPORT
        //
        // Get KSC Version
        //
        Status = SendKscCommand(0x90);      // SMC_READ_REVISION
        ASSERT_EFI_ERROR(Status);
        if (!EFI_ERROR(Status)) {
            Status = ReceiveKscData(&KscVerHigh);
            Status = ReceiveKscData(&KscVerLow);
            if(!EFI_ERROR(Status)){
                InitString(HiiHandle, STRING_TOKEN(STR_KSC_VALUE), L"%X.%02X", KscVerHigh, KscVerLow);
            }
        }
#endif
//EIP136991 <<
    }
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
