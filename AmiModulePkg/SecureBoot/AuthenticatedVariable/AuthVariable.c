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

#include <Setup.h>
#include <../SecureBoot.h>
#include "AuthVariable.h"

//
// Global defines and variables
// 

BOOLEAN AVarRuntime = FALSE;

extern EFI_GUID  gEfiGlobalVariableGuid;
extern BOOLEAN PhysicalUserPresent ( VOID  ) ;
static EFI_GUID  mSignatureSupport[SIGSUPPORT_NUM] = {SIGSUPPORT_LIST};

UINT8  PublicKeyHashArray[HASH_SHA256_LEN];
UINT8  mPlatformMode = SETUP_MODE;
UINT8  mSetupMode = USER_MODE;
static UINT8  mSecureBootSupport = NONSECURE_BOOT;

AMI_DIGITAL_SIGNATURE_PROTOCOL *mDigitalSigProtocol = NULL;

static CHAR16* ReservedReadOnlyVarNameList[] = {
 EFI_SETUP_MODE_NAME,
 EFI_SECURE_BOOT_NAME,
 EFI_SIGNATURE_SUPPORT_NAME,
 EFI_IMAGE_SECURITY_DATABASE_DEFAULT, 
 EFI_IMAGE_SECURITY_DATABASE1_DEFAULT,
 EFI_PLATFORM_KEY_NAME_DEFAULT,
 EFI_KEY_EXCHANGE_KEY_NAME_DEFAULT,
 NULL
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AuthServiceVirtualFixup
//
// Description: This function will be invoked to convert
//              runtime pointers to virtual address
//
// Input:   none
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AuthServiceVirtualFixup()
{

    static BOOLEAN  bSigProtoVirtual = FALSE;
    VOID            **p;
    UINT8           i;

    //if mDigitalSigProtocol is NULL, nothing to fix up
    if (!mDigitalSigProtocol) return;
    
    // This function gets called from Nvramdxe.c, do nothing when 
    // the function is called second time.
    if (bSigProtoVirtual == TRUE) return;
    else bSigProtoVirtual = TRUE;
//AVAR_TRACE((-1,"AuthVarService mDigitalSig Virtual addr Fixup\n"));
    //Fixup mDigitalSigProtocol member functions
    for(i=0,p = (VOID**)mDigitalSigProtocol; p < (VOID**)(mDigitalSigProtocol + 1); i++,p++)
//    {
//AVAR_TRACE((-1,"mSigAPI[%d] before Virtual MemFixup = %lx (%lx), ", i, p));
        pRS->ConvertPointer(0, p);
//AVAR_TRACE((-1,"After = %lx\n", p));
//    }
    pRS->ConvertPointer(0,&mDigitalSigProtocol);
    AVarRuntime = TRUE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitSmmAuthServiceCallback
//
// Description: This function initialize mDigitalSigProtocol in SMM
//              
//
//  Input:      IN EFI_EVENT Event - Event that was triggered
//              IN VOID *Context - data pointer to information that is defined 
//              when the event is registered
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static EFI_STATUS InitSmmAuthServiceCallback (
            IN CONST EFI_GUID  *Protocol,
            IN VOID            *Interface,
            IN EFI_HANDLE      Handle
)
{
//    return pBS->LocateProtocol(
    return pSmst->SmmLocateProtocol(    
            &gAmiSmmDigitalSignatureProtocolGuid, NULL, &mDigitalSigProtocol
            );
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitSmmAuthService
//
// Description: This function calls InitSmmAuthServiceCallback to initialaze 
//              DigitalSigProtocol in SMM by trying to Locate
//              DigitalSigProtocol. If Protocol is not installed yet 
//              RegisterProtocolCallback will be called.
//              
//
//  Input:      None
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AuthVariableServiceInitSMM ()
{
//    EFI_EVENT       SmmHashEvent;
    VOID            *pSmm;
    EFI_STATUS    Status;
    
    // w/a: don't print debug traces from SMM
    AVarRuntime = TRUE;
    //
    // Authenticated variable initialize.
    //
    // Check PK database's existence to determine the value.
    // Then create  "SetupMode" with EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS set. 
    //
    // Update mPlatformMode  
    GetPlatformMode();

    // Update mSecureBootSupport global variable
    GetmSecureBootSupport(mPlatformMode);

//    Status = InitSmmAuthServiceCallback (NULL, NULL);
    Status = InitSmmAuthServiceCallback (NULL, NULL, NULL);
    if (EFI_ERROR(Status))
/*        
        RegisterProtocolCallback(
            &gAmiSmmDigitalSignatureProtocolGuid,
            InitSmmAuthServiceCallback,
            NULL,
            &SmmHashEvent,
            &pSmm
        );
*/
        pSmst->SmmRegisterProtocolNotify(
            &gAmiSmmDigitalSignatureProtocolGuid,
            InitSmmAuthServiceCallback,
            &pSmm
        );
}

//------------------------------------------------------------------------------
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitAuthServiceCallback
//
// Description: This function initialize mDigitalSigProtocol not in SMM
//              
//
//  Input:      IN EFI_EVENT Event - Event that was triggered
//              IN VOID *Context - data pointer to information that is defined 
//              when the event is registered
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static EFI_STATUS InitAuthServiceCallback (IN EFI_EVENT Event, IN VOID *Context)
{
    return pBS->LocateProtocol(
            &gAmiDigitalSignatureProtocolGuid, NULL, &mDigitalSigProtocol
            );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AuthVariableServiceInit
//
// Description: This function calls InitAuthServiceCallback to initialaze 
//              DigitalSigProtocol not in SMM by trying to Locate
//              DigitalSigProtocol. If Protocol is not installed yet 
//              RegisterProtocolCallback will be called.
//              
//
//  Input:      None
//
// Output:      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AuthVariableServiceInit ()
{
    EFI_EVENT       Event;
    VOID            *p;
    UINT8           Data;

    //
    // Create "SignatureSupport" with EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS set. 
    //
    DxeSetVariable(EFI_SIGNATURE_SUPPORT_NAME,&gEfiGlobalVariableGuid,
        EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
        SIGSUPPORT_NUM * sizeof(EFI_GUID), &mSignatureSupport
    );
    //
    // Check presence of PK database to determine the value.
    // Then create  "SetupMode" with EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS set.   
    // Update mPlatformMode  
    GetPlatformMode();

    // Update mSecureBootSupport global variable
    GetmSecureBootSupport(mPlatformMode);

    // Set SecureBoot, both conditions must be met 
    Data = (mPlatformMode==USER_MODE && mSecureBootSupport==SECURE_BOOT)?1:0;
    DxeSetVariable(EFI_SECURE_BOOT_NAME,&gEfiGlobalVariableGuid, 
        EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, 
        sizeof(UINT8), &Data);
   
    // Create "SetupMode" variable with EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS set
    UpdatePlatformMode(mPlatformMode);

    if (EFI_ERROR(InitAuthServiceCallback (NULL, NULL)))
        RegisterProtocolCallback(
            &gAmiDigitalSignatureProtocolGuid,
            InitAuthServiceCallback,
            NULL,
            &Event,
            &p
        );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   StrSize16
//
//  Description:
//  This function returns UNICODE string size in bytes 
//
//  Input:
//  CHAR16 *String - Pointer to string
//
//  Output:
//  UINT32 - Size of string in bytes excluding the nul-terminator
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 StrSize16(CHAR16 *String)
{
    UINT32 Size = 0;//2;
    while(*String++)
        Size += 2;
    return Size;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   StrCmp16
//
//  Description:
//  This function compares two UNICODE strings 
//
//  Input:
//     IN CHAR16 *Dest - Pointer to destination string
//     IN CHAR16 *Src - Pointer to source string
//
//  Output:
//     INTN - Zero if strings are equal, non-zero otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
INTN StrCmp16(CHAR16 *Dest, CHAR16 *Src)
{
    return MemCmp(Dest, Src, StrSize16(Src));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   IsPkVar
//
//  Description:
//  This function compares two UNICODE strings 
//
//  Input:
//     CHAR16   - Pointer to UNICODE Variable Name
//     EFI_GUID - Pointer to Variable GUID
//
//  Output:
//     BOOLEAN - TRUE if strings are equal
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsPkVar(
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid
)
{
    return ((guidcmp(VendorGuid, &gEfiGlobalVariableGuid) == 0) && 
            (StrCmp16 ((CHAR16 *)VariableName, (CHAR16 *)EFI_PLATFORM_KEY_NAME) == 0));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   IsKekVar
//
//  Description:
//  This function compares two UNICODE strings 
//
//  Input:
//     CHAR16   - Pointer to UNICODE Variable Name
//     EFI_GUID - Pointer to Variable GUID
//
//  Output:
//     BOOLEAN - TRUE if strings are equal
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsKekVar(
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid
)
{
    return ((guidcmp(VendorGuid, &gEfiGlobalVariableGuid) == 0) && 
            (StrCmp16 ((CHAR16 *)VariableName, (CHAR16 *)EFI_KEY_EXCHANGE_KEY_NAME) == 0));
}    


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsReservedVariableName
//
// Description: 
//     Check if Variable name matches one from Predefined Read-Only Variable Name list
//
// Input:
//     CHAR16   - Pointer to UNICODE Variable Name
//     EFI_GUID *VendorGuid - Variable GUID
//
// Output:      
//     BOOLEAN 
//     TRUE if Variable name found in the Predefined Read-Only Variable Name lists
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
IsReservedVariableName(
    CHAR16 *VariableName,
    EFI_GUID *VendorGuid
)
{
    UINT8   Index;
    Index = 0;

    while(ReservedReadOnlyVarNameList[Index] != NULL)
    {
        if(StrCmp16 (VariableName, ReservedReadOnlyVarNameList[Index]) == 0)
        {
// Reserved Secure Boot RO variables are gEfiGlobalVariableGuid.
            return (guidcmp(VendorGuid, &gEfiGlobalVariableGuid) == 0);
        }
        Index++;
    }
// Not yet implemented. Need eLink defined lists and a make file to pass the Oem Var names
//    Index = 0;
//    while(OemReadOnlyVariableList[Index] != NULL)
//    {
//        if(StrCmp16 (VariableName, OemReadOnlyVariableList[Index]) == 0)
//           return TRUE;
//        Index++;
//    }

    return FALSE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   IsDbVar
//
//  Description:
//  This function compares two UNICODE strings 
//
//  Input:
//     IN CHAR16 *Dest - Pointer to destination string
//     IN CHAR16 *Src - Pointer to source string
//
//  Output:
//  INTN - Zero if strings are equal, non-zero otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsDbVar(
    IN EFI_GUID *VendorGuid
)
{
    return (!guidcmp (VendorGuid, &gEfiImageSecurityDatabaseGuid));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetPlatformMode
//
// Description: 
//              Initializes for authenticated varibale service.
//
// Input:
//              UINT8 *PlatformMode
//
// Output:      EFI_STATUS Function successfully executed.
//              UINT8 *PlatformMode
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetPlatformMode (  
    VOID
)
{
    EFI_STATUS  Status;
    UINT32      VarAttr;
    UINTN       VarDataSize;
    VOID        *Data;
    
    mPlatformMode = USER_MODE;

    //
    // Check PK database's existence to determine the value.
    //
    VarDataSize = 0;
    Status = FindVariable (
             EFI_PLATFORM_KEY_NAME, 
             &gEfiGlobalVariableGuid, 
             &VarAttr, 
             &VarDataSize,
             &Data
    );
    if (Status != EFI_SUCCESS || Data == NULL || VarDataSize == 0 ||
      !(VarAttr & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES)
    ) {
        mPlatformMode = SETUP_MODE;
    }
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetmSecureBootSupport
//
// Description: 
//              Initializes for authenticated varibale service.
//
// Input:
//              SetupMode Disable SecureBoot control if Platform runs in SetupMode (no PK)
//          
// Output:      EFI_STATUS Function successfully executed.
//              EFI_OUT_OF_RESOURCES  Fail to allocate enough memory resources.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetmSecureBootSupport (
    UINT8 SetupMode
)
{
    EFI_STATUS Status  = EFI_SUCCESS;
#if defined(ImageVerification_SUPPORT) && ImageVerification_SUPPORT==1
    EFI_GUID  gSecureSetupGuid = SECURITY_FORM_SET_GUID;
    SECURE_BOOT_SETUP_VAR   SecureBootSetup;
    UINTN      DataSize=0;
    UINT32     Attributes;
    //
    // Get Setup variable, check SecureBoot and set the EFI Var
    //
    Status = DxeGetVariable(
        AMI_SECURE_BOOT_SETUP_VAR,
        &gSecureSetupGuid,
        &Attributes,
        &DataSize,
        &SecureBootSetup
    );
    if(Status == EFI_BUFFER_TOO_SMALL)
        Status = DxeGetVariable(
            AMI_SECURE_BOOT_SETUP_VAR,
            &gSecureSetupGuid,
            &Attributes,
            &DataSize,
            &SecureBootSetup
        );
    ASSERT_EFI_ERROR (Status);
    mSecureBootSupport = (SecureBootSetup.SecureBootSupport);
// Disable SecureBoot Setup Option if system is in Setup mode 
#if DEFAULT_SECURE_BOOT_ENABLE == 0
    if(mSecureBootSupport == 1 &&
    (SetupMode == SETUP_MODE && SecureBootSetup.DefaultKeyProvision == 0)
       )
    {
        mSecureBootSupport = NONSECURE_BOOT;
    }
#endif
#else
    mSecureBootSupport = NONSECURE_BOOT;
#endif
  
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdatePlatformMode
//
//- also set up secureBoot based on mSecureBoot + SetupMode
//
// Input:       Mode  SETUP_MODE or USER_MODE.
//
// Output:      none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
UpdatePlatformMode( 
    IN  UINT8 Mode
)
{
AVAR_TRACE((TRACE_ALWAYS,"Update Setup Mode\nCurrent=%x, New=%x\n", mPlatformMode, Mode));
    //
    // update global mPlatformMode var
    //
    mPlatformMode = Mode;
    //
    // Set "SetupMode" variable.
    //
    // Setting of mSetupMode=SETUP_MODE will un-lock access to runtime R/O vars
    mSetupMode = SETUP_MODE;

    DxeSetVariable(EFI_SETUP_MODE_NAME, &gEfiGlobalVariableGuid, 
            EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
            sizeof(UINT8), &mPlatformMode);

// Re-lock access to runtime protected vars
    mSetupMode = USER_MODE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ValidateSignatureList
//
// Description: 
//              Validate the data payload begins with valid Signature List header
//              and based on the results returns Status.
//
// Input:
//              IN VOID *Data - pointer to the Var data
//              IN UINTN DataSize - size of Var data
//
// Output:      EFI_STATUS
//              UINTN RealDataSize - only the size of the combined length of Signature Lists
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ValidateSignatureList (
    IN VOID         *Data,
    IN UINTN        DataSize
)
{
    EFI_STATUS            Status;
    EFI_SIGNATURE_LIST   *SigList;
    UINTN                 Index;

    Status = EFI_SECURITY_VIOLATION;

    if(Data == NULL || DataSize == 0)
        return Status; // Sig not found

    SigList  = (EFI_SIGNATURE_LIST *)Data;

// loop till end of DataSize for all available SigLists

// Verify signature is one from SigDatabase list mSignatureSupport / sizeof(EFI_GUID)
// SigData begins with SigOwner GUID
// SignatureHdrSize = 0 for known Sig Types

    while ((DataSize > 0) && (DataSize >= SigList->SignatureListSize)) {

        for (Index = 0; Index < SIGSUPPORT_NUM; Index++) {
            if (!guidcmp ((EFI_GUID*) &(SigList->SignatureType), &mSignatureSupport[Index]))
                break;
        }
AVAR_TRACE((TRACE_ALWAYS,"SigList.Type-"));
        if(Index >= SIGSUPPORT_NUM)
            return EFI_SECURITY_VIOLATION; // Sig not found

AVAR_TRACE((TRACE_ALWAYS,"OK\nSigList.Size-"));
        if(SigList->SignatureListSize < 0x4c || // Min size for SHA2 Hash Certificate sig list
           SigList->SignatureListSize > NVRAM_SIZE)
            return EFI_SECURITY_VIOLATION; 

AVAR_TRACE((TRACE_ALWAYS,"OK\nSigList.HdrSize-"));
        if(SigList->SignatureHeaderSize != 0)
            return EFI_SECURITY_VIOLATION; // Sig not found
AVAR_TRACE((TRACE_ALWAYS,"OK\n"));
        DataSize -= SigList->SignatureListSize;
        SigList = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigList + SigList->SignatureListSize);

        Status = EFI_SUCCESS;
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessVarWithPk
//
// Description: Process variable with platform key for verification.
//
//
// Input:
//          VariableName                Name of Variable to be found.
//          VendorGuid                  Variable vendor GUID.
//          Data                        Data pointer.
//          DataSize                    Size of Data found. If size is less than the
//                                      data, this value contains the required size.
//          Variable                    The variable information which is used to 
//                                      keep track of variable usage.
//          Attributes                  Attribute value of the variable
//          IsPk                        Indicate whether it is to process pk.
//
// Output:      EFI_SUCCESS             Variable passed validation successfully.
//              EFI_INVALID_PARAMETER   Invalid parameter.
//              EFI_SECURITY_VIOLATION  The variable does NOT pass the validation. 
//                                      check carried out by the firmware. 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
ProcessVarWithPk (
  IN  VOID               *Data,
  IN  UINTN              DataSize,
  IN  UINT32             Attributes ,
  IN  BOOLEAN            IsPk
  )
{
  EFI_STATUS              Status = EFI_SUCCESS;
  EFI_SIGNATURE_LIST     *OldPkList;
  EFI_SIGNATURE_DATA     *OldPkData;
  UINT32                  VarAttr;
  UINT8                  *VarData;
  UINTN                   VarDataSize=0;

  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // PK and KEK should set EFI_VARIABLE_NON_VOLATILE attribute.
    //
//AVAR_TRACE((TRACE_ALWAYS,"PK NOT_VOLITILE\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (mPlatformMode == USER_MODE)
  {

    //
    // Get platform key from variable.
    //
    Status = FindVariable (
               EFI_PLATFORM_KEY_NAME,
               &gEfiGlobalVariableGuid,
               &VarAttr,
               &VarDataSize,
               &VarData
               );
// PK should have been set when we were in SETUP_MODE. This condition is INVALID.
    if (EFI_ERROR (Status) || VarData==NULL || !VarDataSize)
        return EFI_SECURITY_VIOLATION;

    OldPkList = (EFI_SIGNATURE_LIST *) VarData;
    OldPkData = (EFI_SIGNATURE_DATA *) ((UINT8 *) OldPkList + sizeof (EFI_SIGNATURE_LIST) + OldPkList->SignatureHeaderSize);

// Authenticate
    Status    = VerifyDataPayload (Data, DataSize, OldPkData->SignatureData);
//AVAR_TRACE((TRACE_ALWAYS,"PK VerifyPayload %r\n",Status));

 }

  return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessVarWithKek
//
// Description: Process variable with key exchange key for verification.
//
//
// Input:
//          Data                        Data pointer.
//          DataSize                    Size of Data found. If size is less than the
//                                      data, this value contains the required size.
//          Variable                    The variable information which is used to 
//                                      keep track of variable usage.
//          Attributes                  Attribute value of the variable
//
// Output:      EFI_SUCCESS             Variable passed validation successfully.
//              EFI_INVALID_PARAMETER   Invalid parameter.
//              EFI_SECURITY_VIOLATION  The variable does NOT pass the validation. 
//                                      check carried out by the firmware. 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
ProcessVarWithKek (
  IN  VOID         *Data,
  IN  UINTN        DataSize, 
  IN  UINT32       Attributes 
  )
{
// so far can be DB or DBx variables

  EFI_STATUS                      Status = EFI_SUCCESS;
  EFI_SIGNATURE_LIST              *KekList;
  EFI_SIGNATURE_DATA              *KekItem;
  UINT32                          KekCount;
  EFI_VARIABLE_AUTHENTICATION     *CertData;
  EFI_CERT_BLOCK_RSA_2048_SHA256  *CertBlock;
//  BOOLEAN                         IsFound;
  UINT32                          Index;
  UINT32                  VarAttr;
  UINT8                   *VarData;
  UINTN                   VarDataSize = 0;

  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // PK and KEK should set EFI_VARIABLE_NON_VOLATILE attribute.
    //
    return EFI_INVALID_PARAMETER;
  }

//
// If in setup mode, no authentication needed.
//
  if (mPlatformMode == USER_MODE)
  {

    CertData  = (EFI_VARIABLE_AUTHENTICATION *) Data;
    CertBlock = (EFI_CERT_BLOCK_RSA_2048_SHA256 *)&(CertData->AuthInfo.CertData);
    //
    // Get Key database from KEK variable.
    //
    Status = FindVariable (
               EFI_KEY_EXCHANGE_KEY_NAME, 
               &gEfiGlobalVariableGuid, 
               &VarAttr, 
               &VarDataSize,
               &VarData
               );

    KekList     = (EFI_SIGNATURE_LIST *) VarData;
    if (EFI_ERROR(Status) || guidcmp ((EFI_GUID*) &(KekList->SignatureType), &gEfiCertRsa2048Guid))
      return EFI_SECURITY_VIOLATION;
    //
    // Enumerate all Kek items in this list to verify the variable certificate data.
    // If anyone is authenticated successfully, it means the variable is correct!
    //
//    IsFound   = FALSE;
//
// scan thru multiple Sig Lists if exist. Add 1 more loop....
// actually, KEK would have one list since KEK owner may club all Kek keys together before signing with PK
//    do {
        KekCount  = (KekList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - KekList->SignatureHeaderSize) / KekList->SignatureSize;
        KekItem   = (EFI_SIGNATURE_DATA *) ((UINT8 *) KekList + sizeof (EFI_SIGNATURE_LIST) + KekList->SignatureHeaderSize);
        for (Index = 0; Index < KekCount; Index++) {
          if (MemCmp (KekItem->SignatureData, CertBlock->PublicKey, EFI_CERT_TYPE_RSA2048_SIZE) == 0) {
//            IsFound = TRUE;
            break;
          }
          KekItem = (EFI_SIGNATURE_DATA *) ((UINT8 *) KekItem + KekList->SignatureSize);
        }
//        KekList = (EFI_SIGNATURE_LIST *) ((UINT8 *) KekList + KekList->SignatureListSize);
//    } while (!IsFound || (UINT8*)KekList < (VarData+VarDataSize));
//    if (!IsFound) {
   if (Index >= KekCount) {
      return EFI_SECURITY_VIOLATION;
    }
// Authenticate
    Status = VerifyDataPayload (Data, DataSize, CertBlock->PublicKey);
//AVAR_TRACE((TRACE_ALWAYS,"KEK VerifyPayload %r\n",Status));
  }

  return Status;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VerifyDataPayload
//
// Description: 
//              Verify data payload with AuthInfo in EFI_CERT_TYPE_RSA2048_SHA256 type.
//              Follow the steps in UEFI2.2. This function does signature 
//              authetication and based on the results returns Status.
//
// Input:
//              IN VOID *Data - pointer to the Var data
//              IN UINTN DataSize - size of Var data
//              UINT8 *PubKey - PublicKey used for Sig verification.
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VerifyDataPayload (
    IN VOID *Data,
    IN UINTN DataSize, 
    IN UINT8 *PubKey
)
{
    EFI_STATUS Status;
    EFI_VARIABLE_AUTHENTICATION     *CertData;
    EFI_CERT_BLOCK_RSA_2048_SHA256  *CertBlock;
    UINT8 Digest[HASH_SHA256_LEN];
    UINT8 *Addr[2];
    UINTN   Len[2];
    UINTN   CertHdrSize;

    CRYPT_HANDLE   PublicKey;
    CRYPT_HANDLE   HashToVerify;

    CertData  = (EFI_VARIABLE_AUTHENTICATION *)Data;
//    CertHdrSize = AUTHINFO_SIZE(Data);
// SCT 2.3.1. TW UEFI Plugfest BUG with listing of a Cert Size in Cert->AuthInfo.Hdr.dwLength
    CertHdrSize = 0x230;

    CertBlock = (EFI_CERT_BLOCK_RSA_2048_SHA256*)&(CertData->AuthInfo.CertData);

    //
    // Hash data payload with SHA256.
    //
    Addr[0] = (UINT8*) Data + (CertHdrSize) ;
    Len[0] = DataSize - (CertHdrSize);
    Addr[1] = (UINT8*)&(CertData->MonotonicCount);
    Len[1] = sizeof(UINT64);
    Status = mDigitalSigProtocol->Hash(mDigitalSigProtocol, 
                            &gEfiHashAlgorithmSha256Guid,
                            2, Addr, Len, (UINT8*)&Digest[0]);
    if (EFI_ERROR(Status)) 
        return Status;

    PublicKey.AlgGuid = gEfiCertRsa2048Guid;
    PublicKey.BlobSize = EFI_CERT_TYPE_RSA2048_SIZE;
    PublicKey.Blob = PubKey;//(UINT8*)&(CertData->AuthInfo.CertData.PublicKey);

    HashToVerify.AlgGuid = gEfiHashAlgorithmSha256Guid;
    HashToVerify.BlobSize = SHA256_DIGEST_SIZE;
    HashToVerify.Blob = &Digest[0];

    Status = mDigitalSigProtocol->Pkcs1Verify(
            mDigitalSigProtocol,
            &PublicKey,
            &HashToVerify,
            (UINT8*)&(CertBlock->Signature), 
            EFI_CERT_TYPE_RSA2048_SHA256_SIZE, EFI_CRYPT_RSASSA_PKCS1V15/*EFI_CRYPT_RSASSA_PSS*/);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindInSignatureDb 
//
// Description: 
//              For variables with GUID EFI_IMAGE_SECURITY_DATABASE_GUID 
//              (i.e. where the data buffer is formatted as EFI_SIGNATURE_LIST), 
//              the driver shall not perform an append of EFI_SIGNATURE_DATA values 
//              that are already part of the existing variable value (Note: This situation 
//              is not considered an error, and shall in itself not cause a status code other 
//              than EFI_SUCCESS to be returned or the timestamp associated with the variable not 
//              to be updated).
//
// Input:
//              EFI_GUID *VendorGuid   Variable vendor GUID.
//              IN UINT32 Attributes - Attributes of the Var
//              VOID  *Data - pointer to data block within AutVar Data
//              UINTN *DataSize - ptr to size of data block
//              VOID  *SigDb - current SigDb
//              UINTN  SigDbSize 
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FindInSignatureDb (
    IN EFI_GUID *VendorGuid,
    IN UINT32    Attributes,
    IN VOID     *Data,
    IN OUT UINTN    *DataSize,
    IN VOID     *SigDb,
    IN UINTN    SigDbSize
){
    EFI_SIGNATURE_LIST             *SigList;
    EFI_SIGNATURE_LIST             *SigListNew;
    EFI_SIGNATURE_DATA             *SigItem;
    EFI_SIGNATURE_DATA             *SigItemNew;
    UINT32                          SigCount;
    UINT32                          Index;
    UINT32                          SigCountNew;
    UINT32                          IndexNew;

    UINTN                          SigNewSize;
    BOOLEAN                        bSigMatch;
    // Is Append & SigDB
    if ( (SigDb && SigDbSize) &&
        ((Data != SigDb) && (Attributes & EFI_VARIABLE_APPEND_WRITE)) &&
        // Validate Signature List integrity 
        !EFI_ERROR(ValidateSignatureList (Data, *DataSize))
    ) {
        SigList     = (EFI_SIGNATURE_LIST *)SigDb;
        SigListNew  = (EFI_SIGNATURE_LIST *)Data;
AVAR_TRACE((TRACE_ALWAYS,"FindInDB(x)\nDataSize In %d (0x%X)\n",*DataSize,*DataSize));
        //
        // Enumerate all Sig items in this list to verify the variable certificate data.
        //
        //
        // scan through multiple Sig Lists in DB exist.
        while ((SigDbSize > 0) && (SigDbSize >= SigList->SignatureListSize)) {
            SigCount  = (SigList->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - SigList->SignatureHeaderSize) / SigList->SignatureSize;
            SigItem   = (EFI_SIGNATURE_DATA *) ((UINT8 *) SigList + sizeof (EFI_SIGNATURE_LIST) + SigList->SignatureHeaderSize);
//AVAR_TRACE((TRACE_ALWAYS,"Org SigList Count: %d, SigSize %X\n", SigCount, SigList->SignatureSize));
            // scan through multiple Sig Lists in NewSigList.
            for (Index = 1; Index <= SigCount; Index++) {
////AVAR_TRACE((TRACE_ALWAYS,"OrgCert %d, Data %X\n",Index, *(UINT32*)SigItem->SignatureData));
                SigListNew  = (EFI_SIGNATURE_LIST *)Data;
                SigNewSize  = *DataSize;
                while ((SigNewSize > 0) && (SigNewSize >= SigListNew->SignatureListSize)) {
                    bSigMatch = FALSE;
                    SigItemNew = (EFI_SIGNATURE_DATA *) ((UINT8 *) SigListNew + sizeof (EFI_SIGNATURE_LIST) + SigListNew->SignatureHeaderSize);
                    SigCountNew  = (SigListNew->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - SigListNew->SignatureHeaderSize) / SigListNew->SignatureSize;
                    if (!guidcmp ((EFI_GUID*) &(SigList->SignatureType), (EFI_GUID*)&(SigListNew->SignatureType)) && 
                        SigList->SignatureSize == SigListNew->SignatureSize) {
//AVAR_TRACE((TRACE_ALWAYS,"New SigDb Size %X\nNew SigList Count: %d, SigSize %X\n", SigNewSize, SigNewCount, SigNewList->SignatureSize));
                      // loop through all instances of NewSigList->SigData. 
                      for (IndexNew = 1; IndexNew <= SigCountNew; IndexNew++) {
////AVAR_TRACE((TRACE_ALWAYS,"NewCert %d, Data %X\n",IndexNew, *(UINT32*)SigNewItem->SignatureData));
                            if (MemCmp (SigItem->SignatureData, SigItemNew->SignatureData, SigList->SignatureSize-sizeof(EFI_GUID)) == 0) {
//AVAR_TRACE((TRACE_ALWAYS,"---> match found!!!\n"));
//AVAR_TRACE((TRACE_ALWAYS,"OrgCert %4d, Data %X\n",Index, *(UINT32*)SigItem->SignatureData));
//AVAR_TRACE((TRACE_ALWAYS,"NewCert %4d, Data %X\n",IndexNew, *(UINT32*)SigNewItem->SignatureData));
                               if(SigCountNew == 1) {
                                // only 1 SigData per SigList - discard this SigList
                                      bSigMatch = TRUE;
//AVAR_TRACE((TRACE_ALWAYS,"Before: DataSize=%x\nAfter : DataSize=%x\n", *DataSize, *DataSize-SigNewList->SignatureListSize));
                                      // 1. Decrease *Datasize by SigNewList->SignatureSize 
                                      SigNewSize -= SigListNew->SignatureListSize;
                                      *DataSize -= SigListNew->SignatureListSize;
                                      // 2. replace this SigData block with data following it
                                      MemCpy (SigListNew, (void*)((UINTN)SigListNew+SigListNew->SignatureListSize), SigNewSize); 
                                      // 3. Skip to next SigListNew    
                                      break;
                                } else {
                                // more then 1 - discard this SigData
                                    // 1. replace this SigData block with data following it
                                    MemCpy (SigItemNew, (void*)((UINTN)SigItemNew+SigListNew->SignatureSize), ((UINTN)Data+*DataSize)-((UINTN)SigItemNew+SigListNew->SignatureSize));
                                    // 2. Decrease SigNewList->SignatureListSize by SigNewList->SignatureSize 
                                    SigListNew->SignatureListSize-=SigListNew->SignatureSize;
                                    *DataSize-=SigListNew->SignatureSize;
//AVAR_TRACE((TRACE_ALWAYS,"Upd SignatureListSize=%x, DataSize=%x\n",SigNewList->SignatureListSize, *DataSize));
                                    // 3. If this is last SigData element
                                    if((SigListNew->SignatureListSize - sizeof (EFI_SIGNATURE_LIST) - SigListNew->SignatureHeaderSize)==0)
                                    {  
//AVAR_TRACE((TRACE_ALWAYS,"SigList is Empty!\n"));
                                        break;
                                    }         
                                    // 4. Skip incrementing of SigNewItem
                                    continue;
                                }
                            } // if cmp 
                            SigItemNew = (EFI_SIGNATURE_DATA *) ((UINT8 *) SigItemNew + SigListNew->SignatureSize);
                        } // for SigNewItem 
                    } // if guid
                    // Skip incrementing of SigNewList if bSigListMatch is found - we already on next siglist
                    if(!bSigMatch) {
                        SigNewSize -= SigListNew->SignatureListSize;
                        SigListNew = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigListNew + SigListNew->SignatureListSize);
                    }
                } // while SigNewList
                SigItem = (EFI_SIGNATURE_DATA *) ((UINT8 *) SigItem + SigList->SignatureSize);
            } // for SigItem
            SigDbSize -= SigList->SignatureListSize;
            SigList = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigList + SigList->SignatureListSize);
        } // while SigList 

AVAR_TRACE((TRACE_ALWAYS,"DataSize Out: %d (0x%X)\n",*DataSize, *DataSize));
    if(*DataSize==0)
        return EFI_ALREADY_STARTED;

AVAR_TRACE((TRACE_ALWAYS,"APPEND OK!\n"));
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VerifyVariable1
//
// Description: 
//              Verify data payload with AuthInfo in EFI_CERT_TYPE_RSA2048_SHA256 type.
//              Follow the steps in UEFI2.2.
//              This function is called every time variable with 
//              EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS attribute is 
//              created, updated or deleted. This function does all necessary 
//              authetication checks and based on the results returns Status.
//              Also it returns the Mc Hash of PublicKey from Variable's AuthInfo Hdr
//
// Input:
//              CHAR16 *VariableName  Name of Variable to be found.
//              EFI_GUID *VendorGuid   Variable vendor GUID.
//              IN UINT32 Attributes - Attributes of the Var
//              VOID **Data - pointer to data block within AutVar Data
//              UINTN *DataSize - size of data block
//              VOID  *OldData - pointer to Existing in NVRAM data block 
//              UINTN  OldDataSize - size of data block
//              UINT64 ExtFlags.MonotonicCount - value of MC or TIME stamp 
//              UINT8  ExtFlags.KeyHash[32] - pointer to memory, allocated by caller, 
//                     where Hash of Public Key will be returned.
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VerifyVariable1 (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32    Attributes,
    IN VOID    **Data,
    IN UINTN    *DataSize, 
    IN VOID     *OldData,
    IN UINTN     OldDataSize,
    IN OUT EXT_SEC_FLAGS *ExtFlags
){
    EFI_STATUS Status;

    VOID        *RealData;
    EFI_VARIABLE_AUTHENTICATION *CertData;
    EFI_CERT_BLOCK_RSA_2048_SHA256   *CertBlock;
    UINT8       *PubKey, *PubKeyHash;
    UINTN       CertHdrSize;
    UINTN       Size;
    BOOLEAN     IsSigListVar; 
    BOOLEAN     IsPk;

    RealData = *Data;
    Status = EFI_SUCCESS;
    IsSigListVar = TRUE;
    IsPk = FALSE;

// must have Auth attribute to go deeper
    if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) == 0)
        return EFI_INVALID_PARAMETER;

    if(*DataSize < sizeof(EFI_VARIABLE_AUTHENTICATION))
        return EFI_SECURITY_VIOLATION;

// specific for EFI_VARIABLE_AUTHENTICATION mode variables
//--->>>
    CertData  = (EFI_VARIABLE_AUTHENTICATION *) *Data;
    CertBlock = (EFI_CERT_BLOCK_RSA_2048_SHA256*)&(CertData->AuthInfo.CertData);
// SCT 2.3.1 TW Plugfest BUG with listing of a Cert Size in Cert->AuthInfo.Hdr.dwLength
// 0x22b vs 0x230
//    CertHdrSize = AUTHINFO_SIZE(CertData);//(CertData->AuthInfo.Hdr.Hdr.dwLength + sizeof(CertData->MonotonicCount));
    AVAR_TRACE((TRACE_ALWAYS,"VerifyVariable CertHdr Size 0x%x (expected(0x230)\n",AUTHINFO_SIZE(CertData)));

    CertHdrSize = 0x230;

    //
    // wCertificateType should be WIN_CERT_TYPE_EFI_GUID.
    // Cert type should be EFI_CERT_TYPE_RSA2048_SHA256.
    //
    if ((CertData->AuthInfo.Hdr.wCertificateType != WIN_CERT_TYPE_EFI_GUID) ||  
        guidcmp ((EFI_GUID*) &(CertData->AuthInfo.CertType), &gEfiCertTypeRsa2048Sha256Guid)
    ) {
        //
        // Invalid AuthInfo type, return EFI_SECURITY_VIOLATION.
        //
        AVAR_TRACE((TRACE_ALWAYS,"VerifyVariable AuthHdr GUID test Fails\nWinCert_Type\nExpected  %x\nReceived %x\nGUID\nExpected  %g\nReceived %g\n",WIN_CERT_TYPE_EFI_GUID, CertData->AuthInfo.Hdr.wCertificateType, gEfiCertTypeRsa2048Sha256Guid, (CertData->AuthInfo.CertType)));
// SCT 2.3.1 TW Plugfest uses wrong GUID
            return EFI_SECURITY_VIOLATION;
    } 

    if(*DataSize < CertHdrSize)
    {
AVAR_TRACE((TRACE_ALWAYS,"VerifyVariable DataSize test fails: DataSize(%x) < AuthHdrSize (%x)\n", *DataSize, CertHdrSize));
        return EFI_SECURITY_VIOLATION;
    }
    //
    // Monotonic count check fail, suspicious replay attack, return EFI_SECURITY_VIOLATION.
    //
AVAR_TRACE((TRACE_ALWAYS,"Check MC:\nOld=%x\nNew=%x\n",ExtFlags->Mc, CertData->MonotonicCount));
    if (((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) &&
        (OldData && ExtFlags->Mc >= CertData->MonotonicCount)
    ) {
        AVAR_TRACE((TRACE_ALWAYS,"Failed\n"));
        return EFI_SECURITY_VIOLATION;
    }
AVAR_TRACE((TRACE_ALWAYS,"Pass\n"));

// AppendWrite: Only update Timestamp if New one is greater then current
    if( (Attributes & EFI_VARIABLE_APPEND_WRITE) && 
        (OldData && ExtFlags->Mc > CertData->MonotonicCount)
       );
    else
        ExtFlags->Mc = CertData->MonotonicCount;
    //
    // Process PK, KEK, Sig db separately.
    //
    if (IsPkVar(VariableName, VendorGuid)){
      IsPk = TRUE;
      Status = ProcessVarWithPk (*Data, *DataSize, Attributes, TRUE);
    } else if (IsKekVar(VariableName, VendorGuid)) {
      Status = ProcessVarWithPk (*Data, *DataSize, Attributes, FALSE);
    } else if (IsDbVar(VendorGuid)) {
      Status = ProcessVarWithKek (*Data, *DataSize, Attributes);
      // verify process db(x) with one of KEK keys or if not found within KEK - with PK
      if (Status == EFI_SECURITY_VIOLATION) 
        Status = ProcessVarWithPk (*Data, *DataSize, Attributes, FALSE);
    } else {
        IsSigListVar = FALSE;
        PubKey = &(CertBlock->PublicKey[0]);
        PubKeyHash = &PublicKeyHashArray[0];
        // Verify SelfSigned variable is signed with a valid Key
        Status = VerifyDataPayload (*Data, *DataSize, PubKey);
        if (!EFI_ERROR(Status)) {

            Size = RSA2048_PUB_KEY_LEN;
            Status = mDigitalSigProtocol->Hash(mDigitalSigProtocol, 
                        &gEfiHashAlgorithmSha256Guid,
                        1, &PubKey, (const UINTN*)&Size, PubKeyHash); 

            if (OldData && MemCmp(&ExtFlags->KeyHash[0], PubKeyHash, HASH_SHA256_LEN)){
//AVAR_TRACE((TRACE_ALWAYS,"Self Signed MC Var Key Compare FAILED!\n"));
                return EFI_SECURITY_VIOLATION;
            }
            // Setting key Hash for self signed variables
            MemCpy(&ExtFlags->KeyHash[0], PubKeyHash, HASH_SHA256_LEN);
        }
    }
    if (EFI_ERROR(Status))  
        return EFI_SECURITY_VIOLATION;

    *DataSize   =  *DataSize - CertHdrSize;
    *Data       = (UINT8*)RealData + CertHdrSize;

    if(IsSigListVar == TRUE) {

        // Validate Signature List integrity 
        if(*DataSize && EFI_ERROR(ValidateSignatureList (*Data, *DataSize)))
            return EFI_SECURITY_VIOLATION;
        //
        // If delete PK in user mode -> change to setup mode.
        // If enroll PK in setup mode -> change to user mode.
        //
        if(IsPk) {
            if (*DataSize == 0) 
                UpdatePlatformMode (SETUP_MODE);
            else
                UpdatePlatformMode (USER_MODE);
        }
    }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VerifyVariable
//
// Description: 
//              This function is called every time variable with 
//              EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS and 
//              EFI_VARIABLE_AUTHENTICATED_TIME_BASED_ACCESS attributes 
//              created, updated or deleted. This function does all necessary 
//              Authentication checks and based on the results returns Status.
//              Also it returns the Mc Hash of PublicKey from Variable's AuthInfo Hdr
//
// Input:
//              CHAR16 *VariableName  Name of Variable to be found.
//              EFI_GUID *VendorGuid   Variable vendor GUID.
//              IN UINT32 Attributes - Attributes of the Var
//              VOID **Data - pointer to data block within AutVar Data
//              UINTN *DataSize - size of data block
//              VOID  *OldData - pointer to Existing in NVRAM data block 
//              UINTN  OldDataSize - size of data block
//              UINT64 ExtFlags.MonotonicCount - value of MC or TIME stamp 
//              UINT8  ExtFlags.KeyHash[32] - pointer to memory, allocated by caller, 
//                     where Hash of Public Key will be returned.
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VerifyVariable (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32   *Attributes,
    IN VOID    **Data,
    IN UINTN    *DataSize,
    IN VOID     *OldData,
    IN UINTN     OldDataSize,
    IN OUT EXT_SEC_FLAGS *ExtSecFlags
){
    EFI_STATUS  Status;
    UINT32      AuthAttributes; 
    UINT64      OldMC;

    Status = EFI_SUCCESS; 

    // bypass var RO check when updating 
    // SetupMode and SecureBoot variables after changing of a PK
    if(mSetupMode == SETUP_MODE) {
// Faking presense of NV attribute for SetupMode in order to meet UEFI requirement 
// to display SetupMode state even in runtime (after exit boot services)
//        AuthAttributes = *Attributes & (~EFI_VARIABLE_NON_VOLATILE);
//        *Attributes = AuthAttributes;
//        *Attributes &= (UINT32)(~EFI_VARIABLE_NON_VOLATILE);
        // Re-lock access to runtime protected vars
        mSetupMode = USER_MODE;
        return Status;//EFI_SUCCESS;
    }

    // existing reserved variables are RO!!!
    if(OldData && OldDataSize && IsReservedVariableName(VariableName, VendorGuid))
        return EFI_WRITE_PROTECTED;

    AuthAttributes = ExtSecFlags->AuthFlags & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES;

    while ((*Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES)
            // Old Variable with no attributes can be erased after proper AuthHeader validation
            // EIP88439: irrespective of SetupMode, only properly formatted Auth Variable can be erased
            || (AuthAttributes) 
    ){
    // get mPlatformMode 
        GetPlatformMode ();

    // check if both attributes are set  
        if ((*Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES) == 
                           UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES
        )
        { Status = EFI_INVALID_PARAMETER; break;}

#if USER_MODE_POLICY_OVERRIDE == 1
        // ignore Variable Authentication rules while in Phys User Presense
        if (PhysicalUserPresent()) {
AVAR_TRACE((TRACE_ALWAYS,"Physical User Present : suppress Variable Authentication\n"));
            if(*DataSize==0 || *Data==NULL) {
                if(IsPkVar(VariableName, VendorGuid))
                    UpdatePlatformMode(SETUP_MODE);
    
                Status = EFI_SUCCESS; 
                break;
            }
            mPlatformMode = SETUP_MODE;
        }
#endif
//    Old - nonAuth, New - nonAuth - exit with EFI_SUCCESS
//    Old - nonAuth, New - Auth    - Continue with new Auth attr
//    Old - Auth,    New - nonAuth - if *Attribs=0 - Erase in progress if in SetupMode
//                                   else EFI_SECURITY_VIOLATION
//    Old - Auth,    New - Auth    - Continue if AuthAttr matching
//                                   else EFI_SECURITY_VIOLATION
// OldVar AuthAttributes mismatch
        if( AuthAttributes && *Attributes &&
          !(AuthAttributes & (*Attributes & UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES))
        )
        // Attribute mismatch
        { Status = EFI_SECURITY_VIOLATION; break;}

        // else in process of erasing or Setting AuthVar

        AuthAttributes |= *Attributes;
        OldMC = ExtSecFlags->Mc;

        if(*DataSize==0 || *Data==NULL)
        { Status = EFI_SECURITY_VIOLATION; break;}

        if(!mDigitalSigProtocol)
        { Status = EFI_UNSUPPORTED; break;}

//AVAR_TRACE((TRACE_ALWAYS,"AuthVarName: %S, Attr=%X, Data=%X, Size=%d\n", VariableName, *Attributes, *Data, *DataSize));
        if (AuthAttributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)
            Status = VerifyVariable2(VariableName, VendorGuid, AuthAttributes, Data, DataSize, OldData, OldDataSize, ExtSecFlags);
        else
            Status = VerifyVariable1(VariableName, VendorGuid, AuthAttributes, Data, DataSize, OldData, OldDataSize, ExtSecFlags);
//AVAR_TRACE((TRACE_ALWAYS, "Exit - %r, Data - %X, Size %d, Attributes %x\n",Status, *Data, *DataSize, *Attributes));
        if (EFI_ERROR(Status)) 
            break;

      // Find out if New Var is a Signature Db and the Sig already present in current Sig DB Variable
      // bail out SetVar if present - nothing to change
        Status = FindInSignatureDb(VendorGuid, *Attributes, *Data, DataSize, OldData, OldDataSize);
        if (EFI_ERROR(Status)) {
            // Only update the Timestamp if new Sig found in OldSig list
            if(OldMC != ExtSecFlags->Mc)
            {
               *DataSize = 0;
                Status = EFI_SUCCESS;
                
            }
            // else  Variable not changed, abort the SetVar
        } 

        break;
    } // end while loop

    return Status; // variable not changed
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
