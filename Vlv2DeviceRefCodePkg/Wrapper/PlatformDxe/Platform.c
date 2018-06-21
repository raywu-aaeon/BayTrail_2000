/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Platform.c

Abstract:

  Platform Initialization Driver.

 
--*/

#include "PlatformDxe.h"
#include "Platform.h"
#include "PchCommonDefinitions.h"
#include <Protocol/UsbPolicy.h>
#include <Protocol/PchPlatformPolicy.h>
#include <Protocol/TpmMp.h>
#include <Library/S3BootScriptLib.h>
#include <Guid/PciLanInfo.h>
#include <Guid/ItkData.h>
#include <Library/PciLib.h>
#include <PlatformBootMode.h>
#include <Guid/EventGroup.h>
#include <Protocol/SeCPlatformPolicy.h>
#include <Protocol/SeCOperation.h>
#include <Guid/Vlv2Variable.h>
#include <PchRegs/PchRegsLan.h>
#include <token.h>

EFI_GUID mPlatformDriverGuid = EFI_PLATFORM_DRIVER_GUID;
SETUP_DATA            mSystemConfiguration;
UINT32                mAttributes = 0; //EIP168675
EFI_HANDLE            mImageHandle;
BOOLEAN               mMfgMode = FALSE;
VOID                  *mDxePlatformStringPack;
UINT32                mPlatformBootMode = PLATFORM_NORMAL_MODE;
extern CHAR16 gItkDataVarName[];


UINT8 mSmbusRsvdAddresses[] = PLATFORM_SMBUS_RSVD_ADDRESSES;
UINT8 mNumberSmbusAddress = sizeof( mSmbusRsvdAddresses ) / sizeof( mSmbusRsvdAddresses[0] );
EFI_PLATFORM_INFO_HOB      mPlatformInfo;
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *mPciRootBridgeIo;
EFI_EVENT  mReadyToBootEvent;

UINT32 mSubsystemVidDid;
UINT32 mSubsystemAudioVidDid;

UINTN   mPciLanCount = 0;
VOID    *mPciLanInfo;

EFI_USB_POLICY_PROTOCOL         mUsbPolicyData = {0};

VOID
EfiOrMem(
    IN VOID   *Destination,
    IN VOID   *Source,
    IN UINTN  Length
);

#if defined(FIRMWARE_ID_BACKWARD_COMPATIBLE) && (FIRMWARE_ID_BACKWARD_COMPATIBLE != 0)
STATIC
VOID
InitFirmwareId();
#endif

EFI_STATUS
UsbLegacyPlatformInstall(
);

VOID
InitializeSlotInfo (
);

VOID
InitTcoReset(
);

VOID
InitItk();

VOID
InitExI ();

VOID
InitPlatformBootMode();



VOID
InitMfgAndConfigModeStateVar();

VOID
InitPchPlatformPolicy(
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN SB_SETUP_DATA            *PchPolicyData
);

VOID
InitVlvPlatformPolicy(
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN NB_SETUP_DATA            *VlvPolicyData
);

VOID
PchInitBeforeBoot(
);

VOID
UpdateDVMTSetup(
);

VOID
InitPlatformUsbPolicy(
    IN SB_SETUP_DATA             *PchPolicyData
);

VOID
InitSeC(
    VOID
);

VOID
InitTdtPolicy(
    VOID
);

//(CSP20130111D-)>>
/*
VOID
InitPciDevPME (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
///Program Gbe PME_EN
  PchMmPci32Or (0,
                PCI_BUS_NUMBER_PCH_LAN,
                PCI_DEVICE_NUMBER_PCH_LAN,
                PCI_FUNCTION_NUMBER_PCH_LAN,
                R_PCH_LAN_PMCS,
                B_PCH_LAN_PMCS_PMEE
                );

///Program EHCI PME_EN
  PchMmPci32Or (0,
                0,
                PCI_DEVICE_NUMBER_PCH_USB,
                PCI_FUNCTION_NUMBER_PCH_EHCI,
                R_PCH_EHCI_PWR_CNTL_STS,
                B_PCH_EHCI_PWR_CNTL_STS_PME_EN
                );
}
*/
//(CSP20130111D-)<<
#if defined SUPPORT_LVDS_DISPLAY && SUPPORT_LVDS_DISPLAY

#endif

EFI_BOOT_SCRIPT_SAVE_PROTOCOL *mBootScriptSave;

