/** @file
  Mmio device driver

  This driver provides the enumeration support for the MMIO devices.

  Copyright (c) 2012 - 2013, Intel Corporation All rights
  reserved. This program and the accompanying materials are
  licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <IndustryStandard/Pci22.h>
#include <Protocol/DevicePath.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/I2cAcpi.h>
#include <Protocol/PciEnumerationComplete.h>
#include <PchRegs.h>
#include "I2cMmioConfig.h"

EFI_EVENT   mI2cMmioDeviceEnumerationEvent = NULL;
VOID        *mI2cMmioDeviceEnumerationRegistration;

extern EFI_GUID gLpssDummyProtocolGuid;
extern EFI_GUID gAmiExtPciBusProtocolGuid;  //AMI_OVERRIDE - EIP137990 Use AMI PciBus

#define I2C_A0

I2C_DEVICE_INFO mI2cDeviceInfoList[ ] = {
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0, GLOBAL_NVS_OFFSET(I2C1Addr) },
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1, GLOBAL_NVS_OFFSET(I2C2Addr) },

#ifdef I2C_A0
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2, GLOBAL_NVS_OFFSET(I2C3Addr) },
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3, GLOBAL_NVS_OFFSET(I2C4Addr) },
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4, GLOBAL_NVS_OFFSET(I2C5Addr) },
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5, GLOBAL_NVS_OFFSET(I2C6Addr) },
  {0, DEFAULT_PCI_BUS_NUMBER_PCH,  PCI_DEVICE_NUMBER_PCH_LPSS_I2C, PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6, GLOBAL_NVS_OFFSET(I2C7Addr) },
#endif

};

#define PciD31F0RegBase   EC_BASE + (UINT32) (31 << 15)

#define PCI_CFG_ADDRESS(bus, dev, func, reg)  ( \
      (UINT64) ((((UINTN) bus) << 24) + (((UINTN) dev) << 16) + (((UINTN) func) << 8) + ((UINTN) reg)) \
    ) & 0x00000000ffffffff

/**
  Driver path template
**/

CONST STATIC EFI_DEVICE_PATH_PROTOCOL mEndOfPath = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  {
    END_DEVICE_PATH_LENGTH,
    0
  }
};

UINT32 mPmcBase;

UINT8 mAcpi_INTC33B1_I2C0 [ ] = {  //  I2C0 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};

UINT8 mAcpi_INTC33B1_I2C1 [ ] = {  //  I2C1 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};

#ifdef I2C_A0
UINT8 mAcpi_INTC33B1_I2C2 [ ] = {  //  I2C2 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};

UINT8 mAcpi_INTC33B1_I2C3 [ ] = {  //  I2C3 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};


UINT8 mAcpi_INTC33B1_I2C4 [ ] = {  //  I2C4 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};

UINT8 mAcpi_INTC33B1_I2C5 [ ] = {  //  I2C5 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};

UINT8 mAcpi_INTC33B1_I2C6 [ ] = {  //  I2C6 controller
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  35, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0,
  0,
  'I', 'N', 'T', 'C', '3', '3', 'B', '1', 0
};
#endif



CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c0Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c0BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c0BusEnum.ConfigApi }
};

CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c1Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c1BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c1BusEnum.ConfigApi }
};


#ifdef I2C_A0
CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c2Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c2BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c2BusEnum.ConfigApi }
};

CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c3Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c3BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c3BusEnum.ConfigApi }
};

CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c4Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c4BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c4BusEnum.ConfigApi }
};

CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c5Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c5BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c5BusEnum.ConfigApi }
};

CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM mI2c6Protocols [ ] = {
  { &gEfiI2cAcpiProtocolGuid,  &mI2c6BusEnum.AcpiApi },
  { &gEfiI2cBusConfigurationManagementProtocolGuid, &mI2c6BusEnum.ConfigApi }
};
#endif






//----------------------------------------------------------------------
//  MMIO devices
//----------------------------------------------------------------------

