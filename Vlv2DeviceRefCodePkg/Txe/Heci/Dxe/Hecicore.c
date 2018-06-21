/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2007 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Hecicore.c

Abstract:

  Heci driver core. For Dxe Phase, determines the HECI device and initializes it.

--*/
#ifdef ECP_FLAG
#include "Hecidrv.h"
#include "HeciHpet.h"
#include "HeciRegs.h"
#include "SeCState.h"
#include "SeCLib.h"
#endif
#include "Hecicore.h"


#define B_EXCLUSION BIT8
//#define WAIT4_30SECS
//
// //////////////////////////////////////////////////////////////////////////////////
// Globals used in Heci driver
////////////////////////////////////////////////////////////////////////////////////
//
UINT16        HECICtlrBDF;
static UINT32 HeciMBAR = 0, HeciMBAR0 = 0;

//
// //////////////////////////////////////////////////////////////////////////////////
// Macro definition for function used in Heci driver
////////////////////////////////////////////////////////////////////////////////////
//
UINT32
MmIoReadDword (
  UINTN a
)
/*++

Routine Description:

  The routing of MmIo Read Dword

Arguments:

  a - The address of Mmio

Returns:

  Return the valut of MmIo Read

--*/
{
  volatile HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;

  HeciRegHCsrPtr = (HECI_HOST_CONTROL_REGISTER *) a;
  return HeciRegHCsrPtr->ul;
}

VOID
MmIoWriteDword (
  UINTN  a,
  UINT32 b
)
/*++

Routine Description:

  The routing of MmIo Write Dword

Arguments:

  a - The address of Mmio
  b - Value revised

Returns:

  None

--*/
{
  volatile HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;

  HeciRegHCsrPtr      = (HECI_HOST_CONTROL_REGISTER *) a;

  HeciRegHCsrPtr->ul  = b;
}

#define MMIOREADDWORD(a)      MmIoReadDword (a)
#define MMIOWRITEDWORD(a, b)  MmIoWriteDword (a, b)

//
// Extern for shared HECI data and protocols
//
extern HECI_INSTANCE  *mHeciContext;

//
// //////////////////////////////////////////////////////////////////////////////////
// Forward declaration
////////////////////////////////////////////////////////////////////////////////////
//
UINT8
FilledSlots (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer
);

EFI_STATUS
OverflowCB (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer,
  IN      UINT32                    BufferDepth
);

EFI_STATUS
WaitForSECReady (
  VOID
);

EFI_STATUS
WaitForSECReadyEx(
  VOID
);

#ifdef EFI_DEBUG

VOID
ShowBuffer (
  UINT8  *Message,
  UINT32 Length
)
/*++

Routine Description:

  For serial debugger used, it will show the buffer message to serila consol.

Arguments:

  Message - the address point of buffer message
  Length  - message length

Returns:

  None.

--*/
{
  UINT32  LineBreak;
  UINT32  Index;
  LineBreak = 0;
  Index     = 0;

  while (Length-- > 0) {
    if (LineBreak == 0) {
      DEBUG ((EFI_D_ERROR, "%02x: ", (Index & 0xF0)));
    }

    DEBUG ((EFI_D_ERROR, "%02x ", Message[Index++]));
    LineBreak++;
    if (LineBreak == 16) {
      DEBUG ((EFI_D_ERROR, "\n"));
      LineBreak = 0;
    }

    if (LineBreak == 8) {
      DEBUG ((EFI_D_ERROR, "- "));
    }
  }

  DEBUG ((EFI_D_ERROR, "\n"));
  return ;
}