VOID
EnableAcpiCallback(
    IN EFI_EVENT        Event,
    IN VOID             *Context
)
/*++

Routine Description:
  SMI handler to enable ACPI mode

  Dispatched on reads from APM port with value 0xA0

  Disables the SW SMI Timer.
  ACPI events are disabled and ACPI event status is cleared.
  SCI mode is then enabled.

   Disable SW SMI Timer

   Clear all ACPI event status and disable all ACPI events
   Disable PM sources except power button
   Clear status bits

   Disable GPE0 sources
   Clear status bits

   Disable GPE1 sources
   Clear status bits

   Guarantee day-of-month alarm is invalid (ACPI 1.0 section 4.7.2.4)

   Enable SCI

Arguments:
  DispatchHandle  - EFI Handle
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:
  Nothing

--*/
{
  // EIP_141184 >>
  // Enable Azalia controller Memory Decode
  PchMmPci32Or (0, 
                0, 
                PCI_DEVICE_NUMBER_PCH_AZALIA, 
                PCI_FUNCTION_NUMBER_PCH_AZALIA, 
                R_PCH_HDA_STSCMD, 
                0x06
                );
  // EIP_141184 <<
  IoWrite8(SW_SMI_IO_ADDRESS, SW_SMI_ACPI_ENABLE); //P20120624_4 - Enable SW SMI. 
}