CONST EFI_MMIO_DEVICE_PROTOCOL gI2cMmioDeviceList [ ] = {
  //
  //  I2C-0
  //
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C0 [ 0 ],
    1,
    0x00010000,
    (CONST VOID *)&mI2c0ControllerConfig,
    DIM ( mI2c0Protocols ),
    mI2c0Protocols
  },

  //
  //  I2C-1
  //
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C1 [ 0 ],
    2,
    0x00010000,
    (CONST VOID *)&mI2c1ControllerConfig,
    DIM ( mI2c1Protocols ),
    mI2c1Protocols
  },


#ifdef I2C_A0
  //
  //  I2C-2
  //
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C2 [ 0 ],
    3,
    0x00010000,
    (CONST VOID *)&mI2c2ControllerConfig,
    DIM ( mI2c2Protocols ),
    mI2c2Protocols
  },

  //
  //  I2C-3
  //
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C3 [ 0 ],
    4,
    0x00010000,
    (CONST VOID *)&mI2c3ControllerConfig,
    DIM ( mI2c3Protocols ),
    mI2c3Protocols
  },

  //
  //  I2C-4
  //
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C4 [ 0 ],
    5,
    0x00010000,
    (CONST VOID *)&mI2c4ControllerConfig,
    DIM ( mI2c4Protocols ),
    mI2c4Protocols
  },

//
//  I2C-5
//
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C5 [ 0 ],
    6,
    0x00010000,
    (CONST VOID *)&mI2c5ControllerConfig,
    DIM ( mI2c5Protocols ),
    mI2c5Protocols
  },

//
//  I2C-6
//
  {
    (CONST ACPI_EXTENDED_HID_DEVICE_PATH *)&mAcpi_INTC33B1_I2C6 [ 0 ],
    7,
    0x00010000,
    (CONST VOID *)&mI2c6ControllerConfig,
    DIM ( mI2c6Protocols ),
    mI2c6Protocols
  },
#endif


};



CONST UINTN gI2cMmioDeviceCount = DIM ( gI2cMmioDeviceList );

void patchAcpiPath()
{
  mAcpi_INTC33B1_I2C0[2] = sizeof(mAcpi_INTC33B1_I2C0);
  mAcpi_INTC33B1_I2C1[2] = sizeof(mAcpi_INTC33B1_I2C1);
#ifdef I2C_A0
  mAcpi_INTC33B1_I2C2[2] = sizeof(mAcpi_INTC33B1_I2C2);
  mAcpi_INTC33B1_I2C3[2] = sizeof(mAcpi_INTC33B1_I2C3);
  mAcpi_INTC33B1_I2C4[2] = sizeof(mAcpi_INTC33B1_I2C4);
  mAcpi_INTC33B1_I2C5[2] = sizeof(mAcpi_INTC33B1_I2C5);
  mAcpi_INTC33B1_I2C6[2] = sizeof(mAcpi_INTC33B1_I2C6);
#endif
  patchI2c0AcpiPath();
  patchI2c1AcpiPath();
#ifdef I2C_A0
  patchI2c2AcpiPath();
  patchI2c3AcpiPath();
  patchI2c4AcpiPath();
  patchI2c5AcpiPath();
  patchI2c6AcpiPath();
#endif
}


