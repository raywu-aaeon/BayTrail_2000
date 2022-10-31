//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1987-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SharkBayRefCodes/Haswell/AMI Cpu PKG/CPU Core/CpuSmbios.c 4     2/27/13 5:00a Crystallee $
//
// $Revision: 4 $
//
// $Date: 2/27/13 5:00a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SharkBayRefCodes/Haswell/AMI Cpu PKG/CPU Core/CpuSmbios.c $
// 
// 4     2/27/13 5:00a Crystallee
// [TAG]  		EIP115822
// [Category]  	Improvement
// [Description]  	Add SMBIOS type7 information for L4 cache if CPU
// supported
// And create a token to control this.
// 
// 3     3/21/12 4:33a Davidhsieh
// 
// 2     3/20/12 3:23a Davidhsieh
// Create SMBIOS type4 and type7 in AMI CPU module part
// 
// 1     2/07/12 3:58a Davidhsieh
//
// 
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiCpuSmbios.c
//
// Description:
//  Installs TYPE 4 and TYPE 7 SMBIOS tables.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiDxeLib.h>
#include <Protocol\AmiSmBios.h>
#include <Protocol\AmiCpuInfo.h>
#include <Library\CpuCspLib.h>
#include <Token.h>
#include "Cpu.h"

AMI_CPU_INFO                  *gGetCpuInfo = NULL;
EFI_SMBIOS_PROTOCOL           *gSmbiosProtocol;

EFI_EVENT 	gSmbiosEvent;
VOID		*gSmbiosRegistration = 0;

UINT8 GetCacheSharedThreads(IN UINT8 Level);

CACHE_DESCRIPTOR_INFO *gCacheDescInfo;
UINT8 gCacheTypeTable[] = {4, 3, 0, 5};
UINT8 gAssociativityTable[] = {2, 6 ,4 ,1 ,5 ,1 ,1 ,1 ,7 ,1 ,1 ,1 ,9 ,1 ,1, 1, 8, 1, 1, 1, 0xe,
    1, 1, 1, 0xa, 1, 1, 1, 1, 1, 1, 1, 0xb
};

typedef struct {
    UINT32 Size;
    UINT8 Type;
    UINT8 Assoc;
} CACHE_INFO;

#if !CPU_CACHE_L4_DISPLAY_IN_SMBIOS
CACHE_INFO gCacheInfo[3]; //[0] = L1, [1] = L2, [2] = L3
#else
CACHE_INFO gCacheInfo[4]; //[0] = L1, [1] = L2, [2] = L3, [3] = L4
#endif

#define SMBIOS_MAX_NUM_SOCKETS 8

CHAR8 *gSocketDesgination[8] = {
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_0),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_1),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_2),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_3),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_4),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_5),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_6),
	CONVERT_TO_STRING(SMBIOS_TYPE_4_SOCKET_DESIGINTATION_SOCKET_7)
};