EFI_STATUS
EFIAPI
InitializePlatform(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
/*++

  Routine Description:

  This is the standard EFI driver point for the Driver. This
    driver is responsible for setting up any platform specific policy or
    initialization information.

  Arguments:

  ImageHandle  -  Handle for the image of this driver.
  SystemTable  -  Pointer to the EFI System Table.

  Returns:

  EFI_SUCCESS  -  Policy decisions set.

--*/
{
    EFI_STATUS                          Status;
    UINTN                               	 VarSize;
    EFI_HANDLE                        Handle;
    SB_SETUP_DATA                     PchPolicyData;
    NB_SETUP_DATA                     VlvPolicyData;

    EFI_EVENT                           mEfiExitBootServicesEvent;

    // Use DxeInitializeDriverLib so that gDS pointer is initialized.

    ReportStatusCodeEx(
        EFI_PROGRESS_CODE,
        EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_INIT,
        0,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
    );

    mImageHandle = ImageHandle;

//  Status = BootScriptSaveInitialize (ImageHandle, SystemTable);
//  ASSERT_EFI_ERROR (Status);

    //
    // Make a new handle and install the protocol
    //
    Handle = NULL;
    /*
      Status = gBS->InstallProtocolInterface (
                      &Handle,
                      &mSetupInfoGuid,
                      EFI_NATIVE_INTERFACE,
                      &mSetupInfo
                      );
    */

    Status = gBS->LocateProtocol(
                 &gEfiPciRootBridgeIoProtocolGuid,
                 NULL,
                 &mPciRootBridgeIo
             );
    ASSERT_EFI_ERROR(Status);

    VarSize = sizeof(EFI_PLATFORM_INFO_HOB);
    Status = gRT->GetVariable(
                 L"PlatformInfo",
                  &gEfiVlv2VariableGuid,
                 NULL,
                 &VarSize,
                 &mPlatformInfo
             );
    DEBUG((EFI_D_ERROR, "GetVariable PlatformInfo:%r  \n",Status));
    //
    // Initialize Product Board ID variable
    //

    InitMfgAndConfigModeStateVar();
    InitPlatformBootMode();

    VarSize = sizeof(SETUP_DATA);
    Status = gRT->GetVariable(
                 NORMAL_SETUP_NAME,
                 &gEfiNormalSetupGuid,
                 &mAttributes, //EIP168675
                 &VarSize,
                 &mSystemConfiguration
             );
    DEBUG((EFI_D_ERROR, "GetVariable NORMAL_SETUP_NAME:%r  \n",Status));
    EfiCreateEventReadyToBootEx(TPL_CALLBACK,
                                ReadyToBootFunction,
                                NULL,
                                &mReadyToBootEvent
                               );

//(CSP20130111D-)>>
//// Create a ReadyToBoot Event to run the PME init process
//  Status = EfiCreateEventReadyToBootEx (TPL_CALLBACK,
//                                    InitPciDevPME,
//                                    NULL,
//                                    &mReadyToBootEvent
//                                    );
//(CSP20130111D-)<<


//  //
//  // Dis-arm RTC Alarm Interrupt
//  //
//  IoWrite8(PCAT_RTC_ADDRESS_REGISTER, RTC_ADDRESS_REGISTER_B);
//  IoWrite8(PCAT_RTC_DATA_REGISTER, IoRead8(PCAT_RTC_DATA_REGISTER) & ~B_RTC_ALARM_INT_ENABLE);

    ReportStatusCodeEx(
        EFI_PROGRESS_CODE,
        EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP1,
        0,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
    );

    // Get Setup data from SB and NB.
    GetSbSetupData((VOID*)gRT, &PchPolicyData, FALSE);
    GetNbSetupData((VOID*)gRT, &VlvPolicyData, FALSE);

    // Install SB Policy and NB Policy.
    InitPchPlatformPolicy(ImageHandle, SystemTable, &PchPolicyData);
    InitVlvPlatformPolicy(ImageHandle, SystemTable, &VlvPolicyData);

    //
    //  Add usb policy
    //
    InitPlatformUsbPolicy(&PchPolicyData);
    InitializeSlotInfo();
    InitTcoReset();
//  InitPlatformSetupBrowserPolicy();
//Init ExI
  InitExI();

    ReportStatusCodeEx(
        EFI_PROGRESS_CODE,
        EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP2,
        0,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
    );

    //
    // Initialize Sub-System Vendor & Device IDs
    //
    //InitializeSubsystemIds ();

    InitItk();

    //
    // Process dynamic entries
    //
//  FindDataRecords();

    ReportStatusCodeEx(
        EFI_PROGRESS_CODE,
        EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP3,
        0,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
    );
    //
    // Process Event Log
    //
//  ProcessEventLog();

    //
    // Initialize Password States and Callbacks
    //
//  InstallSecurityCallbackRoutine ();
//  SetPasswordState ();

    PchInitBeforeBoot();
//  UpdateDVMTSetup();

#if defined SUPPORT_LVDS_DISPLAY && SUPPORT_LVDS_DISPLAY

#endif

#if defined(FIRMWARE_ID_BACKWARD_COMPATIBLE) && (FIRMWARE_ID_BACKWARD_COMPATIBLE != 0)
    //
    // Re-write Firmware ID if it is changed
    //
    InitFirmwareId();
#endif

    ReportStatusCodeEx(
        EFI_PROGRESS_CODE,
        EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_STEP4,
        0,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
    );

#if defined ME_SUPPORT && ME_SUPPORT
    MeInfoCallback();
#endif
    
    InitSeC();    //(EIP121724+)

    ReportStatusCodeEx(
        EFI_PROGRESS_CODE,
        EFI_COMPUTING_UNIT_CHIPSET | EFI_CU_PLATFORM_DXE_INIT_DONE,
        0,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
    );


    Status = gBS->CreateEventEx(
                 EVT_NOTIFY_SIGNAL,
// [ EIP257385 ]>>
//                 TPL_NOTIFY,
                 TPL_CALLBACK,
// [ EIP257385 ]<<
                 EnableAcpiCallback,
                 NULL,
                 &gEfiEventExitBootServicesGuid,
                 &mEfiExitBootServicesEvent
             );

    return EFI_SUCCESS;
}

VOID
EfiOrMem(
    IN VOID   *Destination,
    IN VOID   *Source,
    IN UINTN  Length
)
/*++

Routine Description:

  Source Or Destination with Length bytes.

Arguments:

  Destination - Target memory

  Source      - Source memory

  Length      - Number of bytes

Returns:

  None

--*/
{
    CHAR8 *Destination8;
    CHAR8 *Source8;

    if(Source < Destination) {
        Destination8  = (CHAR8 *) Destination + Length - 1;
        Source8       = (CHAR8 *) Source + Length - 1;
        while(Length--) {
            *(Destination8--) |= *(Source8--);
        }
    } else {
        Destination8  = (CHAR8 *) Destination;
        Source8       = (CHAR8 *) Source;
        while(Length--) {
            *(Destination8++) |= *(Source8++);
        }
    }
}

VOID
PchInitBeforeBoot()
{
    //
    // Saved SPI Opcode menu to fix EFI variable unable to write after S3 resume.
    //
    S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN)(SPI_BASE_ADDRESS + (R_PCH_SPI_OPMENU0)),
        1,
        (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_PCH_SPI_OPMENU0)));

    S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN)(SPI_BASE_ADDRESS + (R_PCH_SPI_OPMENU1)),
        1,
        (VOID *)(UINTN)(SPI_BASE_ADDRESS + (R_PCH_SPI_OPMENU1)));

    S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint16,
        (UINTN)(SPI_BASE_ADDRESS + R_PCH_SPI_OPTYPE),
        1,
        (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_PCH_SPI_OPTYPE));

    S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint16,
        (UINTN)(SPI_BASE_ADDRESS + R_PCH_SPI_PREOP),
        1,
        (VOID *)(UINTN)(SPI_BASE_ADDRESS + R_PCH_SPI_PREOP));

    return;
}

