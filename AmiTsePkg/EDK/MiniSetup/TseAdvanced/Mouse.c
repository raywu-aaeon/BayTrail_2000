//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2012, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
// $Archive: /Alaska/BIN/Modules/AMITSE2_0/AMITSE/TseAdvanced/Mouse.c $
//
// $Author: Premkumara $
//
// $Revision: 14 $
//
// $Date: 5/28/12 11:55a $
//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:		Mouse.c
//
// Description:	This file contains code to handle Mouse Operations
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

//#include "minisetup.h"
#ifdef TSE_FOR_APTIO_4_50

#include "Token.h" 
#include <Efi.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>

#include "TseCommon.h"
#include "commonoem.h"

#else //#ifdef TSE_FOR_APTIO_4_50

#include "minisetup.h"

#endif //#ifdef TSE_FOR_APTIO_4_50

#if TSE_STYLE_GTSE_BIN_SUPPORT
#include "gtseconfig.h"
#endif
 
#if MINISETUP_MOUSE_SUPPORT 

#if OLD_MOUSE_DRIVER_SUPPORT 
#include "Include\Protocol\MouseLib\MouseLib.h"
#else 
#include "Include\Protocol\MouseProtocol.h"
#endif 

#if AMITSE_SOFTKBD_SUPPORT
#include "Include\Protocol\SoftKbdProtocol.h"

SOFT_KBD_PROTOCOL *TSESoftKbd_n = NULL;
EFI_GUID gSoftKbdGuid = EFI_SOFT_KBD_PROTOCOL_GUID;
#endif 


#define CURSOR_WIDTH   20
#define CURSOR_HEIGHT  20

EFI_GUID gMouseDriverGuid = EFI_MOUSE_DRIVER_PROTOCOL_GUID;
DXE_MOUSE_PROTOCOL *TSEMouse=NULL;
INT32 Screen_Top=0,Screen_Left=0;		//Calculated Setup Screen Top and Left
EFI_STATUS GetScreenResolution(UINTN *ResX, UINTN *ResY);
UINTN HiiGetGlyphWidth(VOID);
UINTN HiiGetGlyphHeight(VOID);
UINTN StyleGetStdMaxRows(VOID);
extern UINTN gMaxRows;
extern UINTN gMaxCols;
VOID TSEStringReadLoopEntryHook(VOID);
VOID TSEStringReadLoopExitHook(VOID);
VOID TSEMouseInitHook(VOID);
VOID TSEMouseStopHook(VOID);
VOID TSEMouseRefreshHook(VOID);
VOID TSEMouseStartHook(VOID);
BOOLEAN TSEMouseIgnoreMouseActionHook(VOID);
VOID TSEMouseDestroyHook(VOID);

#if !OVERRIDE_TSEMouseInit
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseInit
//
// Description:	Function to initialize the mouse
//
// Input:		None 
//
// Output:		VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEMouseInit(VOID)
{
	EFI_STATUS Status;
    UINTN HorizontalResolution = 0;
    UINTN VerticalResolution = 0;

    Status = gBS->LocateProtocol(&gMouseDriverGuid, NULL, &TSEMouse);

    if( Status==( EFI_INVALID_PARAMETER | EFI_NOT_FOUND ) )
    { 
        TSEMouse=NULL;
    }
    else
    {
		Status = GetScreenResolution(&HorizontalResolution, &VerticalResolution);

		if ( EFI_ERROR( Status ) )
 	 		TSEMouse = NULL;

        if(TSEMouse)
        {

#if TSE_STYLE_GTSE_BIN_SUPPORT
			{
				GTSESetupConfigData* Data;
				Data = GTSEGetConfigData();
				Screen_Top =  Data->StartY ;
				Screen_Left = Data->StartX ;
			}
#else
			Screen_Top = (INT32)(VerticalResolution - (gMaxRows*HiiGetGlyphHeight()) ) /2;
			Screen_Left = (INT32)(HorizontalResolution - (gMaxCols*HiiGetGlyphWidth()) ) /2;
#endif

#if OLD_MOUSE_DRIVER_SUPPORT 
            TSEMouse->Initialize(
                        (UINT32)Screen_Left,
                        (UINT32)Screen_Top,
                        (UINT32)(Screen_Left + (gMaxCols*HiiGetGlyphWidth())),
                        (UINT32)(Screen_Top + (gMaxRows*HiiGetGlyphHeight()))
                        );
#else
            //Initializing the Mouse (CURSOR,BOUNDARY,MAPPING etc)
            TSEMouse->InitializePointingDevice(TSEMouse,
                        (INT32)Screen_Left,
                        (INT32)Screen_Top,
                        (INT32)(Screen_Left + (gMaxCols*HiiGetGlyphWidth())),
                        (INT32)(Screen_Top + (gMaxRows*HiiGetGlyphHeight()))
                        );

#endif

        }
    }

    TSEMouseInitHook();
}
#endif

