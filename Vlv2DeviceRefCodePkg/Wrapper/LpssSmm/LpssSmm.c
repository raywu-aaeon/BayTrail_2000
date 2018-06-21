//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
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

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        LpssSmm.c
//
// Description: Provide functions to restore MMC devices for S3 resume
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <LpssSmm.h>
#include <token.h>


//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)
UINT32 mSccMmioBase0 = 0;
UINT32 mSccMmioBase1 = 0;
UINT32 mSdCardMmioBase0 = 0; //EIP143364 
UINT32 mSdCardMmioBase1 = 0; //EIP143364 


EFI_STATUS
EFIAPI
S3SccAcpiModeEnable(
    UINT8       EmmcMode
)
{
    BOOLEAN     SetAcpiMode;
    UINT32      AcpiModeRegOffset;
    UINT32      AcpiModeRegValue;
    UINT32      Buffer32;

    SetAcpiMode = FALSE;
    AcpiModeRegValue = (B_PCH_SCC_EP_PCICFGCTR1_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR1_PCI_CFG_DIS1);
    
    if (EmmcMode == EMMC_441) {
      AcpiModeRegOffset = R_PCH_SCC_EP_PCICFGCTR1;
      SetAcpiMode = TRUE;
    }
     
    if (EmmcMode == EMMC_45) {
      AcpiModeRegOffset = R_PCH_SCC_EP_PCICFGCTR4;
      SetAcpiMode = TRUE;
    }
    
	//EIP143364 >>
    if (EmmcMode == SD_CARD) {
      AcpiModeRegOffset = R_PCH_SCC_EP_PCICFGCTR2;
      SetAcpiMode = TRUE;
    }
	//EIP143364 <<

    if (SetAcpiMode == TRUE) {
      SccMsgBus32AndThenOr(
          AcpiModeRegOffset,
          Buffer32,
          0xFFFFFFFF,
          AcpiModeRegValue
      );

      SccMsgBusRead32(
          AcpiModeRegOffset,
          Buffer32
      );
      TRACE((TRACE_ALWAYS, "Buffer32 = %x\n", Buffer32));
    }

    return EFI_SUCCESS;
}

EFI_STATUS
Emmc441SwSmiCallback(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL
    )