VOID
ReadyToBootFunction(
    EFI_EVENT  Event,
    VOID       *Context
)
{
    EFI_STATUS                      Status;
    EFI_ISA_ACPI_PROTOCOL           *IsaAcpi;
    EFI_ISA_ACPI_DEVICE_ID          IsaDevice;
    UINTN                           Size;
    UINT16                          State;
    //(EIP125858-)>>
    /*
    EFI_TPM_MP_DRIVER_PROTOCOL      *TpmMpDriver;
    EFI_CPU_IO_PROTOCOL             *CpuIo;
    UINT8                           Data;
    UINT8                           ReceiveBuffer [64];
    UINT32                          ReceiveBufferSize;

    UINT8 TpmForceClearCommand [] =              {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0A,
            0x00, 0x00, 0x00, 0x5D
                                                 };
    UINT8 TpmPhysicalPresenceCommand [] =        {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0C,
            0x40, 0x00, 0x00, 0x0A,
            0x00, 0x00
                                                 };
    UINT8 TpmPhysicalDisableCommand [] =         {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0A,
            0x00, 0x00, 0x00, 0x70
                                                 };
    UINT8 TpmPhysicalEnableCommand [] =          {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0A,
            0x00, 0x00, 0x00, 0x6F
                                                 };
    UINT8 TpmPhysicalSetDeactivatedCommand [] =  {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0B,
            0x00, 0x00, 0x00, 0x72,
            0x00
                                                 };
    UINT8 TpmSetOwnerInstallCommand [] =         {0x00, 0xC1,
            0x00, 0x00, 0x00, 0x0B,
            0x00, 0x00, 0x00, 0x71,
            0x00
                                                 };
    */
    //(EIP125858-)<<

    Size = sizeof(UINT16);
    Status = gRT->GetVariable(VAR_EQ_FLOPPY_MODE_DECIMAL_NAME,
                              &gEfiNormalSetupGuid,
                              NULL,
                              &Size,
                              &State
                             );
    DEBUG((EFI_D_ERROR, "GetVariable VAR_EQ_FLOPPY_MODE_DECIMAL_NAME:%r  \n",Status));
    //
    // Disable Floppy Controller if needed
    //
    Status = gBS->LocateProtocol(&gEfiIsaAcpiProtocolGuid, NULL, &IsaAcpi);
    if(!EFI_ERROR(Status) && (State == 0x00)) {
        IsaDevice.HID = EISA_PNP_ID(0x604);
        IsaDevice.UID = 0;
        Status = IsaAcpi->EnableDevice(IsaAcpi, &IsaDevice, FALSE);
    }
    /*
      if (mMfgMode) {
        //
        // If in manufacturing mode....
        // Set the Normal Setup to its defaults, clear memory configuration
        //

        Status = gRT->SetVariable (
                  gEfiNormalSetupName,
                  &gEfiNormalSetupGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof(SYSTEM_CONFIGURATION),
                  &mSystemDefaults
                  );
        //
        // Clear memory configuration moved to SaveMemoryConfig driver.
        //

        //
        // Get the Event Log Protocol.
        //
        Status = gBS->LocateProtocol (&gEfiEventLogProtocolGuid, NULL, &EventLog);
        if (!EFI_ERROR(Status)) {
          //
          // If in manufacturing mode....
          //
          Status = EventLog->ClearEvents(EventLog);
        }
      }*/

    // save LAN info to a variable
    gRT->SetVariable(L"PciLanInfo",
                     &gEfiPciLanInfoGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS, //EIP168675
                     mPciLanCount * sizeof(PCI_LAN_INFO),
                     mPciLanInfo
                    );

    gBS->FreePool(mPciLanInfo);

    //(EIP125858-)>>
    /*
    // Handle ACPI OS TPM requests here
    Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, &CpuIo);
    Status = gBS->LocateProtocol(&gEfiTpmMpDriverProtocolGuid, NULL, &TpmMpDriver);
    if(!EFI_ERROR(Status)) {
        Data = ReadCmosBank1Byte(CpuIo, EFI_ACPI_TPM_REQUEST);
        // Clear pending ACPI TPM request indicator
        WriteCmosBank1Byte(CpuIo, EFI_ACPI_TPM_REQUEST, 0x00);
		
        if(Data != 0) {
            WriteCmosBank1Byte(CpuIo, EFI_ACPI_TPM_LAST_REQUEST, Data);
            // Assert Physical Presence for these commands
            TpmPhysicalPresenceCommand [11] = 0x20;
            ReceiveBufferSize = sizeof(ReceiveBuffer);
            TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalPresenceCommand,
                                  sizeof(TpmPhysicalPresenceCommand),
                                  ReceiveBuffer, &ReceiveBufferSize);
            // PF PhysicalPresence = TRUE
            TpmPhysicalPresenceCommand [11] = 0x08;
            ReceiveBufferSize = sizeof(ReceiveBuffer);
            TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalPresenceCommand,
                                  sizeof(TpmPhysicalPresenceCommand),
                                  ReceiveBuffer, &ReceiveBufferSize);
            if(Data == 0x01) {
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                      sizeof(TpmPhysicalEnableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x02) {
                // TPM_PhysicalDisable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalDisableCommand,
                                      sizeof(TpmPhysicalDisableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x03) {
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x04) {
                // TPM_PhysicalSetDeactivated=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x05) {
                // TPM_ForceClear
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmForceClearCommand,
                                      sizeof(TpmForceClearCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x06) {
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                      sizeof(TpmPhysicalEnableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x07) {
                // TPM_PhysicalSetDeactivated=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalDisable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalDisableCommand,
                                      sizeof(TpmPhysicalDisableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x08) {
                // TPM_SetOwnerInstall=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                      sizeof(TpmSetOwnerInstallCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x09) {
                // TPM_SetOwnerInstall=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                      sizeof(TpmSetOwnerInstallCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
            }
            if(Data == 0x0A) {
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                      sizeof(TpmPhysicalEnableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // Do TPM_SetOwnerInstall=TRUE on next reboot
                WriteCmosBank1Byte(CpuIo, EFI_ACPI_TPM_REQUEST, 0xF0);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x0B) {
                // TPM_SetOwnerInstall=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                      sizeof(TpmSetOwnerInstallCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalDisable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalDisableCommand,
                                      sizeof(TpmPhysicalDisableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0x0E) {
                // TPM_ForceClear
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmForceClearCommand,
                                      sizeof(TpmForceClearCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalEnable
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalEnableCommand,
                                      sizeof(TpmPhysicalEnableCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                // TPM_PhysicalSetDeactivated=FALSE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmPhysicalSetDeactivatedCommand [10] = 0x00;
                TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalSetDeactivatedCommand,
                                      sizeof(TpmPhysicalSetDeactivatedCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
            }
            if(Data == 0xF0) {
                // Second part of ACPI TPM request 0x0A: OEM custom TPM_SetOwnerInstall=TRUE
                ReceiveBufferSize = sizeof(ReceiveBuffer);
                TpmSetOwnerInstallCommand [10] = 0x01;
                TpmMpDriver->Transmit(TpmMpDriver, TpmSetOwnerInstallCommand,
                                      sizeof(TpmSetOwnerInstallCommand),
                                      ReceiveBuffer, &ReceiveBufferSize);
                WriteCmosBank1Byte(CpuIo, EFI_ACPI_TPM_LAST_REQUEST, 0x0A);
            }
            // Deassert Physical Presence
            TpmPhysicalPresenceCommand [11] = 0x10;
            ReceiveBufferSize = sizeof(ReceiveBuffer);
            TpmMpDriver->Transmit(TpmMpDriver, TpmPhysicalPresenceCommand,
                                  sizeof(TpmPhysicalPresenceCommand),
                                  ReceiveBuffer, &ReceiveBufferSize);
        }
    }
    */
    //(EIP125858-)<<
    return;
}

