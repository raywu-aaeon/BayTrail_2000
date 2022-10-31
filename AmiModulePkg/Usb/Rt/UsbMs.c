#pragma warning(disable: 4001)
//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usbms.c 38    8/21/12 2:33a Roberthsu $
//
// $Revision: 38 $
//
// $Date: 8/21/12 2:33a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           UsbMs.c
//
//  Description:    AMI USB mouse support implementation
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"
#include "UsbKbd.h"

extern  USB_GLOBAL_DATA     *gUsbData;

extern UINT8 IsKbcAccessBlocked;	//(EIP29733+)

extern  UINT8   USB_InstallCallBackFunction (CALLBACK_FUNC);

VOID*     USB_MemAlloc(UINT16);
UINT8     USB_MemFree (VOID _FAR_ *, UINT16);

VOID        USBMSInitialize (VOID);
DEV_INFO*   USBMSConfigureDevice (HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT16);
UINT8       USBMSProcessMouseData (HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);
VOID        USBKeyRepeat(HC_STRUC*, UINT8);

VOID UHCI_PausePolling( HC_STRUC* fpHCStruc, DEV_INFO* fpDevInfo);
VOID		SetMouseData (UINT8*,USBMS_DATA*,UINT8,UINT8,HID_STRUC);		//(EIP127014+)		
UINT8       Org_bButtonStatus = 0;		//(EIP91835)

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBMSInitialize
//
// Description: This routine is called once to initialize the USB mouse data
//              area
//
// Input:       None
//
// Output:      Nothing
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMSInitialize()
{
    //
    // Initialize the mouse input buffer head and tail values
    //
    gUsbData->fpMouseInputBufferHeadPtr = &gUsbData->aMouseInputBuffer[0];
    gUsbData->fpMouseInputBufferTailPtr = &gUsbData->aMouseInputBuffer[0];
USB_DEBUG(DEBUG_LEVEL_3, "USBMSInitialize: Head and Tail are at %x\n", gUsbData->fpMouseInputBufferHeadPtr);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMSConfigureDevice
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
USBMSConfigureDevice (
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
// Name:        USBMSProcessMouseData
//
// Description: This function is called at regular intervals with USB mouse
//              report data. This function handles the translation of USB
//              mouse data into PS/2 mouse data, and makes the PS/2 data
//              available to software using ports 60/64 to communicate with
//              a PS/2 mouse.
//
// Input:       fpHCStruc   Pointer to HCStruc
//              fpDevInfo   Pointer to device information structure
//              fpTD        Pointer to the polling TD
//              fpBuffer    Pointer to the data buffer
//
// Output:      Nothing
//
// Notes:       The format of 3 byte data packet is as follow:
//               Byte              Description
//          -----------------------------------------------------------
//              0   Bit     Description
//              -------------------------------------------
//                   0      If set, button 1 is pressed
//                   1      If set, button 2 is pressed
//                   2      If set, button 3 is pressed
//                   3-7        Reserved
//              -------------------------------------------
//              1   X displacement value
//              2   Y displacement value
//          -----------------------------------------------------------
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

extern EFI_EMUL6064MSINPUT_PROTOCOL* gMsInput;


PUBLIC
UINT8
USBMSProcessMouseData (
    HC_STRUC    *fpHcStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       *fpTD,
    UINT8       *fpBuffer
)
{
    UINT8*  fpPtr = (UINT8*)(UINTN)0x410;
    PS2MouseData mouseData;
    INT32   Coordinates;
										//(EIP38434+)>
	HIDReport_STRUC 	*Hidreport;
	UINT8 	offset_tmp=0,X_start,X_end,Y_start,Y_end,button_start,wheel_start,i,j;
	USBMS_DATA Tempdata;
    UINT8   Button_Set = 0,X_Set = 0,Y_Set =0,Wheel_Set =0;         //(EIP91835)

    //Is KBC access allowed?
    if(IsKbcAccessBlocked) return USB_SUCCESS;	//(EIP29733+)
    MemSet((UINT8*)&Tempdata, sizeof(USBMS_DATA), 0);	//(EIP127014)

	Hidreport = &(fpDevInfo->Hidreport);
//		USB_DEBUG (DEBUG_LEVEL_3, "count is  %x\n",Hidreport->bTotalCount);	

	if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_PROTOCOL) 
	{	

		//serach button and X Y
		for(i=0;i<Hidreport->bTotalCount;i++)
		{
			//Check is input?
		//		USB_DEBUG (DEBUG_LEVEL_3, "report%x Flag is %x\n",i,Hidreport->pReport[i].bFlag);
			if((Hidreport->pReport[i].bFlag & HID_BFLAG_INPUT ))
			{
				//if report id exist, check first byte
				if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID) 
				{
					if(Hidreport->pReport[i].bReportID != *(fpBuffer))continue;
				}

				//Check Button
				if((Hidreport->pReport[i].bUsagePage == 9)&&(Hidreport->pReport[i].wUsageMin[0] == 1)) //(EIP84455)
				{
                                        //(EIP91835+)>
                    Button_Set = 1;         
					button_start=offset_tmp;
					if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)button_start+=8; 
					button_start/=8;
                    Tempdata.ButtonByte = *(fpBuffer+button_start);		
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
                            X_Set = 1;         
							X_start=(offset_tmp+j*Hidreport->pReport[i].bReportSize); 
							if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)X_start+=8;  
							X_end = X_start + Hidreport->pReport[i].bReportSize;
                            Tempdata.FillUsage = 0x30;	//(EIP127014)
							SetMouseData(fpBuffer,&Tempdata,X_start,X_end,Hidreport->pReport[i]);	//(EIP127014)
						}

						//find Y
						if(Hidreport->pReport[i].bUsage[j] == 0x31)
						{
                            Y_Set =  1;         
							Y_start=(offset_tmp+j*Hidreport->pReport[i].bReportSize);
							if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)Y_start+=8;  
							Y_end=Y_start + Hidreport->pReport[i].bReportSize;
                            Tempdata.FillUsage = 0x31;	//(EIP127014)
							SetMouseData(fpBuffer,&Tempdata,Y_start,Y_end,Hidreport->pReport[i]);	//(EIP127014)
						}
						//find Wheel
						if(Hidreport->pReport[i].bUsage[j] == 0x38)
						{
                            Wheel_Set = 1;
							wheel_start=(offset_tmp+j*Hidreport->pReport[i].bReportSize)/8;
							if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID)wheel_start+=1;   
						}   
					}
				}
				offset_tmp+=(Hidreport->pReport[i].bReportCount*Hidreport->pReport[i].bReportSize);
		//			USB_DEBUG (DEBUG_LEVEL_3, "offset_tmp %x \n",offset_tmp); 		
			}
		}
                                        
        for(i=0;i<8;i++)
          *(fpBuffer+i)=0; 

		//fill MS DATA
        if(Button_Set != 0)
        {
    		*fpBuffer      = Tempdata.ButtonByte;
            Org_bButtonStatus = Tempdata.ButtonByte;
        }
        else
        {
            *fpBuffer = Org_bButtonStatus;
        }	

        if(X_Set == 1)
    	    *(fpBuffer+1)  = Tempdata.X;
        if(Y_Set == 1) 
		    *(fpBuffer+2)  = Tempdata.Y;
        if(Wheel_Set == 1)  
		    *(fpBuffer+3)  =*(fpBuffer+wheel_start);
                                        //<(EIP91835+)

	}
										//<(EIP38434+)

    if (fpDevInfo->wIncompatFlags & USB_INCMPT_BOOT_PROTOCOL_IGNORED) {
        fpBuffer++;
    }
                                        //(EIP67400+)>