STATIC
VOID
EFIAPI
OnLpssPci2Acpi(
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *gPciRootBridgeIo;
  EFI_STATUS           Status;
  UINT32 i = 0;
  UINT32 AcpiAddress;
  UINT32 Register_I2C1 = R_PCH_LPSS_FAB2CTLP1;
  UINT32 AcpidModeVal;
  UINT32 i2c_mmioaddr;
  MmioWrite32( EC_BASE + MC_MCRX, Register_I2C1 & MSGBUS_MASKHI );
  MmioWrite32( EC_BASE + MC_MCR, ( (PCH_LPSS_EP_PRIVATE_READ_OPCODE << 24) | (PCH_LPSS_EP_PORT_ID << 16) | ((Register_I2C1 & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN));
  AcpidModeVal = MmioRead32( EC_BASE + MC_MDR );
  if(Event != (EFI_EVENT)0) gBS->CloseEvent(Event);
  DEBUG((EFI_D_INFO, "\r\n\r\n!!!Close the event so that the callback willn't be called multiple times\r\n"));
  DEBUG((EFI_D_INFO, "AcpiModeVal: 0x%x\r\n", AcpidModeVal));
  if(B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS == (AcpidModeVal & B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS) ) {
    DEBUG((EFI_D_INFO, "switch PCI to ACPI mode\r\n"));
    Status = gBS->LocateProtocol (
                    &gEfiGlobalNvsAreaProtocolGuid,
                    NULL,
                    (VOID **) &GlobalNvsArea
                    );
    if (EFI_ERROR(Status)) {
      return;
    }

    for( i = 0; i < DIM(gI2cMmioDeviceList); i ++) {
      if (((MmioRead32 (mPmcBase + R_PCH_PMC_FUNC_DIS) & (B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1 << i))) == 0) {
        AcpiAddress = (*(UINT32 *)((UINT8 *)GlobalNvsArea->Area + mI2cDeviceInfoList[i].GNVSOffset));
        DEBUG((EFI_D_INFO, "I2C%02d: 0x%08x\r\n",  i, AcpiAddress));
        if( (AcpiAddress == 0) || ((AcpiAddress & 3) != 0)) {
          continue;
        }
        ((I2C_PIO_PLATFORM_CONTEXT *)((gI2cMmioDeviceList [ i ].DriverResources)))->BaseAddress = (EFI_PHYSICAL_ADDRESS)AcpiAddress;
      }
    }
  } else {
    DEBUG((EFI_D_INFO, "Still in PCI mode, but mmio is reallocated\r\n"));
    Status = gBS->LocateProtocol (
                    &gEfiPciRootBridgeIoProtocolGuid,
                    NULL,
                    (VOID **) &gPciRootBridgeIo
                    );

    if(!EFI_ERROR(Status)) {
      for(i = 0; i < DIM(gI2cMmioDeviceList); i ++) {
        if (((MmioRead32 (mPmcBase + R_PCH_PMC_FUNC_DIS) & (B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1 << i))) == 0) {
          Status = gPciRootBridgeIo->Pci.Read(
                                       gPciRootBridgeIo,
                                       EfiPciWidthUint32,
                                       PCI_CFG_ADDRESS(mI2cDeviceInfoList[i].BusNum,
                                         mI2cDeviceInfoList[i].DeviceNum,
                                         mI2cDeviceInfoList[i].FunctionNum,
                                         PCI_BASE_ADDRESSREG_OFFSET),
                                       1,
                                       &i2c_mmioaddr);
          DEBUG (( EFI_D_INFO, "I2C%d MMIO Addr read : %r 0x%08x\r\n", i, Status, i2c_mmioaddr ));
          if( EFI_ERROR ( Status ) || ( i2c_mmioaddr == 0 ) || ((i2c_mmioaddr & 3) != 0)) {
            continue;
          }
          ((I2C_PIO_PLATFORM_CONTEXT *)((gI2cMmioDeviceList [ i ].DriverResources)))->BaseAddress = (EFI_PHYSICAL_ADDRESS)i2c_mmioaddr;
        }
      }
    }

  }
}


/**
  Callback function to enumerate the MMIO devices.

  @param Event           the event that is signaled.
  @param Context         not used here.

**/
VOID
EFIAPI
I2cMmioDeviceEnumerationEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_HANDLE Device;
  UINT8 * Buffer;
  ACPI_EXTENDED_HID_DEVICE_PATH * DevicePath;
  UINTN LengthInBytes;
  CONST EFI_MMIO_DEVICE_PROTOCOL * MmioDeviceListEnd;
  CONST EFI_MMIO_DEVICE_PROTOCOL * MmioDevice;
  CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM * Protocol;
  CONST EFI_MMIO_DEVICE_PROTOCOL_ITEM * ProtocolEnd;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *gPciRootBridgeIo;
  EFI_STATUS Status;
  UINT32 i2c_mmioaddr;
  EFI_EVENT LpssPci2AcpiEvent;
  VOID  *mLPSSRegistration = NULL;
  UINT32 i = 0;
  UINT32 Register_I2C1 = R_PCH_LPSS_FAB2CTLP1;
  UINT32 AcpidModeVal;

  DEBUG (( DEBUG_INFO, "I2C MMIO device enumeration entered\r\n" ));
  patchAcpiPath();

  mPmcBase = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  MmioWrite32( EC_BASE + MC_MCRX, Register_I2C1 & MSGBUS_MASKHI );
  MmioWrite32( EC_BASE + MC_MCR, ( (PCH_LPSS_EP_PRIVATE_READ_OPCODE << 24) | (PCH_LPSS_EP_PORT_ID << 16) | ((Register_I2C1 & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN));
  AcpidModeVal = MmioRead32( EC_BASE + MC_MDR );
  DEBUG((EFI_D_INFO, "AcpiModeVal: 0x%x\r\n", AcpidModeVal));
  if(B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS != (AcpidModeVal & B_PCH_LPSS_FABXCTLPX_PCI_CFG_DIS) ) {
    DEBUG((EFI_D_INFO, "Acpi mode not enabled\r\n"));
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    OnLpssPci2Acpi,
                    NULL,
                    &LpssPci2AcpiEvent
                    );

    Status = gBS->RegisterProtocolNotify (
                    &gLpssDummyProtocolGuid,
                    LpssPci2AcpiEvent,
                    &mLPSSRegistration
                    );

    mLPSSRegistration = NULL;
    Status = gBS->LocateProtocol (
                    &gEfiPciRootBridgeIoProtocolGuid,
                    NULL,
                    (VOID **) &gPciRootBridgeIo
                    );

    if(!EFI_ERROR(Status)) {
      for(i = 0; i < DIM(gI2cMmioDeviceList); i ++) {
        if (((MmioRead32 (mPmcBase + R_PCH_PMC_FUNC_DIS) & (B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1 << i))) == 0) {
          Status = gPciRootBridgeIo->Pci.Read(
                     gPciRootBridgeIo,
                     EfiPciWidthUint32,
                     PCI_CFG_ADDRESS(mI2cDeviceInfoList[i].BusNum,
                       mI2cDeviceInfoList[i].DeviceNum,
                       mI2cDeviceInfoList[i].FunctionNum,
                       PCI_BASE_ADDRESSREG_OFFSET),
                     1,
                     &i2c_mmioaddr);
          DEBUG (( EFI_D_INFO, "I2C%d MMIO Addr read : %r 0x%08x\r\n", i, Status, i2c_mmioaddr ));
          if( EFI_ERROR ( Status ) || ( i2c_mmioaddr == 0 ) || ((i2c_mmioaddr & 3) != 0)) {
            continue;
          }
          ((I2C_PIO_PLATFORM_CONTEXT *)((gI2cMmioDeviceList [ i ].DriverResources)))->BaseAddress = (EFI_PHYSICAL_ADDRESS)i2c_mmioaddr;
        }
      }
    }
  } else {
    OnLpssPci2Acpi((EFI_EVENT) 0, (void *) 0);
  }
  DEBUG (( DEBUG_INFO, "MMIO device enumeration starting\r\n" ));

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;
  if ( 0 < gI2cMmioDeviceCount ) {
    //
    //  Walk the list of devices
    //
    MmioDevice = &gI2cMmioDeviceList [ 0 ];
    MmioDeviceListEnd = &MmioDevice [ gI2cMmioDeviceCount ];
    while ( MmioDeviceListEnd > MmioDevice ) {
      if ( NULL == MmioDevice->AcpiPath ) {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - AcpiPath is NULL in gMmioDeviceList [ %d ]\r\n",
                 MmioDevice - &gI2cMmioDeviceList [ 0 ]));
        Status = EFI_INVALID_PARAMETER;
        break;
      }

      LengthInBytes = DevicePathNodeLength ( MmioDevice->AcpiPath );
      if ( sizeof ( ACPI_EXTENDED_HID_DEVICE_PATH ) > LengthInBytes ) {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - invalid AcpiPath length in gMmioDeviceList [ %d ]\r\n",
                 MmioDevice - &gI2cMmioDeviceList [ 0 ]));
        Status = EFI_INVALID_PARAMETER;
        break;
      }

      if ( NULL == MmioDevice->DriverResources ) {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - DriverResources is NULL in gMmioDeviceList [ %d ]\r\n",
                 MmioDevice - &gI2cMmioDeviceList [ 0 ]));
        Status = EFI_INVALID_PARAMETER;
        break;
      }


      Buffer = AllocatePool ( LengthInBytes + sizeof ( mEndOfPath ));
      if ( NULL == Buffer ) {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - Failed to allocate device path\r\n" ));
        Status = EFI_OUT_OF_RESOURCES;
        break;
      }

      if (((MmioRead32 (mPmcBase + R_PCH_PMC_FUNC_DIS) & (B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1 << (MmioDevice->UnitIdentification - 1)))) == 0) {
        //
        //  Build the device path for the device
        //
        DevicePath = (ACPI_EXTENDED_HID_DEVICE_PATH *)Buffer;
        CopyMem ( DevicePath,
                  MmioDevice->AcpiPath,
                  LengthInBytes );
        if ( 0 != MmioDevice->UnitIdentification) {
          DevicePath->UID = MmioDevice->UnitIdentification;
        }
        CopyMem ( &Buffer [ LengthInBytes ],
                  &mEndOfPath,
                  sizeof ( mEndOfPath ));

        //
        //  Create a handle for the device and install the
        //  protocols
        //
        Device = NULL;
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &Device,
                        &gEfiMmioDeviceProtocolGuid,
                        MmioDevice,
                        &gEfiDevicePathProtocolGuid,
                        DevicePath,
                        NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                   "ERROR - Failed to install protocol, Status: %r\r\n",
                   Status ));
        }

        //
        //  Install the additional protocols
        //
        Protocol = MmioDevice->ProtocolArray;
        ProtocolEnd = &Protocol [ MmioDevice->ProtocolCount ];
        while ( ProtocolEnd > Protocol ) {
          Status = gBS->InstallMultipleProtocolInterfaces (
                          &Device,
                          Protocol->Guid,
                          Protocol->Context,
                          NULL );
          if ( EFI_ERROR ( Status )) {
            DEBUG (( DEBUG_ERROR,
                     "ERROR - Failed to additional install protocol GUID, Status: %r\r\n",
                     Status ));
          }
          Protocol += 1;
        }
      }

      //
      //  Set the next device
      //
      MmioDevice += 1;
    }
  }

  DEBUG (( DEBUG_INFO, "MMIO device enumeration done, Status: %r\r\n", Status ));
  return;
}


//refer ReadyToBoot4UpdateLpss in Lpss module


/**
  Entry point for I2C MMIO devices.

  @param[in] ImageHandle  Handle for the image
  @param[in] SystemTable  Address of the system table

  @retval EFI_SUCCESS           Successful enumeration of the MMIO devices

**/
EFI_STATUS
EFIAPI
I2C_MmioEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS      Status;

  //
  // Register Protocol notify for Hotkey service
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  I2cMmioDeviceEnumerationEvent,
                  NULL,
                  &mI2cMmioDeviceEnumerationEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for protocol notifications on this event
  //
  Status = gBS->RegisterProtocolNotify (
                  &gAmiExtPciBusProtocolGuid, //AMI_OVERRIDE - EIP137990 Use AMI PciBus
                  mI2cMmioDeviceEnumerationEvent,
                  &mI2cMmioDeviceEnumerationRegistration
                  );

  return Status;
}