VOID
InitMfgAndConfigModeStateVar()
/*++

Routine Description:
  Initializes manufacturing and config mode setting.

Arguments:
  None

Returns:
  None

--*/
{
    EFI_PLATFORM_SETUP_ID           *BootModeBuffer;
    VOID                            *HobList;
    UINT16                          State;

    //
    // Variable initialization
    //
    State = FALSE;

    HobList = GetFirstGuidHob(&gEfiPlatformBootModeGuid);
    if(HobList != NULL) {
        BootModeBuffer = GET_GUID_HOB_DATA(HobList);
        //
        // Check if in Manufacturing mode
        //
        if(!CompareMem(
                    &BootModeBuffer->SetupName,
                    MANUFACTURE_SETUP_NAME,
                    StrSize(MANUFACTURE_SETUP_NAME)
                )
          ) {
            mMfgMode = TRUE;
        }

        //
        // Check if in safe mode
        //
        if(!CompareMem(
                    &BootModeBuffer->SetupName,
                    SAFE_SETUP_NAME,
                    StrSize(SAFE_SETUP_NAME)
                )
          ) {
            State = TRUE;
        }
    }

}

VOID
InitPlatformBootMode()
/*++

Routine Description:
  Initializes manufacturing and config mode setting.

Arguments:
  None

Returns:
  None

--*/
{
    EFI_PLATFORM_SETUP_ID           *BootModeBuffer;
    VOID                            *HobList;

    HobList = GetFirstGuidHob(&gEfiPlatformBootModeGuid);
    if(HobList != NULL) {
        BootModeBuffer = GET_GUID_HOB_DATA(HobList);
        mPlatformBootMode = BootModeBuffer->PlatformBootMode;
    }
}