#if (!BOOT_PROTOCOL_SUPPORT)
    if(!(Button_Set || X_Set || Y_Set || Wheel_Set))return USB_SUCCESS;         //(EIP91835)
#endif    
                                        //<(EIP67400+)
    
    if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {

        gUsbData->MouseData.ButtonStatus = *(UINT8*)fpBuffer;                   //(EIP63752)

        Coordinates = (INT16)Tempdata.EfiX;		//(EIP127014)
        gUsbData->MouseData.MouseX += Coordinates;

        Coordinates = (INT16)Tempdata.EfiY;		//(EIP127014)
        gUsbData->MouseData.MouseY += Coordinates;

        Coordinates=*((INT8*)fpBuffer + 3);
        gUsbData->MouseData.MouseZ += Coordinates;

        return USB_SUCCESS; // Here should be code that prepares buffer for AMIUHCD
    }

    if (!(*fpPtr & 4)) {
        return  USB_SUCCESS;    // No mouse indication in BIOS Data area equipment byte
    }
    gUsbData->fpSavedHCStruc   = fpHcStruc;
/*
										//(EIP57745+)>
    //
    // Check the version of CSM16, support is available for ver 7.64 or later
    //
    {
        UINT8	MjCsmVer = *(UINT8*)0xF0018;
        UINT8	MnCsmVer = *(UINT8*)0xF0019;
		UINT8	mouse_flag3 = *((UINT8*)((UINTN)((*(UINT16*)0x40E) << 4) + 0x30));

        if (MjCsmVer > 7 || MnCsmVer > 0x63) {
			if(!(mouse_flag3 & BIT0)) {
				return USB_SUCCESS;
			}
        }
    } 
										//<(EIP57745+)
*/
    //
    // Check mouse data availability
    //
    if ( gMsInput != 0 ){
        //
        // Get mouse status byte and prepare it.
        // Bit 2, 1, 0 = Middle, right and left button status
        // Bit 3 is always 1
        //
        mouseData.flags = (*(UINT8*)fpBuffer)& 7 | 8;

        //
        // Get mouse X, Y position
        //
        mouseData.x = (*((UINT8*)fpBuffer + 1));
        mouseData.y = (UINT8)(-*((INT8*)fpBuffer + 2)); // Y data is opposite in USB than PS2

        //
        // Verify the direction of X-axis movement
        //
        if (mouseData.x >= 0x80) {
            mouseData.flags     |= 0x10;    // Negative X-axis movement
        }
        if (mouseData.y >= 0x80) {
            mouseData.flags     |= 0x20;    // Negative Y-axis movement
        }

        if(gUsbData->kbc_support || (gUsbData->dUSBStateFlag & USB_FLAG_6064EMULATION_ON)){
            gMsInput->Send(gMsInput,&mouseData);
            USBKeyRepeat(NULL, 2);          // Enable Key repeat	//(EIP49214+)
        }				    
    }

    return USB_SUCCESS;
}
										//(EIP127014+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetMouseData
