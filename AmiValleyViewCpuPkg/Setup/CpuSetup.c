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

//**********************************************************************
// $Header: /Alaska/SOURCE/CPU/Intel/Nehalem/CPUSetup.c 28    3/22/11 6:43p Markw $
//
// $Revision: 28 $
//
// $Date: 3/22/11 6:43p $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/CPU/Intel/Nehalem/CPUSetup.c $
// 
// 28    3/22/11 6:43p Markw
// [TAG]  		EIP55581
// [Category]  	New Feature
// [Description]  	Add Ivy Bridge Support.
// [Files]  		CpuPeiBoard.c, CpuDxeBoard.c, CpuSetup.c, PlatformCpuLib.c,
// Cpu.h, CpuCspLib.c, CpuPei.c, PeiCpuCache.c, CpuDxe.c
// 
// 27    3/11/11 5:21p Markw
// Fix on previous ver 26 for 52658. Fix if statement using IsHtSupported.
// This is a boolean.
// 
// 26    3/10/11 7:31p Markw
// [TAG]  		EIP52658
// [Category]  	Bug Fix
// [Severity]  	Normal
// [Symptom]  	Displaying number of Hyper-threading support and number of
// cores of currently enabled instead of supported.
// [RootCause]  	Reading variables for enabled instead of supported.
// [Solution]  	Update AMI CPU info2 protocol to support getting supported
// and using these values.
// [Files]  		CpuDxeBoard.c, CpuDxe.h, CpuDxe.c, CpuSetup.c
// 
// 25    1/03/11 4:04p Markw
// [TAG]  		EIP47298
// [Category]  	New Feature
// [Description]  	Added data to setup for each socket:
// * Max CPU speed/Min CPU Speed
// * Number of cores/HT support
// * VT-d (VMX) support/SMX support
// * Cache information
// [Files]  		CpuSetup.c, Cpu.uni, Cpu.sd, Cpu.sdl
// 
// 24    11/12/10 1:31p Markw
// [TAG]  		EIP47298
// [Category]  	Improvement
// [Description]  	Display setup information for 2 CPUs.
// [Files]  		Cpu.sd, Cpu.uni, CpuSetup.c, PlatformCpuLib.h,
// PlatformCpuLib.c, CpuDxe.c, CpuSmbios.c
// 
// 23    11/05/10 1:37p Markw
// [TAG]  		EIP40858
// [Category]  	Improvement
// [Description]  	Provide a setup question set maximum frequency
// including Turbo mode.
// Provide a setup question set boot frequency.
// This is enabled by tokens CPU_SETUP_SET_MAX_RATIO and
// CPU_SETUP_SET_BOOT_RATIO.
// This replaces SET_MAX_NON_TURBO and the CPU specific module no longer
// needs to be updated for this.
// [Files]  		CPU.sdl
// CPU.sd
// CPU.uni
// CPUSetup.c
// PlatformCpuLib.h
// PlatformCpuLib.c
// PowerManagment.c
// CpuPei.c
// 
// 22    10/09/10 11:27p Markw
// Fix Label 33.
// 
// 21    10/09/10 10:02p Markw
// Fix Label 32.
// 
// 20    9/15/10 9:08p Markw
// Add strings for maximum and minimum frequency.
// 
// 19    4/02/10 11:34a Markw
// Use SANDY_BRIDGE_VID_SUPPORT to enable/disable VID support. Defautlt
// disabled. Will always be availabe when fully tested.
// 
// 18    4/01/10 9:40p Markw
// Add commented out code to update STR_CPU_FACTY_VID_VALUE.
// 
// 17    3/08/10 3:01p Markw
// Add XE support for Sandy Bridge.
// 
// 16    1/08/10 2:26p Markw
// Update XE Factory defaults fro TDC/TDP.
// 
// 15    7/02/09 3:26p Markw
// Fix header.
// 
// 14    6/11/09 11:26a Markw
// Report number cores numerically.
// 
// 13    4/23/09 3:31p Markw
// Removed SEC Reference specific code. This will be part of a separte
// reference code module.
// 
// 12    3/04/09 11:33a Markw
// Update function header spacing.
// 
// 11    3/03/09 10:52p Markw
// Update copyright headers. 
// 
// 10    2/26/09 11:49a Markw
// Add reversion 8 changes back.
// 
// 9     2/02/09 5:36p Markw
// EIP #17903 BWG .55 -- Use Platform Policy CPU functions to read setup
// and implement recommended setup questions in BWG .55.
// 
// 8     1/13/09 3:23p Markw
// Use static to isolate global variables in this object from other
// objects.
// 
// 7     11/07/08 5:23p Markw
// Add CMOS usage as tokens.
// 
// 6     10/08/08 11:07a Markw
// 
// 5     8/21/08 4:09p Markw
// Add BSP switching support.
// 
// 4     6/15/08 3:16p Markw
// SMT, Num cores enabled, and VT no longer uses CMOS.
// 
// 3     5/08/08 4:03p Markw
// Update active core setting cmos.
// 
// 2     5/07/08 2:44p Markw
// C-States questions, core limits., smt, active cores
// 
// 1     11/02/07 1:59p Markw
//
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:	CPUSetup.c
//
// Description:	CPU Setup Rountines
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Setup.h>
#include <AmiCspLibInc.h>
#include <AmiDxeLib.h>
#include <Protocol/AmiCpuInfo.h>
#include <Protocol/AmiCpuInfo2.h>
#include <AmiHobs.h>
#include <CpuHobs.h>
#include <Cpu.h>
#include <Protocol/PchExtendedReset.h>

