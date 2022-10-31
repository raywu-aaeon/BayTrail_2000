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

#include <Token.h>
#include <AmiDxeLib.h>
#include <Protocol/SmiFlash.h>
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
//#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmSxDispatch.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <AmiSmm.h>
//PI 1.1 ++
#include <Protocol/SmmAccess2.h>

#if FWCAPSULE_RECOVERY_SUPPORT == 1
#include <Capsule.h>
#include <Guid/CapsuleVendor.h>
#endif

#include <AmiHobs.h>

#include <Protocol/SecSmiFlash.h>
#include <Protocol/FlashProtocol.h>
#include "AmiCertificate.h"

#define RETURN(status) {return status;}
//----------------------------------------------------------------------
// Module defined global variables
static EFI_GUID FlashUpdGuid           = FLASH_UPDATE_GUID;

extern EFI_GUID gEfiSmmCpuProtocolGuid;
extern EFI_GUID gSecureSMIFlashProtocolGuid;
extern EFI_GUID gAmiGlobalVariableGuid;
extern EFI_GUID gEfiSmmSxDispatch2ProtocolGuid;

static EFI_SMM_CPU_PROTOCOL            *gSmmCpu;


EFI_GUID gFWCapsuleGuid         = APTIO_FW_CAPSULE_GUID;
EFI_GUID gPRKeyGuid             = PR_KEY_GUID;
EFI_GUID gFwCapFfsGuid          = AMI_FW_CAPSULE_FFS_GUID;

static FLASH_UPD_POLICY FlUpdatePolicy = {FlashUpdatePolicy, BBUpdatePolicy};

EFI_SHA256_HASH  *gHashTbl = NULL;
UINT8     gHashDB[SHA256_DIGEST_SIZE];
static CRYPT_HANDLE  gpPubKeyHndl;

AMI_DIGITAL_SIGNATURE_PROTOCOL *gAmiSig;

#if FWCAPSULE_RECOVERY_SUPPORT == 1
extern EFI_GUID gEfiCapsuleVendorGuid;
EFI_CAPSULE_BLOCK_DESCRIPTOR  *gpEfiCapsuleHdr    = NULL;
#endif

// BIOS allocates the space in AcpiNVS for new BIOS image to be uploaded by Flash tool
// Alternatively the buffer may be reserved within the SMM TSEG. Check NEW_BIOS_MEM_ALLOC Token
// AFU would have to execute a sequence of SW SMI calls to load new BIOS image to mem
UINTN    gRomFileSize      = FWCAPSULE_IMAGE_SIZE;
UINT32   *pFwCapsuleLowMem = NULL;
APTIO_FW_CAPSULE_HEADER    *pFwCapsuleHdr;

