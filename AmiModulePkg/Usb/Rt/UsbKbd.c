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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usbkbd.c 49    8/07/12 4:47a Roberthsu $
//
// $Revision: 49 $
//
// $Date: 8/07/12 4:47a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           UsbKbd.c
//
//  Description:    USB keyboard driver SMI routines
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"
#include "UsbKbd.h"
#include <UsbDevDriverElinks.h>         //(EIP90887+)

extern USB_GLOBAL_DATA *gUsbData;
extern BOOLEAN KeyRepeatStatus;
extern UINT16 gKbcSetTypeRate11CharsSec;
extern UINT16 gKbcSetTypeDelay500MSec;
UINT8   gLastKeyCodeArray[8]={0,0,0,0,0,0,0,0};

//----------------------------------------------------------------------------
// Typematic rate delay table will have counts to generate key repeat delays.
// Since the periodic interrupt is set to 8msec the following repeat times
// will generate necessary delay.
// First three numbers are meant to define the frequency of the repeated keys;
// four other numbers are used to define the amount of delay between the first
// keypress-and-hold til the key actually starts repeating; the appropriate values
// of this table are selected using the equates defined in UsbKbd.h
//
UINT8   aTypematicRateDelayTable[]      =   {2, 4, 8, 16, 32, 48, 64, 96};

//
// The global data variables are stored in USB_GLOBAL_DATA structure and can be accessed through
// gUsbData->xxx
//

LEGACY_USB_KEYBOARD mLegacyKeyboard;

extern UINT8 IsKbcAccessBlocked;		//(EIP29733+)
extern  VOID USBKB_LEDOn();

BOOLEAN     gEfiMakeCodeGenerated=FALSE;
BOOLEAN     gLegacyMakeCodeGenerated=FALSE;

extern EFI_EMUL6064KBDINPUT_PROTOCOL* gKbdInput ;

                                        //(EIP90887+)>
typedef	BOOLEAN (KBD_BUFFER_CHECK_FUNCTIONS)( 
    DEV_INFO	*DevInfo,
    UINT8       *Buffer
	);
extern KBD_BUFFER_CHECK_FUNCTIONS KBD_BUFFER_CHECK_ELINK_LIST EndOfInitList;
KBD_BUFFER_CHECK_FUNCTIONS* KbdBufferCheckFunctionsList[] = {KBD_BUFFER_CHECK_ELINK_LIST NULL};
                                        //<(EIP90887+)

