/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/*++
Module Name:

  SdHostDriver.c

Abstract:

  UEFI Driver Entry and support.

--*/

#include "SdHostDriver.h"
#include "PchRegs.h"
#include "PchAccess.h"
#include "PlatformBaseAddresses.h"

extern UINT32 gMMIOAddress;
extern   PCH_STEPPING EFIAPI PchStepping (VOID);
UINT32   EmmcDevNumber;
BOOLEAN  LpsseMMC45Enabled ;

//
// MMCSDIOController Driver Global Variables
//
STATIC PEI_SD_CONTROLLER_PPI mSdControllerPpi = {
  EFI_SD_HOST_IO_PROTOCOL_REVISION_01,
  {
    0, // HighSpeedSupport
    0, // V18Support
    0, // V30Support
    0, // V33Support
    0, // Reserved0
    0, // BusWidth4
    0, // BusWidth8
    0, // Reserved1
    0,
    0,
    0,
    0,
    (512 * 1024) //BoundarySize

  },
  HostSendCommand,
  SetClockFrequency,
  SetBusWidth,
  SetHostVoltage,
  SetHostDdrMode,
  ResetSdHost,
  EnableAutoStopCmd,
  DetectCardAndInitHost,
  SetBlockLength,
  SetupDevice,
};

typedef struct {
  UINT8 portid;
  UINT16 RegOff;
  UINT32 mask;
  UINT32 value;
} SCCCONFIG;

SCCCONFIG SCCData[] = {

  //
  // 1. Configure Master DLL
  //

  //
  // C,F Init
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_INIT_SCORE_MDL_CF_INIT,
    0,
    0x78000
  },

  //
  // Configure Swing,FSM for Master DLL
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_CTRL_SCORE_MDL_FSM_CTRL,
    0,
    0x133
  },

  //
  // Run+Local Reset on Master DLL
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_CTRL_SCORE_MDL_FSM_CTRL,
    0,
    0x1933
  },

  //
  // 3. Override Slave Path
  //0x4950
  //  [19] = 1
  //  [18:15] = F
  //  [14:0] = 0FFF
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH_SCORE_MDL_WRITE_PATH_C_F_ADDR,
    (UINT32)(0xFFFFFFFF << 20),
    (UINT32)((1<<19) | (0 & MDL_FSM_VALS_MASK))
  },

  //
  // 4.Configure Write Path
  // 0x4954
  //   [14:10] = 10011
  //   [9:5]   = 10011
  //   [4:0]   = 10011
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH1_MUX_SCORE_DLL_WRITE_PATH1_MUX,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x11 << 10) | (0xF << 5) | 0x13)
  },

  //
  // 0x4958
  //   [14:10] = 10011
  //   [9:5]   = 10011
  //   [4:0]   = 10011
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH2_MUX_SCORE_DLL_WRITE_PATH2_MUX,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x11 << 10) | (0xF << 5) | 0x13)
  },

  //
  // 0x495C
  //   [14:10] = 10011
  //   [9:5]   = 10011
  //   [4:0]   = 10011
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH3_MUX_SCORE_DLL_WRITE_PATH3_MUX,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x11 << 10) | (0xF << 5) | 0x13)
  },

  //
  // 5.Configure Read Path
  //
  // 0x43E4
  //   [14:10] = 0x4
  //   [9:5]   = 0x2
  //   [4:0]   = 0x5
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    SDMMC1_CLK_PCONF1,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x4 << 10) | (0x2 << 5) | 0x5)
  },

  //
  // 0x4324
  //   [14:10] = 0x3
  //   [9:5]   = 0x3
  //   [4:0]   = 0x5
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    SDMMC2_CLK_PCONF1,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x3 << 10) | (0x3 << 5) | 0x5)
  },

  //
  // 0x42B4
  //   [14:10] = 0x4
  //   [9:5]   = 0x2
  //   [4:0]   = 0x5
  //
  {
    CFIO_SCORE_SB_PORT_ID,
    SDMMC3_CLK_PCONF1,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x4 << 10) | (0x2 << 5) | 0x5)
  },
  //
  // Enable IOSF Snoop
  //
  {
    PCH_SCC_EP_PORT_ID,
    R_PCH_SCC_EP_IOSFCTL,
    0xFFFFFF7F,
    (B_PCH_SCC_EP_IOSFCTL_NSNPDIS)
  }
};