#if !OVERRIDE_TSEMouseStop
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseStop
//
// Description:	Function to handle the mouse stop
//
// Input:		None 
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEMouseStop(VOID)
{
    //Stop mouse pointer draw
    if(TSEMouse!=NULL)
    {
#if OLD_MOUSE_DRIVER_SUPPORT 
        TSEMouse->Stop();
#else
        TSEMouse->StopUpdatingMouseCursor(TSEMouse);
#endif
     }

    TSEMouseStopHook();
}
#endif

#if !OVERRIDE_TSEIsMouseClickedonSoftkbd
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEIsMouseClickedonSoftkbd
//
// Description:	Function to handle the mouse Clicked on Softkbd
//
// Input:		None 
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN TSEIsMouseClickedonSoftkbd(VOID)
{
	EFI_STATUS Status = EFI_UNSUPPORTED;
	UINT32 x=0,y=0, z=0;
	UINT32 SoftKbdStartX=0, SoftKbdStartY=0, SoftKbdWidth=0, SoftKbdHeight=0;

#if AMITSE_SOFTKBD_SUPPORT
Status = gBS->LocateProtocol(&gSoftKbdGuid, NULL, &TSESoftKbd_n);

    	if( Status==( EFI_INVALID_PARAMETER | EFI_NOT_FOUND ) )
    	{ 
        	TSESoftKbd_n=NULL;
    	}
	
	TSEMouse->GetCoordinates(TSEMouse,&x,&y,&z);
	TSESoftKbd_n->GetPosition(TSESoftKbd_n, FALSE ,&SoftKbdStartY,&SoftKbdStartX);
	TSESoftKbd_n->GetDimension(TSESoftKbd_n,&SoftKbdWidth, &SoftKbdHeight);
	//EIP 83753 : SoftKbd flickers when positioned near Top/Left end of the screen display. 
     if((( (INT32)x > (INT32)(SoftKbdStartX-HiiGetGlyphHeight()))&&((INT32)y > (INT32)(SoftKbdStartY-(2*HiiGetGlyphHeight()))))&& (((INT32)x < (INT32)(SoftKbdStartX+SoftKbdWidth))&&((INT32)y < (INT32)(SoftKbdStartY + SoftKbdHeight))) )
		return TRUE;
	else
		return FALSE;
#endif
return FALSE;
}
#endif

#if !OVERRIDE_TSEMouseRefresh
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseRefresh
//
// Description:	Function to handle the mouse Refresh
//
// Input:		None 
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEMouseRefresh(VOID)
{
	 EFI_STATUS Status = EFI_UNSUPPORTED;

 	if((TRUE == TSEMouseIgnoreMouseActionHook())&&(TRUE == TSEIsMouseClickedonSoftkbd()))
	{
		TSEMouse->StopUpdatingMouseCursor(TSEMouse);
	}

	TSEMouseRefreshHook();

    //refresh mouse pointer
    if(TSEMouse!=NULL)
    {
#if OLD_MOUSE_DRIVER_SUPPORT 
        TSEMouse->Refresh();
#else
	TSEMouse->UpdateMousePointerProperty(TSEMouse ,MOUSE_POINTER_PROPERTY_NORMAL);
        TSEMouse->RefreshMouseCursor(TSEMouse);
#endif
    }
}
#endif

#if !OVERRIDE_TSEMouseStart
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseStart
//
// Description:	Function to handle the mouse Startd
//
// Input:		None 
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEMouseStart(VOID)
{
    TSEMouseStartHook();

	if(TSEMouse!=NULL)
#if OLD_MOUSE_DRIVER_SUPPORT 
		TSEMouse->Start();
#else
        TSEMouse->StartUpdatingMouseCursor(TSEMouse);
#endif
}
#endif

