/*++

Copyright (c) 2005-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  MemoryInit.c

Abstract:

  Framework PEIM to initialize memory on a Memory Controller.

--*/

//
// Statements that include other files
//
#include "VlvAccess.h"
#include "MemoryInit.h"
#include "MchRegs.h"
#include "DetectDimms.h"
#include "SCRegs.h"


#include <Guid/MemoryConfigData.h>
#include <Guid/AcpiVariable.h>
#ifdef ECP_FLAG
#include <Guid/GlobalVariable/GlobalVariable.h>
#include <Guid/SmramMemoryReserve/SmramMemoryReserve.h>
#include <Guid/PlatformInfo.h>  // ERROR : This is in Platform tip.
#include <Guid/Vlv2Variable.h>
#define __EDKII_GLUE_PCD_PcdCpuIEDEnabled__   FALSE
#define __EDKII_GLUE_PCD_PcdCpuIEDRamSize__   0x20000
#else
#include <Library/PcdLib.h>
#include <Guid/Vlv2Variable.h>
#include <Guid/GlobalVariable.h>
#include <Guid/SmramMemoryReserve.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Guid/PlatformInfo.h>

#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/ReadOnlyVariable.h>
#include <Ppi/Capsule.h>
#endif
#include <Ppi/SeCUma.h>
#if defined (TABLET_PF_ENABLE) && (TABLET_PF_ENABLE == 1)    // AMI_OVERRIDE - Fix building error.
#include <Ppi/MfgMemoryTest.h>
#include <Guid/SetupVariable.h>
#endif
#include <Ppi/fTPM.h>
#include <AmiLib.h> //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.
#include <Setup.h>  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.
#include <Library/MemoryDownLib.h>  //AMI_OVERRIDE - EIP168616 memory down function

#define EFI_CU_MEMORY_PC_COMPLETE         (EFI_SUBCLASS_SPECIFIC | 0x00000007)
#ifdef ECP_FLAG
EFI_GUID  gEfiGlobalVariableGuid = EFI_GLOBAL_VARIABLE_GUID;
EFI_GUID gEfiPlatformInfoGuid  = EFI_PLATFORM_INFO_GUID;
EFI_GUID gEfiMemoryConfigDataGuid = EFI_MEMORY_CONFIG_DATA_GUID;
EFI_GUID gEfiVlv2VariableGuid = EFI_VLV2_VARIABLE;
#ifdef FTPM_ENABLE
EFI_GUID gSeCfTPMPpiGuid = SEC_FTPM_PPI_GUID;
#endif
#ifdef SEC_SUPPORT_FLAG
EFI_GUID gSeCUmaPpiGuid = SEC_UMA_PPI_GUID;
#endif
#if SMM_SUPPORT
EFI_GUID gEfiSmmPeiSmramMemoryReserveGuid = EFI_SMM_PEI_SMRAM_MEMORY_RESERVE;
#endif
#endif

  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.>>
#define NB_MAX_TOLUD_1G         0xC00
#define NB_MAX_TOLUD_1_25G      0xB00
#define NB_MAX_TOLUD_1_5G       0xA00
#define NB_MAX_TOLUD_1_75G      0x900
#define NB_MAX_TOLUD_2G         0x800
#define NB_MAX_TOLUD_2_25G      0x700
#define NB_MAX_TOLUD_2_5G       0x600
#define NB_MAX_TOLUD_2_75G      0x500
#define NB_MAX_TOLUD_3G         0x400
#define NB_MAX_TOLUD_3_25G      0x300
#define NB_MAX_TOLUD_3_5G       0x200
#define MemoryCeilingVariable   L"MemCeil."
  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.<<

CHAR16    EfiMemoryConfigVariable[] = L"MemoryConfig";

UINT8   SPD_FBTable[] = {
        SPD_DDR3_MANUFACTURER_ID_LO,    //117     
        SPD_DDR3_MANUFACTURER_ID_HI,    //118
        SPD_DDR3_MANUFACTURE_LOCATION,  //119
        SPD_DDR3_MANUFACTURE_DATE_LO,  //120
        SPD_DDR3_MANUFACTURE_DATE_HI,  //121
        SPD_DDR3_SERIAL_NUMBER_1,      //122 
        SPD_DDR3_SERIAL_NUMBER_2,      //123 
        SPD_DDR3_SERIAL_NUMBER_3,      //124
        SPD_DDR3_SERIAL_NUMBER_4,      //125
};

// AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)>>
extern EFI_GUID                 gAmiPeiBeforeMrcGuid;
extern EFI_GUID                 gAmiPeiAfterMrcGuid;
extern EFI_GUID                 gAmiPeiEndOfMemDetectGuid;

static EFI_PEI_PPI_DESCRIPTOR mAmiPeiBeforeMrcDesc[] = {
{ (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST), \
&gAmiPeiBeforeMrcGuid, \
NULL }
};

static EFI_PEI_PPI_DESCRIPTOR mAmiPeiCompelteMrcDesc[] = {
{ (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST), \
&gAmiPeiAfterMrcGuid, \
NULL }
};

static EFI_PEI_PPI_DESCRIPTOR mAmiPeiEndOfMrcDesc[] = {
{ (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST), \
&gAmiPeiEndOfMemDetectGuid, \
NULL }
};
// AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)<<


