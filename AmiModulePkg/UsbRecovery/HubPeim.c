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
//**********************************************************************
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        HubPeim.C
//
// Description: This file belongs to "Framework".
//              This file is modified by AMI to include copyright message,
//              appropriate header and integration code.
//              This file contains generic routines needed for USB recovery
//              PEIM
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

/*++
   This file contains 'Framework Code' and is licensed as such
   under the terms of your license agreement with Intel or your
   vendor.  This file may not be modified, except as allowed by
   additional terms of your license agreement.
   --*/

/*++

   Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.


   Module Name:

    HubPeim.c

   Abstract:

    Usb Hub Request Support In PEI Phase

   Revision History

   --*/
//NOT NEEDED FOR APTIO
//#include "Efi.h"
//#include "EfiDriverLib.h"
//#include "usb.h"

#include "HubPeim.h"
#include "UsbPeim.h"
#include "PeiUsbLib.h"
#include <Library/BaseMemoryLib.h>

#include EFI_PPI_DEFINITION( Stall )
static EFI_GUID gPeiStallPpiGuid = EFI_PEI_STALL_PPI_GUID;


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   PeiHubGetPortStatus
//
// Description: 
//      This function uses a device's PEI_USB_IO_PPI interface to execute a 
//      control transfer on the default control pipe to issue a Hub 
//      Class-specific request to obtain the port status and port status
//      change bits.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices
//                  --  PEI Sevices table pointer
//      IN PEI_USB_IO_PPI *UsbIoPpi
//                  --  PEI_USB_IO_PPI interface pointer for the device
//                      that is being accessed
//      IN UINT8 Port
//                  --  This value is the hub port number for which the
//                      status is to be returned.
//      OUT UINT32 *PortStatus
//                  --  This output value is the USB Specification 
//                      (Revision 2.0) Hub Port Status field (upper word)
//                      and Change Status field (lower word) values as 
//                      returned by the Get Port Status Hub Class device 
//                      standard request.
//
// Output: 
//      EFI_STATUS (Return Value)
//                  = EFI_SUCCESS on successful completion
//                      or valid EFI error code
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiHubGetPortStatus (
    IN  EFI_PEI_SERVICES    **PeiServices,
    IN  PEI_USB_IO_PPI      *UsbIoPpi,
    IN  UINT8               HubSpeed,
    IN  UINT8               Port,
    OUT UINT32              *PortStatus )
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;
	EFI_USB_PORT_STATUS     HubPortSts;
	USB3_HUB_PORT_STATUS    Usb3HubPortSts;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_GET_PORT_STATUS_REQ_TYPE;
    DevReq.Request = HUB_GET_PORT_STATUS;
    DevReq.Value = 0;
    DevReq.Index = Port;
    DevReq.Length = sizeof(UINT32);

    Timeout = 3000;

    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        UsbDataIn,
        Timeout,
        &HubPortSts,
        sizeof(UINT32)
                );

	if (HubSpeed == USB_SUPER_SPEED_DEVICE) {
		*((UINT32*)&Usb3HubPortSts) = *((UINT32*)&HubPortSts);
		*((UINT32*)&HubPortSts) = 0;

		if (Usb3HubPortSts.PortStatus.Connected) {
			HubPortSts.PortStatus |= USB_PORT_STAT_CONNECTION;
			HubPortSts.PortStatus |= USB_PORT_STAT_SUPER_SPEED;
		}
		if (Usb3HubPortSts.PortStatus.Enabled) {
			HubPortSts.PortStatus |= USB_PORT_STAT_ENABLE;
		}
		if (Usb3HubPortSts.PortStatus.OverCurrent) {
			HubPortSts.PortStatus |= USB_PORT_STAT_OVERCURRENT;
		}
		if (Usb3HubPortSts.PortStatus.Reset) {
			HubPortSts.PortStatus |= USB_PORT_STAT_RESET;
		}
		if (Usb3HubPortSts.PortStatus.Power) {
			HubPortSts.PortStatus |= USB_PORT_STAT_POWER;
		}
		if (Usb3HubPortSts.PortChange.ConnectChange) {
			HubPortSts.PortChangeStatus |= USB_PORT_STAT_C_CONNECTION;
		}
		if (Usb3HubPortSts.PortChange.OverCurrentChange) {
			HubPortSts.PortChangeStatus |= USB_PORT_STAT_C_OVERCURRENT;
		}
		if (Usb3HubPortSts.PortChange.ResetChange) {
			HubPortSts.PortChangeStatus |= USB_PORT_STAT_C_RESET;
		}
		if (Usb3HubPortSts.PortChange.BhResetChange) {
			HubPortSts.PortChangeStatus |= USB_PORT_STAT_C_CONNECTION;
		}
	}

	*PortStatus = *((UINT32*)&HubPortSts);

    return EfiStatus;
}