#if !OVERRIDE_TSEMouseFreeze
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseFreeze
//
// Description:	Function to handle the mouse Destory
//
// Input:		None 
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEMouseFreeze(VOID)
{
    //Stop mouse pointer draw
    if(TSEMouse!=NULL)
#if OLD_MOUSE_DRIVER_SUPPORT 
        return;
#else
        TSEMouse->UpdateMousePointerProperty(TSEMouse ,MOUSE_POINTER_PROPERTY_FREEZE);
#endif

}
#endif

#if !OVERRIDE_TSEGetCoordinates
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEGetCoordinates
//
// Description:	Function to handle the mouse Destory
//
// Input:		None 
//
// Output:		EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS TSEGetCoordinates(INT32 *x, INT32 *y, INT32 *z)
{
EFI_STATUS Status = EFI_SUCCESS;
    //Stop mouse pointer draw
    if(TSEMouse!=NULL)
#if OLD_MOUSE_DRIVER_SUPPORT 
        return;
#else
        TSEMouse->GetCoordinates(TSEMouse ,x,y,z);
#endif
   return EFI_SUCCESS;	
}
#endif

#if !OVERRIDE_TSEMouseReadInfo
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseReadInfo
//
// Description:	Function to read the information using mouse
//
// Input:		MOUSE_INFO *MouseInfo 
//
// Output:		EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS TSEMouseReadInfo(MOUSE_INFO *MouseInfo)
{
    EFI_STATUS Status = EFI_UNSUPPORTED;
    UINT32 Button_Status=0;
    UINT32 x=0,y=0, z=0;
    UINT32 SoftKbdStartX=0, SoftKbdStartY=0, SoftKbdWidth=0, SoftKbdHeight=0;

    if(NULL == TSEMouse)
        return EFI_UNSUPPORTED;

    TSEMouse->GetCoordinates(TSEMouse,&x,&y,&z);

    // If mouse action are consumed by Modules like SoftKeyBoard.
    if(TSEMouseIgnoreMouseActionHook() == TRUE)
    {

#if AMITSE_SOFTKBD_SUPPORT
 	Status = gBS->LocateProtocol(&gSoftKbdGuid, NULL, &TSESoftKbd_n);

    	if( Status==( EFI_INVALID_PARAMETER | EFI_NOT_FOUND ) )
    	{ 
        	TSESoftKbd_n=NULL;
    	}
	
		TSESoftKbd_n->GetPosition(TSESoftKbd_n,FALSE, &SoftKbdStartY,&SoftKbdStartX);
		TSESoftKbd_n->GetDimension(TSESoftKbd_n,&SoftKbdWidth, &SoftKbdHeight);
	     if((( x > SoftKbdStartX)&&(y > SoftKbdStartY))&& ((x < (SoftKbdStartX+SoftKbdWidth))&&(y <(SoftKbdStartY + SoftKbdHeight))) )
		  {
			TSESoftKbd_n->UpdateMouseInterface(TSESoftKbd_n, TSEMouse);
	
	        return EFI_NOT_FOUND;
		  }
		  else
			TSESoftKbd_n->UpdateMouseInterface(TSESoftKbd_n, NULL);
#endif

	       // return EFI_NOT_FOUND;
    }
#if OLD_MOUSE_DRIVER_SUPPORT 
    //Getting Mouse Button Status
    TSEMouse->GetButtonStatus(&Button_Status);     

    //Getting Mouse Pointer Position
    TSEMouse->GetPosition((INT32*)&x,(INT32*)&y);    
#else
   
   TSEMouse->MousePoll(TSEMouse);
   TSEMouse->GetButtonStatus(TSEMouse,&Button_Status);     

    //Getting Mouse Pointer Position
    TSEMouse->GetCoordinates(TSEMouse,&x,&y,&z);    
#endif

    y-=Screen_Top;
    x-=Screen_Left;

    //Converting Graphics Resolution to Text Mode Location
    MouseInfo->Left = (UINT16)(x/HiiGetGlyphWidth());
    MouseInfo->Top = (UINT16)(y/HiiGetGlyphHeight());

    //Setting the Calculated SCREEN_TOP and SCREEN_LEFT Rather than Hardcoding
//    MouseInfo->Top-=Screen_Top;
//    MouseInfo->Left-=Screen_Left;
    MouseInfo->ButtonStatus=Button_Status;

    return EFI_SUCCESS;		
}
#endif

