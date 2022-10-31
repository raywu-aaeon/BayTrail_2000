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

//**********************************************************************
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
// Name:        CpuCspLib.h
//
// Description: Header file for Cpu Csp Lib.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __CPULIB_H__
#define __CPULIB_H__

#include <Efi.h>
#include <Pei.h>
#include "AmiHobs.h"
#include "Smm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    EFI_STATUS_CODE_DATA    DataHeader;
    UINT32                  Bist;
} AMI_STATUS_CODE_CPU_BIST_DATA;

#pragma pack(push, 1)

typedef struct {
    UINT32 HeaderVersion;
    UINT32 UpdateRevision;
    UINT32 Date;
    UINT32 CpuSignature;
    UINT32 Checksum;
    UINT32 LoaderRevison;
    UINT32 Flags:8;
    UINT32 RV3:24;
    UINT32 DataSize;
    UINT32 TotalSize;
    UINT32 RV4[3];
} MICROCODE_HEADER;

typedef struct {
    UINT32  CpuSignature;
    UINT32  Flags;
    UINT32  Checksum;
} PROC_SIG;

typedef struct {
    UINT32      Count;
    UINT32      Checksum;
    UINT8       Rsv[12];
    PROC_SIG    ProcSig[1];
} MICROCODE_EXT_PROC_SIG_TABLE;

#pragma pack(pop)

// {CD541D77-6699-4b36-A31E-1AA4C5D5B946}
#define AMI_STATUS_CODE_CPU_BIST_DATA_GUID \
    {0xcd541d77, 0x6699, 0x4b36, 0xa3, 0x1e, 0x1a, 0xa4, 0xc5, 0xd5, 0xb9, 0x46}

UINT64  ReadMsr (UINT32 Msr);
VOID    WriteMsr(UINT32 Msr, UINT64 Value);
VOID    ReadWriteMsr(UINT32 Msr, UINT64 Value, UINT64 Mask);
VOID    CPULib_CpuID(UINT32 CpuIDIndex, UINT32 * RegEAX, UINT32 * RegEBX, 
                UINT32 * RegECX, UINT32 * RegEDX);
UINT32  GetCpuSignature();
UINT32  GetCpuFsbFromMsr();
UINT32  GetCpuFamily(UINT32 CpuSignature);
UINT32  GetCpuModel(UINT32 CpuSignature);
UINT32  GetCpuPlatformId();
UINT32  GetSmrrBaseMsr();
UINT8   NumSupportedThreadsPerCore();
UINT8   NumSupportedCpuCores();
UINT8   NumCpuCores();
UINT8   NumLogicalCpus();
BOOLEAN IsHtEnabled();
BOOLEAN IsHt0();
BOOLEAN IsCore0();
BOOLEAN isXDSupported(CPU_FEATURES *Features);
BOOLEAN isTurboModeSupported();
BOOLEAN isXETdcTdpLimitSupported();
BOOLEAN isXECoreRatioLimitSupported();
BOOLEAN isLimitCpuidSupported();
BOOLEAN IsMachineCheckSupported(CPU_FEATURES *Features);
BOOLEAN IsEnergyPerfBiasSupported();
BOOLEAN IsCxInterruptFilteringSupported();
BOOLEAN IsVmxSupported(CPU_FEATURES *Features);
BOOLEAN IsSmxSupported(CPU_FEATURES *Features);
BOOLEAN IsSmrrSupported(CPU_FEATURES *Features);
BOOLEAN IsX64Supported(CPU_FEATURES *Features);

BOOLEAN CPULib_IsVmxEnabled();
BOOLEAN CPULib_IsSmxEnabled();
BOOLEAN CPULib_IsSmrrEnabled();
BOOLEAN CPULib_IsLocalApicEnabled();
BOOLEAN CPULib_IsLocalX2ApicEnabled();
BOOLEAN CPULib_IsFeatureControlLocked();
UINT32  NumberOfCpuSocketsPopulated();
VOID    DisableCacheInCR0();
VOID    EnableCacheInCR0();
VOID    CPULib_DisableInterrupt();
VOID    CPULib_EnableInterrupt();
BOOLEAN CPULib_GetInterruptState();
VOID*   CPULIB_GetPageTable();
VOID    CPULib_Pause();
UINT16  GetCsSegment();
UINT64  ReadRtdsc();
VOID    WaitForever();
VOID    HltCpu();
VOID    WaitForSemaphore(volatile VOID*);
VOID    WaitUntilZero8(volatile VOID*);
VOID    WaitUntilZero32(volatile VOID*);
UINT16  GetCsSegment();

VOID EnableMachineCheck();

VOID CPULib_LockByteInc(UINT8* ptr);
VOID CPULib_LockByteDec(UINT8* ptr);
VOID CPULib_LoadGdt(VOID *ptr);
VOID CPULib_SaveGdt(VOID *ptr);
VOID CPULib_LoadIdt(VOID *ptr);
VOID CPULib_SaveIdt(VOID *ptr);

UINT32 MemRead32(UINT32 *Address);
VOID MemReadWrite32(UINT32 *Address, UINT32 Value, UINT32 Mask);

#ifdef __cplusplus
}
#endif
#endif


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