EFI_STATUS
EFIAPI
PeimMemoryInit (
#ifdef ECP_FLAG
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
#endif
  )
{
  EFI_STATUS                      Status;
  EFI_BOOT_MODE                   BootMode;
  MRC_PARAMETER_FRAME             CurrentMrcData;
  EFI_PLATFORM_INFO_HOB           *PlatformInfo;
#ifdef ECP_FLAG
  EFI_HOB_GUID_TYPE               *GuidHob;
#else
  EFI_PEI_HOB_POINTERS            Hob;
#endif
  UINT8                           RevId;

//  UINT32 IoBase;

  UINT8   Channel;
  BOOLEAN coldBootRequired = FALSE;
#ifdef SEC_SUPPORT_FLAG
  SEC_UMA_PPI              *SeCUma;
#ifdef FTPM_ENABLE
  SEC_FTPM_PPI             *fTPMUma;
#endif
  UINT8                     InitStat;
#endif
  MRC_DRAM_INPUT Input_Struct;

  // AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)>>
// Install the NB Before Mrc Notify PPI
Status = (*PeiServices)->InstallPpi(PeiServices, &mAmiPeiBeforeMrcDesc[0]);
ASSERT_EFI_ERROR (Status);
  // AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)<<

  //
  // OEM TO ADD INITIALIZATION !!!
  // CPU_IO
  // PCI_CFG
  // SB_INIT
  // - SMBUS Base Address (Bus:00 Device:31 Function:03 Register:0x20) this I/O space must be enabled in the SMBUS CMD register (Bus:00 Device:31 Function:03 Register:0x04)
  // - RCBA (Bus:00 Device:31 Function:00 Register:0xF0)
  // NB_INIT
  // - PCIEXPBAR (Bus:00 Device:00 Function:00 Register:0x60)
  // - MCHBAR (Bus:00 Device:00 Function:00 Register: 0x48)
  // - Graphics UMA and GTT size (Bus: 00 Device:00 Function:00 Register:0x52)
  //

  //
  // Initialize data structures with value 0
  //
#ifdef ECP_FLAG
  memset(&CurrentMrcData, 0, sizeof(MRC_PARAMETER_FRAME));
  memset(&Input_Struct, 0, sizeof(MRC_DRAM_INPUT));
#else
  SetMem(&CurrentMrcData, sizeof(MRC_PARAMETER_FRAME), 0);
  SetMem(&Input_Struct, sizeof(MRC_DRAM_INPUT), 0);
#endif

  //
  // Disable PCH Watchdog timer at SB_RCBA+0x3410 // "No Reboot Strap" bit moved to PBASE + 0x08 [4]
  //
  Mmio32(PMC_BASE_ADDRESS, 0x08) |= 0x10;

  //
  // Determine boot mode
  //
  Status = (*PeiServices)->GetBootMode (
    PeiServices,
    &BootMode
  );
  ASSERT_EFI_ERROR (Status);

#ifdef SEC_SUPPORT_FLAG
  InitStat = 0;
#endif
  
  //
  // Set OEM MRC Data
  //
  Status = SetOemMrcData (
    PeiServices,
    &CurrentMrcData
  );
  
#ifdef ECP_FLAG
  GuidHob = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (GuidHob != NULL);
  if(GuidHob  == NULL)
  {
    return FAILURE;
  }  
  PlatformInfo = (EFI_PLATFORM_INFO_HOB *)GET_GUID_HOB_DATA(GuidHob);
#else
   Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  if(Hob.Raw  == NULL)
  {
    return FAILURE;
  }  
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);
#endif

  switch (PlatformInfo->BoardId) {
    case BOARD_ID_CVH:
      CurrentMrcData.currentPlatform = PLAT_SV_DDR3L_ALPINE_VALLEY;
      CurrentMrcData.DDRType = DDRType_DDR3L;
      break;
    case BOARD_ID_AV_SVP:
      if (PlatformInfo->DDRDaughterCardCh0Id == 0x2) {
        CurrentMrcData.currentPlatform = PLAT_SV_LPDDR3_ALPINE_VALLEY;
        CurrentMrcData.DDRType = DDRType_LPDDR3;
      } else if (PlatformInfo->DDRDaughterCardCh0Id == 0x5) {
          CurrentMrcData.currentPlatform = PLAT_SV_DDR3L_ECC_ALPINE_VALLEY;
          CurrentMrcData.DDRType = DDRType_DDR3ECC;
      } else {
        CurrentMrcData.currentPlatform = PLAT_SV_DDR3L_ALPINE_VALLEY;
        CurrentMrcData.DDRType = DDRType_DDR3L;
      }
      break;  
    case BOARD_ID_BL_RVP:
    case BOARD_ID_BL_FFRD:
    case BOARD_ID_BL_FFRD8:
    case BOARD_ID_BL_STHI:
      CurrentMrcData.currentPlatform = PLAT_EV_LPDDR3_BAYLAKE;  //need the mem config
      CurrentMrcData.DDRType = DDRType_LPDDR3;
      if (PlatformInfo->MemCfgID == 1) {
        CurrentMrcData.RankIndex = 2;
      } else {
        CurrentMrcData.RankIndex = 1;
      }
      break;
    case BOARD_ID_BB_RVP:
    case BOARD_ID_BL_RVP_DDR3L:
      CurrentMrcData.currentPlatform = PLAT_EV_DDR3L_BAY_TRAIL;
      CurrentMrcData.DDRType = DDRType_DDR3L;
      break;
    case BOARD_ID_BS_RVP:
      CurrentMrcData.currentPlatform = PLAT_SV_DDR3L_ECC_ALPINE_VALLEY;
      CurrentMrcData.DDRType = DDRType_DDR3ECC;
      break;
    default:
      CurrentMrcData.DDRType = DDRType_DDR3L;
      CurrentMrcData.currentPlatform = PLAT_SV_DDR3L_ALPINE_VALLEY;
  }

  //Hook for OEM to Current Platform Design
  CurrentMrcData.currentPlatformDesign = DEFAULT_PLATFORM_DESIGN;
  //Hook for OEM to override CPU ODT value
  CurrentMrcData.override_cpu_ODT_value = CPU_ODT_DEFAULT;
  //Hook for OEM to override DRAM ODT value, default is RTTWR_120
  CurrentMrcData.override_dram_ODT_value = DRAM_ODT_DEFAULT;
  //Hook for OEM to override DRAM RTT NOM, default is RTT NOM Enabled
  CurrentMrcData.override_dram_RTT_NOM = DRAM_RTT_NOM_DEFAULT;
  //Hook for OEM to override Auto Self Refresh MR2 ASR, default is ASR ON for VLV CR and OFF for others
  CurrentMrcData.overrideAutoSelfRefreshASR = AUTO_SELF_REFRESH_DEFAULT;


  //OEM to set dram type used in the CurrentMrcData for memory down
  //OEM to configure CurrentMrcData.currentPlatform,  CurrentMrcData.DDRType, CurrentMrcData.currentPlatformDesign if BoardId is not supported 

  //RVP DDR3L memdown
  if (PlatformInfo->BoardId == BOARD_ID_BL_RVP_DDR3L) {
      CurrentMrcData.currentPlatformDesign = BLK_RVP_DDR3L;
  }
  if (CurrentMrcData.currentPlatformDesign == BLK_RVP_DDR3L) {
      Input_Struct.Rank_En[0][0] = 1;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[0][1] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[1][0] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[1][1] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.DIMM_DWidth[0][0] = 0x1;    /**<  [Channel][Slot] DIMM0 DRAM device data width 00:x8, 01:x16, 02:x32*/
      Input_Struct.DIMM_Density[0][0] = 0x2;   /**< [Channel][Slot] DIMM0 DRAM device data density  00:1Gbit, 01:2Gbit,02:4Gbit,03:8Gbit*/
      Input_Struct.DRAM_Speed = 0x2;           /**< 00:800, 01:1066, 02:1333, 03:1600 */ 
      Input_Struct.DRAM_Type = 0x1;            /**< 00:DDR3, 01:DDR3L, 02:DDR3U, 04:LPDDR2, 05:LPDDR3, 06:DDR4 */ 
      Input_Struct.DIMM_MemDown = 0x1;         /**< 0:DIMM, 1:Memory Down */    
      Input_Struct.DIMM_BusWidth[0][0] = 3;    /**< [Channel][Slot] 000:8 bits; 01:16bits, 02:32bits, 03:64bits */
      Input_Struct.DIMM_Sides[0][0] = 0;       /**< [Channel][Slot] ranks per dimm 00:1rank, 01:2ranks, 02:3ranks, 03:4ranks */
      Input_Struct.tCL = 9;                    /**< actual CL */ 
      Input_Struct.tRP_tRCD = 9;               /**< TRP and tRCD in dram clk - 5:12.5ns, 6:15ns, 7:*/ 
      Input_Struct.tWR = 10;                   /**< in dram clk  */ 
      Input_Struct.tWTR = 5;                   /**< in dram clk  */  
      Input_Struct.tRRD = 5;                   /**< in dram clk  */  
      Input_Struct.tRTP = 5;                   /**< in dram clk  */ 
      Input_Struct.tFAW = 30;  
  }

  //Crestview Hills DDR3L memdown
  if (PlatformInfo->BoardId == BOARD_ID_CVH) {
      Input_Struct.Rank_En[0][0] = 1;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[0][1] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[1][0] = 1;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[1][1] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.DIMM_DWidth[0][0] = 0x1;    /**<  [Channel][Slot] DIMM0 DRAM device data width 00:x8, 01:x16, 02:x32*/
      Input_Struct.DIMM_DWidth[1][0] = 0x1;    /**<  [Channel][Slot] DIMM0 DRAM device data width 00:x8, 01:x16, 02:x32*/
      Input_Struct.DIMM_Density[0][0] = 0x2;   /**< [Channel][Slot] DIMM0 DRAM device data density  00:1Gbit, 01:2Gbit,02:4Gbit,03:8Gbit*/
      Input_Struct.DIMM_Density[1][0] = 0x2;   /**< [Channel][Slot] DIMM0 DRAM device data density  00:1Gbit, 01:2Gbit,02:4Gbit,03:8Gbit*/
      Input_Struct.DRAM_Speed = 0x2;           /**< 00:800, 01:1066, 02:1333, 03:1600 */ 
      Input_Struct.DRAM_Type = 0x1;            /**< 00:DDR3, 01:DDR3L, 02:DDR3U, 04:LPDDR2, 05:LPDDR3, 06:DDR4 */ 
      Input_Struct.DIMM_MemDown = 0x1;         /**< 0:DIMM, 1:Memory Down */    
      Input_Struct.DIMM_BusWidth[0][0] = 3;    /**< [Channel][Slot] 000:8 bits; 01:16bits, 02:32bits, 03:64bits */
      Input_Struct.DIMM_BusWidth[1][0] = 3;    /**< [Channel][Slot] 000:8 bits; 01:16bits, 02:32bits, 03:64bits */
      Input_Struct.DIMM_Sides[0][0] = 0;       /**< [Channel][Slot] ranks per dimm 00:1rank, 01:2ranks, 02:3ranks, 03:4ranks */
      Input_Struct.DIMM_Sides[1][0] = 0;       /**< [Channel][Slot] ranks per dimm 00:1rank, 01:2ranks, 02:3ranks, 03:4ranks */
      Input_Struct.tCL = 9;                    /**< actual CL */ 
      Input_Struct.tRP_tRCD = 9;               /**< TRP and tRCD in dram clk - 5:12.5ns, 6:15ns, 7:*/ 
      Input_Struct.tWR = 10;                   /**< in dram clk  */ 
      Input_Struct.tWTR = 5;                   /**< in dram clk  */  
      Input_Struct.tRRD = 5;                   /**< in dram clk  */  
      Input_Struct.tRTP = 5;                   /**< in dram clk  */ 
      Input_Struct.tFAW = 30;                  /**< in dram clk  */  
  }

  //Baytrail 25x27 4 Layers Memory Down Design
  //1 Channel, Single Rank, Memory Speed 1066Mhz, Device Width x16, Device Density 4Gbit
  if (CurrentMrcData.currentPlatformDesign == BBY_25x27_4LAYERS_DDR3L_MEMDOWN) {
      CurrentMrcData.currentPlatform = PLAT_EV_DDR3L_BAY_TRAIL;
      CurrentMrcData.DDRType = DDRType_DDR3L;
      Input_Struct.Rank_En[0][0] = 1;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[0][1] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[1][0] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.Rank_En[1][1] = 0;          /**<  [Channel][Rank] Ranks Present with MAX_RANKS defined in Imemory.h */
      Input_Struct.DIMM_DWidth[0][0] = 0x1;    /**<  [Channel][Slot] DIMM0 DRAM device data width 00:x8, 01:x16, 02:x32*/
      Input_Struct.DIMM_Density[0][0] = 0x2;   /**< [Channel][Slot] DIMM0 DRAM device data density  00:1Gbit, 01:2Gbit,02:4Gbit,03:8Gbit*/
      Input_Struct.DRAM_Speed = 0x1;           /**< 00:800, 01:1066, 02:1333, 03:1600 */ 
      Input_Struct.DRAM_Type = 0x1;            /**< 00:DDR3, 01:DDR3L, 02:DDR3U, 04:LPDDR2, 05:LPDDR3, 06:DDR4 */ 
      Input_Struct.DIMM_MemDown = 0x1;         /**< 0:DIMM, 1:Memory Down */    
      Input_Struct.DIMM_BusWidth[0][0] = 3;    /**< [Channel][Slot] 000:8 bits; 01:16bits, 02:32bits, 03:64bits */
      Input_Struct.DIMM_Sides[0][0] = 0;       /**< [Channel][Slot] ranks per dimm 00:1rank, 01:2ranks, 02:3ranks, 03:4ranks */
      Input_Struct.tCL = 7;                    /**< actual CL */
      Input_Struct.tRP_tRCD = 7;               /**< TRP and tRCD in dram clk - 5:12.5ns, 6:15ns, 7:*/
      Input_Struct.tWR = 8;                   /**< in dram clk  */
      Input_Struct.tWTR = 4;                   /**< in dram clk  */
      Input_Struct.tRRD = 4;                   /**< in dram clk  */
      Input_Struct.tRTP = 4;                   /**< in dram clk  */
      Input_Struct.tFAW = 20;
  }
  