#if !OVERRIDE_TSEMouseDestroy
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEMouseDestroy
//
// Description:	Function to handle the mouse Destory
//
// Input:		None 
//
// Output:		None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEMouseDestroy(VOID)
{
    //Stop mouse pointer draw
	if(TSEMouse!=NULL)
	{
#if OLD_MOUSE_DRIVER_SUPPORT 
		TSEMouse->Stop();
#else	
		TSEMouse->StopUpdatingMouseCursor(TSEMouse);
		TSEMouse->StopPointingDevice(TSEMouse);	//EIP-84150 
#endif
	}

    TSEMouseDestroyHook();
}
#endif

#if !OVERRIDE_TSEGetactualScreentop
//EIP 79962 : START
// Mouse pointer corruption in GTSE  
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEGetactualScreentop
//
// Description:	Function to retrun the actual Screen Top value
//
// Input:		None 
//
// Output:		INT32
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
INT32 TSEGetactualScreentop(VOID)
{
	return Screen_Top;
}
//EIP 79962 : END
#endif

#if AMITSE_SOFTKBD_SUPPORT
#if !OVERRIDE_TSENumericSoftKbdInit
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSENumericSoftKbdInit
//
// Description:	Function to display the numeric softkbd
//
// Input:		None 
//
// Output:		INT32
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSENumericSoftKbdInit(VOID)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  Status = gBS->LocateProtocol(&gSoftKbdGuid, NULL, &TSESoftKbd_n);

   	if( Status==( EFI_INVALID_PARAMETER | EFI_NOT_FOUND ) )
   	{ 
       	TSESoftKbd_n=NULL;
   	}
	if(NULL == TSESoftKbd_n)
	 return;
	
	TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_NUMERIC);
	TSEStringReadLoopEntryHook();		
}
#endif

#if !OVERRIDE_TSENumericSoftKbdExit
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSENumericSoftKbdExit
//
// Description:	Function to destroy the numeric softkbd display
//
// Input:		None 
//
// Output:		INT32
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSENumericSoftKbdExit (VOID)
{
	if(NULL == TSESoftKbd_n)
	 return;
	TSEStringReadLoopExitHook();
	TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_FULL_US_KEYBOARD);
}
#endif

//EIP-88912 Starts
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSESetPwdKeyboardLayout
//
// Description:	function to initialize TSESetKeyboardLayout
//
// Input:		VOID
//
// Output:		VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSESetPwdKeyboardLayout(VOID)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  Status = gBS->LocateProtocol(&gSoftKbdGuid, NULL, &TSESoftKbd_n);

	if(EFI_ERROR( Status ))
	{ 
		TSESoftKbd_n=NULL;
	}
	if(NULL == TSESoftKbd_n)
	 return;

	if ( SETUP_STORE_KEYCODE_PASSWORD )
		TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_FULL_US_KEYBOARD );
	else
		TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_PRINTABLE_KEYS );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEResetPwdKeyboardLayout
//
// Description:	function to exit TSEResetKeyboardLayout and reset with default keyboard layout
//
// Input:		VOID
//
// Output:		VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEResetPwdKeyboardLayout(VOID)
{
	if(NULL == TSESoftKbd_n)
	 return;
	TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_FULL_US_KEYBOARD);
}
//EIP-88912 Ends

//EIP-89272 Starts
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEPrintableKeysSoftKbdInit
//
// Description:	function to launch PrintableKeysSoftkbd layout
//
// Input:		VOID
//
// Output:		VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEPrintableKeysSoftKbdInit(VOID)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  Status = gBS->LocateProtocol(&gSoftKbdGuid, NULL, &TSESoftKbd_n);

	if(EFI_ERROR( Status ))
	{ 
		TSESoftKbd_n=NULL;
	}
	if(NULL == TSESoftKbd_n)
	 return;
	
	TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_PRINTABLE_KEYS);
	TSEStringReadLoopEntryHook();		
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	TSEPrintableKeysSoftKbdExit
//
// Description:	function to exit PrintableKeysSoftkbd layout and reset with default keyboard layout
//
// Input:		VOID
//
// Output:		VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID TSEPrintableKeysSoftKbdExit(VOID)
{
	if(NULL == TSESoftKbd_n)
	 return;
	TSEStringReadLoopExitHook();
	TSESoftKbd_n->SetKbdLayout(TSESoftKbd_n, SOFT_KEY_LAYOUT_FULL_US_KEYBOARD);
}

//EIP-89272 Ends

#endif //AMITSE_SOFTKBD_SUPPORT
#endif //MINISETUP_MOUSE_SUPPORT

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