static EFI_SMRAM_DESCRIPTOR *mSmramRanges;
static UINTN                mSmramRangeCount;
//----------------------------------------------------------------------
// Flash Upd Protocol defines
//----------------------------------------------------------------------
typedef EFI_STATUS (EFIAPI *FLASH_READ_WRITE)(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
typedef EFI_STATUS (EFIAPI *FLASH_ERASE)(
    VOID* FlashAddress, UINTN Size
);

FLASH_PROTOCOL      *Flash;

FLASH_READ_WRITE    pFlashWrite; // Original Ptr inside FlashAPI
FLASH_ERASE         pFlashErase;

static   UINT32    Flash4GBMapStart;
ROM_AREA *RomLayout = NULL;
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// UpFront Function definitions
EFI_STATUS CapsuleValidate (
    IN OUT UINT8     **pFwCapsule,
    IN OUT APTIO_FW_CAPSULE_HEADER     **pFWCapsuleHdr
);

EFI_STATUS LoadFwImage(
    IN OUT FUNC_BLOCK   *pFuncBlock
);
EFI_STATUS GetFlUpdPolicy(
    IN OUT FLASH_POLICY_INFO_BLOCK  *InfoBlock
);
EFI_STATUS SetFlUpdMethod(
    IN OUT FUNC_FLASH_SESSION_BLOCK    *pSessionBlock
);
EFI_STATUS FindCapHdrFFS(
    IN  VOID    *pCapsule,
    OUT UINT8 **pFfsData
);
BOOLEAN IsAddressInSmram (
    IN EFI_PHYSICAL_ADDRESS  Buffer,
    IN UINT64                Length
);

//----------------------------------------------------------------------
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SecSmiFlash
//
// Description: 
//
// Input:   
//
// Output: 
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_SEC_SMI_FLASH_PROTOCOL    SecSmiFlash = {
    LoadFwImage,
    GetFlUpdPolicy,
    SetFlUpdMethod,
    (void*)CapsuleValidate,
    0,
    0,
    0
};

#if FWCAPSULE_RECOVERY_SUPPORT == 1

#if CSLIB_WARM_RESET_SUPPORTED == 0

//#if (defined x64_BUILD && x64_BUILD == 1)
//VOID    flushcaches();
void    DisableCacheInCR0();
//#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// FUNCTION:  ReadRtcIndexedRegister
//
// DESCRIPTION: Used to read RTC register indexed by the argument
//
// Input:
//    IN UINT8    Index        Index of the register to read
//                                                         
//
// Output:
//    UINT8                    Current value of the register
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 ReadRtcIndexedRegister(IN UINT8 Index){

    UINT8 Byte = IoRead8(0x70) & 0x80;   // preserve bit 7
    IoWrite8(0x70, Index | Byte);
    Byte = IoRead8(0x71);              
    return Byte;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// FUNCTION:  ReadRtcIndexedRegister
//
// DESCRIPTION: Used to write to RTC register indexed by the argument
//
// Input:
//    IN UINT8    Index        Index of the register to write to 
//                                                         
//      IN UINT8  Value        Value to write to the RTC register
//
// Output:
//    VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID WriteRtcIndexedRegister(IN UINT8 Index, IN UINT8 Value){
 
    IoWrite8(0x70,Index | (IoRead8(0x70) & 0x80));
    IoWrite8(0x71,Value);   
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   S3RTCresume
//
// Description: This function puts system into ACPI S3 State.
//              if token ENABLE_RTC_ONE_SECOND_WAKEUP = 1, then it setups RTC
//              1 second alarm as well.
//
// Input:       None
//
// Output:      None, system will enter ACPI S3 State.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID S3RTCresume (VOID)
{
    UINT32 IoData;
    UINT8 Hour, Minute, Second;
    BOOLEAN     inBCD = TRUE;

    //flush caches befor going to S3
//#if (defined x64_BUILD && x64_BUILD == 1)
//        flushcaches();
    DisableCacheInCR0();
//#else
//        _asm wbinvd
//#endif
    
    // determine if RTC is in BCD mode
    if( ReadRtcIndexedRegister(0xB) & 0x4 ) // bit 2
        inBCD = FALSE;
    // wait for time update to complete before reading the values
    while( ReadRtcIndexedRegister(0xA) & 0x80 ); // while bit 7 is set the 
                                                // time update is in progress
    //read current hour, minute, second
    Hour = ReadRtcIndexedRegister(0x4);
    Minute = ReadRtcIndexedRegister(0x2);
    Second = ReadRtcIndexedRegister(0x0);

    //convert second to decimal from BCD and increment by 1
    if(inBCD)
        Second = (Second >> 4) * 10 + (Second & 0x0F);
    Second += 2;
    
    if(Second > 59){
        Second -= 60;
        if(inBCD)
            Minute = (Minute >> 4) * 10 + (Minute & 0x0F);
        Minute++;
        if(Minute > 59){
            Minute = 0;
            if(inBCD)
                Hour = (Hour >> 4) * 10 + (Hour & 0x0F);
            Hour++;
            // check 24 hour mode/12 hour mode
            if( ReadRtcIndexedRegister(0xB) & 0x2 ) {// bit 1 1=24hour else 12 hour
                if(Hour > 23)
                    Hour = 0;
            } else {
                if(Hour > 11)
                    Hour = 0;
            }

            if(inBCD)
                Hour = Hour % 10 + ( (Hour / 10) << 4 ) ;
        }
        if(inBCD)
            Minute = Minute % 10 + ( (Minute / 10) << 4 ) ;
    }

    //convert from decimal to BCD
    if(inBCD)
        Second = Second % 10 + ( (Second / 10) << 4 ) ;
    
    //set the alarm

    WriteRtcIndexedRegister(0x5, Hour);
    WriteRtcIndexedRegister(0x3, Minute);
    WriteRtcIndexedRegister(0x1, Second);
    //enable the alarm
    WriteRtcIndexedRegister(0xB, ( ReadRtcIndexedRegister(0xB) | ((UINT8)( 1 << 5 )) ));

// ========== PORTING REQUIRED ===========================================================
//  Current implementation to simulate the Warm Reboot may not be sufficient on some platforms. 
//  S3 transition may require additional Chipset/Platform coding.
//  If needed add any necessary OEM hooks to be able to put the system into S3 at the end of this handler
//========================================================================================

    //set RTC_EN bit in PM1_EN to wake up from the alarm
    IoWrite16(PM_BASE_ADDRESS + 0x02, ( IoRead16(PM_BASE_ADDRESS + 0x02) | (1 << 10) ));
    
    //Disable Sleep SMI to avoid SMI re-entrance.
    IoWrite16(PM_BASE_ADDRESS + 0x30, ( IoRead16(PM_BASE_ADDRESS + 0x30) & (~BIT4) ));
    
    //modify power management control register to reflect S3
    IoData = IoRead32(PM_BASE_ADDRESS + 0x04);
    //following code is applicable to Intel PCH only. 
    IoData &= ~(0x1C00);
    IoData |= 0x1400; //Suspend to RAM
/*
    // AMD w/a to enter S3 state
    IoData |= 0x2C00; //Suspend to RAM
    {
        UINT8 Temp8;
        IoWrite8(0xCD6, 0x004);
        Temp8 = IoRead8(0xCD7);
        Temp8 &= ~(BIT7);
        IoWrite8(0xCD6, 0x004);
        IoWrite8(0xCD7, Temp8);
        IoWrite8(0xCD6, 0x007);
        IoWrite8(0xCD7, BIT7);
    }
} 
*/
    IoWrite32(PM_BASE_ADDRESS + 0x04, IoData );
}
//#else
//extern SBLib_ResetSystem( IN EFI_RESET_TYPE ResetType );
#endif
extern SBLib_ResetSystem( IN EFI_RESET_TYPE ResetType );


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// FUNCTION:  SmiS5CapsuleCallback
//
// DESCRIPTION: SMI handler to perform capsule reset (bounce from S5 to S3)
// ========== PORTING REQUIRED ===========================================================
//  Current implementation to simulate the Warm Reboot may not be sufficient on some platforms. 
//  S3 transition may require additional Chipset/Platform coding.
//  If needed add any necessary OEM hooks to be able to put the system into S3 at the end of this handler
//========================================================================================
//
// Input:
//    IN  EFI_HANDLE    DispatchHandle                   Handle of SMI dispatch  protocol
//    IN  EFI_SMM_SX_DISPATCH_CONTEXT* DispatchContext   Pointer to SMI dispatch
//                                                       context structure
//
// Output:
//    VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SmiS5CapsuleCallback ( IN  EFI_HANDLE                    DispatchHandle,
                            IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext
){
    EFI_PHYSICAL_ADDRESS      IoData;
    UINTN       Size=sizeof(UINTN); //CSP20130805
    EFI_CAPSULE_HEADER   *CapsuleHeader;
    EFI_CAPSULE_BLOCK_DESCRIPTOR *pCapsuleMailboxPtr;

//TRACE((TRACE_ALWAYS,"SecSMI. S5 Trap\n"));

    Size=sizeof(EFI_PHYSICAL_ADDRESS);
    if(pRS->GetVariable(
           EFI_CAPSULE_VARIABLE_NAME, &gEfiCapsuleVendorGuid,
            NULL, &Size, &IoData)==EFI_SUCCESS)
    {     
        // verify the FW capsule is in memory. May first check if pCapsuleMailboxPtr == IoData
        pCapsuleMailboxPtr = (EFI_CAPSULE_BLOCK_DESCRIPTOR*)IoData;
        CapsuleHeader = (EFI_CAPSULE_HEADER*)pCapsuleMailboxPtr[0].Union.DataBlock;
        //
        // Compare GUID with APTIO_FW_CAPSULE_GUID 
        //
        if (guidcmp (&CapsuleHeader->CapsuleGuid, &gFWCapsuleGuid))
            return;

#if CSLIB_WARM_RESET_SUPPORTED == 1
        DisableCacheInCR0(); //EIP127538
        SBLib_ResetSystem(EfiResetWarm);
#else
        S3RTCresume();
#endif

    } 
}
/*
VOID SmiS5CapsuleCallback ( IN  EFI_HANDLE                    DispatchHandle,
                            IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext 
){
    EFI_STATUS          Status = EFI_DEVICE_ERROR;
    UINTN       Size=sizeof(AMI_FLASH_UPDATE_BLOCK);
    EFI_CAPSULE_HEADER   *CapsuleHeader;
    AMI_FLASH_UPDATE_BLOCK FlUpdateBlock;

    if(pRS->GetVariable(FLASH_UPDATE_VAR,&FlashUpdGuid, NULL, &Size, &FlUpdateBlock)==EFI_SUCCESS)
    {     
        // verify the FW capsule is in memory. May first check if pFlUpdateBlock == IoData
        if(FlUpdateBlock->FlashUpdate & FlCapsule)
            return;

        CapsuleHeader = (EFI_CAPSULE_HEADER*)gpEfiCapsuleHdr[0].DataBlock;
        //
        // Compare GUID with APTIO_FW_CAPSULE_GUID 
        //
        if (guidcmp (&CapsuleHeader->CapsuleGuid, &gFWCapsuleGuid))
            return;

        // SMM variant of EFI Var Service
        Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS;
        Status = pRS->SetVariable (
             CAPSULE_UPDATE_VAR,  
             &CapsuleVendorGuid,     
             Attributes,  
             sizeof (EFI_PHYSICAL_ADDRESS), 
             (VOID *) &gpEfiCapsuleHdr);

        if(EFI_ERROR(Status))
            return;
    );

#if CSLIB_WARM_RESET_SUPPORTED == 1
        SBLib_ResetSystem(EfiResetWarm);
#else
        S3RTCresume();
#endif

    } 
}
*/
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    SupportUpdateCapsuleReset
//
// Description:  This function returns if the platform supports update capsule across a system reset.
//
// Input:        None
//
// Output:      TRUE  - memory can be preserved across reset
//              FALSE - memory integrity across reset is not guaranteed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SupportUpdateCapsuleReset (
    VOID
)
{
  //
  //If the platform has a way to guarantee the memory integrity across a system reset, return 
  //TRUE, else FALSE. 
  //
    if( (FlUpdatePolicy.FlashUpdate & FlCapsule) || 
        (FlUpdatePolicy.BBUpdate & FlCapsule))
        return TRUE;

    return FALSE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    UpdateCapsule
//
// Description:    This code prepares Capsule Update EFI Variable
//
// Input:        
//  IN EFI_CAPSULE_HEADER **CapsuleHeaderArray - array of pointers to capsule headers passed in
//
// Output:      EFI_SUCCESS - capsule processed successfully
//              EFI_INVALID_PARAMETER - CapsuleCount is less than 1,CapsuleGuid is not supported
//              EFI_DEVICE_ERROR - capsule processing failed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS UpdateCapsule (
    IN FUNC_FLASH_SESSION_BLOCK *pSessionBlock
){
    EFI_STATUS          Status;
    EFI_CAPSULE_BLOCK_DESCRIPTOR *pCapsuleMailboxPtr;
    UINT32              Attributes ;
    //
    //Compare GUID with APTIO_FW_CAPSULE_GUID 
    //
    if (!guidcmp (&pFwCapsuleHdr->CapHdr.CapsuleGuid, &gFWCapsuleGuid)
    ){
        pCapsuleMailboxPtr = gpEfiCapsuleHdr;
        pCapsuleMailboxPtr[0].Length = pFwCapsuleHdr->CapHdr.HeaderSize;
        pCapsuleMailboxPtr[0].Union.DataBlock = (EFI_PHYSICAL_ADDRESS)pFwCapsuleHdr;
        pCapsuleMailboxPtr[1].Length = pFwCapsuleHdr->CapHdr.CapsuleImageSize-pFwCapsuleHdr->CapHdr.HeaderSize;
        if((UINT32*)pFwCapsuleLowMem == (UINT32*)pFwCapsuleHdr) {
        // Fw Cap Hdr is on top of Payload

            pCapsuleMailboxPtr[1].Union.DataBlock = pCapsuleMailboxPtr[0].Union.DataBlock+pCapsuleMailboxPtr[0].Length;
        } else {
        // Fw Cap Hdr is embedded inside Payload
            pCapsuleMailboxPtr[1].Union.DataBlock = (EFI_PHYSICAL_ADDRESS)pFwCapsuleLowMem;
        }
        pCapsuleMailboxPtr[2].Length = 0;
        pCapsuleMailboxPtr[2].Union.DataBlock = 0;
        //
        //Check if the platform supports update capsule across a system reset
        //
        if (!SupportUpdateCapsuleReset()) {
            return EFI_UNSUPPORTED;
        }
        // SMM variant of EFI Var Service
        Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS;
        Status = pRS->SetVariable (
             EFI_CAPSULE_VARIABLE_NAME, &gEfiCapsuleVendorGuid,
             Attributes,  
             sizeof(EFI_PHYSICAL_ADDRESS),
            (VOID*)&pCapsuleMailboxPtr); 

        if(!EFI_ERROR(Status))
            return Status;
    }
 
    return EFI_DEVICE_ERROR;
}

