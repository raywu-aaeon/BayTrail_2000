/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  VlvPolicy.h

Abstract:

  Interface definition details between ValleyView MRC and platform drivers during PEI phase.

--*/

#ifndef _VLV_POLICY_PPI_H_
#define _VLV_POLICY_PPI_H_

//
// MRC Policy provided by platform for PEI phase {7D84B2C2-22A1-4372-B12C-EBB232D3A6A3}
//
#define VLV_POLICY_PPI_GUID \
  { \
    0x7D84B2C2, 0x22A1, 0x4372, 0xB1, 0x2C, 0xEB, 0xB2, 0x32, 0xD3, 0xA6, 0xA3 \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gVlvPolicyPpiGuid;

//
// PPI revision number
// Any backwards compatible changes to this PPI will result in an update in the revision number
// Major changes will require publication of a new PPI
//
#define MRC_PLATFORM_POLICY_PPI_REVISION  1

#ifndef MAX_SOCKETS
#define MAX_SOCKETS 4
#endif

#define S3_TIMING_DATA_LEN          9
#define S3_READ_TRAINING_DATA_LEN   16
#define S3_WRITE_TRAINING_DATA_LEN  12

#ifndef S3_RESTORE_DATA_LEN
#define S3_RESTORE_DATA_LEN (S3_TIMING_DATA_LEN + S3_READ_TRAINING_DATA_LEN + S3_WRITE_TRAINING_DATA_LEN)
#endif // S3_RESTORE_DATA_LEN
#pragma pack(1)
//
// MRC Platform Data Structure
//
typedef struct {
  UINT8   SpdAddressTable[MAX_SOCKETS];
  UINT8   TSonDimmSmbusAddress[MAX_SOCKETS];

  UINT16  SmbusBar;
  UINT32  IchRcba;
  UINT32  WdbBaseAddress; // Write Data Buffer area (WC caching mode)
  UINT32  WdbRegionSize;
  UINT32  SmBusAddress;
  UINT8   UserBd;
  UINT8   PlatformType;
  UINT8   FastBoot;
  UINT8   DynSR;
  UINT8   SgMode;                            ///< SgMode (0=Disabled, 1=SG Muxed, 2=SG Muxless, 3=PEG)
  UINT16  SgSubSystemId;                     ///< Switchable Graphics Subsystem ID
} VLV_PLATFORM_DATA;


typedef struct {
  UINT16  MmioSize;
  UINT16  GttSize;
  UINT8   IgdDvmt50PreAlloc;
  UINT8   PrimaryDisplay;
  UINT8   PAVPMode;
  UINT8   ApertureSize;
  UINT8   InternalGraphics;
  UINT8   IgdTurboEn;
} GT_CONFIGURATION;

typedef struct {
  UINT8   EccSupport;
  UINT16  DdrFreqLimit;
  UINT8   MaxTolud;
} MEMORY_CONFIGURATION;

///
/// SA GPIO Data Structure
///
typedef struct {
  UINT8   Value;  ///< GPIO Value
  BOOLEAN Active; ///< 0=Active Low; 1=Active High
} SG_GPIO_INFO;

///
/// Defines the Switchable Graphics configuration parameters for System Agent.
///
typedef struct {
  BOOLEAN       GpioSupport;        ///< 1=Supported; 0=Not Supported
  SG_GPIO_INFO  *SgDgpuPwrOK;       ///< This field contain dGPU PWROK GPIO value and Level information
  SG_GPIO_INFO  *SgDgpuHoldRst;     ///< This field contain dGPU HLD RESET GPIO value and level information
  SG_GPIO_INFO  *SgDgpuPwrEnable;   ///< This field contain dGPU_PWR Enable GPIO value and level information
  SG_GPIO_INFO  *SgDgpuPrsnt;       ///< This field contain dGPU_PRSNT# GPIO value and level information
} SG_GPIO_DATA;

//
// MRC Platform Policiy PPI
//
typedef struct _VLV_POLICY_PPI {
  UINT8                 Revision;
  VLV_PLATFORM_DATA      PlatformData;
  GT_CONFIGURATION      GtConfig;
  MEMORY_CONFIGURATION  MemConfig;
  VOID                  *S3DataPtr; // was called MRC_PARAMS_SAVE_RESTORE
  UINT8                 ISPEn;            //ISP (IUNIT) Device Enabled
  UINT8                 ISPPciDevConfig;  //ISP (IUNIT) Device Config: 0->B0/D2/F0 for Window OS, 1->B0D3/F0 for Linux OS
  UINT8                 S0ixEn;
  SG_GPIO_DATA          *SgGpioData;               ///< Switchable Graphics GPIO data (REVISION_3)
} VLV_POLICY_PPI;

#pragma pack()

#endif // _VLV_POLICY_PPI_H_