UINT8	UsbControlTransfer(HC_STRUC*, DEV_INFO*, DEV_REQ, UINT16, VOID*);
UINT8	UsbKbdDataHandler(DEV_INFO*, UINT8*);

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBKBDInitialize (VOID)
//
// Description: This function returns fills the host controller driver
//              routine pointers in the structure provided
//
// Input:       Nothing
//
// Output:      Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBKBDInitialize (VOID)
{
    UINT8       bTemp;

    //
    // Initialize the typematic rate to 500 ms, 10.9 Char/Sec and auto repeat flag
    // to disabled
    //
    gUsbData->wUSBKBC_StatusFlag |= ((gKbcSetTypeRate11CharsSec << KBC_TYPE_RATE_BIT_SHIFT) +
                                        (gKbcSetTypeDelay500MSec << KBC_TYPE_DELAY_BIT_SHIFT));

    USB_DEBUG (DEBUG_LEVEL_5, "USBKBDInitialize:  CodeBufferStart : %lx\n", gUsbData->aKBCScanCodeBufferStart);

    //
    // Initialize the scanner buffer
    //
    gUsbData->fpKBCScanCodeBufferPtr       = gUsbData->aKBCScanCodeBufferStart;
    gUsbData->bLastUSBKeyCode              = 0;

    //
    // Initialize the character buffer
    //
    gUsbData->fpKBCCharacterBufferHead     = gUsbData->aKBCCharacterBufferStart;
    gUsbData->fpKBCCharacterBufferTail     = gUsbData->aKBCCharacterBufferStart;

    gUsbData->fpKeyRepeatDevInfo=NULL;

    //
    // Set scan code set to 2 in the scanner flag
    //
    gUsbData->wUSBKBC_StatusFlag           |= KBC_SET_SCAN_CODE_SET2;

    gUsbData->bUSBKBShiftKeyStatus = 0;

    //
    // Get the keyboard controller command byte (CCB) and store it locally
    //
    //USBKBC_GetAndStoreCCB();
    gUsbData->bCCB = 0x40;

    for (bTemp=0; bTemp<6; bTemp++) mLegacyKeyboard.KeyCodeStorage[bTemp] = 0;
    mLegacyKeyboard.KeyToRepeat = 0;

    return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBKBDConfigureKeyboard
//
// Description: This routine checks an interface descriptor of the USB device
//      detected to see if it describes a HID/Boot/Keyboard device.
//      If the device matches the above criteria, then the device is
//      configured and initialized
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
                                        //(EIP84455+)>
DEV_INFO*
USBKBDConfigureDevice (
    DEV_INFO*   fpDevInfo
)
{
    UINT16          wIndex;

    fpDevInfo->fpPollTDPtr  = 0;                                      
    wIndex = USBKBDFindUSBKBDeviceTableEntry(fpDevInfo);
    if (wIndex == 0xFFFF) {
        wIndex  = USBKBDFindUSBKBDeviceTableEntry(NULL);
    }
    if(wIndex != 0xFFFF)
    {
        gUsbData->aUSBKBDeviceTable[wIndex]    = fpDevInfo;
        if  (!(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)) {
            if(BOOT_PROTOCOL_SUPPORT || (fpDevInfo->Hidreport.bFlag & HID_REPORT_BFLAG_LED_FLAG))
                USBKB_LEDOn();
        }
    } 
    else    return 0;
    
    return fpDevInfo;
}
                                        //<(EIP84455+)
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBKBDFindUSBKBDeviceTableEntry
//
// Description: This routine searches for the HID table entry which matches
//              the provided device info structure
//
// Input:       fpDevInfo   Pointer to DeviceInfo structure
//
// Output:      offset of the HID table for the requested fpDevinfo
//              0xFFFF -on error
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBKBDFindUSBKBDeviceTableEntry(DEV_INFO* fpDevinfo)
{
    UINT16      wCount ;

    for (wCount = 0; wCount < USB_DEV_HID_COUNT; wCount++)
    {
        if(gUsbData->aUSBKBDeviceTable[wCount] == fpDevinfo )
            return wCount;
    }
    USB_DEBUG (DEBUG_LEVEL_3, "No Free KBD DevInfo Entry\n");
    return 0xFFFF;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBKBDDisconnectDevice
//
// Description: This routine disconnects the keyboard by freeing
//              the USB keyboard device table entry
//
// Input:       fpDevInfo   Pointer to DeviceInfo structure
//
// Output:      USB_SUCCESS/USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBKBDDisconnectDevice (DEV_INFO*   fpDevInfo)
{
    UINT16      wIndex;
                                        //(EIP93637+)>
    UINT8       ScanCodeCount = (UINT8)(gUsbData->fpKBCScanCodeBufferPtr - 
                         (UINT8*)gUsbData->aKBCScanCodeBufferStart);   //(EIP102150+) 
    UINT8       i,CurrentDeviceID,Key;  //(EIP102150+)

    wIndex = USBKBDFindUSBKBDeviceTableEntry(fpDevInfo);
    if (wIndex == 0xFFFF) {
        USBLogError(USB_ERR_KBCONNECT_FAILED);
        return USB_ERROR;
    } else {

                                        //(EIP102150+)>
        CurrentDeviceID = (UINT8)(1 << ((fpDevInfo->bDeviceAddress) -1));
        for (i = 0; i < ScanCodeCount; i++) {
            if (gUsbData->aKBCDeviceIDBufferStart[i] & CurrentDeviceID) {
                gUsbData->aKBCDeviceIDBufferStart[i] &= ~CurrentDeviceID;
                if(gUsbData->aKBCDeviceIDBufferStart[i] == 0){
                    Key = gUsbData->aKBCScanCodeBufferStart[i]; 
                    if((Key == 0xe5)||(Key == 0xe1)){
                        gUsbData->bUSBKBShiftKeyStatus &= ~(KB_RSHIFT_KEY_BIT_MASK+KB_LSHIFT_KEY_BIT_MASK);
                    }
                    USBKB_DiscardCharacter(&gUsbData->aKBCShiftKeyStatusBufferStart[i]); 
                    gUsbData->fpKBCScanCodeBufferPtr--;
                }
            }
        }  
 
        gUsbData->aUSBKBDeviceTable[wIndex] = 0;
        return USB_SUCCESS;
    }
                                        //<(EIP93637+)
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBKBDProcessKeyboardData
//
// Description: This routine is called with USB keyboard report data.  This
//              routine handles the translation of  USB keyboard data
//              into PS/2 keyboard data, and makes the PS/2 data available
//              to software using ports 60/64h by communicating with
//              PS/2 keyboard controller.
//
// Input:       fpHCStruc   Pointer to HCStruc
//              fpDevInfo   Pointer to device information structure
//              fpTD        Pointer to the polling TD
//              fpBuffer    Pointer to the data buffer
//
// Output:      Nothing
//
// Notes:   TD's control status field has the packet length (0 based).
//      It could be one of three values 0,1 or 7 indicating packet
//      lengths of 1, 2 & 8 repectively.
//      The format of 8 byte data packet is as follow:
//           Byte              Description
//      -----------------------------------------------------------
//          0   Modifier key (like shift, cntr & LED status)
//          1   Reserved
//          2   Keycode of 1st key pressed
//          3   Keycode of 2nd key pressed
//          4   Keycode of 3rd key pressed
//          5   Keycode of 4th key pressed
//          6   Keycode of 5th key pressed
//          7   Keycode of 6th key pressed
//      -----------------------------------------------------------
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBKBDProcessKeyboardData (
    HC_STRUC    *fpHcStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       *fpTD,
    UINT8       *fpBuffer
)
{
    UINT8   wCount=8;
    UINT8   i,j,k=0,l,m,n=0;
	UINT8	usage_array[20] = {0};

	UINT32	kb_usage_end = 0,kb_usage_start = 0,temp = 0,size ;                 //EIP95349
	UINT8	temp_offset = 0, test = 0;
	HIDReport_STRUC 	*Hidreport;
	UINT8	*usage_temp;
	UINT8 	offset_tmp=0,start_offset=0;
    UINT8   valid_data=0;               //EIP67400 
    UINT8   CountEnd,CountStart;		//(EIP107262)
    UINTN   OemHookIndex;
 
	Hidreport = &(fpDevInfo->Hidreport);

	if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_PROTOCOL) 
	{
        
		//serach modify key and keycode
		for(i=0;i<Hidreport->bTotalCount;i++)
		{
			//Check is input?
			if((Hidreport->pReport[i].bFlag & HID_BFLAG_INPUT ))
			{
    			//if report id exist, check first byte
    			if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID) {
    				if(Hidreport->pReport[i].bReportID != *fpBuffer) {
                        continue;
    				}
    			}
                
                                        //(EIP105587+)>
                if(Hidreport->pReport[i].bFlag & HID_BFLAG_DATA_BIT){
    			    offset_tmp+=(Hidreport->pReport[i].bReportCount*Hidreport->pReport[i].bReportSize);                     
                    continue;
                }
                                        //<(EIP105587+)
                //find start offset
    			if(Hidreport->pReport[i].bUsagePage ==0x7)
    			{
                    valid_data = 1;
    			    usage_temp = USB_MemAlloc(GET_MEM_BLK_COUNT(Hidreport->pReport[i].bReportCount * Hidreport->pReport[i].bReportSize)); 
                    ASSERT(usage_temp);
                    if (!usage_temp) {
                        return USB_ERROR;
                    }
                    kb_usage_start = 0;
	    			if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID){
                        kb_usage_start += 8;
	    			}
                    kb_usage_start += offset_tmp;
                    kb_usage_end = Hidreport->pReport[i].bReportCount * Hidreport->pReport[i].bReportSize + kb_usage_start;
                    size = (kb_usage_end - kb_usage_start) / 8;
                    if((kb_usage_end - kb_usage_start) % 8 != 0)
                        size++;

					if((Hidreport->pReport[i].bFlag & 0x2 )== 0){        //array
					    for(l =0; l< size;l++)
                        {
                            if (*(fpBuffer+(kb_usage_start / 8)+l) !=0) {
    					        usage_array[k++] = *(fpBuffer + (kb_usage_start / 8) + l);
                            }
                        }
    				}
					else{
                        //fill usage min until max into usage temp
                        for(temp=0,m=0;m <Hidreport->pReport[i].bUsageMaxCount;m++){	//(EIP107262)
        					for(j = (UINT8)Hidreport->pReport[i].wUsageMin[m]; j <= (UINT8)Hidreport->pReport[i].wUsageMax[m] ; j++){
        						usage_temp[temp++] = j;
        					}
                        }
                        for(n=0;n< Hidreport->pReport[i].bUsageCount;n++)
                            usage_temp[temp++]=Hidreport->pReport[i].bUsage[n];
                        
                                        //(EIP107262+)>
    					for(l = 0,j = 0; l < size ; l++){
    						temp = *(fpBuffer + (kb_usage_start / 8) + l);
                            CountStart = 0;
                            CountEnd   = 8;
                            if(kb_usage_start % 8 && (l == 0) )
                                CountStart = kb_usage_start % 8;
                            if((kb_usage_end % 8) &&(l == size-1))
                                CountEnd = kb_usage_end % 8;
                            
    						for( ; CountStart < CountEnd ; CountStart++,j++){
    							if((temp >> CountStart) & 1){
    								usage_array[k++] = usage_temp[j];
    							}
    						}
    					}
                                        //<(EIP107262+)
    				}
                    USB_MemFree(usage_temp, GET_MEM_BLK_COUNT(Hidreport->pReport[i].bReportCount * Hidreport->pReport[i].bReportSize));

                }
    			offset_tmp+=(Hidreport->pReport[i].bReportCount*Hidreport->pReport[i].bReportSize);
			}
		}

        MemSet(fpBuffer, 8, 0);

        for (l = 0, i=0 ; l < 20 ; l++) {
            if (usage_array[l] >= 0xE0 && usage_array[l] <= 0xE7){
            	*fpBuffer |= 1 << (usage_array[l] - 0xE0);
            } else {
            	*(fpBuffer + i + 2) = usage_array[l];
            	i++;
            }
        }
    	if(!valid_data)return USB_SUCCESS;                
	}

                                        //(EIP90887+)>
    // Call all the OEM hooks that wants to check KBD buffer    
    for (OemHookIndex = 0; KbdBufferCheckFunctionsList[OemHookIndex]; OemHookIndex++) {
        if (KbdBufferCheckFunctionsList[OemHookIndex](fpDevInfo, fpBuffer)) {
            return USB_SUCCESS;
		}
	}
	                                    //<(EIP90887+)	
	//Is KBC access allowed?
	if (IsKbcAccessBlocked) {
		if (!(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) || !gEfiMakeCodeGenerated) {
			return USB_SUCCESS;
		}
		MemSet(fpBuffer, 8, 0);
	}

    //
    // Save the device info pointer for later use
    //
    gUsbData->fpSavedHCStruc   = fpHcStruc;
    gUsbData->fpKeyRepeatDevInfo = fpDevInfo;

    for (i = 0, wCount = 8; i < 8; i++, wCount--) {
		if (fpBuffer[i]) {
			break;
		}
    }

    if((gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)) {
        if(wCount==0) {
            gEfiMakeCodeGenerated=FALSE;
        } else {
            gEfiMakeCodeGenerated=TRUE;
            gLegacyMakeCodeGenerated=FALSE;
        }
    } else {
        if(wCount==0) {
            gLegacyMakeCodeGenerated=FALSE;
        } else {
            gLegacyMakeCodeGenerated=TRUE;
            gEfiMakeCodeGenerated=FALSE;
        }
    }

    //
    // checks for new key stroke.
    // if no new key got, return immediately.
    //
    for (i = 0; i < 8; i ++) {
        if(fpBuffer[i] != gLastKeyCodeArray[i]) {
            break;
        }
    }

#if USB_HID_KEYREPEAT_USE_SETIDLE == 1 
    if((i == 8) && KeyRepeatStatus) {
        USBKBDPeriodicInterruptHandler(fpHcStruc);
        return USB_SUCCESS;
    }
#endif

    //
    // Update LastKeycodeArray[] buffer in the
    // Usb Keyboard Device data structure.
    //
    for (i = 0; i < 8; i ++) {
        gLastKeyCodeArray[i] = fpBuffer[i];
    }

    if ((!(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) || gLegacyMakeCodeGenerated) &&(!gEfiMakeCodeGenerated))
    {
        if(wCount==0) {
            gLegacyMakeCodeGenerated=FALSE;
        }
        UsbScanner(fpDevInfo, fpBuffer);
    } else {

        if(wCount==0) {
            gEfiMakeCodeGenerated=FALSE;
        }

		UsbKbdDataHandler(fpDevInfo, fpBuffer);
    }

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBKBDPeriodicInterruptHandler
//
// Description: This routine is called every 16ms and is used to send
//              the characters read from USB keyboard to the keyboard
//              controller for legacy operation. Also this function updates
//              the keyboard LED status
//
// Input:       fpHCStruc   Pointer to the HCStruc structure
//
// Output:      Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBKBDPeriodicInterruptHandler (HC_STRUC* fpHcStruc)
{
    if (!(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)) {
        LegacyAutoRepeat(fpHcStruc);
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbScanner
//
// Description: This routine is executed to convert USB scan codes into PS/2
//              make/bread codes.
//
// Input:       fpDevInfo   - USB keyboard device
//              fpBuffer    - USB scan codes data buffer
//
// Output:      Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbScanner(
    DEV_INFO *fpDevInfo,
    UINT8 *fpBuffer
)
{
    if(gUsbData->kbc_support || ((gUsbData->dUSBStateFlag & USB_FLAG_6064EMULATION_ON) 
        && (gUsbData->dUSBStateFlag & USB_FLAG_6064EMULATION_IRQ_SUPPORT))) {
        USBKBC_GetAndStoreCCB();
        USBKB_Scanner (fpDevInfo, fpBuffer);
        USBKB_UpdateLEDState (0xFF);
    }else {
        USBKB_Int9(fpBuffer);
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        LegacyAutoRepeat
//
// Description: This routine is called periodically based on 8ms TD and used
//              to implement the key repeat.
//
// Input:       Hc   Pointer to the HCStruc structure
//
// Output:      Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
LegacyAutoRepeat(
    HC_STRUC *Hc
)
{
    if(gUsbData->kbc_support || ((gUsbData->dUSBStateFlag & USB_FLAG_6064EMULATION_ON) 
        && (gUsbData->dUSBStateFlag & USB_FLAG_6064EMULATION_IRQ_SUPPORT))) {
        SysKbcAutoRepeat(Hc);
    } else {
        SysNoKbcAutoRepeat();
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbKbdSetLed
//
// Description: This routine set the USB keyboard LED status.
//
// Input:       DevInfo		Pointer to device information structure
//				LedStatus	LED status
//
// Output:      USB_SUCCESS or USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbKbdSetLed (
	DEV_INFO    *DevInfo,
	UINT8		LedStatus
)
{
	HC_STRUC	*HcStruc = gUsbData->HcTable[DevInfo->bHCNumber - 1];
	DEV_REQ		DevReq;
	UINT8		Status;
	UINT8		ReportType;
	UINT8		ReportId;
	UINT16		ReportLen;
	UINT8		*ReportData = NULL;
	UINT8		Index;

	if (BOOT_PROTOCOL_SUPPORT == 0 && 
		!(DevInfo->Hidreport.bFlag & HID_REPORT_BFLAG_LED_FLAG)) {
		return USB_ERROR;
	}

	ReportData = USB_MemAlloc(GET_MEM_BLK_COUNT(4));
	if (ReportData == NULL) return USB_ERROR;

	
	ReportType = 0x02;		// Output
	ReportId = 0;
	ReportLen = 1;
	ReportData[0] = LedStatus & 0x7;

	if (DevInfo->Hidreport.bFlag & HID_REPORT_BFLAG_REPORT_PROTOCOL) { 
		for (Index = 0; Index < DevInfo->Hidreport.bTotalCount; Index++) {
			//find start offset
			if ((DevInfo->Hidreport.pReport[Index].bUsagePage == 0x8) &&
				(DevInfo->Hidreport.bFlag & HID_REPORT_BFLAG_REPORT_ID) &&
				(DevInfo->Hidreport.pReport[Index].wUsageMin[0] == 1)) {
				ReportId |= DevInfo->Hidreport.pReport[Index].bReportID;
				ReportData[1] = ReportData[0];
				ReportData[0] = ReportId;
				ReportLen = 2;
			}
		}
	}

	DevReq.wRequestType = HID_RQ_SET_REPORT;
	DevReq.wValue = (ReportType << 8) | ReportId;
	DevReq.wIndex = DevInfo->bInterfaceNum;
	DevReq.wDataLength = ReportLen;

	Status = UsbControlTransfer(HcStruc, DevInfo, DevReq, 100, ReportData);

	USB_MemFree(ReportData, GET_MEM_BLK_COUNT(4));
	return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        InsertKey
//
// Description: Insert a key to key buffer.
//
// Input:       KeyBuffer	- Pointer to the key buffer
//				Key			- The key to be inserted
//
// Output:      None
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
InsertKey (
	KEY_BUFFER	*KeyBuffer,
	VOID		*Key
)
{
	if (KeyBuffer == NULL || Key == NULL) return;	

	//EfiCopyMem(KeyBuffer->Buffer[KeyBuffer->Head++], Key, KeyBuffer->KeySize);
	MemCpy(KeyBuffer->Buffer[KeyBuffer->Head], Key, KeyBuffer->KeySize);

	KeyBuffer->Head = ++KeyBuffer->Head % KeyBuffer->MaxKey;

    if (KeyBuffer->Head == KeyBuffer->Tail){
        //Drop data from buffer
		KeyBuffer->Tail = ++KeyBuffer->Tail % KeyBuffer->MaxKey;
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        FindKey
//
// Description: Find the specific key in key buffer.
//
// Input:       KeyBuffer	- Pointer to the key buffer
//				Key			- The key to be inserted
//
// Output:      None
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID*
FindKey (
	KEY_BUFFER	*KeyBuffer,
	VOID		*Key
)
{
	UINT8	Index = 0;

	if (KeyBuffer == NULL || Key == NULL) return NULL;

	for (Index = KeyBuffer->Tail; 
		Index != KeyBuffer->Head; 
		Index = ++Index % KeyBuffer->MaxKey) {
		if (MemCmp(KeyBuffer->Buffer[Index], Key, KeyBuffer->KeySize) == 0) {
			return KeyBuffer->Buffer[Index];
		}
	}

	return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetNextKey
//
// Description: Get the next available key in the buffer
//
// Input:       KeyBuffer	- Pointer to the key buffer
//				Key			- The key to be inserted
//
// Output:      None
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID*
GetNextKey (
	KEY_BUFFER	*KeyBuffer,
	VOID		*Key
)
{
	UINT8	Index = 0;
	BOOLEAN	KeyFound = FALSE;

	if (KeyBuffer == NULL) return NULL;

	for (Index = KeyBuffer->Tail; 
		Index != KeyBuffer->Head; 
		Index = ++Index % KeyBuffer->MaxKey) {

		if (Key == NULL || KeyFound) {
			return KeyBuffer->Buffer[Index];
		}
		if (KeyBuffer->Buffer[Index] == Key) {
			KeyFound = TRUE;
		}
	}

	// If the key is not available, return the first available key
	if (!KeyFound && (KeyBuffer->Tail != KeyBuffer->Head)) {
		return KeyBuffer->Buffer[KeyBuffer->Tail];
	}

	return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        RemoveKey
//
// Description: Remove the key in key buffer.
//
// Input:       KeyBuffer	- Pointer to the key buffer
//				Key			- The key to be inserted
//
// Output:      None
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
RemoveKey (
	KEY_BUFFER	*KeyBuffer,
	VOID		*Key
)
{
	UINT8	Index = 0;
	UINT8	NextIndex = 0;

	if (KeyBuffer == NULL || Key == NULL) return;

	for (Index = KeyBuffer->Tail; 
		Index != KeyBuffer->Head; 
		Index = ++Index % KeyBuffer->MaxKey) {
		if (KeyBuffer->Buffer[Index] == Key) {
			break;
		}
	}
	
	if (Index == KeyBuffer->Head) {
		return;
	}

	for (; Index != KeyBuffer->Tail; 
		Index = (--Index + KeyBuffer->MaxKey) % KeyBuffer->MaxKey) {
		NextIndex = ((Index - 1) + KeyBuffer->MaxKey) % KeyBuffer->MaxKey;
		MemCpy(KeyBuffer->Buffer[Index],
				KeyBuffer->Buffer[NextIndex], KeyBuffer->KeySize);
	}

	KeyBuffer->Tail = ++KeyBuffer->Tail % KeyBuffer->MaxKey;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        KeypressCallback
//
// Description: This routine is called every time the keyboard data is updated
//
// Input:      HcStruc, DevInfo, fpBuf2 always NULL
//             Buf1  - Pointer to 8 bytes USB data array
//
// Output:      USB_SUCCESS or USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbKbdDataHandler (
    DEV_INFO    *DevInfo,
    UINT8       *Data
)
{
	UINT8		KeyCode;
	UINT8		Modifier;
	UINT8		WorkSpace[16] = {0};
	UINT8		WorkSpaceIndex = 0;
	UINT8		Index = 0;
	VOID		*Key = NULL;
	USB_KEY		UsbKey = {0};

	// Translate the modifier to USB key code
	for (KeyCode = 0xE0, Modifier = Data[0];
		KeyCode <= 0xE7;
		KeyCode++, Modifier >>= 1) {
		if (Modifier & BIT0) {
			WorkSpace[WorkSpaceIndex++] = KeyCode;
		}
	}

	for (Index = 2; Index < 8 && Data[Index]; Index++) {
		// Check if the input overrun
		if (Data[Index] == 1) {
			UsbKey.KeyCode = Data[Index];
			UsbKey.Press = TRUE;
			InsertKey((KEY_BUFFER*)DevInfo->UsbKeyBuffer, &UsbKey);
			return USB_SUCCESS;
		}
		WorkSpace[WorkSpaceIndex++] = Data[Index];
	}

	while (Key = GetNextKey(DevInfo->KeyCodeBuffer, Key)) {
		KeyCode = *(UINT8*)Key;
		for (Index = 0; Index < WorkSpaceIndex; Index++) {
			if (KeyCode == WorkSpace[Index]) {
				break;
			}
		}

		if (Index == WorkSpaceIndex) {
			// The key in key code buffer is released
			RemoveKey((KEY_BUFFER*)DevInfo->KeyCodeBuffer, Key);

			UsbKey.KeyCode = KeyCode;
			UsbKey.Press = FALSE;
			InsertKey((KEY_BUFFER*)DevInfo->UsbKeyBuffer, &UsbKey);
		}
	}

	// Check if the key is in the key code buffer
	for (Index = 0; Index < WorkSpaceIndex && WorkSpace[Index]; Index++) {
		if (!FindKey(DevInfo->KeyCodeBuffer, &WorkSpace[Index])) {
			// A new key pressed, insert the key code buffer
			InsertKey((KEY_BUFFER*)DevInfo->KeyCodeBuffer, &WorkSpace[Index]);

			UsbKey.KeyCode = WorkSpace[Index];
			UsbKey.Press = TRUE;
			InsertKey((KEY_BUFFER*)DevInfo->UsbKeyBuffer, &UsbKey);
		}
	}

	return USB_SUCCESS;
}


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