#endif // End Of EFI_DEBUG
// )
// //////////////////////////////////////////////////////////////////////////////////
// Heci driver function definitions
////////////////////////////////////////////////////////////////////////////////////
//
EFI_STATUS
InitializeHeciPrivate (
  VOID
)
/*++

  Routine Description:
    Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
{
  volatile SICR_HOST_ALIVENESS_REQ    *SicrHostAlivenessReqPtr;
  volatile HICR_HOST_ALIVENESS_RESP   *HicrHostAlivenessRespPtr;
  volatile SICR_HOST_IPC_READINESS    *SicrHostIPCReadinessPtr;
  volatile HICR_SEC_IPC_READINESS     *HicrSeCIPCReadinessPtr;
  volatile HHISR              *HostInterruptPtr;
//  HECI_HOST_CONTROL_REGISTER          HeciRegHCsr;
//  volatile HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;
  EFI_STATUS                          Status;
  HECI_FWS_REGISTER                   SeCFirmwareStatus;
  UINT32                                     SeC_Exclusion_req;
  UINT32                                       SeC_Exclusion_cause;
//  UINT32                              Data32;


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
        //  If bit[4] == 1 and bits [3:0] have value of 0000b, 0100b, 0101b, or 0110b,
        // then the BIOS takes the SeC Error BIOS path. Refer to Section ?3.3.5.1?

      case 0x0:
      case 0x4:
      case 0x5:
      case 0x6:
        DEBUG ((EFI_D_INFO, "Error: No HECI command send \n"));
        return EFI_UNSUPPORTED;
      default:
        break;
    }
  }

  /*
    DEBUG ((EFI_D_ERROR, "InitializeHeciPrivate ++ \n "));
    DEBUG ((EFI_D_ERROR, "Send Shadow Done Message Start\n"));
    Data32 = HeciPciRead32 (0x64);
    DEBUG ((EFI_D_ERROR, "Send Shadow Done Message read Offset 0x64 is %x \n", Data32));
    Data32 |= 1;
    DEBUG ((EFI_D_ERROR, "Send Shadow Done Message write Offset 0x64 is %x try to send Shadow Done Message \n", Data32));
    HeciPciWrite32(0x64,Data32);
    DEBUG ((EFI_D_ERROR, "Sent Shadow Done Message End\n"));
   */ Status = EFI_SUCCESS;

  //
  // Check for SEC MemValid status
  //
  if ((HeciPciRead32 (R_SEC_MEM_REQ) & B_SEC_MEM_REQ_INVALID) == B_SEC_MEM_REQ_INVALID) {
    //
    // SEC failed to start so no HECI
    //
    DEBUG ((EFI_D_ERROR, "SEC failed to start so no HECI\n"));
    return EFI_UNSUPPORTED;
  }

  SaveHpet ();
  do {
    //
    // Store HECI vendor and device information away
    //
    mHeciContext->DeviceInfo = HeciPciRead16 (PCI_DEVICE_ID_OFFSET);

    //
    // Check for HECI-1 PCI device availability
    //
    if (mHeciContext->DeviceInfo == 0xFFFF) {
      Status = EFI_DEVICE_ERROR;
      break;
    }

    SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

    //
    if(SeCFirmwareStatus.r.SeCOperationMode != SEC_MODE_NORMAL && SeCFirmwareStatus.r.CurrentState != SEC_IN_RECOVERY_MODE) {
      return EFI_UNSUPPORTED;
    }
    //
    // Check for SEC FPT Bad
    //
    if (SeCFirmwareStatus.r.FptBad) {
      Status = EFI_DEVICE_ERROR;
      break;
    }

    //
    // Check for SEC error status
    //
    if (SeCFirmwareStatus.r.ErrorCode) {
      //
      // SEC failed to start so no HECI
      //
      Status = EFI_DEVICE_ERROR;
      break;
    }

    //
    // Store HECI revision ID
    //
    mHeciContext->RevisionInfo = HeciPciRead8 (PCI_REVISION_ID_OFFSET);

    //
    // Get HECI_MBAR and see if it is programmed
    // to a useable value
    //
    mHeciContext->HeciMBAR  = HeciPciRead32 (R_HECIMBAR1) & 0xFFFFFFF0;
    HeciMBAR                = mHeciContext->HeciMBAR;

    mHeciContext->HeciMBAR0  = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
    HeciMBAR0                = mHeciContext->HeciMBAR0;

    //
    // Load temporary address for HECI_MBAR if one is not assigned
    //
    if (mHeciContext->HeciMBAR == 0) {
      DEBUG ((EFI_D_ERROR, "Heci MMIO Bar not programmed in DXE phase\n"));
    }
    //
    // Enable HECI BME, MSE and SERR
    //
    HeciPciOr32 (
      PCI_COMMAND_OFFSET,
      EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_SERR
    );

    //
    // Set HECI interrupt delivery mode.
    // HECI-1 using legacy/MSI interrupt
    //
    HeciPciAnd8 (R_HIDM, 0xFC);
#ifdef TESTMENU_FLAG
    SeCPolicyLibInit();
    if(SeCHECIEnabled() != TRUE) {
      DEBUG ((EFI_D_ERROR, "HECI Unsupported ++ \n "));
      return EFI_UNSUPPORTED;
    }
#endif


    //
    // HECI MSG is unsupported if SEC MODE is in SEC ALT Disabled & SECOVR JMPR
    //
    if ((SeCFirmwareStatus.r.SeCOperationMode == SEC_OPERATION_MODE_SECOVR_JMPR) ||
        (SeCFirmwareStatus.r.SeCOperationMode == SEC_OPERATION_MODE_ALT_DISABLED) ||
        (SeCFirmwareStatus.r.SeCOperationMode == SEC_OPERATION_MODE_SOFT_TEMP_DISABLE) ||
        (SeCFirmwareStatus.r.SeCOperationMode == SEC_OPERATION_MODE_SECOVR_HECI_MSG)
       ) {
      return EFI_UNSUPPORTED;
    }

    //
    // 3. Host checks the value of SICR_HOST_ALIVENESS_REQ.ALIVENESS_REQ and HICR_HOST_ALIVENESS_RESP.ALIVENESS_RESP bits.
    // If different, host waits for an interrupt about HICR_HOST_ALIVENESS_RESP.ALIVENESS_RESP bit change or polls for it.
    // when both bits are same, if they are set, host clears the SICR_HOST_ALIVENESS_REQ.ALIVENESS_REQ bit and waits for
    // an interrupt about HICR_HOST_ALIVENESS_RESP.ALIVENESS_RESP bit change or polls for it. Otherwise, host skips to step 5.
    //

    SicrHostAlivenessReqPtr =  (VOID *)(UINTN)(HeciMBAR + H_ALIVENESS_REQ);
    HicrHostAlivenessRespPtr =  (VOID *)(UINTN)(HeciMBAR + H_ALIVENESS_ACK);

    while(SicrHostAlivenessReqPtr->r.H_ALIVE_REQ != HicrHostAlivenessRespPtr->r.H_ACK) {
      // DEBUG ((EFI_D_ERROR, " Host Alive..... %x %x ",SicrHostAlivenessReqPtr->r.H_ALIVE_REQ,HicrHostAlivenessRespPtr->r.H_ACK));
      IoDelay (HECI_WAIT_DELAY);
    }

    if(SicrHostAlivenessReqPtr->r.H_ALIVE_REQ == 1) {
      SeCAlivenessRequest( &HeciMBAR, 0);
    }



//#ifndef SEC_TEST
//   while(1)
//   {
//      HISRPtr = Mmio32(HeciMBAR, H_HISR);
//      if ((HISRPtr & BIT0)==BIT0) break;
//  }
//#endif
    //
    // 5.   Host sets the SICR_HOST_IPC_READINESS.RDY_CLR bit.
    //
    SicrHostIPCReadinessPtr =  (VOID *)(UINTN)(HeciMBAR + S_HOST_IPC_READINESS);

    // SicrHostIPCReadiness.ul  = SicrHostIPCReadinessPtr->ul;
    SicrHostIPCReadinessPtr->r.RDY_CLR = 1;
    // SicrHostIPCReadinessPtr->ul = SicrHostIPCReadiness.ul;

    //
    //  8. Host waits for an interrupt indicating that HICR_HOST_IPC_READINESS.SEC_RDY
    //     bit is set or polls for it, if such an interrupt has not arrived since step ?5.
    //
    HicrSeCIPCReadinessPtr =  (VOID *)(UINTN)(HeciMBAR + H_SEC_IPC_READINESS);

    // HicrSeCIPCReadiness.ul  = HicrSeCIPCReadinessPtr->ul;
    while(HicrSeCIPCReadinessPtr->r.SEC_RDY != 1);

    //
    // 10. Host then clears the interrupt generated by HISR.INT2_STS (if such an interrupt was generated)
    // and any additional interrupt status bits along the Host interrupt path (see Section ?3.4.1.2)  in order
    // to ensure that any messages sent by SeC prior to SeC step ?12 are not misconstrued after this flow's
    // completion as messages that arrived after reset.
    //
    HostInterruptPtr =  (VOID *)(UINTN)(HeciMBAR + H_HHISR);

    HostInterruptPtr->r.INT_BAR0_STS = 0;
    HostInterruptPtr->r.INT_BAR1_STS = 0;


    //
    // 13.  Host sets the HICR_SEC_IPC_OUTPUT_STATUS.IPC_OUTPUT_READY bit.
    //
    Mmio32Or(HeciMBAR, S_SEC_IPC_OUTPUT_STATUS, 1);

    //
    // 14. Host sets bit SICR_HOST_IPC_READINESS.HOST_RDY.
    //
    SicrHostIPCReadinessPtr->r.HOST_RDY = 1;

  } while (EFI_ERROR (Status));

  RestoreHpet ();

  DEBUG ((EFI_D_ERROR, "InitializeHeciPrivate -- \n "));
  SeCAlivenessRequest(&HeciMBAR, 1);

  return Status;
}

