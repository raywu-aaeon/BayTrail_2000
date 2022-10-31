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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  GraphicsConsole.h
//
// Description:  Graphics console driver that produces the Simple Text Out
//		interface
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _GRAPHCICS_CONSOLE_H_
#define _GRAPHCICS_CONSOLE_H_

#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Guid/MdeModuleHii.h>

#include <AmiDxeLib.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiFont.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/HiiDatabase.h>
#include <Token.h>

//-----------------------------------------------------------------------------

#define	NARROW_GLYPH_WIDTH	EFI_GLYPH_WIDTH
#define WIDE_GLYPH_WIDTH	NARROW_GLYPH_WIDTH * 2

#define MODE_ZERO_MIN_HOR_RES NARROW_GLYPH_WIDTH * 80
#define MODE_ZERO_MIN_VER_RES EFI_GLYPH_HEIGHT * 25

#define CURSOR_THICKNESS	3
#define CURSOR_OFFSET   	15      //base line of simple narrow font

#define MAX_RES             0
#define MIN_RES             -1

#define	NULL_CHAR			0x0000
#define	BACKSPACE			0x0008
#define	LINE_FEED			0x000A
#define	CARRIAGE_RETURN		0x000D

//-----------------------------------------------------------------------------
// Data Structures

#pragma pack(1)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name: TEXT_MODE
//
// Description: 
// This structure represents text mode internal information structure
//
// Fields: Name          Type                    Description
//----------------------------------------------------------------------------
// ModeNum              INT32               Mode number
// Col                  INT32               Max number of columns
// Row                  INT32               Max number of rows
// VideoCol             UINT32              Horizontal screen resolution
// VideoRow             UINT32              Vertical screen resolution
// 
// Notes:  
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _TEXT_MODE  {
	INT32	ModeNum;
	INT32	Col;
	INT32	Row;
	UINT32	VideoCol; // horizontal pixels
	UINT32	VideoRow; // vertical pixels
} TEXT_MODE;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name: GC_TEXT_MODE
//
// Description: 
// This structure represents text mode extended internal information structure
//
// Fields: Name          Type                    Description
//----------------------------------------------------------------------------
// ModeNum              INT32               Mode number
// Col                  INT32               Max number of columns
// Row                  INT32               Max number of rows
// VideoCol             UINT32              Horizontal screen resolution
// VideoRow             UINT32              Vertical screen resolution
// Supported            BOOLEAN             Flag if this mode supported
// GraphicsMode         UINT32              Correspondent graphics mode
// 
// Notes:  
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _GC_TEXT_MODE  {
	INT32	ModeNum;
	INT32	Col;
	INT32	Row;
	UINT32	VideoCol; // horizontal pixels
	UINT32	VideoRow; // vertical pixels
    BOOLEAN Supported;
    UINT32  GraphicsMode;
} GC_TEXT_MODE;

typedef struct _GC_DATA GC_DATA;

typedef VOID (* AGC_UPDATE_BLT_BUFFER ) (
	IN     GC_DATA 			             *This,
	IN     UINT32			             Width,
    IN     UINT32                        Height,
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer
	);

typedef VOID (* AGC_CLEAR_SCREEN) (
	IN OUT GC_DATA *This
	);

typedef VOID (* AGC_SCROLL_UP) (
	IN GC_DATA *This
	);

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name: GC_DATA
//
// Description: 
// This structure represents internal information structure for Graphics console
// driver
//
// Fields: Name          Type                               Description
//----------------------------------------------------------------------------
// SimpleTextOut        EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     Protocol structure
// Signature            UINT32                              Unique signature
// GraphicsOutput       EFI_GRAPHICS_OUTPUT_PROTOCOL*       Pointer to Gop driver
// Hii                  EFI_HII_PROTOCOL*                   Pointer to HII driver
// SupportedModes       GC_TEXT_MODE*                       Pointer to supported modes array
// MaxRows              UINT32                              Max rows in current mode
// MaxColumns           UINT32                              Max columns in current mode
// DeltaX               UINT32                              Horizontal indent of text window on screen in pixels
// DeltaY               UINT32                              Vertical indent of text window on screen in  pixels
// Cursor               EFI_GRAPHICS_OUTPUT_BLT_PIXEL       Array for saving cursor image
// BlinkVisible         BOOLEAN                             Current state of cursor in blinking mode
// CursorEvent          EFI_EVENT                           Event generated to blink cursor
// OemUpdateBltBuffer   AGC_UPDATE_BLT_BUFFER               Custom porting hook
// OemClearScreen       AGC_CLEAR_SCREEN                    Custom porting hook
// OemScrollUp          AGC_SCROLL_UP                       Custom porting hook
// 
// Notes:  
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

struct _GC_DATA{
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	SimpleTextOut;
	UINT32				            Signature;			            //signature (must be 0x54445348 ('GRCS') )
    UINT32                          Version;
	EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput;
	EFI_HII_FONT_PROTOCOL		    *HiiFont;
    GC_TEXT_MODE                    *SupportedModes;
    UINT32                          MaxRows;                        //max number of rows in current mode
    UINT32                          MaxColumns;                     //max number of columns in current mode
    UINT32                          DeltaX;                         //horizontal offset in pixels for current text mode
    UINT32                          DeltaY;                         //vertical offset in pixels for current text mode
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Cursor[NARROW_GLYPH_WIDTH * 3]; //Save cursor image
	BOOLEAN				            BlinkVisible;                   //if true cursor is visible, otherwise - invisible
	EFI_EVENT			            CursorEvent;
	AGC_UPDATE_BLT_BUFFER		    OemUpdateBltBuffer;		        //pointer to custom hook
	AGC_CLEAR_SCREEN		        OemClearScreen;		            //pointer to custom hook
	AGC_SCROLL_UP			        OemScrollUp;			        //pointer to custom hook
};
#pragma pack()

#endif

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