//
// Description: This routine checks for mouse type device from the
//              interface data provided
//
// Input:   bBaseClass  USB base class code
//          bSubClass   USB sub-class code
//          bProtocol   USB protocol code
//
// Output:  BIOS_DEV_TYPE_MOUSE type on success or 0FFH on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SetMouseData (
    UINT8           *fpBuffer,
    USBMS_DATA 	    *MsData,
    UINT8 		    start,
    UINT8 		    end,
    HID_STRUC       Report
)
{
	UINT8 	reportsize, size,preskip,postskip;
	UINT16  tempdata = 0;
    UINT16  MinMask = 0;
    UINT16  Multi = 1;
    UINT16  Resolution;
    UINT16  Count = 0;
    UINT16  i;

    if((Report.PhysicalMax == 0) && (Report.PhysicalMin == 0)){
        Report.PhysicalMax = Report.wLogicalMax;
        Report.PhysicalMin = Report.wLogicalMin;
    }
    if(Report.UnitExponent != 0){
        Count = (~Report.UnitExponent) + 1;
    }
    
    for(i = 0; i < Count; i++){
        Multi = Multi * 10;
    }        
    
    Resolution = ((INT16)Report.wLogicalMax - (INT16)Report.wLogicalMin) * Multi /
                 ((INT16)Report.PhysicalMax - (INT16)Report.PhysicalMin);


	reportsize = end - start;
    MinMask = ((~MinMask) >> reportsize) << reportsize;

	size =  reportsize /8;
	if((reportsize%8)!=0 )size++;


	ASSERT(size > 0 && size <= sizeof(tempdata));
    if ((size == 0) || (size > sizeof(tempdata))) {
        return;
    }

    MemCpy(&tempdata, fpBuffer + start / 8, size);

	preskip =  start % 8;
	postskip =  end % 8;
		
	if(preskip != 0)
		tempdata=tempdata >> preskip;
	if(postskip != 0)
	{
		tempdata=tempdata << postskip; 	
		tempdata=tempdata >> postskip; 			
	}	

    if(tempdata > Report.wLogicalMax)
        tempdata |= MinMask;

    if(MsData->FillUsage == 0x30){
        MsData->EfiX = tempdata;
        MsData->X = (UINT8)tempdata; 
    }
    if(MsData->FillUsage == 0x31){
        MsData->EfiY = tempdata;
        MsData->Y = (UINT8)tempdata; 
    } 

	return;
} 
										//<(EIP127014+)
//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
