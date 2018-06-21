//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
/** @file
  BlockS3Var Nvram Library file.
**/
//----------------------------------------------------------------------------
// Includes
#include <AmiDxeLib.h>
#include <BlockS3VarsElink.h>

//----------------------------------------------------------------------------
// Local prototypes
#define BDS_CONNECT_DRIVERS_PROTOCOL_GUID \
    { 0x3aa83745, 0x9454, 0x4f7a, { 0xa7, 0xc0, 0x90, 0xdb, 0xd0, 0x2f, 0xab, 0x8e } }

// Type Definition(s)
typedef struct {
    CHAR16      *Name;
    EFI_GUID    Guid;
} VAR_STRUCT;

// Function Prototype(s)
VOID BdsEvtFunc (
    IN EFI_EVENT Event,
    IN VOID      *Context
);

//----------------------------------------------------------------------------
// Local Variables
VAR_STRUCT  gBlockedS3VarList[] = { BLOCKED_S3_VAR_LIST
            {NULL, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}};
static BOOLEAN  gInitializationCompleted = FALSE;
static BOOLEAN  gEndOfDxe = FALSE;
static EFI_GUID gBdsConnectDriversProtocolGuid = BDS_CONNECT_DRIVERS_PROTOCOL_GUID;

//----------------------------------------------------------------------------
// Function Externs
extern  EFI_BOOT_SERVICES       *pBS;

//----------------------------------------------------------------------------
// Function Definitions
/**
 * This function checks the input Variable whether it needs to be blocked or not.
 *
 * @param VariableName Pointer to Variable Name in Unicode
 * @param VendorGuid Pointer to Variable GUID
 * @param Attributes Attributes of the Variable
 * @param DataSize Size of the Variable
 * @param Data Pointer to memory where Variable data is stored
 *
 * @retval EFI_UNSUPPORTED The Variable isn't found.
 * @retval EFI_WRITE_PROTECTED The Variable is found.
 */
EFI_STATUS SetVariableS3Hook (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32   Attributes,
    IN UINTN    DataSize,
    IN VOID     *Data )
{
    EFI_STATUS      Status;
    EFI_EVENT       BdsEvt;
    VOID            *Registration = NULL;
    UINT32          i;
    
    if (gBlockedS3VarList[0].Name == NULL) return EFI_UNSUPPORTED; // no Variable needs to be checked.

    if (!gInitializationCompleted) {
        gInitializationCompleted = TRUE;
        if (!pSmst) {
            Status = pBS->CreateEvent(EVT_NOTIFY_SIGNAL, TPL_CALLBACK, BdsEvtFunc, NULL, &BdsEvt);
            if (EFI_ERROR(Status)) {
                TRACE((TRACE_ALWAYS,"Unable to create BDS event\n"));
                return Status;
            }
            pBS->RegisterProtocolNotify (\
                            &gBdsConnectDriversProtocolGuid, BdsEvt, &Registration);
        }
    }

    if (gEndOfDxe || pSmst) {
        for (i = 0; gBlockedS3VarList[i].Name != NULL; i++) {
            if (!Wcscmp(VariableName, gBlockedS3VarList[i].Name) && !guidcmp(VendorGuid, &gBlockedS3VarList[i].Guid)) {
                    return EFI_WRITE_PROTECTED;
            }
        }
    }

    return EFI_UNSUPPORTED;
}

/**
 * This function is called when BDS is ready to connect drivers.
 *
 * @param Event Event of callback
 * @param Context Context of callback.
 */
VOID 
BdsEvtFunc (
    IN EFI_EVENT Event,
    IN VOID      *Context )
{
    gEndOfDxe = TRUE;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************