VOID
InitItk(
  )
/*++

Routine Description:
  Initializes ITK.

Arguments:
  None

Returns:
  None

--*/
{
  EFI_STATUS                          Status;
  UINT16                              ItkModBiosState;
  UINT8                               Value;
  UINTN                               DataSize;
  UINT32                              Attributes;
  //
  // Setup local variable according to ITK variable
  //
  //
  // Read ItkBiosModVar to determine if BIOS has been modified by ITK
  // If ItkBiosModVar = 0 or if variable hasn't been initialized then BIOS has not been modified by ITK modified
  // Set local variable VAR_EQ_ITK_BIOS_MOD_DECIMAL_NAME=0 if BIOS has not been modified by ITK
  //
  DataSize = sizeof (Value);
  Status = gRT->GetVariable (
                  ITK_BIOS_MOD_VAR_NAME,
                  &gItkDataVarGuid,
                  &Attributes,
                  &DataSize,
                  &Value
                  );
  if (Status == EFI_NOT_FOUND) {
    //
    // Variable not found, hasn't been initialized, intialize to 0
    //
    Value=0x00;
        //
  // Write variable to flash.
        //
//EIP168675	>>	
  gRT->SetVariable (ITK_BIOS_MOD_VAR_NAME,
         &gItkDataVarGuid,
         EFI_VARIABLE_NON_VOLATILE |
         EFI_VARIABLE_BOOTSERVICE_ACCESS,
         sizeof (Value),
         &Value
         );
//EIP168675 <<
}
  if ( (!EFI_ERROR (Status)) || (Status == EFI_NOT_FOUND) ) {
    if (Value == 0x00) {
      ItkModBiosState = 0x00;
    }
    else{
      ItkModBiosState = 0x01;
    }
    gRT->SetVariable (VAR_EQ_ITK_BIOS_MOD_DECIMAL_NAME,
                        &gEfiNormalSetupGuid,
                        EFI_VARIABLE_BOOTSERVICE_ACCESS,
                        2,
                        (VOID *)&ItkModBiosState
                        );
  }
}

