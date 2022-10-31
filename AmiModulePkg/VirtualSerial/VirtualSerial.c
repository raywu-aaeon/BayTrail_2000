//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/VirtualSerialDevice/VirtualSerial.c 2     4/13/10 12:35a Rameshr $
//
// $Revision: 2 $
//
// $Date: 4/13/10 12:35a $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name: VirtualSerial.c
//
// Description: Create devicepath for the Virtual Serial Device. So that
//              SerialIO can consume this device and redirection can be done
//              via this Serial device.
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>


#include "VirtualSerial.h"
#include <GenericSio.h>


#define     MAXIMUM_NO_VIRTUAL_DEVICES 3

EFI_GUID             gDxeSvcTblGuid = DXE_SERVICES_TABLE_GUID;
extern EFI_GUID     gEfiAmiSioProtocolGuid;
static DXE_SERVICES        *gDxeSvcTbl=NULL;

VIRTUAL_SERIAL_DETAILS VirtualSerialDevices[MAXIMUM_NO_VIRTUAL_DEVICES]= {
    { 
        VIRTUAL_SERIAL_DEVICE1_BASE_ADDRESS,
        VIRTUAL_SERIAL_DEVICE1_IRQ,
        VIRTUAL_SERIAL_DEVICE1_UID
    },

    { 
        VIRTUAL_SERIAL_DEVICE2_BASE_ADDRESS,
        VIRTUAL_SERIAL_DEVICE2_IRQ,
        VIRTUAL_SERIAL_DEVICE2_UID
    },

    { 
        VIRTUAL_SERIAL_DEVICE3_BASE_ADDRESS,
        VIRTUAL_SERIAL_DEVICE3_IRQ,
        VIRTUAL_SERIAL_DEVICE3_UID
    }

};

