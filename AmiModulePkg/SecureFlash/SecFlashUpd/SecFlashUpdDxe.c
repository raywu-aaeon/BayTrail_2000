//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiHobs.h>
#include <FlashUpd.h>
#include <Protocol/AmiDigitalSignature.h>
#include "AmiCertificate.h"
#include <Library/PciLib.h>

#include <Guid/CapsuleVendor.h>
//
// Global variables
//
extern EFI_BOOT_SERVICES    *pBS;

static EFI_GUID gFlashUpdGuid = FLASH_UPDATE_GUID;
static EFI_GUID gHobListGuid  = HOB_LIST_GUID;

#if FLASH_LOCK_EVENT_NOTIFY == 1
// Trigger to set Ready to lock event
extern EFI_GUID  gBdsConnectDriversProtocolGuid;

// Trigger to check Flash lock status.
#ifndef BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID
#define BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID \
    {0xdbc9fd21, 0xfad8, 0x45b0, 0x9e, 0x78, 0x27, 0x15, 0x88, 0x67, 0xcc, 0x93}
#endif
EFI_GUID gBdsAllDriversConnectedProtocolGuid = BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID;

// Ready to lock event
#ifndef AMI_EVENT_FLASH_WRITE_LOCK
// {49D34AE7-1348-4551-8F71-467D8C0E4EF5}
#define AMI_EVENT_FLASH_WRITE_LOCK \
    { 0x49D34AE7, 0x9454, 0x4551, 0x8F, 0x71, 0x46, 0x7D, 0x8C, 0x0E, 0x4E, 0xF5 }
#endif
static EFI_GUID gBiosReadyToLockEventGuid = AMI_EVENT_FLASH_WRITE_LOCK;

// TBD
// Flash Lock Ready to lock event
//#ifndef AMI_EVENT_FLASH_WRITE_LOCK_SET
//// {707DF0E9-84BE-4A36-8996-D4311D788429}
//#define AMI_EVENT_FLASH_WRITE_LOCK_SET \
//    { 0x707DF0E9, 0x84BE, 0x4A36, 0x89, 0x96, 0xD4, 0x31, 0x1D, 0x78, 0x84, 0x29 }
//#endif
//static EFI_GUID gBiosLockSetEventGuid = AMI_EVENT_FLASH_WRITE_LOCK_SET;

#endif

typedef struct{
    UINT8 Key;
    UINT8 Mode;
    UINT8 Lock;
} SECURE_FLASH_SETUP_VAR;

#define AMI_SECURE_FLASH_SETUP_VAR  L"SecureFlashSetupVar"
// {35C936AF-E1E1-441a-BAD1-E1544E9D97A6}
#define AMI_SECURE_FLASH_SETUP_VAR_GUID \
    {0x35c936af, 0xe1e1, 0x441a, 0xba, 0xd1, 0xe1, 0x54, 0x4e, 0x9d, 0x97, 0xa6}

SECURE_FLASH_SETUP_VAR SecureFlashSetupVar = {0,0,0};
EFI_GUID gSecureFlashSetupVarGuid = AMI_SECURE_FLASH_SETUP_VAR_GUID;    

