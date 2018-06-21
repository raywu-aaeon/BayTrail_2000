/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2010 -2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCUma.c

Abstract:

  Framework PEIM to SeCUma

--*/

#include <SeCUma.h>
#ifdef ECP_FLAG
EFI_GUID gSeCUmaPpiGuid = SEC_UMA_PPI_GUID;
#endif

//#include EFI_PPI_CONSUMER (Wdt)

//#define SEC_FPT_BIT_TEST  // this bit define when FTP bit need to test priore to UMA allocation.
//#define SEC_SIMIC_TEST
//#define HECI_INIT_PEI
//#define WAIT4FWSTS
//
// Function Declarations
//
static SEC_UMA_PPI         mSeCUmaPpi = {
  SeCSendUmaSize,
  SeCConfigDidReg,
  HandleSeCBiosAction
};

static EFI_PEI_PPI_DESCRIPTOR mSeCUmaPpiList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gSeCUmaPpiGuid,
    &mSeCUmaPpi
  }
};

UINT32
SeCSendUmaSize (
  IN EFI_PEI_SERVICES **PeiServices
  )
/*++

Routine Description:

  This procedure will read and return the amount of SeC UMA requested
  by SeC ROM from the HECI device.

Arguments:

  PeiServices   - General purpose services available to every PEIM.

Returns:

  Return SeC UMA Size

--*/
{


#ifndef SEC_SIMIC_TEST
  UINT32  SeCUmaSize;
  UINT32  SeCMemReq;
  UINT32  SeCDeviceExpose;
  UINT32  SeCFwStatus;
  UINT32  SeC_Exclusion_req;
  UINT32  SeC_Exclusion_cause;
  UINT32 SeCDisable;


#ifdef SEC_TEST
  PEI_STALL_PPI                     *StallPpi;
  EFI_STATUS                        Status;
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gEfiPeiStallPpiGuid,
                             0,
                             NULL,
                             (VOID **) &StallPpi
                             );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "\n Read and Return the SeC UMA amount \n"));

#endif

#ifndef PCIE_PORT_NO
  //
  // Work around bus numbers so that PCIe slots are hidden
  //
  PciOr32 (PCI_LIB_ADDRESS (0, 0x1c, 0, 0x18), 0x00010100);
  PciOr32 (PCI_LIB_ADDRESS (0, 0x1c, 1, 0x18), 0x00020200);
  PciOr32 (PCI_LIB_ADDRESS (0, 0x1c, 2, 0x18), 0x00030300);
  PciOr32 (PCI_LIB_ADDRESS (0, 0x1c, 3, 0x18), 0x00040400);