typedef struct {
    //Private Data
    UINT8 *StrBuf;
    INT32 StrBufSize;
    INT32 StrBufAvail;
    UINT8 Tok;
} SMBIOS_TABLE_STR_BUFFER;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   EstablishStringBuffer
//
// Description: Initialize string buffer variables
//
// Input:
//  IN VOID *Buffer - Buffer address
//  IN UINT32 Size - Buffer Size
//  OUT SMBIOS_TABLE_STR_BUFFER **StrBuffer - String Buffer structure
//
// Output: VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EstablishStringBuffer(
    IN  VOID   *Buffer,
    IN  UINT32  Size,
    OUT SMBIOS_TABLE_STR_BUFFER **StrBuffer
)
{
    EFI_STATUS Status;
    SMBIOS_TABLE_STR_BUFFER *Buf;

    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(SMBIOS_TABLE_STR_BUFFER), &Buf);
    ASSERT_EFI_ERROR(Status);
  
    Buf->StrBuf = Buffer;
    Buf->StrBufSize = Size;
    Buf->StrBufAvail = Size;
    Buf->Tok = 0;

    *StrBuffer = Buf;

}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   AddStrGetTok
//
// Description: Add string to buffer and return token value.
//
// Input:
//  IN SMBIOS_TABLE_STR_BUFFER *StrBuffer - String Buffer structure
//  IN UINT8 *Str - Pointer to string.
//
// Output: UINT8 - token of string
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 AddStrGetTok(
    IN SMBIOS_TABLE_STR_BUFFER *StrBuffer,
    IN UINT8 *Str
)
{
    INT32 Len = (INT32)Strlen(Str);

    if (Len == 0) return 0;                 //No string

    //For string buffer, 2 bytes at end reserved for double 0, so that is why
    //gStrBufAvail - 2
    if (Len > (StrBuffer->StrBufAvail - 2)) return 0; //Not enough space left.

    Strcpy(StrBuffer->StrBuf, Str);
    StrBuffer->StrBuf += Len + 1;
    StrBuffer->StrBufAvail -= Len + 1;
    ++StrBuffer->Tok;
    return StrBuffer->Tok;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  GetBufferSizeUsed
//
// Description: Get amount storage space used by strings.
//
// Input:
//  IN SMBIOS_TABLE_STR_BUFFER *StrBuffer - String Buffer structure
//
// Output: UINT32 - Size needed for strings.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 GetBufferSizeUsed(IN SMBIOS_TABLE_STR_BUFFER *StrBuffer)
{
    return StrBuffer->StrBufSize - StrBuffer->StrBufAvail;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  RemoveStringBuffer
//
// Description: Remove String Buffer structure.
//
// Input:
//  IN SMBIOS_TABLE_STR_BUFFER *StrBuffer - String Buffer structure
//
// Output: VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID RemoveStringBuffer(IN SMBIOS_TABLE_STR_BUFFER *StrBuffer)
{
    pBS->FreePool(StrBuffer);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GetMaxSpeedFromBrandString
//
// Description: Get the max speed from the brand string.
//
// Input:
//  IN CHAR8    *CpuBrandString - Pointer to CPU brand string.
//
// Output:  UINT32 - frequency found in MHz.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 GetMaxSpeedFromBrandString(IN CHAR8 *CpuBrandString)
{
    UINT32  i;
    UINT8   CharCount;
    BOOLEAN TransToMHz = FALSE;
    BOOLEAN FreqStringFound = FALSE;
    CHAR8   FrequencyString[] = "0000";

    while (*CpuBrandString != 0) {
        if (*CpuBrandString == 'G' && *(CpuBrandString + 1) == 'H' && *(CpuBrandString + 2) == 'z') {
            FreqStringFound = TRUE;
            TransToMHz = TRUE;
            break;
        } else if (*CpuBrandString == 'M' && *(CpuBrandString+1) == 'H' && *(CpuBrandString + 2) == 'z') {
            FreqStringFound = TRUE;
            break;
        } else ++CpuBrandString;
    }

    --CpuBrandString;   //first numeric char

    //search numeric char
    CharCount = 0;
    for(i = 0 ; i < 4; ++i) {
        if (*CpuBrandString >= '0' && *CpuBrandString <= '9') {
            --CpuBrandString;
            ++CharCount;
        } else if (*CpuBrandString == '.') {
            --CpuBrandString;
            ++CharCount;
        } else break;
    }

    ++CpuBrandString;   //first numeric char

    if (FreqStringFound && CharCount > 0) {
        for(i = 0; i < CharCount; ++i) {
            if (TransToMHz && *CpuBrandString == '.') CpuBrandString++;

            FrequencyString[i] = *CpuBrandString;
            ++CpuBrandString;
        }
        if (TransToMHz) FrequencyString[3] = '0';
    } else FreqStringFound = FALSE;

    return FreqStringFound ? Atoi(FrequencyString) : 0;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  CollectCacheInfo
//
// Description: Store cache information in variables.
//
// Input:  VOID
//
// Output: VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CollectCacheInfo()
{
    INT32 i;
    UINT8 Type;
    UINT8 Assoc;
    UINT32 NumCores = NumSupportedCpuCores();

    MemSet(gCacheInfo, sizeof(gCacheInfo), 0);

    gCacheDescInfo = gGetCpuInfo->CacheInfo;
    for(i = 0; gCacheDescInfo[i].Desc; ++i) {  // End of descriptors, Desc = 0.
        UINT8 Level = gGetCpuInfo->CacheInfo[i].Level;    
        switch(Level) {
#if !CPU_CACHE_L4_DISPLAY_IN_SMBIOS
        case 1: case 2: case 3:
#else
		case 1: case 2: case 3: case 4:
#endif
            Type = gCacheTypeTable[gCacheDescInfo[i].Type];
            if (gCacheDescInfo[i].Associativity < sizeof(gAssociativityTable)/sizeof(UINT8))
                Assoc = gAssociativityTable[gCacheDescInfo[i].Associativity];
            else Assoc = 1;
            
            //If multiple caches of same level, add sizes.
            gCacheInfo[Level - 1].Size  += gCacheDescInfo[i].Size;

            //If multiple caches of same level have different types report as other.
            if (gCacheInfo[Level - 1].Type == 0) gCacheInfo[Level - 1].Type = Type;
            else if (gCacheInfo[Level - 1].Type != Type) gCacheInfo[Level - 1].Type = 1;

            if (gCacheInfo[Level - 1].Assoc == 0) gCacheInfo[Level - 1].Assoc = Assoc;
            else if (gCacheInfo[Level - 1].Assoc != Assoc) gCacheInfo[Level - 1].Assoc = 1;
        }
    }

    if (GetCacheSharedThreads(1) <= 2) gCacheInfo[0].Size *= NumCores;
    if (GetCacheSharedThreads(2) <= 2) gCacheInfo[1].Size *= NumCores;
    if (GetCacheSharedThreads(3) <= 2) gCacheInfo[2].Size *= NumCores;
    //Coner case, when L2 is the last level cache
    if (!(gCacheInfo[2].Size) && (NumCores > 1)) {
		gCacheInfo[1].Size *= NumCores>>1;
#if ValleyView_Industry == 1
{
		UINT8  MsrPlatform;
		MsrPlatform = (UINT8) (RShiftU64 ((AsmReadMsr64 (MSR_IA32_PLATFORM_ID) & B_PLATFORM_ID_BITS_MASK),  \
				N_PLATFORM_ID_BITS));
		if ((NumCores == 2) && (MsrPlatform == 0)){
		    gCacheInfo[1].Size = gCacheInfo[1].Size<<1;
			}
}
#endif
    	}

#if CPU_CACHE_L4_DISPLAY_IN_SMBIOS
	if (GetCacheSharedThreads(4) <= 2) gCacheInfo[3].Size *= NumCores;
#endif
}

#define TYPE7_STRING_BUFFER_SIZE 100
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  CreateSmbiosTable7
//
// Description: Create SMBIOS Table 7
//
// Input:  IN UINT8 CacheLevel (L1, L2, L3)
//
// Output: UINT16 - Handle for table.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16 CreateSmbiosTable7(IN UINT8 CacheLevel)
{
    SMBIOS_CACHE_INFO *CacheInfo;
    UINT16 Handle = 0xfffe;
    EFI_STATUS  Status;
    UINT32  Type7ActStrBufSize;
    SMBIOS_TABLE_STR_BUFFER *StrBuffer;

    if (gCacheInfo[CacheLevel-1].Size == 0) return 0xffff;

    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(SMBIOS_CACHE_INFO) + TYPE7_STRING_BUFFER_SIZE, &CacheInfo);
    ASSERT_EFI_ERROR(Status);

    EstablishStringBuffer((UINT8*)CacheInfo + sizeof(SMBIOS_CACHE_INFO), TYPE7_STRING_BUFFER_SIZE, &StrBuffer);

    CacheInfo->StructureType.Type = 7;
    CacheInfo->StructureType.Length = 0x13;
    CacheInfo->StructureType.Handle = 0xfffe;	//To be updated by SMBIOS driver.
    CacheInfo->SocketDesignation = 0;

    CacheInfo->MaxCacheSize = gCacheInfo[CacheLevel-1].Size;
    CacheInfo->InstalledSize = gCacheInfo[CacheLevel-1].Size;
    CacheInfo->SystemCacheType = gCacheInfo[CacheLevel-1].Type;
    CacheInfo->Associativity  = gCacheInfo[CacheLevel-1].Assoc;

    switch (CacheLevel) {
    case 1:
        CacheInfo->CacheConfig = 0x180;
#ifdef SMBIOS_TYPE_7_L1_SOCKET_DESIGNATION    
        CacheInfo->SocketDesignation = AddStrGetTok(StrBuffer, CONVERT_TO_STRING(SMBIOS_TYPE_7_L1_SOCKET_DESIGNATION));
#endif
        break;
    case 2:
        CacheInfo->CacheConfig = 0x181;
#ifdef SMBIOS_TYPE_7_L2_SOCKET_DESIGNATION    
        CacheInfo->SocketDesignation = AddStrGetTok(StrBuffer, CONVERT_TO_STRING(SMBIOS_TYPE_7_L2_SOCKET_DESIGNATION));
#endif
        break;
    case 3:
        CacheInfo->CacheConfig = 0x182;
#ifdef SMBIOS_TYPE_7_L3_SOCKET_DESIGNATION    
        CacheInfo->SocketDesignation = AddStrGetTok(StrBuffer, CONVERT_TO_STRING(SMBIOS_TYPE_7_L3_SOCKET_DESIGNATION));
#endif
        break;
#if CPU_CACHE_L4_DISPLAY_IN_SMBIOS
	case 4:
		CacheInfo->CacheConfig = 0x183;
#ifdef SMBIOS_TYPE_7_L4_SOCKET_DESIGNATION
		CacheInfo->SocketDesignation = AddStrGetTok(StrBuffer, CONVERT_TO_STRING(SMBIOS_TYPE_7_L4_SOCKET_DESIGNATION));
#endif
		break;
#endif
    }

    CacheInfo->SupportSRAM = 2;
    CacheInfo->CurrentSRAM = 2;
    CacheInfo->CacheSpeed = 0;    
    CacheInfo->ErrorCorrectionType = 0x05;  //ECC
    Type7ActStrBufSize = GetBufferSizeUsed(StrBuffer);

    if (Type7ActStrBufSize == 0) {
        *(UINT16*)(CacheInfo + 1) = 0;    //Double NULL.
    } else {
        *((UINT8*)(CacheInfo + 1) + Type7ActStrBufSize) = 0; //End structure of NULL.
    }

    Status = gSmbiosProtocol->Add(
			gSmbiosProtocol,
			NULL,
			&Handle,
			(EFI_SMBIOS_TABLE_HEADER*)CacheInfo
	);

    return !EFI_ERROR(Status) ? Handle : 0xffff;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  AutodetectFamily
//
// Description: Return family type from Brand String.
//
// Input:  IN CHAR8 *BrandString
//
// Output: UINT8 - Family
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 AutodetectFamily(IN CHAR8 *BrandString)
{
    CHAR8 *p = BrandString;

    while(*p) {
        if (MemCmp(p, "Xeon", 4) == 0) return 0xb3;
        if (MemCmp(p, "i7", 2) == 0) return 0xc6;
        if (MemCmp(p, "i5", 2) == 0) return 0xcd;
        if (MemCmp(p, "i3", 2) == 0) return 0xce;
        if (MemCmp(p, "Pentiu", 6) == 0) return 0x0b;
        if (MemCmp(p, "Celero", 6) == 0) return 0x0f;
        if (MemCmp(p, "Atom", 4) == 0) return 0x2b;		
        ++p;
    }

    return 0xcd;    //default as i5 family
}

#define TYPE4_STRING_BUFFER_SIZE 200

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  CreateSmbiosTable4
//
// Description: Create SMBIOS Table 4
//
// Input:  IN UINT32 PhysSocket
//
// Output: VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CreateSmbiosTable4(IN UINT32 PhysSocket)
{
    SMBIOS_PROCESSOR_INFO * ProcInfo;
    EFI_STATUS Status;
    UINT32  ActStrBufSize;
    UINT32  MaxFreqBrandStr;
    SMBIOS_TABLE_STR_BUFFER *StrBuffer;
    BOOLEAN HandleExists = FALSE;
	UINT16 Handle = 0xfffe;

    Status = pBS->AllocatePool(
        EfiBootServicesData, sizeof(SMBIOS_PROCESSOR_INFO) + TYPE4_STRING_BUFFER_SIZE, &ProcInfo
    );
    
    EstablishStringBuffer((UINT8*)ProcInfo + sizeof(SMBIOS_PROCESSOR_INFO), TYPE4_STRING_BUFFER_SIZE, &StrBuffer);

    ProcInfo->StructureType.Type = 4;
    ProcInfo->StructureType.Length = 0x2a;
    ProcInfo->StructureType.Handle = 0xfffe;    //To be updated by SMBIOS driver.   
 
    if (PhysSocket >= SMBIOS_MAX_NUM_SOCKETS) ProcInfo->SocketDesignation = 0;
    else ProcInfo->SocketDesignation = AddStrGetTok(StrBuffer, gSocketDesgination[PhysSocket]);

    ProcInfo->ProcessotType = 3;    //Central processor

    if (SMBIOS_TYPE_4_PROC_FAMILY != 0) ProcInfo->Family = SMBIOS_TYPE_4_PROC_FAMILY;
    else ProcInfo->Family = AutodetectFamily(gGetCpuInfo->BrandString);

    ProcInfo->ProcessorManufacturer = AddStrGetTok(StrBuffer, "Intel");

    ProcInfo->ProcessorID_1 = (UINT32)(gGetCpuInfo->Version);
    ProcInfo->ProcessorID_2 = (UINT32)(gGetCpuInfo->Features);
    ProcInfo->ProcessorVersion = AddStrGetTok(StrBuffer, gGetCpuInfo->BrandString);    
    ProcInfo->Voltage = (UINT8)(12) + BIT7; // 1.2 volts
    //ProcInfo->Voltage = BIT7; // 1.2 volts
    ProcInfo->MaxSpeed = SMBIOS_TYPE_4_MAX_SPEED;

    MaxFreqBrandStr = GetMaxSpeedFromBrandString(gGetCpuInfo->BrandString);
    if (MaxFreqBrandStr) ProcInfo->CurrentSpeed = MaxFreqBrandStr;
    else ProcInfo->CurrentSpeed = gGetCpuInfo->IntendedFreq;

    ProcInfo->ExtClockFreq = (UINT16)(gGetCpuInfo->FSBFreq);
    ProcInfo->Status = 0x41;    //Populated and enabled.
    ProcInfo->Upgrade = SMBIOS_TYPE_4_PROC_UPGRADE;    
    ProcInfo->SerialNumber = 0;

#ifdef SMBIOS_TYPE_4_ASSET_TAG
    ProcInfo->AssetTag = AddStrGetTok(StrBuffer, CONVERT_TO_STRING(SMBIOS_TYPE_4_ASSET_TAG));
#else
    ProcInfo->AssetTag = 0;
#endif
#ifdef SMBIOS_TYPE_4_PART_NUMBER
    ProcInfo->PartNumber = AddStrGetTok(StrBuffer, CONVERT_TO_STRING(SMBIOS_TYPE_4_PART_NUMBER));
#else
    ProcInfo->PartNumber = 0;
#endif
    ProcInfo->CoreCount = NumSupportedCpuCores();   //This must be the same across sockets.
    ProcInfo->CoreEnabled = gGetCpuInfo->NumCores;
    ProcInfo->ThreadCount = NumSupportedCpuCores() * NumSupportedThreadsPerCore(); //This must be the same across sockets.
    ProcInfo->ProcessorChar = 4; //X64 Support
    ProcInfo->Family2 = ProcInfo->Family;

    ActStrBufSize = GetBufferSizeUsed(StrBuffer);
    if (ActStrBufSize == 0) {
        *(UINT16*)(ProcInfo + 1) = 0;    //Double NULL.
    } else {
        *((UINT8*)(ProcInfo + 1) + ActStrBufSize) = 0; //End structure of NULL.
    }
	
    ProcInfo->L1CacheHandle = CreateSmbiosTable7(1);
    ProcInfo->L2CacheHandle = CreateSmbiosTable7(2);
    ProcInfo->L3CacheHandle = CreateSmbiosTable7(3);
#if CPU_CACHE_L4_DISPLAY_IN_SMBIOS
	CreateSmbiosTable7(4);
#endif

	Status = gSmbiosProtocol->Add(
			gSmbiosProtocol,
			NULL,
			&Handle,
			(EFI_SMBIOS_TABLE_HEADER*)ProcInfo
	);			
    
    pBS->FreePool(ProcInfo);
    RemoveStringBuffer(StrBuffer);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  CreateCpuSmbiosTables
//
// Description: Create CPU SMBIOS Tables 4 and 7.
//
// Input:
//  IN EFI_EVENT Event - Not used
//  IN VOID *Context - Note Used
//
// Output: VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CreateCpuSmbiosTables(IN EFI_EVENT Event, IN VOID *Context)
{
    EFI_STATUS 				Status;
    UINT32 					SocketPopBitmap = 0;
    EFI_SMBIOS_TABLE_HEADER *Hdr;
	EFI_SMBIOS_HANDLE 		Handle;
	EFI_SMBIOS_TYPE			Type;
	UINT32 					Cpu = 0;
	AMI_CPU_INFO_PROTOCOL   *gAmiCpuInfoProtocol;
	EFI_GUID gAmiCpuInfoProtocolGuid    = AMI_CPU_INFO_PROTOCOL_GUID;
	
    ASSERT(NUMBER_CPU_SOCKETS <= SMBIOS_MAX_NUM_SOCKETS);
	
    Status = pBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, &gSmbiosProtocol);
    if (EFI_ERROR(Status)) return;

	//Delete existing CPU SBIOS structures type 4 and type 7.
	Type = 4;
	Handle = 0xfffe;
	while (!EFI_ERROR(gSmbiosProtocol->GetNext(gSmbiosProtocol, &Handle, &Type, (EFI_SMBIOS_TABLE_HEADER**)&Hdr, NULL))) {
        gSmbiosProtocol->Remove(gSmbiosProtocol, Handle);
		Handle = 0xfffe;
    }

	Type = 7;
	Handle = 0xfffe;
	while (!EFI_ERROR(gSmbiosProtocol->GetNext(gSmbiosProtocol, &Handle, &Type, (EFI_SMBIOS_TABLE_HEADER**)&Hdr, NULL))) {	
		gSmbiosProtocol->Remove(gSmbiosProtocol, Handle);
		Handle = 0xfffe;
    }

	Status = pBS->LocateProtocol (&gAmiCpuInfoProtocolGuid, NULL, &gAmiCpuInfoProtocol);
	if(!EFI_ERROR(Status)) {
		Status = gAmiCpuInfoProtocol->GetCpuInfo(gAmiCpuInfoProtocol, Cpu, &gGetCpuInfo);
		//Add CPU SMBIOS structures
		CollectCacheInfo();
		CreateSmbiosTable4(0);
	}
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  CpuSmbios
//
// Description: Create CPU SMBIOS Tables. Installs notification on SMBIOS handlers.
//
// Input: VOID
//
// Output: VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiCpuSmbiosEntryPoint(
	IN EFI_HANDLE 		ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status;
    
    InitAmiLib(ImageHandle, SystemTable);

    Status = RegisterProtocolCallback(
    	&gEfiSmbiosProtocolGuid,
        CreateCpuSmbiosTables,
        NULL,
        &gSmbiosEvent,
        &gSmbiosRegistration
    );
    ASSERT_EFI_ERROR(Status);

   CreateCpuSmbiosTables(gSmbiosEvent, NULL);
   
   return EFI_SUCCESS;
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1987-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