//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------
#if FLASH_LOCK_EVENT_NOTIFY == 1

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: ReadyToLockCallback
//
// Description: Callback on ready to Lock Flash part
//              Action - Signal Ready to Lock the Flash event. 
//
// Input:   Event             - The event that triggered this notification function  
//          ParentImageHandle - Pointer to the notification functions context
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ReadyToLockCallback (
  EFI_EVENT           Event,
  VOID                *ParentImageHandle
  )
{
    EFI_HANDLE  Handle = NULL;
    TRACE((-1,"\nSecure Fl Upd: Flash_Ready_To_Lock callback\n"));

// Signal Event.....
    pBS->InstallProtocolInterface (
        &Handle, &gBiosReadyToLockEventGuid, EFI_NATIVE_INTERFACE,NULL
    );
    pBS->UninstallProtocolInterface (
        Handle,&gBiosReadyToLockEventGuid, NULL
    );
    //
    //Kill the Event
    //
    pBS->CloseEvent(Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: CheckBiosLockStatus
//
// Description: Check for a BIOS flash region Lock
//
// Input:   Event             - The event that triggered this notification function  
//          ParentImageHandle - Pointer to the notification functions context
//
// Output:  UIN8 - BIOS Flash lock status
// 
// 0 - Flash Lock information N/A
// 1 - Flash Lock Disabled
// 2 - Flash Lock Enabled    
//                    
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#ifndef ICH_REG_LPC_BIOS_CNTL
// LPC Bridge Bus Device Function Definitions
#define ICH_REG_LPC_VID 0
#define ICH_REG_LPC_BIOS_CNTL 0xDC
#define LPC_BUS     0
#define LPC_DEVICE  31
#define LPC_FUNC    0    
#endif    
#define PCI_LIB_ADDRESS(Bus,Device,Function,Register)   \
  (((Register) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))

UINT8 CheckBiosLockStatus()
{
    UINT8  Byte;
    UINT16 Word;
// Sample implementation works only for Intel PCH's with SPI controller programming of the BIOS flash region lock
    Word = PciRead16(PCI_LIB_ADDRESS(LPC_BUS, LPC_DEVICE, LPC_FUNC, ICH_REG_LPC_VID));
    Byte = PciRead8 (PCI_LIB_ADDRESS(LPC_BUS, LPC_DEVICE, LPC_FUNC, ICH_REG_LPC_BIOS_CNTL));
    TRACE((-1,"\nVendor %x, reg 0xDC[0,1,5]  = 0x%X\n", Word, Byte));
    if(Word != 0x8086 || Byte == 0xFF)
        return 0; // lock info not available
    else
        if((Byte & (BIT00 | BIT01 | BIT05)) ==(BIT01 | BIT05))
            return 2; // enabled
        else
            return 1; // disabled
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: AfterLockCallback
//
// Description: Trap on Flash Lock Complete event. 
//              Update SecureFlash setup variable with state of BIOS region Lock
//
// Input:   Event             - The event that triggered this notification function  
//          ParentImageHandle - Pointer to the notification functions context
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
AfterLockCallback (
  EFI_EVENT           Event,
  VOID                *ParentImageHandle
  )
{
    EFI_STATUS Status;
    UINTN Size;
    UINT32 Attributes;
    
    TRACE((-1,"\nSecure Fl Upd: Flash Lock Complete callback\n"));
    
    Status = pRS->GetVariable (AMI_SECURE_FLASH_SETUP_VAR, 
            &gSecureFlashSetupVarGuid,  
            &Attributes,
            &Size,
            &SecureFlashSetupVar);
    if(!EFI_ERROR(Status) && Size == sizeof(SECURE_FLASH_SETUP_VAR)) 
    {
//    
// test only. will remove the call when actual BiosLockSet event is supported
//
        SecureFlashSetupVar.Lock = CheckBiosLockStatus();
// end
        pRS->SetVariable (AMI_SECURE_FLASH_SETUP_VAR, 
                &gSecureFlashSetupVarGuid,                  Attributes,
                Size,
                &SecureFlashSetupVar);
    }
    //
    //Kill the Event
    //
    pBS->CloseEvent(Event);
}
#endif
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SecFlashUpdDxe_Init
//
// Description: Entry point of Flash Update Policy driver
//
// Input:       EFI_HANDLE           ImageHandle,
//              EFI_SYSTEM_TABLE     *SystemTable
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SecFlashUpdDxe_Init (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
#if FLASH_LOCK_EVENT_NOTIFY == 1
    VOID     *gSbHobList ;
    EFI_EVENT mSecureModEvent;
    VOID      *mSecureModReg;
#endif
    EFI_STATUS  Status = EFI_SUCCESS;
    UINT32                  Attributes;
    UINTN                   Size;
    
    FLASH_UPD_POLICY FlUpdatePolicy = {FlashUpdatePolicy, BBUpdatePolicy};
    CRYPT_HANDLE  pPubKeyHndl;
    AMI_DIGITAL_SIGNATURE_PROTOCOL *AmiSig;
    EFI_GUID PRKeyGuid       = PR_KEY_GUID;
    UINT8                    Byte;

    InitAmiLib(ImageHandle, SystemTable);

    Size = 0;
    if(pRS->GetVariable( FLASH_UPDATE_VAR,&gFlashUpdGuid,&Attributes,&Size, NULL)==EFI_BUFFER_TOO_SMALL) {
        // Erase Flash Var
        pRS->SetVariable (FLASH_UPDATE_VAR,&gFlashUpdGuid,Attributes,0,NULL);
        // Clear pending Capsule Update Var
        // only if FlashOp is pending. We don't want to interfere with other types of Capsule Upd
        Size = 0;
        if(pRS->GetVariable(EFI_CAPSULE_VARIABLE_NAME, &gEfiCapsuleVendorGuid, &Attributes, &Size, NULL) ==EFI_BUFFER_TOO_SMALL)
            pRS->SetVariable(EFI_CAPSULE_VARIABLE_NAME, &gEfiCapsuleVendorGuid, Attributes,0,NULL);
    }
///////////////////////////////////////////////////////////////////////////////
//
// Create Flash Lock event.
//
///////////////////////////////////////////////////////////////////////////////
#if FLASH_LOCK_EVENT_NOTIFY == 1
    // Get Hob List
    gSbHobList = GetEfiConfigurationTable(SystemTable, &gHobListGuid);
    if (!gSbHobList)
        return EFI_INVALID_PARAMETER;

// Locking SPI before BDS Connect on normal boot
    if (((EFI_HOB_HANDOFF_INFO_TABLE*)gSbHobList)->BootMode!=BOOT_IN_RECOVERY_MODE && 
        ((EFI_HOB_HANDOFF_INFO_TABLE*)gSbHobList)->BootMode!=BOOT_ON_FLASH_UPDATE
    ) 
        Status = RegisterProtocolCallback ( &gBdsConnectDriversProtocolGuid, \
                                        ReadyToLockCallback, \
                                        NULL, \
                                        &mSecureModEvent, \
                                        &mSecureModReg );
// Locking SPI after ReFlash(BDS) if in Recovery/Flash Upd mode
    else
        Status = CreateReadyToBootEvent ( TPL_CALLBACK,
                                        ReadyToLockCallback,
                                        NULL,
                                        &mSecureModEvent);
    ASSERT_EFI_ERROR (Status);
    // Installing a callback on BIOS Lock set event.
    //                                  &gBiosLockSetEventGuid,
    Status = RegisterProtocolCallback ( &gBdsAllDriversConnectedProtocolGuid, \
                                        AfterLockCallback, \
                                        NULL, \
                                        &mSecureModEvent, \
                                        &mSecureModReg );
#endif
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Updated Secure Flash Setup Var
    //
    ///////////////////////////////////////////////////////////////////////////////
    // Key
    // 0 - Disabled, No Key
    // 1 - Enabled, RSA2048
    // 2 - Enabled, SHA256
    // 3 - Enabled, x509
    // 4 - Disabled, DummyKey
    // Lock 
    // 0 - Flash Lock information N/A
    // 1 - Flash Lock Disabled
    // 2 - Flash Lock Enabled    
    // Mode
    // 0 - Flash Updates disabled
    // 1 - Flash Update: Runtime, Recovery, Capsule
    // 2 - Flash Update: Runtime, Recovery
    // 3 - Flash Update: Runtime
    //
    // Look up for FwKey 
    //
    Status = pBS->LocateProtocol(&gAmiDigitalSignatureProtocolGuid, NULL, &AmiSig);
    if (EFI_ERROR(Status)) return Status;

// Get PRKey and move it in SMM protected location
    pPubKeyHndl.Blob = NULL;
    pPubKeyHndl.BlobSize = 0;
    Status = AmiSig->GetKey(AmiSig, &pPubKeyHndl, &PRKeyGuid, pPubKeyHndl.BlobSize, 0);
TRACE((TRACE_ALWAYS,"GetKey %r (%x, %d bytes)\n",Status, pPubKeyHndl.Blob,pPubKeyHndl.BlobSize));
    if (!EFI_ERROR(Status) || Status == EFI_BUFFER_TOO_SMALL) {
        if(!guidcmp(&pPubKeyHndl.AlgGuid, &gEfiCertRsa2048Guid)) 
            SecureFlashSetupVar.Key = 1;
        if(!guidcmp(&pPubKeyHndl.AlgGuid, &gEfiCertSha256Guid)) 
            SecureFlashSetupVar.Key = 2;
        if(!guidcmp(&pPubKeyHndl.AlgGuid, &gEfiCertX509Guid))
            SecureFlashSetupVar.Key = 3;
        
        // check If dummy key - return Mode = 0
        Byte = pPubKeyHndl.Blob[0];
        for(Size = 1; Size < pPubKeyHndl.BlobSize && (Byte == pPubKeyHndl.Blob[Size]); Size++);
        if(Size == pPubKeyHndl.BlobSize) {
            SecureFlashSetupVar.Key = 4; // dummy key
        }
    } else
    //
    // Flash Upd Mode
    //
    if(SecureFlashSetupVar.Key == 4)    // dummy key
        FlUpdatePolicy.FlashUpdate = 0; // Sec Flash disabled

#if FWCAPSULE_RECOVERY_SUPPORT == 0
    FlUpdatePolicy.FlashUpdate &=~FlCapsule;
#endif    
    if((FlUpdatePolicy.FlashUpdate & (FlCapsule | FlRecovery | FlRuntime)) ==
            (FlCapsule | FlRecovery | FlRuntime))
            SecureFlashSetupVar.Mode = 1;
    if((FlUpdatePolicy.FlashUpdate & (FlCapsule | FlRecovery | FlRuntime)) ==
            (FlRecovery | FlRuntime))
            SecureFlashSetupVar.Mode = 2;
    if((FlUpdatePolicy.FlashUpdate & (FlCapsule | FlRecovery | FlRuntime)) ==
            (FlRuntime))
            SecureFlashSetupVar.Mode = 3;
    //
    // Flash Lock
    //
    SecureFlashSetupVar.Lock = 0; // N/A, TBD
    //
    // Set Flash Upd Setup Var
    //
    Size = sizeof(SECURE_FLASH_SETUP_VAR);
    pRS->SetVariable (AMI_SECURE_FLASH_SETUP_VAR, 
            &gSecureFlashSetupVarGuid,  
            EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
            Size,
            &SecureFlashSetupVar);

    return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
