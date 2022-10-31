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
//*************************************************************************
// Revision History
// ----------------
// $Log: $
// 
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        CpuPolicy.h
//
// Description: CPU setup data header file, define all the CPU
//              setup items and a structures in this file. 
//
// Notes:       The context of the CPU_SETUP_DATA may be able to copy from
//              CPU.SD directly 
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Setup.h>

#ifndef _AMI_CPU_SETUP_POLICY_H // To Avoid this header get compiled twice
#define _AMI_CPU_SETUP_POLICY_H

#ifdef __cplusplus
extern "C" {
#endif

#define AMI_CPU_PLATFORM_INFO_HOB_GUID \
    {0x65ed5ad8, 0xd5f0, 0x4012, 0xb9, 0x5c, 0xc5, 0x42, 0xe4, 0x7a, 0x32, 0xe6}

////////////////////////////////////////////////////////////////
//This is a private structure that is used for setup by Cpu.Sd
//This structure can change any time. No drivers should refer
// directly to this structure.
////////////////////////////////////////////////////////////////

typedef struct {
    UINT8	XDBitAvailable;
    UINT8	HTAvailable;
    UINT8   MultiCoreAvailable;
    UINT8	VTAvailable;
    UINT8	LimitCpuidAvailable;
    UINT8   MultiSocketAvailable;
    UINT8   MultiSocketPopulated;
    UINT8   LocalX2ApicAvailable;
    UINT8   L1DataPrefAvailable;
    UINT8   NumCores;
    UINT8   CpuGroup;   //Arbitrary number
    UINT8   IsSandyBridge;
    UINT8   Skt0Pop;
    UINT8   Skt1Pop;
    UINT8   Skt2Pop;
    UINT8   Skt3Pop;
    UINT8	SmxAvailable;
    UINT8   CpuMismatch;
    UINT8   XECoreRatioLimitAvailable;
    UINT8   CurrentLimitAvailable;
    UINT8   TccActivationAvailable;

    UINT8   XETdcTdpLimitAvailable;
    UINT8   CxAvailable;
    UINT8   CxIntrFilterAvailable;
    UINT8   C3Available;
    UINT8   C6Available;
    UINT8   C7Available;
    UINT8   EISTAvailable;
    UINT8   CpuEngPerfBiasAvailable;
    UINT8   TurboModeAvailable;
    UINT8   DataReuseOptAvailable;
} SETUP_CPU_FEATURES;

/*
//
// Interface structure for the Platform CPU Protocol
//
typedef struct _EFI_PLATFORM_CPU_PROTOCOL {
  BOOLEAN                               HtState;
  EFI_PLATFORM_CPU_STALL                Stall;
  EFI_PLATFORM_CPU_RETRIEVE_MICROCODE   RetrieveMicrocode;
  EFI_PLATFORM_CPU_GET_TM2_CONTROL_INFO GetTm2ControlInfo;
  BOOLEAN                               EnableL3Cache;
  BOOLEAN                               LimitCpuidMaximumValue;
  EFI_PLATFORM_CPU_GET_MAX_COUNT        GetMaxCount;
  EFI_PLATFORM_CPU_GET_CPU_INFO         GetCpuInfo;
  EFI_PLATFORM_CPU_GET_CPU_STATE        GetCpuState;
  EFI_PLATFORM_CPU_STATE_CHANGE_CAP     StateChangeCap;
  EFI_PLATFORM_CPU_CHANGE_CPU_STATE     ChangeCpuState;
  EFI_PLATFORM_CPU_OVERRIDE_CPU_POLICY  OverridePolicy;
  UINT8                                 BspSelection;
  UINT8                                 UpBootSelection;
  UINT8                                 ProcessorBistEnable;
  UINT8                                 ProcessorHyperThreadingDisable;
  UINT8                                 ProcessorVmxEnable;
  UINT8                                 EnableCoresInSbsp;
  UINT8                                 EnableCoresInNbsp;
  UINT32                                DcaPrefetchDelayValue;
  UINT32                                VirtualWireMode;
  BOOLEAN                               ProcessorMsrLockControl;
  BOOLEAN                               Processor3StrikeControl;
  BOOLEAN                               DcaState;
  BOOLEAN                               ExecuteDisableBit;
  BOOLEAN                               CcxEnable;
  BOOLEAN                               C1AutoDemotion;
  BOOLEAN                               C3AutoDemotion;
  UINT8                                 PackageCState;
  BOOLEAN                               C1eEnable;
  BOOLEAN                               Gv3State;
  BOOLEAN                               PsdState;
  BOOLEAN                               CmpState;
  BOOLEAN                               PECIEnable;
  BOOLEAN                               LtEnable;
  BOOLEAN                               L2Enable;
  BOOLEAN                               L2EccEnable;
  BOOLEAN                               FastString;
  BOOLEAN                               MachineCheckEnable;
  BOOLEAN                               MLCSpatialPrefetcherEnable;
  BOOLEAN                               MLCStreamerPrefetcherEnable;
  BOOLEAN                               DCUStreamerPrefetcherEnable;
  BOOLEAN                               DCUIPPrefetcherEnable;
  BOOLEAN                               EchoTprDisable;
  BOOLEAN                               MonitorMwaitEnable;
  BOOLEAN                               TurboModeEnable;
  BOOLEAN                               ExtremeEnable;
  BOOLEAN                               XapicEnable;
  BOOLEAN                               Vr11Enable;
  BOOLEAN                               TdcLimitOverride;
  UINT16                                TdcLimit;
  BOOLEAN                               TdpLimitOverride;
  UINT16                                TdpLimit;
  UINT8                                 RatioLimit1C;
  UINT8                                 RatioLimit2C;
  UINT8                                 RatioLimit3C;
  UINT8                                 RatioLimit4C;
  UINT64                                CpuCommonFeature;
  BOOLEAN                               DCUModeSelection;
  BOOLEAN                               BiDirectionalProchot;
  EFI_PLATFORM_CATEGORY                 PlatformCategory;
  UINT8                                 ActiveProcessorCores;  
} EFI_PLATFORM_CPU_PROTOCOL;

extern EFI_GUID gEfiPlatformCpuProtocolGuid;

*/

#ifndef VFRCOMPILE

#pragma pack(push, 1)

typedef struct _CPU_SETUP_DATA  CPU_SETUP_DATA;

typedef struct _CPU_SETUP_DATA {
    // CPU Setup header
    UINT32  CpuPolicyVersion;

//    UINT8       Clpm; // Current low power mode

    UINT8       ProcessorVmxEnable;
    UINT8       ProcessorHtMode;
    UINT8       ExecuteDisableBit;
    UINT8       ProcessorCcxEnable;
    UINT8       ProcessorEistEnable;
    UINT8       CpuidMaxValue;
    UINT8       MlcStreamerPrefetcherEnable;
    UINT8       MlcSpatialPrefetcherEnable;
    UINT8       DCUStreamerPrefetcherEnable;
    UINT8       DCUIPPrefetcherEnable;
    UINT8       TurboModeEnable;
    UINT8       ProcessorXEEnable;
    UINT8       ProcessorXapic;
    UINT8       ProcessorTDCLimitOverrideEnable;
    UINT8       ProcessorTDCLimit;
    UINT8       ProcessorTDPLimitOverrideEnable;
    UINT8       ProcessorTDPLimit;
    UINT8       RatioLimit1C;
    UINT8       RatioLimit2C;
    UINT8       RatioLimit3C;
    UINT8       RatioLimit4C;
    UINT8       ProcessorVirtualWireMode;
    UINT8       ActiveProcessorCores;
    UINT8       PackageCState;
    UINT8       PsdState;
} CPU_SETUP_DATA;


typedef struct _AMI_CPU_PLATFORM_INFO_HOB
{
    EFI_HOB_GUID_TYPE EfiHobGuidType;
    CPU_SETUP_DATA CpuPolicyData;
} AMI_CPU_PLATFORM_INFO_HOB;

#pragma pack(pop)

typedef VOID (CPU_OEM_SETUP_CALLBACK) (
    IN VOID                 *Services,
    IN OUT CPU_SETUP_DATA    *CpuSetupData,
    IN SETUP_DATA           *SetupData,
    IN BOOLEAN              Pei
);


VOID GetCpuSetupData (
    IN VOID                 *Service,
    IN OUT CPU_SETUP_DATA    *CpuSetupData,
    IN BOOLEAN              Pei
);
#endif
/****** DO NOT WRITE BELOW THIS LINE *******/
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