STATIC EFI_PEI_PPI_DESCRIPTOR mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiSdhcPpiGuid,
  NULL
};

extern PEI_SDHOST_DATA  gSdHostData;

EFI_STATUS
EnableSdhcController (
  IN PEI_SDHOST_DATA          *PeiSdhostData,
  IN UINT8                    SdControllerId
  );

EFI_STATUS
EFIAPI
SdHostDriverEntryPoint (
#ifdef ECP_FLAG
  IN  EFI_FFS_FILE_HEADER    *FileHandle,
  IN  EFI_PEI_SERVICES       **PeiServices
#else
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
#endif
  )
/*++

Routine Description:

  Entry point for EFI drivers.

Arguments:

  ImageHandle - EFI_HANDLE
  SystemTable - EFI_SYSTEM_TABLE

Returns:

  EFI_SUCCESS         Success
  EFI_DEVICE_ERROR    Fail

--*/
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    AllocateAddress;
  PEI_SDHOST_DATA         *PeiSdhostData;
#ifdef ECP_FLAG
  PEI_PCI_CFG_PPI         *PciCfgPpi;
#else
  EFI_PEI_PCI_CFG2_PPI    *PciCfgPpi;
#endif
  UINT32                   Data;
  UINT32                Buffer32 = 0;
  UINT32                MDL_FSM_VALS = 0;
  UINT8                  Index;

// Silicon Steppings

// A0/A1: eMMC 4.41
// B0/B1+: eMMC 4.5

  switch (PchStepping()) {
    case PchA0:
    case PchA1:
      EmmcDevNumber = 16;
      LpsseMMC45Enabled = 0;
      DEBUG ((EFI_D_INFO, " SOC A0/A1: eMMC 4.41 EmmcDevNumber = %0d\n", EmmcDevNumber));
      break;
    case PchB0:
    default:
      EmmcDevNumber = 23;
      LpsseMMC45Enabled = 1;
      DEBUG ((EFI_D_INFO, " SOC B0 and later: eMMC 4.5 EmmcDevNumber = %0d\n", EmmcDevNumber));
      break;
  }
  //---------------------------------------------
  /*
  20.1.1  EMMC
  SDMMC1_CLK -         write 0x2003ED01 to IOBASE + 0x03E0
  SDMMC1_CMD -        write 0x2003EC81 to IOBASE + 0x0390
  SDMMC1_D0 -           write 0x2003EC81 to IOBASE + 0x03D0
  SDMMC1_D1 -           write 0x2003EC81 to IOBASE + 0x0400
  SDMMC1_D2 -           write 0x2003EC81 to IOBASE + 0x03B0
  SDMMC1_D3_CD_B - write 0x2003EC81 to IOBASE + 0x0360
  MMC1_D4_SD_WE -   write 0x2003EC81 to IOBASE + 0x0380
  MMC1_D5 -                write 0x2003EC81 to IOBASE + 0x03C0
  MMC1_D6 -                write 0x2003EC81 to IOBASE + 0x0370
  MMC1_D7 -                write 0x2003EC81 to IOBASE + 0x03F0
  MMC1_RESET_B -       write 0x2003ED01 to IOBASE + 0x0330
  */
  if(LpsseMMC45Enabled == 0x0) {
    MmioWrite32 (IO_BASE_ADDRESS + 0x03E0, 0x2003ED01); //EMMC 4.41
    MmioWrite32 (IO_BASE_ADDRESS + 0x0390, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03D0, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0400, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03B0, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0360, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0380, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03C0, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0370, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03F0, 0x2003EC81);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0330, 0x2003ED01);
  }

  if(LpsseMMC45Enabled == 0x1) {
    MmioWrite32 (IO_BASE_ADDRESS + 0x03E0, 0x2003ED03); // EMMC 4.5
    MmioWrite32 (IO_BASE_ADDRESS + 0x0390, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03D0, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0400, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03B0, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0360, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0380, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03C0, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0370, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x03F0, 0x2003EC83);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0330, 0x2003ED03);
  }
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "eMMC DLL Settings for ALPV & BBAY .\n"));


  for (Index=0; Index<sizeof (SCCData)/sizeof(SCCCONFIG); Index++) {

    if (Index==3) {
      //
      // 2. Populate Values from DLL
      //
      PchMsgBusRead32 (
        CFIO_SCORE_SB_PORT_ID,
        DLL_VALS_SCORE_MDL_FSM_VALS,
        MDL_FSM_VALS,
        PCH_SCC_EP_PRIVATE_READ_OPCODE,
        PCH_SCC_EP_PRIVATE_WRITE_OPCODE
        );
      SCCData[Index].value |= (UINT32)(MDL_FSM_VALS & MDL_FSM_VALS_MASK);
    }

    PchMsgBusAndThenOr32(
      SCCData[Index].portid,
      SCCData[Index].RegOff,
      Buffer32,
      SCCData[Index].mask,
      SCCData[Index].value,
      PCH_SCC_EP_PRIVATE_READ_OPCODE,
      PCH_SCC_EP_PRIVATE_WRITE_OPCODE
      )

  }