struct {
    ACPI_HID_DEVICE_PATH      AcpiDevicePath;
    EFI_DEVICE_PATH_PROTOCOL  EndDevicePath;
} VirtualSerialDevicePath[MAXIMUM_NO_VIRTUAL_DEVICES] = {
    {
        {
            ACPI_DEVICE_PATH,
            ACPI_DP,
            (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
            (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),
            EISA_PNP_ID (0x501), VIRTUAL_SERIAL_DEVICE1_UID
        },

        {
            END_DEVICE_PATH, END_ENTIRE_SUBTYPE,
            sizeof(EFI_DEVICE_PATH_PROTOCOL)
        }
    },

    {
        {
            ACPI_DEVICE_PATH,
            ACPI_DP,
            (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
            (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),
            EISA_PNP_ID (0x501), VIRTUAL_SERIAL_DEVICE2_UID
        },

        {
            END_DEVICE_PATH, END_ENTIRE_SUBTYPE,
            sizeof(EFI_DEVICE_PATH_PROTOCOL)
        }
    },

    {
        {
            ACPI_DEVICE_PATH,
            ACPI_DP,
            (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
            (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),
            EISA_PNP_ID (0x501), VIRTUAL_SERIAL_DEVICE3_UID
        },

        {
            END_DEVICE_PATH, END_ENTIRE_SUBTYPE,
            sizeof(EFI_DEVICE_PATH_PROTOCOL)
        }
    }
};

SPIO_DEV                    VirtualDevice[MAXIMUM_NO_VIRTUAL_DEVICES];
SPIO_DEV_LST                VirtualDeviceList[MAXIMUM_NO_VIRTUAL_DEVICES];

T_ITEM_LIST     VirtualSerialResources;
ASLR_FixedIO    VirtualSerialBaseAddress;
ASLR_IRQNoFlags VirtualSerialIrq;
VOID            *ResourceList[2];

//<AMI_PHDR_START>
//============================================================================
// Procedure:   VirtualSerialRegister
//
// Description: Get the Virtual Serial Device Register details
//
// Input:   
//         
//          IN AMI_SIO_PROTOCOL *This,
//          IN BOOLEAN          Write,
//          IN BOOLEAN          ExitCfgMode,
//          IN UINT8            Register,
//          IN OUT UINT8        *Value 
//
// Output: 
//          This is not supported for virtual Serial device 
//
// Referrals:
//
//============================================================================
//<AMI_PHDR_END> 
EFI_STATUS 
VirtualSerialRegister (
    IN AMI_SIO_PROTOCOL     *This,
    IN BOOLEAN              Write,
    IN BOOLEAN              ExitCfgMode,
    IN UINT8                Register,
    IN OUT UINT8            *Value
)
{

    return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//============================================================================
// Procedure:   VirtualSerialCRS
//
// Description: Get the Virtual Serial Device current resource
//
// Input:   
//         
//          IN AMI_SIO_PROTOCOL *This,
//          IN BOOLEAN          Set,
//          IN OUT T_ITEM_LIST  **Resources
//
// Output: 
//          Return the virutal serial device current resource 
//
// Referrals:
//
//============================================================================
//<AMI_PHDR_END> 
EFI_STATUS 
VirtualSerialCRS(
    IN AMI_SIO_PROTOCOL     *This,
    IN BOOLEAN              Set,
    IN OUT T_ITEM_LIST      **Resources
)
{

    SPIO_DEV*    VirtualDevice=(SPIO_DEV*)This;

    if (!This || !Resources) {
        return EFI_INVALID_PARAMETER;
    }

    if (Set) {
        return EFI_UNSUPPORTED;
    } else {

        // Virtual Serial Device Base address Resource
        VirtualSerialBaseAddress.Hdr.Name   = ASLV_RT_FixedIO;
        VirtualSerialBaseAddress._BAS       = VirtualDevice->VlData.DevBase1;
        VirtualSerialBaseAddress._LEN       = 0x8;

        //Virtual Device IRQ details.
        VirtualSerialIrq.Hdr.Name           = ASLV_RT_IRQ;
        VirtualSerialIrq._INT               = VirtualDevice->VlData.DevIrq1;

        ResourceList[0]                     =(VOID*)&VirtualSerialBaseAddress;
        ResourceList[1]                     =(VOID*)&VirtualSerialIrq;

        //Set the Resource details
        VirtualSerialResources.InitialCount = 2;
        VirtualSerialResources.ItemCount    = 2;
        VirtualSerialResources.Items        =(VOID*)&ResourceList[0];

        *Resources                          = &VirtualSerialResources;
    }

    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//============================================================================
// Procedure:   VirtualSerialCRS
//
// Description: Get the Virtual Serial Device current resource
//
// Input:   
//         
//          IN AMI_SIO_PROTOCOL *This,
//          IN BOOLEAN          Set,
//          IN OUT T_ITEM_LIST  **Resources
//
// Output: 
//          Not Supported 
//
// Referrals:
//
//============================================================================
//<AMI_PHDR_END> 
EFI_STATUS 
VirtualSerialPRS(
    IN AMI_SIO_PROTOCOL     *This,
    IN BOOLEAN              Set,
    IN OUT T_ITEM_LIST      **Resources
)
{
    return EFI_UNSUPPORTED;
}
//<AMI_PHDR_START>
//============================================================================
// Procedure:   InitilizeVirtualSerialDevice
//
// Description: Initilize the Virtual Serial Device
//
// Input:   
//
// Output: 
//
// Referrals:
//
//============================================================================
//<AMI_PHDR_END> 
InitilizeVirtualSerialDevice(
)
{
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//============================================================================
// Procedure: VirtualSerialEntryPoint
//
// Description:  Virtual Seral device Entry point. It creates the device path
//               for the Virtual Serial device
//
// Input:   ImageHandle         Image handle for this driver
//          SystemTable          Pointer to the EFI system table.
//
// Output: 
//          EFI_SUCCESS     The function completed successfully.
//
// Referrals:
//
//============================================================================
//<AMI_PHDR_END> 
EFI_STATUS
EFIAPI
VirtualSerialEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
    EFI_STATUS              Status;
    EFI_HANDLE              VirtualDeviceHandle=NULL;
    EFI_PHYSICAL_ADDRESS    addr=0;
    UINT16                  Len =8;
    UINT8                   Aln=0;
    UINT16                  IrqMask;
    UINT32                  Count=0;

    // Initialize the EFI driver library
    InitAmiLib(ImageHandle, SystemTable);

    gDxeSvcTbl=(DXE_SERVICES*)GetEfiConfigurationTable(SystemTable,
                                &gDxeSvcTblGuid);
    
    if(!gDxeSvcTbl) {
        return EFI_NOT_FOUND;
    }

    // Initilize the Virtual Serial Device
    InitilizeVirtualSerialDevice();

    for(Count=0;Count<MAXIMUM_NO_VIRTUAL_DEVICES;Count++) {

        if(VirtualSerialDevices[Count].BaseAddress == 0 ||
                VirtualSerialDevices[Count].Irq == 0) {

            // Device is not present. So continue for next device.
            continue;
        }

        addr=(EFI_PHYSICAL_ADDRESS)VirtualSerialDevices[Count].BaseAddress;
        Status=gDxeSvcTbl->AddIoSpace (
                            EfiGcdIoTypeIo,
                            addr,
                            Len
                            );

        if(EFI_ERROR(Status)) {
            continue;
        }

        // Inform the generic driver that Virtual Serial device base address
        //  is used. So that nobody else can use the resource
        Status=gDxeSvcTbl->AllocateIoSpace(EfiGcdAllocateAddress,
                                        EfiGcdIoTypeIo,
                                        Aln,
                                        Len, 
                                        &addr,
                                        ImageHandle,
                                        NULL
                                        );
        if(EFI_ERROR(Status)|| 
                ((UINT16)addr != VirtualSerialDevices[Count].BaseAddress)) {
            continue;
        }

        Status=AmiIsaIrqMask(&IrqMask, TRUE);

        if(Status==EFI_NOT_FOUND){
            IrqMask=ISA_IRQ_MASK;
            IrqMask |= (1<<VirtualSerialDevices[Count].Irq);
            Status = AmiIsaIrqMask(&IrqMask, FALSE);
        } else {
            IrqMask |= (1<<VirtualSerialDevices[Count].Irq);
            Status=AmiIsaIrqMask(&IrqMask, FALSE);
        }

        VirtualDevice[Count].AmiSio.Access          = VirtualSerialRegister;
        VirtualDevice[Count].AmiSio.CurrentRes      = VirtualSerialCRS;
        VirtualDevice[Count].AmiSio.PossibleRes     = VirtualSerialPRS;

        VirtualDevice[Count].VlData.DevImplemented  = 1;
        VirtualDevice[Count].VlData.DevBase1        = VirtualSerialDevices[Count].BaseAddress;
        VirtualDevice[Count].VlData.DevBase2        = 0;
        VirtualDevice[Count].VlData.DevIrq1         = VirtualSerialDevices[Count].Irq;
        VirtualDevice[Count].VlData.DevIrq2         = 0;
        VirtualDevice[Count].VlData.DevDma1         = 0;
        VirtualDevice[Count].VlData.DevDma2         = 0;

        VirtualDeviceList[Count].Type               = dsUART;
        VirtualDeviceList[Count].LDN                = 0;
        VirtualDeviceList[Count].UID                = VirtualSerialDevices[Count].Uid;
        VirtualDeviceList[Count].PnpId              = 0x501;
        VirtualDeviceList[Count].Implemented        = TRUE;
        VirtualDeviceList[Count].HasSetup           = FALSE;

        VirtualDevice[Count].DeviceInfo             = &VirtualDeviceList[Count];

        VirtualDeviceHandle=NULL;
        Status=pBS->InstallMultipleProtocolInterfaces( 
                                            &VirtualDeviceHandle, 
                                            &guidDevicePath, 
                                            &VirtualSerialDevicePath[Count], 
                                            &gEfiAmiSioProtocolGuid, 
                                            &VirtualDevice[Count].AmiSio, 
                                            NULL 
                                            );
    }

    return Status;
}
 
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