EFI_STATUS PeiHubSetPortFeature (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi,
    IN UINT8            Port,
    IN UINT8            Value )

/*++

   Routine Description:
    Set specified feature to a give hub port

   Arguments:
    UsbIoPpi           -   EFI_USB_IO_PROTOCOL instance
    Port            -   Usb hub port number (starting from 1).
    Value           -   New feature value.

   Returns:
    EFI_SUCCESS
    EFI_DEVICE
    EFI_TIME_OUT
    EFI_INVALID_PARAMETER

   --*/

{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_SET_PORT_FEATURE_REQ_TYPE;
    DevReq.Request = HUB_SET_PORT_FEATURE;
    DevReq.Value = Value;
    DevReq.Index = Port;
    DevReq.Length = 0;

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        EfiUsbNoData,
        Timeout,
        NULL,
        0
                );

    return EfiStatus;
}


EFI_STATUS PeiHubClearPortFeature (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi,
    IN UINT8            Port,
    IN UINT8            Value )

/*++

   Routine Description:
    Clear a specified feature of a given hub port

   Arguments:
    UsbIoPpi           -   EFI_USB_IO_PROTOCOL instance
    Port            -   Usb hub port number (starting from 1).
    Value           -   Feature value that will be cleared from
                        that hub port.

   Returns:
    EFI_SUCCESS
    EFI_DEVICE
    EFI_TIME_OUT
    EFI_INVALID_PARAMETER

   --*/
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_CLEAR_FEATURE_PORT_REQ_TYPE;
    DevReq.Request = HUB_CLEAR_FEATURE_PORT;
    DevReq.Value = Value;
    DevReq.Index = Port;
    DevReq.Length = 0;

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        EfiUsbNoData,
        Timeout,
        NULL,
        0
                );

    return EfiStatus;
}


EFI_STATUS PeiHubGetHubStatus (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi,
    OUT UINT32          *HubStatus )

/*++

   Routine Description:
    Get Hub Status

   Arguments:
    UsbIoPpi           -   EFI_USB_IO_PROTOCOL instance
    HubStatus       -   Current Hub status and change status.

   Returns:
    EFI_SUCCESS
    EFI_DEVICE
    EFI_TIME_OUT

   --*/
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_GET_HUB_STATUS_REQ_TYPE;
    DevReq.Request = HUB_GET_HUB_STATUS;
    DevReq.Value = 0;
    DevReq.Index = 0;
    DevReq.Length = sizeof(UINT32);

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        UsbDataIn,
        Timeout,
        HubStatus,
        sizeof(UINT32)
                );

    return EfiStatus;
}


EFI_STATUS PeiHubSetHubFeature (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi,
    IN UINT8            Value )

/*++

   Routine Description:
    Set a specified feature to the hub

   Arguments:
    UsbIoPpi           -   EFI_USB_IO_PROTOCOL instance
    Value           -   Feature value that will be set to the hub.

   Returns:
    EFI_SUCCESS
    EFI_DEVICE
    EFI_TIME_OUT

   --*/
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_SET_HUB_FEATURE_REQ_TYPE;
    DevReq.Request = HUB_SET_HUB_FEATURE;
    DevReq.Value = Value;
    DevReq.Index = 0;
    DevReq.Length = 0;

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        EfiUsbNoData,
        Timeout,
        NULL,
        0
                );

    return EfiStatus;
}


EFI_STATUS PeiHubClearHubFeature (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi,
    IN UINT8            Value )

/*++

   Routine Description:
    Set a specified feature to the hub

   Arguments:
    UsbIoPpi           -   EFI_USB_IO_PROTOCOL instance
    Value           -   Feature value that will be cleared from the hub.

   Returns:
    EFI_SUCCESS
    EFI_DEVICE
    EFI_TIME_OUT

   --*/
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_CLEAR_FEATURE_REQ_TYPE;
    DevReq.Request = HUB_CLEAR_FEATURE;
    DevReq.Value = Value;
    DevReq.Index = 0;
    DevReq.Length = 0;

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        EfiUsbNoData,
        Timeout,
        NULL,
        0
                );

    return EfiStatus;

}