#endif //#if FWCAPSULE_RECOVERY_SUPPORT == 1

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    SetFlashUpdateVar
//
// Description:    This code finds if the capsule needs reset to update, if no, update immediately.
//
// Input:
//  IN EFI_CAPSULE_HEADER **CapsuleHeaderArray - array of pointers to capsule headers passed in
//  IN UINTN CapsuleCount - number of capsule
//  IN EFI_PHYSICAL_ADDRESS ScatterGatherList - physical address of datablock list points to capsule
//
// Output:        EFI_SUCCESS - capsule processed successfully
//              EFI_INVALID_PARAMETER - CapsuleCount is less than 1,CapsuleGuid is not supported
//              EFI_DEVICE_ERROR - capsule processing failed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetFlashUpdateVar (
    IN FUNC_FLASH_SESSION_BLOCK    *pSessionBlock
){
    EFI_STATUS          Status = EFI_DEVICE_ERROR;
    UINTN               Size=0;
    UINT32              CounterHi = 0;

    if(pSessionBlock->FlUpdBlock.FlashOpType == FlRecovery &&
        pSessionBlock->FlUpdBlock.FwImage.AmiRomFileName[0] == 0
    )
        return Status;//EFI_DEVICE_ERROR;

// Better yet - get EFI_TIME
    Size = sizeof(UINT32);
    if(EFI_ERROR(pRS->GetVariable(L"MonotonicCounter", &gAmiGlobalVariableGuid,
                  NULL, &Size, &CounterHi))
    )
//        return Status;//EFI_DEVICE_ERROR;
//SetMode should set FlashUpd even if no MC var detected.
        CounterHi=0xffffffff;

    pSessionBlock->FlUpdBlock.MonotonicCounter = CounterHi;
    CounterHi = (EFI_VARIABLE_NON_VOLATILE |
        EFI_VARIABLE_RUNTIME_ACCESS |
        EFI_VARIABLE_BOOTSERVICE_ACCESS);
    Status = pRS->SetVariable (
        FLASH_UPDATE_VAR,
        &FlashUpdGuid,
        CounterHi,
        sizeof(AMI_FLASH_UPDATE_BLOCK),
        (VOID*) &pSessionBlock->FlUpdBlock
    );

    if(!EFI_ERROR(Status))
        return EFI_SUCCESS;

    return EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   GetFlUpdPolicy
//
// Description: 
//
// Input:   
//
// Output: 
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetFlUpdPolicy(
    IN OUT FLASH_POLICY_INFO_BLOCK  *InfoBlock
)
{
    UINT32  KeySize = DEFAULT_RSA_KEY_MODULUS_LEN;

//TRACE((TRACE_ALWAYS,"SecSMI. GetPolicy. %X_%X\n",FlUpdatePolicy.FlashUpdate, FlUpdatePolicy.BBUpdate));

    if(IsAddressInSmram((EFI_PHYSICAL_ADDRESS)InfoBlock, sizeof(FLASH_POLICY_INFO_BLOCK)))
        return EFI_DEVICE_ERROR;

    MemCpy(&InfoBlock->FlUpdPolicy, &FlUpdatePolicy, sizeof(FLASH_UPD_POLICY));
    MemSet(&InfoBlock->PKpub, KeySize, 0xFF);
    if(gpPubKeyHndl.BlobSize < KeySize)
        KeySize = gpPubKeyHndl.BlobSize;
    MemCpy(&InfoBlock->PKpub, gpPubKeyHndl.Blob, KeySize);

    InfoBlock->ErrorCode = 0;

    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SetFlUpdMethod
//
// Description: 
//
// Input:   
//
// Output: 
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetFlUpdMethod(
    IN OUT FUNC_FLASH_SESSION_BLOCK    *pSessionBlock
)
{
    EFI_STATUS          Status = EFI_DEVICE_ERROR;
#if RUNTIME_SECURE_UPDATE_FLOW == 1
    UINT32              HashBlock;
    UINT32              BlockSize;
    UINT8              *BlockAddr;
#endif
    UINT32             *FSHandl;

//TRACE((TRACE_ALWAYS,"SecSMI. SetFlash\nSize     : %X\n",pSessionBlock->FlUpdBlock.ImageSize));

    if(IsAddressInSmram((EFI_PHYSICAL_ADDRESS)pSessionBlock, sizeof(FUNC_FLASH_SESSION_BLOCK)))
        return EFI_DEVICE_ERROR;

//if(pSessionBlock->FlUpdBlock.FlashOpType == FlRecovery)
//TRACE((TRACE_ALWAYS,"File Name: %s\n",pSessionBlock->FlUpdBlock.FwImage.AmiRomFileName));
//else
//TRACE((TRACE_ALWAYS,"Image Adr: %X\n",pSessionBlock->FlUpdBlock.FwImage.CapsuleMailboxPtr[0]));

//TRACE((TRACE_ALWAYS,"ROMmap   : %X\n",pSessionBlock->FlUpdBlock.ROMSection));
//TRACE((TRACE_ALWAYS,"FlOpType : %X\n",pSessionBlock->FlUpdBlock.FlashOpType));
// Verify if chosen Flash method is compatible with FlUpd Policy
    if(((pSessionBlock->FlUpdBlock.ROMSection & (1<<BOOT_BLOCK)) && (pSessionBlock->FlUpdBlock.FlashOpType & FlUpdatePolicy.BBUpdate)) || 
      (!(pSessionBlock->FlUpdBlock.ROMSection & (1<<BOOT_BLOCK))&& (pSessionBlock->FlUpdBlock.FlashOpType & FlUpdatePolicy.FlashUpdate))
    ){

//TRACE((TRACE_ALWAYS,"Buff Adr : %X\nBuff Size: %X\n",pFwCapsuleLowMem, gRomFileSize));

//!!! make sure Flash blocks BOOT_BLOCK, MAIN_, NV_ and EC_ are matching enum types in FlashUpd.h
        // Get Flash Update mode   
        switch(pSessionBlock->FlUpdBlock.FlashOpType)
        {
#if FWCAPSULE_RECOVERY_SUPPORT == 1
            case FlCapsule:
#endif
            case FlRuntime:
            //  common for FlRuntime or Capsule
                if(pSessionBlock->FlUpdBlock.ImageSize > gRomFileSize)
                    break; // suspecting buffer overrun. 

                SecSmiFlash.pFwCapsule = pFwCapsuleLowMem;
                // AFU updates the address in CapsuleMailboxPtr if 
                // it's capable of allocating large buffer to load entire FW Capsule image
                if(pSessionBlock->FlUpdBlock.FwImage.CapsuleMailboxPtr[0] != 0 )
                {
#if NEW_BIOS_MEM_ALLOC != 2
                    if(SecSmiFlash.pFwCapsule != NULL)
                        MemCpy((UINT8*)SecSmiFlash.pFwCapsule,
                            (UINT8*)pSessionBlock->FlUpdBlock.FwImage.CapsuleMailboxPtr[0],
                            pSessionBlock->FlUpdBlock.ImageSize);
                    else
#endif
                        SecSmiFlash.pFwCapsule = (UINT32*)pSessionBlock->FlUpdBlock.FwImage.CapsuleMailboxPtr[0];
                } 
                // else AFU must've uploaded the image to designated SMM space using LoadFw command

// verify we got a capsule at pFwCapsuleLowMem, update a ptr to FwCapHdr within Payload image
                Status = CapsuleValidate((UINT8**)&(SecSmiFlash.pFwCapsule), &pFwCapsuleHdr);
                if(EFI_ERROR(Status)) break;

                if(pSessionBlock->FlUpdBlock.FlashOpType == FlRuntime)
                {
#if RUNTIME_SECURE_UPDATE_FLOW == 1
                    // Fill in gShaHashTbl Hash Table
                    BlockSize = FLASH_BLOCK_SIZE;
                    BlockAddr = (UINT8*)SecSmiFlash.pFwCapsule;
                    for(HashBlock = 0; HashBlock < SEC_FLASH_HASH_TBL_BLOCK_COUNT; HashBlock++) 
                    {

                        Status = gAmiSig->Hash(gAmiSig, &gEfiHashAlgorithmSha256Guid, 
                            1, &BlockAddr, (const UINTN*)&BlockSize, gHashTbl[HashBlock]); 
                        if (EFI_ERROR(Status)) break;
                        BlockAddr+= (UINTN)(BlockSize);
                    }
#endif
                    // done for Runtime Upd
                    break;
                }
            // Set Capsule EFI Var if Capsule(Verify Capsule Mailbox points to FW_CAPSULE) 
                pSessionBlock->FlUpdBlock.ImageSize = pFwCapsuleHdr->CapHdr.CapsuleImageSize;
#if FWCAPSULE_RECOVERY_SUPPORT == 1
                Status = UpdateCapsule (pSessionBlock);
                if(EFI_ERROR(Status)) break;
#endif
            //  common for Recovery or Capsule
            case FlRecovery:
                //  Set FlUpd EFI Var (Get MC, verify RecFileName)
                Status = SetFlashUpdateVar (pSessionBlock);
                break;

            default:
                Status = EFI_DEVICE_ERROR;
        }
    }

    // Set Error Status
    if (Status != EFI_SUCCESS) { 
        SecSmiFlash.FSHandle = 0;
        SecSmiFlash.pFwCapsule = NULL;
        SecSmiFlash.RomLayout = RomLayout; // back to default RomLayout
        pSessionBlock->FSHandle  = 0;
        pSessionBlock->ErrorCode = 1;
        return EFI_DEVICE_ERROR;
    }
    // FSHandle is updated if Capsule validation passed.
    // Create FSHandle as 1st 4 bytes of gHashTbl. It must be different each time 
    //  SetMethod is called with new Image
    FSHandl = (UINT32*)gHashTbl;
    SecSmiFlash.FSHandle = *FSHandl; // should be unique per Capsule;
    SecSmiFlash.pFwCapsule = SecSmiFlash.pFwCapsule; // may be changed 
    // use RomLayout from new Secure Image if it's loaded in memory and validated
    SecSmiFlash.RomLayout = (ROM_AREA *)(UINTN)((UINT32)pFwCapsuleHdr+pFwCapsuleHdr->RomLayoutOffset);
    pSessionBlock->FSHandle  = SecSmiFlash.FSHandle;
    pSessionBlock->ErrorCode = 0;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   LoadFwImage
//
// Description: Routine is called in a loop by the Flash tool. 
//              Depending on the OS environment, Flash tool passes either an entire 
//              Flash Image into SMM buffer or block by block. 
//              E.g AFUDOS could allocate a contiguous buffer for the entire ROM buffer,
//              while certain OSes (Linux) may only allocate limited buffer sizes
//
// Input:       FUNC_BLOCK -> Address, size
//
// Output:      FUNC_BLOCK -> Status
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LoadFwImage(
    IN OUT FUNC_BLOCK   *pFuncBlock
)
{
    if(IsAddressInSmram((EFI_PHYSICAL_ADDRESS)pFuncBlock, sizeof(FUNC_BLOCK)))
        return EFI_DEVICE_ERROR;

//    if(IsAddressInSmram((EFI_PHYSICAL_ADDRESS)pFuncBlock->BufAddr, sizeof(EFI_PHYSICAL_ADDRESS)))
//        return EFI_DEVICE_ERROR;

    pFuncBlock->ErrorCode = 1;
    SecSmiFlash.FSHandle = 0; // clear out Hndl. Will be set to valid number in SetFlashMethod
    SecSmiFlash.pFwCapsule = NULL;
    SecSmiFlash.RomLayout = RomLayout; // back to default RomLayout

//TRACE((TRACE_ALWAYS,"SecSMI. LoadImage at %X\n",(UINT32)pFwCapsuleLowMem + pFuncBlock->BlockAddr));

    if(pFwCapsuleLowMem == NULL) 
        return EFI_DEVICE_ERROR;

// assuming the address in 0 based offset in new ROM image
    if(((UINT32)pFwCapsuleLowMem + pFuncBlock->BlockAddr + pFuncBlock->BlockSize) >
       ((UINT32)pFwCapsuleLowMem + gRomFileSize)
    )
        return EFI_DEVICE_ERROR;

    MemCpy((VOID*)((UINT32)pFwCapsuleLowMem+pFuncBlock->BlockAddr), 
            (UINT8*)pFuncBlock->BufAddr, pFuncBlock->BlockSize);

    pFuncBlock->ErrorCode = (UINT8)MemCmp(
        (VOID*)((UINT32)pFwCapsuleLowMem+pFuncBlock->BlockAddr), 
        (VOID*)pFuncBlock->BufAddr, pFuncBlock->BlockSize);

    pFuncBlock->ErrorCode = 0;

    return EFI_SUCCESS;
}
// End Secured Flash Update API

#if RUNTIME_SECURE_UPDATE_FLOW == 1
// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: BeforeSecureUpdate
//
// Description: Verifies if the Update range is protected by Signature
//              1. return Success if flash region is inside unSigned RomArea
//              2. if region is signed - verify range against internal the Hash
//                  and return pointer to internal DataBuffer
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash 
//
// Output:      Status
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS BeforeSecureUpdate (
    VOID* FlashAddress, UINTN Size, UINT8 **DataBuffer
)
{
    EFI_STATUS  Status = EFI_SUCCESS;
    ROM_AREA    *Area;
    UINT8       *BuffAddr;
    UINT8       *PageAddr;
    UINT8       HashCounter;
    UINTN       PageSize;
    UINTN       PageCount;
    UINT32      *FSHandl;

    FSHandl = (UINT32*)gHashTbl;

    Area = SecSmiFlash.RomLayout;

    if ( Area == NULL )
        return EFI_WRITE_PROTECTED;

    for (; Area && Area->Size!=0; Area++)
    {
        if(((EFI_PHYSICAL_ADDRESS)(UINTN)FlashAddress >= Area->Address) && 
           ((EFI_PHYSICAL_ADDRESS)(UINTN)FlashAddress+Size) <= (Area->Address+Area->Size))
        {
            if (Area->Attributes & ROM_AREA_FV_SIGNED)
            {
                Status = EFI_WRITE_PROTECTED;
                break;
            }
        }
    }
    if(Status == EFI_WRITE_PROTECTED &&
        (FlUpdatePolicy.FlashUpdate & FlRuntime)
    ){
        // check Verify status by comparing FSHandl with gHashTbl[0]
        // should be unique per Capsule;
        if(SecSmiFlash.FSHandle == 0 || 
           SecSmiFlash.FSHandle != *FSHandl)
            return Status; // EFI_WRITE_PROTECTED

        PageSize = FLASH_BLOCK_SIZE;
        PageCount=( (UINTN)FlashAddress - Flash4GBMapStart) / PageSize;

        if(SecSmiFlash.pFwCapsule != NULL)
        {
            // Flash Write -> Update ptr to internal Acpi NVS or SMM Buffer
            BuffAddr = (UINT8*)SecSmiFlash.pFwCapsule;
            PageAddr = (UINT8*)((UINTN)BuffAddr + (PageSize * PageCount));
            BuffAddr = (UINT8*)((UINTN)BuffAddr + ((UINTN)FlashAddress - Flash4GBMapStart));

            Status = EFI_SUCCESS;
            HashCounter = 2; // addr may rollover to next flash page
            while(HashCounter-- && PageCount < SEC_FLASH_HASH_TBL_BLOCK_COUNT)
            { 
                // compare calculated block hash with corresponding hash from the Hw Hash Table
                // if no match -> make Size=0 to skip Flash Write Op
                Status = gAmiSig->Hash(gAmiSig, &gEfiHashAlgorithmSha256Guid, 
                    1, (const UINT8**)&PageAddr, (const UINTN*)&PageSize, gHashDB); 
                if(EFI_ERROR(Status) || 
                    MemCmp(gHashDB, SecSmiFlash.HashTbl[PageCount], SHA256_DIGEST_SIZE)
                ){   
                    //TRACE((-1, "Hash Err! FlashBuff = %8X, Data = %8X, BlockAddr=%x, BlockSize=%x\n", BuffAddr, *((UINT32*)BuffAddr), PageAddr, Size));
                    return EFI_WRITE_PROTECTED;
                }
                // repeat Hash check on next Flash Block if Write Block overlaps the Flash Block boundary
                PageCount++;
                PageAddr = (UINT8*)((UINTN)PageAddr + PageSize);
                if((BuffAddr+Size) <= PageAddr)
                    break;
            }
            // Erase 
            if(DataBuffer != NULL)
                *DataBuffer = BuffAddr;
        }            
    }

    return Status;
}
// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SecureFlashWrite
//
// Description: Allows to write to flash device is Secure Capsule is loaded into memory
//              Function replacing Flash->Write API call
//
// Input:       VOID* FlashAddress, UINTN Size, VOID* DataBuffer
// 
//
// Output:      EFI_SUCCESS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS SecureFlashWrite (
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
)
{
    EFI_STATUS  Status;
    UINT8       *CurrBuff;

    CurrBuff = (UINT8*)DataBuffer;
    Status = BeforeSecureUpdate(FlashAddress, Size, &CurrBuff);
//TRACE((-1, "SecSMIFlash Write %X, BuffAddr=%X(%X) Lock Status=%r\n", FlashAddress, DataBuffer, CurrBuff, Status));
    if(!EFI_ERROR(Status))
        return pFlashWrite(FlashAddress, Size, CurrBuff);

    return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SecureFlashErase
//
// Description: Allows erase of flash device is Secure Capsule is loaded into memory
//              Function replacing Flash->Erase API call
//
// Input:       NON
// 
//
// Output:      EFI_SUCCESS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS SecureFlashErase (
    VOID* FlashAddress, UINTN Size
)
{
    EFI_STATUS  Status;

    Status = BeforeSecureUpdate(FlashAddress, Size, NULL);
//TRACE((-1, "SecSMIFlash Erase %X - %X Lock Status=%r\n", FlashAddress, Size, Status));
    if(!EFI_ERROR(Status)) 
        return pFlashErase(FlashAddress, Size);

    return Status;//EFI_SUCCESS;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  GetFwCapFfs
//
// Description:    Loads binary from RAW section of X firwmare volume
//
//  Input:
//               NameGuid  - The guid of binary file
//               Buffer    - Returns a pointer to allocated memory. Caller must free it when done.
//               Size      - Returns the size of the binary loaded into the buffer.
//
// Output:         Buffer - returns a pointer to allocated memory. Caller
//                        must free it when done.
//               Size  - returns the size of the binary loaded into the
//                        buffer.
//               EFI_NOT_FOUND  - Can't find the binary.
//               EFI_LOAD_ERROR - Load fail.
//               EFI_SUCCESS    - Load success.
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
GetFwCapFfs (
  IN      EFI_GUID       *NameGuid,
  IN OUT  VOID           **Buffer,
  IN OUT  UINTN          *Size
  )
{
  EFI_STATUS                    Status;
  UINTN                         HandleCount;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
  EFI_HANDLE                    *HandleBuff;
  UINT32                        AuthenticationStatus;

 *Buffer=0;
 *Size=0;
  Status = pBS->LocateHandleBuffer (ByProtocol,&gEfiFirmwareVolume2ProtocolGuid,NULL,&HandleCount,&HandleBuff);
  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
  }
  //
  // Find desired image in all Fvs
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = pBS->HandleProtocol (HandleBuff[Index],&gEfiFirmwareVolume2ProtocolGuid,&Fv);

    if (EFI_ERROR (Status)) {
       continue;//return EFI_LOAD_ERROR;
    }
    //
    // Try a raw file
    //
    Status = Fv->ReadSection (
                  Fv,
                  NameGuid, //&gFwCapFfsGuid,
                  EFI_SECTION_FREEFORM_SUBTYPE_GUID,//EFI_SECTION_RAW
                  0,    //Instance
                  Buffer,
                  Size,
                  &AuthenticationStatus
                  );

    if (Status == EFI_SUCCESS) break;
  }

  pBS->FreePool(HandleBuff);

  if (Index >= HandleCount) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: GetRomLayout
//
// Description: 
//
// Input:
//  IN EFI_HANDLE               ImageHandle     Image Handle
//  IN EFI_SYSTEM_TABLE         *SystemTable    Pointer to System Table
//
// Output:      EFI_STATUS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS GetRomLayout(
    IN EFI_SYSTEM_TABLE    *SystemTable,
    OUT ROM_AREA **RomLayout
)
{
    EFI_STATUS Status;
    static EFI_GUID HobListGuid = HOB_LIST_GUID;
    static EFI_GUID AmiRomLayoutHobGuid = AMI_ROM_LAYOUT_HOB_GUID;
    ROM_LAYOUT_HOB *RomLayoutHob;
    UINTN RomLayoutSize=0, Size;
    ROM_AREA *Area;
    APTIO_FW_CAPSULE_HEADER *FwCapHdr;
    UINT8*  pFwCapHdr=NULL;

// 1. Try to locate RomLayout from embedded CapHdr Ffs 
    //Locate from Ffs or install the Hob in SecureFlashDxe.c
    Status = GetFwCapFfs(&gFwCapFfsGuid, &pFwCapHdr, &Size);
    if(!EFI_ERROR(Status)) 
    {
        // Skip over Section GUID
        FwCapHdr = (APTIO_FW_CAPSULE_HEADER*)pFwCapHdr;
        (UINT8*)FwCapHdr += sizeof (EFI_GUID);
        Size -= sizeof (EFI_GUID);
        *RomLayout = (ROM_AREA *)(UINTN)((UINT32)FwCapHdr+FwCapHdr->RomLayoutOffset);
        TRACE((-1, "Get Rom Map from the FwCap FFS at %X(size 0x%X)\nRomLayout offs %X\n", FwCapHdr, Size, FwCapHdr->RomLayoutOffset));
        RomLayoutSize = sizeof(ROM_AREA);
        for (Area=*RomLayout; Area->Size!=0 && RomLayoutSize<=(Size - FwCapHdr->RomLayoutOffset); Area++)
        {
            RomLayoutSize+=sizeof(ROM_AREA);
        }
        Area=*RomLayout; 
    }
    else
    {
// 2. Backup: Use primary RomLayout from Rom Layout HOB. 
//  This one does not yet report the Rom Hole regions
    //----- Get HobList -------------------------------------
        RomLayoutHob = GetEfiConfigurationTable(SystemTable, &HobListGuid);
        if (RomLayoutHob!=NULL)
        {
    // -------- Get RomLayoutHob ----------------------
            if (!EFI_ERROR( 
                        FindNextHobByGuid(&AmiRomLayoutHobGuid, &RomLayoutHob)
                    ))
            {
                RomLayoutSize =   RomLayoutHob->Header.Header.HobLength
                                  - sizeof(ROM_LAYOUT_HOB);
          
                Area=(ROM_AREA*)((UINT8*)RomLayoutHob+1);
    TRACE((-1, "Get Default Rom Map from the Hob at %X\n", Area));
            }
        }
    }
    if(RomLayoutSize)
    {
        //---Allocate memory for RomLayout------------------------------
        VERIFY_EFI_ERROR(
                pSmst->SmmAllocatePool(EfiRuntimeServicesData, RomLayoutSize,(void **)RomLayout)
        );
        if (*RomLayout)
        {
            pBS->CopyMem(
                *RomLayout, Area, RomLayoutSize
            );
            if(pFwCapHdr)
              pBS->FreePool(pFwCapHdr);
            return EFI_SUCCESS;
        }
    }

    return EFI_NOT_FOUND;
}
#endif //#if RUNTIME_SECURE_UPDATE_FLOW == 1

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: IsAddressInSmram
//
// Description: CThis function check if the address is in SMRAM
//
// Input: 
//  Address - the buffer address to be checked.
//  Range   - the buffer length to be checked
//
// Output: 
//  TRUE  this address is in SMRAM.
//  FALSE this address is NOT in SMRAM.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsAddressInSmram (
  IN EFI_PHYSICAL_ADDRESS  Buffer,
  IN UINT64                Length
)
{
  UINTN  Index;
//TRACE((TRACE_ALWAYS,"Addr in SMRAM %X_%X\n",Buffer, Length));
  for (Index = 0; Index < mSmramRangeCount; Index ++) {
    if (((Buffer >= mSmramRanges[Index].CpuStart) && (Buffer < mSmramRanges[Index].CpuStart + mSmramRanges[Index].PhysicalSize)) ||
        ((mSmramRanges[Index].CpuStart >= Buffer) && (mSmramRanges[Index].CpuStart < Buffer + Length))) {
//TRACE((TRACE_ALWAYS,"TRUE\n"));
      return TRUE;
    }
  }
//TRACE((TRACE_ALWAYS,"FALSE\n"));
  return FALSE;
}