#ifdef ECP_FLAG
  Status = ((**PeiServices).AllocatePages) (
                              PeiServices,
                              EfiLoaderData,
                              1,
                              &AllocateAddress
                              );
#else
  Status = (**PeiServices).AllocatePages (
                             PeiServices,
                             EfiLoaderData,
                             1,
                             &AllocateAddress
                             );
#endif
  ASSERT_EFI_ERROR (Status);

  PciCfgPpi = (**PeiServices).PciCfg;

  PeiSdhostData = (PEI_SDHOST_DATA *)((UINTN)AllocateAddress);
  ZeroMem (PeiSdhostData, sizeof(PEI_SDHOST_DATA));

  PeiSdhostData->Signature            = PEI_SDHOST_DATA_SIGNATURE;
  CopyMem(&(PeiSdhostData->SdControllerPpi), &mSdControllerPpi, sizeof(PEI_SD_CONTROLLER_PPI));
  CopyMem(&(PeiSdhostData->PpiList), &mPpiList, sizeof(mPpiList));
  PeiSdhostData->PpiList.Ppi          = &PeiSdhostData->SdControllerPpi;
  PeiSdhostData->PciCfgPpi            = PciCfgPpi;

  PeiSdhostData->BaseAddress = PCI_LIB_ADDRESS (0, EmmcDevNumber, 0, 0);
  Status = EnableSdhcController (PeiSdhostData, 0);

  PeiSdhostData->SdControllerPpi.ResetSdHost (&PeiSdhostData->SdControllerPpi, Reset_All);
  Data = *(UINT32 *)(gMMIOAddress+MMIO_CAP);

  DEBUG ((EFI_D_INFO, " MMIO_CAP = 0x%08x\n", Data));

  if ((Data & BIT18) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.BusWidth8 = TRUE;
    DEBUG ((EFI_D_INFO, " BusWidth8\n"));
  }

  if ((Data & BIT21) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.HighSpeedSupport = TRUE;
    DEBUG ((EFI_D_INFO, " HighSpeedSupport\n"));
  }

  if ((Data & BIT24) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.V33Support = TRUE;
    DEBUG ((EFI_D_INFO, " V33Support\n"));
  }

  if ((Data & BIT25) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.V30Support = TRUE;
    DEBUG ((EFI_D_INFO, " V30Support\n"));
  }

  if ((Data & BIT26) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.V18Support = TRUE;
    DEBUG ((EFI_D_INFO, " V18Support\n"));
  }

  PeiSdhostData->SdControllerPpi.HostCapability.BusWidth4 = TRUE;
  DEBUG ((EFI_D_INFO, " BusWidth4\n"));

  PeiSdhostData->BaseClockInMHz = (Data >> 8) & 0xFF;
  PeiSdhostData->BlockLength    = BLOCK_SIZE;
  PeiSdhostData->IsAutoStopCmd  = TRUE;
  PeiSdhostData->SdControllerPpi.SetHostVoltage (&PeiSdhostData->SdControllerPpi, 0);
  DEBUG ((EFI_D_INFO, " Copy to gSdHostData\n"));
  CopyMem(&gSdHostData, PeiSdhostData, sizeof(PEI_SDHOST_DATA));
  ASSERT_EFI_ERROR (Status);


  //
  // Install SD Controller PPI
  //
  Status = (**PeiServices).InstallPpi (
                             PeiServices,
                             &PeiSdhostData->PpiList
                             );
  ASSERT_EFI_ERROR (Status);



  return Status;
}