EFI_STATUS
WaitForSECReady (
  VOID
)
/*++

  Routine Description:
    Waits for the SEC to report that it is ready for communication over the HECI
    interface.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
{
  UINT32                    TimerStart;
  UINT32                    TimerEnd;
  volatile SEC_IPC_INPUT_STATUS     *SeCIpcInputStatusPtr;
  UINT32            _HeciBAR0;

  _HeciBAR0 = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;

  //
  //  Wait for SEC ready
  //
  //
  // Check for SEC ready status
  //
  StartTimer (&TimerStart, &TimerEnd, HECI_READ_TIMEOUT);
  SeCIpcInputStatusPtr = (VOID *)(UINTN)(_HeciBAR0 + SEC_IPC_INPUT_STS);
  while (SeCIpcInputStatusPtr->r.IPC_INPUT_READY != 1) {

    DEBUG ((EFI_D_ERROR, "WaitForSECReady  ++ %x\n ", HeciMBAR0 + SEC_IPC_INPUT_STS));
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }
    //
    // Perform IO delay
    //
    IoDelay (HECI_WAIT_DELAY);
  }
  //
  // SEC ready!!!
  //
  return EFI_SUCCESS;
}


EFI_STATUS
WaitForSECReadyEx (
  VOID
)
/*++

  Routine Description:
    Waits for the SEC to report that it is ready for communication over the HECI
    interface with timer up to 90 seconds.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
{
  UINT32                    TimerStart;
  UINT32                    TimerEnd;
  volatile SEC_IPC_INPUT_STATUS     *SeCIpcInputStatusPtr;
  UINT32            _HeciBAR0;

  _HeciBAR0 = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;

  //
  //  Wait for SEC ready
  //
  //
  // Check for SEC ready status
  //
  StartTimer (&TimerStart, &TimerEnd, HECI_READ_TIMEOUT_EX);
  SeCIpcInputStatusPtr = (VOID *)(UINTN)(_HeciBAR0 + SEC_IPC_INPUT_STS);
  while (SeCIpcInputStatusPtr->r.IPC_INPUT_READY != 1) {

    DEBUG ((EFI_D_ERROR, "WaitForSECReady  ++ %x\n ", HeciMBAR0 + SEC_IPC_INPUT_STS));
    //
    // If 120 second timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }
    //
    // Perform IO delay
    //
    IoDelay (HECI_WAIT_DELAY);
  }
  //
  // SEC ready!!!
  //
  return EFI_SUCCESS;
}

BOOLEAN
CheckForHeciReset (
  VOID
)
/*++

  Routine Description:
    Checks if HECI reset has occured.

  Arguments:
    None.

  Returns:
    TRUE - HECI reset occurred
    FALSE - No HECI reset occurred

--*/
{
  //HICR_HOST_IPC_READINESS.SEC_RDY
  volatile HICR_SEC_IPC_READINESS   *HicrSeCIPCReadinessPtr;
// HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
// HECI_SEC_CONTROL_REGISTER    HeciRegSeCCsrHa;

  HicrSeCIPCReadinessPtr =  (VOID *)(UINTN)(HeciMBAR + H_SEC_IPC_READINESS);
  //
  // Init Host & SEC CSR
  //
//  HeciRegHCsr.ul    = MMIOREADDWORD (HeciMBAR + H_CSR);
//  HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);

//  if ((HeciRegSeCCsrHa.r.SEC_RDY_HRA == 0) || (HeciRegHCsr.r.H_RDY == 0)) {
//    return TRUE;
//  }
  if(HicrSeCIPCReadinessPtr->r.SEC_RDY == 0) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
HeciInitialize (
  VOID
)
/*++

  Routine Description:
    Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
{

// HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;


  DEBUG ((EFI_D_ERROR, "HECI Initialize  ++ \n "));
  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Need to do following on SEC init:
  //
  //  1) wait for SEC_CSR_HA reg SEC_RDY bit set
  //
//  if (WaitForSECReady () != EFI_SUCCESS) {
//    return EFI_TIMEOUT;
//  }
  //
  //  2) setup H_CSR reg as follows:
  //     a) Make sure H_RST is clear
  //     b) Set H_RDY
  //     c) Set H_IG
  //
//  HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
// if (HeciRegHCsr.r.H_RDY == 0) {
//   HeciRegHCsr.r.H_RST = 0;
//   HeciRegHCsr.r.H_RDY = 1;
//   HeciRegHCsr.r.H_IG  = 1;
//   MMIOWRITEDWORD (HeciMBAR + H_CSR, HeciRegHCsr.ul);
// }
  InitializeHeciPrivate();
  DEBUG ((EFI_D_ERROR, "HECI Initialize  -- \n "));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciReInitialize (
  VOID
)
/*++

  Routine Description:
    Heci Re-initializes it for Host

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
{
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  EFI_STATUS                  Status;

  Status = EFI_SUCCESS;
  //
  // Need to do following on SEC init:
  //
  //  1) wait for HOST_CSR_HA reg H_RDY bit set
  //
  //    if (WaitForHostReady() != EFI_SUCCESS) {
  //
  if (SeCResetWait (HECI_INIT_TIMEOUT) != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }

  HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  if (HeciRegHCsr.r.H_RDY == 0) {
    Status = ResetHeciInterface ();

  }

  return Status;
}

EFI_STATUS
EFIAPI
HeciReInitialize2 (
  VOID
)
/*++

  Routine Description:
    Heci Re-initializes it for SeC

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
{
  HECI_SEC_CONTROL_REGISTER  HeciRegSeCCsrHa;
  EFI_STATUS                Status;
  UINT32                    TimerStart;
  UINT32                    TimerEnd;
  Status = EFI_SUCCESS;
  //
  // Need to do following on SEC init:
  //
  //  1) wait for HOST_CSR_HA reg H_RDY bit set
  //
  //    if (WaitForHostReady() != EFI_SUCCESS) {
  //
  StartTimer (&TimerStart, &TimerEnd, HECI_INIT_TIMEOUT);
  HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
  while (HeciRegSeCCsrHa.r.SEC_RDY_HRA == 1) {
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }

    IoDelay (HECI_WAIT_DELAY);

    HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
  }

  if (WaitForSECReady () != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }

  return Status;
}

EFI_STATUS
HECIPacketRead (
  IN      UINT32                    Blocking,
  OUT     HECI_MESSAGE_HEADER       *MessageHeader,
  OUT     UINT32                    *MessageData,
  IN OUT  UINT32                    *Length
)
/*++

  Routine Description:
    Function to pull one messsage packet off the HECI circular buffer.
    Corresponds to HECI HPS (part of) section 4.2.4


  Arguments:
    Blocking      - Used to determine if the read is BLOCKING or NON_BLOCKING.
    MessageHeader - Pointer to a buffer for the message header.
    MessageData   - Pointer to a buffer to recieve the message in.
    Length        - On input is the size of the callers buffer in bytes.  On
                    output this is the size of the packet in bytes.

  Returns:
    EFI_STATUS

--*/
{
  BOOLEAN                     GotMessage;
  UINT32                      TimerStart;
  UINT32                      TimerEnd;
//  UINT32                      TimerStart1;
//  UINT32                      TimerEnd1;
  UINT32                      i;
  UINT32                      LengthInDwords;
  volatile HICR_SEC_IPC_OUTPUT_DOORBELL        *HicrSeCIpcOutputDoorbellPtr;


  //
  // Initialize memory mapped register pointers
  //
  volatile SICR_SEC_IPC_OUTPUT_STATUS *SicrSeCIpcOutputStatusPtr;
  //volatile HECI_SEC_CONTROL_REGISTER   *HeciRegSeCCsrHaPtr;
  volatile UINT32                     *IpcOutputLoadPtr;   //IPC_OUTPUT_PAYLOAD

  GotMessage = FALSE;
  DEBUG ((EFI_D_ERROR, "HeciPacketRead++\n\n "));
//  HeciMemBar0                 = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
  IpcOutputLoadPtr            = (VOID *)(UINTN)(HeciMBAR + IPC_OUTPUT_PAYLOAD);
  SicrSeCIpcOutputStatusPtr   = (VOID *)(UINTN)(HeciMBAR + S_SEC_IPC_OUTPUT_STATUS);
  HicrSeCIpcOutputDoorbellPtr = (VOID *)(UINTN)(HeciMBAR + SEC_IPC_OUTPUT_DRBELL);


  DEBUG ((EFI_D_ERROR, "HECIPacketRead  ++ \n "));
  //
  // clear Interrupt Status bit
  //
// HeciRegHCsr.ul      = MMIOREADDWORD (HeciMBAR + H_CSR);
// HeciRegHCsr.r.H_IS  = 1;

  //
  // test for circular buffer overflow
  //
// HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
// if (OverflowCB (
//       HeciRegSeCCsrHa.r.SEC_CBRP_HRA,
  //     HeciRegSeCCsrHa.r.SEC_CBWP_HRA,
  //   HeciRegSeCCsrHa.r.SEC_CBD_HRA
  // ) != EFI_SUCCESS) {
  //
  // if we get here, the circular buffer is overflowed
  //
//   *Length = 0;
//    return EFI_DEVICE_ERROR;
//  }
  //
  // If NON_BLOCKING, exit if the circular buffer is empty
  //
//  HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);;
// if ((FilledSlots (HeciRegSeCCsrHa.r.SEC_CBRP_HRA, HeciRegSeCCsrHa.r.SEC_CBWP_HRA) == 0) && (Blocking == NON_BLOCKING)) {
//   *Length = 0;
//   return EFI_NO_RESPONSE;
// }
  //
  // Start timeout counter
  //
  if(Blocking == LONG_BLOCKING) {
    StartTimer (&TimerStart, &TimerEnd, HECI_READ_TIMEOUT_EX);
  } else {
    StartTimer (&TimerStart, &TimerEnd, HECI_READ_TIMEOUT);
  }

  //SicrSeCIpcOutputStatusPtr->r.IPC_OUTPUT_READY=1;

  //
  // loop until we get a message packet
  //
  while (!GotMessage) {
    //
    // If 1 second timeout has expired, return fail as we have not yet received a full message.
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      *Length = 0;
      return EFI_TIMEOUT;
    }

    // 1. Host IPC driver receives notification that there was a write to HICR_SEC_IPC_OUTPUT_DOORBELL.
    if(SicrSeCIpcOutputStatusPtr->r.IPC_OUTPUT_READY /*HicrSeCIpcOutputDoorbellPtr->r.IPC_OUTPUT_DOORBELL*/ != 0) continue;

    //  if (FilledSlots (HeciRegSeCCsrHa.r.SEC_CBRP_HRA, HeciRegSeCCsrHa.r.SEC_CBWP_HRA) > 0) {
    //
    // Eat the HECI Message header
    //
    MessageHeader->Data = *IpcOutputLoadPtr;

    //
    // Compute required message length in DWORDS
    //
    LengthInDwords = ((MessageHeader->Fields.Length + 3 + 4) / 4);

    //
    // Just return success if Length is 0
    //
    if (MessageHeader->Fields.Length == 0) {
      //
      // Set Interrupt Generate bit and return
      //
      //  MMIOREADDWORD (HeciMBAR + H_CSR);
      //  HeciRegHCsr.r.H_IG = 1;
      //   MMIOWRITEDWORD (HeciMBAR + H_CSR, HeciRegHCsr.ul);
      *Length = 0;
      return EFI_SUCCESS;
    }
    //
    // Make sure that the message does not overflow the circular buffer.
    //
    //  HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
    // if ((MessageHeader->Fields.Length + sizeof (HECI_MESSAGE_HEADER)) > (HeciRegSeCCsrHa.r.SEC_CBD_HRA * 4)) {
    //    *Length = 0;
    //    return EFI_DEVICE_ERROR;
    //  }
    //
    // Make sure that the callers buffer can hold the correct number of DWORDS
    //
    if ((MessageHeader->Fields.Length) <= *Length) {
      //
      // Start timeout counter for inner loop
      //
      //   StartTimer (&TimerStart1, &TimerEnd1, HECI_READ_TIMEOUT);

      //
      // Wait here until entire message is present in circular buffer
      //
      //   HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
      //  while (LengthInDwords > FilledSlots (HeciRegSeCCsrHa.r.SEC_CBRP_HRA, HeciRegSeCCsrHa.r.SEC_CBWP_HRA)) {
      //
      // If 1 second timeout has expired, return fail as we have not yet received a full message
      //
      //    if (Timeout (TimerStart1, TimerEnd1) != EFI_SUCCESS) {
      //       *Length = 0;
      //      return EFI_TIMEOUT;
      //     }
      //
      // Wait before we read the register again
      //
      //      IoDelay (HECI_WAIT_DELAY);

      //
      // Read the register again
      //
      //        HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
      //     }
      //
      // copy rest of message
      //
      for (i = 1; i < LengthInDwords; i++) {
        MessageData[i - 1] = IpcOutputLoadPtr[i];
      }
      //
      // Update status and length
      //
      GotMessage  = TRUE;
      *Length     = MessageHeader->Fields.Length;

    } else {
      //
      // Message packet is larger than caller's buffer
      //
      *Length = 0;
      return EFI_BUFFER_TOO_SMALL;
    }
    // }
    //
    // Wait before we try to get a message again
    //
    IoDelay (HECI_WAIT_DELAY);
  }
  //
  // Read SEC_CSR_HA.  If the SEC_RDY bit is 0, then an SEC reset occurred during the
  // transaction and the message should be discarded as bad data may have been retrieved
  // from the host's circular buffer
  //
//  HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
//  if (HeciRegSeCCsrHa.r.SEC_RDY_HRA == 0) {
//    *Length = 0;
//    return EFI_DEVICE_ERROR;
//  }

  //
  // 3. Once host IPC driver completed consumption of BRIDGE_SEC_IPC_OUTPUT_PAYLOAD, it sets SICR_SEC_IPC_OUTPUT_STATUS.IPC_OUTPUT_READY.
  //
  SicrSeCIpcOutputStatusPtr->r.IPC_OUTPUT_READY = 1;
  DEBUG ((EFI_D_ERROR, "HeciPacketRead--\n\n "));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciReceive (
  IN      UINT32  Blocking,
  IN OUT  UINT32  *MessageBody,
  IN OUT  UINT32  *Length
)
/*++

  Routine Description:
    Reads a message from the SEC across HECI.

  Arguments:
    Blocking    - Used to determine if the read is BLOCKING or NON_BLOCKING.
    MessageBody - Pointer to a buffer used to receive a message.
    Length      - Pointer to the length of the buffer on input and the length
                  of the message on return. (in bytes)

  Returns:
    EFI_STATUS

--*/
{

  HECI_MESSAGE_HEADER PacketHeader;
  UINT32              CurrentLength;
  UINT32              MessageComplete;
  EFI_STATUS          ReadError;
  EFI_STATUS          Status;
  UINT32              PacketBuffer;
  UINT32              timer_start;
  UINT32              timer_end;
  BOOLEAN             QuitFlag;

  CurrentLength   = 0;
  MessageComplete = 0;
  Status          = EFI_SUCCESS;
  QuitFlag        = FALSE;

  DEBUG ((EFI_D_ERROR, "HeciReceive  ++ \n "));
  SaveHpet ();

  do {
    //
    // Make sure that HECI device BAR is correct and device is enabled.
    //
    HeciMBAR = CheckAndFixHeciForAccess ();

    //
    // Make sure we do not have a HECI reset
    //
    if (CheckForHeciReset ()) {
      //
      // if HECI reset than try to re-init HECI
      //
      Status = HeciInitialize ();

      if (EFI_ERROR (Status)) {
        Status = EFI_DEVICE_ERROR;
        break;
      }
    }
    //
    // Make sure that HECI is ready for communication.
    //
    //
    // Set up timer for BIOS timeout.
    //
    if(Blocking == LONG_BLOCKING) {
      if (WaitForSECReadyEx () != EFI_SUCCESS) {
        Status = EFI_TIMEOUT;
        break;
      }
      StartTimer (&timer_start, &timer_end, HECI_READ_TIMEOUT_EX);
    } else {
      if (WaitForSECReady () != EFI_SUCCESS) {
        Status = EFI_TIMEOUT;
        break;
      }
      StartTimer (&timer_start, &timer_end, HECI_READ_TIMEOUT);
    }
    while ((CurrentLength < *Length) && (MessageComplete == 0)) {
      //
      // If 1 second timeout has expired, return fail as we have not yet received a full message
      //
      if (Timeout (timer_start, timer_end) != EFI_SUCCESS) {
        Status    = EFI_TIMEOUT;
        QuitFlag  = TRUE;
        break;
      }

      PacketBuffer = *Length - CurrentLength;
      ReadError = HECIPacketRead (
                    Blocking,
                    &PacketHeader,
                    (UINT32 *) &MessageBody[CurrentLength / 4],
                    &PacketBuffer
                  );

      //
      // Check for error condition on read
      //
      if (EFI_ERROR (ReadError)) {
        *Length   = 0;
        Status    = ReadError;
        QuitFlag  = TRUE;
        break;
      }
      //
      // Get completion status from the packet header
      //
      MessageComplete = PacketHeader.Fields.MessageComplete;

      //
      // Check for zero length messages
      //
      if (PacketBuffer == 0) {
        //
        // If we are not in the middle of a message, and we see Message Complete,
        // this is a valid zero-length message.
        //
        if ((CurrentLength == 0) && (MessageComplete == 1)) {
          *Length   = 0;
          Status    = EFI_SUCCESS;
          QuitFlag  = TRUE;
          break;
        } else {
          //
          // We should not expect a zero-length message packet except as described above.
          //
          *Length   = 0;
          Status    = EFI_DEVICE_ERROR;
          QuitFlag  = TRUE;
          break;
        }
      }
      //
      // Track the length of what we have read so far
      //
      CurrentLength += PacketBuffer;

    }

    if (QuitFlag == TRUE) {
      break;
    }
    //
    // If we get here the message should be complete, if it is not
    // the caller's buffer was not large enough.
    //
    if (MessageComplete == 0) {
      *Length = 0;
      Status  = EFI_BUFFER_TOO_SMALL;
      break;
    }

    *Length = CurrentLength;

    DEBUG ((EFI_D_ERROR, "HECI ReadMsg:\n"));
#ifdef EFI_DEBUG
    DEBUG_CODE (
      ShowBuffer ((UINT8 *) MessageBody, *Length);
    );
#endif
  } while (EFI_ERROR (Status));

  RestoreHpet ();

  return Status;
}

EFI_STATUS
EFIAPI
HeciSend (
  IN UINT32                     *Message,
  IN UINT32                     Length,
  IN UINT8                      HostAddress,
  IN UINT8                      SeCAddress
)
/*++

  Routine Description:
    Function sends one messsage (of any length) through the HECI circular buffer.

  Arguments:
    Message     - Pointer to the message data to be sent.
    Length      - Length of the message in bytes.
    HostAddress - The address of the host processor.
    SeCAddress   - Address of the SEC subsystem the message is being sent to.

  Returns:
    EFI_STATUS

--*/
{
  UINT32                      CBLength;
  UINT32                      SendLength;
  UINT32                      CurrentLength;
  HECI_MESSAGE_HEADER         MessageHeader;
  EFI_STATUS                  WriteStatus;
  EFI_STATUS                  Status;
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  UINT32                      SeCMode;

  CurrentLength = 0;
  Status        = EFI_SUCCESS;

  SaveHpet ();

  HeciGetSeCMode (&SeCMode);
  do {
    if (SeCMode == SEC_MODE_SECOVER) {
      Status = EFI_UNSUPPORTED;
      break;
    }
    //
    // Make sure that HECI device BAR is correct and device is enabled.
    //
    HeciMBAR = CheckAndFixHeciForAccess ();

    //
    // Make sure we do not have a HECI reset
    //
    if (CheckForHeciReset ()) {
      //
      // if HECI reset than try to re-init HECI
      //
      Status = HeciInitialize ();

      if (EFI_ERROR (Status)) {
        Status = EFI_DEVICE_ERROR;
        break;
      }
    }

    DEBUG ((EFI_D_ERROR, "HECI SendMsg:\n"));

#ifdef EFI_DEBUG
    DEBUG_CODE (
      ShowBuffer ((UINT8 *) Message, Length);
    );
#endif


    //
    // 5.    Host waits for SEC_IPC_INPUT_STATUS.INPUT_READY_BIT to be set. This could be done via polling or using an interrupt-driven model.
    //  Make sure that HECI is ready for communication.
    //
    if (WaitForSECReady () != EFI_SUCCESS) {
      Status = EFI_TIMEOUT;
      break;
    }
    //
    // Set up memory mapped registers
    //
    HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);

    //
    // Grab Circular Buffer length
    //
    CBLength = 16;

    //
    // Prepare message header
    //
    MessageHeader.Data                = 0;
    MessageHeader.Fields.SeCAddress    = SeCAddress;
    MessageHeader.Fields.HostAddress  = HostAddress;

    //
    // Break message up into CB-sized packets and loop until completely sent
    //
    while (Length > CurrentLength) {
      //
      // Set the Message Complete bit if this is our last packet in the message.
      // Needs to be 'less than' to account for the header.
      //
      if ((((Length - CurrentLength) + 3) / 4) < CBLength) {
        MessageHeader.Fields.MessageComplete = 1;
      }
      //
      // Calculate length for Message Header
      //    header length == smaller of circular buffer or remaining message (both account for the size of the header)
      //
      SendLength = ((CBLength <= (((Length - CurrentLength) + 3) / 4)) ? ((CBLength - 1) * 4) : (Length - CurrentLength));
      MessageHeader.Fields.Length = SendLength;

      DEBUG ((EFI_D_ERROR, "HECI Header: %08x\n", MessageHeader.Data));

      //
      // send the current packet (CurrentLength can be treated as the index into the message buffer)
      //
      WriteStatus = HeciPacketWrite (&MessageHeader, (UINT32 *) ((UINTN) Message + CurrentLength));
      if (EFI_ERROR (WriteStatus)) {
        Status = WriteStatus;
        break;
      }
      //
      // Update the length information
      //
      CurrentLength += SendLength;
    }

    if (EFI_ERROR (Status)) {
      break;
    }
  } while (EFI_ERROR (Status));

  RestoreHpet ();

//  EFI_DEADLOOP();
  return Status;

}