#endif
  //
  // Exposure of SeC device (Device ID in the range of 0x0F18 ?0x0F1B) at BDF B0/D26/F0.
  //
  SeCDeviceExpose = isSeCExpose(PeiServices);

  if(SeCDeviceExpose == 0x00) {
    // if no SeC device discover
    DEBUG ((EFI_D_INFO, "Error: SeC device not present or Device ID wrong\n"));
    // If bit[4] == 1 and bits [3:0] have value of 0000b, 0100b, 0101b, or 0110b, then the BIOS
    // takes the SeC Error BIOS path. Refer to Section ?3.3.5.1
    // follow error path
    // 3.3.5.3. SeC Disable BIOS Path
    // The exact path depends on the value of SEC_FW_STS0.SEC_CURR_OPER_MODE.
    // In case of "Debug Mode":
    // Task Description
    // HECI Messages    The BIOS does not send any HECI messages.
    // Other Messages   BIOS sends only the DRAM Init Done register-based message.
    // SeC Device   Hide SeC device before OS boot. It means that SeC IPC driver is not loaded in OS environment.
    SeCDisable = Mmio32 ((UINTN) PMC_BASE_ADDRESS, R_PCH_PMC_FUNC_DIS);
    SeCDisable &= (~B_PCH_PMC_FUNC_DIS_SEC);

    MmioAnd32 ((UINTN) (PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS), (UINT32) (SeCDisable));
    return 0;
  }

  //
  //
  // Check the exclusion bit8 of PMC's register 0xC
  //
  SeC_Exclusion_req = Mmio32(PMC_BASE_ADDRESS, 0xc);
  if((SeC_Exclusion_req & B_EXCLUSION) == B_EXCLUSION) {
    DEBUG ((EFI_D_INFO, "Error: SeC exclusion error \n"));
    SeC_Exclusion_cause = Mmio32(PMC_BASE_ADDRESS, 0x2c);
    // Follow error path.
    // 3.3.5.1. SeC Error BIOS Path
    switch(SeC_Exclusion_cause & 0x0f) {
        // B0:D0:F0 register 0x2C (names TBD):
        // If bit[4] == 1 and bits [3:0] have value of 0000b, 0100b, 0101b, or 0110b,
        // then the BIOS takes the SeC Error BIOS path. Refer to Section ?3.3.5.1?

      case 0x0:
      case 0x4:
      case 0x5:
      case 0x6:
        DEBUG ((EFI_D_INFO, "Error: No HECI command send \n"));
        return 0;
      default:
        break;
    }
  }

  //Fix WHCK Bug
  HeciPciWrite32 (0x80, 0x4803A001);

  // Write Heci Base Address and enable Heci device
  //
  // 3.3.1. SeC Device Initializaiton:
  // System BIOS must program the SeC registers as using the following sequence.
  // BIOS should initialize SeC Interface prior to the system memory initialization.
  //
  HeciPciWrite32 (HECI_BAR0, HECI_BASE_ADDRESS);  // BAR0  Dev 26 Func 0    10h bits 31:0
  HeciPciWrite32 (HECI_BAR1, HECI2_BASE_ADDRESS); // BAR1  Dev 26 Func 0    14h bits 31:0
  //
  // Memory space enable: Set to enable SeC memory mapped register space.
  // Bus Master Enable: Set to enable SeC bus master functionality.
  // SERR Enable: Set to enable SERR capability..
  // bit 8 for SERR enable

  HeciPciWrite32 (R_COMMAND, 0x106);             // Enable BAR Dev 26 Func 0    04h bits 8, 2:1
  //
  // PAVP enabling code moved to GraphicsDxeInit
  //
  // PciOr32 (PCI_LIB_ADDRESS (0, 2, 0, 0x74), 0x00000007);

  SeCMemReq  = HeciPciRead32 (R_SEC_MEM_REQ);
  SeCUmaSize   = 0x0;

  //
  // Poll on UMASZV until it indicates a valid size is present.
  //

  /*
    BIOS must then poll on the SEC_MEM_REQ register until either the MEM_REQ_VALID
        bit is set to 1 or the MEM_REQ_IN: bit is set to 1. No timeout should be
        applied in this waiting loop because SeC ROM guarantees either of these bits to be set.
  */

  while (((SeCMemReq & B_SEC_MEM_REQ_VALID) != B_SEC_MEM_REQ_VALID) && ((SeCMemReq & B_SEC_MEM_REQ_INVALID)) != B_SEC_MEM_REQ_INVALID) {
    SeCMemReq = HeciPciRead32 (R_SEC_MEM_REQ);
  }

  //
  // Check for B_SEC_MEM_REQ_VALID and B_SEC_MEM_REQ_INVALID
  //
  /*
    This bit is set when the firmware discovers a bad checksum of Flash
    Partition Table (FPT). When this bit set, it may or may not have the error code shown
    in bit [15:12]. The system can get this bit clear by flashing the proper firmware
    image again.
  */

  SeCFwStatus = (HeciPciRead32 (R_SEC_FW_STS0));
  if( ((SeCMemReq & B_SEC_MEM_REQ_VALID) == B_SEC_MEM_REQ_VALID)) {
    SeCUmaSize = HeciPciRead32 (R_SEC_MEM_REQ) & S_SEC_UMA_SIZE_MASK;
    DEBUG ((EFI_D_INFO, "SeC UMA Size Requested: %d KB\n", SeCUmaSize));
  } else if((SeCMemReq & B_SEC_MEM_REQ_INVALID) == B_SEC_MEM_REQ_INVALID) {
    DEBUG ((EFI_D_INFO, "Error: SeC UMA Size Request invalid\n"));
  } else {
    DEBUG ((EFI_D_INFO, "Error: SeC not working properly \n"));
  }

  //
  // Return SeCUmaSize value
  //
  SeCUmaSize = SeCUmaSize / 1024;
  if(SeCUmaSize >= 32) SeCUmaSize = 31;
  return SeCUmaSize;

