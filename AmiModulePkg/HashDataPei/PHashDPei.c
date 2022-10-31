//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
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
// Revision History
// ----------------
// $Log: $
// 
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           PHashDPei.c
//
//  Description:    
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>
#include <Token.h>
#include <AmiPeiLib.h>
#include "PPI\LoadFile.h"
#include <Protocol\Hash.h>
#include <PPI\CryptoPPI.h>
#include "AmiCertificate.h"
#include <cryptlib.h>
//(EIP119182)>>
#include <Ppi/PeiHashPPi.h>
//(CSP20130813A+)>>
#include <Pi\PiHob.h>
#include <PeiRamBoot.h>
//(CSP20130813A+)<<
#include <RomLayout.h>  //<EIP134987+> 2013/09/09

#define AMI_PLATFORM_FV_ADDRESS_HOB_GUID\
    { 0xf1966d02, 0x6eb3, 0x4378, 0x96, 0x35, 0x77, 0xd, 0x3b, 0xa8, 0x9a, 0xcc }

EFI_GUID                        AmiPeiHashSha256Guid = PEI_HASH_SHA256_PPI_GUID;

#define HASH_SHA256_LEN                   sizeof(EFI_SHA256_HASH)     // 32


typedef struct {
    EFI_HOB_GUID_TYPE EfiHobGuidType;
    EFI_PHYSICAL_ADDRESS                   Address;
    UINT8                                       FVNumber;
} EFI_HOB_FV_ADDRESS;

//(EIP124704)>>
EFI_STATUS PeiHashSha256(
    IN EFI_PEI_SERVICES **PeiServices,
    IN UINT32                      Address,
    IN UINT32                      Siz,
    IN UINT32                      HashAddress
);
//(EIP124704)<<

static  EFI_HASH_SHA256_PPI mPeiHashPpi = { PeiHashSha256 };

static EFI_PEI_PPI_DESCRIPTOR mPpiList[] = {
    {
        (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
        &AmiPeiHashSha256Guid, &mPeiHashPpi
    }

};
//(EIP119182)<<
//(CSP20130813A+)>>
#define ROM_CACHE_ENABLE_PPI_GUID \
{ 0x36E835BB, 0x661D, 0x4D37, 0x8D, 0xE5, 0x88, 0x53, 0x25, 0xDA, 0xE9, 0x10 }
EFI_GUID gRomCacheEnablePpiGuid = ROM_CACHE_ENABLE_PPI_GUID;

//<EIP134987+> 2013/09/09 >>>
EFI_STATUS GetRomLayout(
    IN  EFI_PEI_SERVICES **PeiServices,
    OUT ROM_AREA         **Layout
);
//<EIP134987+> 2013/09/09 <<<

EFI_STATUS
VerifyFvBeforePublish (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
);

static EFI_PEI_NOTIFY_DESCRIPTOR VerifyFvBeforePublishNotify[] =
{
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiMemoryDiscoveredPpiGuid,  //<EIP134987*> 2013/09/04
        VerifyFvBeforePublish
    }
};