EFI_STATUS
HeciPacketWrite (
  IN  HECI_MESSAGE_HEADER       *MessageHeader,
  IN  UINT32                    *MessageData
)
/*++

  Routine Description:
   Function sends one messsage packet through the HECI circular buffer
   Corresponds to HECI HPS (part of) section 4.2.3

  Arguments:
    MessageHeader - Pointer to the message header.
    MessageData   - Pointer to the actual message data.

  Returns:
    EFI_STATUS

--*/
{
  UINT32                      timer_start;
  UINT32                      timer_end;
  UINT32                      i;
  UINT32                      LengthInDwords;

  volatile SEC_IPC_INPUT_DOORBELL     *SeCIpcInputDoorBellPtr;
  volatile UINT32                     *IpcInputLoadPtr;   //IPC_INPUT_PAYLOAD

  DEBUG ((EFI_D_ERROR, "\nDEX: HeciPacketWrite++\n"));
  HeciMBAR0 =  HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
  IpcInputLoadPtr   = (VOID *)(UINTN)(HeciMBAR0 + IPC_INPUT_PAYLOAD);
  //
  // Make sure that HECI is ready for communication.
  //
  if (WaitForSECReady () != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }
  //
  // Start timeout counter
  //
  StartTimer (&timer_start, &timer_end, HECI_SEND_TIMEOUT);

  //
  // Compute message length in DWORDS
  //
  LengthInDwords = ((MessageHeader->Fields.Length + 3 + 4) / 4);


  //
  // Write Message Header
  //
  IpcInputLoadPtr[0] = MessageHeader->Data;

  //
  // Write Message Body
  //
  for (i = 1; i < LengthInDwords; i++) {
    IpcInputLoadPtr[i] = MessageData[i - 1];
  }

  // 7. Host sets bit 0 in SEC_IPC_INPUT_DOORBELL.IPC_INPUT_DOORBELL.
  //    Note that HW automatically clears SEC_IPC_INPUT_STATUS.INPUT_READY bit.
  //
  SeCIpcInputDoorBellPtr = (VOID *)(UINTN)(HeciMBAR0 + SEC_IPC_INPUT_DRBELL);
  SeCIpcInputDoorBellPtr->r.IPC_INPUT_DOORBELL = 1;

  //
  // Test if SEC Ready bit is set to 1, if set to 0 a fatal error occured during
  // the transmission of this message.
  //
//  if (HeciRegSeCCsrHaPtr->r.SEC_RDY_HRA == 0) {
//   return EFI_DEVICE_ERROR;
// }
  DEBUG ((EFI_D_ERROR, "DEX: HeciPacketWrite--"));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciSendwACK (
  IN OUT  UINT32  *Message,
  IN      UINT32  Length,
  IN OUT  UINT32  *RecLength,
  IN      UINT8   HostAddress,
  IN      UINT8   SeCAddress
)
/*++

  Routine Description:
    Function sends one messsage through the HECI buffer and waits
    for the corresponding ACK message.

  Arguments:
    Message     - Pointer to the message buffer.
    SendLength  - Length of the message in bytes.
    RecLength   - Length of the message response in bytes.
    HostAddress - Address of the sending entity.
    MeAddress   - Address of the SEC entity that should receive the message.

  Returns:
    EFI_STATUS

--*/
{
  EFI_STATUS  Status;
  UINT16      RetryCount;
  UINT32      TempRecLength;
  UINT32      SeCMode;

  HeciGetSeCMode(&SeCMode);
  if (SeCMode == SEC_MODE_SECOVER) {
    return EFI_UNSUPPORTED;
  }
  //
  // Send the message
  //
  Status = HeciSend (Message, Length, HostAddress, SeCAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Wait for ACK message
  //
  TempRecLength = *RecLength;
  for (RetryCount = 0; RetryCount < HECI_MAX_RETRY; RetryCount++) {
    //
    // Read Message
    //
    Status = HeciReceive (BLOCKING, Message, &TempRecLength);

    if (!EFI_ERROR (Status)) {
      break;
    }
    //
    // Reload receive length as it has been modified by the read function
    //
    TempRecLength = *RecLength;
  }
  //
  // Return read length and status
  //
  *RecLength = TempRecLength;
  return Status;
}

EFI_STATUS
EFIAPI
SeCResetWait (
  IN  UINT32  Delay
)
/*++

Routine Description:

  SeC reset and waiting for ready

Arguments:

  Delay - The biggest waiting time

Returns:

  EFI_TIMEOUT - Time out
  EFI_SUCCESS - SeC ready

--*/
{
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  UINT32                      TimerStart;
  UINT32                      TimerEnd;

  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Wait for the HOST Ready bit to be cleared to signal a reset
  //
  StartTimer (&TimerStart, &TimerEnd, Delay);
  HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  while (HeciRegHCsr.r.H_RDY == 1) {
    //
    // If timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }

    HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ResetHeciInterface (
  VOID
)
/*++

  Routine Description:
    Function forces a reinit of the heci interface by following the reset heci interface via host algorithm
    in HPS 0.90 doc 4-17-06 njy

  Arguments:
    none

  Returns:
    EFI_STATUS

--*/
{
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  HECI_SEC_CONTROL_REGISTER    HeciRegSeCCsrHa;
  UINT32                      TimerStart;
  UINT32                      TimerEnd;

  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Enable Reset
  //
  HeciRegHCsr.ul      = MMIOREADDWORD (HeciMBAR + H_CSR);
  HeciRegHCsr.r.H_RST = 1;
  HeciRegHCsr.r.H_IG  = 1;
  MMIOWRITEDWORD (HeciMBAR + H_CSR, HeciRegHCsr.ul);

  //
  // Make sure that the reset started
  //
  // HeciRegHCsr.ul = MMIOREADDWORD(HeciMBAR + H_CSR);
  //
  StartTimer (&TimerStart, &TimerEnd, HECI_INIT_TIMEOUT);
  do {
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }
    //
    // Read the SEC CSR
    //
    HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  } while (HeciRegHCsr.r.H_RDY == 1);

  //
  // Wait for SEC to perform reset
  //
  // HeciRegSeCCsrHa.ul = MMIOREADDWORD(HeciMBAR + SEC_CSR_HA);
  //
  StartTimer (&TimerStart, &TimerEnd, HECI_INIT_TIMEOUT);
  do {
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }
    //
    // Read the SEC CSR
    //
    HeciRegSeCCsrHa.ul = MMIOREADDWORD (HeciMBAR + SEC_CSR_HA);
  } while (HeciRegSeCCsrHa.r.SEC_RDY_HRA == 0);

  //
  // Make sure IS has been signaled on the HOST side
  //
  // HeciRegHCsr.ul = MMIOREADDWORD(HeciMBAR + H_CSR);
  //
  StartTimer (&TimerStart, &TimerEnd, HECI_INIT_TIMEOUT);
  do {
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }
    //
    // Read the SEC CSR
    //
    HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  } while (HeciRegHCsr.r.H_IS == 0);

  //
  // Enable host side interface
  //
  HeciRegHCsr.ul      = MMIOREADDWORD (HeciMBAR + H_CSR);;
  HeciRegHCsr.r.H_RST = 0;
  HeciRegHCsr.r.H_IG  = 1;
  HeciRegHCsr.r.H_RDY = 1;
  MMIOWRITEDWORD (HeciMBAR + H_CSR, HeciRegHCsr.ul);

  return EFI_SUCCESS;
}

UINT8
FilledSlots (
  IN  UINT32 ReadPointer,
  IN  UINT32 WritePointer
)
/*++

  Routine Description:
    Calculate if the circular buffer has overflowed.
    Corresponds to HECI HPS (part of) section 4.2.1

  Arguments:
    ReadPointer  - Location of the read pointer.
    WritePointer - Location of the write pointer.

  Returns:
    Number of filled slots.

--*/
{
  UINT8 FilledSlots;

  //
  // Calculation documented in HECI HPS 0.68 section 4.2.1
  //
  FilledSlots = (((INT8) WritePointer) - ((INT8) ReadPointer));

  return FilledSlots;
}

EFI_STATUS
OverflowCB (
  IN  UINT32 ReadPointer,
  IN  UINT32 WritePointer,
  IN  UINT32 BufferDepth
)
/*++

  Routine Description:
    Calculate if the circular buffer has overflowed
    Corresponds to HECI HPS (part of) section 4.2.1

  Arguments:
    ReadPointer - Value read from host/me read pointer
    WritePointer - Value read from host/me write pointer
    BufferDepth - Value read from buffer depth register

  Returns:
    EFI_STATUS

--*/
{
  UINT8 FilledSlots;

  //
  // Calculation documented in HECI HPS 0.68 section 4.2.1
  //
  FilledSlots = (((INT8) WritePointer) - ((INT8) ReadPointer));

  //
  // test for overflow
  //
  if (FilledSlots > ((UINT8) BufferDepth)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciGetSeCStatus (
  IN UINT32                     *SeCStatus
)
/*++

  Routine Description:
    Return SEC Status

  Arguments:
    SeCStatus pointer for status report

  Returns:
    EFI_STATUS

--*/
{
  HECI_FWS_REGISTER SeCFirmwareStatus;

  if (SeCStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_NORMAL && SeCFirmwareStatus.r.ErrorCode == SEC_ERROR_CODE_NO_ERROR) {
    *SeCStatus = SEC_READY;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_RECOVERY) {
    *SeCStatus = SEC_IN_RECOVERY_MODE;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_INIT) {
    *SeCStatus = SEC_INITIALIZING;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_DISABLE_WAIT) {
    *SeCStatus = SEC_DISABLE_WAIT;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_TRANSITION) {
    *SeCStatus = SEC_TRANSITION;
  } else {
    *SeCStatus = SEC_NOT_READY;
  }

  if (SeCFirmwareStatus.r.FwUpdateInprogress) {
    *SeCStatus |= SEC_FW_UPDATES_IN_PROGRESS;
  }

  if (SeCFirmwareStatus.r.FwInitComplete == SEC_FIRMWARE_COMPLETED) {
    *SeCStatus |= SEC_FW_INIT_COMPLETE;
  }

  if (SeCFirmwareStatus.r.SeCBootOptionsPresent == SEC_BOOT_OPTIONS_PRESENT) {
    *SeCStatus |= SEC_FW_BOOT_OPTIONS_PRESENT;
  }

  DEBUG ((EFI_D_ERROR, "HECI SeCStatus %X\n", SeCFirmwareStatus.ul));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciGetSeCMode (
  IN UINT32                     *SeCMode
)
/*++

  Routine Description:
    Return SEC Mode

  Arguments:
    SeCMode pointer for SEC Mode report

  Returns:
    EFI_STATUS

--*/
{
  HECI_FWS_REGISTER SeCFirmwareStatus;

  if (SeCMode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  switch (SeCFirmwareStatus.r.SeCOperationMode) {
    case SEC_OPERATION_MODE_NORMAL:
      *SeCMode = SEC_MODE_NORMAL;
      break;

    case SEC_OPERATION_MODE_ALT_DISABLED:
      *SeCMode = SEC_DEBUG_MODE_ALT_DIS; //debug Mode
      break;

    case SEC_OPERATION_MODE_SOFT_TEMP_DISABLE:
      *SeCMode = SEC_MODE_TEMP_DISABLED;
      break;

    case SEC_OPERATION_MODE_SECOVR_JMPR:
    case SEC_OPERATION_MODE_SECOVR_HECI_MSG:
      *SeCMode = SEC_MODE_SECOVER;
      break;

    default:
      *SeCMode = SEC_MODE_FAILED;
  }

  DEBUG ((EFI_D_ERROR, "HECI SeCMode %X\n", SeCFirmwareStatus.r.SeCOperationMode));

  return EFI_SUCCESS;
}


EFI_STATUS
SeCAlivenessRequest (
  IN      UINT32                       *HeciMemBar,
  IN      UINT32            Request
)
{
  UINT32                        HostAlivenessResponse;
  UINT32                            TimerStart;
  UINT32                            TimerEnd;
//  volatile UINT32                   *HpetTimer;

  //
  // Wait for SEC ready
  //
  //
  // Check for SEC ready status
  //
  StartTimer (&TimerStart, &TimerEnd, PEI_HECI_INIT_TIMEOUT);

  if(Request == 1)
    Mmio32Or(*HeciMemBar, R_SICR_HOST_ALIVENESS_REQ, Request);
  else
    Mmio32And(*HeciMemBar, R_SICR_HOST_ALIVENESS_REQ, 0xfffffffE);

  HostAlivenessResponse =  Mmio32(*HeciMemBar, R_HICR_HOST_ALIVENESS_RESP);

//  Timeout=0;
  while((HostAlivenessResponse & Request) != Request) {
    HostAlivenessResponse = Mmio32(*HeciMemBar, R_HICR_HOST_ALIVENESS_RESP);
    //Timeout++;
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }
  }
  return EFI_SUCCESS;
}

