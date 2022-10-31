//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Core/CORE_DXE/ConSplitter/Out.c 18    10/27/11 12:50p Felixp $
//
// $Revision: 18 $
//
// $Date: 10/27/11 12:50p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:        Out.c
//
// Description: File contains the Simple Text Output functionality for the 
//		Console Splitter Driver
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------------

#include "ConSplit.h"
#include <Protocol/GraphicsOutput.h>

//----------------------------------------------------------------------------

#define DefaultAttribute 0x0F
#define DefaultCursorVisible TRUE

SUPPORT_RES *SupportedModes = NULL;

SIMPLE_TEXT_OUTPUT_MODE	MasterMode =
	{
	1, 		// MaxMode
	0, 		// Current Mode
	0x0F,	// Attribute
	0, 		// Column
	0, 		// Row
	1  		// CursorVisible
	};


EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	mCSOutProtocol	=
	{
	CSReset,
	CSOutputString,
	CSTestString,
	CSQueryMode,
	CSSetMode,
	CSSetAttribute,
	CSClearScreen,
	CSSetCursorPosition,
	CSEnableCursor,
	&MasterMode
	};

//****************************** Virtual ConOut **********************//
CHAR16 *ScreenBuffer =  NULL;
CHAR16 *SaveScreenBuffer =  NULL;
CHAR16 *EndOfTheScreen = NULL;
INT32 *AttributeBuffer = NULL;
INT32 *SaveAttributeBuffer = NULL;
INT32 Columns = 0;
BOOLEAN BlankScreen = TRUE;

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ScrollScreen
//
// Description:
//  This function scrolls internal splitter buffer
//
// Input:   
//  VOID
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID ScrollScreen(
    VOID
)
{
	INT32 i;
	CHAR16 *pChar = EndOfTheScreen - Columns;
	INT32 *pAttr = AttributeBuffer + (pChar-ScreenBuffer);
	
	pBS->CopyMem(
		ScreenBuffer+Columns, ScreenBuffer,
		sizeof(CHAR16)* (EndOfTheScreen - pChar)
	);

	for(i=0; i<Columns; i++)
    { 
        *pChar++ = ' '; 
        *pAttr++ = MasterMode.Attribute;
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: MemOutputString
//
// Description:
//  This function puts string into internal splitter buffer
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN CHAR16 *String - pointer to string
//  OUT INT32 *RowOffset - pointer to store value of row offset after string output
//  OUT INT32 *LastColumn - pointer to store value of column after string output
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS MemOutputString(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN CHAR16                          *String,
    OUT INT32                          *RowOffset,
    OUT INT32                          *LastColumn
)
{
    INT32 RowOff = 0;
	INT32 Col = MasterMode.CursorColumn;
	UINTN Offset = MasterMode.CursorRow * Columns;
	CHAR16 *pChar = ScreenBuffer + Offset + Col;
	INT32 *pAttr =  AttributeBuffer + Offset + Col;

	BlankScreen = FALSE;


	while(TRUE){
		switch(*String){
		case 0: 
		return EFI_SUCCESS;
		case '\n': 
			if ((pChar + Columns) >= EndOfTheScreen) 
                ScrollScreen();
			else 
            {
                pChar += Columns; 
                pAttr += Columns;
                RowOff++;
            }
		break;
		case '\r':
			pChar -= Col; 
            pAttr -= Col; 
            Col = 0;
		break;
		case '\b':
			if (Col) 
            { 
                pChar--; 
                pAttr--; 
                Col--; 
            }
		break;
		default:
			if ((pChar + 1) == EndOfTheScreen)
            {
				ScrollScreen();
				pChar -= Col; 
                pAttr -= Col; 
                Col = 0;
			}
            else
            {
				*pChar++ = *String; 
                *pAttr++ = MasterMode.Attribute;
				Col++; 
				if (Col >= Columns) {
                    RowOff++;
                    Col = 0;
                }
			}
		break;
		}
		String++;
	}

    if(RowOffset)
        *RowOffset = RowOff;
    if(LastColumn)
        *LastColumn = Col;

	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: RestoreTextScreen
//
// Description:
//  This function restores screen after switching mode
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *pScreen - pointer to protocol instance
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID RestoreTextScreen(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *pScreen
)
{
	CHAR16 *pChar = ScreenBuffer, *pStr = ScreenBuffer;
	INT32 Col = 0, *pAttr = AttributeBuffer, Attr;
	CHAR16 c;

	if (BlankScreen) 
        return;

	Attr = *pAttr;
	pScreen->SetCursorPosition(pScreen,0,0);
	pScreen->SetAttribute(pScreen,Attr);

	for(; pChar<EndOfTheScreen; pChar++, pAttr++)
	{
		if (Col == Columns || *pAttr != Attr)
		{
			c = *pChar;
			*pChar = 0;
			pScreen->OutputString(pScreen, pStr);
			*pChar = c;
			pStr = pChar;
			if (*pAttr != Attr)
			{
				Attr = *pAttr;
				pScreen->SetAttribute(pScreen, Attr);
			}
			else
			{
				Col=1; 
                continue;
			}
		}
		Col++;
	}
	//print last row
	c = *--pChar;
	*pChar = 0;
	pScreen->OutputString(pScreen, pStr);
	*pChar = c;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: MemClearScreen
//
// Description:
//  This function clears internal splitter buffer
//
// Input:   
//  VOID
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS  MemClearScreen(
    VOID
)
{
	CHAR16  *pChar;
	INT32 *pAttr;
	for(  pChar = ScreenBuffer, pAttr =  AttributeBuffer
		; pChar < EndOfTheScreen
		; pChar++, pAttr++	)
    { 
        *pChar = ' '; 
        *pAttr = MasterMode.Attribute; 
    }
	BlankScreen = TRUE;
	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: MemReset
//
// Description:
//  This function resets internal splitter buffer
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN BOOLEAN EV - extended verification flag
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS MemReset(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This, 
    IN BOOLEAN                          EV
)
{
	return MemClearScreen();
}

///////////////// Uga Save/Restore
EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelBuffer = NULL;
UINT32 GraphicsMode, TextMode = 0;

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: RestoreUgaScreen
//
// Description:
//  This function restores screen after switching from text to graphics mode
//
// Input:   
//  IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This - pointer to protocol instance
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID RestoreUgaScreen(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
)
{
	if (PixelBuffer){
        EFI_STATUS Status;
        Status = Gop->SetMode(Gop, GraphicsMode);
        if (!EFI_ERROR(Status))
		    Gop->Blt(
                Gop, PixelBuffer, EfiBltBufferToVideo, 0, 0, 0, 0,
                Gop->Mode->Info->HorizontalResolution,
                Gop->Mode->Info->VerticalResolution, 0
            );
		pBS->FreePool(PixelBuffer); 
        PixelBuffer = NULL;
	}
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SaveUgaScreen
//
// Description:
//  This function saves graphics screen before switching to text mode
//
// Input:   
//  IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This - pointer to protocol instance
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID SaveUgaScreen(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop
)
{
    if (PixelBuffer) 
        pBS->FreePool(PixelBuffer);

    GraphicsMode=Gop->Mode->Mode;
	PixelBuffer = Malloc(
        Gop->Mode->Info->VerticalResolution*
        Gop->Mode->Info->HorizontalResolution*
        sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

	if (PixelBuffer)
    {
		EFI_STATUS Status=Gop->Blt(
            Gop, PixelBuffer, EfiBltVideoToBltBuffer, 0, 0, 0, 0,
            Gop->Mode->Info->HorizontalResolution,
            Gop->Mode->Info->VerticalResolution, 0);

		if (EFI_ERROR(Status))
        {
            pBS->FreePool(PixelBuffer); 
            PixelBuffer = NULL;
        }
	}
}

static EFI_GUID guidGop = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: RestoreTextScreenDelta
//
// Description:
//  This function checks to see if anything has changed on the screen since the 
//  graphics output device was disabled. If something has changed, then it will
//  add that to the display.
//
// Input:   
//      VOID
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID RestoreTextScreenDelta(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *pScreen
)
{
	UINT32 i;
	CHAR16 StrBuffer[2] = {0, 0};
	CONST UINT32 SizeOfScreen = (UINT32)((UINT8*)EndOfTheScreen-(UINT8*)ScreenBuffer) / sizeof(CHAR16);


	// Check for differences from the old state, one by one
	// If anything is different, update the screen with Simple Out protocol
	for(i = 0; i < SizeOfScreen; i++)
	{
		if( (*(SaveScreenBuffer+i) != *(ScreenBuffer + i)) ||
			(*(SaveAttributeBuffer+i) != *(AttributeBuffer + i)) )
		{
			StrBuffer[0] = *(ScreenBuffer + i);
			pScreen->SetCursorPosition(pScreen, i % Columns, i / Columns);
			pScreen->SetAttribute(pScreen, (UINT8)*(AttributeBuffer + i) );
			pScreen->OutputString(pScreen, StrBuffer );
		}
	}

}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: RestoreTheScreen
//
// Description:
//  This function restores screen of the output device after switching modes
//
// Input:   
//  IN EFI_HANDLE ControllerHandle - handle of output device to restore
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut - pointer to protocol instance
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID RestoreTheScreen(
    IN EFI_HANDLE                      ControllerHandle, 
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut
)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

	if (EFI_ERROR(pBS->HandleProtocol(ControllerHandle, &guidGop, (VOID**)&Gop)))
		RestoreTextScreen(SimpleOut);
	else 
	{
		RestoreUgaScreen(Gop);
		if(SaveScreenBuffer != NULL) 
		{
			RestoreTextScreenDelta(SimpleOut);
			pBS->FreePool(SaveScreenBuffer);
			SaveScreenBuffer = NULL;
			pBS->FreePool(SaveAttributeBuffer);
		}
	}
	SimpleOut->SetAttribute(SimpleOut,MasterMode.Attribute);
	SimpleOut->EnableCursor(SimpleOut,MasterMode.CursorVisible);
	SimpleOut->SetCursorPosition(SimpleOut,MasterMode.CursorColumn,MasterMode.CursorRow);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SaveTheScreen
//
// Description:
//  This function saves the screen of the output device before switching modes
//
// Input:   
//  IN EFI_HANDLE ControllerHandle - handle of output device to save
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut - pointer to protocol instance
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID SaveTheScreen(
    IN EFI_HANDLE                      ControllerHandle, 
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
	UINT32 SizeOfScreen, SizeOfAttribute;
	EFI_TPL     OldTpl;

	if (!EFI_ERROR(pBS->HandleProtocol(ControllerHandle, &guidGop, (VOID**)&Gop))) 
	{

		SizeOfScreen = (UINT32)((UINT8*)EndOfTheScreen-(UINT8*)ScreenBuffer);
		SizeOfAttribute = (UINT32)SizeOfScreen*(sizeof(INT32)/sizeof(CHAR16));

		// Allocate memory for saving the screen state, error out if not enough memory
        SaveScreenBuffer=Malloc(SizeOfScreen);
        if(SaveScreenBuffer==NULL) return;
        SaveAttributeBuffer=Malloc(SizeOfAttribute);
		if(SaveAttributeBuffer==NULL){
			pBS->FreePool(SaveScreenBuffer);
			SaveScreenBuffer = NULL;
            return;
		}
		// Make this high priority so that the saved text screen is exactly
		//  the same as graphics screen
		OldTpl = pBS->RaiseTPL(TPL_HIGH_LEVEL);

		// Perform using MemCpy (instead of pBS->MemCopy) to prevent TPL assert
		MemCpy(SaveScreenBuffer, ScreenBuffer, SizeOfScreen);
		MemCpy(SaveAttributeBuffer, AttributeBuffer, SizeOfAttribute);

		pBS->RestoreTPL(OldTpl);
		SaveUgaScreen(Gop);
	}
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SaveUgaMode
//
// Description:
//  This function saves the graphics mode of the output device before switching modes
//
// Input:   
//  IN EFI_HANDLE ControllerHandle - handle of output device to save
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID SaveUgaMode(
    IN EFI_HANDLE ControllerHandle
)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

	if (EFI_ERROR(pBS->HandleProtocol(ControllerHandle, &guidGop, (VOID**)&Gop))) 
        return;

	TextMode=Gop->Mode->Mode;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: RestoreUgaMode
//
// Description:
//  This function restores the graphics mode of the output device after switching modes
//
// Input:   
//  IN EFI_HANDLE ControllerHandle - handle of output device to restore
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID RestoreUgaMode(
    IN EFI_HANDLE ControllerHandle
)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

	if (EFI_ERROR(pBS->HandleProtocol(ControllerHandle, &guidGop, (VOID**)&Gop))) 
        return;

	if (TextMode!=Gop->Mode->Mode)
    {
        Gop->SetMode(Gop,TextMode);
    }
    else
    {//Just clear the screen
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL Buffer = {0,0,0,0};
 	    Gop->Blt( Gop, &Buffer, EfiBltVideoFill,0,0,0,0,
		    Gop->Mode->Info->HorizontalResolution,
		    Gop->Mode->Info->VerticalResolution, 
		    0
        );
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSReset
//
// Description:
//  This function resets the text output device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN BOOLEAN EV - extended verification flag
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - all devices, handled by splitter were reset successfully
//          EFI_DEVICE_ERROR - error occured during resetting the device
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS  CSReset(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN BOOLEAN                         EV
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);

	if (SimpleOut == NULL) {
        MasterMode.Attribute = EFI_BACKGROUND_BLACK | EFI_WHITE;
        MasterMode.CursorColumn = 0;
        MasterMode.CursorRow = 0;
        MemReset(This, EV);
		return EFI_DEVICE_ERROR;
    }

	// we need to loop through all the registered simple text out devices
	//	and call each of their Reset function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->Reset(SimpleOut->SimpleOut, EV);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}
	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);

	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	MemReset(This,EV);

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSOutputString
//
// Description:
//  This function writes a string to the output device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN CHAR16 *String - pointer to string to write
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
//          EFI_DEVICE_ERROR - error occured during output string
//          EFI_WARN_UNKNOWN_GLYPH - some of characters were skipped during output
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSOutputString(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN CHAR16                          *String
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
    INT32 RowOffset;
    INT32 LastColumn;

	MemOutputString(This, String, &RowOffset, &LastColumn);
	
	if (SimpleOut == NULL) {
        MasterMode.CursorColumn = LastColumn;
        MasterMode.CursorRow += RowOffset;
		return EFI_DEVICE_ERROR;
    }

	// we need to loop through all the registered simple text out devices
	//	and call each of their OutputString function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->OutputString(SimpleOut->SimpleOut, String);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSTestString
//
// Description:
//  This function tests whether all characters in String can be drawn on device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN CHAR16 *String - pointer to string to test
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - all characters can be drawn
//          EFI_UNSUPPORTED - there are characters that cannot be drawn
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS  CSTestString(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN CHAR16 *String)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	if (SimpleOut == NULL)
		return EFI_DEVICE_ERROR;

	// we need to loop through all the registered simple text out devices
	//	and call each of their TestString function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->TestString(SimpleOut->SimpleOut, String);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSQueryMode
//
// Description:
//  This function returns information about text mode referred by ModeNum
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN UINTN ModeNum - mode number to obtain information about
//  OUT UINTN *Col - max number of columns supported in this mode
//  OUT UINTN *Row - max number of rows supported in this mode
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
//          EFI_UNSUPPORTED - given mode unsupported
//          EFI_DEVICE_ERROR - error occured during execution
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSQueryMode(
	IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
	IN  UINTN                           ModeNum,
	OUT UINTN                           *Col,
	OUT UINTN                           *Row
)
{
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	if((ModeNum >= (UINTN)MasterMode.MaxMode) || (!SupportedModes[ModeNum].AllDevices)) {

        return EFI_UNSUPPORTED;
	}
		

	if (SimpleOut == NULL)
	{
		// since we use a default text mode, return that value
		*Col = 80;
		*Row = 25;
		return EFI_DEVICE_ERROR;
	}

	*Col = SupportedModes[ModeNum].Columns;
	*Row = SupportedModes[ModeNum].Rows;

	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSSetMode
//
// Description:
//  This function sets text mode referred by ModeNumber
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN UINTN ModeNum - mode number to set
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
//          EFI_UNSUPPORTED - given mode unsupported
//          EFI_DEVICE_ERROR - error occured during execution
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSSetMode(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN UINTN                           ModeNum
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	INT32 DeviceMode;

	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);

	if((ModeNum >= (UINTN)MasterMode.MaxMode) || (!SupportedModes[ModeNum].AllDevices)) 
		return EFI_UNSUPPORTED;

	if (SimpleOut == NULL)
		return EFI_DEVICE_ERROR;

	TestStatus = ResizeSplitterBuffer((INT32)ModeNum);
	if (EFI_ERROR(TestStatus))
		return TestStatus;

	MasterMode.Mode = (INT32)ModeNum;
	// we need to loop through all the registered simple text out devices
	//	and call each of their SetMode function
	while ( SimpleOut != NULL)
	{
		TestStatus = IsModeSupported(SimpleOut->SimpleOut, ModeNum, &DeviceMode);

		TestStatus = SimpleOut->SimpleOut->SetMode(SimpleOut->SimpleOut, DeviceMode);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	return EFI_ERROR(Status);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSSetAttribute
//
// Description:
//  This function sets the foreground color and background color for the screen
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN UINTN Attribute - attribute to set
//
// Output:
//      EFI_STATUS
//		    EFI_SUCCESS - attribute was changed successfully
//		    EFI_DEVICE_ERROR - device had an error
//		    EFI_UNSUPPORTED - attribute is not supported
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS  CSSetAttribute(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN UINTN                           Attribute
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	if (SimpleOut == NULL) {
        MasterMode.Attribute = (INT32)Attribute;
		return EFI_DEVICE_ERROR;
    }

	// we need to loop through all the registered simple text out devices
	//	and call each of their SetAttribute function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->SetAttribute(SimpleOut->SimpleOut, Attribute);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSClearScreen
//
// Description:
//  This function clears screen of output device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
//          EFI_DEVICE_ERROR - error occured during execution
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS  CSClearScreen(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	if (SimpleOut == NULL) {
        MasterMode.CursorColumn = 0;
        MasterMode.CursorRow = 0;
        MemClearScreen();
		return EFI_DEVICE_ERROR;
    }

	// we need to loop through all the registered simple text out devices
	//	and call each of their ClearScreen function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->ClearScreen(SimpleOut->SimpleOut);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	MemClearScreen();
	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSSetCursorPosition
//
// Description:
//  This function sets cursor position of output device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN UINTN Column - column position
//  IN UINTN Row - row position
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
//          EFI_DEVICE_ERROR - error occured during execution
//          EFI_UNSUPPORTED - given position cannot be set
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSSetCursorPosition(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN UINTN                           Column,
	IN UINTN                           Row
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	if (SimpleOut == NULL) {
        if(Column < 80 && Row < 25) {
            MasterMode.CursorColumn = (INT32)Column;
            MasterMode.CursorRow = (INT32)Row;
        }
		return EFI_DEVICE_ERROR;
    }

	// we need to loop through all the registered simple text out devices
	//	and call each of their SetCursorPosition function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->SetCursorPosition(SimpleOut->SimpleOut, Column, Row);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	UpdateMasterMode(SimpleOut->SimpleOut->Mode);

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSEnableCursor
//
// Description:
//  This function enables / disables cursor on the screen
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol instance
//  IN BOOLEAN Visible - if TRUE cursor will be visible, FALSE - not visible
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
//          EFI_DEVICE_ERROR - error occured during execution
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS  CSEnableCursor(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN BOOLEAN Visible	)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	if (SimpleOut == NULL)
		return EFI_DEVICE_ERROR;

	// we need to loop through all the registered simple text out devices
	//	and call each of their SetCursorPosition function
	while ( SimpleOut != NULL)
	{
		TestStatus = SimpleOut->SimpleOut->EnableCursor(SimpleOut->SimpleOut, Visible);
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

    MasterMode.CursorVisible = Visible;
	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: UpdateMasterMode
//
// Description:
//  This function updates splitter mode to values referred by Mode
//
// Input:   
//  IN SIMPLE_TEXT_OUTPUT_MODE *Mode - pointer to values to be set
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID UpdateMasterMode(
    SIMPLE_TEXT_OUTPUT_MODE *Mode)
{
	MasterMode.Attribute = Mode->Attribute;
	MasterMode.CursorColumn = Mode->CursorColumn;
	MasterMode.CursorRow = Mode->CursorRow;
    while(MasterMode.CursorColumn>=SupportedModes[MasterMode.Mode].Columns){
        MasterMode.CursorColumn-=SupportedModes[MasterMode.Mode].Columns;
        MasterMode.CursorRow++;
    };
    if (MasterMode.CursorRow>=SupportedModes[MasterMode.Mode].Rows)
        MasterMode.CursorRow=SupportedModes[MasterMode.Mode].Rows-1;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