#if defined(FIRMWARE_ID_BACKWARD_COMPATIBLE) && (FIRMWARE_ID_BACKWARD_COMPATIBLE != 0)
STATIC
VOID
InitFirmwareId(
)
/*++

Routine Description:
  Initializes the BIOS FIRMWARE ID from the FIRMWARE_ID build variable.

Arguments:
  None

Returns:
  None

--*/
{
    EFI_STATUS   Status;
    CHAR16       FirmwareIdNameWithPassword[] = FIRMWARE_ID_NAME_WITH_PASSWORD;

    //
    // First try writing the variable without a password in case we are
    // upgrading from a BIOS without password protection on the FirmwareId
    //
    Status = gRT->SetVariable(
                 (CHAR16 *)&gFirmwareIdName,
                 &gFirmwareIdGuid,
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS, //EIP168675
                 sizeof(FIRMWARE_ID) - 1,
                 FIRMWARE_ID
             );

    if(Status == EFI_INVALID_PARAMETER) {

        //
        // Since setting the firmware id without the password failed,
        // a password must be required.
        //
        Status = gRT->SetVariable(
                     (CHAR16 *)&FirmwareIdNameWithPassword,
                     &gFirmwareIdGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS, //EIP168675
                     sizeof(FIRMWARE_ID) - 1,
                     FIRMWARE_ID
                 );
    }
}
#endif

//(CSP20130125C-)>> Remove un-use code.
/*
VOID
UpdateDVMTSetup(
  )
{
  // Workaround to support IIA bug.
  // IIA request to change option value to 4, 5 and 7 relatively
  // instead of 1, 2, and 3 which follow Lakeport Specs.
  // Check option value, temporary hardcode GraphicsDriverMemorySize
  // Option value to fulfill IIA requirment. So that user no need to
  // load default and update setupvariable after update BIOS.
  //   Option value hardcoded as: 1 to 4, 2 to 5, 3 to 7.
  // *This is for broadwater and above product only.
  //

  SETUP_DATA        SystemConfiguration;
  UINTN                       VarSize;
  EFI_STATUS                  Status;

  VarSize = sizeof(SETUP_DATA);
  Status = gRT->GetVariable(NORMAL_SETUP_NAME,
              &gEfiNormalSetupGuid,
              NULL,
              &VarSize,
              &SystemConfiguration);

#ifndef AMI_SYSCFG_OVERRIDE
  SystemConfiguration.GraphicsDriverMemorySize = 0;    //No find in vfi
#endif
  if((SystemConfiguration.GraphicsDriverMemorySize < 4) && !EFI_ERROR(Status) ){
    switch (SystemConfiguration.GraphicsDriverMemorySize){
      case 1:
        SystemConfiguration.GraphicsDriverMemorySize = 4;
        break;
      case 2:
        SystemConfiguration.GraphicsDriverMemorySize = 5;
        break;
      case 3:
        SystemConfiguration.GraphicsDriverMemorySize = 7;
        break;
      default:
        break;
     }

    Status = gRT->SetVariable (
                NORMAL_SETUP_NAME,
                &gEfiNormalSetupGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                sizeof(SETUP_DATA),
                &SystemConfiguration);
  }
}
*/
//(CSP20130125C-)<<

VOID
InitPlatformUsbPolicy(
    IN SB_SETUP_DATA             *PchPolicyData
)