// !!! do not install if OFBD SecFlash is installed 
#if INSTALL_SECURE_FLASH_SW_SMI_HNDL == 1
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SecSMIFlashSMIHandler
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SecSMIFlashSMIHandler (
    IN  EFI_HANDLE                  DispatchHandle,
        IN CONST VOID               *Context OPTIONAL,
        IN OUT VOID                 *CommBuffer OPTIONAL,
        IN OUT UINTN                *CommBufferSize OPTIONAL
)
{
    EFI_STATUS  Status = EFI_SUCCESS;
    UINT8        Data;
    UINT64       pCommBuff;
    UINT32       HighBufferAddress = 0;
    UINT32       LowBufferAddress = 0;
    UINTN       Cpu;

    Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    //
    // Found Invalid CPU number, return
    //
    if(Cpu == (UINTN)-1) RETURN(Status);

    gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RBX, \
                                      Cpu, \
                                      &LowBufferAddress );
    gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \

                                      EFI_SMM_SAVE_STATE_REGISTER_RCX, \
                                      Cpu, \
                                      &HighBufferAddress );

    Data = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->CommandPort;

    pCommBuff            = HighBufferAddress;
    pCommBuff            = Shl64(pCommBuff, 32);
    pCommBuff            += LowBufferAddress;

//TRACE((-1, "Sec SW SMI Flash Hook == 0x%x (%r)\n", Data, Status));

    switch(Data)
    {
        case SecSMIflash_Load:             // 0x1d Send Flash Block to memory
            Status = LoadFwImage((FUNC_BLOCK *)pCommBuff);
            break;

        case SecSMIflash_GetPolicy:        // 0x1e Get Fl Upd Policy 
            Status = GetFlUpdPolicy((FLASH_POLICY_INFO_BLOCK *)pCommBuff);
            break;
        
        case SecSMIflash_SetFlash:        // 0x1f Set Flash method
            Status = SetFlUpdMethod((FUNC_FLASH_SESSION_BLOCK *)pCommBuff);
            break;
    }