//AMI_OVERRIDE - EIP168616 memory down function >> 
#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
  FillMemoryDownParam (&Input_Struct);
#endif
//AMI_OVERRIDE - EIP168616 memory down function <<

  //
  // Execute DetectDimms
  //

  if (BootMode == BOOT_ON_S3_RESUME) {
    CurrentMrcData.BootMode = S3Path;
  } 
  Status = GetPlatformSettings (&CurrentMrcData);
  if (Status == FAILURE) {
    return FAILURE;
  }
  
  //FastBoot
  CurrentMrcData.FastBootEnable = 1;

    if (CurrentMrcData.BootMode != S3Path) {
        MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_PRESENCE_DETECT));

        if ((CurrentMrcData.FastBootEnable) && (CurrentMrcData.OemMrcData.MrcParamsValidFlag)) {

        	//DEBUG ((EFI_D_ERROR, "fastboot\n"));
            if (Input_Struct.DIMM_MemDown || (CurrentMrcData.DDRType == DDRType_LPDDR3)) {
            	RevId = * (UINT8 *)(0xe00f8008);
            	FillDimmsParam (&CurrentMrcData, Input_Struct);
                if ((CurrentMrcData.OemMrcData.CoreFreq) == CurrentMrcData.DdrFreqCap) {
                    CurrentMrcData.BootMode = FBPath;
                } else {
                	CurrentMrcData.BootMode = S5Path;
                }
            } else {

                for (Channel = 0; Channel < MAX_CHANNELS_TOTAL; Channel++) {
                	CurrentMrcData.TotalDimm[Channel] = 0;
                    if ( GetSpdData(&CurrentMrcData, Channel, &SPD_FBTable[0], ((sizeof SPD_FBTable)/(sizeof SPD_FBTable[0]))) == SUCCESS) {
            
                    coldBootRequired = CheckColdBootRequired (&CurrentMrcData, Channel);
                    //DEBUG ((EFI_D_INFO, "coldBootRequired %x\n",coldBootRequired));
                    if (coldBootRequired) break;
                    } else {
                    //DEBUG ((EFI_D_INFO, "GetSpdData Failed\n"));
                    return FAILURE;
                    }
                }  //end of for channel
    
                if (coldBootRequired) {
                    //DEBUG ((EFI_D_INFO, "DetectDimms\n"));
                    CurrentMrcData.BootMode = S5Path;
                    MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_PRESENCE_DETECT));
                    if ( DetectDimms(&CurrentMrcData) != SUCCESS) {
                        //DEBUG ((EFI_D_INFO, "DetectDimms not success\n"));
                        MRC_PEI_REPORT_ERROR_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED), EFI_ERROR_MAJOR);
                        MRC_DEADLOOP();
                    }
                } else {
                    CurrentMrcData.BootMode = FBPath;
                }
            }  //end of if not memory down
        } else {  //Data not valid
            CurrentMrcData.BootMode = S5Path;
            if (( Input_Struct.DIMM_MemDown) || (CurrentMrcData.DDRType == DDRType_LPDDR3) ){
                FillDimmsParam (&CurrentMrcData, Input_Struct);
            } else {
                if ( DetectDimms(&CurrentMrcData) != SUCCESS) {
                    MRC_PEI_REPORT_ERROR_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED), EFI_ERROR_MAJOR);
                    MRC_DEADLOOP();
                }
            }
        }
    } //S3path
    


