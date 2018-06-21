/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiPlatform.c

Abstract:

  ACPI Platform Driver

 
--*/

#include "AcpiPlatform.h"


extern EFI_GUID gEfiSetupVariableGuid;


EFI_ACPI_SUPPORT_PROTOCOL     *mEfiAcpiSupport=NULL;
EFI_ACPI_TABLE_PROTOCOL       *mAcpiTableProtocol=NULL;
EFI_PLATFORM_INFO_HOB         *mPlatformInfo;
SETUP_DATA                    mSystemConfiguration;
UINTN                         mMcfgTableKey=0;



VOID *GetDsdtTable ()
{
  INTN    Index;
  PACPI_HDR Table;
  EFI_ACPI_TABLE_VERSION  Version;
  UINTN   Handle;
  EFI_STATUS  Status;

  for (Index = 0;;++Index) {
    Status = mEfiAcpiSupport->GetAcpiTable(
                                mEfiAcpiSupport,
                                Index,
                                &Table,
                                &Version,
                                &Handle);
    if (EFI_ERROR(Status)) return 0;
    if (((PACPI_HDR)Table)->Signature == FACP_SIG) return(VOID*)(UINTN)((PFACP32)Table)->DSDT;

  }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   BuildHpetTable
//
// Description: This function will build Hpet Table
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID BuildHpetTable (
    VOID
)
{
  EFI_STATUS                  Status = EFI_SUCCESS;
  HPET_20                     *Hpet;
  UINT8                       OemId[6] = ACPI_OEM_ID_MAK; //EIP134732
  UINT8                       OemTblId[8] = ACPI_OEM_TBL_ID_MAK;  //EIP134732
  UINT64                      HpetBase = HPET_BASE_ADDRESS;
  UINTN                       HpetTblHandle;


  TRACE((-1,"BuildHpetTable Starts....\n"));

  Hpet = MallocZ(sizeof(HPET_20));
  ASSERT(Hpet);
  if (Hpet) {
    // Fill Table header;
    Hpet->Header.Signature = HPET_SIG;
    Hpet->Header.Length    = sizeof(HPET_20);
    Hpet->Header.Revision  = 1;
    Hpet->Header.Checksum  = 0;
    pBS->CopyMem (&(Hpet->Header.OemId[0]), OemId, 6);
    pBS->CopyMem (&(Hpet->Header.OemTblId[0]), OemTblId, 8);
    Hpet->Header.OemRev     = ACPI_OEM_REV;
    Hpet->Header.CreatorId  = 0x2e494d41;//"AMI."
    Hpet->Header.CreatorRev = CORE_REVISION;

    // Fill HPET Fields
    // The GAS structure
    Hpet->BaseAddress.AddrSpcID   = GAS_SYS_MEM;
    Hpet->BaseAddress.RegBitWidth = 64;
    Hpet->BaseAddress.RegBitOffs  = 0;
    // Base address of 1K HPET RegBlock space
    Hpet->BaseAddress.Address = HpetBase;

    Hpet->EvtTmrBlockId.TMR_BLK_ID = *(UINT32*)(UINTN)HpetBase;

    Hpet->MinTickPeriod = 0x80; // Referred by Intel

    // Add table
    Status = mEfiAcpiSupport->SetAcpiTable(
                                mEfiAcpiSupport, \
                                Hpet, \
                                TRUE, \
                                EFI_ACPI_TABLE_VERSION_ALL, \
                                &HpetTblHandle
                                );
    TRACE((-1,"ACPISupport.SetAcpiTable() = %r \n", Status));
    ASSERT_EFI_ERROR(Status);

    // Free memory used for table image
    pBS->FreePool(Hpet);
  }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   BuildMcfgTable
//
// Description: This function will build the MCFG ACPI table
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID BuildMcfgTable (
    VOID
)
{
  EFI_STATUS                  Status = 0;
  MCFG_20                     *Mcfg;
  UINT8                       OemId[6] = ACPI_OEM_ID_MAK;  //EIP134732
  UINT8                       OemTblId[8] = ACPI_OEM_TBL_ID_MAK;  //EIP134732

  TRACE((-1,"BuildMcfgTable Starts....\n"));
  Mcfg = MallocZ(sizeof(MCFG_20));
  ASSERT(Mcfg);
  if(!Mcfg) return;

  // Fill Table header;
  Mcfg->Header.Signature = MCFG_SIG;
  Mcfg->Header.Length = sizeof(MCFG_20);
  Mcfg->Header.Revision = 1;
  Mcfg->Header.Checksum = 0;
  pBS->CopyMem (&(Mcfg->Header.OemId[0]), OemId, 6);
  pBS->CopyMem (&(Mcfg->Header.OemTblId[0]), OemTblId, 8);
  Mcfg->Header.OemRev = ACPI_OEM_REV;
  Mcfg->Header.CreatorId = 0x5446534d;//"MSFT" 4D 53 46 54
  Mcfg->Header.CreatorRev = 0x97;

  // Fill MCFG Fields

  // Base address of 256/128/64MB extended config space
  Mcfg->BaseAddr = PCIEX_BASE_ADDRESS;
  // Segment # of PCI Bus
  Mcfg->PciSeg = 0;
  // Start bus number of PCI segment
  Mcfg->StartBus = 0;
  // End bus number of PCI segment
  Mcfg->EndBus = ((PCIEX_LENGTH/0x100000)-1);

  // Add table
  Status = mAcpiTableProtocol->InstallAcpiTable (
                                mAcpiTableProtocol,
                                Mcfg,
                                sizeof(MCFG_20),
                                &mMcfgTableKey
                                );
  TRACE((TRACE_ALWAYS, "Installing AcpiTable (MCFG) = %r \n", Status));
  ASSERT_EFI_ERROR(Status);

  // Free memory used for table image
  pBS->FreePool(Mcfg);
}

//EIP150027 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   AcpiPlatformReadyToBootCallBack
//
// Description: 
//              
//
// Input:       Event   - Event of callback
//              Context - Context of callback.
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AcpiPlatformReadyToBootCallBack (
    IN EFI_EVENT  Event,
    IN VOID       *Context )
{
    EFI_STATUS              Status;
    EFI_ACPI_SDT_HEADER     *Table = NULL;
    EFI_ACPI_TABLE_VERSION  Version;
    PFACP_20                FadtPointer;
    UINT16                  OLD_IAPC_BOOT_ARCH;
    UINTN                   TableHandle;
    UINTN                   VariableSize = sizeof(SETUP_DATA);
    INTN                    Index = 0;
    SETUP_DATA              *SetupData = NULL;
    
    if (mEfiAcpiSupport == NULL){
        // Find the AcpiSupport protocol
        Status = pBS->LocateProtocol(&gEfiAcpiSupportProtocolGuid, NULL, &mEfiAcpiSupport);
        ASSERT_EFI_ERROR (Status);
    }
    if (mAcpiTableProtocol == NULL){
        // Find the Acpi Table Protocol
        Status = pBS->LocateProtocol(&gEfiAcpiTableProtocolGuid, NULL, &mAcpiTableProtocol);
        ASSERT_EFI_ERROR (Status);
    }
    Status = pBS->AllocatePool( EfiBootServicesData,
                                VariableSize,
                                &SetupData );
    TRACE((-1, "Locate memory pool for Setup Data %r\n", Status));
    ASSERT_EFI_ERROR(Status);
    Status = GetEfiVariable(L"Setup", &gEfiSetupVariableGuid, NULL, &VariableSize, &SetupData);
    ASSERT_EFI_ERROR (Status);
    
    do{
        Status = mEfiAcpiSupport->GetAcpiTable( mEfiAcpiSupport, Index,
                &Table, &Version, &TableHandle);
        if (Status == EFI_NOT_FOUND) break;
        
        if (Table->Signature == FACP_SIG){
            // prepare updating ACPI_IA_BOOT_ARCH according Native PCIE
            // and Native ASPM setup Item
            FadtPointer = (PFACP_20) Table;
            OLD_IAPC_BOOT_ARCH = FadtPointer->IAPC_BOOT_ARCH;
            
            // if  Native ASPM is disabled, set FACP table to skip Native ASPM
            if ((SetupData->PciExpNative == 0) || (SetupData->NativeAspmEnable == 0)){
                FadtPointer->IAPC_BOOT_ARCH |= 0x10;
            }
            if (FadtPointer->IAPC_BOOT_ARCH != OLD_IAPC_BOOT_ARCH){
                TRACE((TRACE_ALWAYS, "Barret: Install Acpi Table..."));
                Status = mAcpiTableProtocol->InstallAcpiTable(mAcpiTableProtocol,
                                                              Table,
                                                              Table->Length,
                                                              &TableHandle);
                TRACE((TRACE_ALWAYS, "%r", Status));
                ASSERT_EFI_ERROR (Status);
            }
            pBS->FreePool (SetupData);
            pBS->FreePool (Table);
            break;
        }
        Index++;
    }while(1);
    
    // Kill the Event
    pBS->CloseEvent(Event);
}
//EIP150027 <<

EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Entry point for Acpi platform driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
{
	  EFI_STATUS                    Status,Status2; //EIP128694
	  UINTN                         VarSize;
	  EFI_HANDLE                    Handle;
	  PACPI_HDR                     DsdtTable;
	  UINT32                        Length;
	  UINT8                         *ptr;
	  ASL_OBJ_INFO                  ObjInfo;
	  SB_SETUP_DATA                 PchPolicyData;
	  NB_SETUP_DATA                 VlvPolicyData;
	  PCH_STEPPING                  ScStepping;
	  UINT32                        Data32;
	  UINT8                         RevId;
	  EFI_PEI_HOB_POINTERS          Hob;
	  DXE_VLV_PLATFORM_POLICY_PROTOCOL* mPlatformPolicy;    //(EIP134992+)
	  //CSP20140424_23(-)	  EFI_EVENT                     AcpiPlatformReadyToBootEvent; //EIP150027 

	  InitAmiLib(ImageHandle,SystemTable);
	  
	  TRACE((TRACE_ALWAYS, "AcpiPlatform Init....\n"));
	  //
	  // Update HOB variable for PCI resource information
	  // Get the HOB list.  If it is not present, then ASSERT.
	  //
	  Hob.Raw = GetFirstGuidHob(&gEfiPlatformInfoGuid);
	  ASSERT(Hob.Raw != NULL);
	  mPlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);
	  
	  //
	  // Search for the Memory Configuration GUID HOB.  If it is not present, then
	  // there's nothing we can do. It may not exist on the update path.
	  //
	  VarSize = sizeof(SETUP_DATA);
	  //EIP128694 >>
	  Status2 = pRS->GetVariable(
	                            L"Setup",
	                            &gEfiSetupVariableGuid,
	                            NULL,
	                            &VarSize,
	                            &mSystemConfiguration
	                            );
	  
	  TRACE((TRACE_ALWAYS, "AcpiPlatform GetVariable Status:%r\n",Status2));
	  //
	  // If GetVariable return error, we give a default value with mGlobalNvsArea.Area 
	  //
	  if(!EFI_ERROR(Status2)) {	  
		  GetSbSetupData (  (VOID*)pRS, &PchPolicyData, FALSE);
	  
		  GetNbSetupData (  (VOID*)pRS, &VlvPolicyData, FALSE);
	  }
	  //EIP128694 <<
	  //
	  // Find the AcpiSupport protocol
	  //
	  Status = pBS->LocateProtocol(&gEfiAcpiSupportProtocolGuid, NULL, &mEfiAcpiSupport);
	  ASSERT_EFI_ERROR (Status);

	  //
	  // Find the AcpiTable protocol
	  //
	  Status = pBS->LocateProtocol(&gEfiAcpiTableProtocolGuid, NULL, &mAcpiTableProtocol);
	  ASSERT_EFI_ERROR (Status);

	  //
	  // Find the Vlv Platform Policy protocol
	  //
	  Status = pBS->LocateProtocol(&gDxeVlvPlatformPolicyGuid, NULL, &mPlatformPolicy);    //(EIP134992+)

	  //
	  // Allocate and initialize the NVS area for SMM and ASL communication.
	  //
	  Status = pBS->AllocatePool (EfiACPIMemoryNVS, sizeof (EFI_GLOBAL_NVS_AREA), &mGlobalNvsArea.Area);
	  ASSERT_EFI_ERROR (Status);
	  
	  pBS->SetMem (mGlobalNvsArea.Area, sizeof (EFI_GLOBAL_NVS_AREA), 0);

	  DsdtTable = GetDsdtTable ();

	  if (DsdtTable) {
	      Length = DsdtTable->Length - sizeof(ACPI_HDR);
	      ptr = (UINT8*)DsdtTable + sizeof(ACPI_HDR);
	      Status = GetAslObj(ptr, Length, "GNVS", otOpReg, &ObjInfo);
	      ASSERT_EFI_ERROR (Status);

	      ptr = (UINT8*)ObjInfo.DataStart;
	      *(UINT32*)(ptr + 2) = (UINT32)(UINTN)mGlobalNvsArea.Area;      
	      *(UINT16*)(ptr + 2 + 5) = sizeof (EFI_GLOBAL_NVS_AREA); 
	      TRACE((TRACE_ALWAYS, "ACPI Global NVS Ptr=0x%X, Length=0x%X\n", (UINT32)(UINTN) mGlobalNvsArea.Area, sizeof (EFI_GLOBAL_NVS_AREA)));
	  }

	  //EIP128694 >>
	  //
	  // If GetVariable return error, we give a default value with mGlobalNvsArea.Area 
	  //
	  if(!EFI_ERROR(Status2)) {
		  //
		  // Update global NVS area for ASL and SMM init code to use
		  //
		  mGlobalNvsArea.Area->ApicEnable                 = 1;
		  mGlobalNvsArea.Area->EmaEnable                  = 0;

		  mGlobalNvsArea.Area->NumberOfBatteries          = 1;
		  mGlobalNvsArea.Area->BatteryCapacity0           = 100;
		  mGlobalNvsArea.Area->BatteryStatus0             = 84;
		  mGlobalNvsArea.Area->OnboardCom                 = 1;
		  mGlobalNvsArea.Area->IdeMode                    = 0;
		  mGlobalNvsArea.Area->PowerState                 = 1;

		  mGlobalNvsArea.Area->LogicalProcessorCount    = 4;
		  mGlobalNvsArea.Area->PassiveThermalTripPoint  = VlvPolicyData.PassiveThermalTripPoint;
		  mGlobalNvsArea.Area->PassiveTc1Value          = VlvPolicyData.PassiveTc1Value;
		  mGlobalNvsArea.Area->PassiveTc2Value          = VlvPolicyData.PassiveTc2Value;
		  mGlobalNvsArea.Area->PassiveTspValue          = VlvPolicyData.PassiveTspValue;
		  mGlobalNvsArea.Area->CriticalThermalTripPoint = VlvPolicyData.CriticalThermalTripPoint;

		  mGlobalNvsArea.Area->IgdPanelType             	= VlvPolicyData.IgdFlatPanel;
		  mGlobalNvsArea.Area->IgdPanelScaling         	  = VlvPolicyData.PanelScaling;
		  mGlobalNvsArea.Area->IgdSciSmiMode         	    = VlvPolicyData.IgdSciSmiMode;
      mGlobalNvsArea.Area->IgdPAVP                    = VlvPolicyData.PavpMode; //CSP20131018
		  mGlobalNvsArea.Area->IgdTvFormat              	= VlvPolicyData.IgdTvFormat;
		  mGlobalNvsArea.Area->IgdTvMinor               	= VlvPolicyData.IgdTvMinor;
		  mGlobalNvsArea.Area->IgdSscConfig             	= VlvPolicyData.IgdSscConfig;
		  mGlobalNvsArea.Area->IgdBiaConfig             	= VlvPolicyData.IgdBiaConfig;
		  mGlobalNvsArea.Area->IgdBlcConfig             	= VlvPolicyData.IgdBlcConfig;
		  mGlobalNvsArea.Area->IgdDvmtMemSize      	      = VlvPolicyData.IgdDvmtMemSize;

		  mGlobalNvsArea.Area->BacklightControlSupport  = VlvPolicyData.BacklightControlSupport;
		  mGlobalNvsArea.Area->BrightnessPercentage    = VlvPolicyData.BrightnessPercentage;
		  mGlobalNvsArea.Area->IgdState = VlvPolicyData.IgdState;
		  mGlobalNvsArea.Area->LidState = VlvPolicyData.LidStatus;

		  mGlobalNvsArea.Area->DeviceId1 = VlvPolicyData.DeviceId1;
		  mGlobalNvsArea.Area->DeviceId2 = VlvPolicyData.DeviceId2;
		  mGlobalNvsArea.Area->DeviceId3 = VlvPolicyData.DeviceId3;
		  mGlobalNvsArea.Area->DeviceId4 = VlvPolicyData.DeviceId4;
		  mGlobalNvsArea.Area->DeviceId5 = VlvPolicyData.DeviceId5;
		  mGlobalNvsArea.Area->NumberOfValidDeviceId = VlvPolicyData.NumberOfValidDeviceId;
		  mGlobalNvsArea.Area->CurrentDeviceList = VlvPolicyData.CurrentDeviceList;
		  mGlobalNvsArea.Area->PreviousDeviceList = VlvPolicyData.PreviousDeviceList;

		  //
		  // DPTF related
		  //
		  mGlobalNvsArea.Area->SdpProfile                               = mPlatformPolicy->DptfSettings.SdpProfile; //CSP20130819 Match RC 1.0.3 update //(EIP134992+)
		  mGlobalNvsArea.Area->DptfEnable                               = VlvPolicyData.EnableDptf;
		  mGlobalNvsArea.Area->DptfSysThermal0                          = VlvPolicyData.DptfSysThermal0;
		  mGlobalNvsArea.Area->DptfSysThermal1                          = VlvPolicyData.DptfSysThermal1;
		  mGlobalNvsArea.Area->DptfSysThermal2                          = VlvPolicyData.DptfSysThermal2;
		  mGlobalNvsArea.Area->DptfSysThermal3                          = VlvPolicyData.DptfSysThermal3;  
		  mGlobalNvsArea.Area->DptfSysThermal4                          = VlvPolicyData.DptfSysThermal4;    	
		  mGlobalNvsArea.Area->DptfCharger                              		= VlvPolicyData.DptfChargerDevice;
		  mGlobalNvsArea.Area->DptfDisplayDevice                       = VlvPolicyData.DptfDisplayDevice;
		  mGlobalNvsArea.Area->DptfSocDevice                            	= VlvPolicyData.DptfSocDevice;      
		  mGlobalNvsArea.Area->DptfProcessor                            		= VlvPolicyData.DptfProcessor;
		  mGlobalNvsArea.Area->DptfProcCriticalTemperature              = VlvPolicyData.CriticalThermalTripPoint;
		  mGlobalNvsArea.Area->DptfProcPassiveTemperature               = VlvPolicyData.PassiveThermalTripPoint;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature0          = VlvPolicyData.GenericCriticalTemp0;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature0           = VlvPolicyData.GenericPassiveTemp0;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature1          = VlvPolicyData.GenericCriticalTemp1;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature1           = VlvPolicyData.GenericPassiveTemp1;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature2          = VlvPolicyData.GenericCriticalTemp2;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature2           = VlvPolicyData.GenericPassiveTemp2;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature3          = VlvPolicyData.GenericCriticalTemp3;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature3           = VlvPolicyData.GenericPassiveTemp3;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature4          = VlvPolicyData.GenericCriticalTemp4;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature4           = VlvPolicyData.GenericPassiveTemp4;
		  mGlobalNvsArea.Area->CLpmSetting                              = VlvPolicyData.Clpm;
		  mGlobalNvsArea.Area->DptfSuperDbg                             = VlvPolicyData.SuperDebug;
		  mGlobalNvsArea.Area->LPOEnable                                = VlvPolicyData.LPOEnable;
		  mGlobalNvsArea.Area->LPOStartPState                           = VlvPolicyData.LPOStartPState;
		  mGlobalNvsArea.Area->LPOStepSize                              = VlvPolicyData.LPOStepSize;
		  mGlobalNvsArea.Area->LPOPowerControlSetting                   = VlvPolicyData.LPOPowerControlSetting;
		  mGlobalNvsArea.Area->LPOPerformanceControlSetting             = VlvPolicyData.LPOPerformanceControlSetting;
		  mGlobalNvsArea.Area->DppmEnabled                              = VlvPolicyData.EnableDppm;
		  mGlobalNvsArea.Area->AmbientTripPointChange                   = VlvPolicyData.AmbientTripPointChange; //P20130628

		  //
		  //
		  // Platform Flavor
		  //
		  mGlobalNvsArea.Area->PlatformFlavor = mPlatformInfo->PlatformFlavor; //P20120624_2 

		  //(EIP120879+)>>
		  // Update SOC Stepping
		  ScStepping = PchStepping();
		  mGlobalNvsArea.Area->SocStepping  = (UINT8) ScStepping;
		  mGlobalNvsArea.Area->WittEnable   = PchPolicyData.WittEnable;
		  //(EIP120879+)<<

          mGlobalNvsArea.Area->SarEnable         = 0;    // Need to check this, and GloblNvs only define it. //CSP20130910 - Match RC 1.1.0
		  mGlobalNvsArea.Area->OsSelection       = PchPolicyData.OsSelect;
		  mGlobalNvsArea.Area->LpssSccMode       = PchPolicyData.LpssPciModeEnabled; //CSP20130910 - Match RC 1.1.0
      
		  // Update FRC version 0.70 (EIP120879+)>>
		  if (PchPolicyData.eMMCEnabled== 1) {// Auto detect mode
			  TRACE((TRACE_ALWAYS, "Auto detect mode------------start\n"));
			  switch (ScStepping) {
			  case PchA0:
			  case PchA1:
				  TRACE((TRACE_ALWAYS, "eMMC 4.41 Configuration\n"));
				  mGlobalNvsArea.Area->emmcVersion              =  0;
				  break;
			  case PchB0:
			  //CSP20130910 >>	   
			  case PchB1:
			  case PchB2:
				  TRACE((TRACE_ALWAYS, "eMMC 4.5 Configuration\n"));
				  mGlobalNvsArea.Area->emmcVersion              =  1;
				  break;
			  default:
				  TRACE((TRACE_ALWAYS, "Unknown Stepping, eMMC 4.5 Configuration\n"));
				  mGlobalNvsArea.Area->emmcVersion              =  1;
				  break;
			  //CSP20130910 <<	   
			  }
		  } else if (PchPolicyData.eMMCEnabled == 2) { // eMMC 4.41
			  TRACE((TRACE_ALWAYS, "eMMC 4.41 Configuration\n"));
			  mGlobalNvsArea.Area->emmcVersion              =  0;
		  } else if (PchPolicyData.eMMCEnabled == 3) { // eMMC 4.5
			  TRACE((TRACE_ALWAYS, "eMMC 4.5 Configuration\n"));
			  mGlobalNvsArea.Area->emmcVersion              =  1;
		  } else { // Disable eMMC controllers
			  TRACE((TRACE_ALWAYS, "Disable eMMC controllers\n"));
			  mGlobalNvsArea.Area->emmcVersion              =  0;
		  }
	  
		  MsgBus32Read (VLV_BUNIT, BUNIT_BMBOUND, Data32);
		  mGlobalNvsArea.Area->BmBound      = Data32;
		  mGlobalNvsArea.Area->FsaStatus    = FSA_SUPPORT;// 0 - Fsa is off, 1- Fsa is on
	      
		  // Update the Platform id
		  mGlobalNvsArea.Area->BoardID      = mPlatformInfo->BoardId;

		  // Update the Platform id
		  mGlobalNvsArea.Area->FabID        = mPlatformInfo->BoardRev;

		  mGlobalNvsArea.Area->OtgMode      = PchPolicyData.PchUsbOtg;// 0- OTG disable 1- OTG PCI mode  
      
		  RevId = MmioRead8 (
				  	  MmPciAddress (0,
						  DEFAULT_PCI_BUS_NUMBER_PCH,
						  PCI_DEVICE_NUMBER_PCH_LPC,
						  PCI_FUNCTION_NUMBER_PCH_LPC,
						  R_PCH_LPC_RID_CC));
		  
		  mGlobalNvsArea.Area->Stepping     = RevId;// Stepping  
  
		  mGlobalNvsArea.Area->XhciMode     = PchPolicyData.PchUsb30Mode;
		  //
		  // Override invalid Pre-Boot Driver and XhciMode combination
		  //
		  //
		  // Override invalid Pre-Boot Driver and XhciMode combination
		  //
		  if ((PchPolicyData.PchUsbPreBootSupport == 0) && (PchPolicyData.PchUsb30Mode == 3)) { //CSP20130723_C
			  mGlobalNvsArea.Area->XhciMode               = 2;
		  }
		  if ((PchPolicyData.PchUsbPreBootSupport == 1) && (PchPolicyData.PchUsb30Mode == 2)) { //CSP20130723_C
			  mGlobalNvsArea.Area->XhciMode               = 3;
		  }
		  //PMIC is enabled by default. When it is disabled, we will not expose it in DSDT.
		  mGlobalNvsArea.Area->PmicEnable           = PchPolicyData.PmicEnable;
		  mGlobalNvsArea.Area->ISPDevSel            = VlvPolicyData.ISPDevSel;
		  mGlobalNvsArea.Area->LpeEnable            = PchPolicyData.Lpe;
		  mGlobalNvsArea.Area->UartSelection        = PchPolicyData.UartDebugEnable; //EIP133060
		  mGlobalNvsArea.Area->PcuUart1Enable       = PchPolicyData.PcuUart1;
		  mGlobalNvsArea.Area->PcuUart2Enable       = PchPolicyData.PcuUart2;
//EIP158981 >>
		  mGlobalNvsArea.Area->NfcEnable            = PchPolicyData.NfcEnable;
		  mGlobalNvsArea.Area->TouchPadEnable       = PchPolicyData.TouchPadEnable;
		  mGlobalNvsArea.Area->I2CTouchAddress      = PchPolicyData.I2CTouchAddress;
//EIP158981 <<
		  
		  mGlobalNvsArea.Area->S0ix                 = PchPolicyData.S0ixSupport;   //(EIP114446)
  
  
		  mGlobalNvsArea.Area->WPCN381U = GLOBAL_NVS_DEVICE_DISABLE;

		  mGlobalNvsArea.Area->DockedSioPresent = GLOBAL_NVS_DEVICE_DISABLE;		  
		  
		  mGlobalNvsArea.Area->NativePCIESupport          = mSystemConfiguration.PciExpNative; //EIP150027 
	  } else {
		  //
		  // Update global NVS area for ASL and SMM init code to use
		  //
		  mGlobalNvsArea.Area->ApicEnable                 = 1;
		  mGlobalNvsArea.Area->EmaEnable                  = 0;

		  mGlobalNvsArea.Area->NumberOfBatteries          = 1;
		  mGlobalNvsArea.Area->BatteryCapacity0           = 100;
		  mGlobalNvsArea.Area->BatteryStatus0             = 84;
		  mGlobalNvsArea.Area->OnboardCom                 = 1;
		  mGlobalNvsArea.Area->IdeMode                    = 0;
		  mGlobalNvsArea.Area->PowerState                 = 1;

		  mGlobalNvsArea.Area->LogicalProcessorCount    = 4;
		  mGlobalNvsArea.Area->PassiveThermalTripPoint  = 95;
		  mGlobalNvsArea.Area->PassiveTc1Value          = 0;
		  mGlobalNvsArea.Area->PassiveTc2Value          = 0;
		  mGlobalNvsArea.Area->PassiveTspValue          = 0;
		  mGlobalNvsArea.Area->CriticalThermalTripPoint = 100;

		  mGlobalNvsArea.Area->IgdPanelType             = 0;
		  mGlobalNvsArea.Area->IgdPanelScaling         	= 0;
		  mGlobalNvsArea.Area->IgdSciSmiMode         	  = 0;
      mGlobalNvsArea.Area->IgdPAVP                  = 1; //CSP20131018
		  mGlobalNvsArea.Area->IgdTvFormat              = 0;
		  mGlobalNvsArea.Area->IgdTvMinor               = 0;
		  mGlobalNvsArea.Area->IgdSscConfig             = 0;
		  mGlobalNvsArea.Area->IgdBiaConfig             = 0;
		  mGlobalNvsArea.Area->IgdBlcConfig             = 0;
		  mGlobalNvsArea.Area->IgdDvmtMemSize      		  = 2;

		  mGlobalNvsArea.Area->BacklightControlSupport  = 2;
		  mGlobalNvsArea.Area->BrightnessPercentage     = 100;
		  mGlobalNvsArea.Area->IgdState 				= 1;
		  mGlobalNvsArea.Area->LidState 				= 0;

		  mGlobalNvsArea.Area->DeviceId1 				= 0x80000100;
		  mGlobalNvsArea.Area->DeviceId2 				= 0x80000400;
		  mGlobalNvsArea.Area->DeviceId3 				= 0x80000200;
		  mGlobalNvsArea.Area->DeviceId4 				= 0x04;
		  mGlobalNvsArea.Area->DeviceId5 				= 0x05;
		  mGlobalNvsArea.Area->NumberOfValidDeviceId 	= 4;
		  mGlobalNvsArea.Area->CurrentDeviceList 		= 0x0F;
		  mGlobalNvsArea.Area->PreviousDeviceList 		= 0x0F;
		  
		  //
		  // DPTF related
		  //
		  mGlobalNvsArea.Area->DptfEnable               		= 0;
		  mGlobalNvsArea.Area->DptfSysThermal0          		= 0;
		  mGlobalNvsArea.Area->DptfSysThermal1          		= 1;
		  mGlobalNvsArea.Area->DptfSysThermal2          		= 1;
		  mGlobalNvsArea.Area->DptfSysThermal3          		= 0;  
		  mGlobalNvsArea.Area->DptfSysThermal4          		= 0;    	
		  mGlobalNvsArea.Area->DptfCharger              		= 1;
		  mGlobalNvsArea.Area->DptfDisplayDevice        		= 1;
		  mGlobalNvsArea.Area->DptfSocDevice                    = 1;      
		  mGlobalNvsArea.Area->DptfProcessor                    = 0;
		  mGlobalNvsArea.Area->DptfProcCriticalTemperature      = 100;
		  mGlobalNvsArea.Area->DptfProcPassiveTemperature       = 95;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature0  = 70;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature0   = 60;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature1  = 70;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature1   = 60;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature2  = 70;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature2   = 60;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature3  = 70;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature3   = 60;
		  mGlobalNvsArea.Area->DptfGenericCriticalTemperature4  = 70;
		  mGlobalNvsArea.Area->DptfGenericPassiveTemperature4   = 60;
		  mGlobalNvsArea.Area->CLpmSetting                      = 3;
		  mGlobalNvsArea.Area->DptfSuperDbg                     = 0;
		  mGlobalNvsArea.Area->LPOEnable                        = 0;
		  mGlobalNvsArea.Area->LPOStartPState                   = 0;
		  mGlobalNvsArea.Area->LPOStepSize                      = 25;
		  mGlobalNvsArea.Area->LPOPowerControlSetting           = 2;
		  mGlobalNvsArea.Area->LPOPerformanceControlSetting     = 2;
		  mGlobalNvsArea.Area->DppmEnabled                      = 1;
		  mGlobalNvsArea.Area->AmbientTripPointChange           = 1; //P20130628

		  //
		  //
		  // Platform Flavor
		  //
		  mGlobalNvsArea.Area->PlatformFlavor = mPlatformInfo->PlatformFlavor; //P20120624_2 

		  //(EIP120879+)>>
		  // Update SOC Stepping
		  ScStepping = PchStepping();
		  mGlobalNvsArea.Area->SocStepping  = (UINT8) ScStepping;
		  mGlobalNvsArea.Area->WittEnable   = 0;
		  //(EIP120879+)<<

		  // Update FRC version 0.70 (EIP120879+)>>
		  TRACE((TRACE_ALWAYS, "Disable eMMC controllers\n"));
		  mGlobalNvsArea.Area->emmcVersion              =  0;
	  
		  MsgBus32Read (VLV_BUNIT, BUNIT_BMBOUND, Data32);
		  mGlobalNvsArea.Area->BmBound      = Data32;
		  mGlobalNvsArea.Area->FsaStatus    = FSA_SUPPORT;// 0 - Fsa is off, 1- Fsa is on
	      
		  // Update the Platform id
		  mGlobalNvsArea.Area->BoardID      = mPlatformInfo->BoardId;

		  // Update the Platform id
		  mGlobalNvsArea.Area->FabID        = mPlatformInfo->BoardRev;

		  mGlobalNvsArea.Area->OtgMode      = 0;// 0- OTG disable 1- OTG PCI mode  
      
		  RevId = MmioRead8 (
				  	  MmPciAddress (0,
						  DEFAULT_PCI_BUS_NUMBER_PCH,
						  PCI_DEVICE_NUMBER_PCH_LPC,
						  PCI_FUNCTION_NUMBER_PCH_LPC,
						  R_PCH_LPC_RID_CC));
		  
		  mGlobalNvsArea.Area->Stepping     = RevId;// Stepping  
  
		  mGlobalNvsArea.Area->XhciMode     = 1;

		  //PMIC is enabled by default. When it is disabled, we will not expose it in DSDT.
		  mGlobalNvsArea.Area->PmicEnable           = 1;
		  mGlobalNvsArea.Area->ISPDevSel            = 1;
		  mGlobalNvsArea.Area->LpeEnable            = 0;
		  mGlobalNvsArea.Area->UartSelection        = DEFAULT_INTERNAL_UART_DEBUG_ENABLE; //EIP133060
		  mGlobalNvsArea.Area->PcuUart1Enable       = SOC_UART_PRESENT; //EIP133060
		  mGlobalNvsArea.Area->PcuUart2Enable       = 0;
//EIP158981 >>
      mGlobalNvsArea.Area->NfcEnable            = 0;
      mGlobalNvsArea.Area->TouchPadEnable       = 0;
//EIP158981 <<
		  mGlobalNvsArea.Area->I2CTouchAddress      = 0;
		  mGlobalNvsArea.Area->S0ix                 = 1;   //(EIP114446)
  
  
		  mGlobalNvsArea.Area->WPCN381U = GLOBAL_NVS_DEVICE_DISABLE;

		  mGlobalNvsArea.Area->DockedSioPresent = GLOBAL_NVS_DEVICE_DISABLE;
		  
		  mGlobalNvsArea.Area->NativePCIESupport    = 0; //EIP150027 
		  
		  TRACE((TRACE_ALWAYS, "GetVariable return error, mGlobalNvsArea.Area load default value\n"));
	  }
	  //EIP128694 <<
	
	  TRACE((TRACE_ALWAYS, "Dumping DPTF settings in global nvs init...\n"));
	  TRACE((TRACE_ALWAYS, "DPTFEnabled = %d\n", mGlobalNvsArea.Area->DptfEnable));
	  TRACE((TRACE_ALWAYS, "SdpProfile = %d\n", mGlobalNvsArea.Area->SdpProfile));    //(EIP134992+)
	  TRACE((TRACE_ALWAYS, "CpuParticipantCriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfProcCriticalTemperature));
	  TRACE((TRACE_ALWAYS, "CpuParticipantPassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfProcPassiveTemperature));  
	  TRACE((TRACE_ALWAYS, "GenParticipant0CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature0));  
	  TRACE((TRACE_ALWAYS, "GenParticipant0PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature0));    
	  TRACE((TRACE_ALWAYS, "GenParticipant1CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature1));  
	  TRACE((TRACE_ALWAYS, "GenParticipant1PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature1));      
	  TRACE((TRACE_ALWAYS, "GenParticipant2CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature2));  
	  TRACE((TRACE_ALWAYS, "GenParticipant2PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature2));
	  TRACE((TRACE_ALWAYS, "GenParticipant3CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature3));  
	  TRACE((TRACE_ALWAYS, "GenParticipant3PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature3));
	  TRACE((TRACE_ALWAYS, "GenParticipant4CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature4));  
	  TRACE((TRACE_ALWAYS, "GenParticipant4PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature4));      

	  TRACE((TRACE_ALWAYS, "ClpmSetting = %d\n", mGlobalNvsArea.Area->CLpmSetting));        
	  TRACE((TRACE_ALWAYS, "SuperDebug = %d\n", mGlobalNvsArea.Area->DptfSuperDbg));
	  TRACE((TRACE_ALWAYS, "LPOEnable = %d\n", mGlobalNvsArea.Area->LPOEnable));
	  TRACE((TRACE_ALWAYS, "LPOStartPState = %d\n", mGlobalNvsArea.Area->LPOStartPState));  
	  TRACE((TRACE_ALWAYS, "LPOStepSize = %d\n", mGlobalNvsArea.Area->LPOStepSize));        
	  TRACE((TRACE_ALWAYS, "LPOPowerControlSetting = %d\n", mGlobalNvsArea.Area->LPOPowerControlSetting));
	  TRACE((TRACE_ALWAYS, "LPOPerformanceControlSetting = %d\n", mGlobalNvsArea.Area->LPOPerformanceControlSetting));  
	  TRACE((TRACE_ALWAYS, "bDppmEnabled = %d\n", mGlobalNvsArea.Area->DppmEnabled));

	  TRACE((TRACE_ALWAYS, "DptfEnable = %d\n", mGlobalNvsArea.Area->DptfEnable));        
	  TRACE((TRACE_ALWAYS, "DptfSysThermal0 = %d\n", mGlobalNvsArea.Area->DptfSysThermal0));
	  TRACE((TRACE_ALWAYS, "DptfSysThermal1 = %d\n", mGlobalNvsArea.Area->DptfSysThermal1));
	  TRACE((TRACE_ALWAYS, "DptfSysThermal2 = %d\n", mGlobalNvsArea.Area->DptfSysThermal2));  
	  TRACE((TRACE_ALWAYS, "DptfSysThermal3 = %d\n", mGlobalNvsArea.Area->DptfSysThermal3));        
	  TRACE((TRACE_ALWAYS, "DptfCharger = %d\n", mGlobalNvsArea.Area->DptfCharger));
	  TRACE((TRACE_ALWAYS, "DptfDisplayDevice = %d\n", mGlobalNvsArea.Area->DptfDisplayDevice));  
	  TRACE((TRACE_ALWAYS, "DptfSocDevice = %d\n", mGlobalNvsArea.Area->DptfSocDevice));
	  TRACE((TRACE_ALWAYS, "DptfProcessor = %d\n", mGlobalNvsArea.Area->DptfProcessor));
	  
	  TRACE((TRACE_ALWAYS, "PlatformFlavor = %d\n", mGlobalNvsArea.Area->PlatformFlavor));
	  TRACE((TRACE_ALWAYS, "BoardID = %d\n", mGlobalNvsArea.Area->BoardID));
	  TRACE((TRACE_ALWAYS, "FabID = %d\n", mGlobalNvsArea.Area->FabID));
	  TRACE((TRACE_ALWAYS, "XhciMode = %d\n", mGlobalNvsArea.Area->XhciMode));
	  TRACE((TRACE_ALWAYS, "PmicEnable = %d\n", mGlobalNvsArea.Area->PmicEnable));
	  TRACE((TRACE_ALWAYS, "BatteryChargingSolution = %d\n", mGlobalNvsArea.Area->BatteryChargingSolution));
	  TRACE((TRACE_ALWAYS, "ISPDevSel = %d\n", mGlobalNvsArea.Area->ISPDevSel));
	  TRACE((TRACE_ALWAYS, "LpeEnable = %d\n", mGlobalNvsArea.Area->LpeEnable)); 
//EIP158981 >>
    TRACE((TRACE_ALWAYS, "NfcEnable = %d\n", mGlobalNvsArea.Area->NfcEnable));
    TRACE((TRACE_ALWAYS, "TouchPadEnable = %d\n", mGlobalNvsArea.Area->TouchPadEnable));
//EIP158981 <<	  
	  TRACE((TRACE_ALWAYS, "I2CTouchAddress = %d\n", mGlobalNvsArea.Area->I2CTouchAddress));

	  TRACE((TRACE_ALWAYS, "SocStepping = %d\n", mGlobalNvsArea.Area->SocStepping));
	  TRACE((TRACE_ALWAYS, "WittEnable = %d\n", mGlobalNvsArea.Area->WittEnable));
	  TRACE((TRACE_ALWAYS, "emmcVersion = %d\n", mGlobalNvsArea.Area->emmcVersion));
	  TRACE((TRACE_ALWAYS, "BmBound = %d\n", mGlobalNvsArea.Area->BmBound));
	  TRACE((TRACE_ALWAYS, "FsaStatus = %d\n", mGlobalNvsArea.Area->FsaStatus));
	  TRACE((TRACE_ALWAYS, "OtgMode = %d\n", mGlobalNvsArea.Area->OtgMode));
	  TRACE((TRACE_ALWAYS, "NativePCIESupport = %d\n", mGlobalNvsArea.Area->NativePCIESupport)); //EIP150027 

  mGlobalNvsArea.Area->SDIOMode = SettingSDIOMODE;
  Handle = NULL;
  Status = pBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiGlobalNvsAreaProtocolGuid,
                  &mGlobalNvsArea,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Install HPET Table
  //
  if(!EFI_ERROR(Status2)) {
	  if (mSystemConfiguration.HpetEnable)
		  	  BuildHpetTable ();	  
  } else {
	  BuildHpetTable ();
  }
  
  //
  // Install MCFG Table
  //
  //(EIP130199-) BuildMcfgTable ();
  
  //CSP20140424_23(-) >>
  //EIP150027 >>
  //Status = CreateReadyToBootEvent (
  //                 TPL_CALLBACK,
  //                 AcpiPlatformReadyToBootCallBack,
  //                 NULL,
  //                 &AcpiPlatformReadyToBootEvent);
  //EIP150027 <<
  //CSP20140424_23(-) <<
  
  //
  // Finished
  //
  return EFI_SUCCESS;
}