//    TRACE((TRACE_ALWAYS,"Exit  with %r\n", Status));
    RETURN(Status);
}
#endif
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   InSmmFunction
//
// Description: 
//
// Input:   
//
// Output: 
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InSmmFunction(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
#if INSTALL_SECURE_FLASH_SW_SMI_HNDL == 1
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext;
    UINTN                           Index;
#endif
#if FWCAPSULE_RECOVERY_SUPPORT == 1
const EFI_SMM_SX_REGISTER_CONTEXT      SxRegisterContext = {SxS5, SxEntry};
      EFI_SMM_SX_DISPATCH2_PROTOCOL    *SxDispatchProtocol;
#endif

    EFI_HANDLE              Handle = NULL;
    EFI_HANDLE              DummyHandle = NULL;
    EFI_STATUS              Status;

    UINTN   DescSize=0;
    UINT8   MinSMIPort = SecSMIflash_Load;    //0x1d
    //UINT8   MinSMIPort = SecSMIflash_GetPolicy; //0x1e;
    UINT8   MaxSMIPort = SecSMIflash_SetFlash; //0x1f;

    UINT32                  Flags=0;
    EFI_SMM_ACCESS2_PROTOCOL     *SmmAccess;
    UINTN                         Size;

     InitAmiSmmLib( ImageHandle, SystemTable );
    //
    // Get SMRAM information
    //
    Status = pBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR(Status)) return Status;
    Size = 0;
    Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
    ASSERT (Status == EFI_BUFFER_TOO_SMALL);
    if (Size==0) return EFI_NOT_FOUND;
    Status = pSmst->SmmAllocatePool (EfiRuntimeServicesData,Size,(VOID **)&mSmramRanges);
    if (EFI_ERROR(Status)) return Status;
    Status = SmmAccess->GetCapabilities (SmmAccess, &Size, mSmramRanges);
    if (EFI_ERROR(Status)) return Status;
    mSmramRangeCount = Size / sizeof (EFI_SMRAM_DESCRIPTOR);

    Status = pSmst->SmmLocateProtocol(&gAmiSmmDigitalSignatureProtocolGuid, NULL, &gAmiSig);