typedef struct {
    STRING_REF Ver;
    STRING_REF Id;
    STRING_REF Microcode;
    STRING_REF MaxSpeed;
    STRING_REF MinSpeed;
    STRING_REF L1DataSize;
    STRING_REF L1CodeSize;
    STRING_REF L2Size;
    STRING_REF L3Size;
    STRING_REF NumCores;
    STRING_REF HyperThreading;
    STRING_REF Vtx;
    STRING_REF Smx;
} SKT_STR_TOK;

static SKT_STR_TOK gSktStrTok[] = {
{   STRING_TOKEN(STR_CPU_SKT0_VERSION_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_CPUID_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_MICROCODE_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_MAXSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_MINSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_L1_DATA_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_L1_CODE_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_L2_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_L3_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_NUMCORE_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_HT_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_VTX_VALUE),
    STRING_TOKEN(STR_CPU_SKT0_SMX_VALUE)
},
{   STRING_TOKEN(STR_CPU_SKT1_VERSION_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_CPUID_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_MICROCODE_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_MAXSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_MINSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_L1_DATA_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_L1_CODE_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_L2_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_L3_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_NUMCORE_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_HT_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_VTX_VALUE),
    STRING_TOKEN(STR_CPU_SKT1_SMX_VALUE)
},
{   STRING_TOKEN(STR_CPU_SKT2_VERSION_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_CPUID_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_MICROCODE_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_MAXSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_MINSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_L1_DATA_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_L1_CODE_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_L2_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_L3_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_NUMCORE_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_HT_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_VTX_VALUE),
    STRING_TOKEN(STR_CPU_SKT2_SMX_VALUE)
},
{   STRING_TOKEN(STR_CPU_SKT3_VERSION_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_CPUID_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_MICROCODE_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_MAXSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_MINSPEED_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_L1_DATA_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_L1_CODE_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_L2_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_L3_CACHE_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_NUMCORE_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_HT_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_VTX_VALUE),
    STRING_TOKEN(STR_CPU_SKT3_SMX_VALUE)
}};

static UINT32 gL1DataCacheSize;
static UINT32 gL1CodeCacheSize;
static UINT32 gL2CacheSize;
static UINT32 gL3CacheSize;