static EFI_PEI_PPI_DESCRIPTOR mRecoveryModePpi[] = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiBootInRecoveryModePpiGuid, NULL
};
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InvalidateFvHob
//
// Description: 
//
// Input:       
//
// Output:      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InvalidateFvHob (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PHYSICAL_ADDRESS         qFvAddress,
    IN HOB_ROM_IMAGE                *pRamBootHob         
)
{
    VOID                    *p;
    EFI_PHYSICAL_ADDRESS	qFvMemAddress = qFvAddress;
    UINT32                  i;
    
    for (i = 0; i < pRamBootHob->NumOfFv; i++) {
        if (pRamBootHob->FvInfo[i].FvAddress != (UINT32)qFvAddress) continue;
        if (pRamBootHob->FvInfo[i].FvMemReady == FALSE) continue;
        qFvMemAddress = (EFI_PHYSICAL_ADDRESS)pRamBootHob->FvInfo[i].MemAddress;
        // Corrupt FV contains in memory.
        ((EFI_FIRMWARE_VOLUME_HEADER*)qFvMemAddress)->Signature = -1;
        (*PeiServices)->SetMem ( (UINT8*)qFvMemAddress + \
                            sizeof (EFI_FIRMWARE_VOLUME_HEADER), 0x100, 0xff);
        break;    
    }
    for ((*PeiServices)->GetHobList(PeiServices,&p); \
                !(FindNextHobByType(EFI_HOB_TYPE_FV,&p)); ) {
        if (((EFI_HOB_FIRMWARE_VOLUME*)p)->BaseAddress != qFvMemAddress) continue;
        // Invalidate FV HOB    
        ((EFI_HOB_FIRMWARE_VOLUME*)p)->Header.HobType = EFI_HOB_TYPE_UNUSED;
//-SMDbgPrint("   HOB Address %x\n", (UINT32)((EFI_HOB_FIRMWARE_VOLUME*)p)->BaseAddress);
        break;
    }    
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VerifyFvBeforePublish
//
// Description: Function to publish PEI Firmware Volumes in PEI phase after
//              memory is available
//
// Input:       IN EFI_FFS_FILE_HEADER *FfsHeader - pointer to image FFS file
//              IN EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//
// Output:      EFI_SUCCESS if all FVs published successfully
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
VerifyFvBeforePublish (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
)
{
    VOID                    *p;
    INTN                    Result;
    UINT32                  i, Stage2MemBase, FvBbMemoryBase;
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_BOOT_MODE           BootMode = BOOT_WITH_FULL_CONFIGURATION;
    HOB_ROM_IMAGE           *pPrbHob;
    EFI_GUID                HobRomImageGuid = ROM_IMAGE_MEMORY_HOB_GUID;
    EFI_HOB_FIRMWARE_VOLUME *FVHob;  //<EIP134987+> 2013/09/04
    ROM_AREA                *RomLayout = NULL;  //<EIP134987+> 2013/09/09

    // 1. Find PeiRamBoot HOB. 
    for ((*PeiServices)->GetHobList(PeiServices,&p); \
            !(FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &p));	) {
        Result = guidcmp(&((EFI_HOB_GUID_TYPE*)p)->Name, &HobRomImageGuid);
        if (!Result) break;
    }
//<EIP134987*> 2013/09/04 <<<
    //if (Result != 0) return EFI_NOT_FOUND;
	//If didn't find PeiRamBoot hob, there might be corrupt firmware volume.
	//Calculate the hash directly.
    if( Result != 0 ) {
        Status = PeiHashSha256 (PeiServices, (UINT32)MICROCODE_ADDRESS, \
                                            HASH_DATA_SIZE, HASH_KEY_DATA_ADDRESS);
        if (!EFI_ERROR(Status)) return EFI_SUCCESS;
        Status = PeiHashSha256 (PeiServices, (UINT32)HASH_FV_BB_ADDRESS, \
                                HASH_FV_BB_SIZE, BB_HASH_KEY_DATA_ADDRESS);
        if(EFI_ERROR(Status)) {
            Status = PeiHashSha256 (PeiServices, (UINT32)HASH_FV_BB_BACKUP_ADDRESS, \
                                    HASH_FV_BB_SIZE, BB_HASH_KEY_DATA_ADDRESS);
            if(EFI_ERROR(Status)) {
                while(1) IoWrite8 (0x80, 0xdd);
            }
            //Publish FV
            Status = (*PeiServices)->CreateHob(
                                        PeiServices, 
                                        EFI_HOB_TYPE_FV,
                                        sizeof(EFI_HOB_FIRMWARE_VOLUME),
                                        &FVHob);
            if (EFI_ERROR(Status)) 
                return Status;
            FVHob->BaseAddress = (EFI_PHYSICAL_ADDRESS)HASH_FV_BB_BACKUP_ADDRESS;
            FVHob->Length = (UINT64)HASH_FV_BB_SIZE;
        }
        BootMode = BOOT_IN_RECOVERY_MODE;
        Status = (*PeiServices)->SetBootMode(PeiServices, BootMode);
        (*PeiServices)->InstallPpi(PeiServices, mRecoveryModePpi);
        return Status;
    }
//<EIP134987*> 2013/09/04 >>>
    // 2. Validate PeiRamBoot HOB
    for (i = 0, pPrbHob = (HOB_ROM_IMAGE*)p; i < pPrbHob->NumOfFv; i++) {
        if (pPrbHob->FvInfo[i].FvMemReady == 0) return EFI_NOT_READY;
        if (pPrbHob->FvInfo[i].FvAddress == MICROCODE_ADDRESS) 
            Stage2MemBase = pPrbHob->FvInfo[i].MemAddress;
        if (pPrbHob->FvInfo[i].FvAddress == FV_BB_ADDRESS) 
            FvBbMemoryBase = pPrbHob->FvInfo[i].MemAddress;
    }

//<EIP134987+> 2013/09/27 >>>
    Status = PeiHashSha256 (PeiServices, Stage2MemBase, \
                                        HASH_DATA_SIZE, HASH_KEY_DATA_ADDRESS);
    if (!EFI_ERROR(Status)) return EFI_SUCCESS;
//<EIP134987+> 2013/09/27 <<<

    // 3. Stage2 image Corrupted, re-copy Stage2 from ROM to RAM, then verify 
    //    it again.
    (*PeiServices)->CopyMem ((UINT8*)Stage2MemBase, \
                             (UINT8*)MICROCODE_ADDRESS, 
                             MICROCODE_SIZE + FV_MAIN_SIZE + FV_BB_SIZE);
    Status = PeiHashSha256 (PeiServices, Stage2MemBase, \
                                        HASH_DATA_SIZE, HASH_KEY_DATA_ADDRESS);
//-SMDbgPrint("   ---------- Copy and Verify Stage2 again : %r\n", Status);
    if (!EFI_ERROR(Status)) return EFI_SUCCESS;

    // 4. Stage2 image Corrupted, enter recovery mode, now verify FV_BB image.
    Status = PeiHashSha256 (PeiServices, FvBbMemoryBase, \
                                HASH_FV_BB_SIZE, BB_HASH_KEY_DATA_ADDRESS);
//-SMDbgPrint("   ---------- Verify FV BootBlock : %r\n", Status);
    if (EFI_ERROR(Status)) {
        // 5. FV_BB Image Corrupted, enter recovery mode, now verify 
        //    FV_BB_BACKUP image.
        Status = PeiHashSha256 (PeiServices, \
                                (UINT32)HASH_FV_BB_BACKUP_ADDRESS, \
                                HASH_FV_BB_SIZE, BB_HASH_KEY_DATA_ADDRESS);
//-SMDbgPrint("   ---------- Verify FV BootBlock Backup : %r\n", Status);
        if (EFI_ERROR(Status)) {
        // 6. FV_BB_BACKUP Image Corrupted, no secure BootBlock image, 
        //    halt the system.
            while(1) { IoWrite8 (0x80, 0xdd); }    
        }
        (*PeiServices)->CopyMem ((UINT8*)FvBbMemoryBase, \
                                 (UINT8*)HASH_FV_BB_BACKUP_ADDRESS, \
                                 FV_BB_SIZE);    

    }

//<EIP134987+> 2013/09/09 >>>
    Status = GetRomLayout(PeiServices, &RomLayout);
    if ( EFI_ERROR(Status) || (RomLayout == NULL) ) {
        return Status;
    } else {
        while(RomLayout->Size != 0) {
            if(RomLayout->Address == FV_BB_ADDRESS) {
                RomLayout->Address = FvBbMemoryBase;
                break;
            }
            RomLayout++;
        }
    }
//<EIP134987+> 2013/09/09 <<<

    BootMode = BOOT_IN_RECOVERY_MODE;
    Status = (*PeiServices)->SetBootMode(PeiServices, BootMode);
    (*PeiServices)->InstallPpi(PeiServices, mRecoveryModePpi);
    InvalidateFvHob(PeiServices, FV_MAIN_BASE, pPrbHob);
    return  EFI_SUCCESS;
}
//(CSP20130813A+)<<
//<AMI_PHDR_START>
//=============================================================================
// Procedure:			PeiHash
//
// Description:  
//
// Input:
//
// Output: 
//
// Returns:
//
// Notes:
//
//=============================================================================
//<AMI_PHDR_END>
EFI_STATUS PeiHash (
  IN CONST EFI_GUID               *HashAlgorithm,
  IN UINTN                        num_elem,
  IN CONST UINT8                  *addr[],
  IN CONST UINTN                  *len,
  OUT UINT8                       *Hash
  )
{
    UINT32      HashLen = HASH_SHA256_LEN;

    MemSet(Hash, HashLen, 0);
    sha256_vector(num_elem, addr, len, Hash);

    return  EFI_SUCCESS;
}