//    Status = pBS->LocateProtocol(&gAmiSmmDigitalSignatureProtocolGuid, NULL, &gAmiSig);
    if (EFI_ERROR(Status)) return Status;

// Get PRKey and move it in SMM protected location
    gpPubKeyHndl.Blob = NULL;
    gpPubKeyHndl.BlobSize = 0;
    Status = gAmiSig->GetKey(gAmiSig, &gpPubKeyHndl, &gPRKeyGuid, gpPubKeyHndl.BlobSize, Flags);
TRACE((TRACE_ALWAYS,"GetKey %r (%x, %x bytes)\n",Status, gpPubKeyHndl.Blob,gpPubKeyHndl.BlobSize));
    if (EFI_ERROR(Status)) {
        if(Status == EFI_BUFFER_TOO_SMALL) 
            return EFI_SUCCESS;
        return Status;
    }
    //
    // Allocate scratch buffer to hold entire Signed BIOS image for Secure Capsule and Runtime Flash Updates
    // AFU would have to execute a sequence of SW SMI calls to push entire BIOS image to SMM
    //
    //
    //NEW_BIOS_MEM_ALLOC == 2 AFU will allocate a buffer and provide pointer via SET_FLASH_METHOD API call. 
    //
#if NEW_BIOS_MEM_ALLOC == 0
    //
    // Alternatively the buffer may be reserved within the SMM TSEG memory 
    //
    Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, gRomFileSize, (void**)&pFwCapsuleLowMem);

