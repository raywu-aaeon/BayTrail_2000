#pragma warning(disable: 4001)
//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2011, American Megatrends, Inc.          **
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usbpoint.c 7     9/14/12 5:11a Roberthsu $
//
// $Revision: 7 $
//
// $Date: 9/14/12 5:11a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           UsbPoint.c
//
//  Description:    AMI USB Absolute Device support implementation
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"
#include "UsbKbd.h"

extern  USB_GLOBAL_DATA     *gUsbData;

extern  UINT8   USB_InstallCallBackFunction (CALLBACK_FUNC);
static BOOLEAN globalAbsolutePolling = TRUE;

UINT8       USBAbsProcessMouseData (HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);
VOID		SetABSData (UINT8*,UINT16*,UINT8,UINT8,UINT16);			 
UINT16      PerviousXPosition=0;
UINT16      PerviousYPosition=0;


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAbsConfigureDevice 
//
// Description: This routine checks an interface descriptor of the USB device
//              detected to see if it describes a HID/Boot/Mouse device.
//              If the device matches the above criteria, then the device is
//              configured and initialized
//
// Input:       fpHCStruc   HCStruc pointer
//              fpDevInfo   Device information structure pointer
//              fpDesc      Pointer to the descriptor structure
//              wStart      Offset within interface descriptor
//                      supported by the device
//              wEnd        End offset of the device descriptor
//
// Output:      FPDEV_INFO  New device info structure, 0 on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBAbsConfigureDevice (
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16      wStart,
    UINT16      wEnd)
{
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBAbsProcessMouseData
//
// Description: 
//
// Input:       fpHCStruc   Pointer to HCStruc
//              fpDevInfo   Pointer to device information structure
//              fpTD        Pointer to the polling TD
//              fpBuffer    Pointer to the data buffer
//
// Output:      Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

extern EFI_EMUL6064MSINPUT_PROTOCOL* gMsInput;

PUBLIC
UINT8
USBAbsProcessMouseData (
    HC_STRUC    *fpHcStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       *fpTD,
    UINT8       *fpBuffer
)
{
	USBABS_DATA	fpABSbuffer;
	HIDReport_STRUC 	*Hidreport;
	UINT8 	offset_tmp=0,X_start=0,X_end,Y_start=0,Y_end,button_start=0,button_end=0,i,j;		//(EIP101990)
	UINT16 	max_x,max_y;
	Hidreport = &(fpDevInfo->Hidreport);
                                        //(EIP80173+)>
    fpABSbuffer.X =0;
    fpABSbuffer.Y =0;
    fpABSbuffer.Button =0;
                                        //<(EIP80173+)
//	serach button and X Y
	for(i=0;i<Hidreport->bTotalCount;i++)
	{
		//Check is input?
		if((Hidreport->pReport[i].bFlag & HID_BFLAG_INPUT ))
		{
			//if report id exist, check first byte
			if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID) 
				{
					if(Hidreport->pReport[i].bReportID != *(fpBuffer))continue;
				}
			if((Hidreport->pReport[i].bUsagePage == 0xd)&&(Hidreport->pReport[i].bUsageCount)!=0)
			{ 
				//serach 
				for(j=0;j<Hidreport->pReport[i].bUsageCount;j++) 
				{
					//Check Tip switch
					if(Hidreport->pReport[i].bUsage[j] == 0x42)	//(EIP79323)
					{
						if(button_start != 0)break;								//(EIP66231)
						button_start=offset_tmp;
						if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)button_start+=8; 
                        button_end = button_start + Hidreport->pReport[i].bReportSize * Hidreport->pReport[i].bReportCount;		//(EIP101990)
                        SetABSData(fpBuffer,(UINT16*)(&fpABSbuffer.Button),button_start,button_end,0xffff);        				//(EIP101990) 
					}
				}
			}
			//Check Button
   		    if((Hidreport->pReport[i].bUsagePage == 9)&&(Hidreport->pReport[i].wUsageMin[0] == 1))      //(EIP114280)
			{
                if(button_start != 0)break;     //(EIP114280)
				button_start=offset_tmp;
				if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)button_start+=8; 
                button_end = button_start + Hidreport->pReport[i].bReportSize * Hidreport->pReport[i].bReportCount;		//(EIP101018)
                SetABSData(fpBuffer,(UINT16*)(&fpABSbuffer.Button),button_start,button_end,0xffff);        				//(EIP101018)
			}
			//Check X,Y
			if((Hidreport->pReport[i].bUsagePage == 1)&&(Hidreport->pReport[i].bUsageCount)!=0)
			{
				//serach 
				for(j=0;j<Hidreport->pReport[i].bUsageCount;j++)
				{
					//find X
					if(Hidreport->pReport[i].bUsage[j] == 0x30)
					{
						if(X_start!=0)break;									//(EIP66231)
						X_start=(offset_tmp+j*Hidreport->pReport[i].bReportSize); 
						if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)X_start+=8;  
						X_end = X_start + Hidreport->pReport[i].bReportSize;
						SetABSData(fpBuffer,&fpABSbuffer.X,X_start,X_end,Hidreport->pReport[i].wLogicalMax);        //(EIP81983)
						max_x = Hidreport->pReport[i].wLogicalMax;
					}

					//find Y
					if(Hidreport->pReport[i].bUsage[j] == 0x31)
					{
						if(Y_start!=0)break; 									//(EIP66231)
						Y_start=(offset_tmp+j*Hidreport->pReport[i].bReportSize);
						if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)Y_start+=8;  
						Y_end=Y_start + Hidreport->pReport[i].bReportSize;
						max_y = Hidreport->pReport[i].wLogicalMax; 
						SetABSData(fpBuffer,&fpABSbuffer.Y,Y_start,Y_end,Hidreport->pReport[i].wLogicalMax);        //(EIP81983)
					}
				}
			}
			offset_tmp+=(Hidreport->pReport[i].bReportCount*Hidreport->pReport[i].bReportSize);
		}
	}
	
	if(fpABSbuffer.Button == 0 && fpABSbuffer.X == 0 && fpABSbuffer.Y == 0)	//(EIP79323)
		return USB_SUCCESS;

    if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {
        gUsbData->AbsMouseData[0].ButtonStauts=fpABSbuffer.Button;
        gUsbData->AbsMouseData[0].Xcoordinate=fpABSbuffer.X;
        gUsbData->AbsMouseData[0].Ycoordinate=fpABSbuffer.Y;
        gUsbData->AbsMouseData[0].Pressure=0;
		gUsbData->AbsMouseData[0].Max_X = max_x;
		gUsbData->AbsMouseData[0].Max_Y = max_y;
    }
    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetABSData
//
// Description: 
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SetABSData (
    UINT8       *fpBuffer,
    UINT16 		*reportdata,
    UINT8 		start,
    UINT8 		end,
    UINT16 		wMaxvalue
    
)
{
	UINT8 	reportsize, size,preskip,postskip;
	UINT32  tempdata = 0;
	reportsize = end - start;
	size =  reportsize /8;
	if((reportsize%8)!=0 )size++;

	ASSERT(size > 0 && size <= sizeof(tempdata));
    if ((size == 0) || (size > sizeof(tempdata))) {
        return;
    }
    
	MemCpy(&tempdata, fpBuffer + start / 8, size);
    
	preskip =  start % 8;
	postskip =  end % 8;

	if(preskip != 0) {
		tempdata=tempdata >> preskip;
	}
	if(postskip != 0) {
		tempdata=tempdata << postskip;
		tempdata=tempdata >> postskip;
	}
	if(tempdata > wMaxvalue) {
		tempdata &= wMaxvalue;
	}
	*reportdata = (UINT16)tempdata;
	USB_DEBUG (DEBUG_LEVEL_4, "out data %x\n",*reportdata); 
	return;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2011, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
