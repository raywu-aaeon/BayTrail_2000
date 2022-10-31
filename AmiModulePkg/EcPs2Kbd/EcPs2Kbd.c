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
//
// $Header: /Alaska/SOURCE/Modules/EcPs2Kbd/EcPs2Kbd.c 3     6/29/10 3:01p Stacyh $
//
// $Revision: 3 $
//
// $Date: 6/29/10 3:01p $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        EcPs2Kbd.C
//
// Description: This file contains code necessary to install the EC PS2 devices
//              for EFI, ACPI and DOS.
//
// Notes:       
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>
#include "token.h"
#include "EcPs2Kbd.h"

#define BDS_CONNECT_DRIVERS_PROTOCOL_GUID \
    { 0x3aa83745, 0x9454, 0x4f7a, 0xa7, 0xc0, 0x90, 0xdb, 0xd0, 0x2f, 0xab, 0x8e }

EFI_GUID  gBdsConnectDriversProtocolGuid = BDS_CONNECT_DRIVERS_PROTOCOL_GUID;

#define EC_KBC_DATA_PORT	0x60
#define EC_KBC_CMD_PORT		0x64

static EFI_GUID gSioDevStatusVarGuid = SIO_DEV_STATUS_VAR_GUID;

static PS2_KBD_DEVICE_PATH  mEcPs2KbdDevicePath[1][1] = {
    {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),
        EISA_PNP_ID(0x303),
        0,
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        END_DEVICE_PATH_LENGTH,
        0
    }
};
  
static PS2_KBD_DEVICE_PATH  mEcPs2MsDevicePath[1][1] = {
    {
        ACPI_DEVICE_PATH,
        ACPI_DP,
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),
        EISA_PNP_ID(0xF03),
        0,
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        END_DEVICE_PATH_LENGTH,
        0
    }
};