//TRACE((TRACE_ALWAYS,"SecSmiFlash: Alloc 0x%X bytes in SMM, %r\n",gRomFileSize, Status));
#else
#if NEW_BIOS_MEM_ALLOC == 1
    //
    // The buffer is reserved within the ACPI NVS memory
    //
    Status = pST->BootServices->AllocatePool(EfiACPIMemoryNVS, gRomFileSize, &pFwCapsuleLowMem);
//TRACE((TRACE_ALWAYS,"SecSmiFlash: AllocatePool=%X,(0x%x) %r\n",pFwCapsuleLowMem,gRomFileSize,  Status));
#endif
#endif
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return Status;
#if NEW_BIOS_MEM_ALLOC < 2
    MemSet((void*)pFwCapsuleLowMem, gRomFileSize, 0 );
#endif
    //
    // Allocate space to hold a Hash table for all Flash blocks
    //
    DescSize = SEC_FLASH_HASH_TBL_SIZE;
    Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, DescSize, (void**)&gHashTbl);
//    TRACE((TRACE_ALWAYS,"SecSmiFlash: AllocateHashTbl Pool=%X,(0x%x) %r\n",HashTbl,HashTbl,  Status));
    if (EFI_ERROR(Status)) return Status;
    MemSet((void*)gHashTbl, DescSize, 0xdb );