/**

  @brief
  Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  SMM_BWP and BLE bits will be set here

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
    UINTN         SccPciMmBase;

    TRACE((TRACE_ALWAYS, "eMMC441SwSmiCallback ...\n"));

    SccPciMmBase = CSP_PCIE_CFG_ADDRESS (0, 16, 0, 0);
    
    if (mSccMmioBase0 == 0) {
      mSccMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      mSccMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      TRACE((TRACE_ALWAYS, "Save: mSccMmioBase0 = %x\n", mSccMmioBase0));
      TRACE((TRACE_ALWAYS, "Save: mSccMmioBase1 = %x\n", mSccMmioBase1));
    } else {
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) mSccMmioBase0);
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1), (UINT32) mSccMmioBase1);
      MmioOr32 (
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
        (UINT32) (B_PCH_SCC_SDIO_STSCMD_INTRDIS | B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE)
      );
      mSccMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      mSccMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      TRACE((TRACE_ALWAYS, "Restore: mSccMmioBase0 = %x\n", mSccMmioBase0));
      TRACE((TRACE_ALWAYS, "Restore: mSccMmioBase1 = %x\n", mSccMmioBase1));
      TRACE((TRACE_ALWAYS, "mSccMmioBase0[0] = %x\n", MmioRead32 ((UINTN) mSccMmioBase0)));
      TRACE((TRACE_ALWAYS, "mSccMmioBase1[0] = %x\n", MmioRead32 ((UINTN) mSccMmioBase1)));
      S3SccAcpiModeEnable(EMMC_441);
    }
    
    return EFI_SUCCESS;
}

EFI_STATUS
Emmc45SwSmiCallback(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL
    )
/**

  @brief
  Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  SMM_BWP and BLE bits will be set here

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
    UINTN         SccPciMmBase;

    TRACE((TRACE_ALWAYS, "eMMC45SwSmiCallback ...\n"));

    SccPciMmBase = CSP_PCIE_CFG_ADDRESS (0, 23, 0, 0);
    
    if (mSccMmioBase0 == 0) {
      mSccMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      mSccMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      TRACE((TRACE_ALWAYS, "Save: mSccMmioBase0 = %x\n", mSccMmioBase0));
      TRACE((TRACE_ALWAYS, "Save: mSccMmioBase1 = %x\n", mSccMmioBase1));
    } else {
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) mSccMmioBase0);
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1), (UINT32) mSccMmioBase1);
      MmioOr32 (
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
        (UINT32) (B_PCH_SCC_SDIO_STSCMD_INTRDIS | B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE)
      );
      mSccMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      mSccMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      TRACE((TRACE_ALWAYS, "Restore: mSccMmioBase0 = %x\n", mSccMmioBase0));
      TRACE((TRACE_ALWAYS, "Restore: mSccMmioBase1 = %x\n", mSccMmioBase1));
      S3SccAcpiModeEnable(EMMC_45);
    }
    
    return EFI_SUCCESS;
}

//EIP143364 >>
EFI_STATUS
SdCardSwSmiCallback(
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL
    )
/**

  @brief
  Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  SMM_BWP and BLE bits will be set here

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
    UINTN         SccPciMmBase;

    TRACE((TRACE_ALWAYS, "SdCardSwSmiCallback ...\n"));

    SccPciMmBase = CSP_PCIE_CFG_ADDRESS (0, 18, 0, 0);
    
    if (mSdCardMmioBase0 == 0) {
      mSdCardMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      mSdCardMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      TRACE((TRACE_ALWAYS, "Save: mSdCardMmioBase0 = %x\n", mSdCardMmioBase0));
      TRACE((TRACE_ALWAYS, "Save: mSdCardMmioBase1 = %x\n", mSdCardMmioBase1));
    } else {
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) mSdCardMmioBase0);
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1), (UINT32) mSdCardMmioBase1);
      MmioOr32 (
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
        (UINT32) (B_PCH_SCC_SDIO_STSCMD_INTRDIS | B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE)
      );
      mSdCardMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      mSdCardMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      TRACE((TRACE_ALWAYS, "Restore: mSdCardMmioBase0 = %x\n", mSdCardMmioBase0));
      TRACE((TRACE_ALWAYS, "Restore: mSdCardMmioBase1 = %x\n", mSdCardMmioBase1));
      S3SccAcpiModeEnable(SD_CARD);
      //
      // Bit 0~2 is the GPIO function indication, use F0 under ACPI mode for SD card
      // In PCI mode needn't to save this because PlatformEarlyInit will be dispatched during S3 resume
      //
      MmioWrite32 ((UINTN) (IO_BASE_ADDRESS + 0x03A0), 0x80);
    }
    
    return EFI_SUCCESS;
}
//EIP143364 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   LpssSmmInSmmFunction
//
// Description: This function is part of the Lpss Smm driver and invoked during
//              SMM initialization.  As the name suggests this function is
//              called from SMM
//
// Input:       IN  EFI_HANDLE ImageHandle - Handle for this FFS image
//              IN  EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LpssSmmInSmmFunction(
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS                    Status;
    EFI_HANDLE                    Handle = NULL;
    EFI_SMM_SW_DISPATCH2_PROTOCOL *SwDispatch = NULL;
    EFI_HANDLE                    DummyHandle = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT   Emmc441Context = {EMMC_441_SW_SMI};
    EFI_SMM_SW_REGISTER_CONTEXT   Emmc45Context = {EMMC_45_SW_SMI};
    EFI_SMM_SW_REGISTER_CONTEXT   SdCardContext = {SD_CARD_SW_SMI}; //EIP143364 

    Status = InitAmiSmmLib(ImageHandle, SystemTable);
    if(EFI_ERROR(Status)) return Status;

    Status = pSmst->SmmLocateProtocol(&gEfiSmmSwDispatch2ProtocolGuid , \
                                      NULL, \
                                      &SwDispatch);
    if(EFI_ERROR(Status)) return Status;

    ///
    /// Register EMMC 4.41 SW SMI handler
    ///
    Status = SwDispatch->Register(SwDispatch, \
                                  Emmc441SwSmiCallback, \
                                  &Emmc441Context, \
                                  &Handle);
    TRACE((TRACE_ALWAYS, "Register Emmc441SwSmiCallback = %r\n", Status));
    if(EFI_ERROR(Status)) return Status;
    
    ///
    /// Register EMMC 4.5 SW SMI handler
    ///
    Status = SwDispatch->Register(SwDispatch, \
                                  Emmc45SwSmiCallback, \
                                  &Emmc45Context, \
                                  &Handle);
    TRACE((TRACE_ALWAYS, "Register Emmc45SwSmiCallback = %r\n", Status));
    if(EFI_ERROR(Status)) return Status; 

	//EIP143364 >> 
    ///
    /// Register SD CARD SW SMI handler
    ///
    Status = SwDispatch->Register(SwDispatch, \
                                  SdCardSwSmiCallback, \
                                  &SdCardContext, \
                                  &Handle);
    TRACE((TRACE_ALWAYS, "Register SdCardSwSmiCallback = %r\n", Status));
    if(EFI_ERROR(Status)) return Status;
	//EIP143364 <<

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   LpssSmmInit
//
// Description: This function is the entry point for Lpss Smm driver.
//
// Input:       IN  EFI_HANDLE ImageHandle - Handle for this FFS image
//              IN  EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
LpssSmmInit(
    IN  EFI_HANDLE              ImageHandle,
    IN  EFI_SYSTEM_TABLE        *SystemTable
)
{
    InitAmiLib(ImageHandle,SystemTable);

    return InitSmmHandler(ImageHandle,
                          SystemTable,
                          LpssSmmInSmmFunction,
                          NULL);
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