EFI_EVENT   gEcPs2DeviceEvent;
VOID        *gEcPs2DeviceReg;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: InstallEcPs2Devices
//
// Description: Install DevicePath protocol for the EC Ps2 devices 
//
// Input:   Event   - The event that triggered this notification function  
//          Context - Pointer to the notification functions context
//
// Output:  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
InstallEcPs2Devices (
  EFI_EVENT           Event,
  VOID                *Context
  )
{
    EFI_DEVICE_PATH_PROTOCOL    *KbTmpDp = NULL;
    EFI_DEVICE_PATH_PROTOCOL    *MsTmpDp = NULL;
    ACPI_HID_DEVICE_PATH        RbDp;
    PCI_DEVICE_PATH             PciDp;
    PS2KBD_DEV                  *PrivateData;
    SIO_DEV_STATUS              SioDevStatusVar;
    UINTN                       SioDevStatusVarSize = sizeof(SIO_DEV_STATUS);
    UINT32                      SioDevStatusVarAttributes = 0;   
    EFI_STATUS                  Status;
    UINT16                      AvailableIsaIrq=0;
    

    // Get root bridge device path

    RbDp.Header.Type = ACPI_DEVICE_PATH;
    RbDp.Header.SubType = ACPI_DP;
    SET_NODE_LENGTH(&RbDp.Header, ACPI_DEVICE_PATH_LENGTH);
    RbDp.HID = EISA_PNP_ID(0x0A03);
    RbDp.UID = 0;
    KbTmpDp = DPAddNode(KbTmpDp, &RbDp.Header);
    MsTmpDp = DPAddNode(MsTmpDp, &RbDp.Header);

    // Get PCI device path

    PciDp.Header.SubType = HW_PCI_DP;
    PciDp.Header.Type = HARDWARE_DEVICE_PATH;
    SET_NODE_LENGTH(&PciDp.Header, HW_PCI_DEVICE_PATH_LENGTH);
    PciDp.Function = SB_FUN_NUMBER;
    PciDp.Device = SB_DEV_NUMBER;
    KbTmpDp = DPAddNode(KbTmpDp, &PciDp.Header);
    MsTmpDp = DPAddNode(MsTmpDp, &PciDp.Header);

    // Install PS2 Keyboard Device Path

    Status = pBS->AllocatePool (EfiBootServicesData, sizeof (PS2KBD_DEV), \
                                &PrivateData);
    
    ASSERT (!EFI_ERROR (Status));

    KbTmpDp = DPAddNode(KbTmpDp, (EFI_DEVICE_PATH_PROTOCOL *) \
                                                &mEcPs2KbdDevicePath [0][0]);
    PrivateData->DevicePath = KbTmpDp;
    PrivateData->Handle = NULL;

    Status = pBS->InstallProtocolInterface (&PrivateData->Handle, \
                        &gEfiDevicePathProtocolGuid, EFI_NATIVE_INTERFACE, \
                        PrivateData->DevicePath);

    if (EFI_ERROR (Status)) {
        pBS->FreePool (PrivateData);
        return ;
    }

    // Install PS2 Mouse Device Path

    Status = pBS->AllocatePool (EfiBootServicesData, sizeof (PS2KBD_DEV), \
                        &PrivateData);
    
    ASSERT (!EFI_ERROR (Status));

    MsTmpDp = DPAddNode(MsTmpDp, (EFI_DEVICE_PATH_PROTOCOL *) \
                                                 &mEcPs2MsDevicePath [0][0]);
    PrivateData->DevicePath = MsTmpDp;
    PrivateData->Handle = NULL;

    Status = pBS->InstallProtocolInterface (&PrivateData->Handle, \
                    &gEfiDevicePathProtocolGuid, EFI_NATIVE_INTERFACE, \
                    PrivateData->DevicePath);

    if (EFI_ERROR (Status)) {
        pBS->FreePool (PrivateData);
        return ;
    }

    // Create/Update SIO_DEV_STATUS EFI variable

    Status = pRS->GetVariable(SIO_DEV_STATUS_VAR_NAME, &gSioDevStatusVarGuid, 
        &SioDevStatusVarAttributes, &SioDevStatusVarSize, 
        &SioDevStatusVar.DEV_STATUS);

    if (EFI_ERROR(Status)) {
        SioDevStatusVar.DEV_STATUS = 0;
        SioDevStatusVarAttributes = EFI_VARIABLE_BOOTSERVICE_ACCESS;
    }   

    SioDevStatusVar.Key60_64 = 1;
    SioDevStatusVar.Ps2Mouse = 1;

    Status = pRS->SetVariable(SIO_DEV_STATUS_VAR_NAME, &gSioDevStatusVarGuid, 
        SioDevStatusVarAttributes, SioDevStatusVarSize, 
        &SioDevStatusVar);
    ASSERT_EFI_ERROR( Status );
	
    // Consume IRQ1, so that it won't be allocated to other devices

    Status = AmiIsaIrqMask(&AvailableIsaIrq, TRUE);
    if(Status==EFI_NOT_FOUND){
            AvailableIsaIrq=ISA_IRQ_MASK;
    }
    AvailableIsaIrq |= 0x2;
    Status = AmiIsaIrqMask(&AvailableIsaIrq, FALSE);
    ASSERT_EFI_ERROR( Status );
    //
    //Kill the Event
    //
    pBS->CloseEvent(Event);
    
    return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   EcPs2KbdEntry
//
// Description: This function installs the Device Path for the EC Ps2.
//
// Input:       ImageHandle Image handle
//              SystemTable Pointer to the system table
//
// Output:      EFI_STATUS      Returned from PCI read call
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  EcPs2KbdEntry (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS                  Status;
    EFI_PHYSICAL_ADDRESS        IoAddress;
    DXE_SERVICES                *DxeSvcTbl=NULL;
    UINT8                       IoAllign;
    UINT8                       IoLength;

    
    InitAmiLib(ImageHandle,SystemTable);
    DxeSvcTbl=(DXE_SERVICES*)GetEfiConfigurationTable( SystemTable, \
                                                &gEfiDxeServicesTableGuid);

    IoAddress = EC_KBC_DATA_PORT;
    IoLength = 1;
    IoAllign=0;


    // Add KBC Data Port to GCD

    DxeSvcTbl->AddIoSpace (
                     EfiGcdIoTypeIo,
                     IoAddress,
                     IoLength
                     );

    // Allocate the KBC Data port from GCD

    DxeSvcTbl->AllocateIoSpace(EfiGcdAllocateAddress,
                                EfiGcdIoTypeIo,
                                IoAllign,
                                IoLength,
                                &IoAddress,
                                ImageHandle,
                                NULL);


    // Add KBC Command Port to GCD

    IoAddress = EC_KBC_CMD_PORT;
    IoLength = 1;
    IoAllign=0;

    DxeSvcTbl->AddIoSpace (
                     EfiGcdIoTypeIo,
                     IoAddress,
                     IoLength
                     );
    

    // Allocate the KBC Data port from GCD

    DxeSvcTbl->AllocateIoSpace(EfiGcdAllocateAddress,
                                EfiGcdIoTypeIo,
                                IoAllign,
                                IoLength,
                                &IoAddress,
                                ImageHandle,
                                NULL);

    Status = RegisterProtocolCallback ( &gBdsConnectDriversProtocolGuid, \
                                        InstallEcPs2Devices, \
                                        NULL, \
                                        &gEcPs2DeviceEvent, \
                                        &gEcPs2DeviceReg );

    ASSERT_EFI_ERROR(Status);
    
    return EFI_SUCCESS;
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