#if FWCAPSULE_RECOVERY_SUPPORT == 0
    FlUpdatePolicy.FlashUpdate &=~FlCapsule;
    FlUpdatePolicy.BBUpdate &=~FlCapsule;
#else    
    //
    // Reserve pool in non-smm runtime memory for capsule's mailbox list
    //
    // EfiACPIMemoryNVS
    DescSize = 4*sizeof(EFI_CAPSULE_BLOCK_DESCRIPTOR) + sizeof(EFI_CAPSULE_HEADER); // (4*16)+28
    Status = pBS->AllocatePool(EfiACPIMemoryNVS, DescSize, &gpEfiCapsuleHdr);
    if (EFI_ERROR(Status)) return Status;
    //    pEfiCapsuleHdr&=0xFFFFFFFFFFFFFFF8;  // !!!make sure descriptor is 8-byte aligned
    MemSet((void*)gpEfiCapsuleHdr, DescSize, 0 );

    //
    // Install callback on S5 Sleep Type SMI. Needed to transition to S3 if Capsule's mailbox ie pending
    // Locate the Sx Dispatch Protocol
    //
    // gEfiSmmSxDispatch2ProtocolGuid
    Status = pSmst->SmmLocateProtocol( &gEfiSmmSxDispatch2ProtocolGuid, NULL, &SxDispatchProtocol);
    ASSERT_EFI_ERROR (Status);  
    if (EFI_ERROR(Status)) return Status;
    //
    // Register the callback for S5 entry
    //
    if (SxDispatchProtocol && SupportUpdateCapsuleReset()) {
      Status = SxDispatchProtocol->Register (
                    SxDispatchProtocol,
                    (EFI_SMM_HANDLER_ENTRY_POINT2)SmiS5CapsuleCallback,
                    &SxRegisterContext,
                    &Handle
                    );
      ASSERT_EFI_ERROR (Status);  
    }
    if (EFI_ERROR(Status)) goto Done;
#endif
#if RUNTIME_SECURE_UPDATE_FLOW == 1
/*
AFU For Rom Holes in Runtime/Capsule upd
    1. Read full ROM image to ROM buffer
    2. Merge Rom Hole from input file to ROM buffer
    3. call "LoadImage" for full BIOS
    3. call "SetFlash" with Runtime update (NVRAM block should be unsigned!!!)
    4. calls to upd  Rom hole -erase,write should pass
*/
    Status = GetRomLayout(SystemTable, &RomLayout);
TRACE((TRACE_ALWAYS,"SecSmiFlash: Get Rom Layout ptr=%X, %r\n",RomLayout, Status));
    // Rom Layout HOB may not be found in Recovery mode and if FW does not include built in FwCapsule Hdr file
    // In this case another attempt will be made to locate RomLayout during SetFlashMode Command 
//    if (EFI_ERROR(Status)) goto Done;
    //
    // Trap the original Flash Driver API calls to enforce 
    // Flash Write protection in SMM at the driver API level
    //
    Status = pBS->LocateProtocol(&gFlashSmmProtocolGuid, NULL, &Flash);
TRACE((TRACE_ALWAYS,"SecSmiFlash: Flash Protocol Fixup %X->%X\n",Flash->Write,SecureFlashWrite));
    if (EFI_ERROR(Status)) goto Done;

    // preserve org Flash API
    pFlashWrite = Flash->Write; 
    pFlashErase = Flash->Erase;
    // replace with local functions 
    Flash->Erase = SecureFlashErase;
    Flash->Write = SecureFlashWrite;
    // Calculate the flash mapping start address. This is calculated
    // as follows:
    //  1. Find the total size of the flash (FLASH_BLOCK_SIZE * NUMBER_OF_BLOCKS)
    //  2. Subtract the total flash size from 4GB
    Flash4GBMapStart = 0xFFFFFFFF - (FLASH_BLOCK_SIZE * NUMBER_OF_BLOCKS);
    Flash4GBMapStart ++;    
#endif
    //
    // Install Secure SMI Flash Protocol 
    //
    SecSmiFlash.pFwCapsule = pFwCapsuleLowMem;
    SecSmiFlash.HashTbl = gHashTbl;
    SecSmiFlash.RomLayout = RomLayout;
    SecSmiFlash.FSHandle = 0;
    Status = pSmst->SmmInstallProtocolInterface(
                 &DummyHandle,
                 &gSecureSMIFlashProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &SecSmiFlash
             );
    ASSERT_EFI_ERROR(Status);
//    if (EFI_ERROR(Status)) return Status;
    if (EFI_ERROR(Status)) goto Done;

    //    
    // Install SW SMI callbacks for 3 SecSMI Flash functions
    // !!! do not install if OFBD SecFlash is installed 
    //
#if INSTALL_SECURE_FLASH_SW_SMI_HNDL == 1

    Status = pSmst->SmmLocateProtocol( \
                        &gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;


    Status = pSmst->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, &gSmmCpu);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    for(Index=MinSMIPort;Index<=MaxSMIPort;Index++)
    {
        SwContext.SwSmiInputValue    = Index;
        Status    = pSwDispatch->Register(pSwDispatch, SecSMIFlashSMIHandler, &SwContext, &Handle);
        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) break;
        //TODO: If any errors, unregister any registered SwSMI by this driver.
        //If error, and driver is unloaded, then a serious problem would exist.
    }
#endif
Done:
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SecSMIFlashDriverEntryPoint
//
// Description: Secure SMI Flash driver init 
//
// Input:   
//
// Output: 
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SecSMIFlashDriverEntryPoint(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    InitAmiLib(ImageHandle, SystemTable);
    return InitSmmHandlerEx(ImageHandle, SystemTable, InSmmFunction, NULL);
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