static BOOLEAN gL1Shared;
static BOOLEAN gL2Shared;
static BOOLEAN gL3Shared;
static BOOLEAN gL2LLC;

static EFI_GUID gAmiSetupGuid = SETUP_GUID;
static EFI_GUID gAmiCpuInfoProtocolGuid    = AMI_CPU_INFO_PROTOCOL_GUID;
static EFI_GUID gAmiCpuInfo2ProtocolGuid   = AMI_CPU_INFO_2_PROTOCOL_GUID;
static EFI_GUID gHobListGuid               = HOB_LIST_GUID;
static EFI_GUID gAmiInternalFactoryTdcTdpHobGuid = AMI_INTERNAL_FACTORY_TDC_TDP_HOB_GUID;
static EFI_GUID gSetupNvramUpdateGuid      = { \
    0xd84beff0, 0x159a, 0x4b60, 0x9a, 0xb9, 0xac, 0x5c, 0x47, 0x4b, 0xd3, 0xb1
};
static EFI_GUID gCpuInfoHobGuid = AMI_CPUINFO_HOB_GUID;
static EFI_HII_HANDLE  gHiiHandle;
static CPUINFO_HOB *gCpuInfoHob;
static AMI_INTERNAL_FACTORY_TDC_TDP_HOB *gTdcTdpHob; //May not be present.

static AMI_CPU_INFO_PROTOCOL   *gCpuInfoProt;
static AMI_CPU_INFO_2_PROTOCOL *gCpuInfo2Prot;

static VOID InitCpuInfo();

VOID    CPULib_CpuID(UINT32 CpuIDIndex, UINT32 * RegEAX, UINT32 * RegEBX,
                UINT32 * RegECX, UINT32 * RegEDX);

