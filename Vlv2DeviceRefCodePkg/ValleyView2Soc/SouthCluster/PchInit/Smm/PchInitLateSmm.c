/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchInitLateSmm.c

  @brief
  This is the driver that initializes the Intel PCH for save/restore during S3.

--*/

#include "PchInitSmm.h"

///
/// Global Variables
///
UINT32    XhciMemoryBase = 0x90600000;
BOOLEAN PatchFlag = FALSE;

XHCI_MMIO_REGISTER_INFO    XhciMmioState[]= {/*Dnctrl*/{0x94, 0}, /*Dcbaap_Lo*/{0xB0, 0}, /*Dcbaap_Hi*/{0xB4, 0}, /*Config*/{0xB8, 0}, 
                       /*Erstsz1*/ {0x2028, 0}, {0x2048, 0}, {0x2068, 0}, {0x2088, 0}, {0x20A8, 0}, {0x20C8, 0}, {0x20E8, 0}, {0x2108, 0},
                  /*Erstba_Lo1*/{0x2030, 0}, {0x2034, 0}, {0x2050, 0}, {0x2054, 0}, {0x2070, 0}, {0x2074, 0}, {0x2090, 0}, {0x2094, 0},
                                         {0x20B0, 0}, {0x20B4, 0}, {0x20D0, 0}, {0x20D4, 0}, {0x20F0, 0}, {0x20F4, 0}, {0x2110, 0}, {0x2114, 0},
                  /*Erdp_Lo1*/   {0x2038, 0}, {0x203C, 0}, {0x2058, 0}, {0x205C, 0}, {0x2078, 0}, {0x207C, 0}, {0x2098, 0}, {0x209C, 0},
                                         {0x20B8, 0}, {0x20BC, 0}, {0x20D8, 0}, {0x20DC, 0}, {0x20F8, 0}, {0x20FC, 0}, {0x2118, 0}, {0x211C, 0},
                      /*Iman1*/   {0x2020, 0}, {0x2040, 0}, {0x2060, 0}, {0x2080, 0}, {0x20A0, 0}, {0x20C0, 0}, {0x20E0, 0}, {0x2100, 0},
                      /*Imod1*/   {0x2024, 0}, {0x2044, 0}, {0x2064, 0}, {0x2084, 0}, {0x20A4, 0}, {0x20C4, 0}, {0x20E4, 0}, {0x2104, 0}};

VOID
XhciSaveRuntimeRegister (
)
/**

  @brief
  Save the Xhci MMIO registers before S3

**/
{
  UINT8       SaveCmdRegister;
  UINT16     SavePmcRegister;
  UINT32      PmcBase;
  UINT32	    Index;
  UINT32     SaveMemoryBaseRegister;
  UINTN       PciD31F0RegBase;
  UINTN       XhciPciMmBase;



  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0
                    );
  PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  ///
  /// Check if XHCI controller is enabled
  ///
  if ((MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS) & (UINT32) B_PCH_PMC_FUNC_DIS_USH) != 0) {
    return ;
  }
  
  XhciPciMmBase = MmPciAddress (
                    0,
                    DEFAULT_PCI_BUS_NUMBER_PCH,
                    PCI_DEVICE_NUMBER_PCH_XHCI,
                    PCI_FUNCTION_NUMBER_PCH_XHCI,
                    0
                    );

  SaveCmdRegister = MmioRead8 (XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER);
  SaveMemoryBaseRegister = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_MEM_BASE);
  XhciMemoryBase = SaveMemoryBaseRegister & B_PCH_XHCI_MEM_BASE_BA; //AMI_OVERRIDE - EIP166185 System will show USB-hot-plug event after S3 resume
  ///
  /// Bring back to D0
  ///
  SavePmcRegister = MmioRead16 (XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS);
  if ( SavePmcRegister & V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3) {
     MmioAnd16 ((XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), (UINT16) ~V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3);
     PchPmTimerStall (10 * 1000);

  }
  // Set memory base and enable MSE
  MmioWrite32((XhciPciMmBase + R_PCH_XHCI_MEM_BASE), XhciMemoryBase);
  MmioWrite8((XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER), B_PCH_XHCI_COMMAND_MSE);

  // Save runtime registers
  for (Index = 0; Index < (sizeof(XhciMmioState)/sizeof(XHCI_MMIO_REGISTER_INFO)); Index++) {
	XhciMmioState[Index].RegisterData	= *(UINT32 *)(UINTN)(XhciMemoryBase + XhciMmioState[Index].RegisterIndex);
  }

  MmioWrite8 ((XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER), SaveCmdRegister);
  MmioWrite32 ((XhciPciMmBase + R_PCH_XHCI_MEM_BASE), SaveMemoryBaseRegister);
  if ( SavePmcRegister & V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3) {
     MmioWrite16 ((XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), SavePmcRegister);
     PchPmTimerStall (10 * 1000);
  }

  PatchFlag = TRUE;
}