#ifdef SEC_SUPPORT_FLAG

    //
    // Locate SeCUma PPI which will be used in the following flows: coldboot and S3Resume
    //
    Status = (*PeiServices)->LocatePpi (PeiServices, &gSeCUmaPpiGuid, 0, NULL, &SeCUma);
    ASSERT_EFI_ERROR ( Status);
#ifdef FTPM_ENABLE
		Status = (*PeiServices)->LocatePpi (PeiServices, &gSeCfTPMPpiGuid, 0, NULL, &fTPMUma);
		if (EFI_ERROR(Status)) {
      fTPMUma = NULL;
    }
#endif

    //
    // SEC Stolen Size in MB units
    //
    //DEBUG ((EFI_D_INFO, "MRC getting memory size from SeC ...\n"));
    CurrentMrcData.SeCUmaSize = SeCUma->SeCSendUmaSize ((EFI_PEI_SERVICES    **)PeiServices);  //expect in MB 
    //DEBUG ((EFI_D_INFO, "MRC SeCUmaSize memory size from SeC ... %x \n", CurrentMrcData.SeCUmaSize));
#ifdef FTPM_ENABLE
		//DEBUG ((EFI_D_INFO, "MRC getting fTPM memory size from SeC ...\n"));
    if (fTPMUma == NULL) {
		  CurrentMrcData.SeCfTPMUmaSize = 0;  //expect in MB
    } else {
		  CurrentMrcData.SeCfTPMUmaSize = fTPMUma->SeCSendfTPMSize ((EFI_PEI_SERVICES    **)PeiServices);  //expect in MB
    }
		//DEBUG ((EFI_D_INFO, "MRC SeCfTPMUmaSize memory size from SeC ... %x \n", CurrentMrcData.SeCfTPMUmaSize));
#endif
#endif
  // Override Detected DIMM settings here
  MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_TIMING));

  CurrentMrcData.PcdCpuIEDEnabled = PcdGetBool (PcdCpuIEDEnabled);
  CurrentMrcData.IedSize = PcdGet32(PcdCpuIEDRamSize);

  //
  // Execute ConfigureMemory
  //
  MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_CONFIGURING));
  DEBUG ((EFI_D_INFO, "Configuring Memory...\n"));
  Status = ConfigureMemory(&CurrentMrcData);
  if (Status != EFI_SUCCESS) {
    MRC_PEI_REPORT_ERROR_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_USEFUL), EFI_ERROR_MAJOR);
    //DEBUG ((EFI_D_INFO, "Configuring Memory failed...\n"));
    MRC_DEADLOOP();
  }
  
  DEBUG ((EFI_D_ERROR, "MRC INIT DONE\n"));
#ifdef SEC_SUPPORT_FLAG
    //DEBUG ((EFI_D_INFO, "MRC allocating memory for SeC ...\n"));
#ifdef FTPM_ENABLE
    if (fTPMUma != NULL) {
  		fTPMUma->SeCConfigfTPM ((EFI_PEI_SERVICES           **)PeiServices, CurrentMrcData.BootMode, 0x01, CurrentMrcData.SeCfTPMUmaBase,CurrentMrcData.SeCfTPMUmaSize);
    }
#endif
    SeCUma->SeCConfigDidReg ((EFI_PEI_SERVICES           **)PeiServices, CurrentMrcData.BootMode, 0x00, CurrentMrcData.SeCUmaBase,CurrentMrcData.SeCUmaSize);
#endif

    // AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)>>