EFI_STATUS PeiSetHubDepth (
    IN EFI_PEI_SERVICES        **PeiServices,
	IN PEI_USB_IO_PPI          *UsbIoPpi,
	IN UINT16                  HubDepth)
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = HUB_SET_HUB_DEPTH_REQ_TYPE;
    DevReq.Request = HUB_SET_HUB_DEPTH;
    DevReq.Value = HubDepth;
    DevReq.Index = 0;
    DevReq.Length = 0;

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        EfiUsbNoData,
        Timeout,
        NULL,
        0
                );

	return EfiStatus;
}

EFI_STATUS PeiGetHubDescriptor (
    IN EFI_PEI_SERVICES        **PeiServices,
    IN PEI_USB_IO_PPI          *UsbIoPpi,
    IN UINT8                   HubSpeed,
    IN UINTN                   DescriptorSize,
    OUT EFI_USB_HUB_DESCRIPTOR *HubDescriptor )

/*++

   Routine Description:
    Get the hub descriptor

   Arguments:
    UsbIoPpi           -   EFI_USB_IO_PROTOCOL instance
    DescriptorSize  -   The length of Hub Descriptor buffer.
    HubDescriptor   -   Caller allocated buffer to store the hub descriptor
                        if successfully returned.

   Returns:
    EFI_SUCCESS
    EFI_DEVICE
    EFI_TIME_OUT

   --*/
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;
	UINT8      DescriptorType;

	DescriptorType = HubSpeed == USB_SUPER_SPEED_DEVICE ? 
									USB_DT_SS_HUB : USB_DT_HUB;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = USB_RT_HUB | 0x80;
    DevReq.Request = HUB_GET_DESCRIPTOR;
    DevReq.Value = DescriptorType << 8;
    DevReq.Index = 0;
    DevReq.Length = (UINT16) DescriptorSize;

    Timeout = 3000;
    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        UsbDataIn,
        Timeout,
        HubDescriptor,
        (UINT16) DescriptorSize
                );

    return EfiStatus;

}


EFI_STATUS PeiDoHubConfig (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_DEVICE   *PeiUsbDevice )

/*++

   Routine Description:
    Configure the hub

   Arguments:
    PeiUsbDevice         -   Indicating the hub controller device that
                              will be configured

   Returns:
    EFI_SUCCESS
    EFI_DEVICE_ERROR

   --*/
{
    EFI_USB_HUB_DESCRIPTOR HubDescriptor;
    EFI_STATUS Status;
    EFI_USB_HUB_STATUS     HubStatus;
    UINTN  i;
    UINT32 PortStatus;
    PEI_USB_IO_PPI *UsbIoPpi;

    BOOLEAN SkipDebugPort = PeiUsbDevice->UsbHcPpi->DebugPortUsed;
//(EIP67320+)>
    PEI_STALL_PPI       *PeiStall;  
    (**PeiServices).LocatePpi(
        PeiServices,
        &gPeiStallPpiGuid,
        0,
        NULL,
        &PeiStall
    );
//<(EIP67320+)
    ZeroMem( &HubDescriptor, sizeof(HubDescriptor) );
    UsbIoPpi = &PeiUsbDevice->UsbIoPpi;

    //
    // First get the hub descriptor length
    //
    Status = PeiGetHubDescriptor( PeiServices,
        UsbIoPpi,
        PeiUsbDevice->DeviceSpeed,
        2,
        &HubDescriptor
             );
    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    //
    // First get the whole descriptor, then
    // get the number of hub ports
    //
    Status = PeiGetHubDescriptor(
        PeiServices,
        UsbIoPpi,
        PeiUsbDevice->DeviceSpeed,
        HubDescriptor.Length,
        &HubDescriptor
             );
    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    PeiUsbDevice->DownStreamPortNo = HubDescriptor.NbrPorts;

	if (PeiUsbDevice->DeviceSpeed == USB_SUPER_SPEED_DEVICE) {
		Status = PeiSetHubDepth(PeiServices,
			UsbIoPpi,
			PeiUsbDevice->HubDepth);
		if ( EFI_ERROR( Status ) ) {
        	return EFI_DEVICE_ERROR;
    	}
	}

    Status = PeiHubGetHubStatus( PeiServices,
        UsbIoPpi,
        (UINT32 *) &HubStatus
             );

    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    //
    //  Get all hub ports status
    //
    for (i = 0; i < PeiUsbDevice->DownStreamPortNo; i++) {

// Intel Debug port - Second port of the controller
    if(SkipDebugPort && (i == 1) ) 
        continue;

        Status = PeiHubGetPortStatus( PeiServices,
            UsbIoPpi,
            PeiUsbDevice->DeviceSpeed,
            (UINT8) (i + 1),
            &PortStatus
                 );
        if ( EFI_ERROR( Status ) ) {
            continue;
        }
    }

    //
    //  Power all the hub ports
    //
    for (i = 0; i < PeiUsbDevice->DownStreamPortNo; i++) {
// Intel Debug port - Second port of the controller
    if( SkipDebugPort && (i == 1) )
        continue;

        Status = PeiHubSetPortFeature( PeiServices,
            UsbIoPpi,
            (UINT8) (i + 1),
            EfiUsbPortPower
                 );
        if ( EFI_ERROR( Status ) ) {
            continue;
        }
    }

    //
    // Clear Hub Status Change
    //
    Status = PeiHubGetHubStatus( PeiServices,
        UsbIoPpi,
        (UINT32 *) &HubStatus
             );
    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }
    else {
        //
        // Hub power supply change happens
        //
        if (HubStatus.HubChange & HUB_CHANGE_LOCAL_POWER) {
            PeiHubClearHubFeature( PeiServices,
                UsbIoPpi,
                C_HUB_LOCAL_POWER
            );
        }

        //
        // Hub change overcurrent happens
        //
        if (HubStatus.HubChange & HUB_CHANGE_OVERCURRENT) {
            PeiHubClearHubFeature( PeiServices,
                UsbIoPpi,
                C_HUB_OVER_CURRENT
            );
        }
    }
    PeiStall->Stall( PeiServices, PeiStall, (HubDescriptor.PwrOn2PwrGood << 1) *1000);  //(EIP67320)
    return EFI_SUCCESS;

}


