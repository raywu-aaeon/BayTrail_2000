//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file AmiGopPolicy.c
    AmiGopPolicy output initialization in the DXE stage.

*/

//----------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------

#include <Efi.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Setup.h>
#include <Token.h>
#include <Protocol/DevicePath.h>
#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
#include <Protocol/EdidOverride.h>
#endif
#include <AmiGopPolicy.h>
#include <AmiGopPolicyLinks.h>

//----------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//----------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
typedef EFI_STATUS (OEM_GOP_EDID_OVERRIDE_GET_EDID) (
  IN  EFI_EDID_OVERRIDE_PROTOCOL    *This,
  IN  EFI_HANDLE                    *ChildHandle,
  OUT UINT32                        *Attributes,
  IN OUT UINTN                      *EdidSize,
  IN OUT UINT8                      **Edid
);
#endif

// Function Prototype(s)

#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
EFI_STATUS AmiGopEdidOverrideGetEdid (
  IN  EFI_EDID_OVERRIDE_PROTOCOL    *This,
  IN  EFI_HANDLE                    *ChildHandle,
  OUT UINT32                        *Attributes,
  IN OUT UINTN                      *EdidSize,
  IN OUT UINT8                      **Edid
);
#endif

//----------------------------------------------------------------------
// Variable and External Declaration(s)
//----------------------------------------------------------------------
// Variable Declaration(s)

// GUID Definition(s)

static EFI_GUID gAmiGopPolicyVariableGuid = AMI_GOP_POLICY_VARIABLE_GUID;    

// Protocol Definition(s)

#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
EFI_EDID_OVERRIDE_PROTOCOL EfiEdidOverrideProtocol = {
  AmiGopEdidOverrideGetEdid
};
#endif

// External Declaration(s)

#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
extern OEM_GOP_EDID_OVERRIDE_GET_EDID   OEM_GOP_EDID_OVERRIDE_GET_EDID_LIST EndOfOemGopEdidOverrideGetEdidList;
#endif

// Function Definition(s)

VOID ConnectDevicePath(IN EFI_DEVICE_PATH_PROTOCOL *pPath);
#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
OEM_GOP_EDID_OVERRIDE_GET_EDID* OemGopEdidOverrideGetEdidList[] = {OEM_GOP_EDID_OVERRIDE_GET_EDID_LIST NULL};
#endif


/**
    Connect AmiGopPolicyConOut Device Path

    @param VOID

    @retval VOID

    @note  None.

**/

VOID
ConnectAmiGopPolicyConOut (VOID)
{
	/* EIP195032 >>
	  EFI_STATUS                  Status;
    EFI_DEVICE_PATH_PROTOCOL    *GopDevicePath = NULL;
    UINTN                       VariableSize = 0;

    Status = GetEfiVariable (
             L"AmiGopOutputDp",
             &gAmiGopPolicyVariableGuid,
             NULL,
             &VariableSize,
             &GopDevicePath);
    if ((EFI_ERROR(Status)) || (GopDevicePath == NULL)) return ;

    ConnectDevicePath(GopDevicePath);
    pBS->FreePool(GopDevicePath);

    return;
    EIP195032 >>*/
}

/**
    Invoke AmiGopEdidOverrideGetEdid eLink

    @param *This
    @param *ChildHandle
    @param *Attributes
    @param *EdidSize
    @param ***Edid

    @retval EFI_SUCCESS - Get Oem Override Edid Success.

    @note  None.

**/

#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
EFI_STATUS
AmiGopEdidOverrideGetEdid (
  IN      EFI_EDID_OVERRIDE_PROTOCOL  *This,
  IN      EFI_HANDLE                  *ChildHandle,
  OUT     UINT32                      *Attributes,
  IN OUT  UINTN                       *EdidSize,
  IN OUT  UINT8                       **Edid )
{
    EFI_STATUS  Status = EFI_SUCCESS;
    UINTN       i;

    for (i = 0; OemGopEdidOverrideGetEdidList[i] != NULL; i++) {
        Status = OemGopEdidOverrideGetEdidList[i] (This, ChildHandle, Attributes, EdidSize, Edid);
        if(!EFI_ERROR(Status)) break;
    }

    return Status;
}
#endif

/**
    Sample code for AmiGopEdidOverrideGetEdid eLink function

    @param *This
    @param *ChildHandle
    @param *Attributes
    @param *EdidSize
    @param ***Edid

    @retval EFI_UNSUPPORTED - This Function Always Returns UNSUPPORTED.

    @note  None.

**/

#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
EFI_STATUS
AmiDefaultGopEdidOverrideGetEdid (
  IN  EFI_EDID_OVERRIDE_PROTOCOL    *This,
  IN  EFI_HANDLE                    *ChildHandle,
  OUT UINT32                        *Attributes,
  IN OUT UINTN                      *EdidSize,
  IN OUT UINT8                      **Edid )
{
    return EFI_UNSUPPORTED;
}
#endif

/**
    Installs AmiGopPolicy related protocols

    @param VOID

    @retval VOID

    @note  None.

**/

VOID
AmiGopPolicyProtocolInsatll ( VOID )
{
#if OEM_EDID_OVERRIDE_PROTOCOL_SUPPORT
    EFI_HANDLE  Handle = NULL;

    pBS->InstallMultipleProtocolInterfaces(
         &Handle,
         &gEfiEdidOverrideProtocolGuid,
         &EfiEdidOverrideProtocol,
         NULL );
#endif
    return;
}
