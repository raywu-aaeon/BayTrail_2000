//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
//
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        CpuCspLib.c
//
// Description:
//  Contains the CPU library related functions. These functions can be linked
//   with various components in the project.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

//#include <Efi.h>
#include <Token.h>
#include <AmiDxeLib.h>
#include <Smm.h>
#include <Pcie.h>

#include "Cpu.h"
#include "Library/CpuCspLib.h"

//AptioV
//This is a workaournd.
//This Aptio4 code is using x64_BUILD macro, which is wrong.
//The macro is not defined by the Core build flags in Aptio4.
//EFIx64 should be used instead.
#ifdef EFIx64
#ifndef x64_BUILD
#define x64_BUILD 1
#endif
#endif

#ifndef FV_MICROCODE_BASE
#define FV_MICROCODE_BASE FV_MAIN_BASE
#endif

#define MAX_NR_BUS ((PCIEX_LENGTH/0x100000)-1)

static EFI_GUID gMicrocodeFfsGuid = 
    {0x17088572, 0x377F, 0x44ef, 0x8F, 0x4E, 0xB0, 0x9F, 0xFF, 0x46, 0xA0, 0x70};

UINT16 mValleyViewFSBTable[4] = {
  834,          // 83.3MHz
  1000,         // 100MHz
  1334,         // 133MHz
  1167          // 116.7MHz
};
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:  GetCacheSharedThreads
//
// Description: Return number of shared threads for a Information.
//
// Input:  IN UINT8 Level - Cache level
//
// Output: UINT8 - Number of shared threads.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

#define LIMIT_CPUID                 (1 << 22)

UINT8 GetCacheSharedThreads(IN UINT8 Level)
{
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;
    UINT32 i = 0;	
	UINT64 Ia32MiscEnable;
	BOOLEAN LimitCpuidEnabled = FALSE;

	Ia32MiscEnable = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
	if (Ia32MiscEnable & LIMIT_CPUID) {
	  Ia32MiscEnable &= ~LIMIT_CPUID;
	  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable);
	  LimitCpuidEnabled = TRUE;
	}

    for(;;) {
        RegEcx = i;
        CPULib_CpuID(4, &RegEax, &RegEbx, &RegEcx, &RegEdx);
        if ((RegEax & 0x1f) == 0) break;
        if (((RegEax >> 5) & 7) == Level) {
        	if ( LimitCpuidEnabled )
				AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable | LIMIT_CPUID);
			return 1 + ((RegEax >> 14)& 0xfff);
        }
        ++i;
    }
   	if ( LimitCpuidEnabled )
		AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable | LIMIT_CPUID);
    return 0;
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
    Temp = (AsmReadMsr64 (MSR_PSB_CLOCK_STATUS)) & FUSE_BSEL_MASK;
    return mValleyViewFSBTable[(UINT32)(Temp)];
}
		
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