//
// Send reset signal over the given root hub port
//
VOID PeiResetHubPort (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi,
    IN UINT8            HubSpeed,
    UINT8               PortNum )
{
    PEI_STALL_PPI       *PeiStall;
    UINT8               n;
    EFI_USB_PORT_STATUS HubPortStatus;


    (**PeiServices).LocatePpi(
        PeiServices,
        &gPeiStallPpiGuid,
        0,
        NULL,
        &PeiStall
    );

	PeiHubGetPortStatus(
		 PeiServices,
		 UsbIoPpi,
		 HubSpeed,
		 PortNum,
		 (UINT32 *)&HubPortStatus
	 );

	if ((HubPortStatus.PortStatus & USB_PORT_STAT_ENABLE) == 0) {
	    //
	    // reset root port
	    //
	    PeiHubSetPortFeature(
	        PeiServices,
	        UsbIoPpi,
	        PortNum,
	        EfiUsbPortReset
	    );

	    n = 10;
	    do {
	        PeiHubGetPortStatus(
	            PeiServices,
	            UsbIoPpi,
	            HubSpeed,
	            PortNum,
	            (UINT32 *) &HubPortStatus
	        );
	        PeiStall->Stall(
	            PeiServices,
	            PeiStall,
	            1000
	        );
	        n -= 1;
	    } while ( (HubPortStatus.PortChangeStatus &
	               USB_PORT_STAT_C_RESET) == 0 && n > 0 );
	}
/*
    //
    // clear reset root port
    //
    PeiHubClearPortFeature(
        PeiServices,
        UsbIoPpi,
        PortNum,
        EfiUsbPortReset
    );

    PeiStall->Stall(
        PeiServices,
        PeiStall,
        500
    );

    //
    // Set port enable
    //
    PeiHubSetPortFeature(
        PeiServices,
        UsbIoPpi,
        PortNum,
        EfiUsbPortEnable
    );
*/
    PeiStall->Stall(
        PeiServices,
        PeiStall,
        500
    );

    //
    // Clear any change status
    //
    PeiHubClearPortFeature(
        PeiServices,
        UsbIoPpi,
        PortNum,
        EfiUsbPortResetChange
    );

    PeiHubClearPortFeature(
        PeiServices,
        UsbIoPpi,
        PortNum,
        EfiUsbPortConnectChange
    );

	if (HubSpeed == USB_SUPER_SPEED_DEVICE) {
	    PeiHubClearPortFeature(
	        PeiServices,
	        UsbIoPpi,
	        PortNum,
	        EfiUsbPortBhPortResetChange
	    );
	}


    return;
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
//**********************************************************************