UINT16 mValleyViewFSBTable[4] = {
  834,          // 83.3MHz
  1000,         // 100MHz
  1334,         // 133MHz
  1167          // 116.7MHz
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetCpuSignature
//
// Description: Get the cpu signature.
//
// Input:       VOID
//
// Output:      Cpu Signature
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetCpuSignature()
{
    UINT32 CpuSignature, CpuIdEBX, CpuIdECX, CpuIdEDX;
    CPULib_CpuID(1, &CpuSignature, &CpuIdEBX, &CpuIdECX, &CpuIdEDX);
    return CpuSignature;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetCpuFsbFromMsr
//
// Description: Get the cpu Fsb From Msr 0xCD.
//
// Input:       VOID
//
// Output:      FSB Value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetCpuFsbFromMsr()
{
    UINT64	Temp;
    Temp = (AsmReadMsr64 (0xCD)) & 0x03;
    return mValleyViewFSBTable[(UINT32)(Temp)];
}
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GetBoardSocketNumber
//
// Description: Get socket number from Apic ID.
//
// Input:
//  IN UINT32 ApicId
//
// Output:  UINT32 - Physical Socket Id
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

static UINT32 GetBoardSocketNumber(IN UINT32 ApicId)
{
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;
    UINT8  MaxThreadsPackage;

    CPULib_CpuID(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);

    MaxThreadsPackage = (UINT8)(RegEbx >> 16);
    
    return ApicId / MaxThreadsPackage;
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
//  OUT CHAR8   *CpuMaxSpeedString -Pointer to string with format "9999 MHz"; 9999 can be any 4 digit number.
//
// Output:  BOOLEAN - TRUE if frequency found.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN GetMaxSpeedFromBrandString(IN CHAR8 *CpuBrandString, OUT CHAR8 *CpuMaxSpeedString)
{
    UINT32  i;
    UINT8   CharCount;
    BOOLEAN TransToMHz = FALSE;
    BOOLEAN FreqStringFound;

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

            CpuMaxSpeedString[i] = *CpuBrandString;
            ++CpuBrandString;
        }
        if (TransToMHz) CpuMaxSpeedString[3] = '0';
    } else FreqStringFound = FALSE;

    return FreqStringFound;
}

VOID FillCacheData(IN AMI_CPU_INFO_2_CACHE_DESCR *CacheDesc, IN UINT32 NumCacheEntries)
{
    AMI_CPU_INFO_2_CACHE_DESCR *ptr = CacheDesc;
    while (NumCacheEntries--) {
        switch(ptr->Level) {
        case 1:
            switch(ptr->Type) {
            case AMI_CPU_INFO_2_CACHE_TYPE_DATA: gL1DataCacheSize = ptr->Size; break;
            case AMI_CPU_INFO_2_CACHE_TYPE_CODE: gL1CodeCacheSize = ptr->Size; break;
            default: ASSERT(FALSE); // Not valid cache for setup display.
            }
            gL1Shared = ptr->Shared != AMI_CPU_INFO_2_CACHE_SHARED_CORE;
            break;
        case 2:
            switch(ptr->Type) {
            case AMI_CPU_INFO_2_CACHE_TYPE_UNIFIED: gL2CacheSize = ptr->Size; break;
            default: ASSERT(FALSE); //Not valid cache for setup display.
            }
            gL2Shared = ptr->Shared != AMI_CPU_INFO_2_CACHE_SHARED_CORE;
            break;
        case 3:
            switch(ptr->Type) {
            case AMI_CPU_INFO_2_CACHE_TYPE_UNIFIED: gL3CacheSize = ptr->Size; break;
            default: ASSERT(FALSE); //Not valid cache for setup display.
            }
            gL3Shared = ptr->Shared != AMI_CPU_INFO_2_CACHE_SHARED_CORE;
            break;
        default:
            ASSERT(FALSE);  //Invalid entry;
        }

        ptr = (AMI_CPU_INFO_2_CACHE_DESCR*)((UINT8*)ptr + ptr->LengthDesc);
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   UpdateSetupTdcTdp
//
// Description: Update setup for Tdc Tdp.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID UpdateSetupTdcTdp()
{
   if (gTdcTdpHob->IsSandyBridge) {
        UINT8 Fraction = gTdcTdpHob->TdpLimitTime >> 5;
        UINT8 Exponent = gTdcTdpHob->TdpLimitTime & 0x1f;

        //Time = 1.Fraction + 2 ^ Exponent, Fraction is 2 digit binary.
        UINT32 Time = (4 + Fraction) << Exponent >> 2;
        UINT32 Time_ms = (Time * 1000) >> gTdcTdpHob->TimeConv;

        ASSERT(Fraction <= 3);  //If this asserts, formula has changed.

    	InitString(
    		gHiiHandle,
    		STRING_TOKEN(STR_CPU_FACTY_LONG_DUR_PWR_VALUE),
    		L"%d Watts",
    		gTdcTdpHob->Tdp >> gTdcTdpHob->PowerConv
    	);

       InitString(
    		gHiiHandle,
            STRING_TOKEN(STR_CPU_FACTY_LONG_DUR_TIME_VALUE),
            L"%d ms",
            Time_ms
    	);

#if SANDY_BRIDGE_VID_SUPPORT
       InitString(
    		gHiiHandle,
            STRING_TOKEN(STR_CPU_FACTY_VID_VALUE),
            L"%d (1/256 V)",
            gTdcTdpHob->Vid
    	);
#endif
    } else {
    	InitString(
    		gHiiHandle,
    		STRING_TOKEN(STR_CPU_FACTORY_TDP_VALUE),
    		L"%d Watts",
    		gTdcTdpHob->Tdp >> gTdcTdpHob->PowerConv
    	);

        InitString(
            gHiiHandle,
            STRING_TOKEN(STR_CPU_FACTORY_TDC_VALUE),
    		L"%d Amps",
            gTdcTdpHob->Tdc >> gTdcTdpHob->CurConv
    	);
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   LocateCpuInfoProt
//
// Description: Callback on AMI CPU INFO protocol. Then call to init strings.
//
// Input:
//  IN EFI_EVENT Event - Not used
//  IN VOID *Context - Not Used
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID LocateCpuInfoProt(IN EFI_EVENT Event, IN VOID *Context)
{
    pBS->LocateProtocol (&gAmiCpuInfoProtocolGuid, NULL, &gCpuInfoProt);
    InitCpuInfo();
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   LocateCpuInfoProt
//
// Description: Callback on AMI CPU INFO 2 protocol. Then call to init strings.
//
// Input:
//  IN EFI_EVENT Event - Not used
//  IN VOID *Context - Not Used
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID LocateCpuInfo2Prot(IN EFI_EVENT Event, IN VOID *Context)
{
    pBS->LocateProtocol (&gAmiCpuInfo2ProtocolGuid, NULL, &gCpuInfo2Prot);
    InitCpuInfo();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	InitCpuInfo
//
// Description:	Initialize CPU strings.
//
// Input: VOID
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static VOID InitCpuInfo()
{
    UINTN                   CpuNumber = 0;
    AMI_CPU_INFO            *GetCpuInfo = NULL;
//    BOOLEAN                 IsSandyBridge;
    CHAR8	            CpuMaxSpeedString[] = "9999 MHz";
    CHAR8                   *CpuBrandString;
    OUT AMI_CPU_INFO_2_CACHE_DESCR   *CacheDesc;
    OUT UINT32              NumCacheEntries;
    UINT32                  CpuSigNoVer = GetCpuSignature() & 0xfffffff0;
    UINT32                  Bclk;
    UINT32                  i;
    BOOLEAN                 FreqStringFound = FALSE;
    UINT32                  NumSocketsPop = 1; //NumberOfCpuSocketsPopulated();
    EFI_STATUS              Status;

    ASSERT(NumSocketsPop <= NUMBER_CPU_SOCKETS);

    if (gCpuInfoProt == NULL) return;
    
    if (gCpuInfo2Prot == NULL) return;
    
    Bclk = GetCpuFsbFromMsr();	    

    gCpuInfoProt->GetCpuInfo(gCpuInfoProt, CpuNumber, &GetCpuInfo);

#if CPU_SETUP_SET_MAX_RATIO
{
    UINT32  RegEax, RegEbx, RegEcx, RegEdx;
    BOOLEAN HasTurbo;

    CPULib_CpuID(6, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    HasTurbo = !!(RegEax & 2);

    if (HasTurbo) {
        InitString(
            gHiiHandle,
            STRING_TOKEN(STR_CPU_MAX_RATIO_HELP),
    		L"Non Turbo Range: %d - %d. Turbo ratio: %d. If out of range ratio, maximum or minimum ratio is used. This sets the maximum ratio.",
            gCpuInfoHob->Cpuinfo[0].BusRatioMin,
            gCpuInfoHob->Cpuinfo[0].BusRatioMax,
            gCpuInfoHob->Cpuinfo[0].BusRatioMax + 1
    	);
    } else {
        InitString(
            gHiiHandle,
            STRING_TOKEN(STR_CPU_MAX_RATIO_HELP),
    		L"Range: %d - %d. If out of range ratio, maximum or minimum ratio is used. This sets the maximum ratio.",
            gCpuInfoHob->Cpuinfo[0].BusRatioMin,
            gCpuInfoHob->Cpuinfo[0].BusRatioMax
    	);
    }
}
#endif
#if CPU_SETUP_SET_BOOT_RATIO
    InitString(
        gHiiHandle,
        STRING_TOKEN(STR_CPU_SET_BOOT_RATIO_HELP),
		L"Range: %d - %d.  If out of range ratio, maximum ratio is used. This sets the boot ratio. Non-ACPI OSes will use this ratio.",
        gCpuInfoHob->Cpuinfo[0].BusRatioMin,
        gCpuInfoHob->Cpuinfo[0].BusRatioMax
	);
#endif

    if (GetCpuInfo->X64Supported == 0) {
    	InitString(
    		gHiiHandle,
    		STRING_TOKEN(STR_CPU_EMT64_VALUE),
    		L"%a",
    		"Not Supported"
    	);
    }

	InitString(
		gHiiHandle,
		STRING_TOKEN(STR_PROCESSOR_SPEED_VALUE),
		L"%d MHz",
		GetCpuInfo->IntendedFreq
	);

    if (gTdcTdpHob) UpdateSetupTdcTdp();

    for (i = 0; i < NumSocketsPop; ++i) {
        UINT32 PhysSocket;
        UINT32 ApicId;
        UINT32 NumCores;
        UINT32 NumThreads;
        UINT32 NumDisabledCores;
        UINT32 NumDisabledThreads;
        UINT32 UniqueLxCache;
        BOOLEAN IsHtSupported;

        Status = gCpuInfo2Prot->GetApicInfo(gCpuInfo2Prot, i, 0, 0, &ApicId, NULL);
        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) ApicId = 0;
        PhysSocket = GetBoardSocketNumber(ApicId);

        ASSERT(PhysSocket < NUMBER_CPU_SOCKETS);

        Status = gCpuInfo2Prot->GetNumCoresThreads(gCpuInfo2Prot, i,
                &NumCores, &NumThreads, &NumDisabledCores, &NumDisabledThreads
        );
        ASSERT_EFI_ERROR(Status);

        IsHtSupported = (NumThreads + NumDisabledThreads) > (NumCores + NumDisabledCores);

        InitString(
    		gHiiHandle,
    		gSktStrTok[PhysSocket].Ver,
    		L"%a",
    		GetCpuInfo->BrandString
    	);
    
    	InitString(
    		gHiiHandle,
    		gSktStrTok[PhysSocket].Id,
    		L"%x",
    		GetCpuInfo->Version
    	);
    
    	if(GetCpuInfo->MicroCodeVers != 0) {
    		InitString(
    			gHiiHandle,
    			gSktStrTok[PhysSocket].Microcode,
    			L"%x",
    			GetCpuInfo->MicroCodeVers
    		);
    	}

        CpuBrandString = GetCpuInfo->BrandString;
        FreqStringFound = GetMaxSpeedFromBrandString(CpuBrandString, CpuMaxSpeedString);

        if (FreqStringFound) {
        InitString(
            gHiiHandle,
            gSktStrTok[PhysSocket].MaxSpeed,
            L"%a",
            CpuMaxSpeedString
            );
        } else {
            InitString(
                gHiiHandle,
                gSktStrTok[PhysSocket].MaxSpeed,
                L"%d MHz",
                gCpuInfoHob->Cpuinfo[CpuNumber].BusRatioMax * Bclk/10
            );
        }

        InitString(
            gHiiHandle,
            gSktStrTok[PhysSocket].MinSpeed,
            L"%d MHz",
            gCpuInfoHob->Cpuinfo[CpuNumber].BusRatioMin * Bclk/10
        );

        gL1DataCacheSize = 0;
        gL1CodeCacheSize = 0;
        gL2CacheSize = 0;
        gL3CacheSize = 0;
        gL2LLC = 0;

        Status = gCpuInfo2Prot->GetCoreCacheDesc(gCpuInfo2Prot, i, 0, &CacheDesc, &NumCacheEntries);
        ASSERT_EFI_ERROR(Status);

        if (!EFI_ERROR(Status)) {
            FillCacheData(CacheDesc, NumCacheEntries);
        }
        
        if (gL3CacheSize == 0) {
            UINT32 RegEcx, NumCore;
            gL2LLC = TRUE; //Is L2 the Last Level Cache?
            RegEcx = 1;		
            CPULib_CpuID(0xb, NULL, &NumCore, &RegEcx, NULL);
            UniqueLxCache = NumCore;
            if (NumCore > 1) {
                UniqueLxCache = NumCore>>1;
            }
#if ValleyView_Industry == 1
			{
                UINT8  MsrPlatform;
                MsrPlatform = (UINT8) (RShiftU64 ((AsmReadMsr64 (MSR_IA32_PLATFORM_ID) & B_PLATFORM_ID_BITS_MASK),  \
                		N_PLATFORM_ID_BITS));
					if ((NumCore == 2) && (MsrPlatform == 0)){
						gL2CacheSize = gL2CacheSize*2;
						}
			}

#endif
        }
        
        if (gL1DataCacheSize) {
            if (gL1Shared) {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L1DataSize,
                    L"%d kB",
                    gL1DataCacheSize
                );
            } else {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L1DataSize,
                    L"%d kB x %i",
                    gL1DataCacheSize,
                    NumCores
                );
            }
        }

        if (gL1CodeCacheSize) {
            if (gL1Shared) {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L1CodeSize,
                    L"%d kB",
                    gL1CodeCacheSize
                );
            } else {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L1CodeSize,
                    L"%d kB x %i",
                    gL1CodeCacheSize,
                    NumCores
                );
            }
        }

        if (gL2CacheSize) {
            if ((gL2Shared)&&(gL2LLC)) {
                    InitString(
                        gHiiHandle,
                        gSktStrTok[PhysSocket].L2Size,
                        L"%d kB x %i",
                        gL2CacheSize,
                        UniqueLxCache
                    );
            } else if (gL2Shared) {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L2Size,
                    L"%d kB",
                    gL2CacheSize
                );
            } else {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L2Size,
                    L"%d kB x %i",
                    gL2CacheSize,
                    NumCores
                );
            }
        }

        if (gL3CacheSize) {
            if (gL3Shared) {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L3Size,
                    L"%d kB",
                    gL3CacheSize
                );
            } else {
                InitString(
                    gHiiHandle,
                    gSktStrTok[PhysSocket].L3Size,
                    L"%d kB x %i",
                    gL3CacheSize,
                    NumCores
                );
            }
        }

        InitString(
            gHiiHandle,
            gSktStrTok[PhysSocket].NumCores,
            L"%i",
            NumCores + NumDisabledCores
        );

        if(IsHtSupported) {
            InitString(
                gHiiHandle,
                gSktStrTok[PhysSocket].HyperThreading,
                L"%a",
                "Supported"
            );
        }

        if (GetCpuInfo->Features & (UINT64)1 << (32 + 5)) {   //CPUID eax=1 ecx[5]
            InitString(
                gHiiHandle,
                gSktStrTok[PhysSocket].Vtx,
                L"%a",
                "Supported"
            );
        }

        if (GetCpuInfo->Features & (UINT64)1 << (32 + 6)) {   //CPUID eax=1 ecx[6]
            InitString(
                gHiiHandle,
                gSktStrTok[PhysSocket].Smx,
                L"%a",
                "Supported"
            );
        }

        CpuNumber += GetCpuInfo->NumCores * (GetCpuInfo->NumHts == 0 ? 1 : 2);
    	gCpuInfoProt->GetCpuInfo(gCpuInfoProt, CpuNumber, &GetCpuInfo);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	InitCpuInfo
//
// Description:	Initialize CPU strings.
//
// Input:
//      IN EFI_HII_HANDLE   HiiHandle
//      IN UINT16           Class
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitCPUStrings(EFI_HII_HANDLE HiiHandle, UINT16 Class)
{
    EFI_EVENT   AmiCpuInfoEvt;
    EFI_STATUS  Status;
    VOID        *CpuNotifyReg;
    VOID        *FirstHob;

    if (Class != ADVANCED_FORM_SET_CLASS) 
        return;

    gHiiHandle = HiiHandle;

    FirstHob = GetEfiConfigurationTable(pST, &gHobListGuid);
    ASSERT(FirstHob);
    if (FirstHob == NULL) return;

    gCpuInfoHob = (CPUINFO_HOB*)FirstHob;
    //Find CpuInfo Info Hob.
    while (!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &gCpuInfoHob))) {
        if (guidcmp(&gCpuInfoHob->EfiHobGuidType.Name, &gCpuInfoHobGuid) == 0) break;
    }
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return;

    gTdcTdpHob = (AMI_INTERNAL_FACTORY_TDC_TDP_HOB*)FirstHob;
    while (!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &gTdcTdpHob))) {
        if (guidcmp(&gTdcTdpHob->EfiHobGuidType.Name, &gAmiInternalFactoryTdcTdpHobGuid) == 0) break;
    }
    if (EFI_ERROR(Status)) gTdcTdpHob = NULL;

    Status = pBS->LocateProtocol (&gAmiCpuInfoProtocolGuid, NULL, &gCpuInfoProt);
    if (EFI_ERROR(Status))
    	gCpuInfoProt = NULL;

    Status = pBS->LocateProtocol (&gAmiCpuInfo2ProtocolGuid, NULL, &gCpuInfo2Prot);
    if (EFI_ERROR(Status)) 
    	gCpuInfo2Prot = NULL;

    InitCpuInfo();

    if (gCpuInfoProt == NULL) {
        Status = RegisterProtocolCallback(
            &gAmiCpuInfoProtocolGuid,
            LocateCpuInfoProt,
            NULL,
            &AmiCpuInfoEvt,
            &CpuNotifyReg
        );
        ASSERT_EFI_ERROR(Status);
    }
    if (gCpuInfoProt == NULL) {
        Status = RegisterProtocolCallback(
            &gAmiCpuInfo2ProtocolGuid,
            LocateCpuInfo2Prot,
            NULL,
            &AmiCpuInfoEvt,
            &CpuNotifyReg
        );
        ASSERT_EFI_ERROR(Status);
    }
}

