//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/usbbus.c 72    9/12/12 3:49a Wilsonlee $
//
// $Revision: 72 $
//
// $Date: 9/12/12 3:49a $
//
//****************************************************************************
//<AMI_FHDR_START>
//
//  Name:           UsbBus.c
//
//  Description:    USB Bus driver implementation
//
//<AMI_FHDR_END>
//****************************************************************************

#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"

#include "UsbDef.h"
#include "UsbBus.h"
#include "ComponentName.h"

//#pragma warning(disable: 4244)

extern EFI_GUID  gEfiUsb2HcProtocolGuid;

EFI_DRIVER_BINDING_PROTOCOL gUSBBusDriverBinding = {
    UsbBusSupported,
    UsbBusStart,
    UsbBusStop,
    USB_DRIVER_VERSION,
    NULL,
    NULL
};

extern USB_GLOBAL_DATA*     gUsbData;

TREENODE_T  UsbRootRoot = {0,};
TREENODE_T* gUsbRootRoot = &UsbRootRoot;
EFI_EVENT   gEvUsbEnumTimer=0;
int         gCounterUsbEnumTimer=0;
int         gBustreeLock = 0;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbIo2Dev
//
// Description: This function returns a pointer to USB device from UsbIo.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

USBDEV_T* UsbIo2Dev(EFI_USB_IO_PROTOCOL* UsbIo)
{
    return (USBDEV_T*)((char*)UsbIo - (UINTN)&((USBDEV_T*)0)->io );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        HcByIndex
//
// Description: Predicate for searching host controller node in the tree
//              by bHcNumber
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int HcByIndex(VOID* n, VOID* d)
{
    USBBUS_HC_T* HcNode = (USBBUS_HC_T*)n;

    return n && (HcNode->type == NodeHC)
         && (HcNode->hc_data->bHCNumber == *(UINT8*)d );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        HcByHandle
//
// Description: Predicate for searching host controller node in the tree
//              by EFI controller handle
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int HcByHandle(VOID* n, VOID* d)
{
    USBBUS_HC_T* HcNode = (USBBUS_HC_T*)n;
    return (HcNode->type == NodeHC) && (HcNode->hc_data->Controller == *(EFI_HANDLE*)d );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        DevByIndex
//
// Description: Predicate for searching device node in the tree
//              by index of the DEV_INFO structure in the aDevInfoTable
//              array of USB data
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int DevByIndex(VOID* n, VOID* d)
{
    USBDEV_T* Dev = (USBDEV_T*)n;
    return (Dev->type == NodeDevice) && (Dev->dev_info ==
        gUsbData->aDevInfoTable  + *(UINT8*)d );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        DevGrpByAddr
//
// Description: Predicate for searching device node in the tree
//              by USB address of the device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int DevGrpByAddr(VOID* n, VOID* d)
{
    USBDEV_T* Dev = (USBDEV_T*)n;
    return (Dev->type == NodeDevice || Dev->type ==  NodeGroup) &&
        (Dev->dev_info->bDeviceAddress == *(UINT8*)d );
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    DevGrpByPortIf
//
// Description: Predicate for searching device node in the tree
//              by parent hub port of the device, interface and LUN
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int DevGrpByPortIf(VOID* n, VOID* d)
{
    USBDEV_T* Dev = (USBDEV_T*)n;
    return (Dev->type == NodeDevice || Dev->type ==  NodeGroup) &&
        (Dev->dev_info->bHubPortNumber == ((DEV_INFO*)d)->bHubPortNumber ) &&
        (Dev->dev_info->bInterfaceNum == ((DEV_INFO*)d)->bInterfaceNum ) &&
        (Dev->dev_info->bLUN == ((DEV_INFO*)d)->bLUN );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    DevByInfo
//
// Description: Predicate for searching device node in the tree
//              by comparing pointers to the DEV_INFO structure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int DevByInfo(VOID* n, VOID* d )
{
    USBDEV_T* Dev = (USBDEV_T*)n;
    return (Dev->type == NodeDevice) && (Dev->dev_info == (DEV_INFO*)d );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    DevByAdrIf
//
// Description: Predicate for searching device node in the tree
//              by USB address and interface number of the device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int DevByAdrIf(VOID* n, VOID* d)
{
    USBDEV_T* Dev = (USBDEV_T*)n;

    return ((Dev->type == NodeDevice) &&
        (Dev->dev_info->bDeviceAddress == ((DEV_INFO*)d)->bDeviceAddress ) &&
        (Dev->dev_info->bInterfaceNum == ((DEV_INFO*)d)->bInterfaceNum )&&
        (Dev->dev_info->bLUN == ((DEV_INFO*)d)->bLUN));
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:         UsbDevGetGroup
//
// Description:  Retrieve DEVGROUP_T that is parent of
//               the specified USB device in the USB Bus tree
//
// Input:        Device for which the parent is requested
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEVGROUP_T* UsbDevGetGroup(USBDEV_T* Dev)
{
    return (DEVGROUP_T*)Dev->node.parent->data;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        DpAddUsbSegment
//
// Description: Builds a new path appending a USB segment
//
// Output:      Pointer to a callee allocated memory buffer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_DEVICE_PATH_PROTOCOL*
DpAddUsbSegment(
    EFI_DEVICE_PATH_PROTOCOL*   Dp,
    UINT8                       bHubPortNumber,
    UINT8                       bInterfaceNum
)
{
    USB_DEVICE_PATH DpNewSegment = {0,};
    DpNewSegment.Header.Type = MESSAGING_DEVICE_PATH;
    DpNewSegment.Header.SubType = MSG_USB_DP;
    SET_NODE_LENGTH(&DpNewSegment.Header, sizeof(DpNewSegment));

    DpNewSegment.InterfaceNumber = bInterfaceNum;
    DpNewSegment.ParentPortNumber = bHubPortNumber;
    return  EfiAppendDevicePathNode(Dp,(EFI_DEVICE_PATH_PROTOCOL*)&DpNewSegment);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        DpAddScsiSegment
//
// Description: Builds a new path appending a SCSI segment
//
//
// Output:      Pointer to a callee allocated memory buffer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_DEVICE_PATH_PROTOCOL*
DpAddScsiSegment(
    EFI_DEVICE_PATH_PROTOCOL* Dp,
    UINT8   Pun,
    UINT8   Lun
)
{
    SCSI_DEVICE_PATH DpNewSegment = {0,};

    DpNewSegment.Header.Type = MESSAGING_DEVICE_PATH;
    DpNewSegment.Header.SubType = MSG_SCSI_DP;
    SET_NODE_LENGTH(&DpNewSegment.Header, sizeof(DpNewSegment));
    DpNewSegment.Pun = Pun;
    DpNewSegment.Lun = Lun;
    return  EfiAppendDevicePathNode(Dp,(EFI_DEVICE_PATH_PROTOCOL*)&DpNewSegment);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    ReadUsbDescriptor
//
// Description: This function executes a get descriptor command to the
//              given USB device and endpoint
//
// Input:       dev         a pointer to USBDEV_T corresponding to the device
//              fpBuffer    Buffer to be used for the transfer
//              wLength     Size of the requested descriptor
//              bDescType   Requested descriptor type
//              bDescIndex  Descriptor index
//              wLangIndex  LangIndex
//
// Output:      Pointer to memory buffer containing the descriptor
//              NULL on error
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8*
ReadUsbDescriptor( USBDEV_T* Dev,
    UINT8*  Buffer,
    UINT16  Length,
    UINT8   DescType,
    UINT8   DescIndex,
    UINT16  LangIndex )
{
    HC_STRUC*   HcStruc = Dev->hc_info;
    DEV_INFO*   DevInfo = Dev->dev_info;
    UINT8       GetDescIteration;
    UINT16      Reg;
    UINT16      Status;

    for (GetDescIteration = 0; GetDescIteration < 3; GetDescIteration++) {
        Reg = (UINT16)((DescType << 8) + DescIndex);
        Status = UsbSmiControlTransfer(
                        HcStruc,
                        DevInfo,
                        (UINT16)USB_RQ_GET_DESCRIPTOR,
                        (UINT16)LangIndex,
                        Reg,
                        Buffer,
                        Length);
        if (Status) {
            return Buffer;
        }
        if (gUsbData->bLastCommandStatus & USB_TRNSFR_TIMEOUT) {
            break;
        }
        pBS->Stall(10 * 1000);
    }

    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetUsbDescriptor
//
// Description: Allocates memory necessary to hold complete descriptor
//              and returns the descriptor there
//
// Input:       dev         a pointer to USBDEV_T corresponding to the device
//              type        Requested descriptor type
//              index       Descriptor index
//              langindex   LangIndex
//
// Output:      Pointer to memory buffer containing the descriptor
//              NULL on error
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

USB_DESCRIPTOR_T*
GetUsbDescriptor(
    USBDEV_T*   Dev,
    UINT8       Type,
    UINT8       Index,
    UINT16      LangIndex)
{
    UINT8 Tmp[0x40] = {0};
    USB_DESCRIPTOR_T *Desc;
    UINT8* p = ReadUsbDescriptor(
                Dev, Tmp, sizeof(Tmp),
                Type, Index, LangIndex );

    if (p == NULL || ((USB_DESCRIPTOR_T*)Tmp)->DescriptorType != Type) return NULL;
    gBS->AllocatePool (EfiBootServicesData, ((USB_DESCRIPTOR_T*)Tmp)->Length, &Desc);

	if (((USB_DESCRIPTOR_T*)Tmp)->Length <= sizeof(Tmp)) {
		EfiCopyMem(Desc, Tmp, ((USB_DESCRIPTOR_T*)Tmp)->Length);
		return Desc;
	}

	EfiZeroMem(Desc, ((USB_DESCRIPTOR_T*)Tmp)->Length);
    p = ReadUsbDescriptor(Dev, (UINT8*)Desc,
        ((USB_DESCRIPTOR_T*)Tmp)->Length, Type, Index, LangIndex );
    //ASSERT(Desc->DescriptorType == Type);     //(EIP60640-)
    if( p == NULL ){
        gBS->FreePool(Desc);
        return NULL;
    }
    //
    //Decriptor Type cannot be 0, this case means that Get Descriptor cmd timed out
    //
    if (Desc->DescriptorType == 0) {
        return NULL;
    } else {
      return Desc;
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    FreeConfigDesc
//
// Description: Delocates memory that was used by the descriptor
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID FreeConfigDesc( VOID* Desc )
{
    if (Desc != 0) gBS->FreePool(Desc);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    NextDescriptor
//
// Description: Returns pointer to the next descriptor for the pack of
//              USB descriptors located in continues memory segment
//              - result of reading CONFIG_DESCRIPTOR
// Notes:
//              Uses TotalLength of the CONFIG_DESCRIPTOR and Length
//              field of each USB descriptor found inside the pack
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

int
NextDescriptor(
    EFI_USB_CONFIG_DESCRIPTOR* Desc,
    UINTN* Offset
)
{
    if( Desc == NULL || *Offset >= Desc->TotalLength ) return FALSE;
    if( ((EFI_USB_CONFIG_DESCRIPTOR*)((char*)Desc+*Offset))->Length == 0) return FALSE;
    *Offset += ((EFI_USB_CONFIG_DESCRIPTOR*)((char*)Desc+*Offset))->Length;
    if( *Offset >= Desc->TotalLength ) return FALSE;

    return TRUE;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    UsbDevConfigDesc
//
// Description: Returns a pointer to the memory containing CONFIG_DESCRIPTOR
//              reported by the USB device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_USB_CONFIG_DESCRIPTOR*
DevGroupConfigDesc( DEVGROUP_T* Grp ){
    return Grp->f_DevDesc && (Grp->active_config != -1)? 
        Grp->configs[Grp->active_config]:NULL;
}

EFI_USB_CONFIG_DESCRIPTOR*
UsbDevConfigDesc( USBDEV_T* Dev ){
    return DevGroupConfigDesc( UsbDevGetGroup(Dev));
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbDevLoadAllDescritors
//
// Description: Reads DEVICE and CONFIG descriptors for each
//              configuration available in the device. Marks
//              the index of the buffer containing CONFIG descriptor
//              for active configurations currently selected in
//              USB device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbDevLoadAllDescritors(
    DEVGROUP_T* Dev
)
{
    UINT8 i;
	EFI_STATUS Status;

    //ASSERT( Dev->f_DevDesc == 0 );
    Dev->configs = NULL;
    //
    // Device descriptor
    //
    if( UsbSmiGetDescriptor(Dev->hc_info, Dev->dev_info, (UINT8*)&Dev->dev_desc,
        sizeof(Dev->dev_desc), DESC_TYPE_DEVICE, 0) == 0 )
    {
        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USB Bus: loadAllDescritors: DEVICE desc not found\n");
        return ;
    }
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USB Bus: dev descr: ");
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "cls:%x;subcls:%x;proto:%x;vndr:%x;id:%x\n",
                Dev->dev_desc.DeviceClass,
                Dev->dev_desc.DeviceSubClass,
                Dev->dev_desc.DeviceProtocol,
                Dev->dev_desc.IdVendor,
                Dev->dev_desc.IdProduct);
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "\t\tstr1:%x,str2:%x,str3:%x\n",
                Dev->dev_desc.StrManufacturer,
                Dev->dev_desc.StrProduct,
                Dev->dev_desc.StrSerialNumber );

    Dev->f_DevDesc = TRUE;

    //
    // Config descriptor
    //
    Dev->config_count = Dev->dev_desc.NumConfigurations;
//  dev->configs = (EFI_USB_CONFIG_DESCRIPTOR**)MallocZ(dev->config_count*sizeof(EFI_USB_CONFIG_DESCRIPTOR*));
    Status = gBS->AllocatePool (EfiBootServicesData,
        Dev->config_count*sizeof(EFI_USB_CONFIG_DESCRIPTOR*), (VOID *)&Dev->configs);
	ASSERT_EFI_ERROR(Status);
    EfiZeroMem(Dev->configs, Dev->config_count*sizeof(EFI_USB_CONFIG_DESCRIPTOR*));

    Dev->active_config = -1;
    for(i=0; i<Dev->config_count; ++i){
        //read each configuration
        //first failed read will terminate loop

        //TODO: optimization: allloc&read MAX size first
        //      and read second time only if total length is greater
        //Read 1 : get total length
        EFI_USB_CONFIG_DESCRIPTOR tmp = {0,0,};
        UINT8* p = ReadUsbDescriptor((USBDEV_T*)Dev,(UINT8*)&tmp,
            sizeof(tmp),DESC_TYPE_CONFIG,i, 0 );
        //ASSERT(tmp.DescriptorType == DESC_TYPE_CONFIG);
        //ASSERT(tmp.TotalLength >= sizeof(tmp));
        //
        //Addressing timeouts caused by device errors - empty DESC strucuture will be returned
        //
        if( (p == NULL) || (tmp.DescriptorType == 0) ) {
          break;
        }

        //Read 2: Actual content
//      dev->configs[i] = MallocZ(tmp.TotalLength);
        Status = gBS->AllocatePool (EfiBootServicesData, tmp.TotalLength, &Dev->configs[i]);
		ASSERT_EFI_ERROR(Status);
        EfiZeroMem(Dev->configs[i], tmp.TotalLength);

        p = ReadUsbDescriptor((USBDEV_T*)Dev, (UINT8*)Dev->configs[i],
            tmp.TotalLength, DESC_TYPE_CONFIG, i, 0);
        //ASSERT(Dev->configs[i]->DescriptorType == DESC_TYPE_CONFIG);  //(EIP60640-)
        //
        //Addressing timeouts caused by device errors - empty DESC strucuture will be returned
        //
        if( (p == NULL) || (Dev->configs[i]->DescriptorType == 0) ){
            gBS->FreePool(Dev->configs[i]);
            Dev->configs[i] = 0;
            break;
        }
        //config Desc is here

        //Active Config
        if( Dev->configs[i]->ConfigurationValue == Dev->dev_info->bConfigNum ){
            Dev->active_config = i;
        }
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbDevLoadEndpoints
//
// Description: Locates information about each endpoint inside the
//              descriptors pack loaded with CONFIG descriptor
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbDevLoadEndpoints(
    USBDEV_T* Dev
)
{
    EFI_USB_INTERFACE_DESCRIPTOR  *TmpDesc;
    EFI_USB_ENDPOINT_DESCRIPTOR  *EpDesc;
    EFI_USB_CONFIG_DESCRIPTOR  *CfgDesc;
    int j;
    UINTN   Offset;
    DEVGROUP_T *Grp = UsbDevGetGroup(Dev);

    Dev->first_endpoint = Grp->endpoint_count;
    Dev->end_endpoint = Grp->endpoint_count;

    CfgDesc = UsbDevConfigDesc(Dev);
    USB_DEBUG(DEBUG_LEVEL_USBBUS,
        "\tSanning config desc: ");
    USB_DEBUG(DEBUG_LEVEL_USBBUS,
        "%x(type:%x;len:%x;val:%x;total:%x)\n",
        CfgDesc,CfgDesc->DescriptorType,CfgDesc->Length,
        CfgDesc->ConfigurationValue,CfgDesc->TotalLength );

    //
    // Search intrface descriptor
    //
    for(Offset = 0; NextDescriptor(CfgDesc,&Offset);)
    {
        TmpDesc = (EFI_USB_INTERFACE_DESCRIPTOR *)((char*)CfgDesc+Offset);
        USB_DEBUG(DEBUG_LEVEL_USBBUS,
            "\t\tdesc: %x(type:%x;len:%x;if:%x;aif:%x)\n",
            TmpDesc,TmpDesc->DescriptorType,TmpDesc->Length,
            TmpDesc->InterfaceNumber, TmpDesc->AlternateSetting );
        if( TmpDesc->DescriptorType == DESC_TYPE_INTERFACE &&
            TmpDesc->InterfaceNumber== Dev->dev_info->bInterfaceNum &&
            TmpDesc->AlternateSetting == Dev->dev_info->bAltSettingNum)
        {
            //found
            USB_DEBUG(DEBUG_LEVEL_USBBUS,
                "\t...IF found.\n" );

            Dev->descIF= TmpDesc;

            ASSERT(TmpDesc->NumEndpoints < COUNTOF(Grp->endpoints));
            for(j=0;j<TmpDesc->NumEndpoints && NextDescriptor(CfgDesc,&Offset);){
                EpDesc = (EFI_USB_ENDPOINT_DESCRIPTOR*)((char*)CfgDesc+Offset);
                if( EpDesc->DescriptorType == DESC_TYPE_ENDPOINT ){
                    USB_DEBUG(DEBUG_LEVEL_USBBUS,
                        "\t\tend-point desc: %x", EpDesc);
                    USB_DEBUG(DEBUG_LEVEL_USBBUS,
                        "(index:%x;type:%x;len:%x;addr:%x;pcksz:%x;attr:%x,t:%x)\n",
                        Grp->endpoint_count,
                        EpDesc->DescriptorType,EpDesc->Length,
                        EpDesc->EndpointAddress, EpDesc->MaxPacketSize,
                        EpDesc->Attributes, EpDesc->Interval);
                    Grp->endpoints[Grp->endpoint_count].address = EpDesc->EndpointAddress;
                    Grp->endpoints[Grp->endpoint_count++].desc = EpDesc;
                    Grp->a2endpoint[COMPRESS_EP_ADR(EpDesc->EndpointAddress)] = EpDesc;
                    j++;
                }
                if( EpDesc->DescriptorType == DESC_TYPE_INTERFACE ){
                    //Oops, We stepped into another interface
                    break;
                }
            }
            Dev->end_endpoint = Grp->endpoint_count;
            USB_DEBUG(DEBUG_LEVEL_USBBUS,
                    "\t\tinterface end-points: ");
            USB_DEBUG(DEBUG_LEVEL_USBBUS,
                    "first:%x; end:%x, grp.endpoint_count:%x\n",
                        Dev->first_endpoint, Dev->end_endpoint, Grp->endpoint_count);
            return;
        }
    }
    USB_DEBUG(DEBUG_LEVEL_USBBUS,
        "\t...IF not found.\n" );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetMaxPacket
//
// Description: Retrieves information about a max packet size
//              for the specified endpoint of the device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
GetMaxPacket(
    UINT8       Endpoint,
    USBDEV_T*   Dev
)
{
    DEVGROUP_T *Grp = UsbDevGetGroup(Dev);
    EFI_USB_ENDPOINT_DESCRIPTOR* Desc = Grp->a2endpoint[COMPRESS_EP_ADR(Endpoint)];
    if (Desc == 0) return 0;
    return Desc->MaxPacketSize;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetEndpointDesc
//
// Description: Retrieves information about a max packet size
//              for the specified endpoint of the device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_USB_ENDPOINT_DESCRIPTOR*
GetEndpointDesc(
    UINT8       Endpoint,
    USBDEV_T*   Dev
)
{
    DEVGROUP_T *Grp = UsbDevGetGroup(Dev);
    EFI_USB_ENDPOINT_DESCRIPTOR* Desc;

    if (((Endpoint & 0x7F)==0) || ((Endpoint & 0x7F) > 0xF))
        return NULL;
    Desc = Grp->a2endpoint[COMPRESS_EP_ADR(Endpoint)];

    return Desc;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
//  Name:  UsbIoControlTransfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoControlTransfer(
    IN EFI_USB_IO_PROTOCOL      *UsbIo,
    IN EFI_USB_DEVICE_REQUEST   *Request,
    IN EFI_USB_DATA_DIRECTION   Direction,
    IN UINT32                   Timeout,
    IN OUT VOID                 *Data,
    IN UINTN                    DataLength,
    OUT UINT32                  *UsbStatus
)
{
    USBDEV_T*   Dev = UsbIo2Dev(UsbIo);
    EFI_STATUS  Status;

    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );

    if( Request == NULL || UsbStatus == NULL ) return EFI_INVALID_PARAMETER;
    if (Direction > EfiUsbNoData) return EFI_INVALID_PARAMETER;

    Status = Dev->hc->ControlTransfer(
        Dev->hc, Dev->dev_info->bDeviceAddress, GetSpeed(Dev),
        (UINTN)Dev->dev_info->wEndp0MaxPacket,				//(EIP81612)
        Request, Direction, Data, &DataLength, Timeout, NULL, UsbStatus);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    if ((Request->Request == (USB_RQ_SET_INTERFACE >> 8)) &&
         (Request->RequestType == (USB_RQ_SET_INTERFACE & 0x0F) ) &&
         (Request->Index == Dev->dev_info->bInterfaceNum)) {
        Dev->dev_info->bAltSettingNum = (UINT8)Request->Value;
        UsbDevLoadEndpoints(Dev);
    }
    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    UsbIoBulkTransfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoBulkTransfer(
  IN EFI_USB_IO_PROTOCOL    *UsbIo,
  IN UINT8                  Endpoint,
  IN OUT VOID               *Data,
  IN OUT UINTN              *DataLength,
  IN UINTN                  Timeout,
  OUT UINT32                *UsbStatus
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);
    UINT16 MaxPacket;
	UINT8		ToggleBit = (Endpoint & 0xF) - 1;
										//(EIP84215)>
	UINT16		*wDataSync;
    UINT8       Toggle;
    EFI_STATUS  Status;
    EFI_USB_ENDPOINT_DESCRIPTOR* EpDesc = GetEndpointDesc(Endpoint, Dev);
    DEV_INFO *DevInfoToDataSync;
										//<(EIP84215)
    if (((Endpoint & 0x7F)==0) || ((Endpoint & 0x7F) > 0xF))
        return EFI_INVALID_PARAMETER;

//	    if ( Dev->dev_info->bBulkInEndpoint &&
//	        ((Endpoint & 0x80) && ((Endpoint & 0x7F) != Dev->dev_info->bBulkInEndpoint)) ) {
//	        return EFI_INVALID_PARAMETER;
//	    }

//	    if ( Dev->dev_info->bBulkOutEndpoint &&
//	        (!(Endpoint & 0x80) && (Endpoint != Dev->dev_info->bBulkOutEndpoint)) ) {
//	        return EFI_INVALID_PARAMETER;
//	    }

    if ( Data == NULL || DataLength == NULL || UsbStatus == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    if( EpDesc == NULL ) {
        return EFI_INVALID_PARAMETER;
    }
    MaxPacket = EpDesc->MaxPacketSize;
    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );

    if( UsbStatus == NULL || MaxPacket == 0) {
        return EFI_INVALID_PARAMETER;
    }

    if ( (EpDesc->Attributes & EP_DESC_FLAG_TYPE_BITS) != EP_DESC_FLAG_TYPE_BULK ) {
        return EFI_INVALID_PARAMETER;
    }
										//(EIP84215+)>
    if (Dev->dev_info->fpLUN0DevInfoPtr) {
        DevInfoToDataSync = Dev->dev_info->fpLUN0DevInfoPtr;
    }else {
        DevInfoToDataSync = Dev->dev_info;
    }

    if (Endpoint & 0x80) {
        wDataSync = &DevInfoToDataSync->wDataInSync;
    }else {
        wDataSync = &DevInfoToDataSync->wDataOutSync;
    }
										//<(EIP84215+)
    GETBIT( *wDataSync, Toggle, ToggleBit );

    Status = Dev->hc->BulkTransfer(
        Dev->hc, Dev->dev_info->bDeviceAddress, Endpoint, GetSpeed(Dev),
        MaxPacket, 1, &Data, DataLength, &Toggle, Timeout, NULL, UsbStatus);

    SETBIT( *wDataSync, Toggle, ToggleBit );

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoIsochronousTransfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoAsyncInterruptTransfer(
    IN EFI_USB_IO_PROTOCOL  *UsbIo,
    IN UINT8                Endpoint,
    IN BOOLEAN              IsNewTransfer,
    IN UINTN                PollingInterval,
    IN UINTN                DataLength,
    IN EFI_ASYNC_USB_TRANSFER_CALLBACK InterruptCallback,
    IN VOID                 *Context
)
{
    USBDEV_T*   Dev = UsbIo2Dev(UsbIo);
    UINT8       Toggle;
	UINT8		ToggleBit = (Endpoint & 0xF) - 1;
										//(EIP84215)>
	UINT16		*wDataSync;
    EFI_STATUS  Status;
    EFI_USB_ENDPOINT_DESCRIPTOR* EpDesc = GetEndpointDesc(Endpoint, Dev);
    DEV_INFO *DevInfoToDataSync;
										//<(EIP84215)
	ASSERT( Dev->dev_info );
	ASSERT( Dev->hc );

	// Check whether Endpoint is valid
    if(EpDesc == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    if ((EpDesc->Attributes & EP_DESC_FLAG_TYPE_BITS) != EP_DESC_FLAG_TYPE_INT ) {
        return EFI_INVALID_PARAMETER;
    }

	if (IsNewTransfer && (PollingInterval < 1 || PollingInterval > 255)) {
		return EFI_INVALID_PARAMETER;
	}
										//(EIP84215+)>
    if (Dev->dev_info->fpLUN0DevInfoPtr) {
        DevInfoToDataSync = Dev->dev_info->fpLUN0DevInfoPtr;
    }else {
        DevInfoToDataSync = Dev->dev_info;
    }

    if (Endpoint & 0x80) {
        wDataSync = &DevInfoToDataSync->wDataInSync;
    }else {
        wDataSync = &DevInfoToDataSync->wDataOutSync;
    }
										//<(EIP84215+)
    GETBIT( *wDataSync, Toggle, ToggleBit );

    Status = Dev->hc->AsyncInterruptTransfer(
        Dev->hc, Dev->dev_info->bDeviceAddress, Endpoint,
        GetSpeed(Dev), EpDesc->MaxPacketSize, IsNewTransfer,
        &Toggle, PollingInterval, DataLength, NULL,
        InterruptCallback, Context );

    SETBIT( *wDataSync, Toggle, ToggleBit );

	if (!EFI_ERROR(Status)) {
		Dev->async_endpoint = IsNewTransfer ? Endpoint : 0;
	}
    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoSyncInterruptTransfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoSyncInterruptTransfer(
    IN     EFI_USB_IO_PROTOCOL  *UsbIo,
    IN     UINT8                Endpoint,
    IN OUT VOID                 *Data,
    IN OUT UINTN                *DataLength,
    IN     UINTN                Timeout,
    OUT    UINT32               *UsbStatus
)
{
    USBDEV_T*   Dev = UsbIo2Dev(UsbIo);
    UINT8       Toggle;
	UINT8		ToggleBit = (Endpoint & 0xF) - 1;
										//(EIP84215)>
	UINT16		*wDataSync;
    EFI_STATUS  Status;
    EFI_USB_ENDPOINT_DESCRIPTOR* EpDesc = GetEndpointDesc(Endpoint, Dev);
    DEV_INFO *DevInfoToDataSync;
										//<(EIP84215)
	ASSERT( Dev->dev_info );
	ASSERT( Dev->hc );

	// Check whether Endpoint is valid
	if( EpDesc == NULL ) {
		return EFI_INVALID_PARAMETER;
	}

	if ( (EpDesc->Attributes & EP_DESC_FLAG_TYPE_BITS) != EP_DESC_FLAG_TYPE_INT ) {
		 return EFI_INVALID_PARAMETER;
	}

    if ( Data == NULL || DataLength == NULL || UsbStatus == NULL ) {
        return EFI_INVALID_PARAMETER;
    }
										//(EIP84215+)>
    if (Dev->dev_info->fpLUN0DevInfoPtr) {
        DevInfoToDataSync = Dev->dev_info->fpLUN0DevInfoPtr;
    }else {
        DevInfoToDataSync = Dev->dev_info;
    }

    if (Endpoint & 0x80) {
        wDataSync = &DevInfoToDataSync->wDataInSync;
    }else {
        wDataSync = &DevInfoToDataSync->wDataOutSync;
    }
										//<(EIP84215+)
    GETBIT( *wDataSync, Toggle, ToggleBit );
    Status = Dev->hc->SyncInterruptTransfer(
        Dev->hc, Dev->dev_info->bDeviceAddress, Endpoint,
        GetSpeed(Dev), EpDesc->MaxPacketSize,
        Data, DataLength, &Toggle, Timeout, NULL, UsbStatus);
    SETBIT( *wDataSync, Toggle, ToggleBit );

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
//  Name:   UsbIoIsochronousTransfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoIsochronousTransfer(
    IN EFI_USB_IO_PROTOCOL  *UsbIo,
    IN     UINT8            Endpoint,
    IN OUT VOID             *Data,
    IN     UINTN            DataLength,
    OUT    UINT32           *Status
)
{
    USBDEV_T*   Dev = UsbIo2Dev(UsbIo);
    UINT16      MaxPacket;// = GetMaxPacket(Endpoint,dev);
    EFI_USB_ENDPOINT_DESCRIPTOR* EpDesc = GetEndpointDesc(Endpoint, Dev);

    if( EpDesc == NULL ) return EFI_INVALID_PARAMETER;

    MaxPacket = EpDesc->MaxPacketSize;
    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );

    if( Status == NULL || MaxPacket == 0 ) return EFI_INVALID_PARAMETER;

    if ( (EpDesc->Attributes & EP_DESC_FLAG_TYPE_BITS) != EP_DESC_FLAG_TYPE_ISOC )
        return EFI_INVALID_PARAMETER;

    return Dev->hc->IsochronousTransfer(
        Dev->hc, Dev->dev_info->bDeviceAddress, Endpoint, GetSpeed(Dev),
        MaxPacket, 1, &Data, DataLength, NULL, Status);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoAsyncIsochronousTransfer
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoAsyncIsochronousTransfer(
    IN EFI_USB_IO_PROTOCOL  *UsbIo,
    IN UINT8                Endpoint,
    IN OUT VOID             *Data,
    IN     UINTN            DataLength,
    IN EFI_ASYNC_USB_TRANSFER_CALLBACK  IsochronousCallback,
    IN VOID                 *Context
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);
    UINT16 MaxPacket;// = GetMaxPacket(Endpoint, Dev);
    EFI_USB_ENDPOINT_DESCRIPTOR* EpDesc = GetEndpointDesc(Endpoint, Dev);
    if( EpDesc == NULL ) return EFI_INVALID_PARAMETER;
    MaxPacket = EpDesc->MaxPacketSize;
    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );
    if ( (EpDesc->Attributes & EP_DESC_FLAG_TYPE_BITS) != EP_DESC_FLAG_TYPE_ISOC )
        return EFI_INVALID_PARAMETER;

    return Dev->hc->AsyncIsochronousTransfer(
        Dev->hc, Dev->dev_info->bDeviceAddress, Endpoint, GetSpeed(Dev),
        MaxPacket, 1, &Data, DataLength, NULL, IsochronousCallback, Context );
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoPortReset
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoPortReset(
    IN EFI_USB_IO_PROTOCOL  *UsbIo
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);
	DEVGROUP_T* Grp = UsbDevGetGroup(Dev);
	UINT8	Status;
	UINT8	i;

    if (Dev->dev_info->bDeviceType == BIOS_DEV_TYPE_HUB)
        return EFI_INVALID_PARAMETER;

	Status = UsbResetAndReconfigDev(Dev->hc_info, Dev->dev_info);
	if (Status != USB_SUCCESS) {
		return EFI_DEVICE_ERROR;
	}

	if( UsbSmiGetDescriptor(Dev->hc_info, Dev->dev_info, (UINT8*)&Grp->dev_desc,
        sizeof(Grp->dev_desc), DESC_TYPE_DEVICE, 0) == 0 ){
        return EFI_DEVICE_ERROR;
    }

	for(i = 0; i < Grp->dev_desc.NumConfigurations; i++) {
		if (ReadUsbDescriptor(Dev, (UINT8*)Grp->configs[i], 
			Grp->configs[i]->TotalLength, DESC_TYPE_CONFIG, i, 0) == 0) {
			return EFI_DEVICE_ERROR;
		}
	}

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbIoGetDeviceDescriptor
//
// Description: Retrieves the USB Device Descriptor.
//
// Input:
//      UsbIo       A pointer to the EFI_USB_IO_PROTOCOL instance. Type
//                  EFI_USB_IO_PROTOCOL is defined in Section 14.2.5.
//      Desc        A pointer to the caller allocated USB Device Descriptor.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoGetDeviceDescriptor(
    IN EFI_USB_IO_PROTOCOL          *UsbIo,
    OUT EFI_USB_DEVICE_DESCRIPTOR   *Desc
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);
    DEVGROUP_T* Grp = UsbDevGetGroup(Dev);
    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );
    if( Desc == NULL ) return EFI_INVALID_PARAMETER;

    if( Grp->f_DevDesc ){
        *Desc = Grp->dev_desc;
        return EFI_SUCCESS;
    } else
        return EFI_NOT_FOUND;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoGetConfigDescriptor
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoGetConfigDescriptor(
    IN EFI_USB_IO_PROTOCOL          *UsbIo,
    OUT EFI_USB_CONFIG_DESCRIPTOR   *Desc
)
{
    USBDEV_T*   Dev = UsbIo2Dev(UsbIo);
    DEVGROUP_T* Grp = UsbDevGetGroup(Dev);
    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );
    if( Desc == NULL ) return EFI_INVALID_PARAMETER;

    if( Grp->configs && Grp->active_config != -1 &&
        Grp->configs[Grp->active_config] ){
        *Desc = *Grp->configs[Grp->active_config];
        return EFI_SUCCESS;
    } else
        return EFI_NOT_FOUND;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoGetInterfaceDescriptor
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoGetInterfaceDescriptor(
    IN EFI_USB_IO_PROTOCOL              *UsbIo,
    OUT EFI_USB_INTERFACE_DESCRIPTOR    *Desc
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);

    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );
    if (Desc == NULL) return EFI_INVALID_PARAMETER;

    if (Dev->descIF == NULL) return EFI_NOT_FOUND;

    *Desc = *Dev->descIF;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoGetEndpointDescriptor
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoGetEndpointDescriptor(
    IN EFI_USB_IO_PROTOCOL  *UsbIo,
    IN  UINT8               EndpointIndex,
    OUT EFI_USB_ENDPOINT_DESCRIPTOR *Desc
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);
    DEVGROUP_T *Grp;
    EFI_USB_ENDPOINT_DESCRIPTOR* DescCopy;

    if( Desc == NULL || EndpointIndex >= 0x10) return EFI_INVALID_PARAMETER;

    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );

    USB_DEBUG( 3, "Get Endpoint desc: devaddr: 0x%x; Endpoint: 0x%x\n",
            Dev->dev_info->bDeviceAddress, EndpointIndex );
    USB_DEBUG( 3, "\tfirst Endpoint: 0x%x; last Endpoint: 0x%x\n",
            Dev->first_endpoint, Dev->end_endpoint-1 );

    if( Dev->first_endpoint + EndpointIndex >= Dev->end_endpoint )
        return EFI_NOT_FOUND;

    ASSERT( Dev->first_endpoint + EndpointIndex < 0x20 );

    Grp = UsbDevGetGroup(Dev);
    DescCopy = Grp->endpoints[Dev->first_endpoint + EndpointIndex].desc;

    ASSERT( DescCopy );

    if (DescCopy==NULL) return EFI_NOT_FOUND;

    USB_DEBUG( 3, "\tendp addr: 0x%x; attr: 0x%x; MaxPacket: 0x%x\n",
            DescCopy->EndpointAddress,
            DescCopy->Attributes, DescCopy->MaxPacketSize );

    *Desc = *DescCopy;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoGetStringDescriptor
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
UsbIoGetStringDescriptor(
    IN EFI_USB_IO_PROTOCOL  *UsbIo,
    IN  UINT16              LangId,
    IN  UINT8               StringId,
    OUT CHAR16              **String
)
{
    USBDEV_T* Dev = UsbIo2Dev(UsbIo);
    EFI_USB_STRING_DESCRIPTOR* StrDesc;
    UINT16 i;
    UINT16 *LangID;
    DEVGROUP_T* Grp;
    EFI_STATUS Status = EFI_SUCCESS;

    ASSERT(Dev->dev_info);
    ASSERT(Dev->hc);

    Grp = UsbDevGetGroup(Dev);

    if (StringId == 0 || Grp->lang_table == NULL){
        //reserved ids: stringid==0 => langid table descripto;
        return EFI_NOT_FOUND;
    }
    // search langid_table
    for (i = 0, LangID = Grp->lang_table->langID;
        i < Grp->lang_table->len &&
        LangID[i] == LangId; ++i) {
    }
    i--;
    if (LangID[i] != LangId) {
        return EFI_NOT_FOUND;
    }

    //
    // Get string descriptor: variable size
    //
	StrDesc = (EFI_USB_STRING_DESCRIPTOR*)GetUsbDescriptor(Dev, DESC_TYPE_STRING,
			StringId, LangID[i] ); 		//(EIP27593)

    Status = EFI_NOT_FOUND;         //(EIP27593)
    if (StrDesc == 0) {
        return Status;   //(EIP27593)
    }

    if (StrDesc->Length > 2 && StrDesc->DescriptorType == DESC_TYPE_STRING){
        //
        // Allocate memory for string & copy
        //
        if (String != NULL) {
            gBS->AllocatePool(EfiBootServicesData, StrDesc->Length, String);
            EfiZeroMem(*String, StrDesc->Length);
            EfiCopyMem(*String, StrDesc->String, StrDesc->Length -2);
        }
        Status = EFI_SUCCESS;
    }
    gBS->FreePool(StrDesc);
    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  UsbIoGetSupportedLanguages
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbIoGetSupportedLanguages(
    IN EFI_USB_IO_PROTOCOL  *UsbIo,
    OUT UINT16              **LangIdTable,
    OUT UINT16              *TableSize )
{
    USBDEV_T*   Dev = UsbIo2Dev(UsbIo);
    DEVGROUP_T* Grp;

    ASSERT( Dev->dev_info );
    ASSERT( Dev->hc );
    Grp = UsbDevGetGroup(Dev);
    if (LangIdTable == NULL || TableSize == NULL) {
        return EFI_SUCCESS;
    }
    if (Grp->lang_table == NULL) {
        *LangIdTable  = 0;
        *TableSize = 0;
    } else {
        *LangIdTable = Grp->lang_table->langID;
        *TableSize = Grp->lang_table->len*2;
    }
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        LoadName
//
// Description: loads STRING descriptor that corresponds to
//              the name of the USB device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

CHAR16*
LoadName(
    USBDEV_T* Dev
)
{
    EFI_USB_INTERFACE_DESCRIPTOR DescIf = {0,};
    EFI_USB_DEVICE_DESCRIPTOR DescDev = {0,};
    DEVGROUP_T* Grp = UsbDevGetGroup(Dev);
    CHAR16* StrIf = 0;
    CHAR16* StrProduct = 0;
//  CHAR16* StrManufact=0;
    CHAR16 Lang;
    CHAR16  *MassStorageName;
    UINT8   *p;
    UINT8   i;

    for (i = 0; i < 64; i++) {
        if (Dev->dev_info->DevNameString[i] != 0) {
            break;
        }
    }

    if (i != 64) {
        gBS->AllocatePool (EfiBootServicesData, 128, &MassStorageName);
        EfiZeroMem(MassStorageName, 128);
        for (p = (UINT8*)&Dev->dev_info->DevNameString, i=0; i<64; i++) {
            if (p[i] == 0) break;
            MassStorageName[i] = (CHAR16)p[i];
        }
        return MassStorageName;
    }

    if( Grp->lang_table == 0 || Grp->lang_table->len == 0 ) return 0;

    Lang = Grp->lang_table->langID[0];

    UsbIoGetInterfaceDescriptor(&Dev->io,&DescIf);
    if( DescIf.Interface && !EFI_ERROR(
        UsbIoGetStringDescriptor(&Dev->io, Lang,
        DescIf.Interface, &StrIf )))
        return StrIf;

    UsbIoGetDeviceDescriptor(&Dev->io, &DescDev);
    if( DescDev.StrProduct && !EFI_ERROR(
        UsbIoGetStringDescriptor(&Dev->io, Lang,
        DescDev.StrProduct, &StrProduct )))
        return StrProduct;

    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        InstallDevice
//
// Description: Adds a device to the tree; creates an EFI handle for the
//              usb device; installs USB_IO and DEVICEPATH protocols
//              on a new device handle; connects a new device to
//              EFI device drivers
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

TREENODE_T*
UsbDevHubNode(
    TREENODE_T  *HcNode,
    DEV_INFO    *DevInfo
)
{
//    int i;
    TREENODE_T *HubNode=0;
    TREENODE_T *HubGrpNode;

    HubGrpNode = TreeSearchDeep(HcNode->child, DevGrpByAddr, &DevInfo->bHubDeviceNumber );
    if (HubGrpNode != NULL){
        HubNode = HubGrpNode->child; //TODO: what if many child (Multy-IF hub???)
        ASSERT(HubNode);
        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: parent Hub found: %x\n", HubNode );
        return HubNode;
    }
/*
    for( i=0;i<COUNTOF(gUsbData->aDevInfoTable) && (HubNode==0);i++){
        if((gUsbData->aDevInfoTable[i].bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT))
            != (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT))
            continue;
        if( gUsbData->aDevInfoTable[i].bHCNumber == DevInfo->bHCNumber &&
            gUsbData->aDevInfoTable[i].bDeviceAddress == DevInfo->bHubDeviceNumber )
        {
            USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: UsbDevHubNode:  valid hub info [%d]: %x\n",i, gUsbData->aDevInfoTable +i );
            InstallDevice( gUsbData->aDevInfoTable +i );
            HubNode = TreeSearchDeep(HcNode->child, DevByInfo, gUsbData->aDevInfoTable +i );
            ASSERT(HubNode);
        }
    }
*/
    return HubNode;
}

VOID InstallDevice(DEV_INFO* DevInfo)
{
    TREENODE_T* HcNode;
    TREENODE_T* HubNode;
    TREENODE_T* ParentNode;
    USBDEV_T* Dev;
    DEVGROUP_T* Grp;
    EFI_STATUS Status;
	EFI_TPL OldTpl;

    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: InstallDevice ");
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "(hc:%x,hub:%x,port:%x,addr:%x,if:%x,aif:%x,lun:%x)\n",
        DevInfo->bHCNumber, DevInfo->bHubDeviceNumber, DevInfo->bHubPortNumber, DevInfo->bDeviceAddress,
        DevInfo->bInterfaceNum, DevInfo->bAltSettingNum, DevInfo->bLUN );

    // Find HC node in tree
    HcNode = TreeSearchSibling(gUsbRootRoot->child, HcByIndex, &DevInfo->bHCNumber );

    // Do not assert here: it's fine to see a DEV_INFO from not-yet-installed HC
    if( HcNode == NULL ) return;
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: HC node found: %x\n", HcNode );

    // Find a hub node in tree
    if( DevInfo->bHubDeviceNumber & BIT7){ // hub is a root HC
        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: Connecting to root Hub\n", DevInfo->bHCNumber );
        ASSERT( (DevInfo->bHubDeviceNumber & ~BIT7 )== DevInfo->bHCNumber );
        HubNode = HcNode;
    } else { // hub is usb hub device
        HubNode = UsbDevHubNode(HcNode, DevInfo);
        ASSERT(HubNode != NULL);
        if (HubNode == NULL) return;
    }

    ParentNode = NULL;
    ParentNode = TreeSearchSibling(HubNode->child, 
        DevGrpByAddr, &DevInfo->bDeviceAddress );

    if( ParentNode == NULL ){
        // Create group
        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: group created\n" );
        Status = gBS->AllocatePool (EfiBootServicesData, sizeof(DEVGROUP_T), &Grp);
		ASSERT_EFI_ERROR(Status);
        EfiZeroMem(Grp, sizeof(DEVGROUP_T));

        Grp->dev_info = DevInfo;
        Grp->hc = ((USBBUS_HC_T*)HcNode->data)->hc;
        Grp->hc_info  = ((USBBUS_HC_T*)HcNode->data)->hc_data;
        Grp->f_DevDesc = FALSE;
        Grp->configs = NULL;
        Grp->config_count = 0;
        Grp->type = NodeGroup;
        Grp->active_config = 0;
        UsbDevLoadAllDescritors(Grp);
        //
        // Check at least for Device Descriptor present before proceeding
        //
        if(Grp->f_DevDesc == FALSE) {
          //
          //When  no Device Descriptor present quit installing the device
          //
          USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: dev install aborted - no device descriptor\n");
          return;
        }
        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: descriptors loaded\n" );
 
                                        //(EIP66231+)>
        if(!(Grp->dev_desc.StrManufacturer == 0 && Grp->dev_desc.StrProduct == 0 && Grp->dev_desc.StrSerialNumber == 0))
        {
            // Load langid table
            Grp->lang_table = (lang_table_t*)GetUsbDescriptor((USBDEV_T*)Grp,DESC_TYPE_STRING,0,0);
            if( Grp->lang_table && Grp->lang_table->len != 0){
                Grp->lang_table->len = (Grp->lang_table->len -2)/sizeof(UINT16);
                USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: LangID table loaded\n" );
            }
        }
                                        //<(EIP66231+)
        TreeAddChild(HubNode,(ParentNode = TreeCreate(&Grp->node, Grp)));
    } else {
        // Old group was found in tree
        TREENODE_T *tmp = TreeSearchSibling(ParentNode->child, DevByAdrIf, DevInfo );
        if(tmp){
            USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: IF is already in tree: %x\n", tmp );
            return;
        }

        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: multi-function; group node found: %x\n", ParentNode );
        Grp = (DEVGROUP_T*)ParentNode->data;
    }

    // Create new device node as a child of HubNode
    gBS->AllocatePool (EfiBootServicesData, sizeof(USBDEV_T), &Dev);
    ASSERT(Dev);
    if (Dev == NULL) return;
    EfiZeroMem(Dev, sizeof(USBDEV_T));

    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: device node created: %x\n",
        &Dev->node );
    Dev->type = NodeDevice;
    TreeAddChild(ParentNode, TreeCreate(&Dev->node, Dev));
    Dev->dev_info = DevInfo;
    Dev->hc = ((USBBUS_HC_T*)HcNode->data)->hc;
    Dev->hc_info  = ((USBBUS_HC_T*)HcNode->data)->hc_data;
    Dev->f_connected = FALSE;

    UsbDevLoadEndpoints(Dev);

    // Speed 00/10/01 - High/Full/Low
                                        //(EIP81612)>
    switch (Dev->dev_info->bEndpointSpeed) {
        case USB_DEV_SPEED_SUPER:
            Dev->speed = EFI_USB_SPEED_SUPER; 
            break;
        case USB_DEV_SPEED_FULL:
            Dev->speed = EFI_USB_SPEED_FULL;
            break;
        case USB_DEV_SPEED_LOW:
            Dev->speed = EFI_USB_SPEED_LOW;
            break;
        case USB_DEV_SPEED_HIGH:
            Dev->speed = EFI_USB_SPEED_HIGH;
    }
                                        //<(EIP81612)
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: endpoints loaded\n" );

    // Create Device Path
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: preparing DP...\n" );

    ASSERT(((USBDEV_T*)HubNode->data)->dp);
    Dev->dp = DpAddUsbSegment(((USBDEV_T*)HubNode->data)->dp,
        DevInfo->bHubPortNumber, DevInfo->bInterfaceNum);
    if(DevInfo->bLUN){
        Dev->dp =   DpAddScsiSegment( Dev->dp, 0, DevInfo->bLUN );
    }
    ASSERT(Dev->dp);

    // Install USB_IO protocol
    Dev->io.UsbControlTransfer          = UsbIoControlTransfer;
    Dev->io.UsbBulkTransfer             = UsbIoBulkTransfer;
    Dev->io.UsbAsyncInterruptTransfer   = UsbIoAsyncInterruptTransfer;
    Dev->io.UsbSyncInterruptTransfer    = UsbIoSyncInterruptTransfer;
    Dev->io.UsbIsochronousTransfer      = UsbIoIsochronousTransfer;
    Dev->io.UsbAsyncIsochronousTransfer = UsbIoAsyncIsochronousTransfer;

    Dev->io.UsbGetDeviceDescriptor      = UsbIoGetDeviceDescriptor;
    Dev->io.UsbGetConfigDescriptor      = UsbIoGetConfigDescriptor;
    Dev->io.UsbGetInterfaceDescriptor   = UsbIoGetInterfaceDescriptor;
    Dev->io.UsbGetEndpointDescriptor    = UsbIoGetEndpointDescriptor;
    Dev->io.UsbGetStringDescriptor      = UsbIoGetStringDescriptor;
    Dev->io.UsbGetSupportedLanguages    = UsbIoGetSupportedLanguages;

    Dev->io.UsbPortReset                = UsbIoPortReset;

//    DEBUG_DELAY();
    //Install DP_ protocol
	Status = gBS->InstallMultipleProtocolInterfaces (
        &Dev->handle,
        &gEfiUsbIoProtocolGuid, &Dev->io,
        &gEfiDevicePathProtocolGuid, Dev->dp,
        NULL);
	ASSERT_EFI_ERROR(Status);
	
    *(UINTN*)Dev->dev_info->Handle = (UINTN)Dev->handle;
	Dev->dev_info->bFlag |= DEV_INFO_DEV_BUS;
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: USB_IO installed\n" );

    {
        VOID* tmp;
        VERIFY_EFI_ERROR(
            gBS->OpenProtocol (
            Dev->hc_info->Controller,
            &gEfiUsb2HcProtocolGuid,
            &tmp,
            gUSBBusDriverBinding.DriverBindingHandle,
            Dev->handle,
            EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER ));
    }

    PROGRESS_CODE(DXE_USB_HOTPLUG);

    OldTpl = pBS->RaiseTPL(TPL_HIGH_LEVEL);
    pBS->RestoreTPL(TPL_CALLBACK);

    // Connect controller to start device drvirs
    Status = gBS->ConnectController(Dev->handle,NULL,NULL,TRUE);

    pBS->RaiseTPL(TPL_HIGH_LEVEL);
    pBS->RestoreTPL(OldTpl);
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: instdev: connect controller: %r\n", Status );

    if( !EFI_ERROR(Status)){
        Dev->f_connected = TRUE;
    }
    return;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UninstallDevice
//
// Description: Disconnects device; uninstalls USB_IO and DEVICEPATH protocols
//              delocates all memory used USB Bus driver to support the device
//              removes device node from the tree; if device has children, all
//              UninstallDevice procedure is repeated for each child.
//
//              In case if disconnect or protocol uninstall fails, it reports
//              error stauts returned from Boot service procedure.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS UninstallDevice(USBDEV_T* Dev)
{
    EFI_STATUS Status;
	EFI_TPL OldTpl;

    //
    // Uninstall connected devices if it's a hub
    //
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: UninstallDevice: node %x; ", &Dev->node);
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "info:%x [adr:%d;if:%d] uninstalling children...\n",
        Dev->dev_info,Dev->dev_info->bDeviceAddress, Dev->dev_info->bInterfaceNum);
    if( TreeSearchSibling( Dev->node.child, eUninstallDevice, &Status ))
        return Status;

    OldTpl = pBS->RaiseTPL(TPL_HIGH_LEVEL);
    pBS->RestoreTPL(TPL_CALLBACK);

	gBS->DisconnectController(Dev->handle,NULL,NULL);

    pBS->RaiseTPL(TPL_HIGH_LEVEL);
    pBS->RestoreTPL(OldTpl);

	if (Dev->async_endpoint != 0) {
		Status = Dev->io.UsbAsyncInterruptTransfer(&Dev->io, Dev->async_endpoint, FALSE, 
			0, 0, NULL, NULL);
		Dev->async_endpoint = 0;
	}

    Status = gBS->CloseProtocol (
        Dev->hc_info->Controller,
        &gEfiUsb2HcProtocolGuid,
        gUSBBusDriverBinding.DriverBindingHandle,
        Dev->handle
    );
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return Status;

    //
    // Try to uninstall protocols
    //
    // they can be denied to uninstall, which result in
    // keeping this device and all bus alive
    //
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: uninstdev: [%d:%d] uninstalling protocols...",
        Dev->dev_info->bDeviceAddress, Dev->dev_info->bInterfaceNum);
    Status = gBS->UninstallMultipleProtocolInterfaces (
        Dev->handle,
        &gEfiUsbIoProtocolGuid, &Dev->io,
        &gEfiDevicePathProtocolGuid, Dev->dp,
        NULL);
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "%r\n", Status );
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return Status;

	Dev->dev_info->bFlag &= ~DEV_INFO_DEV_BUS;
	if (!(Dev->dev_info->bFlag & (DEV_INFO_MASS_DEV_REGD | DEV_INFO_DEV_PRESENT))) {
		Dev->dev_info->bFlag &= ~DEV_INFO_VALID_STRUC;
	}

    //
    // Unistall succeeded, free usbdev
    //
    TreeRemove(&Dev->node);
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: uninstdev: [%d:%d] done.\n",
        Dev->dev_info->bDeviceAddress, Dev->dev_info->bInterfaceNum);

    if(Dev->name)
        gBS->FreePool(Dev->name);
    gBS->FreePool(Dev);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        eUninstallDevice
//
// Description: Enumeration call-back function that is usded
//              uninstall all enumerated device nodes
//              Stops enumeration as soon as error was recieved
// Input:
//              Node - tree node of the USB device or group
//              Contex - pointer to the EFI_STATUS variable that
//                      recieves status information if error
//                      was recieved
// Output:      TRUE  on error found; this will stop enumeration
//              FALSE on succesfull uninstall operation
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
int
eUninstallDevice(
    VOID* Node,
    VOID* Context
)
{
    EFI_STATUS *Status = (EFI_STATUS*)Context;
    DEVGROUP_T* Grp = (DEVGROUP_T*)Node;
    ASSERT(Status);

    if( Grp->type == NodeGroup ){
        //
        // Uninstall all CONNECTED devices within group
        //
        TreeSearchSibling( Grp->node.child, eUninstallDevice, Status );
        if(EFI_ERROR(*Status)){
            USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: devgroup uninstall failed: devaddr:0x%x\n",
                Grp->dev_info->bDeviceAddress );
            return TRUE; //stop enumeration
        }
        // Free devgroup
        TreeRemove(&Grp->node);
        if(Grp->configs ){
            int i;
            for(i=0;i<Grp->config_count;++i){
                if(Grp->configs[i])
                    gBS->FreePool(Grp->configs[i]);
            }
            gBS->FreePool(Grp->configs);
        }
        gBS->FreePool(Grp);
    } else if( Grp->type == NodeDevice ){
        //
        //Uninstall Device
        //
        *Status = UninstallDevice((USBDEV_T*)Node);
        if(EFI_ERROR(*Status)){
            USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: usbdev uninstall failed: if:%d\n",
                ((USBDEV_T*)Node)->dev_info->bInterfaceNum );
            return TRUE; //stop enumeration
        }
    }
    return FALSE;// continue enumeration
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        RemoveHubEcho
//
// Description: Finds the USB hub node that sits on the same
//              path (seq. of (hub ports,if) ) but have different
//              USB address or DEV_INFO node. This could be the result
//              of lost disconnect event or previous error to uninstall
//              USB_IO
// Input:
//              pDevInfo - DEV_INFO structure that is checked for
//                      echoes in the bus
// Output:      EFI_SUCCESS - echo wasn't found or was succesfully removed
//              otherwise return status resulted from attemp to remove the node
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
RemoveHubEcho(
    TREENODE_T  *HcNode,
    UINT8       Addr
)
{
    while(!( Addr  & BIT7 )){
        //
        // Find hub DEV_INFO
        //
        int i;
        TREENODE_T *HubNode;
        TREENODE_T* DevNode;
        DEV_INFO* DevInfo=NULL;

        for ( i= 1; i < MAX_DEVICES; i++)   {
            if( ((gUsbData->aDevInfoTable[i].bFlag &
            (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT | DEV_INFO_DEV_DUMMY))
            != (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT)) &&
             gUsbData->aDevInfoTable[i].bDeviceAddress == Addr )
        {
                DevInfo = gUsbData->aDevInfoTable+i;
                break;
            }
        }
        if ( DevInfo == NULL )
            return EFI_NOT_FOUND;

        //
        // Search for parent hub
        //
        if( DevInfo->bHubDeviceNumber  & BIT7 ){
            //Root hub
            HubNode = HcNode;
        } else {
            //USB hub device
            TREENODE_T* HubGrpNode = TreeSearchDeep(HcNode->child,
                DevGrpByAddr, &DevInfo->bHubDeviceNumber );
            if( HubGrpNode != NULL ){
                USBDEV_T* Dev;

                HubNode = HubGrpNode->child; //TODO: what if many child (Multy-IF hub???)
                ASSERT(HubNode);
                DevNode = TreeSearchSibling(HubNode->child,
                            DevGrpByPortIf, DevInfo);
                if(DevNode==NULL) return EFI_SUCCESS;
                Dev = (USBDEV_T*)DevNode->data;
                if( Dev && (DevInfo->bDeviceAddress !=
                    Dev->dev_info->bDeviceAddress ||
                    DevInfo != Dev->dev_info ))
                {
                    //
                    // The disconnect event must have been droped
                    // disconnect old info now
                    //

                    USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: echo found [%x]:\n" );
                    USB_DEBUG(DEBUG_LEVEL_USBBUS,
                        "\t(hc:0x%x,hub:0x%x,port:%d,addr:0x%x,if:%d)\n",
                        Dev->dev_info->bHCNumber,
                        Dev->dev_info->bHubDeviceNumber,
                        Dev->dev_info->bHubPortNumber,
                        Dev->dev_info->bDeviceAddress,
                        Dev->dev_info->bInterfaceNum);
                    return RemoveDevInfo(Dev->dev_info);
                }
                return EFI_SUCCESS;
            } else {
                //
                // Either hub wasn't added to bus yet; or there is echo for the
                // brunch. The the later case succesfull removal of the burnch
                // removes an echo for this device info
                //
                Addr = DevInfo->bHubDeviceNumber;

            }
        }
    }
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        RemoveDevInfoEcho
//
// Description: Finds the USB device node that sits on the same
//              path (seq. of (hub ports,if) ) but have different
//              USB address or DEV_INFO node. This could be the result
//              of lost disconnect event or previous error to uninstall
//              USB_IO
// Input:
//              DevInfo - DEV_INFO structure that is checked for
//                      echoes in the bus
// Output:      EFI_SUCCESS - echo wasn't found or was succesfully removed
//              otherwise return status resulted from attemp to remove the node
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
RemoveDevInfoEcho(
    DEV_INFO    *DevInfo
)
{
    USBDEV_T    *Dev;
    TREENODE_T  *DevNode;
    TREENODE_T  *HcNode, *HubNode;

    HcNode = TreeSearchSibling(gUsbRootRoot->child,HcByIndex,
        &DevInfo->bHCNumber);
    if(HcNode==NULL) return EFI_SUCCESS;

    if( DevInfo->bHubDeviceNumber  & BIT7 ){
        //Root hub
        HubNode = HcNode;
    } else {
        //USB hub device
        TREENODE_T* HubGrpNode = TreeSearchDeep(HcNode->child,
            DevGrpByAddr, &DevInfo->bHubDeviceNumber );
        if( HubGrpNode != NULL ){
            HubNode = HubGrpNode->child; //TODO: what if many child (Multy-IF hub???)
            ASSERT(HubNode);
        } else {
            //
            // Either hub wasn't added to bus yet; or there is echo for the
            // brunch. The the later case succesfull removal of the burnch
            // removes an echo for this device info
            //
            //return RemoveHubEcho(HcNode, DevInfo->bHubDeviceNumber);
            return EFI_SUCCESS;
        }
    }
    DevNode = TreeSearchSibling(HubNode->child, DevGrpByPortIf, DevInfo);
    if(DevNode==NULL) return EFI_SUCCESS;
    Dev = (USBDEV_T*)DevNode->data;
    if (Dev != NULL) {
        //
        // The disconnect event must have been droped
        // disconnect old info now
        //

        USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: echo found [%x]:\n" );
        USB_DEBUG(DEBUG_LEVEL_USBBUS,
            "\t(hc:0x%x,hub:0x%x,port:%d,addr:0x%x,if:%d)\n",
            Dev->dev_info->bHCNumber,
            Dev->dev_info->bHubDeviceNumber,
            Dev->dev_info->bHubPortNumber,
            Dev->dev_info->bDeviceAddress,
            Dev->dev_info->bInterfaceNum);
        return RemoveDevInfo(Dev->dev_info);
    }
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        RemoveDevInfo
//
// Description: Removes device node from the USB bus tree. Device node
//              corresponds to the DEV_INFO. Device gets removed in response to
//              the pending removal event sheduled from SMM when disconnect
//              was detected on USB
// Input:
//              DevInfo - DEV_INFO structure that was disconnect
//
// Output:      EFI_SUCCESS - echo wasn't found or was succesfully removed
//              otherwise return status resulted from attemp to remove the node
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS RemoveDevInfo(DEV_INFO* DevInfo)
{
    TREENODE_T  *DevNode;
    TREENODE_T  *HcNode;
    EFI_STATUS  Status;

    HcNode = TreeSearchSibling(gUsbRootRoot->child, HcByIndex,
        &DevInfo->bHCNumber);
    if(HcNode==NULL) return EFI_NOT_FOUND;

    //Check for echoes

    DevNode = TreeSearchDeep(HcNode->child,DevGrpByAddr,
        &DevInfo->bDeviceAddress);
    if (DevNode==NULL){
        USB_DEBUG(DEBUG_LEVEL_USBBUS, "\tdevice is not found in the tree...\n" );
        return EFI_NOT_FOUND;
    }
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "\tdevice found in the tree...\n" );

    Status = EFI_SUCCESS;
    eUninstallDevice(DevNode->data, &Status);
    USB_DEBUG(DEBUG_LEVEL_USBBUS, "\tDisconnect complete: %r.\n", Status );

    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbHcOnTimer
//
// Description: Timer call-back routine that is
//              is used to monitore changes on USB Bus
//              It monitors the state of queueCnnct and queueDiscnct queues
//              which get populated by UsbSmiNotify call-back routine
//
//              When this routine finds a new device connected to usb it
//              will install a device node for that device by calling
//              InstallDevice
//
//              When this routine finds a disconneced device it uninstalls the
//              device node by calling UninstallDevice
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbHcOnTimer(
    EFI_EVENT   Event,
    VOID        *Context
)
{
    DEV_INFO* EventCnnct=0;
    static int i=0;

    UsbSmiPeriodicEvent();

	if (Event) {
		gBS->SetTimer(Event, TimerCancel, ONESECOND/10);
	}

    do {
        ATOMIC({EventCnnct = QueueGet(&gUsbData->QueueCnnctDisc);});

        if (EventCnnct == NULL) break;
        //
        // There is no need to raise tpl here: this is callback of Event with
        // TPL_CALLBACK, so this code doesn't reenter; the Install and Uninstall
        // are also called from driver stop and start. Stop and start protect
        // the code at TPL_CALLBACK level
        //
        if( (EventCnnct->bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT |
            DEV_INFO_DEV_DUMMY)) == (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT) ){
            USB_DEBUG(DEBUG_LEVEL_USBBUS, "UsbHcOnTimer:: event connect [%d]: %x\n", i++, EventCnnct);

            RemoveDevInfoEcho(EventCnnct);
            InstallDevice(EventCnnct);
        } else if ((EventCnnct->bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT |
            DEV_INFO_DEV_DUMMY)) == DEV_INFO_VALID_STRUC) {
            USB_DEBUG(DEBUG_LEVEL_USBBUS,
                "UsbHcOnTimer:: event disconnect [%d]: %x\n", i++, EventCnnct);
            USB_DEBUG(DEBUG_LEVEL_USBBUS,
                "\t(hc:0x%x,hub:0x%x,port:%d,addr:0x%x,if:%d)\n",
                EventCnnct->bHCNumber, EventCnnct->bHubDeviceNumber,
                EventCnnct->bHubPortNumber, EventCnnct->bDeviceAddress,
                EventCnnct->bInterfaceNum);
            //RemoveDevInfoEcho(EventCnnct);
            RemoveDevInfo(EventCnnct);
        }
    } while ( EventCnnct != NULL );

	if (Event) {
		gBS->SetTimer(Event, TimerPeriodic, ONESECOND/10);
	}
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  PopulateTree
//
// Description: Enumerate all DEV_INFO structures in the aDevInfoTable array
//              and install all currently connected device
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID PopulateTree()
{
    int i;

    PROGRESS_CODE(DXE_USB_DETECT);

	UsbHcOnTimer(gEvUsbEnumTimer, NULL);

    for (i = 1; i < COUNTOF(gUsbData->aDevInfoTable); i++) {
        if((gUsbData->aDevInfoTable[i].bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT | 
			DEV_INFO_DEV_DUMMY | DEV_INFO_DEV_BUS)) == (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT)) {
			//
			// Valid and present device behind specified HC, so insert it in the tree
			//
			USB_DEBUG(DEBUG_LEVEL_3, "USBBUS: PopulateTree: found valid dev info[%d]: %x\n",i, gUsbData->aDevInfoTable +i );
			
			//RemoveDevInfoEcho(&gUsbData->aDevInfoTable[i]);
			//InstallDevice( gUsbData->aDevInfoTable + i );
			QueuePut(&gUsbData->QueueCnnctDisc, &gUsbData->aDevInfoTable[i]);
        }
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbBusGetControllerName
//
// Description: This function is a part of binding protocol, it returns
//              the string with the controller name.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

CHAR16*
UsbBusGetControllerName(
    EFI_HANDLE  Controller,
    EFI_HANDLE  Child
)
{
                                        //(EIP60745+)>
    EFI_STATUS Status;

    Status = gBS->OpenProtocol ( Controller,
                            &gEfiUsb2HcProtocolGuid,
                            NULL,
                            gUSBBusDriverBinding.DriverBindingHandle,
                            Controller,
                            EFI_OPEN_PROTOCOL_TEST_PROTOCOL );

    if (Status != EFI_SUCCESS && Status != EFI_ALREADY_STARTED) {
        return NULL;
    }
                                        //<(EIP60745+)
    if(Child) {
        //Get name for USB device
        EFI_USB_IO_PROTOCOL *UsbIo;
        USBDEV_T            *Dev ;
        if( EFI_ERROR(gBS->HandleProtocol(Child,& gEfiUsbIoProtocolGuid, &UsbIo)))
        {
            return NULL;
        }
        Dev = UsbIo2Dev(UsbIo);
        if( Dev->name == 0)
            Dev->name = LoadName(Dev);
        return Dev->name;
    } else {
        //Get name for USB HC
        return L"USB Host Controller (USBBUS)";
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbBusInit
//
// Description: Install driver binding and controller name protocols
//              for the USB bus driver.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbBusInit(
    EFI_HANDLE  ImageHandle,
    EFI_HANDLE  ServiceHandle
)
{
                                        //(EIP59272)>
    static NAME_SERVICE_T usbbus_names;
    gUSBBusDriverBinding.DriverBindingHandle = ServiceHandle;
    gUSBBusDriverBinding.ImageHandle = ImageHandle;

    return gBS->InstallMultipleProtocolInterfaces(
                &gUSBBusDriverBinding.DriverBindingHandle,
                &gEfiDriverBindingProtocolGuid, &gUSBBusDriverBinding,
                &gEfiComponentName2ProtocolGuid, InitNamesProtocol(&usbbus_names,
                L"USB bus", UsbBusGetControllerName),
                NULL);
                                        //<(EIP59272)
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbBusSupported
//
// Description: This is a binding protocol function that returns EFI_SUCCESS
//              for USB controller and EFI_UNSUPPORTED for any other kind of
//              controller.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbBusSupported(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE Controller,
    EFI_DEVICE_PATH_PROTOCOL *DevicePath
)
{
    EFI_STATUS  Status;
    VOID*       Ptr = 0;

    Status = gBS->OpenProtocol ( Controller, &gEfiUsb2HcProtocolGuid,
        &Ptr, This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER );

    if (Status != EFI_SUCCESS && Status != EFI_ALREADY_STARTED) {
        return EFI_UNSUPPORTED;
    } else if (Status == EFI_ALREADY_STARTED) {
        return Status;
    }

    gBS->CloseProtocol ( Controller, &gEfiUsb2HcProtocolGuid,
        This->DriverBindingHandle, Controller);

    Status = gBS->OpenProtocol ( Controller, &gEfiDevicePathProtocolGuid,
        NULL, This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_TEST_PROTOCOL );

    if (Status != EFI_SUCCESS && Status != EFI_ALREADY_STARTED) {
        return EFI_UNSUPPORTED;
    }

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbBusStop
//
// Description: This function is part of binding protocol installed on USB
//              controller. It stops the controller and removes all the
//              children.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbBusStop (
   EFI_DRIVER_BINDING_PROTOCOL     *This,
   EFI_HANDLE                      Controller,
   UINTN                           NumberOfChildren,
   EFI_HANDLE                      *Children  )
{
    EFI_STATUS	Status = EFI_SUCCESS;
    TREENODE_T	*HcNode;
    USBBUS_HC_T	*HcDev;

    //EFI_TPL SaveTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);
    //ASSERT(SaveTpl <= EFI_TPL_NOTIFY );

	HcNode = TreeSearchSibling(gUsbRootRoot->child, HcByHandle, &Controller );
	ASSERT(HcNode);
	if( HcNode == NULL ) {
		//gBS->RestoreTPL(SaveTpl);
		return EFI_DEVICE_ERROR;
	}
	HcDev = (USBBUS_HC_T*)HcNode->data;
    
	UsbHcOnTimer(gEvUsbEnumTimer, NULL);
    
	VERIFY_EFI_ERROR(
		gBS->SetTimer ( gEvUsbEnumTimer, TimerCancel, ONESECOND/10));
    
    
	if (TreeSearchSibling(HcNode->child, eUninstallDevice, &Status) != NULL)
	{
		USB_DEBUG(DEBUG_LEVEL_USBBUS, 
			"USBBUS: Stop HC: [%d] failed to uninstall children\n",
			((USBBUS_HC_T*)HcNode->data)->hc_data->bHCNumber);
		//gBS->RestoreTPL(SaveTpl);
	    VERIFY_EFI_ERROR(
			gBS->SetTimer ( gEvUsbEnumTimer, TimerPeriodic, ONESECOND/10));
		return Status;
	}

	if (NumberOfChildren == 0) {
		TreeRemove(HcNode);

		//
		// Close Protocols opened by driver
		//
		gBS->CloseProtocol (
			Controller, &gEfiUsb2HcProtocolGuid,
			This->DriverBindingHandle, Controller);
		
		gBS->FreePool(HcDev);
		
		if(--gCounterUsbEnumTimer==0){
			VERIFY_EFI_ERROR(
				gBS->SetTimer ( gEvUsbEnumTimer, TimerCancel, ONESECOND/10));
			VERIFY_EFI_ERROR(
				gBS->CloseEvent (gEvUsbEnumTimer));
			gEvUsbEnumTimer=0;
		}
	}

    if (gCounterUsbEnumTimer != 0) {
	    VERIFY_EFI_ERROR(
			gBS->SetTimer ( gEvUsbEnumTimer, TimerPeriodic, ONESECOND/10));
    }
    //gBS->RestoreTPL(SaveTpl);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbBusStart
//
// Description: This function is part of binding protocol installed on USB
//              controller. It starts the USB bus for a given controller.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbBusStart(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    EFI_DEVICE_PATH_PROTOCOL    *DevicePathProtocol
)
{
    USBBUS_HC_T* HcDev = 0;
	EFI_STATUS	Status = EFI_UNSUPPORTED;

    VERIFY_EFI_ERROR (
        gBS->AllocatePool (
            EfiBootServicesData,
            sizeof(USBBUS_HC_T),
            &HcDev ));
    HcDev->type = NodeHC;
    HcDev->hc_data = FindHcStruc(Controller);

//    ASSERT(HcDev->hc_data);
    if (HcDev->hc_data == NULL) {
        gBS->FreePool(HcDev);
        return EFI_DEVICE_ERROR;
    }

    //
    // Open Protocols
    //
	Status = gBS->OpenProtocol ( Controller,
		&gEfiUsb2HcProtocolGuid,	&HcDev->hc,
		This->DriverBindingHandle, Controller,
		EFI_OPEN_PROTOCOL_BY_DRIVER );
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return Status;
	}

    Status = gBS->OpenProtocol ( Controller,
        &gEfiDevicePathProtocolGuid,
        &HcDev->dp, This->DriverBindingHandle,
        Controller, EFI_OPEN_PROTOCOL_GET_PROTOCOL );
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return Status;
	}

    //
    // Install Polling timer
    //
    {
        //EFI_TPL SaveTpl = gBS->RaiseTPL (EFI_TPL_NOTIFY);
        //ASSERT( SaveTpl <= EFI_TPL_NOTIFY);
        if(gEvUsbEnumTimer != 0) {
            VERIFY_EFI_ERROR(
                gBS->SetTimer ( gEvUsbEnumTimer, TimerCancel, ONESECOND/10));
        }
        //
        // Critical section

//        HookSmiNotify(1);

        USB_DEBUG(DEBUG_LEVEL_USBBUS,"**** \tnew hc_struc: %x(type:%x,number:%x)\n",
                HcDev->hc_data, HcDev->hc_data->bHCType, HcDev->hc_data->bHCNumber);

        //
        // Create HC branch in the USB root
        //
        TreeAddChild(gUsbRootRoot, TreeCreate(&HcDev->node,HcDev));

        gCounterUsbEnumTimer++;
        PopulateTree();

		UsbHcOnTimer(gEvUsbEnumTimer, NULL);

        // Setting up global: gUsbDeviceToDisconnect, gUsbDeviceToConnect
        if (gEvUsbEnumTimer == 0) {
            USB_DEBUG(DEBUG_LEVEL_USBBUS, "USBBUS: Start: setup timer callback %x\n", &UsbHcOnTimer );
            VERIFY_EFI_ERROR(
                gBS->CreateEvent ( EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
//(EIP35956) Raise the tpl level more then EFI_TPL_CALLBACK to make it can be called in other callback function.
                EFI_TPL_NOTIFY, UsbHcOnTimer,0,&gEvUsbEnumTimer));  
        }
        VERIFY_EFI_ERROR(
            gBS->SetTimer ( gEvUsbEnumTimer, TimerPeriodic, ONESECOND/10));
        //gBS->RestoreTPL(SaveTpl);
    }

    return EFI_SUCCESS;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