// Install the NB End of Mrc Notify PPI
Status = (*PeiServices)->InstallPpi(PeiServices, &mAmiPeiCompelteMrcDesc[0]);
ASSERT_EFI_ERROR (Status);
DEBUG ((EFI_D_INFO, "Install Complete MRC Ppi.\n"));
Status = (*PeiServices)->GetBootMode( PeiServices, &BootMode );
    // AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)<<

  //
  // Install memory
  //
  DEBUG ((EFI_D_INFO, "Install EFI Memory.\n"));
  if (BootMode == BOOT_ON_S3_RESUME) {
    //DEBUG ((EFI_D_INFO, "Following BOOT_ON_S3_RESUME boot path.\n"));

    Status = InstallS3Memory (PeiServices);
    ASSERT_EFI_ERROR (Status);
  }
  Status = InstallEfiMemory (PeiServices, BootMode, &CurrentMrcData);
  if (BootMode != BOOT_ON_S3_RESUME) {
    MrcParamsSave (PeiServices, &CurrentMrcData);
    //DEBUG ((EFI_D_INFO, "Save MRC params.\n"));
  }
  if (Status != EFI_SUCCESS) {
    MRC_PEI_REPORT_ERROR_CODE(PeiServices, (EFI_SOFTWARE_PEI_SERVICE | EFI_SW_EC_NON_SPECIFIC), EFI_ERROR_MAJOR);
    //DEBUG ((EFI_D_INFO, "MemoryInit Not Installed.\n"));
  } else {
    MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_SOFTWARE_PEI_SERVICE | EFI_SW_PS_PC_INSTALL_PEI_MEMORY));
    //DEBUG ((EFI_D_INFO, "MemoryInit Installed.\n"));
  }


  // AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)>>
// Install the NB End of Mrc Notify PPI
Status = (*PeiServices)->InstallPpi(PeiServices, &mAmiPeiEndOfMrcDesc[0]);
ASSERT_EFI_ERROR (Status);
  // AMI_OVERRIDE - Set Notify point at BeforeMrc and AfterMrc. (EIP127538+)<<

  return EFI_SUCCESS;
}

EFI_STATUS
InstallEfiMemory (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                  **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES            **PeiServices,
#endif
  IN  EFI_BOOT_MODE               BootMode,
  MRC_PARAMETER_FRAME             *CurrentMrcData
  )
{
  EFI_STATUS                      Status;
  EFI_PHYSICAL_ADDRESS            UpperTotalMemory = 0;
  EFI_PHYSICAL_ADDRESS            dBMBOUNDHI = 0;
  EFI_PHYSICAL_ADDRESS            dBMBOUND = 0;
  EFI_PHYSICAL_ADDRESS            dGFXBase = 0;
  EFI_PHYSICAL_ADDRESS            dTSegBase = 0;
  UINT32                          dTSegSize = 0;
  UINT32                          buffer32;
  UINT8                           RemapEnable=0;
  EFI_PHYSICAL_ADDRESS            dPeiMemBase = 0;
// AMI_OVERRIDE - EIP128872 CSM SUPPORT>>
#if defined(CSM_SUPPORT) && CSM_SUPPORT
  UINTN                           PeiMemSize = 0x20000000; //512M, enarge PEI space for 64b CSM BIOS. //EIP133840
#else
  UINTN                           PeiMemSize = 0xA000000;  //160M, enarge PEI space for 64b BIOS.
#endif
// AMI_OVERRIDE - EIP128872 CSM SUPPORT<<
  UINT32                          ReservedMemoryBase;
  UINT32                          ReservedMemorySize;
  UINT32                          AdjustedReservedMemorySize;
  UINT32                          AdjustedReservedMemoryBase;


#if SMM_SUPPORT
  EFI_SMRAM_HOB_DESCRIPTOR_BLOCK  *SmramHobDescriptorBlock;
  UINT32 IedSize;
#endif
  PEI_CAPSULE_PPI                       *Capsule;
  VOID                                  *CapsuleBuffer;
  UINTN                                 CapsuleBufferLength;

  UpperTotalMemory = (EFI_PHYSICAL_ADDRESS)(LShiftU64(CurrentMrcData->C0MemorySize,20));
  //BMBOUND_HI
  //31:24 HOST_IO_BOUND_HIGH - Bits 31:24 are compared with bits 35:28 of incoming addresses of all memory accesses above 4GB 
  MsgBus32Read(VLV_UNIT_BUNIT,BUNIT_BMBOUND_HI_OFFSET,buffer32);
  dBMBOUNDHI    =  (EFI_PHYSICAL_ADDRESS)(LShiftU64((buffer32&0xFF000000),4));

  MsgBus32Read(VLV_UNIT_BUNIT,BUNIT_BMBOUND_OFFSET,buffer32);
  //BMBOUND
  //31:27 HOST_IO_BOUNDARY are compared with incoming Host Memory Request addresses 
  //To understand whether the associated transactions should be routed to memory space (DRAM) or IO space
  dBMBOUND  =  (EFI_PHYSICAL_ADDRESS)(buffer32&0xFF000000);

  buffer32  = MmPci32( 0, 0, 2, 0, 0x70);
  dGFXBase  =  (EFI_PHYSICAL_ADDRESS)(buffer32&0xFFF00000);

  //BSMMRRL
  //15:0 Lower Bound (SMMStart): These bits are compared with bits 35:20 of the incoming address 
  //To determine the lower 1MB aligned value of the protected range.
  MsgBus32Read(VLV_UNIT_BUNIT,BUNIT_BSMMRRL_OFFSET,buffer32);
  dTSegBase =  (EFI_PHYSICAL_ADDRESS)(LShiftU64((buffer32&0x0000FFFF),20));

  if (UpperTotalMemory > dBMBOUND) {
    RemapEnable=1;
  }


  //
  // Report the memory to EFI
  //
  
  // Report first 640KB of system memory
  BuildResourceDescriptorHob (            
    EFI_RESOURCE_SYSTEM_MEMORY,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)(0),
    (UINT64)(0xA0000)
    );

#if SMM_SUPPORT
  SmramHobDescriptorBlock = BuildGuidHob (
             &gEfiSmmPeiSmramMemoryReserveGuid,
             sizeof (EFI_SMRAM_HOB_DESCRIPTOR_BLOCK) + sizeof (EFI_SMRAM_DESCRIPTOR)
             );
  SmramHobDescriptorBlock->NumberOfSmmReservedRegions = (UINT32)(2);

  SmramHobDescriptorBlock->Descriptor[0].PhysicalStart = (EFI_PHYSICAL_ADDRESS)(0xA0000);
  SmramHobDescriptorBlock->Descriptor[0].CpuStart      = (EFI_PHYSICAL_ADDRESS)(0xA0000);
  SmramHobDescriptorBlock->Descriptor[0].PhysicalSize  = (UINT64)(0x20000);
  SmramHobDescriptorBlock->Descriptor[0].RegionState   = (UINT64)(EFI_SMRAM_CLOSED | EFI_CACHEABLE);