//<AMI_PHDR_START>
//=============================================================================
// Procedure:			GetExistHasValue
//
// Description:  		
//
// Input:
//
// Output: 
//
// Returns:
//
// Notes:
//
//=============================================================================
//<AMI_PHDR_END>
//UINT8* GetExistHasValue(){
//	//Porting that you want to get HASH value store in ROM image.
//	
//	return (UINT8*)(0xffffffff-0x20000+0x58+1);
//}

//<AMI_PHDR_START>
//=============================================================================
// Procedure:			VerifyHashValue
//
// Description:  
//
// Input:
//
// Output: 
//
// Returns:
//
// Notes:
//
//=============================================================================
//<AMI_PHDR_END>
//(EIP124704)>>
EFI_STATUS VerifyHashValue(
  IN UINT8 *Digest,
  IN UINT32 HashAddress
  )
{
	UINTN		HashLen = HASH_SHA256_LEN;
	UINT8*		pExistHashValue;
	
//	pExistHashValue = GetExistHasValue();
	pExistHashValue = (UINT8*) HashAddress;
//(EIP124704)<<
	if(pExistHashValue == NULL)
		return EFI_NOT_FOUND;
	
	if((*(UINT32*)pExistHashValue ) == 0x00000000) return EFI_NOT_FOUND; 
 	
	if(!MemCmp(pExistHashValue, Digest, HashLen))
		return EFI_SUCCESS;
	else
		return EFI_SECURITY_VIOLATION;
}
//(EIP119182)>>
//<AMI_PHDR_START>
//=============================================================================
// Procedure:           PeiHashSha256
//
// Description:  
//
// Input:
//
// Output: 
//
// Returns:
//
// Notes:
//
//=============================================================================
//<AMI_PHDR_END>
//(EIP124704)>>
EFI_STATUS PeiHashSha256(
    IN EFI_PEI_SERVICES **PeiServices,
    IN UINT32                      Address,
    IN UINT32                      Size,
    IN UINT32                      HashAddress
){
    
    EFI_STATUS  Status = EFI_SUCCESS;
    EFI_GUID    gEfiHashAlgorithmSha256Guid = EFI_HASH_ALGORITHM_SHA256_GUID;
    AMI_CRYPT_DIGITAL_SIGNATURE_PPI *pAmiSigPPI = NULL;
    UINT8   *Addr[1];
    UINTN   Len[1];
    UINT8   Digest[HASH_SHA256_LEN];

    Addr[0] = (UINT8*)(Address); 
    
//  Addr[0] = (UINT8*)HASH_DATA_ADDRESS;
    //Len[0] = HASH_DATA_SIZE;
    Len[0] = Size;
    
    Status = PeiHash(&gEfiHashAlgorithmSha256Guid, 
                    1, Addr,  Len, (UINT8*)&Digest[0] );
    IoWrite8(0x80,0x41); 
    PEI_TRACE((-1, PeiServices, "Hash the FV_MAIN, Addr[0x%x],Size[0x%x], %r\n",Addr[0] ,Len[0], Status));
    if(EFI_ERROR(Status))
        return Status;
    {
        UINT8   i;
        PEI_TRACE((-1, PeiServices, "FV_MAIN Hash data as below : \n"));
        for(i = 0; i < HASH_SHA256_LEN; i++){
            if(i == 16)
                PEI_TRACE((-1,PeiServices,"\n"));
            
            if((i == 0)||(i == 16))
                PEI_TRACE((-1,PeiServices,"[%02X]",Digest[i]));
            else
                PEI_TRACE((-1,PeiServices,"-[%02X]",Digest[i]));
        }
        PEI_TRACE((-1,PeiServices,"-END \n"));
    }

    Status = VerifyHashValue(Digest,HashAddress);
    PEI_TRACE((-1, PeiServices, "VerifyHashValue %r\n", Status));
    
    if(Status == EFI_SECURITY_VIOLATION)
    {
        PEI_TRACE((-1,PeiServices,"Verify Boot Hash Data match error.\n"));
        IoWrite8(0x80, 0xEE);
//        EFI_DEADLOOP();
    }   
    return Status;
//(EIP124704)<<
}


