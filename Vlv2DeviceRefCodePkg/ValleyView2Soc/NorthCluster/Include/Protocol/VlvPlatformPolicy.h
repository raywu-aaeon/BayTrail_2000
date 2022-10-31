/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  VlvPlatformPolicy.h

Abstract:

  Interface definition details between MCH and platform drivers during DXE phase.

--*/

#ifndef _VLV_PLATFORM_POLICY_H_
#define _VLV_PLATFORM_POLICY_H_

//
// VLV Policy provided by platform for DXE phase {5BAB88BA-E0E2-4674-B6AD-B812F6881CD6}
//
#define DXE_VLV_PLATFORM_POLICY_GUID \
  {0x5bab88ba, 0xe0e2, 0x4674, 0xb6, 0xad, 0xb8, 0x12, 0xf6, 0x88, 0x1c, 0xd6}

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gDxeVlvPlatformPolicyGuid;

//
// Protocol revision number
// Any backwards compatible changes to this protocol will result in an update in the revision number
// Major changes will require publication of a new protocol
//
#define DXE_VLV_PLATFORM_POLICY_PROTOCOL_REVISION 0


typedef struct {
  UINT8  PFITStatus;
  UINT8  IgdTheramlSupport;
  UINT8  ALSEnabled;
  UINT8  LidStatus;
} IGD_PANEL_FEATURES;

typedef struct {
  UINT8   SdpProfile;                     // An enumeration corresponding to SKU.
  UINT8   EnableDptf;                     // Option to enable/disable DPTF
  UINT16  ProcCriticalTemp;               // Processor critical temperature
  UINT16  ProcPassiveTemp;                // Processor passive temperature
  UINT16  GenericCriticalTemp0;           // Critical temperature value for generic sensor0 participant
  UINT16  GenericPassiveTemp0;            // Passive temperature value for generic sensor0 participant
  UINT16  GenericCriticalTemp1;           // Critical temperature value for generic sensor1 participant
  UINT16  GenericPassiveTemp1;            // Passive temperature value for generic sensor1 participant
  UINT16  GenericCriticalTemp2;           // Critical temperature value for generic sensor2 participant
  UINT16  GenericPassiveTemp2;            // Passive temperature value for generic sensor2 participant
  UINT16  GenericCriticalTemp3;           // Critical temperature value for generic sensor3 participant
  UINT16  GenericPassiveTemp3;            // Passive temperature value for generic sensor3 participant
  UINT16  GenericCriticalTemp4;           // Critical temperature value for generic sensor3 participant
  UINT16  GenericPassiveTemp4;            // Passive temperature value for generic sensor3 participant    
  UINT8   Clpm;                          // Current low power mode
  UINT8   SuperDebug;                    // DPTF Super debug option
  UINT32  LPOEnable;                      // DPTF: Instructs the policy to use Active Cores if they are available. If this option is set to 0, then policy does not use any active core controls ?even if they are available
  UINT32  LPOStartPState;                 // DPTF: Instructs the policy when to initiate Active Core control if enabled. Returns P state index.
  UINT32  LPOStepSize;                    // DPTF: Instructs the policy to take away logical processors in the specified percentage steps
  UINT32  LPOPowerControlSetting;         // DPTF: Instructs the policy whether to use Core offliing or SMT offlining if Active core control is enabled to be used in P0 or when power control is applied. 1 ?SMT Off lining 2- Core Off lining
  UINT32  LPOPerformanceControlSetting;   // DPTF: Instructs the policy whether to use Core offliing or SMT offlining if Active core control is enabled to be used in P1 or when performance control is applied.1 ?SMT Off lining 2- Core Off lining 
  UINT8   EnableDppm;                     // DPTF DPPM enable flag
  UINT8   AmbientTripPointChange;         // DPTF: Controls whether _ATI changes other participant's trip point(enabled/disabled)  
} DPTF_SETTINGS;

typedef struct {
  UINT8 LoadVbios    : 1;  // Specifies whether to load dGPU OPROM (GOP/VBIOS) or not (1=Load; 0=Don't load)
  UINT8 ExecuteVbios : 1;  // Specifies whether to execute dGPU OPROM (GOP/VBIOS) or not (1=execute if LoadVbios is 1, 0=Don't execute)
  UINT8 VbiosSource  : 1;  // Specifies source location of dGPU OPROM (GOP/VBIOS) (1=PCI Card, 0 = FW Volume)
  UINT8 Reserved     : 5;
} SG_VBIOS_CONFIGURATION;
//
// MCH DXE Platform Policy ==================================================
//

#define NO_AUDIO   0
#define HD_AUDIO   1
#define LPE_AUDIO  2

typedef struct _DXE_VLV_PLATFORM_POLICY_PROTOCOL {
  UINT8                   Revision;
  IGD_PANEL_FEATURES      IgdPanelFeatures;
  DPTF_SETTINGS           DptfSettings;
  UINT8                   EnableRenderStandby;
  UINT8                   GraphicsPerfAnalyzers;
  UINT8                   MaxInverterPWM;
  UINT8                   MinInverterPWM;  
  UINT8                   PmSupport;
  UINT8                   ForceWake;
  UINT8                   GfxPause;
  UINT8                   GraphicsFreqReq;
  UINT8                   EuControl;
  UINT8                   PmWeights;
  UINT8                   PmLock;
  UINT8                   PavpMode;
  UINT8                   DopClockGating;
  UINT8                   UlClockGating;
  UINT8                   S0ixSupported;
  UINT8                   AudioTypeSupport;
  UINT8                   EnableIGDTurbo;
  SG_VBIOS_CONFIGURATION  VbiosConfig;  // This field is used to describe Hybrid Graphics configuration 
} DXE_VLV_PLATFORM_POLICY_PROTOCOL;

#endif