#endif // SEC_TEST


}

EFI_STATUS
SeCConfigDidReg (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  MRC_BOOT_MODE_T           MrcBootMode,
  UINT8                     InitStat,
  UINT32                    SeCUmaBase,
  UINT32                    SeCUmaSize
  )
/*++

Routine Description:

  This procedure will configure the SEC Host General Status register,
  indicating that DRAM Initialization is complete and SeC FW may
  begin using the allocated SeC UMA space.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  MrcBootMode - MRC BootMode
  InitStat    - H_GS[27:24] Status
  SeCUmaBase  - LSB of base address
  SeCUmaBaseEx - MSB of base address
  SeCUmaSIze -  Allocated size of UMA

Returns:
  EFI_SUCCESS

--*/
{
  UINT32                      UmaBase;      // remove
  UINT32                      UmaBaseExt;
  UINT32                      DidAck;
  UINT32                      SeCHfs;
  UINT32                      SeC_DID_MSG;
  UINT32                      Timeout;
  UINT8                       BiosAction;
  EFI_STATUS                  Status;
  EFI_PEI_STALL_PPI           *StallPpi;
  UINT32                      SeCMemReq;
#if CF9_REG_LOCK_ENABLE    
  HECI_FWS_REGISTER           SeCFirmwareStatus;
  UINTN                       Address;
#endif

  Status = (*PeiServices)->LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  ASSERT_EFI_ERROR (Status);

  //
  //    PAVP enabling moved to GraphicsDxeInit
  //    B:0 D:2 F:0 offset 0x74
  //
  //  PciOr32 (PCI_LIB_ADDRESS (0, 2, 0, 0x74), 0x00000007);

  //
  // Write Heci Base Address and enable Heci device
  //
  // 3.3.1. SeC Device Initializaiton:
  // System BIOS must program the SeC registers as using the following sequence.
  // BIOS should initialize SeC Interface prior to the system memory initialization.
  //
  HeciPciWrite32 (HECI_BAR0, HECI_BASE_ADDRESS);  // BAR0  Dev 26 Func 0    10h bits 31:0
  HeciPciWrite32 (HECI_BAR1, HECI2_BASE_ADDRESS); // BAR1  Dev 26 Func 0    14h bits 31:0
  //
  // Memory space enable: Set to enable SeC memory mapped register space.
  // Bus Master Enable: Set to enable SeC bus master functionality.
  // SERR Enable: Set to enable SERR capability..
  // bit 8 for SERR enable

  HeciPciWrite32 (R_COMMAND, 0x106);             // Enable BAR Dev 26 Func 0    04h bits 8, 2:1

  SeC_DID_MSG   = 0x0;
  Timeout = 0x0;

  DEBUG ((EFI_D_INFO, "Entered SeC DRAM Init Done procedure.\n"));
  if (SeCUmaSize == 0) return EFI_UNSUPPORTED;
  //EFI_DEADLOOP ();
  //
  // Read MESEGBASE
  //
  // SeCUmaBase     = PciRead32 (PCI_LIB_ADDRESS (0, 0, 0, R_MESEG_BASE));       // remove
  // SeCUmaBaseExt  = PciRead32 (PCI_LIB_ADDRESS (0, 0, 0, R_MESEG_BASE + 0x04)); // remove

  // ToDO: wait for DE reply .... use size in

  // 1) Program register field SATT1_BRG_BA_LSB.BRG_BA.
  UmaBase = SeCUmaBase << 20;
  HeciPciWrite32 (R_SATT1_BRG_BA_LSB, UmaBase);


  // 2) Program register field SATT1_CTRL.BRG_BA_MSB.
  UmaBaseExt = (((SeCUmaBase >> (32 - 20)) & 0x0f) << 8);
  HeciPciOr32 (R_SATT1_CTRL, UmaBaseExt); // bit 8:11 is MSB for IOSF address

  // 3) Program register field SATT1_SAP_SIZE.SAP_SIZE.
  HeciPciWrite32 (R_SATT1_SAP_SIZE, (SeCUmaSize * 1024 * 1024));


  DEBUG ((EFI_D_INFO, "  SeCUmaBase read: %x, SeCUmaBaseExt: %x and SeCUmaSize: %d MB\n UmaBase %x, UmaBaseExt %x \n", SeCUmaBase, UmaBaseExt, SeCUmaSize, UmaBase, UmaBaseExt));

  // 4) Set register bit SATT1_CTRL.ENTRY_VLD to 1.
  HeciPciOr32(R_SATT1_CTRL, B_ENTRY_VLD);

  // review end 23092011 4:00pm
  // remove
  //
  // Write DRAM Init Done (DID) data to the SEC H_GS[23:0].
  // H_GS[23:16] = extended UMA base address (reserved)
  // H_GS[15:0] = 1M aligned UMA base address
  // SEC FW will 0 extend these values to determine SeCUmaBase
  //

  //
  // 5) Program register based DRAM Init Done message indicating to SeC firmware that DRAM
  //    initialization is complete and SeC UMA is ready for use. This is done via register DID_MSG.
  //    The register based DRAM Init Done message should be sent by the system BIOS on all boot flows.
  //
  // Set DID_MSG[31:28] = 0x1 indicating DRAM Init Done
  //
  SeC_DID_MSG = InitStat & 0x0F;

  //
  // Set DID_MAG[27:24] = Status
  //   0x0 = Success
  //   0x1 = No Memory in channels
  //   0x2 = Memory Init Error
  //   0x3-0xF = Reserved
  //
  SeC_DID_MSG |= (InitStat << 24);
  SeC_DID_MSG = SeC_DID_MSG | V_SEC_DID_MSG_MASK;
  //
  // Read the SEC SEC_FW_STS0.ACK_DATA Register to look for DID ACK.
  //
  SeCHfs = HeciPciRead32 (R_SEC_FW_STS0);
  DEBUG ((EFI_D_INFO, "  HFS read before DID ACK: %x\n", SeCHfs));


  HeciPciOr32 (R_DID_MSG, SeC_DID_MSG);
  DEBUG ((EFI_D_INFO, "  SEC R_DID_MSG written: %x\n", SeC_DID_MSG));


  //
  // SEC FW typically responds with the DID ACK w/in 1ms
  // Adding short delay to avoid wasting time in the timeout loop
  //
  StallPpi->Stall (PeiServices, StallPpi, 1100);

  DidAck = ((SeCHfs & B_SEC_DID_ACK_MASK) >> 28);

  SeCMemReq = HeciPciRead32 (R_SEC_MEM_REQ);

  if(((SeCMemReq & B_SEC_MEM_REQ_INVALID)) == B_SEC_MEM_REQ_INVALID) {
    return EFI_UNSUPPORTED;
  }
  //
  // ~5 second Timeout for DID ACK
  //
  // 6) BIOS must poll on the register field SEC_FW_STS0.BIOS_MSG_ACK. SeC firmware will respond to the
  //    DRAM Init Done message with a DRAM Init Done ACK message, which will be located in that field.
  //    This ACK message will include a requested "SeC BIOS Action", which the system BIOS should act on
  //    as soon as possible upon receiving the DRAM Init Done ACK message.
  //
  while (!DidAck ) {
    StallPpi->Stall (PeiServices, StallPpi, 1000);
    SeCHfs   = HeciPciRead32 (R_SEC_FW_STS0);
    DidAck  = ((SeCHfs & B_SEC_DID_ACK_MASK) >> 28);
  }

  DEBUG ((EFI_D_INFO, "SEC DRAM Init Done ACK received.\n"));
  //
  //
  // 7) As soon as DRAM Init Done ACK is received, BIOS should continue as instructed, according to register field SEC_FW_STS0.ACK_DATA.
  //  If a DRAM Init Done acknowledgement is not received within 5 seconds, BIOS will continue boot, assuming the SeC is not functional
  // and should ignore the ACK Data.
  //
  BiosAction    = ((SeCHfs & (B_SEC_BIOS_ACTION_MASK)) >> 25);
  DEBUG ((EFI_D_ERROR, "BiosAction = %x\n", BiosAction));

  //
  // 8) If the BIOS needs to perform any power transition after system powers on, the BIOS must wait until receiving the DRAM Init Done ACK message.
  //    The BIOS must conduct the power transition based on the register field SEC_FW_STS0.ACK_DATA. If SEC_FW_STS0.ACK_DATA instructs that BIOS can
  //    continue to POST, then the BIOS can apply its own power transition flow after that.
  //
  Status = HandleSeCBiosAction (PeiServices, BiosAction);

  //
  // 9) If BIOS takes the normal boot path, it must set register field SICR_HOST_ALIVENESS_REQ.ALIVENESS_REQ. This action ensures that SeC remains
  //    powered and thus provides a quick response time to commands arriving from BIOS. Before completing POST, BIOS must clear SICR_HOST_ALIVENESS_REQ.ALIVENESS_REQ
  //    as its last action, before handing over control to the OS.
  //
#if CF9_REG_LOCK_ENABLE  
  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);
  if (!(SeCFirmwareStatus.r.ManufacturingMode)) 
  {
    Address = PciRead32( PCI_LIB_ADDRESS (
              DEFAULT_PCI_BUS_NUMBER_PCH,
              PCI_DEVICE_NUMBER_PCH_LPC,
              PCI_FUNCTION_NUMBER_PCH_LPC,
              R_PCH_LPC_PMC_BASE
              ));
    Address &= B_PCH_LPC_PMC_BASE_BAR;
    MmioOr32 ((UINTN) (Address + R_PCH_PMC_PMIR), B_PCH_PMC_PMIR_CF9LOCK);
  }