{
    EFI_HANDLE              Handle;
    EFI_STATUS              Status;

    Handle = NULL;

    //
    // Example/.
    //
    /*
    mUsbPolicyData.Version                       = (UINT8)USB_POLICY_PROTOCOL_REVISION_2;
    mUsbPolicyData.UsbMassStorageEmulationType   = PchPolicyData.UsbMassStorageEmulationType;
    mUsbPolicyData.UsbOperationMode              = PchPolicyData.UsbOperationMode;
    mUsbPolicyData.LegacyKBEnable                = PchPolicyData.UsbKeyboard;
    mUsbPolicyData.LegacyMSEnable                = PchPolicyData.UsbMouse;
    mUsbPolicyData.LegacyUsbEnable               = PchPolicyData.LegacyUsb;
    */

    mUsbPolicyData.Version                       = (UINT8)USB_POLICY_PROTOCOL_REVISION_2;
    mUsbPolicyData.UsbMassStorageEmulationType   = PchPolicyData->UsbBIOSINT13DeviceEmulation;
    if(mUsbPolicyData.UsbMassStorageEmulationType == 3) {
        mUsbPolicyData.UsbEmulationSize = PchPolicyData->UsbBIOSINT13DeviceEmulationSize;
    } else {
        mUsbPolicyData.UsbEmulationSize = 0;
    }
    mUsbPolicyData.UsbZipEmulationType         = PchPolicyData->UsbZipEmulation;
    mUsbPolicyData.UsbOperationMode              = HIGH_SPEED;
    mUsbPolicyData.LegacyKBEnable                = LEGACY_KB_EN;
    mUsbPolicyData.LegacyMSEnable                = LEGACY_MS_EN;
    mUsbPolicyData.LegacyUsbEnable               = PchPolicyData->UsbLegacy;
    mUsbPolicyData.CodeBase                      = ICBD_CODE_BASE;
    //
    //  Some chipset need Period smi, 0 = LEGACY_PERIOD_UN_SUPP
    //
    mUsbPolicyData.USBPeriodSupport      = LEGACY_PERIOD_UN_SUPP;

    //
    //  Some platform need legacyfree, 0 = LEGACY_FREE_UN_SUPP
    //
    mUsbPolicyData.LegacyFreeSupport    = LEGACY_FREE_UN_SUPP;

    //
    //  Set Code base , TIANO_CODE_BASE =0x01, ICBD =0x00
    //
    mUsbPolicyData.CodeBase    = (UINT8)ICBD_CODE_BASE;

    //
    //  Some chispet 's LpcAcpibase are diffrent,set by platform or chipset,
    //  default is Ich  acpibase =0x040. acpitimerreg=0x08.
    mUsbPolicyData.LpcAcpiBase     = 0x40;
    mUsbPolicyData.AcpiTimerReg    = 0x08;

    //
    //  Set for reduce usb post time
    //
    mUsbPolicyData.UsbTimeTue           = 0x00;
    mUsbPolicyData.InternelHubExist     = 0x00;  //TigerPoint doesn't have RMH
    mUsbPolicyData.EnumWaitPortStableStall    = 100;


    Status = gBS->InstallProtocolInterface(
                 &Handle,
                 &gUsbPolicyGuid,
                 EFI_NATIVE_INTERFACE,
                 &mUsbPolicyData
             );
    ASSERT_EFI_ERROR(Status);

}

UINT8
ReadCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index
)
{
    UINT8                               Data;

    CpuIo->Io.Write(CpuIo, EfiCpuIoWidthUint8, 0x72, 1, &Index);
    CpuIo->Io.Read(CpuIo, EfiCpuIoWidthUint8, 0x73, 1, &Data);
    return Data;
}

VOID
WriteCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index,
    IN  UINT8                           Data
)
{
    CpuIo->Io.Write(CpuIo, EfiCpuIoWidthUint8, 0x72, 1, &Index);
    CpuIo->Io.Write(CpuIo, EfiCpuIoWidthUint8, 0x73, 1, &Data);
}

VOID
InitSeC(
    VOID
)
{
    EFI_STATUS Status;
    DXE_SEC_POLICY_PROTOCOL   *SeCPlatformPolicy;
    SEC_OPERATION_PROTOCOL  *SeCOp;
    DEBUG((EFI_D_ERROR, "InitSeC  ++\n"));
    Status = gBS->LocateProtocol(&gDxePlatformSeCPolicyGuid, NULL, &SeCPlatformPolicy);
    if(EFI_ERROR(Status)) {
        return;
    }
//(CSP20130125C+)>> use the Token to setting the SetSeCEOPEnable.
    SeCPlatformPolicy->SeCConfig.EndOfPostEnabled = mSystemConfiguration.SeCEOPEnable;
//(CSP20130125C+)<<
    DEBUG((EFI_D_ERROR, "InitSeC mDxePlatformSeCPolicy->SeCConfig.EndOfPostEnabled %x %x\n", SeCPlatformPolicy->SeCConfig.EndOfPostEnabled,mSystemConfiguration.SeCEOPEnable));
    if(mSystemConfiguration.SeCEOPDone) {
      mSystemConfiguration.SeCEOPDone = 0;
      Status = gRT->SetVariable (
                NORMAL_SETUP_NAME,
                &gEfiNormalSetupGuid,
                mAttributes, //EIP168675
                sizeof(SETUP_DATA),
                &mSystemConfiguration);
    }    
    
    Status = gBS->LocateProtocol(
                 &gEfiSeCOperationProtocolGuid,
                 NULL,
                 &SeCOp
             );
    if(EFI_ERROR(Status)) {
        return;
    }

    SeCOp->PerformSeCOperation(SEC_OP_CHECK_UNCONFIG);    //(CSP20130221E+) Update as Intel update.
    SeCOp->PerformSeCOperation(SEC_OP_CHECK_HMRFPO);    //(CSP20130221E+) Update as Intel update.
  
}