#endif

  // Report first 0A0000h - 0FFFFFh as RESERVED memory
  //Update Hob as reserved memory
  BuildResourceDescriptorHob(
    EFI_RESOURCE_MEMORY_RESERVED,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)(0xA0000),
    (UINT64)(0x60000)
    );

  if (dGFXBase) {
  //Update Hob as reserved memory
  // Report (TOLUD-GFXBase) to IGD share memory size as reserved memory
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)(dGFXBase),
    (UINT64)(dBMBOUND - dGFXBase)
    );
  }

  dTSegSize = (UINT32) (CurrentMrcData->OemMrcData.TsegSize) << 0x14;

  // Report (TOM-TSEG_SIZE) to TSEG_SIZE as reserved memory (SMRAM TSEG)
  if (dTSegSize) {
#if SMM_SUPPORT
    SmramHobDescriptorBlock->Descriptor[1].PhysicalStart = (EFI_PHYSICAL_ADDRESS)(dTSegBase);
    SmramHobDescriptorBlock->Descriptor[1].CpuStart      = (EFI_PHYSICAL_ADDRESS)(dTSegBase);
    //DEBUG ((EFI_D_INFO,"SmramHobDescriptorBlock->Descriptor[1].PhysicalStart = %X\n", SmramHobDescriptorBlock->Descriptor[1].PhysicalStart));
    //DEBUG ((EFI_D_INFO,"SmramHobDescriptorBlock->Descriptor[1].CpuStart = %X\n", SmramHobDescriptorBlock->Descriptor[1].CpuStart));
    //
    // To isolate SMRAM and IED 
    //
    if (PcdGetBool (PcdCpuIEDEnabled)) {
      IedSize = PcdGet32(PcdCpuIEDRamSize);
      SmramHobDescriptorBlock->Descriptor[1].PhysicalSize  = (UINT64)(dTSegSize - IedSize);
      //DEBUG ((EFI_D_INFO,"dTSegSize = %X\n", dTSegSize));
      //DEBUG ((EFI_D_INFO,"IedSize = %X\n", IedSize));
      //DEBUG ((EFI_D_INFO,"SmramHobDescriptorBlock->Descriptor[1].PhysicalSize = %X\n", SmramHobDescriptorBlock->Descriptor[1].PhysicalSize));
    }else{
      SmramHobDescriptorBlock->Descriptor[1].PhysicalSize  = (UINT64)(dTSegSize);
    }
    SmramHobDescriptorBlock->Descriptor[1].RegionState   = (UINT64)(EFI_SMRAM_CLOSED);
#endif
  //Update Hob as reserved memory
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)(dTSegBase),
    (UINT64)(dTSegSize)
    );
  }

  //dTSegBase is the lowest address of Reserved Memory
  ReservedMemoryBase = (UINT32)dTSegBase;
  ReservedMemorySize = (UINT32)(dBMBOUND - dTSegBase);

  AdjustedReservedMemorySize = ReservedMemorySize;
  AdjustedReservedMemoryBase = ReservedMemoryBase;

  //Change PeiMemory location for EFI-complaint Grub Bootloader, from Lakemorebase with length 64M
   dPeiMemBase = AdjustedReservedMemoryBase - PeiMemSize;
   if (dPeiMemBase <= 0x100000){
     dPeiMemBase = 0x100000;
   }

  Capsule = NULL;
  CapsuleBuffer = NULL;
  CapsuleBufferLength = 0;

  if (BootMode == BOOT_ON_FLASH_UPDATE) {
    Status = (*PeiServices)->LocatePpi (PeiServices, &gPeiCapsulePpiGuid, 0, NULL, (VOID **) &Capsule);
    ASSERT_EFI_ERROR (Status);

    if (Status == EFI_SUCCESS) {
      CapsuleBuffer = (VOID *) (UINTN) 0x100000;
      CapsuleBufferLength = (UINTN) (dPeiMemBase - (EFI_PHYSICAL_ADDRESS) (UINTN) CapsuleBuffer);

      //
      // Call the Capsule PPI Coalesce function to coalesce the capsule data.
      //
      Status = Capsule->Coalesce ((EFI_PEI_SERVICES **) PeiServices, &CapsuleBuffer, &CapsuleBufferLength);
    }
    //
    // If it failed, then NULL out our capsule PPI pointer so that the capsule
    // HOB does not get created below.
    //
    if (Status != EFI_SUCCESS) {
      Capsule = NULL;
    }
  }

  //
  // Report the memory to EFI
  //
  if (BootMode != BOOT_ON_S3_RESUME) {
    Status = (*PeiServices)->InstallPeiMemory(PeiServices, dPeiMemBase, PeiMemSize);

    if (Status != EFI_SUCCESS) {
      MRC_DEADLOOP();
    }
  }

  // Report 1MB to TOLUD - (TSEG size) - (IGD share memory size) as system memory
  // Update Hob as system memory
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)(0x100000),
    (UINT64)(AdjustedReservedMemoryBase - 0x100000)
    );

  if (RemapEnable) {
  // Report 4GB to dBMBOUNDHI as system memory
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    MEM_DET_COMMON_MEM_ATTR1,
    (EFI_PHYSICAL_ADDRESS)(0x100000000),
    (UINT64)(dBMBOUNDHI-0x100000000)
    );
  }

  if (Capsule != NULL) {
    Status = Capsule->CreateState ((EFI_PEI_SERVICES **) PeiServices, CapsuleBuffer, CapsuleBufferLength);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InstallS3Memory (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                  **PeiServices
#else
  IN CONST EFI_PEI_SERVICES            **PeiServices
#endif
  )
{
  EFI_STATUS                      Status = EFI_SUCCESS;
  UINTN                 VarSize;
  UINTN                 VarAttrib;
#ifdef ECP_FLAG
  PEI_READ_ONLY_VARIABLE_PPI      *ReadOnlyVariable;
  EFI_GUID                        gEfiAcpiVariableGuid = EFI_ACPI_VARIABLE_GUID;
#else
  EFI_PEI_READ_ONLY_VARIABLE2_PPI                        *ReadOnlyVariable;
#endif
  UINT64                AcpiVariableSet64;
#ifdef ECP_FLAG
  ACPI_VARIABLE_SET                   *AcpiVariableSet;
#else
  ACPI_VARIABLE_SET_COMPATIBILITY     *AcpiVariableSet;
#endif
  UINTN                           S3MemoryBase;
  UINTN                           S3MemorySize;
  DEBUG ((EFI_D_INFO, "InstallS3Memory()\n"));

  AcpiVariableSet = NULL;
  VarSize         = sizeof (AcpiVariableSet64);
  VarAttrib       = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;

  //
  // Obtain variable services
  //
  Status = (*PeiServices)->LocatePpi(
    PeiServices,
#ifdef ECP_FLAG
    &gPeiReadOnlyVariablePpiGuid,
#else
    &gEfiPeiReadOnlyVariable2PpiGuid,
#endif
    0,
    NULL,
    &ReadOnlyVariable
    );
  ASSERT_EFI_ERROR(Status);
#ifdef ECP_FLAG
  Status = ReadOnlyVariable->PeiGetVariable (
    PeiServices,
    ACPI_GLOBAL_VARIABLE,
    &gEfiAcpiVariableGuid,
    &VarAttrib,
    &VarSize,
    &AcpiVariableSet64
    );
  AcpiVariableSet = (ACPI_VARIABLE_SET *) (UINTN) AcpiVariableSet64;

#else
  Status = ReadOnlyVariable->GetVariable (
    ReadOnlyVariable,
    ACPI_GLOBAL_VARIABLE,
    &gEfiAcpiVariableGuid,
    &VarAttrib,
    &VarSize,
    &AcpiVariableSet64
    );

  AcpiVariableSet = (ACPI_VARIABLE_SET_COMPATIBILITY *) (UINTN) AcpiVariableSet64;
#endif

  if (EFI_ERROR (Status) || (AcpiVariableSet == NULL)) {
    return EFI_OUT_OF_RESOURCES;
  }

  S3MemoryBase = (UINTN)(AcpiVariableSet->AcpiReservedMemoryBase);
  S3MemorySize = (UINTN)(AcpiVariableSet->AcpiReservedMemorySize);

  // Report Memory to EFI
  Status = (*PeiServices)->InstallPeiMemory (PeiServices, S3MemoryBase, S3MemorySize);
  ASSERT_EFI_ERROR(Status);

  return Status;
}


EFI_STATUS
SetOemMrcData (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                  **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES            **PeiServices,
#endif
  MRC_PARAMETER_FRAME             *CurrentMrcData
  )
{
  EFI_STATUS                      Status;
  MRC_OEM_FRAME                   *OemMrcData=&CurrentMrcData->OemMrcData;
//  VLV_MMIO_POLICY_PPI		*VlvMmioPolicyPpi;
  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.>>
#ifdef ECP_FLAG
  PEI_READ_ONLY_VARIABLE_PPI           *ReadOnlyVariable;
#else
  EFI_PEI_READ_ONLY_VARIABLE2_PPI      *ReadOnlyVariable;
#endif
  EFI_GUID pAmiGlobalVariableGuid   = AMI_GLOBAL_VARIABLE_GUID; 
  EFI_GUID gEfiSetupGuid 		    = SETUP_GUID;
  CHAR16		gSetupVariable[]	= L"Setup";
  SETUP_DATA          		        SetupData;
  UINTN                             VariableSize;
  UINT32                            MemoryCeiling;
  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.<<
#if SMM_SUPPORT
  UINT8 Data8 = 0xFF;
#endif

  UINT32 DwordReg = 0;


#if SMM_SUPPORT

//  OemMrcData->TsegSize = 8;
  OemMrcData->TsegSize = TSEG_SIZE >> 20;  // AMI_OVERRIDE - Use token to select t-seg size.
  switch (OemMrcData->TsegSize) {
    case 1: // 1MB
      Data8 = 1;
      break;
    case 2: // 2MB
      Data8 = 2;
      break;
    case 4: // 4MB
      Data8 = 4;
      break;
    case 8: // 8MB
      Data8 = 8;
      break;
// AMI_OVERRIDE - Add 16Mb T-seg size option. - P052711A+ >>
    case 16: // 16MB
      Data8 = 16;
      break;
// AMI_OVERRIDE - Add 16Mb T-seg size option. - P052711A+ <<
    default: 
      // Invalid force 1M
      Data8 = 1;
      break;
  }
  CurrentMrcData->OemMrcData.TsegSize = Data8;
#endif
  // MMIO Size
  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.
//  Status = (**PeiServices).LocatePpi (
//                            PeiServices,
//                            &gVlvMmioPolicyPpiGuid,
//                            0,
//                            NULL,
//                            &VlvMmioPolicyPpi
//                            );
//  if(EFI_ERROR(Status)) {
//    OemMrcData->MmioSize = 0x800;
//  } else {
//    OemMrcData->MmioSize = VlvMmioPolicyPpi->MmioSize;
//  }
  OemMrcData->MmioSize = (UINT32) (0x1000 - (MEMORYCEILING_DEFAULT >> 20)); //AMI_OVERRIDE - EIP141999 Baytrail platform can't boot with discreted PCI express VGA card.
  Status = (*PeiServices)->LocatePpi(
    PeiServices,
  #ifdef ECP_FLAG
    &gPeiReadOnlyVariablePpiGuid,
  #else
    &gEfiPeiReadOnlyVariable2PpiGuid,
  #endif
    0,
    NULL,
    &ReadOnlyVariable
    );
  VariableSize = sizeof(SETUP_DATA);
  Status = ReadOnlyVariable->GetVariable(ReadOnlyVariable,
		  	  	  	  	  	  	 gSetupVariable,
                                 &gEfiSetupGuid,
                                 NULL,
                                 &VariableSize,
                                 &SetupData);
  if(!EFI_ERROR(Status)) { 
	switch (SetupData.MaxTolud)
	{
      case 0: //DYNAMIC
    	  //
    	  // Get the memory ceiling
    	  //
    	  VariableSize = sizeof(MemoryCeiling);
    	  Status = ReadOnlyVariable->GetVariable(ReadOnlyVariable,
    		  	  	  	  	  	  	 	 	 	 MemoryCeilingVariable,
    		  	  	  	  	  	  	 	 	 	 &pAmiGlobalVariableGuid,
    		  	  	  	  	  	  	 	 	 	 NULL,
    		  	  	  	  	  	  	 	 	 	 &VariableSize,
    		  	  	  	  	  	  	 	 	 	 &MemoryCeiling);
    	  if(!EFI_ERROR(Status)) {
    		  if(MemoryCeiling >= 0xE0000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_3_5G;
    		  } else if(MemoryCeiling >= 0xD0000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_3_25G;
    		  } else if(MemoryCeiling >= 0xC0000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_3G;
    		  } else if(MemoryCeiling >= 0xB0000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_2_75G;
    		  } else if(MemoryCeiling >= 0xA0000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_2_5G;
    		  } else if(MemoryCeiling >= 0x90000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_2_25G;
    		  } else if(MemoryCeiling >= 0x80000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_2G;
    		  } else if(MemoryCeiling >= 0x70000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_1_75G; 
    		  } else if(MemoryCeiling >= 0x60000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_1_5G;
    		  } else if(MemoryCeiling >= 0x50000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_1_25G;
    		  } else if(MemoryCeiling >= 0x40000000) {
    			  OemMrcData->MmioSize = NB_MAX_TOLUD_1G;
    		  } 
    	  }
    	  break;
  	  case 1: OemMrcData->MmioSize = NB_MAX_TOLUD_1G; break; //MAX_TOLUD_1G
  	  case 2: OemMrcData->MmioSize = NB_MAX_TOLUD_1_25G; break; //MAX_TOLUD_1.25G
  	  case 3: OemMrcData->MmioSize = NB_MAX_TOLUD_1_5G; break; //MAX_TOLUD_1.5G
  	  case 4: OemMrcData->MmioSize = NB_MAX_TOLUD_1_75G; break; //MAX_TOLUD_1.75G
  	  case 5: OemMrcData->MmioSize = NB_MAX_TOLUD_2G; break; //MAX_TOLUD_2G
  	  case 6: OemMrcData->MmioSize = NB_MAX_TOLUD_2_25G; break; //MAX_TOLUD_2.25G
  	  case 7: OemMrcData->MmioSize = NB_MAX_TOLUD_2_5G; break; //MAX_TOLUD_2.5G
  	  case 8: OemMrcData->MmioSize = NB_MAX_TOLUD_2_75G; break; //MAX_TOLUD_2.75G
  	  case 9: OemMrcData->MmioSize = NB_MAX_TOLUD_3G; break; //MAX_TOLUD_3G
  	  case 10:  OemMrcData->MmioSize = NB_MAX_TOLUD_3_25G; break; //MAX_TOLUD_3.25G
  	  case 11:  OemMrcData->MmioSize = NB_MAX_TOLUD_3_5G; break; //MAX_TOLUD_3.5G
  	  default:  break;
	}
	if(OemMrcData->MmioSize < 0x400) OemMrcData->MmioSize = 0x400; 
  }
  //AMI_OVERRIDE - EIP126704  Select dynamic or fixed mmio size.<<
  OemMrcData->SPDAddressTable[0][0] = 0xA0;
  OemMrcData->SPDAddressTable[1][0] = 0xA2;
  
  if ( MmioRead8 ( MmPciAddress (0, 0, 31, 0, 0x08)) > 3){                      // Anything above B1
    MsgBus32Read(0x04, 0x06, DwordReg);
    if (( (DwordReg & 0x80000000) == 0) && ((DwordReg & 0x40000000) == 0 )){    // Check bit 31 == 0 and bit 30 == 0, i.e 2CH support
        OemMrcData->SPDAddressTable[1][0] = 0xA2;
    }
    else {
        OemMrcData->SPDAddressTable[1][0] = 0x0;
    }
    DEBUG ((EFI_D_INFO, "Reg_EFF_DualCH_EN = 0x%x.\n",DwordReg));
  }

  OemMrcData->MrcConfigMemProgressCodeBase = MRC_MEMORY_CONFIG_PC_BASE;
  //
  // Restore MRC Parameters
  //
  Status = MrcParamsRestore (
    PeiServices,
    CurrentMrcData
  );

  return EFI_SUCCESS;
}

EFI_STATUS
MrcParamsRestore (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                 **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES           **PeiServices,
#endif
  MRC_PARAMETER_FRAME             *CurrentMrcData
  )
{
  EFI_STATUS                      Status;
#ifdef ECP_FLAG
  PEI_READ_ONLY_VARIABLE_PPI           *ReadOnlyVariable;
#else
  EFI_PEI_READ_ONLY_VARIABLE2_PPI      *ReadOnlyVariable;
#endif
  MRC_PARAMS_SAVE_RESTORE         MrcParamsHob;
  UINTN                           BufferSize;

  BufferSize = sizeof (MRC_PARAMS_SAVE_RESTORE);
  BufferSize = (BufferSize + 16) & (~0x3);

  //DEBUG ((EFI_D_INFO, "Buffer size brefore add %x, after adjust %x.\n", sizeof (MRC_PARAMS_SAVE_RESTORE), BufferSize));

  //
  // Obtain variable services
  //
#ifdef ECP_FLAG
    Status = (*PeiServices)->LocatePpi(
    PeiServices,
    &gPeiReadOnlyVariablePpiGuid,
    0,
    NULL,
    &ReadOnlyVariable
  );
#else
  Status = (*PeiServices)->LocatePpi(
    PeiServices,
    &gEfiPeiReadOnlyVariable2PpiGuid,
    0,
    NULL,
    &ReadOnlyVariable
  );
#endif
  ASSERT_EFI_ERROR(Status);

  // Get MRC Parameters
#ifdef ECP_FLAG
  Status = ReadOnlyVariable->PeiGetVariable (
             PeiServices,
             EfiMemoryConfigVariable,
             &gEfiVlv2VariableGuid,
             NULL,
             &BufferSize,
             &MrcParamsHob
             );
#else
  Status = ReadOnlyVariable->GetVariable (
             ReadOnlyVariable,
             EfiMemoryConfigVariable,
             &gEfiVlv2VariableGuid,
             NULL,
             &BufferSize,
             &MrcParamsHob
             );
#endif
  
  if (EFI_ERROR (Status)) {
    //DEBUG ((EFI_D_INFO, "MRC Parameters not valid. status is %x\n", Status));
    return Status;
  } else {
#ifdef ECP_FLAG
      CopyMem (&(CurrentMrcData->OemMrcData),
                               &MrcParamsHob,
                               sizeof (MRC_PARAMS_SAVE_RESTORE));
#else
    (*PeiServices)->CopyMem (&(CurrentMrcData->OemMrcData),
                             &MrcParamsHob,
                             sizeof (MRC_PARAMS_SAVE_RESTORE));
#endif
    CurrentMrcData->OemMrcData.MrcParamsValidFlag = 1;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
MrcParamsSave (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES                 **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES           **PeiServices,
#endif
  MRC_PARAMETER_FRAME             *CurrentMrcData
  )
/*++

Routine Description:

  This function saves the MRC Parameters to Hob for later use.

Arguments:

  PeiServices:     PEI Services Table.
  CurrentMrcData:  Pointer to MRC Output Data that contains MRC Parameters

Returns:
  EFI_SUCCESS      - Hob is successfully built.
  Others           - Errors occur while creating new Hob

--*/
{
  MRC_PARAMS_SAVE_RESTORE         *MrcParamsHob;
  UINTN                           BufferSize;

  BufferSize = sizeof (MRC_PARAMS_SAVE_RESTORE);

  MrcParamsHob = BuildGuidHob (&gEfiMemoryConfigDataGuid, BufferSize);

#ifdef ECP_FLAG
  SetMem ((VOID *) MrcParamsHob, BufferSize, 0);
  CopyMem (MrcParamsHob, &(CurrentMrcData->OemMrcData), sizeof(MRC_PARAMS_SAVE_RESTORE));

#else
  (*PeiServices)->SetMem ((VOID *) MrcParamsHob, BufferSize, 0);

  (*PeiServices)->CopyMem (MrcParamsHob, &(CurrentMrcData->OemMrcData), sizeof(MRC_PARAMS_SAVE_RESTORE));
#endif
  return EFI_SUCCESS;
}