typedef struct {
    UINT32 Stepping:4;
    UINT32 Model:4;
    UINT32 Family:4;
    UINT32 Type:2;
    UINT32 RV:2;
    UINT32 ExtModel:4;
    UINT32 ExtFamily:8;
} CPU_SIGNATURE;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetCpuFamily
//
// Description: Get the cpu family from signature.
//
// Input:       UINT32 CpuSignature
//
// Output:      UINT32 - Cpu Family
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetCpuFamily(UINT32 CpuSignature)
{
    CPU_SIGNATURE *Signature = (CPU_SIGNATURE*)&CpuSignature;
    return Signature->ExtFamily + Signature->Family;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetCpuModel
//
// Description: Get the cpu model from signature.
//
// Input:       UINT32 CpuSignature
//
// Output:      UINT32 - Cpu Model
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetCpuModel(UINT32 CpuSignature)
{
    CPU_SIGNATURE *Signature = (CPU_SIGNATURE*)&CpuSignature;
    return (Signature->ExtModel << 4) + Signature->Model;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetCpuPlatformId
//
// Description: Get the cpu platform Id.
//
// Input:       VOID
//
// Output:      Cpu Platform Id
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32  GetCpuPlatformId(VOID)
{
    return (UINT32)Shr64(ReadMsr(0x17), 50) & 7;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	GetSmrrBaseMsr
//
// Description:	Return the Smrr Base Msr
//
// Input:		VOID
//
// Output:		SMRR Base
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32  GetSmrrBaseMsr()
{
    return 0x1f2;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadWriteMsr
//
// Description: This function writes the CPU MSR with the value provided.
//
// Input:
//  Msr     32bit MSR index
//  Value   64bit OR Value
//  Mask    64Bit AND Mask Value
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ReadWriteMsr(UINT32 Msr, UINT64 Value, UINT64 Mask)
{
    UINT64 OrigData = ReadMsr(Msr);
    UINT64 WriteData = (OrigData & Mask) | Value;
    WriteMsr(Msr, WriteData);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	NumSupportedThreadsPerCore
//
// Description:	Get number of supported threads per core.
//
// Input:   VOID
//
// Output:  UINT8 Number of Threads per core.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 NumSupportedThreadsPerCore()
{
	UINT32	RegEax, RegEbx, RegEcx, RegEdx;

    RegEcx = 0;		
    CPULib_CpuID(0xb, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    return (UINT8)RegEbx;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	NumSupportedCpuCores
//
// Description:	Get number of supported Cpu Cores per package.
//
// Input:   VOID
//
// Output:  UINT8 Number of supported Cpu Cores per package.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 NumSupportedCpuCores()
{
	UINT32	RegEax, RegEbx, RegEcx, RegEdx;
    UINT8  TotLogicalCpus;
    UINT8  LogicalCpusPerCore;

    RegEcx = 1;		
    CPULib_CpuID(0xb, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    TotLogicalCpus  = (UINT8)RegEbx;

    RegEcx = 0;		
    CPULib_CpuID(0xb, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    LogicalCpusPerCore  = (UINT8)RegEbx;

    return TotLogicalCpus / LogicalCpusPerCore;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	NumLogicalCpus
//
// Description:	Get number of logical CPUs.
//
// Input:   VOID
//
// Output:  UINT8 Number of logical CPUs.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 NumLogicalCpus()
{
    UINT64  MsrData = ReadMsr(MSR_CORE_THREAD_COUNT);
    return (UINT8)MsrData;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsHtEnabled
//
// Description: Determine if CPU is HT.
//
// Input:   VOID
//
// Output:  True if HT CPU.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsHtEnabled()
{
#if 0
	UINT8   NumLogCPUs, NumCpuCores;
    UINT64  MsrData = ReadMsr(MSR_CORE_THREAD_COUNT);
    UINT32  CpuSignature = GetCpuSignature() & 0xfffffff0;

    NumCpuCores = (UINT8)((UINT32)MsrData >> 16);

    // Westmere work around
    if (CpuSignature == WESTMERE) NumCpuCores &= 0xf;

    NumLogCPUs = (UINT8)MsrData;

    if ((NumLogCPUs / NumCpuCores) <= 1) return FALSE;
    return TRUE;
#endif
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NumCpuCores
//
// Description: Returns number of CPU Cores
//
// Input: VOID
//
// Output:  Number of CPU Cores.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 NumCpuCores()
{
    UINT32	RegEax, RegEbx, RegEcx, RegEdx;
    RegEcx = 1;		
    CPULib_CpuID(0xb, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    return (UINT8)RegEbx;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsHt0
//
// Description: Determine if CPU thread is logical CPU 0 executing.
//
// Input: VOID   
//
// Output:  True if logical CPU 0.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsHt0()
{
    UINT32 ApicMask;
    UINT32 ApicId;
    UINT8 ThreadsPerCore = NumSupportedThreadsPerCore();
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;

    if (ThreadsPerCore < 2) return TRUE;    //Check if Ht Capable.
    ApicMask = ThreadsPerCore - 1;

    CPULib_CpuID(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    ApicId = RegEbx >> 24; 

    //Use APIC ID to determine if logical CPU.
    if ((ApicId & ApicMask) == 0) return TRUE;  //All logical CPU0 will have bit 0 clear.
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsCore0
//
// Description: Determine if CPU thread is CPU Core 0 executing.
//
// Input:   VOID
//
// Output:  True if logical CPU 0.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsCore0()
{
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;
    UINT8  MaxThreadsPackage;
    UINT32 ApicMask;
    UINT32 ApicId;
    //UINT8 ThreadsPerCore = NumSupportedThreadsPerCore();// not referenced

    ApicMask = ~(NumSupportedThreadsPerCore() - 1);

    CPULib_CpuID(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);

    MaxThreadsPackage = (UINT8)(RegEbx >> 16);
    
    ApicMask &= MaxThreadsPackage - 1;
    ApicId = RegEbx >> 24; 

    //Use APIC ID to determine if logical CPU.
    if ((ApicId & ApicMask) == 0) return TRUE;
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsX64Supported
//
// Description: Determine if CPU supports X64.
//
// Input:   CPU_FEATURES *Features
//
// Output:  True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsX64Supported(CPU_FEATURES *Features)
{
    return ((Features->ExtFeatureEdx) >> 29) & 1;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   isXDSupported
//
// Description: Determine if CPU supports Execute Disable.
//
// Input:   CPU_FEATURES *Features
//
// Output:  True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN isXDSupported(CPU_FEATURES *Features)
{
    return !!(Features->ExtFeatureEdx & (1 << 20));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	isTurboModeSupported
//
// Description:	Determine if CPU supports Turbo mode.
//
// Input:	None
//
// Output:	True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN isTurboModeSupported()
{
	BOOLEAN ret;
	UINT32  RegEax, RegEbx, RegEcx, RegEdx;
    	UINT64 MsrData = ReadMsr(MSR_IA32_MISC_ENABLE);
	CPULib_CpuID(6, &RegEax, &RegEbx, &RegEcx, &RegEdx);
	ret = ((RegEax >> 1) & 1) | (UINT32) (Shr64(MsrData,TURBO_MODE_DISABLE_BIT) & 1); 
	return ret;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	isXETdcTdpLimitSupported
//
// Description:	Determine if CPU supports Programmable TDC/TDP Limit for the Turbo mode.
//
// Input:	None
//
// Output:	True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN isXETdcTdpLimitSupported()
{

	BOOLEAN ret;
    	UINT64 MsrData = ReadMsr(MSR_PLATFORM_INFO);
	ret = (UINT32) (MsrData & (1 << XE_TDP_TDC_PROGRAMMABLE_BIT)) ? 1:0; 
	return ret;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	isXECoreRatioLimitSupported
//
// Description:	Determine if CPU supports Programmable Core Ratio Limit for the Turbo mode.
//
// Input:	None
//
// Output:	True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN isXECoreRatioLimitSupported()
{

	BOOLEAN ret;
    UINT64 MsrData = ReadMsr(MSR_PLATFORM_INFO);
	ret = (UINT32) (MsrData & (1 << XE_CORE_RATIO_PROGRAMMABLE_BIT)) ? 1:0; 
	return ret;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   isLimitCpuidSupported
//
// Description: Determine if CPU supports limiting CpuId to 3.
//
// Input:   VOID
//
// Output:  True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN isLimitCpuidSupported()
{
    UINT32 RegEbx, RegEcx, RegEdx;
    UINT32 LargestCPUIDFunc;
    CPULib_CpuID(0, &LargestCPUIDFunc, &RegEbx, &RegEcx, &RegEdx);
    return LargestCPUIDFunc > 3;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   isMachineCheckSupported
//
// Description: Determine if CPU supports machine check.
//
// Input:   CPU_FEATURES *Features
//
// Output:  True if supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsMachineCheckSupported(CPU_FEATURES *Features)
{
    //Check if MCE and MCA supported.
    return ((Features->FeatureEdx & ((1 << 7) + (1 << 14))) == ((1 << 7) + (1 << 14)));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsVmxSupported
//
// Description: Determine if CPU supports VT extensions Vmx.
//
// Input:   CPU_FEATURES *Features
//
// Output:  True if Vmx supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsVmxSupported(CPU_FEATURES *Features)
{
    return Features->FeatureEcx & (1 << 5);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsSmxSupported
//
// Description: Determine if CPU supports VT extensions Smx.
//
// Input:   CPU_FEATURES *Features
//
// Output:  True if Smx supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsSmxSupported(CPU_FEATURES *Features)
{
    return Features->FeatureEcx & (1 << 6);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPU_IsSmrrSupported
//
// Description: Determine if CPU supports Smrr.
//
// Input:   CPU_FEATURES *Features
//
// Output:  True if Smx supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsSmrrSupported(CPU_FEATURES *Features)
{
    return (BOOLEAN)Features->Flags.SmrrSupport;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsEnergyPerfBiasSupported
//
// Description: Determine if Energy Performance Bias supported.
//
// Input:   VOID
//
// Output:  BOOLEAN - True if Energy Performance Bias supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsEnergyPerfBiasSupported()
{
    UINT32 RegEax;
    UINT32 RegEbx;
    UINT32 RegEcx;
    UINT32 RegEdx;

    UINT32 CpuSignature = GetCpuSignature();
    UINT32 CpuSigNoVer = CpuSignature & 0xfffffff0;
    UINT32 CpuVer = CpuSignature & 0xf;

    BOOLEAN Support = FALSE;

    //This is also used to control setup question. No recommendation in BWG.
    //Thus, for now Sandy Bridge Energy Bias Support is coded separately from previous CPUs.
    //if (CpuSigNoVer == SANDY_BRIDGE && CpuVer >= 3)
    //    return TRUE;
 
    if (CpuSigNoVer == NEHALEM_EX && CpuVer >= 5) Support = TRUE;
    else if (CpuSigNoVer == WESTMERE) Support = TRUE;
    else if (CpuSigNoVer == WESTMERE_EX) Support = TRUE;

    if (!Support) return FALSE;
    ReadWriteMsr(MSR_MISC_PWR_MGMT, (1 << ENG_PERF_BIAS_EN_BIT), (UINT64)-1); //Energy Performance Bias Enable

    CPULib_CpuID(6, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    return !!(RegEcx & BIT3);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsCxInterruptFilteringSupported
//
// Description: Determine if C-state interrupting state supported.
//
// Input:   VOID
//
// Output:  BOOLEAN - True if C-state interrupting supported.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsCxInterruptFilteringSupported()
{
    return FALSE;

    //Not supported until this is used on a project that can test this functionality.
    //DEBUG UINT32 CpuSignature = GetCpuSignature();
    //DEBUG if (CpuSignature != 0x00020652) return FALSE;
    //DEBUG if ((INT32)Shr64(ReadMsr(MSR_IA32_BIOS_SIGN_ID), 32) < 3) return FALSE;
    //DEBUG return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPULib_IsVmxEnabled
//
// Description: Determine if Vmx is enabled.
//
// Input:   VOID
//
// Output:  True if Vmx enabled.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CPULib_IsVmxEnabled()
{
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;
    UINT8  Msr;
    CPULib_CpuID(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    if (!(RegEcx & (1 << 5))) return FALSE;

    Msr = (UINT8)ReadMsr(MSR_IA32_FEATURE_CONTROL);
    return !!(Msr & (1 << 2));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPULib_IsSmxEnabled
//
// Description: Determine if Smx is enabled.
//
// Input:   VOID
//
// Output:  True if Smx enabled.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CPULib_IsSmxEnabled()
{
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;
    UINT8  Msr;
    CPULib_CpuID(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    if (!(RegEcx & BIT6)) return FALSE;

    Msr = (UINT8)ReadMsr(MSR_IA32_FEATURE_CONTROL);
    return !!(Msr & BIT1);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPULib_IsSmrrEnabled
//
// Description: Determine if Smrr is enabled.
//
// Input:   BOOLEAN
//
// Output:  True if Smrr is enabled.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CPULib_IsSmrrEnabled()
{
    //Once SMRR is enabled, the opened SMM Area can't be read outside of SMM.
#if SMM_CACHE_SUPPORT == 0
    return FALSE;
#else
    //Some CPUs, SMRR has an enable bit. Nehalem only has a capability bit.
    UINT32 MtrrCap = (UINT32)ReadMsr(MSR_IA32_MTRR_CAP);
    return !!(MtrrCap & SMRR_SUPPORT_MASK);
#endif
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPULIB_IsLocalApicEnabled
//
// Description: Is APIC enabled, xAPIC or x2APIC
//
// Input:   VOID
//
// Output:  BOOLEAN - True if enabled
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN CPULib_IsLocalApicEnabled()
{
    UINT32 Msr = (UINT32)ReadMsr(MSR_XAPIC_BASE);
    return !!(Msr & (1 << XAPIC_GLOBAL_ENABLE_BIT));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPULIB_IsLocalX2ApicEnabled
//
// Description: Get C-state latency.
//
// Input:   VOID
//
// Output:  BOOLEAN - True if enabled
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN CPULib_IsLocalX2ApicEnabled()
{
    UINT32 Msr = (UINT32)ReadMsr(MSR_XAPIC_BASE);
    return !!(Msr & (1 << XAPIC_X2APIC_ENABLE_BIT));
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CPULib_IsFeatureControlLocked
//
// Description: Check to see if the MSR_IA32_FEATURE_CONTROL is locked.
//
// Input:   VOID
//
// Output:  BOOLEAN - True if MSR_IA32_FEATURE_CONTROL is locked.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CPULib_IsFeatureControlLocked() {
    UINT8 Ia32FeatureCntrl = (UINT8)ReadMsr(MSR_IA32_FEATURE_CONTROL);
    return Ia32FeatureCntrl & 1;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NumberOfCpuSocketsPopulated
//
// Description: Returns number of CPU sockets are populated.
//
// Input:   VOID
//
// Output:  UINT32 - Number of CPU sockets populated.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32  NumberOfCpuSocketsPopulated()
{
#if NUMBER_CPU_SOCKETS > 1
    UINT32  CpuSignature = GetCpuSignature();
    UINT32  CpuSigNoVer  = CpuSignature & 0xfffffff0;
    UINT32 NumCpuSockets = 0;
    UINT32 i;
    UINT32 BusNum;

    //Sandy Bridge Server.
    if (CpuSigNoVer == JAKETOWN) {
        UINT32 *PciAddress = (UINT32*)PCIE_CFG_ADDR(0, 0, 0, 0);
    	if (*PciAddress != 0xffffffff) ++NumCpuSockets;
#if NUMBER_CPU_SOCKETS >= 4
    	PciAddress = (UINT32*)PCIE_CFG_ADDR(0x40, 0, 0, 0);
    	if (*PciAddress != 0xffffffff) ++NumCpuSockets;
#endif
    	PciAddress = (UINT32*)PCIE_CFG_ADDR(0x80, 0, 0, 0);
    	if (*PciAddress != 0xffffffff) ++NumCpuSockets;
#if NUMBER_CPU_SOCKETS >= 4
    	PciAddress = (UINT32*)PCIE_CFG_ADDR(0xc0, 0, 0, 0);
    	if (*PciAddress != 0xffffffff) ++NumCpuSockets;
#endif
        return NumCpuSockets;
    }
    for (i = 0, BusNum = MAX_NR_BUS; i < NUMBER_CPU_SOCKETS; ++i, --BusNum) {
        UINT32 *PciAddress = (UINT32*)PCIE_CFG_ADDR(BusNum, 0, 0, 0);
        if (*PciAddress != 0xffffffff) ++NumCpuSockets;
    }
    return NumCpuSockets;
#else
    return 1;
#endif
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************