#endif
   
#ifdef CHECK_ALIVENESS
  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);
  if(SeCFirmwareStatus.r.SeCOperationMode == SEC_MODE_NORMAL && SeCFirmwareStatus.r.CurrentState == SEC_IN_RECOVERY_MODE) {
    HeciBar1Value =  HeciPciRead32 (HECI_BAR1);
    Mmio32Or(HeciBar1Value, R_SICR_HOST_ALIVENESS_REQ, B_ALIVENESS_REQ);
    DEBUG ((EFI_D_INFO, "  SEC SET B_ALIVENESS_REQ written \n"));

    HAlivenessResponse =  Mmio32(HeciBar1Value, R_HICR_HOST_ALIVENESS_RESP);

    Timeout = 0;
    while(((HAlivenessResponse & B_ALIVENESS_ACK) != B_ALIVENESS_ACK) && (Timeout < HOST_ALIVENESS_RESP_TIMEOUT_MULTIPLIER) ) {
      StallPpi->Stall (PeiServices, StallPpi, 10000);
      HAlivenessResponse = Mmio32(HeciBar1Value, R_HICR_HOST_ALIVENESS_RESP);
      DEBUG ((EFI_D_INFO, "Read HOST Alive ACK: %x %x\n", HAlivenessResponse, Timeout));
      Timeout++;
    }
    // TODO:
    // Note that after host writes to this bit, it must wait for an acknowledgement that is generated by bit <TBD> before assuming that SeC has received the request.
    // Until such acknowledgement is received, Host must not generate additional requests or request cancellations on this bit.
    //
    if (Timeout >= HOST_ALIVENESS_RESP_TIMEOUT_MULTIPLIER) {
      DEBUG ((EFI_D_INFO, "Timeout occurred waiting for host aliveness ACK.\n"));
    } else {
      DEBUG ((EFI_D_INFO, "SEC host aliveness ACK received.\n"));
    }
  }