//<AMI_PHDR_START>
//=============================================================================
// Procedure:			PHashDPeiEntry
//
// Description:  
//
// Input:
//
// Output: 
//
// Returns:
//
// Notes:
//
//=============================================================================
//<AMI_PHDR_END>
EFI_STATUS PHashDPeiEntry (
	IN	EFI_FFS_FILE_HEADER     *FfsHeader,
	IN	EFI_PEI_SERVICES        **PeiServices
  )
{
	EFI_STATUS	Status = EFI_SUCCESS;
    EFI_BOOT_MODE           BootMode = BOOT_WITH_FULL_CONFIGURATION;  //(CSP20130813A+)

//	EFI_GUID	gEfiHashAlgorithmSha256Guid = EFI_HASH_ALGORITHM_SHA256_GUID;
//	AMI_CRYPT_DIGITAL_SIGNATURE_PPI	*pAmiSigPPI = NULL;
//	UINT8 	*Addr[1];
//	UINTN   Len[1];
//	UINT8	Digest[HASH_SHA256_LEN];
//    VOID                            *Hob;
//    EFI_HOB_FV_ADDRESS                  *FVAddress;
//    EFI_GUID                        AmiPlatformFVAddressHobGuid = AMI_PLATFORM_FV_ADDRESS_HOB_GUID;

    
    // Install the PHashDPeiEntry Init Policy PPI
    Status = (*PeiServices)->InstallPpi(PeiServices, &mPpiList[0]);
    ASSERT_PEI_ERROR(PeiServices, Status);    
    
//(CSP20130813A+)>>
    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if (EFI_ERROR(Status) || (BootMode == BOOT_ON_S3_RESUME)) return EFI_SUCCESS;

//CSP20140330_22 >>
#if DEBUG_MODE == 1
    VerifyFvBeforePublish (PeiServices, NULL, NULL);
#endif
//CSP20140330_22 <<

    Status = (*PeiServices)->NotifyPpi ( PeiServices, \
                                         VerifyFvBeforePublishNotify );
//(CSP20130813A+)<<
/*    
    (*PeiServices)->GetHobList(PeiServices, &Hob);
    
    FVAddress = (EFI_HOB_FV_ADDRESS*)Hob;
    while(!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &FVAddress))) {
        if(guidcmp(&FVAddress->EfiHobGuidType.Name, &AmiPlatformFVAddressHobGuid) == 0) {
            PEI_TRACE((TRACE_PEICORE, PeiServices, "Found the FV address HOB 2 \n"));
            break;
        }
    }
    IoWrite8(0x80,0x40); 
    Addr[0] = (UINT8*)(FVAddress->Address);	
	
//	Addr[0] = (UINT8*)HASH_DATA_ADDRESS;
	Len[0] = HASH_DATA_SIZE;
	
	Status = PeiHash(&gEfiHashAlgorithmSha256Guid, 
	                1, Addr,  Len, (UINT8*)&Digest[0] );
    IoWrite8(0x80,0x41); 
	PEI_TRACE((-1, PeiServices, "Hash the FV_MAIN, Addr[0x%x],Size[0x%x], %r\n",Addr[0] ,Len[0], Status));
	if(EFI_ERROR(Status))
		return Status;
	{
		UINT8	i;
		PEI_TRACE((-1, PeiServices, "FV_MAIN Hash data as below : \n"));
		for(i = 0; i < HASH_SHA256_LEN; i++){
			if(i == 16)
				PEI_TRACE((-1,PeiServices,"\n"));
			
			if((i == 0)||(i == 16))
				PEI_TRACE((-1,PeiServices,"[%02X]",Digest[i]));
			else
				PEI_TRACE((-1,PeiServices,"-[%02X]",Digest[i]));
		}
		PEI_TRACE((-1,PeiServices,"-END \n"));
	}

	Status = VerifyHashValue(Digest);
	PEI_TRACE((-1, PeiServices, "VerifyHashValue %r\n", Status));
	
	if(Status == EFI_SECURITY_VIOLATION)
	{
		PEI_TRACE((-1,PeiServices,"Verify Boot Hash Data match error.\n"));
		IoWrite8(0x80, 0xDD);
		EFI_DEADLOOP();
	}	
*/
	return Status;
}
//(EIP119182)<<
//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