VOID
XhciPatchProcedure (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/**

  @brief
  BIOS flow to workaround Xhci registers after wake up from S3. This procedure to be called after Xhci script restore finished.

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
  UINT8	    SaveCmdRegister;
  UINT16     SavePmcRegister;
  UINT16     Count;
  UINT32     PmcBase;
  UINT32	    Index;
  UINT32	    SaveMemoryBase;
  UINT32     SaveHcsParam1;
  UINT32     SaveHcsParam3;
  UINT32     SaveHccParams;
  UINT32     SaveUSB2_PHY_PMC;
  UINT32     SaveUSB_PGC;
  UINT32     SaveXLTP_LTV1;
  UINTN       PciD31F0RegBase;
  UINTN       XhciPciMmBase;
  
  if (!PatchFlag) return;

  PatchFlag = FALSE;

  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0
                    );
  PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  ///
  /// Check if XHCI controller is enabled
  ///
  if ((MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS) & (UINT32) B_PCH_PMC_FUNC_DIS_USH) != 0) {
    return ;
  }

  XhciPciMmBase = MmPciAddress (
                    0,
                    DEFAULT_PCI_BUS_NUMBER_PCH,
                    PCI_DEVICE_NUMBER_PCH_XHCI,
                    PCI_FUNCTION_NUMBER_PCH_XHCI,
                    0
                    );

  SaveCmdRegister = MmioRead8 (XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER);
  SaveMemoryBase = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_MEM_BASE);
  ///
  /// Bring back to D0
  ///
  SavePmcRegister = MmioRead16 (XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS);
  if ( SavePmcRegister & V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3) {
     MmioAnd16 ((XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), (UINT16) ~V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3);
     PchPmTimerStall (10 * 1000);
  }
  MmioWrite32((XhciPciMmBase + R_PCH_XHCI_MEM_BASE), XhciMemoryBase);
  MmioWrite8((XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER), (B_PCH_XHCI_COMMAND_MSE | B_PCH_XHCI_COMMAND_BME));

  SaveHcsParam1 = *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_HCSPARAMS1);
  SaveHcsParam3 = *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_HCSPARAMS3);
  SaveHccParams = *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_HCCPARAMS);
  SaveUSB2_PHY_PMC = *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USB2_PHY_POWER_MANAGEMENT_CONTROL);
  SaveUSB_PGC = *(UINT32 *)(UINTN)(XhciMemoryBase + 0x8168);
  SaveXLTP_LTV1 = *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_LATENCY_TOLERANCE_PARAMETERS_LTV_CONTROL);
  
  // Prepare for restoration
  for (Index = 0; Index < (sizeof(XhciMmioState)/sizeof(XHCI_MMIO_REGISTER_INFO)); Index++) {
	*(UINT32 *)(UINTN)(XhciMemoryBase + XhciMmioState[Index].RegisterIndex) = XhciMmioState[Index].RegisterData;
  }
  // Trigger restore CRS
  (*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBCMD)) |= B_PCH_XHCI_USBCMD_CRS;
     PchPmTimerStall (1);

  // Waiting for RSS = 0
  Count = TIMER_1MS;
  while(((*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBSTS)) & B_PCH_XHCI_USBSTS_RSS) && (Count != 0)){
     PchPmTimerStall (1);
       Count--;
  }
  if (Count == 0) goto PatchExit;

  // Patch the wrong registers
  *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_HCSPARAMS1) = SaveHcsParam1;
  *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_HCSPARAMS3) = SaveHcsParam3;
  *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_HCCPARAMS) = SaveHccParams;
  *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USB2_PHY_POWER_MANAGEMENT_CONTROL) = SaveUSB2_PHY_PMC;
  *(UINT32 *)(UINTN)(XhciMemoryBase + 0x8168) = SaveUSB_PGC;
  *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_LATENCY_TOLERANCE_PARAMETERS_LTV_CONTROL) = SaveXLTP_LTV1;

   // Pulse Run/Stop bit
   (*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBCMD)) |= B_PCH_XHCI_USBCMD_RS;
   PchPmTimerStall (1);
   Count = 20;
   while(((*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBSTS)) & B_PCH_XHCI_USBSTS_HCH) && (Count != 0)){
       PchPmTimerStall (TIMER_1MS);
       Count--;
   }
   PchPmTimerStall (TIMER_1MS);
   (*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBCMD)) &= ~B_PCH_XHCI_USBCMD_RS;
   PchPmTimerStall (1);
   Count = 20;
   while(((~*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBSTS)) & B_PCH_XHCI_USBSTS_HCH) && (Count != 0)){
       PchPmTimerStall (TIMER_1MS);
       Count--;
  }

   // Trigger Save CSS
  *(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBCMD) |= B_PCH_XHCI_USBCMD_CSS;
     PchPmTimerStall (1);

  // Waiting for SSS = 0
  Count = TIMER_1MS;
  while(((*(UINT32 *)(UINTN)(XhciMemoryBase + R_PCH_XHCI_USBSTS)) & B_PCH_XHCI_USBSTS_SSS) && (Count != 0)){
     PchPmTimerStall (1);
       Count--;
  }
  if (Count == 0) goto PatchExit;

PatchExit:
	
   // lock ACCTRL
  MmioWrite8(XhciPciMmBase + R_PCH_XHCI_XHCC1 + 3, MmioRead8(XhciPciMmBase + R_PCH_XHCI_XHCC1 + 3) | 0x80);

  MmioWrite8((XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER), SaveCmdRegister);
  MmioWrite32((XhciPciMmBase + R_PCH_XHCI_MEM_BASE), SaveMemoryBase);
  if ( SavePmcRegister & V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3) {
     MmioWrite16 ((XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), SavePmcRegister);
     PchPmTimerStall (10 * 1000);
  }
}

EFI_STATUS
EFIAPI
PchS3EntryCallBack (
  IN  EFI_HANDLE                        Handle,
  IN  EFI_SMM_SX_DISPATCH_CONTEXT       *Context
  )
/**
  This function save PCH register before enter S3

  @param[in] Handle    Handle of the callback
  @param[in] Context   The dispatch context

  @retval EFI_SUCCESS  PCH register saved
**/
{

  XhciSaveRuntimeRegister();
  
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PchInitLateSmm (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/**
  Initializes the PCH SMM handler for PCH save and restore

  @param[in] ImageHandle - Handle for the image of this driver
  @param[in] SystemTable - Pointer to the EFI System Table

  @retval EFI_SUCCESS    - PCH SMM handler was installed
**/
{
  EFI_STATUS                                         Status;
//AMI_OVERRIDE - EIP166185 System will show USB-hot-plug event after S3 resume   EFI_PHYSICAL_ADDRESS                      MemBase;
  EFI_HANDLE                                         SxDispatchHandle;
  EFI_SMM_SX_DISPATCH_CONTEXT       SxDispatchContext;
  EFI_SMM_SW_DISPATCH_CONTEXT       SwContext;
  EFI_HANDLE                                         SwHandle;

  DEBUG ((DEBUG_INFO, "PchInitLateSmm()  Start\n"));

  if (PchPlatformPchPolicy->XhciWorkaroundSwSmiNumber == 0)
    return EFI_SUCCESS;
  //
  // Register the callback for S3 entry
  //
  SxDispatchContext.Type  = SxS3;
  SxDispatchContext.Phase = SxEntry;
  Status = mSxDispatch->Register (
                                mSxDispatch,
                                PchS3EntryCallBack,
                                &SxDispatchContext,
                                &SxDispatchHandle
                                );
  ASSERT_EFI_ERROR (Status);

  //AMI_OVERRIDE - EIP166185 System will show USB-hot-plug event after S3 resume >>
  /*
  MemBase = 0x0ffffffff;
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchTopDown,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  16,  // 2^16: 64K Alignment
                  0x10000,     // 64K Length
                  &MemBase,
                  ImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  XhciMemoryBase = (UINT32)MemBase;
  */
  //AMI_OVERRIDE - EIP166185 System will show USB-hot-plug event after S3 resume <<
  
    ///
    /// Register BIOS Lock SW SMI handler
    ///
    SwContext.SwSmiInputValue = PchPlatformPchPolicy->XhciWorkaroundSwSmiNumber;
    Status = mSwDispatch->Register (
                            mSwDispatch,
                            XhciPatchProcedure,
                            &SwContext,
                            &SwHandle
                            );

  DEBUG ((DEBUG_INFO, "PchInitLateSmm() End\n"));

  return EFI_SUCCESS;
}