EFI_STATUS
VtSettingCallback(EFI_HII_HANDLE HiiHandle, UINT16 Class, UINT16 SubClass, UINT16 Key)
{
    EFI_STATUS Status;
    UINT8       Control_EVT;
    UINT64	Ia32FeatCtrl;
    CALLBACK_PARAMETERS *Callback;

    Callback = GetCallbackParameters();
//    TRACE((-1,"\n====ForceSetupModeCallback==== Key = %d, Callback %x\n",  Key, Callback));
//    TRACE((-1,"MSR_IA32_FEATURE_CONTROL = %x\n",  AsmReadMsr64 (EFI_MSR_IA32_FEATURE_CONTROL)));

    Status = EFI_SUCCESS;
    if(!Callback) {
        return EFI_SUCCESS;
    }
//    TRACE((-1,"Callback->Action=%x\n",  Callback->Action));

    if (Callback->Action == EFI_BROWSER_ACTION_FORM_OPEN ||\
        Callback->Action == EFI_BROWSER_ACTION_CHANGING)
       return EFI_SUCCESS;

    if (Callback->Action == EFI_BROWSER_ACTION_FORM_CLOSE) {
        Ia32FeatCtrl = AsmReadMsr64 (MSR_IA32_FEATURE_CONTROL);
        if ((Ia32FeatCtrl & B_EFI_MSR_IA32_FEATURE_CONTROL_LOCK) != 0) {
            //set generate the cold reset and let the rc set the VT bit
            Control_EVT = ((UINT8)Ia32FeatCtrl & B_EFI_MSR_IA32_FEATURE_CONTROL_EVT) >> 2;
//          TRACE((-1,"Callback->Value->u8:%x, Control_EVT:%x\n", Callback->Value->u8, Control_EVT));
            if (Control_EVT != Callback->Value->u8) {
                EFI_PCH_EXTENDED_RESET_PROTOCOL   *gExtendedReset;
                PCH_EXTENDED_RESET_TYPES		  ResetType;
                ResetType.GlobalReset = 1;
                pBS->LocateProtocol(&gEfiPchExtendedResetProtocolGuid, NULL, &gExtendedReset);
                gExtendedReset->Reset(gExtendedReset, ResetType);
            }
        }
    }
    return EFI_SUCCESS;
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
