//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2010, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/amiusbhc.c 74    8/29/12 8:34a Ryanchou $
//
// $Revision: 74 $
//
// $Date: 8/29/12 8:34a $
//
//****************************************************************************
//<AMI_FHDR_START>
//
//  Name:           AmuUsbHc.c
//
//  Description:    USB_HC_PROTOCOL implementation
//
//<AMI_FHDR_END>
//****************************************************************************

//#include <Efi.h>
#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"

#include "Tree.h"
//#include "Uhci.h"
#include <Pci.h>
#include <Protocol/Cpu.h>
#include <Protocol/Legacy8259.h>

#define INTERRUPTQUEUESIZE 10

extern EFI_GUID  gEfiUsb2HcProtocolGuid;
extern USB_GLOBAL_DATA *gUsbData;
extern EFI_USB_PROTOCOL            *gAmiUsbController;

static UINT8 SpeedMap[] = { 0x10, 0x01, 0, 0 };

EFI_LEGACY_8259_PROTOCOL    *gPic;
UINT32  gVector = 0;

UINT8	*gUsbBusTempBuffer = NULL;
UINTN	gTempBufferPages = 0;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetThis
//
// Description:
//  Function returns a pointer to HcProtocol2 record of a given protocol
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

static HC_DXE_RECORD* GetThis(EFI_USB2_HC_PROTOCOL* Protocol)
{
    return (HC_DXE_RECORD*)(
        (char*)Protocol - (UINTN)&((HC_DXE_RECORD*)0)->hcprotocol2 );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetThis1
//
// Description:
//  Function returns a pointer to HcProtocol record of a given protocol
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

static HC_DXE_RECORD* GetThis1 (EFI_USB_HC_PROTOCOL* Protocol)
{
    return (HC_DXE_RECORD*)(
        (char*)Protocol - (UINTN)&((HC_DXE_RECORD*)0)->hcprotocol );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetTransferStatus
//
// Description:
//  Function converts the bitmap of gUsbData->dLastCommandStatusExtended into
//  a valid USB error.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

static UINT32 GetTransferStatus()
{
    static struct { int BitDst, BitSrc; } ErrorMap[] = {
        EFI_USB_ERR_STALL,      USB_TRSFR_STALLED,
        EFI_USB_ERR_STALL,      USB_BULK_STALLED,
        EFI_USB_ERR_STALL,      USB_CONTROL_STALLED,
        EFI_USB_ERR_BUFFER,     USB_TRSFR_BUFFER_ERROR,
        EFI_USB_ERR_BABBLE,     USB_TRNSFR_BABBLE,
        EFI_USB_ERR_NAK,        USB_TRNSFR_NAK,
        EFI_USB_ERR_CRC,        USB_TRNSFR_CRCERROR,
        EFI_USB_ERR_TIMEOUT,    USB_TRNSFR_TIMEOUT,	//(EIP84790)
        EFI_USB_ERR_BITSTUFF,   USB_TRNSFR_BITSTUFF,
        EFI_USB_ERR_SYSTEM, 0
    };
    UINT32 Err = 0;
    UINT32 Status = gUsbData->dLastCommandStatusExtended;
    UINT8 i;

    for (i = 0; i<COUNTOF(ErrorMap); ++i){
        if( Status & ErrorMap[i].BitSrc ) {
            Err |= ErrorMap[i].BitDst;
        }
    }

    if( Status == 0 ) {
        return EFI_USB_NOERROR;
    } else {
        return Err;
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbSmiCore
//
// Description:
//  Bridge between DXE code and function in USB Core proc table which is inside
//  our SMI code.
//
// Input:
//  Func - function number opUSBCORE_XXX which corresponds to index in Core Proc table.
//  Rest of the parameters coresponds the callee interface
//
// Output:
//  Whatever callee returns
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

enum {
    opUSBCORE_GetDescriptor,
    opUSBCORE_ReConfigDevice,
    opUSBCORE_ReConfigDevice2,
    opUSBCORE_AllocDevInfo,
    opUSBCORE_PrepareForLegacyOS,
    opUSBCORE_ResetAndReconfigDev,
    opUSBCORE_DevDriverDisconnect,
    opUSBCORE_PeriodicEvent,
    opUSBCORE_Last,
};


int CoreprocStackSize[] = {
    6*sizeof(VOID*), // opUSBSMI_GetDescriptor
    2*sizeof(VOID*), // opUSBCORE_ReConfigDevice
    4*sizeof(VOID*), // opUSBCORE_ReConfigDevice2
    1*sizeof(VOID*), // opUSBCORE_AllocDevInfo
    1*sizeof(VOID*), // opUSBCORE_PrepareForLegacyOS
    2*sizeof(VOID*), // opUSBCORE_ResetAndReconfigDev
    2*sizeof(VOID*), // opUSBCORE_DevDriverDisconnect
    0, // opUSBCORE_PeriodicEvent
};

UINTN UsbSmiCore( UINT8 Func, ...  )
{
    URP_STRUC   Params;
    va_list     ArgList;
    ASSERT(Func < COUNTOF(CoreprocStackSize));
//    ASSERT(CoreprocStackSize[Func] > 0);

    va_start(ArgList, Func);

    Params.bFuncNumber = USB_API_CORE_PROC;
    Params.bSubFunc = Func;
	Params.bRetValue = USB_ERROR;       //(EIP91840+)
    Params.ApiData.CoreProc.paramBuffer = &va_arg(ArgList, int);
    Params.ApiData.CoreProc.paramSize = CoreprocStackSize[Func];
    Params.ApiData.CoreProc.retVal = 0; //(EIP91840+)
    USB_DEBUG(DEBUG_USBHC_LEVEL8,
        "call CORE SMI proc(%d); params: %x\n", Func, Params.ApiData.CoreProc.paramBuffer);

	InvokeUsbApi(&Params);

	va_end(ArgList);

//    ASSERT(Params.bRetValue == USB_SUCCESS);

    return Params.ApiData.CoreProc.retVal;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    UsbSmiGetDescriptor
//
// Description:
//  SW SMI to execute GetDescriptor transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8*
UsbSmiGetDescriptor (
    HC_STRUC*   HostController,
    DEV_INFO*   Device,
    UINT8*      Buffer,
    UINT16      Length,
    UINT8       DescType,
    UINT8       DescIndex
)
{
	EFI_STATUS	Status;
	UINT8		*DataBuffer = Buffer;
	UINTN		ReturnValue;

	if (Shr64((UINTN)Buffer, 32)) {
		if (gUsbBusTempBuffer == NULL || Length > (gTempBufferPages << 12)) {
			if (gUsbBusTempBuffer) {
				gBS->FreePages((EFI_PHYSICAL_ADDRESS)gUsbBusTempBuffer, gTempBufferPages);
				gUsbBusTempBuffer = NULL;
				gTempBufferPages = 0;
			}
			gTempBufferPages = EFI_SIZE_TO_PAGES(Length);
			gUsbBusTempBuffer = (UINT8*)0xFFFFFFFF;
			Status = gBS->AllocatePages(AllocateMaxAddress, EfiBootServicesData,
							gTempBufferPages, (EFI_PHYSICAL_ADDRESS*)&gUsbBusTempBuffer);
			ASSERT_EFI_ERROR(Status);
			if (EFI_ERROR(Status)) {
				return NULL;
			}
			gBS->SetMem (gUsbBusTempBuffer, gTempBufferPages << 12, 0);
		}
		DataBuffer = gUsbBusTempBuffer;
	}
    ReturnValue = UsbSmiCore(opUSBCORE_GetDescriptor,
        HostController, Device, DataBuffer, Length, DescType, DescIndex);
	if (DataBuffer != Buffer) {
		gBS->CopyMem (Buffer, DataBuffer, Length);
	}

	return Buffer;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbSmiReConfigDevice
//
// Description:
//  Perform the device specific configuration
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbSmiReConfigDevice(
    HC_STRUC*   HostController,
    DEV_INFO*   Device
)
{
    return (UINT8)UsbSmiCore(opUSBCORE_ReConfigDevice, HostController, Device);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbAllocDevInfo
//
// Description:
//  Allocate the empty buffer for USB device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
UsbAllocDevInfo()
{
    return (DEV_INFO*)UsbSmiCore(opUSBCORE_AllocDevInfo, 1);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbPrepareForLegacyOS
//
// Description:
//  Callback on LEGACY_BOOT event
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID UsbPrepareForLegacyOS()
{
    UsbSmiCore(opUSBCORE_PrepareForLegacyOS, 1);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbResetAndReconfigDev
//
// Description:
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbResetAndReconfigDev(
    HC_STRUC*   HostController,
    DEV_INFO*   Device
)
{
    return (UINT8)UsbSmiCore(opUSBCORE_ResetAndReconfigDev, HostController, Device);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbDevDriverDisconnect
//
// Description:
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbDevDriverDisconnect(
    HC_STRUC*   HostController,
    DEV_INFO*   Device
)
{
    return (UINT8)UsbSmiCore(opUSBCORE_DevDriverDisconnect, HostController, Device);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbSmiHc
//
// Description:
//  Bridge between DXE code and SMI function in USB HC driver.
//
// Input:
//  Func       - function number opHC_XXX which corresponds to index in HCD_HEADER
//  HcType     - type of USB HC controller; selects an HC driver to call
//  Rest of the parameters coresponds the callee interface
//
// Output:
//  Whatever callee returns
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int HcprocStackSize[] = {
    sizeof(VOID*)*1, // opHC_Start
    sizeof(VOID*)*1, // opHC_Stop
    sizeof(VOID*)*1, // opHC_EnumeratePorts
    sizeof(VOID*)*1, // opHC_DisableInterrupts
    sizeof(VOID*)*1, // opHC_EnableInterrupts
    sizeof(VOID*)*1, // opHC_ProcessInterrupt
    sizeof(VOID*)*2, // opHC_GetRootHubStatus
    sizeof(VOID*)*2, // opHC_DisableRootHub
    sizeof(VOID*)*2, // opHC_EnableRootHub
    sizeof(VOID*)*7, // opHC_ControlTransfer
    sizeof(VOID*)*5, // opHC_BulkTransfer
    sizeof(VOID*)*4, // opHC_InterruptTransfer
    sizeof(VOID*)*2, // opHC_DeactivatePolling
    sizeof(VOID*)*2, // opHC_ActivatePolling
    0, // opHC_DisableKeyRepeat
    0, // opHC_EnableKeyRepeat
};

UINTN
UsbSmiHc(
    UINT8 Func,
    UINT8 HcType,
    ...
)
{
    URP_STRUC   Params;
    va_list     ArgList;

    ASSERT(Func < sizeof(HcprocStackSize)/sizeof(int));
    ASSERT( HcprocStackSize[Func] > 0);

    va_start(ArgList, HcType);

    Params.bFuncNumber = USB_API_HC_PROC;
    Params.bSubFunc = Func;
	Params.bRetValue = USB_ERROR;       //(EIP91840+)
    Params.ApiData.HcProc.paramBuffer = &va_arg(ArgList, int);
    Params.ApiData.HcProc.paramSize = HcprocStackSize[Func];
    Params.ApiData.HcProc.bHCType = HcType;
    Params.ApiData.HcProc.retVal = 0;   //(EIP91840+)
    USB_DEBUG(DEBUG_USBHC_LEVEL8,
        "call HC SMI driver(type:%d;func:%d); params at %x\n",
            HcType, Func, Params.ApiData.HcProc.paramBuffer);

	InvokeUsbApi(&Params);

    va_end(ArgList);
//    ASSERT(Params.bRetValue == USB_SUCCESS);

    return Params.ApiData.HcProc.retVal;
}

VOID
UsbSmiPeriodicEvent(VOID)
{
//    UsbSmiCore(opUSBCORE_PeriodicEvent);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    UsbSmiGetRootHubStatus, UsbSmiEnableRootHub, UsbSmiDisableRootHub
//
// Description:
//  Wrappers for calling USB HC driver functions in USBSMI service
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 UsbSmiGetRootHubStatus(HC_STRUC* HcStruc, UINT8 PortNum)
{
    return (UINT8)UsbSmiHc(
            opHC_GetRootHubStatus, HcStruc->bHCType, HcStruc, PortNum);
}

UINT8 UsbSmiEnableRootHub(HC_STRUC* HcStruc, UINT8 PortNum)
{
    return (UINT8)UsbSmiHc(
            opHC_EnableRootHub, HcStruc->bHCType, HcStruc, PortNum);
}

UINT8 UsbSmiDisableRootHub(HC_STRUC* HcStruc, UINT8 PortNum)
{
    return (UINT8)UsbSmiHc(
            opHC_DisableRootHub, HcStruc->bHCType, HcStruc, PortNum);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    UsbSmiControlTransfer, UsbSmiBulkTransfer, UsbSmiInterruptTransfer
//
// Description:
//  Wrappers for calling USB HC driver USB transfer functions
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>


UINT16
UsbSmiControlTransfer (
    HC_STRUC    *HcStruc,
    DEV_INFO    *DevInfo,
    UINT16      Request,
    UINT16      Index,
    UINT16      Value,
    UINT8       *Buffer,
    UINT16      Length)
{
	EFI_STATUS	Status;
	UINT8		*DataBuffer = Buffer;
	UINTN		ReturnValue;

	if (Shr64((UINTN)Buffer, 32)) {
		if (gUsbBusTempBuffer == NULL || Length > (gTempBufferPages << 12)) {
			if (gUsbBusTempBuffer) {
				gBS->FreePages((EFI_PHYSICAL_ADDRESS)gUsbBusTempBuffer, gTempBufferPages);
				gUsbBusTempBuffer = NULL;
				gTempBufferPages = 0;
			}
			gTempBufferPages = EFI_SIZE_TO_PAGES(Length);
			gUsbBusTempBuffer = (UINT8*)0xFFFFFFFF;
			Status = gBS->AllocatePages(AllocateMaxAddress, EfiBootServicesData,
							gTempBufferPages, (EFI_PHYSICAL_ADDRESS*)&gUsbBusTempBuffer);
			ASSERT_EFI_ERROR(Status);
			if (EFI_ERROR(Status)) {
				return 0;
			}
			gBS->SetMem (gUsbBusTempBuffer, gTempBufferPages << 12, 0);
		}
		DataBuffer = gUsbBusTempBuffer;
	}

    ReturnValue = UsbSmiHc(opHC_ControlTransfer,
        HcStruc->bHCType, HcStruc, DevInfo,
        Request, Index, Value, DataBuffer, Length );
	if (DataBuffer != Buffer) {
		gBS->CopyMem (Buffer, DataBuffer, Length);
	}

	return (UINT16)ReturnValue;
}

UINT32
UsbSmiBulkTransfer(
    HC_STRUC    *HcStruc,
    DEV_INFO    *DevInfo,
    UINT8       XferDir,
    UINT8       *Buffer,
    UINT32      Length
)
{
	EFI_STATUS	Status;
	UINT8		*DataBuffer = Buffer;
	UINTN		ReturnValue;

	if (Shr64((UINTN)Buffer, 32)) {
		if (gUsbBusTempBuffer == NULL || Length > (gTempBufferPages << 12)) {
			if (gUsbBusTempBuffer) {
				gBS->FreePages((EFI_PHYSICAL_ADDRESS)gUsbBusTempBuffer, gTempBufferPages);
				gUsbBusTempBuffer = NULL;
				gTempBufferPages = 0;
			}
			gTempBufferPages = EFI_SIZE_TO_PAGES(Length);
			gUsbBusTempBuffer = (UINT8*)0xFFFFFFFF;
			Status = gBS->AllocatePages(AllocateMaxAddress, EfiBootServicesData,
							gTempBufferPages, (EFI_PHYSICAL_ADDRESS*)&gUsbBusTempBuffer);
			ASSERT_EFI_ERROR(Status);
			if (EFI_ERROR(Status)) {
				return 0;
			}
			gBS->SetMem (gUsbBusTempBuffer, gTempBufferPages << 12, 0);
		}
		DataBuffer = gUsbBusTempBuffer;
	}

    ReturnValue = UsbSmiHc(opHC_BulkTransfer,
      HcStruc->bHCType, HcStruc, DevInfo, XferDir, DataBuffer, Length );
	if (DataBuffer != Buffer) {
		gBS->CopyMem (Buffer, DataBuffer, Length);
	}

	return (UINT32)ReturnValue;
}

UINT8
UsbSmiInterruptTransfer (
    HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT8       *Buffer,
    UINT16      Length
)
{
	EFI_STATUS	Status;
	UINT8		*DataBuffer = Buffer;
	UINTN		ReturnValue;

	if (Shr64((UINTN)Buffer, 32)) {
		if (gUsbBusTempBuffer == NULL || Length > (gTempBufferPages << 12)) {
			if (gUsbBusTempBuffer) {
				gBS->FreePages((EFI_PHYSICAL_ADDRESS)gUsbBusTempBuffer, gTempBufferPages);
				gUsbBusTempBuffer = NULL;
				gTempBufferPages = 0;
			}
			gTempBufferPages = EFI_SIZE_TO_PAGES(Length);
			gUsbBusTempBuffer = (UINT8*)0xFFFFFFFF;
			Status = gBS->AllocatePages(AllocateMaxAddress, EfiBootServicesData,
							gTempBufferPages, (EFI_PHYSICAL_ADDRESS*)&gUsbBusTempBuffer);
			ASSERT_EFI_ERROR(Status);
			if (EFI_ERROR(Status)) {
				return 0;
			}
			gBS->SetMem (gUsbBusTempBuffer, gTempBufferPages << 12, 0);
		}
		DataBuffer = gUsbBusTempBuffer;
	}

    ReturnValue = UsbSmiHc(opHC_InterruptTransfer,
        HcStruc->bHCType, HcStruc, DevInfo, DataBuffer, Length );
	if (DataBuffer != Buffer) {
		gBS->CopyMem (Buffer, DataBuffer, Length);
	}

	return (UINT8)ReturnValue;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    UsbSmiActivatePolling, UsbSmiDeactivatePolling
//
// Description:
//  Wrappers for calling USB HC driver USB polling functions
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 UsbSmiDeactivatePolling (HC_STRUC* HostController, DEV_INFO* DevInfo )
{
    return (UINT8)UsbSmiHc( opHC_DeactivatePolling,
            HostController->bHCType, HostController, DevInfo );
}

UINT8   UsbSmiActivatePolling (HC_STRUC* HostController, DEV_INFO* DevInfo )
{
    return (UINT8)UsbSmiHc( opHC_ActivatePolling,
            HostController->bHCType, HostController, DevInfo );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        ConvertBitmaps
//
// Description:
//  Converts one bit-strng to another using a convertion table
//
// Input:
//  Val - intial 32 bit wide bit-string
//  BitT- array of bitmaptable_t recodrds
//  Cnt - number of records in array BitT
//
// Output:
//  32-bit wide bit-string - result of conversion applied to Val
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

struct BITMAPTABLE_T {unsigned int src; unsigned int dst;};

UINT32
ConvertBitmaps(
    UINT32 Val,
    struct BITMAPTABLE_T* BitT,
    UINT32 Cnt
)
{
    UINT32 Res = 0;
    UINT32 i;
    for( i=0; (BitT->src !=0)&& ( i < Cnt); ++i, BitT++ ){
        if( BitT->src & Val )
            Res |= BitT->dst;
    }
    return Res;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        SearchDevinfoByAdr
//
// Description:
//  Enumerates DEV_INFO structures in aDevInfoTable array of USB data to find
//  one that matches the specified USB address and connects to a specified USB
//  host controller.
//
// Input:
//  DEV_INFO    Start Pointer to the device info structure from
//              where the search begins (if 0 start from first entry)
//  DevAddr    Device address
//  HcStruc    Pointer to the HCStruc structure
//
// Output:
//  Pointer to DEV_INFO structure, NULL if device is not found
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
SearchDevinfoByAdr(
    DEV_INFO*   Start,
    UINT8       DevAddr,
    HC_STRUC*   HcStruc
)
{
    DEV_INFO*   DevInfo;
    DEV_INFO*   Dev = gUsbData->aDevInfoTable + COUNTOF(gUsbData->aDevInfoTable);

    for ( DevInfo = Start!=NULL?Start:&gUsbData->aDevInfoTable[1];
            DevInfo != Dev; ++DevInfo ){
        if ((DevInfo->bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_DUMMY)) ==
			DEV_INFO_VALID_STRUC) {
            if(( gUsbData->HcTable[DevInfo->bHCNumber - 1] == HcStruc) &&
                ( DevInfo->bDeviceAddress == DevAddr )){
                    return DevInfo;
            }
        }
    }
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        DevAddr2Info
//
// Description:
//  Returns a DEV_INFO that corresponds to a device that is connected to a
//  specified host controller and has a specified address
//
// Input:
//  Addr    Device address
//  Hc      Pointer to the HCStruc structure
//
// Output:
//  Pointer to DEV_INFO structure, NULL if device is not found
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
DevAddr2Info(
    UINT8 Addr,
    HC_STRUC* Hc
)
{
    DEV_INFO* Dev = SearchDevinfoByAdr(NULL, Addr, Hc);
    USB_DEBUG(DEBUG_USBHC_LEVEL8,
        "\tDevAddr2Info %x -> %x(hc:%x;hub:%x;port:%x;if:%x)\n",
            Addr, Dev, Dev->bHCNumber, Dev->bHubDeviceNumber,
            Dev->bHubPortNumber, Dev->bInterfaceNum );
    return Dev;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        FindOldTransfer
//
// Description:
//  Searches a DEV_INFO that was used as a temporary structure for the USB transfer.
//
// Input:
//  Device address
//  EndPoint   Interrupt Endpoint number that was assigned to temporary structure
//  HCStruc    Pointer to the HCStruc structure
//
// Output:
//  Pointer to DeviceInfo Structure NULL if device is not found
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
FindOldTransfer(
    UINT8 DevAddr,
    UINT8 EndPoint,
    HC_STRUC* HcStruc
)
{
    DEV_INFO *Dev;
    DEV_INFO *LastDev = gUsbData->aDevInfoTable + 
                            COUNTOF(gUsbData->aDevInfoTable);

    for ( Dev = &gUsbData->aDevInfoTable[1]; Dev != LastDev; ++Dev ){
        if( ((Dev->bFlag & (DEV_INFO_VALID_STRUC|DEV_INFO_DEV_PRESENT) )==
            (DEV_INFO_VALID_STRUC|DEV_INFO_DEV_PRESENT)) &&
            (Dev->bHCNumber == HcStruc->bHCNumber ) &&
            (Dev->bDeviceAddress == DevAddr ) &&
            (Dev->bIntEndpoint == EndPoint ))
        {
            return Dev;
        }
    }
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AllocDevInfo
//
// Description:
//  Allocates temporary DEV_INFO structure in USB data area for use in USB transfer.
//
// Output:
//  Pointer to a DEV_INFO structure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
AllocDevInfo()
{
    DEV_INFO* Dev = UsbAllocDevInfo();
    ASSERT(Dev);
    if(Dev){
        Dev->bDeviceType = BIOS_DEV_TYPE_USBBUS_SHADOW;
        Dev->bFlag |= DEV_INFO_DEV_DUMMY;
    }
    return Dev;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        FreeDevInfo
//
// Description:
//  Marks DEV_INFO structure that it is free for use in consequent operations.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
FreeDevInfo(
    DEV_INFO* Dev
)
{
    ASSERT(Dev);
    if (Dev) {
      Dev->bFlag &= ~(DEV_INFO_VALID_STRUC |  DEV_INFO_DEV_PRESENT | DEV_INFO_DEV_DUMMY);
    }
}

UINT8
TranslateInterval(
    UINT8   Speed,
    UINTN   Interval
)
{
    UINT8  BitCount = 0;

    if (Speed == EFI_USB_SPEED_LOW ||
        Speed == EFI_USB_SPEED_FULL) {
        return (UINT8)Interval;
    }

    for (BitCount = 0; Interval != 0; BitCount++) {
        Interval >>= 1;
    }
    return (BitCount + 2);
}

/////////////////////////////////////////////////////////////////////////////
//                  USB Host Controller API functions
/////////////////////////////////////////////////////////////////////////////

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcReset
//
// Description:
//  Provides software reset for the USB host controller.
//
// Input:
//  This        A pointer to the EFI_USB_HC_PROTOCOL instance. Type
//              EFI_USB_HC_PROTOCOL is defined in Section 14.1.
//  Attributes  A bit mask of the reset operation to perform.
//
// Output:
//  EFI_SUCCESS             The reset operation succeeded.
//  EFI_INVALID_PARAMETER   Attributes is not valid.
//  EFI_UNSUPPORTED         The type of reset specified by Attributes is
//                          not currently supported by the host controller
//                          hardware.
//  EFI_DEVICE_ERROR        An error was encountered while attempting to
//                          perform the reset operation.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcReset(
    IN EFI_USB_HC_PROTOCOL *This,
    IN UINT16 attributes
)
{
    PROGRESS_CODE(DXE_USB_RESET);

    if (!(attributes & (EFI_USB_HC_RESET_GLOBAL | EFI_USB_HC_RESET_HOST_CONTROLLER)))
        return EFI_INVALID_PARAMETER;

    USB_DEBUG(DEBUG_USBHC_LEVEL, "AmiUsbHcReset:");

    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcGetState
//
// Description:
//  Protocol USB HC function that returns Host Controller state.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcGetState(
    IN EFI_USB_HC_PROTOCOL  *This,
    OUT EFI_USB_HC_STATE *State
)
{
    if (State == NULL) return EFI_INVALID_PARAMETER;
    *State = EfiUsbHcStateOperational;
    return EFI_DEVICE_ERROR;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    AmiUsbHcSetState
//
// Description:
//  Protocol USB HC function that sets Host Controller state.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbHcSetState(
    IN EFI_USB_HC_PROTOCOL *This,
    IN EFI_USB_HC_STATE State
)
{
    USB_DEBUG(DEBUG_USBHC_LEVEL, "USB HC:\t\tsetState, %d\n", State);
    if (State >= EfiUsbHcStateMaximum || State < EfiUsbHcStateHalt)
        return EFI_INVALID_PARAMETER;
    return EFI_DEVICE_ERROR;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    AmiUsbHcTransfer
//
// Description:
//  Protocol USB HC function that performs USB transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcTransfer(
    IN EFI_USB_HC_PROTOCOL          *HcProtocol,
    IN     UINT8                    DeviceAddress,
    IN     BOOLEAN                  IsSlowDevice,
    IN     UINT8                    MaximumPacketLength,
    IN     EFI_USB_DEVICE_REQUEST   *Request,
    IN     EFI_USB_DATA_DIRECTION   TransferDirection,
    IN OUT VOID                     *Data,
    IN OUT UINTN                    *DataLength,
    IN     UINTN                    Timeout,
    OUT    UINT32                   *TransferResult
)
{
    return AmiUsb2HcTransfer(&GetThis1(HcProtocol)->hcprotocol2,
        DeviceAddress,
        IsSlowDevice?EFI_USB_SPEED_LOW:EFI_USB_SPEED_FULL,
        MaximumPacketLength, Request, TransferDirection, Data,
        DataLength, Timeout, NULL, TransferResult);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcBulkTransfer
//
// Description:
//  Protocol USB HC function that performs USB bulk transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI AmiUsbHcBulkTransfer(
    IN EFI_USB_HC_PROTOCOL  *HcProtocol,
    IN  UINT8               DeviceAddress,
    IN  UINT8               EndpointAddress,
    IN  UINT8               MaximumPacketLength,
    IN OUT VOID             *Data,
    IN OUT UINTN            *DataLength,
    IN OUT UINT8            *DataToggle,
    IN  UINTN               Timeout,
    OUT UINT32              *TransferResult
)
{
    //
    // Check for valid maximum packet length is 8, 16, 32 or 64
    //
    if ( MaximumPacketLength != 8 &&
         MaximumPacketLength != 16 &&
         MaximumPacketLength != 32 &&
         MaximumPacketLength != 64 ) return EFI_INVALID_PARAMETER;

    return AmiUsb2HcBulkTransfer(&GetThis1(HcProtocol)->hcprotocol2,
        DeviceAddress, EndpointAddress, EFI_USB_SPEED_FULL,
        MaximumPacketLength, 1, &Data, DataLength,
        DataToggle, Timeout, NULL, TransferResult);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcAsyncInterruptTransfer
//
// Description:
//  Protocol USB HC function that performs USB async interrupt transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcAsyncInterruptTransfer(
  IN EFI_USB_HC_PROTOCOL                  *HcProtocol,
  IN     UINT8                            DeviceAddress,
  IN     UINT8                            EndpointAddress,
  IN     BOOLEAN                          IsSlowDevice,
  IN     UINT8                            MaxPacket,
  IN     BOOLEAN                          IsNewTransfer,
  IN OUT UINT8                            *DataToggle,
  IN     UINTN                            PollingInterval  ,
  IN     UINTN                            DataLength,
  IN     EFI_ASYNC_USB_TRANSFER_CALLBACK  CallbackFunction ,
  IN     VOID                             *Context)
{
    return AmiUsb2HcAsyncInterruptTransfer(
            &GetThis1(HcProtocol)->hcprotocol2,
            DeviceAddress, EndpointAddress,
            IsSlowDevice?EFI_USB_SPEED_LOW:EFI_USB_SPEED_FULL,
            MaxPacket, IsNewTransfer, DataToggle,
            PollingInterval, DataLength, NULL, CallbackFunction, Context);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcSyncInterruptTransfer
//
// Description:
//  Protocol USB HC function that performs USB sync interrupt transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
AmiUsbHcSyncInterruptTransfer(
  IN EFI_USB_HC_PROTOCOL    *HcProtocol,
  IN     UINT8              DeviceAddress,
  IN     UINT8              EndpointAddress,
  IN     BOOLEAN            IsSlowDevice,
  IN     UINT8              MaximumPacketLength,
  IN OUT VOID               *Data,
  IN OUT UINTN              *DataLength,
  IN OUT UINT8              *DataToggle,
  IN     UINTN              Timeout,
  OUT    UINT32             *TransferResult
)
{
    return AmiUsb2HcSyncInterruptTransfer(
            &GetThis1(HcProtocol)->hcprotocol2,
            DeviceAddress, EndpointAddress,
            IsSlowDevice?EFI_USB_SPEED_LOW:EFI_USB_SPEED_FULL,
            MaximumPacketLength,
            Data, DataLength, DataToggle, Timeout, NULL, TransferResult );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcIsochronousTransfer
//
// Description:
//  Protocol USB HC function that performs USB sync isochronous transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcIsochronousTransfer(
  IN EFI_USB_HC_PROTOCOL    *This,
  IN     UINT8              DeviceAddress,
  IN     UINT8              EndpointAddress,
  IN     UINT8              MaximumPacketLength,
  IN OUT VOID               *Data,
  IN OUT UINTN              DataLength,
  OUT    UINT32             *TransferResult
)
{
    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcAsyncIsochronousTransfer
//
// Description:
//  Protocol USB HC function that performs USB async isochronous transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcAsyncIsochronousTransfer(
  IN EFI_USB_HC_PROTOCOL              *This,
  IN     UINT8                        DeviceAddress,
  IN     UINT8                        EndpointAddress,
  IN     UINT8                        MaximumPacketLength,
  IN OUT VOID                         *Data,
  IN     UINTN                        DataLength,
  IN EFI_ASYNC_USB_TRANSFER_CALLBACK  IsochronousCallback,
  IN VOID                             *Context
  )
{
    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcGetRootHubPortNumber
//
// Description:
//  Protocol USB HC function that returns the number of ports of a root hub
//  on a given controller.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcGetRootHubPortNumber (
    IN EFI_USB_HC_PROTOCOL *HcProtocol,
    OUT UINT8 *PortNumber
)
{
    HC_DXE_RECORD *This = GetThis1(HcProtocol);

    USB_DEBUG(DEBUG_USBHC_LEVEL,
        "USB HC:\t\tget_roothub_port_number\n");

    if (PortNumber == NULL) return EFI_INVALID_PARAMETER;

    *PortNumber = This->hc_data->bNumPorts;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcGetRootHubPortStatus
//
// Description:
//  Protocol USB HC function that returns the root port status
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcGetRootHubPortStatus (
    EFI_USB_HC_PROTOCOL *HcProtocol,
    UINT8               PortNumber,
    EFI_USB_PORT_STATUS *PortStatus
)
{
    HC_DXE_RECORD *This = GetThis1( HcProtocol );

    static struct BITMAPTABLE_T MapTable[] = {
        {USB_PORT_STAT_DEV_CONNECTED,USB_PORT_STAT_CONNECTION},
        {USB_PORT_STAT_DEV_LOWSPEED,USB_PORT_STAT_LOW_SPEED},
    };
    static struct BITMAPTABLE_T MapTable2[] = {
        {USB_PORT_STAT_DEV_CONNECT_CHANGED,USB_PORT_STAT_C_CONNECTION},
    };

    UINT8   Status1;

    USB_DEBUG(DEBUG_USBHC_LEVEL, "USB HC:\t\tget_roothub_port_status\n" );

    if (PortStatus == NULL || PortNumber >= This->hc_data->bNumPorts)
        return EFI_INVALID_PARAMETER;

    Status1 = UsbSmiGetRootHubStatus(This->hc_data,(UINT8)PortNumber+1);

    PortStatus->PortStatus = (UINT16)ConvertBitmaps(
            Status1, MapTable, COUNTOF(MapTable));
    PortStatus->PortChangeStatus = (UINT16)ConvertBitmaps(
            Status1, MapTable2, COUNTOF(MapTable2)) ;
    PortStatus->PortStatus |= USB_PORT_STAT_ENABLE;   // All ports are enabled

    if (((Status1 & USB_PORT_STAT_DEV_CONNECTED) != 0) &&
        (Status1 & (USB_PORT_STAT_DEV_LOWSPEED|USB_PORT_STAT_DEV_FULLSPEED))==0) {
        PortStatus->PortStatus |= USB_PORT_STAT_HIGH_SPEED;
    }

    USB_DEBUG(DEBUG_USBHC_LEVEL, "\t\tStatus=%x, PortStatus=%x, PortChangeStatus=%x\n",
        Status1, PortStatus->PortStatus, PortStatus->PortChangeStatus );

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  AmiUsbHcSetRootHubPortFeature
//
// Description:
//  Protocol USB HC function set root hub port feature
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcSetRootHubPortFeature(
    IN EFI_USB_HC_PROTOCOL  *HcProtocol,
    IN UINT8                PortNumber,
    IN EFI_USB_PORT_FEATURE PortFeature
)
{
    HC_DXE_RECORD *This = GetThis1(HcProtocol);

    USB_DEBUG(DEBUG_USBHC_LEVEL, "USB HC:\t\tset_roothub_port_feature\n" );

    if ( PortNumber >= This->hc_data->bNumPorts)
        return EFI_INVALID_PARAMETER;

    switch( PortFeature ){
        case EfiUsbPortEnable:
            UsbSmiEnableRootHub(This->hc_data, PortNumber);
            break;
    
        default:
            return EFI_INVALID_PARAMETER;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbHcClearRootHubPortFeature
//
// Description:
//  Protocol USB HC function clear root hub port feature
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsbHcClearRootHubPortFeature(
    IN EFI_USB_HC_PROTOCOL    *HcProtocol,
    IN UINT8                  PortNumber,
    IN EFI_USB_PORT_FEATURE   PortFeature
)
{
    HC_DXE_RECORD *This = GetThis1(HcProtocol);

    USB_DEBUG(DEBUG_USBHC_LEVEL, "USB HC:\t\tclear_roothub_port_feature\n");

    if (PortNumber >= This->hc_data->bNumPorts) return EFI_INVALID_PARAMETER;

    switch (PortFeature ) {
        case EfiUsbPortEnable:
            UsbSmiDisableRootHub(This->hc_data, PortNumber);
            break;
    
        default:
            return EFI_INVALID_PARAMETER;
    }
    return EFI_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
//                  USB2 Host Controller API functions
/////////////////////////////////////////////////////////////////////////////

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    AmiUsb2HcGetCapability
//
// Description:
//  This is USB2HC API to get the host controller capability.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsb2HcGetCapability(
  IN  EFI_USB2_HC_PROTOCOL  *This,
  OUT UINT8                 *MaxSpeed,
  OUT UINT8                 *PortNumber,
  OUT UINT8                 *Is64BitCapable
)
{
    HC_DXE_RECORD *Rec = GetThis(This);

    if (MaxSpeed == NULL || PortNumber == NULL || Is64BitCapable == NULL)
        return EFI_INVALID_PARAMETER;
    *Is64BitCapable = FALSE;    // Driver currently does not support >4GB allocations
                                // even if controller does
                                        //(EIP81612)>
    //*MaxSpeed = (Rec->hc_data->bHCType==USB_HC_EHCI)?
    //                EFI_USB_SPEED_HIGH : EFI_USB_SPEED_FULL;
    switch (Rec->hc_data->bHCType) {
        case USB_HC_OHCI:
        case USB_HC_UHCI:
            *MaxSpeed = EFI_USB_SPEED_FULL;
            break;
        case USB_HC_EHCI:
            *MaxSpeed = EFI_USB_SPEED_HIGH;
            break;
        case USB_HC_XHCI:
            *MaxSpeed = EFI_USB_SPEED_SUPER;
            break;
    }
                                        //<(EIP81612)

    *PortNumber = Rec->hc_data->bNumPorts;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    AmiUsb2HcReset
//
// Description:
//  This is USB2HC API to perform host controller reset.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsb2HcReset(
  IN EFI_USB2_HC_PROTOCOL   *This,
  IN UINT16                 Attributes
)
{
    PROGRESS_CODE(DXE_USB_RESET);

    if (Attributes == 0 || (Attributes & ~(EFI_USB_HC_RESET_GLOBAL
        | EFI_USB_HC_RESET_HOST_CONTROLLER | EFI_USB_HC_RESET_GLOBAL_WITH_DEBUG
        | EFI_USB_HC_RESET_HOST_WITH_DEBUG))==0 )
    {
        return EFI_INVALID_PARAMETER;
    }

    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    AmiUsb2HcGetState
//
// Description:
//  This is USB2HC API to get the host controller state.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcGetState(
  IN  EFI_USB2_HC_PROTOCOL    *This,
  OUT EFI_USB_HC_STATE        *State
)
{
    if (State == NULL) return EFI_INVALID_PARAMETER;
    *State = EfiUsbHcStateOperational;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    AmiUsb2HcSetState
//
// Description:
//  This is USB2HC API to set the host controller state.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcSetState(
  IN EFI_USB2_HC_PROTOCOL    *This,
  IN EFI_USB_HC_STATE        State
)
{
    if (State >= EfiUsbHcStateMaximum || State < EfiUsbHcStateHalt)
        return EFI_INVALID_PARAMETER;
    return EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AsyncInterruptOnTimer
//
// Description:
//  This function checks if queue has a new transfer. If yes, calls a
//  callback with data from transfer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EFIAPI
AsyncInterruptOnTimer (
    EFI_EVENT   Event,
    VOID        *Ctx
)
{
    USBHC_INTERRUPT_DEVNINFO_T *Idi = (USBHC_INTERRUPT_DEVNINFO_T*)Ctx;
    UINT8   Lock;
    VOID    *Data;

    //
    // Check re-entrance
    //
    ATOMIC({Lock = Idi->Lock; Idi->Lock = 1;});
    if(Lock) {
        return; //control is already inside
    }

    while((UINTN)QueueSize(&Idi->QCompleted) >= Idi->DataLength ){
    ATOMIC(Data = QueueRemoveMsg( &Idi->QCompleted, (int)Idi->DataLength ));
        //TRACE((-1,"USBHC: AsyncInterruptOnTimer: calling callback...\n"));
        Idi->CallbackFunction(
            Data, Idi->DataLength, Idi->Context, EFI_USB_NOERROR);
    }

    Idi->Lock = 0;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcBulkTransfer
//
// Description:
//  This function performs USB2 HC Bulk Transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcBulkTransfer(
    IN EFI_USB2_HC_PROTOCOL *HcProtocol,
    IN  UINT8       DeviceAddress,
    IN  UINT8       EndpointAddress,
    IN  UINT8       DeviceSpeed,
    IN  UINTN       MaximumPacketLength,
    IN  UINT8       DataBuffersNumber,
    IN OUT VOID     *Data[EFI_USB_MAX_BULK_BUFFER_NUM],
    IN OUT UINTN    *DataLength,
    IN OUT UINT8    *DataToggle,
    IN  UINTN       Timeout,
    IN EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    OUT UINT32      *TransferResult
)
{
    UINT32  SmiRes;
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    DEV_INFO* DevInfo;
    DEV_INFO* DevSrc = DevAddr2Info( DeviceAddress, This->hc_data );
    UINT8 XferDir = 0;
    UINT16 CurrentTimeout;
	UINT8 ToggleBit = (EndpointAddress & 0xF) - 1;

    //
    // Check Params
    //
                                        //(EIP81612)>
    if( DevSrc == NULL || Data == NULL || Data[0] == NULL ||
        (*DataToggle != 0 && *DataToggle != 1) ||
        (DeviceSpeed != EFI_USB_SPEED_SUPER && DeviceSpeed != EFI_USB_SPEED_HIGH 
        && DeviceSpeed != EFI_USB_SPEED_FULL) ||
        *DataLength == 0 || TransferResult == NULL ||
        MaximumPacketLength == 0 ) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Check for valid maximum packet length
    //
    if ( DeviceSpeed == EFI_USB_SPEED_SUPER && MaximumPacketLength > 1024 ) {
        return EFI_INVALID_PARAMETER;
    }
                                        //<(EIP81612)
    if ( DeviceSpeed == EFI_USB_SPEED_HIGH && MaximumPacketLength > 512 ) {
        return EFI_INVALID_PARAMETER;
    }
    if ( DeviceSpeed == EFI_USB_SPEED_FULL && MaximumPacketLength > 64 ) {
        return EFI_INVALID_PARAMETER;
    }

    //
    // Alloc DEV_INFO
    //
    DevInfo = AllocDevInfo();

    if(DevInfo == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    //
    // Fill DEV_INFO
    //
    DevInfo->bDeviceAddress = DeviceAddress;
    DevInfo->bHCNumber = This->hc_data->bHCNumber;
    DevInfo->bEndpointSpeed = SpeedMap[DeviceSpeed];
    DevInfo->bHubDeviceNumber = DevSrc->bHubDeviceNumber;
    DevInfo->bHubPortNumber = DevSrc->bHubPortNumber;
    DevInfo->DevMiscInfo = DevSrc->DevMiscInfo;	//(EIP84790+)

    if( EndpointAddress & 0x80 ){
        XferDir = 0x80;
        DevInfo->bBulkInEndpoint = EndpointAddress & 0xF;
        DevInfo->wBulkInMaxPkt = (UINT16)MaximumPacketLength;
        DevInfo->wDataInSync = (UINT16)(*DataToggle) << ToggleBit;
    } else {
        XferDir = 0x0;
        DevInfo->bBulkOutEndpoint = EndpointAddress & 0xF;
        DevInfo->wBulkOutMaxPkt = (UINT16)MaximumPacketLength;
        DevInfo->wDataOutSync = (UINT16)(*DataToggle) << ToggleBit;
    }

    //
    // Call SMI routine and retrieve last status
    // if any error
    //
    CRITICAL_CODE( EFI_TPL_NOTIFY,
    {
        CurrentTimeout = gUsbData->wTimeOutValue;
        gUsbData->wTimeOutValue = (UINT16)Timeout;
        gUsbData->dLastCommandStatusExtended = 0;
        SmiRes = UsbSmiBulkTransfer(
            This->hc_data, DevInfo,
            XferDir,
            (UINT8*)Data[0],
            (UINT32)*DataLength );
        *TransferResult = GetTransferStatus();
        gUsbData->wTimeOutValue = CurrentTimeout;
    });

    //
    // Update the data length
    //
    *DataLength = (UINTN)SmiRes;

    //
    // Update Toggle bit
    //
    if( XferDir ){
        *DataToggle = (UINT8)(DevInfo->wDataInSync >> ToggleBit) & 0x1;
    } else {
        *DataToggle = (UINT8)(DevInfo->wDataOutSync >> ToggleBit) & 0x1;
    }
    FreeDevInfo(DevInfo);

    if( (*TransferResult) &  EFI_USB_ERR_TIMEOUT ) {
        return EFI_TIMEOUT;
    }
    return (*TransferResult)? EFI_DEVICE_ERROR:EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcAsyncInterruptTransfer
//
// Description:
//  This function performs USB2 HC Async Interrupt Transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcAsyncInterruptTransfer(
    IN EFI_USB2_HC_PROTOCOL *HcProtocol,
    IN UINT8                DeviceAddress,
    IN UINT8                EndpointAddress,
    IN UINT8                DeviceSpeed,
    IN UINTN                MaxPacket,
    IN BOOLEAN              IsNewTransfer,
    IN OUT UINT8            *DataToggle,
    IN UINTN                PollingInterval,
    IN UINTN                DataLength,
    IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    IN EFI_ASYNC_USB_TRANSFER_CALLBACK  CallbackFunction ,
    IN VOID                 *Context)
{
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    DEV_INFO* DevInfo;
    USBHC_INTERRUPT_DEVNINFO_T* AsyncTransfer = 0;
    UINT8 SmiStatus = USB_SUCCESS;
	UINT8 ToggleBit = (EndpointAddress & 0xF) - 1;

    if (DeviceSpeed != EFI_USB_SPEED_SUPER && 
		DeviceSpeed != EFI_USB_SPEED_HIGH &&
		DeviceSpeed != EFI_USB_SPEED_FULL &&
		DeviceSpeed != EFI_USB_SPEED_LOW){
		return EFI_INVALID_PARAMETER;
    }

#if !EFI_USB_HC_INTERRUPT_OUT_SUPPORT
    if (!(EndpointAddress & BIT7)) {
		return EFI_INVALID_PARAMETER;
    }
#endif
                                        //<(EIP81612)
    if (IsNewTransfer){
        DEV_INFO* DevInfoSrc = DevAddr2Info( DeviceAddress, This->hc_data );
        DevInfo = FindOldTransfer( DeviceAddress, EndpointAddress, This->hc_data );

		if (DataLength == 0) {
			 return EFI_INVALID_PARAMETER;
		}

	    if (*DataToggle != 0 && *DataToggle != 1) {
	        return EFI_INVALID_PARAMETER;
	    }

		if (PollingInterval < 1 || PollingInterval > 255) {
			return EFI_INVALID_PARAMETER;
		}
	
        if( DevInfoSrc == NULL || CallbackFunction == NULL) {
            return EFI_INVALID_PARAMETER;
        }
        if( DevInfo != NULL ){
            USB_DEBUG(DEBUG_LEVEL_3,"Stacked AsyncInterrupt request are not supported\n");
            return EFI_INVALID_PARAMETER;
        }
        DevInfo = AllocDevInfo();
        *DevInfo = *DevInfoSrc;
        DevInfo->bIntEndpoint = EndpointAddress;
        DevInfo->bEndpointSpeed = SpeedMap[DeviceSpeed];
        DevInfo->wIntMaxPkt = (UINT16)MaxPacket;    //(EIP84790+)
        DevInfo->bPollInterval = TranslateInterval(DeviceSpeed, PollingInterval);

        //create new transfer
        gBS->AllocatePool (EfiBootServicesData,
            sizeof(USBHC_INTERRUPT_DEVNINFO_T) + DataLength*INTERRUPTQUEUESIZE,
            &AsyncTransfer );
        EfiZeroMem(AsyncTransfer, sizeof(USBHC_INTERRUPT_DEVNINFO_T)+
                    DataLength*INTERRUPTQUEUESIZE);
        DevInfo->pExtra = (UINT8*)AsyncTransfer;
        AsyncTransfer->QCompleted.data = (VOID *volatile *)AsyncTransfer->Data;
        AsyncTransfer->QCompleted.maxsize = (int)DataLength * INTERRUPTQUEUESIZE;
        AsyncTransfer->DataLength = DataLength;
        AsyncTransfer->EndpointAddress = EndpointAddress;

        DevInfo->bFlag |= DEV_INFO_DEV_DUMMY;
        DevInfo->bIntEndpoint = EndpointAddress;
        VERIFY_EFI_ERROR(
            gBS->CreateEvent ( EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
            EFI_TPL_CALLBACK, AsyncInterruptOnTimer,
            AsyncTransfer,&AsyncTransfer->Event));

		PollingInterval = PollingInterval < 32 ? 32 : PollingInterval;
        VERIFY_EFI_ERROR(
            gBS->SetTimer ( AsyncTransfer->Event, TimerPeriodic,
                PollingInterval * MILLISECOND));
        AsyncTransfer->CallbackFunction = CallbackFunction;
        AsyncTransfer->Context = Context;
		if(EndpointAddress & 0x80) {
	        DevInfo->wDataInSync = (UINT16)(*DataToggle) << ToggleBit;
		} else {
			DevInfo->wDataOutSync = (UINT16)(*DataToggle) << ToggleBit;
		}

        //
        // Activate transfer
        //
        SmiStatus = UsbSmiActivatePolling(This->hc_data, DevInfo);
        ASSERT(SmiStatus==USB_SUCCESS);
    } else {
        //
        // Find old transfer
        //
        DevInfo = FindOldTransfer(
                    DeviceAddress,
                    EndpointAddress,
                    This->hc_data );
        if( DevInfo == NULL  || DevInfo->pExtra == NULL ){
            USB_DEBUG(DEBUG_LEVEL_3,"Canceling bad AsyncInterrupt request\n");
            return EFI_INVALID_PARAMETER;
        }
        AsyncTransfer = (USBHC_INTERRUPT_DEVNINFO_T*)DevInfo->pExtra;
        DevInfo->pExtra = 0;
        // 
        // Deactivate transfer
        //
        SmiStatus = UsbSmiDeactivatePolling (This->hc_data, DevInfo);
        if (DataToggle){
			if(EndpointAddress & 0x80) {
				*DataToggle = (UINT8)(DevInfo->wDataInSync >> ToggleBit) & 0x1;
			} else {
				*DataToggle = (UINT8)(DevInfo->wDataOutSync >> ToggleBit) & 0x1;
			}
        }
        VERIFY_EFI_ERROR(gBS->SetTimer(AsyncTransfer->Event, TimerCancel, 0));
        VERIFY_EFI_ERROR(gBS->CloseEvent (AsyncTransfer->Event));
        gBS->FreePool(AsyncTransfer);
        FreeDevInfo(DevInfo);
    }

    return SmiStatus == USB_SUCCESS? EFI_SUCCESS : EFI_DEVICE_ERROR;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcSyncInterruptTransfer
//
// Description:
//  This function performs USB2 HC Sync Interrupt Transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcSyncInterruptTransfer(
    IN EFI_USB2_HC_PROTOCOL    *HcProtocol,
    IN     UINT8                DeviceAddress,
    IN     UINT8                EndpointAddress,
    IN     UINT8                DeviceSpeed,
    IN     UINTN                MaximumPacketLength,
    IN OUT VOID                 *Data,
    IN OUT UINTN                *DataLength,
    IN OUT UINT8                *DataToggle,
    IN     UINTN                Timeout,
    IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    OUT    UINT32                *TransferResult  )
{
    UINT16      SmiRes;
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    DEV_INFO    *DevInfo;
    DEV_INFO*   DevSrc = DevAddr2Info( DeviceAddress, This->hc_data );
    UINT16      CurrentTimeout;
	UINT8		ToggleBit = (EndpointAddress & 0xF) - 1;
	UINT16		*wDataSync;

    if (DeviceSpeed != EFI_USB_SPEED_SUPER && 
		DeviceSpeed != EFI_USB_SPEED_HIGH && 
		DeviceSpeed != EFI_USB_SPEED_FULL && 
		DeviceSpeed != EFI_USB_SPEED_LOW) {
		return EFI_INVALID_PARAMETER;
    }

#if !EFI_USB_HC_INTERRUPT_OUT_SUPPORT
    if (!(EndpointAddress & BIT7)) {
		return EFI_INVALID_PARAMETER;
    }
#endif

    if (Data == NULL || DataLength == NULL ||
		*DataLength == 0 || TransferResult == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    if ( DeviceSpeed == EFI_USB_SPEED_LOW  && MaximumPacketLength != 8) {
        return EFI_INVALID_PARAMETER;
    }

    if (DeviceSpeed == EFI_USB_SPEED_FULL && 
		(MaximumPacketLength != 8 &&
		MaximumPacketLength != 16 &&
		MaximumPacketLength != 32 &&
		MaximumPacketLength != 64)) {
		return EFI_INVALID_PARAMETER;
    }

    if (DeviceSpeed == EFI_USB_SPEED_HIGH && MaximumPacketLength > 3072 ) {
        return EFI_INVALID_PARAMETER;
    }

	if (*DataToggle != 0 && *DataToggle != 1) return EFI_INVALID_PARAMETER;
	
    if(DevSrc == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    DevInfo = AllocDevInfo();
    DevInfo->bDeviceAddress = DeviceAddress;
    DevInfo->bHCNumber = This->hc_data->bHCNumber;
    DevInfo->bEndpointSpeed = SpeedMap[DeviceSpeed];
    DevInfo->wIntMaxPkt = (UINT16)MaximumPacketLength;
    DevInfo->bIntEndpoint = EndpointAddress;
    DevInfo->bPollInterval = TranslateInterval(DeviceSpeed, 1);
    DevInfo->bHubDeviceNumber = DevSrc->bHubDeviceNumber;
    DevInfo->bHubPortNumber = DevSrc->bHubPortNumber;
	DevInfo->DevMiscInfo = DevSrc->DevMiscInfo;	//(EIP84790+)

	wDataSync = EndpointAddress & 0x80 ? &DevInfo->wDataInSync : 
		&DevInfo->wDataOutSync;

	*wDataSync = (UINT16)(*DataToggle) << ToggleBit;

    CRITICAL_CODE( EFI_TPL_NOTIFY,
    {
        CurrentTimeout = gUsbData->wTimeOutValue;
        gUsbData->wTimeOutValue = (UINT16)Timeout;
        gUsbData->dLastCommandStatusExtended = 0;
        SmiRes = UsbSmiInterruptTransfer(
            This->hc_data,
            DevInfo,
            (UINT8*)Data,
            (UINT16)*DataLength);
        *TransferResult = GetTransferStatus();
        gUsbData->wTimeOutValue = CurrentTimeout;
    });

    *DataLength = (UINTN)SmiRes;
    *DataToggle = (UINT8)(*wDataSync >> ToggleBit) & 0x1;

    FreeDevInfo(DevInfo);
    //
    // Return with error or success
    //
    if ( (*TransferResult) &  EFI_USB_ERR_TIMEOUT ) return EFI_TIMEOUT;
    return (*TransferResult)? EFI_DEVICE_ERROR:EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcIsochronousTransfer
//
// Description:
//  This function performs USB2 HC Isochronous Transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
AmiUsb2HcIsochronousTransfer(
    IN     EFI_USB2_HC_PROTOCOL               *This,
    IN     UINT8                              DeviceAddress,
    IN     UINT8                              EndPointAddress,
    IN     UINT8                              DeviceSpeed,
    IN     UINTN                              MaximumPacketLength,
    IN     UINT8                              DataBuffersNumber,
    IN OUT VOID                               *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
    IN     UINTN                              DataLength,
    IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    OUT    UINT32                             *TransferResult
)
{
                                        //(EIP81612)>
    if ( Data == NULL || Data[0] == NULL || DataLength == 0 ||
        (DeviceSpeed != EFI_USB_SPEED_SUPER && DeviceSpeed != EFI_USB_SPEED_HIGH 
        && DeviceSpeed != EFI_USB_SPEED_FULL) ||
        MaximumPacketLength > 1023 ) {
        return EFI_INVALID_PARAMETER;
    }
                                        //<(EIP81612)
    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcAsyncIsochronousTransfer
//
// Description:
//  This function performs USB2 HC Async Isochronous Transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
AmiUsb2HcAsyncIsochronousTransfer(
    IN     EFI_USB2_HC_PROTOCOL               *This,
    IN     UINT8                              DeviceAddress,
    IN     UINT8                              EndPointAddress,
    IN     UINT8                              DeviceSpeed,
    IN     UINTN                              MaximumPacketLength,
    IN     UINT8                              DataBuffersNumber,
    IN OUT VOID                               *Data[EFI_USB_MAX_ISO_BUFFER_NUM],
    IN     UINTN                              DataLength,
    IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    IN     EFI_ASYNC_USB_TRANSFER_CALLBACK    IsochronousCallBack,
    IN     VOID                               *Context OPTIONAL
)
{
                                        //(EIP81612)>
    if( Data == NULL || Data[0] == NULL || DataLength == 0 ||
        (DeviceSpeed != EFI_USB_SPEED_SUPER && DeviceSpeed != EFI_USB_SPEED_HIGH 
        && DeviceSpeed != EFI_USB_SPEED_FULL) ||
        MaximumPacketLength > 1023 ) {
        return EFI_INVALID_PARAMETER;
    }
                                        //<EIP81612)
    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcTransfer
//
// Description:
//  This function performs USB2 HC Control Transfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcTransfer(
    IN EFI_USB2_HC_PROTOCOL           *HcProtocol,
    IN     UINT8                      DeviceAddress,
    IN     UINT8                      DeviceSpeed,
    IN     UINTN                      MaximumPacketLength,
    IN     EFI_USB_DEVICE_REQUEST     *Request,
    IN     EFI_USB_DATA_DIRECTION     TransferDirection,
    IN OUT VOID                       *Data ,
    IN OUT UINTN                      *DataLength,
    IN     UINTN                      Timeout,
    IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    OUT    UINT32                     *TransferResult
)
{
    UINT16      SmiRes;
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    DEV_INFO    *DevInfo;
    DEV_INFO    *DevSrc = DevAddr2Info( DeviceAddress, This->hc_data );
    UINT16      CurrentTimeout;
    EFI_STATUS  Status;

                                        //(EIP81612)>
    if ( (DeviceSpeed != EFI_USB_SPEED_SUPER && DeviceSpeed != EFI_USB_SPEED_HIGH
            && DeviceSpeed != EFI_USB_SPEED_FULL && DeviceSpeed != EFI_USB_SPEED_LOW)
            || DevSrc == NULL) {
        return EFI_INVALID_PARAMETER;
    }
                                        //<(EIP81612)
    if ( TransferDirection < EfiUsbDataIn || TransferDirection > EfiUsbNoData ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( TransferDirection == EfiUsbNoData && (Data != NULL || *DataLength != 0) ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( TransferDirection != EfiUsbNoData && (Data == NULL || *DataLength == 0) ) {
        return EFI_INVALID_PARAMETER;
    }

    if( Request == NULL || TransferResult == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( DeviceSpeed == EFI_USB_SPEED_LOW  && MaximumPacketLength != 8 ) {
        return EFI_INVALID_PARAMETER;
    }

    if ((DeviceSpeed == EFI_USB_SPEED_HIGH || DeviceSpeed == EFI_USB_SPEED_FULL) &&
            ( MaximumPacketLength != 8 &&
             MaximumPacketLength != 16 &&
             MaximumPacketLength != 32 &&
             MaximumPacketLength != 64 )
        ) {
            return EFI_INVALID_PARAMETER;
        }
                                        //(EIP81612+)>
    if ( DeviceSpeed == EFI_USB_SPEED_SUPER && MaximumPacketLength != 512 ) {
        return EFI_INVALID_PARAMETER;
    }
                                        //<(EIP81612+)

    DevInfo = AllocDevInfo();
    DevInfo->bDeviceAddress = DeviceAddress;
    DevInfo->bHCNumber = This->hc_data->bHCNumber;
    DevInfo->bEndpointSpeed = SpeedMap[DeviceSpeed];
    DevInfo->wEndp0MaxPacket = (UINT16)MaximumPacketLength;
    DevInfo->bHubDeviceNumber = DevSrc->bHubDeviceNumber;
    DevInfo->bHubDeviceNumber = DevSrc->bHubDeviceNumber;
    DevInfo->bHubPortNumber = DevSrc->bHubPortNumber;
    DevInfo->DevMiscInfo = DevSrc->DevMiscInfo;	//(EIP84790+)

    CRITICAL_CODE( EFI_TPL_NOTIFY,
    {
        CurrentTimeout = gUsbData->wTimeOutValue;
        gUsbData->wTimeOutValue = (UINT16)Timeout;
        gUsbData->dLastCommandStatusExtended = 0;
        SmiRes = UsbSmiControlTransfer(This->hc_data, DevInfo,
            (UINT16)((( TransferDirection == EfiUsbDataIn?1:0) << 7) |
            (((UINT16)Request->RequestType)) |
            (((UINT16)Request->Request)<<8)),
            (UINT16)Request->Index, (UINT16)Request->Value,
            (UINT8*)Data, (UINT16)Request->Length);
        *TransferResult = GetTransferStatus();
        gUsbData->wTimeOutValue = CurrentTimeout;
    });

    Status = EFI_SUCCESS;
/*
    if (TransferDirection == EfiUsbDataIn
        && *DataLength != 0
        && SmiRes == 0)
    {
        //
        // Some data is expected, no data is returned
        //
        Status = EFI_DEVICE_ERROR;
    }
*/
    if( (*TransferResult) &  EFI_USB_ERR_TIMEOUT ) Status = EFI_TIMEOUT;
	if( (*TransferResult) &  ~EFI_USB_ERR_TIMEOUT ) Status = EFI_DEVICE_ERROR;

    *DataLength = (UINTN)SmiRes;

    FreeDevInfo(DevInfo);

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcGetRootHubPortStatus
//
// Description:
//  This function returns HC root port status.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcGetRootHubPortStatus (
    EFI_USB2_HC_PROTOCOL *HcProtocol,
    UINT8               PortNumber,
    EFI_USB_PORT_STATUS *PortStatus
)
{
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    return AmiUsbHcGetRootHubPortStatus(
                &This->hcprotocol, PortNumber, PortStatus);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcSetRootHubPortFeature
//
// Description:
//  This function set root hub port features.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcSetRootHubPortFeature(
    IN EFI_USB2_HC_PROTOCOL *HcProtocol,
    IN UINT8                PortNumber,
    IN EFI_USB_PORT_FEATURE PortFeature
)
{
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    return AmiUsbHcClearRootHubPortFeature(
                &This->hcprotocol, PortNumber, PortFeature);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsb2HcClearRootHubPortFeature
//
// Description:
//  This function clears root hub port feature.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
AmiUsb2HcClearRootHubPortFeature(
    IN EFI_USB2_HC_PROTOCOL *HcProtocol,
    IN UINT8                PortNumber,
    IN EFI_USB_PORT_FEATURE PortFeature
)
{
    HC_DXE_RECORD *This = GetThis( HcProtocol );
    return AmiUsbHcClearRootHubPortFeature(
            &This->hcprotocol, PortNumber, PortFeature);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        FindHcStruc
//
// Description:
//  Search gUsbData for information about HC linked to an EFI handle
//
// Input:
//  Controller - Host Controller handle
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

HC_STRUC* FindHcStruc(
    EFI_HANDLE Controller
)
{
    unsigned i;
    for (i = 0; i < gUsbData->HcTableCount; i++) {
        if (gUsbData->HcTable[i] == NULL) continue;
        if (gUsbData->HcTable[i]->Controller == Controller )
            return gUsbData->HcTable[i];
    }
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbHcSupported
//
// Description:
//  This function is a part of Driver Binding Protocol interface.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS UsbHcSupported (
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE Controller,
    EFI_DEVICE_PATH_PROTOCOL *Dp
)
{
    EFI_STATUS  Status;
    VOID*       Ptr;

    Status = gBS->OpenProtocol (
            Controller,
            &gEfiPciIoProtocolGuid,
            &Ptr,
            This->DriverBindingHandle,
            Controller,
            EFI_OPEN_PROTOCOL_BY_DRIVER );
    if (EFI_ERROR (Status)) {
        return Status;
    }

    gBS->CloseProtocol (
        Controller, &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle, Controller);

    if (FindHcStruc(Controller) == NULL) {
        return EFI_UNSUPPORTED;
    } else
        return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbHcStop
//
// Description:
//  This function is a part of Driver Binding Protocol interface.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI
UsbHcStop (
   EFI_DRIVER_BINDING_PROTOCOL     *This,
   EFI_HANDLE                      Controller,
   UINTN                           NumberOfChildren,
   EFI_HANDLE                      *Children  )
{
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        InstallHcProtocols
//
// Description:
//  Start the AMI USB driver; Sets USB_FLAG_DRIVER_STARTED
//
// Input:
//  This                - Protocol instance pointer.
//  ControllerHandle    - Handle of device to test
//  RemainingDevicePath - Not used
//
// Output:
//  EFI_SUCCESS         - USB HC devices were initialized.
//  EFI_UNSUPPORTED     - pThis device is not supported by USB driver.
//  EFI_DEVICE_ERROR    - pThis driver cannot be started due to device error
//  EFI_OUT_OF_RESOURCES
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InstallHcProtocols(
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_PCI_IO_PROTOCOL   		*PciIo,
    IN HC_STRUC              		*HcData
)
{
    HC_DXE_RECORD         *Rec;

    //
    // Create HcDxeRecord
    //
    VERIFY_EFI_ERROR(
        gBS->AllocatePool (
        EfiBootServicesData,
        sizeof(HC_DXE_RECORD),
        &Rec ));
    Rec->hc_data = HcData;
    Rec->pciIo = PciIo;
    Rec->AsyncTransfers.pHead = NULL;
    Rec->AsyncTransfers.pTail = NULL;
    Rec->AsyncTransfers.Size = 0;

    Rec->hcprotocol.Reset = AmiUsbHcReset;
    Rec->hcprotocol.GetState = AmiUsbHcGetState;
    Rec->hcprotocol.SetState = AmiUsbHcSetState;
    Rec->hcprotocol.ControlTransfer = AmiUsbHcTransfer;
    Rec->hcprotocol.BulkTransfer = AmiUsbHcBulkTransfer;
    Rec->hcprotocol.AsyncInterruptTransfer = AmiUsbHcAsyncInterruptTransfer;
    Rec->hcprotocol.SyncInterruptTransfer = AmiUsbHcSyncInterruptTransfer;
    Rec->hcprotocol.IsochronousTransfer = AmiUsbHcIsochronousTransfer;
    Rec->hcprotocol.AsyncIsochronousTransfer = AmiUsbHcAsyncIsochronousTransfer;
    Rec->hcprotocol.GetRootHubPortNumber = AmiUsbHcGetRootHubPortNumber;
    Rec->hcprotocol.GetRootHubPortStatus = AmiUsbHcGetRootHubPortStatus;
    Rec->hcprotocol.SetRootHubPortFeature = AmiUsbHcSetRootHubPortFeature;
    Rec->hcprotocol.ClearRootHubPortFeature = AmiUsbHcClearRootHubPortFeature;

    //
    // Fill USB Revision fields based on type of HC
    //
    {
        //USB_HC_UHCI   USB_HC_OHCI -> 1.1
        //USB_HC_EHCI   -> 2.0
        //USB_HC_XHCI   -> 3.0
        UINT16 VersionMap[] = {    1, 1,	
                                   1, 1,
                                   2, 0,
                                   3, 0, };
        ASSERT((GET_HCD_INDEX(HcData->bHCType) * 2)+1 < (COUNTOF(VersionMap)));

        Rec->hcprotocol.MajorRevision =
            VersionMap[GET_HCD_INDEX(HcData->bHCType) * 2];
        Rec->hcprotocol.MinorRevision =
            VersionMap[(GET_HCD_INDEX(HcData->bHCType) * 2)+1];
    }


    Rec->hcprotocol2.GetCapability = AmiUsb2HcGetCapability;
    Rec->hcprotocol2.Reset = AmiUsb2HcReset;
    Rec->hcprotocol2.GetState = AmiUsb2HcGetState;
    Rec->hcprotocol2.SetState = AmiUsb2HcSetState;
    Rec->hcprotocol2.ControlTransfer = AmiUsb2HcTransfer;
    Rec->hcprotocol2.BulkTransfer = AmiUsb2HcBulkTransfer;
    Rec->hcprotocol2.AsyncInterruptTransfer = AmiUsb2HcAsyncInterruptTransfer;
    Rec->hcprotocol2.SyncInterruptTransfer = AmiUsb2HcSyncInterruptTransfer;
    Rec->hcprotocol2.IsochronousTransfer = AmiUsb2HcIsochronousTransfer;
    Rec->hcprotocol2.AsyncIsochronousTransfer = AmiUsb2HcAsyncIsochronousTransfer;
    Rec->hcprotocol2.GetRootHubPortStatus = AmiUsb2HcGetRootHubPortStatus;
    Rec->hcprotocol2.SetRootHubPortFeature = AmiUsb2HcSetRootHubPortFeature;
    Rec->hcprotocol2.ClearRootHubPortFeature = AmiUsb2HcClearRootHubPortFeature;
    Rec->hcprotocol2.MajorRevision =         Rec->hcprotocol.MajorRevision;
    Rec->hcprotocol2.MinorRevision =         Rec->hcprotocol.MinorRevision;


    //
    // Instal USB_HC_PROTOCOL
    //
    VERIFY_EFI_ERROR(
        gBS->InstallProtocolInterface( &Controller,
            &gEfiUsbHcProtocolGuid, EFI_NATIVE_INTERFACE, &Rec->hcprotocol ));
    VERIFY_EFI_ERROR(
        gBS->InstallProtocolInterface( &Controller,
            &gEfiUsb2HcProtocolGuid, EFI_NATIVE_INTERFACE, &Rec->hcprotocol2 ));

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        XhciAllocateScratchpadBuffers
//
// Description:
//  This function allocates XHCI scratchpad buffers. Data initialization will
//  be done later, in SMI XhciStart function.
//
// Notes:
//  Usb3Hc->DcbaaPtr points to the beginning of memory block first 2048 Bytes
//  of which is used for DCBAA.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
XhciAllocateScratchpadBuffers (
	IN HC_STRUC				*HcStruct,
    IN USB3_HOST_CONTROLLER *Usb3Hc
)
{
    UINT8   NumBufs = Usb3Hc->CapRegs->HcParams2.MaxScratchPadBufs;
    UINT8   Count;
    UINTN   ScratchpadBufArrayBase = (UINTN)Usb3Hc->DcbaaPtr + 0x800;
    VOID    *Buffer;
    UINTN   Base = ScratchpadBufArrayBase;

    for (Count = 0; Count < NumBufs; Count++) {

        // Allocate scratchpad buffer: PAGESIZE block located on
        // a PAGESIZE boundary. Section 4.20.
        Buffer = AllocateHcMemory ( HcStruct->PciIo,
                    Usb3Hc->PageSize4K, Usb3Hc->PageSize4K << 12);
        ASSERT(Buffer != NULL); // See if allocation is successful

        if (Buffer == NULL) return EFI_OUT_OF_RESOURCES;

        // Update *ScratchBufArrayBase
        *(UINTN*)Base = (UINTN)Buffer;
        Base += 8;
    }

    // Update scratchpad pointer only if # of scratch buffers >0
    if (NumBufs > 0) {
        *(UINTN*)Usb3Hc->DcbaaPtr = ScratchpadBufArrayBase;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        XhciInitMemory
//
// Description:
//  This function allocates XHCI memory buffers. Data initialization will be
//  done later, in SMI XhciStart function. These are the memory blocks:
//
//  1. DCBAAP + ScrPadBuf pointers + InpCtx + ERST <-- 8KB
//  2. CommandRing  <-- 1KB
//  3. EventRing    <-- 1KB
//  4. XferRings    <-- 1KB*MaxSlots, 1KB = 32EP per slot times 32 (padded size of TRB_RING)
//  4. N*sizeof(XHCI_DEVICE_CONTEXT) for device context segment <-- N KB or 2*N KB,
//      N is SlotNumber from CONFIG register
//  
//  5. TransferRings <-- MaxSlots*32*1KB
//
// Notes:
//  Scratchpad buffers are optional, they are allocated and initialized separately.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
XhciInitMemory (
	IN HC_STRUC				*HcStruct,
    IN USB3_HOST_CONTROLLER *Usb3Hc
)
{
    EFI_STATUS  Status;
    UINTN       MemSize;
    UINTN       DeviceContextSize;
    UINTN       XfrRingsSize;
    UINTN       XfrTrbsSize;

    XfrRingsSize = Usb3Hc->MaxSlots * 32 * 32;   // 32 endpoints per device, 32 padded size of TRB_RING
    XfrTrbsSize = RING_SIZE*Usb3Hc->MaxSlots*32;
    DeviceContextSize = (XHCI_DEVICE_CONTEXT_ENTRIES * Usb3Hc->ContextSize) * Usb3Hc->MaxSlots;

    MemSize = 0x2800 + XfrRingsSize + XfrTrbsSize + DeviceContextSize;

	Usb3Hc->DcbaaPtr = (XHCI_DCBAA*)AllocateHcMemory(HcStruct->PciIo, 
										EFI_SIZE_TO_PAGES(MemSize), 0x1000);
    gBS->SetMem(Usb3Hc->DcbaaPtr, MemSize, 0);  // Clear buffer

USB_DEBUG(3, "XHCI: Memory allocation - total %x Bytes:\n 0x2800+XfrRings(%x)+XfrTrbs(%x)+DevCtx(%x)\n",
        MemSize, XfrRingsSize, XfrTrbsSize, DeviceContextSize);

    // Assign DCBAA (Device Context Base Address); program the
    // DCBAA Pointer (DCBAAP) register (5.4.6) with a 64-bit address
    // pointing to where the Device Context Base Address Array is located.
    //
    // DCBAA: size 2048 Bytes, within PAGESIZE, 64 Bytes aligned.
    //
    // These requirements can be met by allocating 1 page using
    // pBS->AllocatePages; the address will be 4K aligned and will
    // not span PAGESIZE boundary.

    Status = XhciAllocateScratchpadBuffers(HcStruct, Usb3Hc);
    ASSERT_EFI_ERROR(Status);

    // Assign Input Context; the size of Input Context is either
    // 0x420 or 0x840 depending on HCPARAMS.Csz
    Usb3Hc->InputContext = (VOID*)((UINTN)Usb3Hc->DcbaaPtr + 0x940);

    // Initialize Transfer Rings pointer and store it in Usb3Hc; actual
    // xfer ring initialization happens later, when the EP is being enabled
    Usb3Hc->XfrRings = (TRB_RING*)((UINTN)Usb3Hc->DcbaaPtr + 0x2800);

    // 1024 = 32 bytes is padded sizeof(TRB_RING) times 32 EP per device
    Usb3Hc->XfrTrbs = (UINTN)Usb3Hc->XfrRings + XfrRingsSize;

    // Assign device context memory: Usb3Hc->MaxSlots devices,
    // 1024 (2048 if HCPARAMS.Csz is set) Bytes each
    Usb3Hc->DeviceContext = (VOID*)((UINTN)Usb3Hc->XfrTrbs + XfrTrbsSize);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        XhciExtCapParser
//
// Description:
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
XhciExtCapParser(
    IN USB3_HOST_CONTROLLER *Usb3Hc
)
{
	XHCI_EXT_CAP	*CurPtr;

	if (Usb3Hc->CapRegs->HcParams.Xecp == 0) return EFI_SUCCESS;

	// Starts from first capability
	CurPtr = (XHCI_EXT_CAP *)((UINTN)Usb3Hc->CapRegs + (Usb3Hc->CapRegs->HcParams.Xecp << 2));

    // Traverse all capability structures
	for(;;) {
		switch (CurPtr->CapId) {
			case XHCI_EXT_CAP_USB_LEGACY:
				Usb3Hc->ExtLegCap = (XHCI_EXT_LEG_CAP *)CurPtr;
				// Clear HC BIOS Owned Semaphore bit
				Usb3Hc->ExtLegCap->LegSup.HcBiosOwned = 0;
                // Clear HC OS Owned Semaphore bit
                Usb3Hc->ExtLegCap->LegSup.HcOsOwned = 0;
				// Clear SMI enable bit
				Usb3Hc->ExtLegCap->LegCtlSts.UsbSmiEnable = 0;
				Usb3Hc->ExtLegCap->LegCtlSts.UsbOwnershipChangeSmiEnable = 0;
				USB_DEBUG(3, "XHCI: USB Legacy Ext Cap Ptr %x\n", Usb3Hc->ExtLegCap);
				break;

			case XHCI_EXT_CAP_SUPPORTED_PROTOCOL:
				if (((XHCI_EXT_PROTOCOL*)CurPtr)->MajorRev == 0x02) {
					Usb3Hc->Usb2Protocol = (XHCI_EXT_PROTOCOL*)CurPtr;
					USB_DEBUG(3, "XHCI: USB2 Support Protocol %x, PortOffset %x PortCount %x\n", 
						Usb3Hc->Usb2Protocol, Usb3Hc->Usb2Protocol->PortOffset, Usb3Hc->Usb2Protocol->PortCount);
				} else if (((XHCI_EXT_PROTOCOL*)CurPtr)->MajorRev == 0x03) {
					Usb3Hc->Usb3Protocol = (XHCI_EXT_PROTOCOL*)CurPtr;
					USB_DEBUG(3, "XHCI: USB3 Support Protocol %x, PortOffset %x PortCount %x\n", 
						Usb3Hc->Usb3Protocol, Usb3Hc->Usb3Protocol->PortOffset, Usb3Hc->Usb3Protocol->PortCount);
				}
				break;

			case XHCI_EXT_CAP_POWERMANAGEMENT:
			case XHCI_EXT_CAP_IO_VIRTUALIZATION:
			case XHCI_EXT_CAP_USB_DEBUG_PORT:
				break;
		}
		if(CurPtr->NextCapPtr == 0) break;
	    // Point to next capability
	    CurPtr=(XHCI_EXT_CAP *)((UINTN)CurPtr+ (((UINTN)CurPtr->NextCapPtr) << 2));
	}

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        PreInitXhci
//
// Description:
//  This function initializes XHCI data structures, allocates HC memory and
//  updates the relevant fields in HcStruc. At this point the controller's
//  resources are assigned and accessible.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
PreInitXhci(
    EFI_HANDLE  Handle,
    HC_STRUC    *HcStruc
)
{
#if XHCI_SUPPORT
    UINT8   MaxSlots;
    EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo = HcStruc->PciIo;
    USB3_HOST_CONTROLLER    *Usb3Hc = NULL;

    Status = gBS->AllocatePool(EfiRuntimeServicesData, 
        sizeof(USB3_HOST_CONTROLLER), &Usb3Hc);
    if (EFI_ERROR(Status)) {
        return Status;
    }

	gBS->SetMem(Usb3Hc, sizeof(USB3_HOST_CONTROLLER), 0);

    Usb3Hc->Controller = Handle;

    // Get Capability Registers offset off the BAR
                                        //(EIP101226)>
    Status = PciIo->Pci.Read(PciIo, 
    			EfiPciIoWidthUint32, PCI_BAR0, 1, &Usb3Hc->CapRegs);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) {
		return Status;
    }
    if (((UINT8)(UINTN)Usb3Hc->CapRegs & (BIT1 |BIT2)) == BIT2) {
        Status = PciIo->Pci.Read(PciIo, 
            		EfiPciIoWidthUint32, PCI_BAR0, 
            		sizeof(VOID*)/sizeof(UINT32), &Usb3Hc->CapRegs);
    }

                                        //<(EIP101226)
    //clear all attributes before use
	Usb3Hc->CapRegs = (XHCI_HC_CAP_REGS*)((UINTN)Usb3Hc->CapRegs & ~(0x7F)); // Clear attributes

    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_VID, 1, &Usb3Hc->Vid);

    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_DID, 1, &Usb3Hc->Did);

    Usb3Hc->Access64 = Usb3Hc->CapRegs->HcParams.Ac64;

    Usb3Hc->HciVersion = Usb3Hc->CapRegs->HciVersion;
    Usb3Hc->MaxPorts = Usb3Hc->CapRegs->HcParams1.MaxPorts;

    Usb3Hc->OpRegs = (XHCI_HC_OP_REGS*)((UINTN)Usb3Hc->CapRegs + Usb3Hc->CapRegs->CapLength);
    Usb3Hc->PageSize4K = Usb3Hc->OpRegs->PageSize;
	Usb3Hc->ContextSize = 0x20 << Usb3Hc->CapRegs->HcParams.Csz;

    USB_DEBUG(3, "XHCI: Cap %x, OpRegs: %x Ver %x,\n      MaxPorts 0x%x, PageSize %x*4K\n",
        Usb3Hc->CapRegs, Usb3Hc->OpRegs, Usb3Hc->HciVersion, Usb3Hc->MaxPorts, Usb3Hc->PageSize4K);

    ASSERT(Usb3Hc->PageSize4K < 0x8000);   // Maximum possible page size is 128MB

    Status = PciIo->Pci.Read(PciIo, 
				EfiPciIoWidthUint8, XHCI_PCI_SBRN, 1, &Usb3Hc->SBRN);
    ASSERT_EFI_ERROR(Status);
    USB_DEBUG(3, "XHCI: Serial Bus Release Number is %x\n", Usb3Hc->SBRN);

    // OEM might change the default number of MaxSlots
    Status = Usb3OemGetMaxDeviceSlots(&MaxSlots);

    if (EFI_ERROR(Status)) {
        // Use default number of slots
        MaxSlots = Usb3Hc->CapRegs->HcParams1.MaxSlots;
    }
    else {
        // Validate the porting function output
        ASSERT(MaxSlots > 0 && MaxSlots <= Usb3Hc->CapRegs->HcParams1.MaxSlots);
    }
    Usb3Hc->MaxSlots = MaxSlots;

    // Get maximum number of interrupters
    Usb3Hc->MaxIntrs = Usb3Hc->CapRegs->HcParams1.MaxIntrs;

    Usb3Hc->DbOffset = Usb3Hc->CapRegs->DbOff;

    USB_DEBUG(3, "XHCI: MaxSlots %x, MaxIntrs %x, Doorbell Offset %x\n", Usb3Hc->MaxSlots, Usb3Hc->MaxIntrs, Usb3Hc->DbOffset);

	// Parse all capability structures
	XhciExtCapParser(Usb3Hc);

	if (gUsbData->UsbXhciSupport == 0) {
        gBS->FreePool(Usb3Hc);
		return EFI_UNSUPPORTED;
	}

    // Allocate and initialize memory blocks
    Status = XhciInitMemory(HcStruc, Usb3Hc);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) {
		gBS->FreePool(Usb3Hc);
		return Status;
    }

    HcStruc->usbbus_data = (VOID*)Usb3Hc;

    // Install xHC event handler
    Status = USBPort_InstallEventHandler(HcStruc);
    ASSERT_EFI_ERROR(Status);

#endif

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        PostStopXhci
//
// Description:
//  This function frees the HC memory and clears XHCI data structures.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
PostStopXhci(
    EFI_HANDLE  Handle,
    HC_STRUC    *HcStruc
)
{
#if XHCI_SUPPORT
	USB3_HOST_CONTROLLER *Usb3Hc = (USB3_HOST_CONTROLLER*)HcStruc->usbbus_data;
	UINT8	NumBufs = Usb3Hc->CapRegs->HcParams2.MaxScratchPadBufs;
	UINT8   Count;
	UINTN   ScratchpadBufArrayBase = (UINTN)Usb3Hc->DcbaaPtr + 0x800;
    UINTN   MemSize;
    UINTN   DeviceContextSize;
    UINTN   XfrRingsSize;
    UINTN   XfrTrbsSize;

	for (Count = 0; Count < NumBufs; Count++) {
		gBS->FreePages((EFI_PHYSICAL_ADDRESS)(*((UINTN*)ScratchpadBufArrayBase)), Usb3Hc->PageSize4K);
		ScratchpadBufArrayBase += 8;
	}

    XfrRingsSize = Usb3Hc->MaxSlots * 32 * 32;   // 32 endpoints per device, 32 padded size of TRB_RING
    XfrTrbsSize = RING_SIZE*Usb3Hc->MaxSlots*32;
    DeviceContextSize = (XHCI_DEVICE_CONTEXT_ENTRIES * Usb3Hc->ContextSize) * Usb3Hc->MaxSlots;

    MemSize = 0x2800 + XfrRingsSize + XfrTrbsSize + DeviceContextSize;

	gBS->FreePages((EFI_PHYSICAL_ADDRESS)Usb3Hc->DcbaaPtr, EFI_SIZE_TO_PAGES(MemSize));
	gBS->FreePool(Usb3Hc);

#endif
	return EFI_SUCCESS;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2010, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