EFI_STATUS
EnableSdhcController (
  IN PEI_SDHOST_DATA          *PeiSdhostData,
  IN UINT8                    SdControllerId
  )
{
#ifdef ECP_FLAG
  PEI_PCI_CFG_PPI          *PciCfgPpi;
#else
  EFI_PEI_PCI_CFG2_PPI     *PciCfgPpi;
#endif
  PCI_CLASSC               PciClass;
  UINT32                   VidDid;
  UINT8                    Data8;
  UINTN                    BaseAddress;
  CONST EFI_PEI_SERVICES   **PeiServices;

  PeiServices = GetPeiServicesTablePointer ();

  PciCfgPpi   = PeiSdhostData->PciCfgPpi;
  BaseAddress = PeiSdhostData->BaseAddress;

  VidDid = MmPci32(0,0,EmmcDevNumber,0,PCI_VENDOR_ID_OFFSET);
  DEBUG ((EFI_D_INFO, "SdHostDriver : VidDid = 0x%08x\n", VidDid));
  PeiSdhostData->PciVid = (UINT16)(VidDid & 0xffff);
  PeiSdhostData->PciDid = (UINT16)(VidDid >> 16);
  //PeiSdhostData->EnableVerboseDebug = TRUE;  // Enable verbose message
  PeiSdhostData->IsEmmc = FALSE;

  PciClass.BaseCode = MmPci8(0,0,EmmcDevNumber,0,PCI_CLASSCODE_OFFSET+2);
  PciClass.SubClassCode = MmPci8(0,0,EmmcDevNumber,0,PCI_CLASSCODE_OFFSET+1);
  PciClass.PI = MmPci8(0,0,EmmcDevNumber,0,PCI_CLASSCODE_OFFSET);

  if ((PciClass.BaseCode != PCI_CLASS_SYSTEM_PERIPHERAL) ||
      (PciClass.SubClassCode != PCI_SUBCLASS_SD_HOST_CONTROLLER) ||
      ((PciClass.PI != PCI_IF_STANDARD_HOST_NO_DMA) && (PciClass.PI != PCI_IF_STANDARD_HOST_SUPPORT_DMA))
     ) {
    return  EFI_UNSUPPORTED;
  }

  //
  // Enable SDHC
  //
  Data8 = MmPci8(0,0,EmmcDevNumber,0,PCI_COMMAND_OFFSET);

  //
  // Enable STATUSCOMMAND BME & MSE
  // Bit 1 : MSE   ;  Bit 2 : BME
  //
  Data8 |= 0x06;

  MmPci8(0,0,EmmcDevNumber,0,PCI_COMMAND_OFFSET) =   Data8;
  MmPci32(0,0,EmmcDevNumber,0,PCI_BASE_ADDRESSREG_OFFSET) = gMMIOAddress;

  return EFI_SUCCESS;
}