#endif // CHECK_ALIVENESS


#ifdef WAIT4FWSTS
  SeCHfs = HeciPciRead32 (R_SEC_FW_STS0);
  while (SeCHfs != 0x1f000255) {
    SeCHfs = HeciPciRead32 (R_SEC_FW_STS0);
    StallPpi->Stall (PeiServices, StallPpi, 11000);
    StallPpi->Stall (PeiServices, StallPpi, 11000);
    StallPpi->Stall (PeiServices, StallPpi, 11000);
    DEBUG ((EFI_D_INFO, "FW status : %x\n", SeCHfs));
  }
#endif
  return Status;
}

EFI_STATUS
HandleSeCBiosAction (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  UINT8                     BiosAction
  )
/*++

Routine Description:

  This procedure will enforce the BIOS Action that was requested by SEC FW
  as part of the DRAM Init Done message.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  BiosAction - SeC requests BIOS to act

Returns:

  Return EFI_SUCCESS

--*/
{

  //
  // BIOS ACTION
  //  0x00: No DRAM Init Done ACK received.
  //  0x01: Non-power cycle reset.
  //  0x02: Power cycle reset.
  //  0x03: Go to S3.
  //  0x04: Go to S4.
  //  0x05: Go to S5.
  //  0x06: Perform Global Reset.
  //  0x07: Continue to boot.
  //

  EFI_STATUS  Status;

  //
  // If a reset is required
  //
  if (BiosAction == 0x1 || BiosAction == 0x2 || BiosAction == 0x6) {
    Status = PerformReset (PeiServices, BiosAction);
  }

  switch (BiosAction) {
    case 0:
      //
      // DID ACK was not received
      //
      DEBUG ((EFI_D_ERROR, "DID Ack was not received, no BIOS Action to process.\n"));
      break;

    case 3:
      //
      // Go To S3
      //
      DEBUG ((EFI_D_INFO, "SEC FW DID ACK has requested entry to S3.  Not defined, continuing to POST.\n"));
      break;

    case 4:
      //
      // Go To S4
      //
      DEBUG ((EFI_D_INFO, "SEC FW DID ACK has requested entry to S4.  Not defined, continuing to POST.\n"));
      break;

    case 5:
      //
      // Go To S5
      //
      DEBUG ((EFI_D_INFO, "SEC FW DID ACK has requested entry to S5.  Not defined, continuing to POST.\n"));
      break;

    case 7:
      //
      // Continue POST
      //
      DEBUG ((EFI_D_INFO, "SEC FW DID Ack requested to continue to POST.\n"));
      break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PerformReset (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  UINT8                     ResetType
  )
/*++

Routine Description:

  This procedure will issue a Non-Power Cycle, Power Cycle, or Global Rest.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  ResetType  -  Type of reset to be issued.

Returns:

  Return EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;
  UINT32      ETR3;
  UINT32      GpioBase;
  UINT8       Reset;

  ETR3      = 0;
  Reset     = 0;
  GpioBase  = 0;
  //
  // Clear the DISB bit in PCH (DRAM Initialization Scratchpad Bit - GEN_PMCON2[7]),
  // since S3 Data will not be saved to NVRAM.
  //
  Status = ClearDISB ();

  //
  // Clear CF9GR of PCH (B0/D31/f0 offset 0x0AC[20] = 1b) to  indicate Host reset
  // Make sure CWORWRE (CF9 Without Resume Well Reset Enable) is cleared
  //
  ETR3  = ETR3 &~BIT20;
  ETR3  = ETR3 &~BIT18;

  Reset = IoRead8 (R_PCH_RST_CNT);
  Reset &= 0xF1;

  //
  // If global reset required
  //
  if (ResetType == 0x6) {
    //
    // Get GPIO Base Address
    //
    GpioBase = PciRead32 (PCI_LIB_ADDRESS (0, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_GPIO_BASE)) &~BIT0;
  }

  switch (ResetType) {
    case 1:
      DEBUG ((EFI_D_ERROR, "SEC FW DID ACK has requested a Non Power Cycle Reset.\n"));
      Reset |= 0x06;
      break;

    case 2:
      //
      // Power Cycle Reset requested
      //
      DEBUG ((EFI_D_INFO, "SEC FW DID ACK has requested a Power Cycle Reset.\n"));
      Reset |= 0x0E;
      break;

    case 6:
      //
      // Global Reset
      //
      DEBUG ((EFI_D_ERROR, "SEC FW DID Ack requested a global reset.\n"));

      //
      // Issue global reset CF9 = 0x0E
      //
      DEBUG ((EFI_D_ERROR, "Issuing global reset.\n"));
      Reset |= 0x0E;
      break;
  }
  //
  // Write PCH RST CNT, Issue Reset
  //
  IoWrite8 (R_PCH_RST_CNT, Reset);
  CpuDeadLoop();

  return EFI_SUCCESS;
}


EFI_STATUS
ClearDISB (
  VOID
  )
/*++

Routine Description:

  This procedure will clear the DISB.

Arguments:

Returns:

  Return EFI_SUCCESS

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SeCUmaEntry (
#ifdef ECP_FLAG
  IN  EFI_FFS_FILE_HEADER    *FileHandle,
  IN  EFI_PEI_SERVICES       **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE     *FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
#endif
  )
/*++

Routine Description:

  This function is the entry point for this PEI.

Arguments:

  FfsHeader   - Pointer to the FFS file header
  PeiServices - Pointer to the PEI services table

Returns:

  Return Status based on errors that occurred while waiting for time to expire.

--*/
{

  EFI_PEI_STALL_PPI                *StallPpi;
  EFI_STATUS                        Status;
  Status = (*PeiServices)->InstallPpi (PeiServices, mSeCUmaPpiList);
  Status = (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  ASSERT_EFI_ERROR (Status);
  if(Status == EFI_SUCCESS) {
    DEBUG ((EFI_D_INFO, "Info: SeC PPI load sucessfully\n"));
  } else {
    DEBUG ((EFI_D_INFO, "Error! SeC PPI not sucessfully load\n"));
  }

  return Status;
}


EFI_STATUS
isSeCExpose(
  IN EFI_PEI_SERVICES **PeiServices
  )
/*++

Routine Description:

  This procedure will check the exposure of SeC device.

Arguments:

    PeiServices - Pointer to the PEI services table
Returns:

  Return EFI_SUCCESS

--*/
{
  EFI_STATUS Status = 0x0;

  // Device ID read here
  UINT32 DeviceID;
  DeviceID = (HeciPciRead32 (R_SEC_DevID_VID) & S_SEC_DevID_MASK) >> 16;

  if(DeviceID >= S_SEC_DevID_RANGE_LO && DeviceID <= S_SEC_DevID_RANGE_HI) {
    Status = 0x1;
    DEBUG ((EFI_D_INFO, "SeC Device ID: %x\n", DeviceID));
  }
  return Status;
}
