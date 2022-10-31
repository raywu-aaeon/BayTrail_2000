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
// Name:  GraphicsConsole.c
//
// Description:  Graphics console driver that produces the Simple Text Out
//		interface
//
//<AMI_FHDR_END>
//**********************************************************************

//-----------------------------------------------------------------------------
#include "GraphicsConsole.h"

//-----------------------------------------------------------------------------
// Function Prototypes

EFI_STATUS	DriverBindingSupported ( 
	IN EFI_DRIVER_BINDING_PROTOCOL    *This,
	IN EFI_HANDLE                     ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath);

EFI_STATUS	DriverBindingStart ( 
	IN EFI_DRIVER_BINDING_PROTOCOL    *This,
	IN EFI_HANDLE                     ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath);

EFI_STATUS	DriverBindingStop ( 
	IN EFI_DRIVER_BINDING_PROTOCOL	*This,
	IN EFI_HANDLE			        ControllerHandle,
	IN  UINTN			            NumberOfChildren,
	IN  EFI_HANDLE			        *ChildHandleBuffer);

//******************** Simple Text Output protocol functions prototypes ***********

EFI_STATUS GCReset(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN BOOLEAN ExtendedVerification);

EFI_STATUS GCOutputString(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN CHAR16 *String);

EFI_STATUS GCTestString(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN CHAR16 *String);

EFI_STATUS GCQueryMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN UINTN ModeNum, 
    OUT UINTN *Col, 
    OUT UINTN *Row);

EFI_STATUS GCSetMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN UINTN ModeNum);

EFI_STATUS GCSetAttribute(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN UINTN Attribute);

EFI_STATUS GCClearScreen(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);

EFI_STATUS GCSetCursorPosition(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN UINTN Column, 
    IN UINTN Row);

EFI_STATUS GCEnableCursor(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN BOOLEAN Visible);

//******************** Service functions prototypes ********************************

EFI_STATUS GetColorFromAttribute(
    IN UINT32 Attribute, 
    OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Foreground,
    OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Background);

EFI_STATUS GetGraphicsModeNumber (
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput, 
    IN UINT32                       HorRes,
    IN UINT32                       VerRes,
	OUT UINT32                      *ModeNum,
    IN  BOOLEAN                     ExactMatch,
    OUT UINT32                      *ActualHorRes OPTIONAL,
    OUT UINT32                      *ActualVerRes OPTIONAL );

EFI_STATUS SetupGraphicsDevice(
    IN GC_DATA *GcData);

VOID EFIAPI BlinkCursorEvent ( IN EFI_EVENT Event, IN VOID *Context );

VOID DrawCursor(
    IN GC_DATA *GcData,
    IN BOOLEAN Visible);

VOID ScrollUp(
    IN GC_DATA *GcData);

VOID SaveCursorImage(
    IN GC_DATA *GcData);

VOID ShiftCursor(
    IN GC_DATA *GcData,
    IN UINT16 Step);

//********************** Hooks prototypes ******************************************

VOID GcUpdateBltBuffer (
	IN     GC_DATA 			             *This,			    //pointer to internal structure
	IN     UINT32			             Width,	            //width of the buffer in pixels
    IN     UINT32                        Height,            //height of the buffer in pixels
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer	        //pointer to BLT buffer to update
	);

VOID GcInternalClearScreen (
	IN OUT GC_DATA *This
	);

//-----------------------------------------------------------------------------
// Globals
const TEXT_MODE TextModeArray[] = {GC_MODE_LIST};
const INT32 MaxTextMode=(sizeof(TextModeArray)/sizeof(TEXT_MODE));
const EFI_GRAPHICS_OUTPUT_BLT_PIXEL ColorArray[] = {			
	{ GC_COLOR_BLACK       ,   0}, // case EFI_BLACK: 		// case EFI_BACKGROUND_BLACK >> 4
	{ GC_COLOR_BLUE        ,   0}, // case EFI_BLUE : 		// case EFI_BACKGROUND_BLUE >> 4
	{ GC_COLOR_GREEN       ,   0}, // case EFI_GREEN : 	// case EFI_BACKGROUND_GREEN >> 4
	{ GC_COLOR_CYAN        ,   0}, // case EFI_CYAN : 		// case EFI_BACKGROUND_CYAN >> 4
	{ GC_COLOR_RED         ,	0}, // case EFI_RED : 		// case EFI_BACKGROUND_RED >> 4
	{ GC_COLOR_MAGENTA     ,	0}, // case EFI_MAGENTA : 	// case EFI_BACKGROUND_MAGENTA >> 4
	{ GC_COLOR_BROWN       ,	0}, // case EFI_BROWN : 	// case EFI_BACKGROUND_BROWN >> 4
	{ GC_COLOR_LIGHTGRAY   ,	0}, // case EFI_LIGHTGRAY : // case EFI_BACKGROUND_LIGHTGRAY >> 4
	{ GC_COLOR_DARKGRAY    ,	0}, // case EFI_DARKGRAY : 
	{ GC_COLOR_LIGHTBLUE   ,	0}, // case EFI_LIGHTBLUE : 
	{ GC_COLOR_LIGHTGREEN  ,	0}, // case EFI_LIGHTGREEN : 
	{ GC_COLOR_LIGHTCYAN   ,	0}, // case EFI_LIGHTCYAN : 
	{ GC_COLOR_LIGHTRED    ,	0}, // case EFI_LIGHTRED : 
	{ GC_COLOR_LIGHTMAGENTA,	0}, // case EFI_LIGHTMAGENTA : 
	{ GC_COLOR_YELLOW      ,	0}, // case EFI_YELLOW : 
	{ GC_COLOR_WHITE       ,	0}  // case EFI_WHITE : 
};

//-----------------------------------------------------------------------------
// Protocol implementation
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	mGCProtocol	=
	{
	GCReset,
	GCOutputString,
	GCTestString,
	GCQueryMode,
	GCSetMode,
	GCSetAttribute,
	GCClearScreen,
	GCSetCursorPosition,
	GCEnableCursor,
	NULL
	};


//-----------------------------------------------------------------------------
// Driver Binding Protocol

EFI_DRIVER_BINDING_PROTOCOL gGraphicsConsoleDriverBindingProtocol = {
	DriverBindingSupported,
	DriverBindingStart,
	DriverBindingStop,
	0x10,
	NULL,
	NULL
	};


//-----------------------------------------------------------------------------
// Function Definitions
#ifdef EFI_DEBUG
#ifndef EFI_COMPONENT_NAME2_PROTOCOL_GUID //old Core
#ifndef LANGUAGE_CODE_ENGLISH
#define LANGUAGE_CODE_ENGLISH "eng"
#endif
static BOOLEAN LanguageCodesEqual(
    CONST CHAR8* LangCode1, CONST CHAR8* LangCode2
){
    return    LangCode1[0]==LangCode2[0] 
           && LangCode1[1]==LangCode2[1]
           && LangCode1[2]==LangCode2[2];
}
static EFI_GUID gEfiComponentName2ProtocolGuid = EFI_COMPONENT_NAME_PROTOCOL_GUID;
#endif
//Driver Name
static UINT16 *gDriverName=L"AMI Graphic Console Driver";

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:  GraphicsConsoleGetControllerName
//
// Description: 
//  EFI_COMPONENT_NAME_PROTOCOL GetControllerName function
//
// Input:       
//  IN EFI_COMPONENT_NAME_PROTOCOL* This - pointer to protocol instance
//  IN EFI_HANDLE Controller - controller handle
//  IN EFI_HANDLE ChildHandle - child handle
//  IN CHAR8* Language - pointer to language description
//  OUT CHAR16** ControllerName - pointer to store pointer to controller name
//
// Output:      
//      EFI_STATUS
//          EFI_SUCCESS - controller name returned
//          EFI_INVALID_PARAMETER - language undefined
//          EFI_UNSUPPORTED - given language not supported
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS GraphicsConsoleGetControllerName (
		IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
		IN  EFI_HANDLE                   ControllerHandle,
 		IN  EFI_HANDLE                   ChildHandle        OPTIONAL,
  		IN  CHAR8                        *Language,
  		OUT CHAR16                       **ControllerName
)
{
	return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:   GraphicsConsoleGetDriverName
//
// Description: 
//  EFI_COMPONENT_NAME_PROTOCOL GetDriverName function
//
// Input:       
//  IN EFI_COMPONENT_NAME_PROTOCOL* This - pointer to protocol instance
//  IN CHAR8* Language - pointer to language description
//  OUT CHAR16** DriverName - pointer to store pointer to driver name
//
// Output:      
//  EFI_STATUS
//      EFI_SUCCESS - driver name returned
//      EFI_INVALID_PARAMETER - language undefined
//      EFI_UNSUPPORTED - given language not supported
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS GraphicsConsoleGetDriverName(
    IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
	IN  CHAR8                        *Language,
	OUT CHAR16                       **DriverName
)
{
	//Supports only English
	if(!Language || !DriverName) 
        return EFI_INVALID_PARAMETER;

	if (!LanguageCodesEqual( Language, LANGUAGE_CODE_ENGLISH)) 
        return EFI_UNSUPPORTED;
	else 
        *DriverName=gDriverName;
	
	return EFI_SUCCESS;
}

//Component Name Protocol
static EFI_COMPONENT_NAME2_PROTOCOL gGraphicsConsole = {
  GraphicsConsoleGetDriverName,
  GraphicsConsoleGetControllerName,
  LANGUAGE_CODE_ENGLISH
};
#endif

/**
  HII Database Protocol notification event handler.

  Register default font package when HII Database Protocol has been installed.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID EFIAPI RegisterFontPackage (IN EFI_EVENT Event, IN VOID *Context){
///// Load Font Pack
#define FONT_FFS_FILE_GUID { 0xdac2b117, 0xb5fb, 0x4964, { 0xa3, 0x12, 0xd, 0xcc, 0x77, 0x6, 0x1b, 0x9b } }
// {A7FE7491-2C54-4F72-B80E-7649A8210193}
#define FONT_FFS_FILE_SECTION_GUID { 0xa7fe7491, 0x2c54, 0x4f72, { 0xb8, 0xe, 0x76, 0x49, 0xa8, 0x21, 0x1, 0x93 } }

static EFI_GUID FontFfsFileGuid = FONT_FFS_FILE_GUID;
static EFI_GUID FontFfsFileSectionGuid = FONT_FFS_FILE_SECTION_GUID;
extern UINT8 UsStdNarrowGlyphData[];
	EFI_FIRMWARE_VOLUME2_PROTOCOL *pFV;
	UINTN DataSize;
	EFI_GUID *pSectionGuid = NULL;
	UINT32 Authentication;
	EFI_HANDLE *pHandle;
	UINTN Number,i;
    EFI_STATUS FontStatus;
    EFI_HII_PACKAGE_HEADER *FontPackage;
    EFI_HII_HANDLE HiiFontHandle;

    FontPackage = (EFI_HII_PACKAGE_HEADER*)&UsStdNarrowGlyphData;
	FontStatus = pBS->LocateHandleBuffer(ByProtocol,&gEfiFirmwareVolume2ProtocolGuid, NULL, &Number, &pHandle);
	if (EFI_ERROR(FontStatus)) return;
	for(i=0;i<Number; i++)
	{
		FontStatus=pBS->HandleProtocol(pHandle[i], &gEfiFirmwareVolume2ProtocolGuid, &pFV);
		if (EFI_ERROR(FontStatus)) continue;
		DataSize=0;
		//NOTE: Only one section per FFS file is supported
		FontStatus=pFV->ReadSection (
			pFV,&FontFfsFileGuid,
			EFI_SECTION_FREEFORM_SUBTYPE_GUID,0, &pSectionGuid, &DataSize,
			&Authentication
	 	);
		if (!EFI_ERROR(FontStatus))
		{
			if (guidcmp(pSectionGuid,&FontFfsFileSectionGuid)){
				pBS->FreePool(pSectionGuid);
				pSectionGuid = NULL;
				continue;
			}
			FontPackage=(EFI_HII_PACKAGE_HEADER*)(pSectionGuid+1);
			break;
		} else pSectionGuid = NULL;
	}
	FontStatus = HiiLibPublishPackages(&FontPackage, 1, &FontFfsFileGuid, NULL, &HiiFontHandle);
	if (pSectionGuid!=NULL) pBS->FreePool(pSectionGuid);
	pBS->FreePool(pHandle);
	pBS->CloseEvent(Event);
}


//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCEntryPoint
//
// Description:	
//  Installs gGraphicsConsoleDriverBindingProtocol protocol
//
// Input:
//	IN EFI_HANDLE ImageHandle - driver image handle
//	IN EFI_SYSTEM_TABLE *SystemTable - pointer to system table
//
// Output:
//	EFI_STATUS
//      EFI_SUCCESS - Driver binding protocol was installed
//
// Modified:
//
// Referrals: InitAmiLib InstallMultipleProtocolInterfaces
//
// Notes:
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS	EFIAPI GCEntryPoint (
	IN EFI_HANDLE           ImageHandle,
	IN EFI_SYSTEM_TABLE     *SystemTable
)
{
	EFI_STATUS	Status;
    EFI_EVENT   DatabaseProtocolEvent;

	InitAmiLib(ImageHandle, SystemTable);
    
    //
    // Register notify function on HII Database Protocol to add default font.
    //
    EfiCreateProtocolNotifyEvent (
        &gEfiHiiDatabaseProtocolGuid,
        TPL_CALLBACK,
        RegisterFontPackage,
        NULL,
        &DatabaseProtocolEvent
    );

	// initiaize the ImageHandle and DriverBindingHandle
	gGraphicsConsoleDriverBindingProtocol.DriverBindingHandle = NULL;
	gGraphicsConsoleDriverBindingProtocol.ImageHandle = ImageHandle;

	// Install driver binding protocol here
	Status = pBS->InstallMultipleProtocolInterfaces (
						&gGraphicsConsoleDriverBindingProtocol.DriverBindingHandle,
						&gEfiDriverBindingProtocolGuid, &gGraphicsConsoleDriverBindingProtocol,
#ifdef EFI_DEBUG
						&gEfiComponentName2ProtocolGuid, &gGraphicsConsole,
#endif
						NULL);

	return Status;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: DriverBindingSupported
//
// Description: 
//  Checks to see if this driver can be used
//
// Input:
//	IN EFI_DRIVER_BINDING_PROTOCOL *This - pointer to protocol instance
//	IN EFI_HANDLE Controller - handle of controller to install driver on
//	IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath - pointer to device path
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - Driver supports the Device
//      EFI_NOT_SUPPORTED - Driver cannot support the Device 
//
// Notes:
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS	DriverBindingSupported (
	IN EFI_DRIVER_BINDING_PROTOCOL    *This,
	IN EFI_HANDLE                     ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
)
{
	EFI_STATUS                   Status;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput = NULL;
	EFI_HII_FONT_PROTOCOL		 *HiiFont;

	Status = pBS->OpenProtocol(
                        ControllerHandle, 
                        &gEfiGraphicsOutputProtocolGuid,
    	                &GraphicsOutput, 
                        This->DriverBindingHandle,
		                ControllerHandle, 
                        EFI_OPEN_PROTOCOL_BY_DRIVER);
	if (EFI_ERROR(Status))
        return Status;
    else
		pBS->CloseProtocol( 
                        ControllerHandle, 
                        &gEfiGraphicsOutputProtocolGuid,
					    This->DriverBindingHandle, 
                        ControllerHandle);

	Status = pBS->OpenProtocol(
                        ControllerHandle, 
                        &gEfiDevicePathProtocolGuid,
    	                NULL, 
                        This->DriverBindingHandle,
		                ControllerHandle, 
                        EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
	if (EFI_ERROR(Status)) 
        return Status;

	Status = pBS->LocateProtocol ( &gEfiHiiFontProtocolGuid, NULL, &HiiFont);

	return Status;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: DriverBindingStart
//
// Description: 
//  This function grabs needed protocols and initializes the supported text modes
//
// Input:
//	IN EFI_DRIVER_BINDING_PROTOCOL *This - pointer to protocol instance
//	IN EFI_HANDLE Controller - handle of controller to install driver on
//	IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath - pointer to device path
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - SimpleTextOut Protocol installed
//      EFI_NOT_SUPPORTED - SimpleTextOut Protocol not installed
//
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS	DriverBindingStart (
	IN EFI_DRIVER_BINDING_PROTOCOL *This,
	IN EFI_HANDLE                  ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
)
{
	EFI_STATUS			Status;

#if (CURSOR_BLINK_ENABLE != 0)
	EFI_STATUS			EventStatus;
#endif
	GC_DATA				*GcData = NULL;

	// Create private data structure and fill with proper data
	Status = pBS->AllocatePool(EfiBootServicesData, sizeof(GC_DATA), &GcData);
	if (EFI_ERROR(Status))
        return Status;

	pBS->CopyMem( &(GcData->SimpleTextOut), 
                  &mGCProtocol, 
				  sizeof(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL) );

	Status = pBS->AllocatePool(EfiBootServicesData, 
                               sizeof(SIMPLE_TEXT_OUTPUT_MODE), 
                               &(GcData->SimpleTextOut.Mode));
	// Initialize mode with the invalid value.
	// The correct value will be assigned by GcSetMode (called by GcReset)
	GcData->SimpleTextOut.Mode->Mode = -1;
    if(EFI_ERROR(Status))
    {
        pBS->FreePool(GcData);
        return Status;
    }

	Status = pBS->AllocatePool(EfiBootServicesData, 
                               sizeof(GC_TEXT_MODE) * MaxTextMode, 
                               &(GcData->SupportedModes));
    if(EFI_ERROR(Status))
    {
        pBS->FreePool(GcData->SimpleTextOut.Mode);
        pBS->FreePool(GcData);
        return Status;
    }

	Status = pBS->OpenProtocol( 
                            ControllerHandle, 
                            &gEfiGraphicsOutputProtocolGuid,
					        &(GcData->GraphicsOutput), 
                            This->DriverBindingHandle,
					        ControllerHandle, 
                            EFI_OPEN_PROTOCOL_BY_DRIVER );
	if (EFI_ERROR(Status))
    {
        pBS->FreePool(GcData->SupportedModes);
        pBS->FreePool(GcData->SimpleTextOut.Mode);
        pBS->FreePool(GcData);
		return Status;
    }

	// Find the Hii Protocol and attach it to the datastructure
	Status = pBS->LocateProtocol ( &gEfiHiiFontProtocolGuid, NULL, &(GcData->HiiFont));

	if (EFI_ERROR(Status))
	{
		pBS->CloseProtocol( 
                        ControllerHandle, 
                        &gEfiGraphicsOutputProtocolGuid,
					    This->DriverBindingHandle, 
                        ControllerHandle);
        pBS->FreePool(GcData->SupportedModes);
        pBS->FreePool(GcData->SimpleTextOut.Mode);
        pBS->FreePool(GcData);
		return EFI_UNSUPPORTED;
	}

    Status = SetupGraphicsDevice(GcData);
    if(EFI_ERROR(Status))
	{
		pBS->CloseProtocol( 
                        ControllerHandle, 
                        &gEfiGraphicsOutputProtocolGuid,
					    This->DriverBindingHandle, 
                        ControllerHandle);
        pBS->FreePool(GcData->SupportedModes);
        pBS->FreePool(GcData->SimpleTextOut.Mode);
        pBS->FreePool(GcData);
		return EFI_UNSUPPORTED;
	}	

    //initialize porting hooks and signature
    GcData->Signature = 0x54445348;
    GcData->Version = 1;
    GcData->OemUpdateBltBuffer = GcUpdateBltBuffer;
    GcData->OemClearScreen = GcInternalClearScreen;
    GcData->OemScrollUp = NULL;
    GcData->DeltaX = 0;
    GcData->DeltaY = 0;
    GcData->MaxColumns = 0;
    GcData->MaxRows = 0;

	// Default the cursor blink to the show cursor state
	GcData->BlinkVisible = TRUE;


   (GcData->SimpleTextOut.Mode)->CursorVisible = FALSE;    //since initial position of window is undefined we cannot draw cursor yet
    Status = GCReset(&(GcData->SimpleTextOut), FALSE);
//    GCOutputString(&(GcData->SimpleTextOut), L"AMI Graphics Console Started");

	// install the simple text out protocol
	Status = pBS->InstallMultipleProtocolInterfaces ( 
                    &ControllerHandle,            
                    &gEfiSimpleTextOutProtocolGuid, 
                    &GcData->SimpleTextOut,
                    NULL);
	
	if (EFI_ERROR(Status))
	{
		// close protocols and free allocated memory
		pBS->CloseProtocol( 
                        ControllerHandle, 
                        &gEfiGraphicsOutputProtocolGuid,
					    This->DriverBindingHandle, 
                        ControllerHandle);

        pBS->FreePool(GcData->SupportedModes);
        pBS->FreePool(GcData->SimpleTextOut.Mode);
		pBS->FreePool(GcData);
        return EFI_UNSUPPORTED;
    }

#if (CURSOR_BLINK_ENABLE != 0)
	EventStatus = pBS->CreateEvent (
                    EVT_TIMER | EVT_NOTIFY_SIGNAL,
					TPL_NOTIFY,
					BlinkCursorEvent,
					&(GcData->SimpleTextOut),
					&(GcData->CursorEvent)
					);
	if (!EFI_ERROR(EventStatus))
	{
		pBS->SetTimer(GcData->CursorEvent, TimerPeriodic, 5000000);
	}
#endif
	
	return Status;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: DriverBindingStop
//
// Description: 
//  Uninstalls the driver from given ControllerHandle
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL *This - pointer to protocol instance
//  IN EFI_HANDLE ControllerHandle - controller handle to uninstall driver from
//  IN UINTN NumberOfChildren - number of children supported by this driver
//  IN EFI_HANDLE *ChildHandleBuffer  - pointer to child handles buffer
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS	- driver uninstalled from controller
//      EFI_NOT_STARTED - driver was not started
//		
// Notes:
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS	DriverBindingStop ( 
	IN EFI_DRIVER_BINDING_PROTOCOL *This,
	IN EFI_HANDLE                  ControllerHandle,
	IN UINTN                       NumberOfChildren,
	IN EFI_HANDLE                  *ChildHandleBuffer
)
{
	EFI_STATUS						Status;
	GC_DATA							*GcData = NULL;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*SimpleTextOut = NULL;

	Status = pBS->OpenProtocol (
                            ControllerHandle, 
                            &gEfiSimpleTextOutProtocolGuid, 
					        &SimpleTextOut, 
                            This->DriverBindingHandle, 
					        ControllerHandle, 
                            EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR (Status))
		return EFI_NOT_STARTED;

	Status = pBS->CloseProtocol (
                            ControllerHandle, 
                            &gEfiSimpleTextOutProtocolGuid, 
					        This->DriverBindingHandle, 
                            ControllerHandle);

	GcData = (GC_DATA *) SimpleTextOut;	

	// uninstall the simple text out protocol
	Status = pBS->UninstallMultipleProtocolInterfaces ( 
                            ControllerHandle,            
                            &gEfiSimpleTextOutProtocolGuid, 
                            &GcData->SimpleTextOut,
                            NULL);
	if (EFI_ERROR (Status))
		return Status;

#if (CURSOR_BLINK_ENABLE != 0)
	pBS->SetTimer(GcData->CursorEvent, TimerCancel, 0);
	pBS->CloseEvent(GcData->CursorEvent);
#endif

	Status = pBS->CloseProtocol( 
                            ControllerHandle, 
                            &gEfiGraphicsOutputProtocolGuid,
						    This->DriverBindingHandle, 
                            ControllerHandle);

    pBS->FreePool(GcData->SupportedModes);
    pBS->FreePool(GcData->SimpleTextOut.Mode);
    pBS->FreePool(GcData);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCReset
//
// Description: 
//  Resets the text output device
//
// Input:
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN BOOLEAN ExtendedVerification - indicates that the driver should preform more
//			exhausted verification of the device during reset
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS	- device reset properly
//	    EFI_DEVICE_ERROR - Device is not functioning properly
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  GCReset(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN BOOLEAN                         ExtendedVerification
)
{
	// Set mode clears the screen and sets the cursor back to (0,0) So before 
	//	we do that, set the Attribute colors to default
	This->SetAttribute(This, EFI_BACKGROUND_BLACK | EFI_WHITE);
	This->SetMode(This, 0);
	This->EnableCursor(This, TRUE);
	return	EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCOutputString
//
// Description: 
//  Writes a string to the output device and advances the cursor 
//	as necessary.
//
// Input:
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN CHAR16 *String - pointer to string to be displayed to the screen
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS	- device reset properly
//	    EFI_DEVICE_ERROR - Device reported an Error while outputting a string
//	    EFI_UNSUPPORTED - The output device's mode is not currently in a defined state
//	    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the 
//          characters in the unicode string could not be rendered and were skipped
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GCOutputString(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN CHAR16                          *String
)
{
	EFI_STATUS		                Status;
	GC_DATA			                *GcData;
    BOOLEAN                         UnknownGlyph = FALSE;
    BOOLEAN                         CursorVisible;
    EFI_FONT_DISPLAY_INFO   FontInfo;
    EFI_IMAGE_OUTPUT        *Glyph = NULL;
	
	// find private data structure	
	GcData = (GC_DATA *) This;	

    //retreive colors
    GetColorFromAttribute(
                        (GcData->SimpleTextOut.Mode)->Attribute,
                        &FontInfo.ForegroundColor,
                        &FontInfo.BackgroundColor);
    //use system font
    FontInfo.FontInfoMask =   EFI_FONT_INFO_SYS_FONT 
                            | EFI_FONT_INFO_SYS_SIZE 
                            | EFI_FONT_INFO_SYS_STYLE;
    CursorVisible = (GcData->SimpleTextOut.Mode)->CursorVisible;
	if (CursorVisible)    
        This->EnableCursor(This, FALSE);
	
	// now loop through the string and display it to the output device
	while (*String != 0)
	{
		switch (*String)
		{
			case CARRIAGE_RETURN:
                This->SetCursorPosition(
                                    This, 
                                    0, 
                                    (GcData->SimpleTextOut.Mode)->CursorRow);
				break;

			case LINE_FEED:
                if((GcData->SimpleTextOut.Mode)->CursorRow == (GcData->MaxRows - 1))
                {
                    ScrollUp(GcData);
                    //cursor position not changed, but image under it does - save new image
                    SaveCursorImage(GcData);
                }
                else
                    This->SetCursorPosition(
                                    This, 
                                    (GcData->SimpleTextOut.Mode)->CursorColumn, 
                                    (GcData->SimpleTextOut.Mode)->CursorRow + 1);
				break;
			
			case BACKSPACE:
                if((GcData->SimpleTextOut.Mode)->CursorColumn != 0)
                    This->SetCursorPosition(
                                    This, 
                                    (GcData->SimpleTextOut.Mode)->CursorColumn - 1, 
                                    (GcData->SimpleTextOut.Mode)->CursorRow);
				break;
			
			default:
                Status = GcData->HiiFont->GetGlyph(
                                                GcData->HiiFont,
                                                *String,
                                                &FontInfo,
                                                &Glyph,
                                                0);
                if(EFI_ERROR(Status) || Glyph==NULL)
                    break;

                if(Status == EFI_WARN_UNKNOWN_GLYPH){
                	 UnknownGlyph = TRUE;
                	 if (( Glyph->Width != WIDE_GLYPH_WIDTH) && (Glyph->Width != NARROW_GLYPH_WIDTH))
                		 Glyph->Width = NARROW_GLYPH_WIDTH; // In case of incorrect  Glyph->Width returned
                }


                if((GcData->SimpleTextOut.Mode)->CursorColumn == GcData->MaxColumns - 1 &&
                   Glyph->Width == WIDE_GLYPH_WIDTH)
                    ShiftCursor(GcData, 1);     //can't draw wide glyph on last column in the row


                GcData->OemUpdateBltBuffer(GcData, Glyph->Width, Glyph->Height, Glyph->Image.Bitmap);

                GcData->GraphicsOutput->Blt(
                    GcData->GraphicsOutput,
                    Glyph->Image.Bitmap,
                    EfiBltBufferToVideo,
                    0,
                    0,                   
                    GcData->DeltaX + (GcData->SimpleTextOut.Mode)->CursorColumn * NARROW_GLYPH_WIDTH,
                    GcData->DeltaY + (GcData->SimpleTextOut.Mode)->CursorRow * EFI_GLYPH_HEIGHT,
                    Glyph->Width,
                    Glyph->Height,
                    Glyph->Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));     

                ShiftCursor(GcData, (Glyph->Width / NARROW_GLYPH_WIDTH));
                pBS->FreePool(Glyph->Image.Bitmap);
                pBS->FreePool(Glyph);
                Glyph = NULL; 
                
				break; // end of default case
		}

		String++;
	}
	if (CursorVisible)    
        This->EnableCursor(This, TRUE);

	return (UnknownGlyph) ? EFI_WARN_UNKNOWN_GLYPH : EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCTestString
//
// Description: 
//  Tests to see if the String has the glyphs that correspond to
//	each character in the string
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN CHAR16 *String - pointer to a string that needs to be tested
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - all characters can be drawn
//	    EFI_UNSUPPORTED - there are characters that cannot be drawn
//		
// Notes:
//	Uses the HII function TestString
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  GCTestString(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN CHAR16                          *String
)
{
	GC_DATA		*GcData;
    EFI_STATUS  Status;
    EFI_IMAGE_OUTPUT *Glyph = NULL;
	
	// find private data structure	
	GcData = (GC_DATA *) This;	
	
	// now loop through the string
	while (*String != 0)
	{
		switch (*String)
		{
			case CARRIAGE_RETURN: case LINE_FEED: case BACKSPACE:
				break;
			default:
                Status = GcData->HiiFont->GetGlyph(
                                                GcData->HiiFont,
                                                *String,
                                                NULL,
                                                &Glyph,
                                                0);
                if(EFI_ERROR(Status))
                    return Status;

                pBS->FreePool(Glyph->Image.Bitmap);
                pBS->FreePool(Glyph);
                if(Status == EFI_WARN_UNKNOWN_GLYPH)
                    return EFI_UNSUPPORTED;
                Glyph = NULL; 
				break;
		}
		String++;
	}
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCQueryMode
//
// Description: 
//  Returns information for an available text mode that the output
//	device supports
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN UINTN ModeNum - The mode to return information on
//	OUT UINTN *Col - the number of columns supported
//	OUT UINTN *Row - the number of rows supported
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS	- device reset properly
//	    EFI_DEVICE_ERROR - Device reported an Error while outputting a string
//	    EFI_UNSUPPORTED - The ModeNumber is not supported
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GCQueryMode(
	IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN  UINTN                           ModeNum,
	OUT UINTN                           *Col,
	OUT UINTN                           *Row
)
{
	GC_DATA		*GcData;
    INT32      i = 0;
	
	GcData = (GC_DATA *) This;	

    if(ModeNum >= (UINTN)(GcData->SimpleTextOut.Mode)->MaxMode)
        return EFI_UNSUPPORTED;

    while(i<MaxTextMode && GcData->SupportedModes[i].ModeNum != ModeNum)
        i++;

    if(i>=MaxTextMode || !GcData->SupportedModes[i].Supported)
        return EFI_UNSUPPORTED;

	// if the mode is a valid mode, return the data from the array of
	//	for the height and width
	*Col = GcData->SupportedModes[i].Col;
	*Row = GcData->SupportedModes[i].Row;
	
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCSetMode
//
// Description: 
//  Sets the text mode to the requested mode.  It checks to see if
//	it is a valid mode
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN UINTN ModeNum - mode number to change to
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - the new mode is valid and has been set
//	    EFI_UNSUPPORTED - the new mode is not valid
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GCSetMode(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN UINTN                           ModeNum
)
{
	EFI_STATUS	Status;
	GC_DATA		*GcData;
    BOOLEAN     CursorVisible;
    INT32      i = 0;
    UINT32      DeltaX, DeltaY;
    UINT32      SaveX, SaveY;

	GcData = (GC_DATA *) This;	

    if(ModeNum >= (UINTN)(GcData->SimpleTextOut.Mode)->MaxMode)
        return EFI_UNSUPPORTED;

    while(i<MaxTextMode && GcData->SupportedModes[i].ModeNum != ModeNum)
        i++;

    if(i>=MaxTextMode || !GcData->SupportedModes[i].Supported)
        return EFI_UNSUPPORTED;

    SaveX = GcData->DeltaX;
    SaveY = GcData->DeltaY;

    DeltaX = (GcData->SupportedModes[i].VideoCol - 
              GcData->SupportedModes[i].Col * NARROW_GLYPH_WIDTH) / 2;
    DeltaY = (GcData->SupportedModes[i].VideoRow - 
              GcData->SupportedModes[i].Row * EFI_GLYPH_HEIGHT) / 2;

    //save cursor status and hide it if nesessary
    CursorVisible = (GcData->SimpleTextOut.Mode)->CursorVisible;
	if (CursorVisible)
		This->EnableCursor(This, FALSE);

    if(GcData->GraphicsOutput->Mode->Mode != GcData->SupportedModes[i].GraphicsMode)
    {
        Status = GcData->GraphicsOutput->SetMode(GcData->GraphicsOutput,
                                                 GcData->SupportedModes[i].GraphicsMode);
        if(EFI_ERROR(Status))
            return Status;

        if(GcData->OemClearScreen != GcInternalClearScreen) //we have porting hook installed - call it
            This->ClearScreen(This);
        else
        	GcData->OemClearScreen(GcData);
    }
    else
        This->ClearScreen(This);    //call clear screen with old values

    if(GcData->DeltaX == SaveX && GcData->DeltaY == SaveY)
    {                               //initial position was not changed by porting hook inside C
                                    //ClearScreen function
        GcData->DeltaX = DeltaX;
        GcData->DeltaY = DeltaY;
    }

    GcData->MaxColumns = GcData->SupportedModes[i].Col;
    GcData->MaxRows = GcData->SupportedModes[i].Row;
    (GcData->SimpleTextOut.Mode)->Mode = GcData->SupportedModes[i].ModeNum;

    // Set the cursor position to 0,0
    GCSetCursorPosition(This, 0, 0);

	//  restore cursor at new position
	if (CursorVisible)
		This->EnableCursor(This, TRUE);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCSetAttribute
//
// Description: 
//  Sets the foreground color and background color for the screen
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN UINTN Attribute - the attributes to set
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - the attribute was changed successfully
//	    EFI_DEVICE_ERROR - The device had an error
//	    EFI_UNSUPPORTED - The attribute is not supported
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  GCSetAttribute(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN UINTN                           Attribute
)
{
	GC_DATA	*GcData;
	
	GcData = (GC_DATA *) This;
	
	(GcData->SimpleTextOut.Mode)->Attribute = (INT32) Attribute;

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCClearScreen
//
// Description: 
//  Clears the text screen
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - the screen was cleared
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  GCClearScreen(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
)
{
	GC_DATA			              *GcData;
	INT32 i=0;

	GcData = (GC_DATA *) This;
	while(i<MaxTextMode && GcData->SupportedModes[i].ModeNum != (GcData->SimpleTextOut.Mode)->Mode)
		i++;
	if(i<MaxTextMode && GcData->GraphicsOutput->Mode->Mode != GcData->SupportedModes[i].GraphicsMode)
		GcData->GraphicsOutput->SetMode(
			GcData->GraphicsOutput, GcData->SupportedModes[i].GraphicsMode
		);

    GcData->OemClearScreen(GcData);

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCSetCursorPosition
//
// Description: 
//  This function sets the position of the cursor
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN UINTN Column - the new column
//	IN UINTN Row - The new row
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - the cursor position was changed
//	    EFI_DEVICE_ERROR - The device had an error
//	    EFI_UNSUPPORTED - The device is not in a valid text mode or the 
//	                      cursor position is not valid
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GCSetCursorPosition(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN UINTN                           Column,
	IN UINTN                           Row
)
{
	GC_DATA	*GcData;
    BOOLEAN CursorVisible;
	
	GcData = (GC_DATA *) This;
	
	// check for a valid text mode and check for a valid position 
	//	on the screen
	
	if ( ((UINT32)Column >= GcData->MaxColumns) || 
		 ((UINT32)Row >= GcData->MaxRows) )
		return EFI_UNSUPPORTED;
		
	
    //save cursor status and hide it if nesessary
    CursorVisible = (GcData->SimpleTextOut.Mode)->CursorVisible;
	if (CursorVisible)
		This->EnableCursor(This, FALSE);
	
	(GcData->SimpleTextOut.Mode)->CursorColumn = (INT32)Column;
	(GcData->SimpleTextOut.Mode)->CursorRow = (INT32)Row;

	
	//  restore cursor at new position
	if (CursorVisible)
		This->EnableCursor(This, TRUE);
	
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GCEnableCursor
//
// Description: 
//  Makes cursor invisible or visible
//
// Input: 
//	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to the protocol instance
//	IN BOOLEAN Visible - a boolean that if TRUE the cursor will be visible
//	if FALSE the cursor will be invisible
//
// Output:
//  EFI_STATUS
//	    EFI_SUCCESS - the cursor visibility was set correctly
//	    EFI_DEVICE_ERROR - The device had an error
//	    EFI_UNSUPPORTED - The device does not support visibilty control
//	                      for the cursor
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  GCEnableCursor(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN BOOLEAN                         Visible
)
{
	GC_DATA	*GcData;
	
	GcData = (GC_DATA *) This;

	// check to see if the we are already set to the same cursor visibility mode
	if (Visible != (GcData->SimpleTextOut.Mode)->CursorVisible)
	{
		(GcData->SimpleTextOut.Mode)->CursorVisible = Visible;
//if we put cursor back we have to update image under it in order it contains older data
        if(Visible)
            SaveCursorImage(GcData);

		DrawCursor(GcData, Visible);
	}
	
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GetColorFromAttribute
//
// Description: 
//  Turns color attributes into Pixel values
//
// Input: 
//	IN  UINT32 Attribute - The color to set
//	OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Foreground - foreground color
//	OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Background - background color
//
// Output:
//  EFI_STATUS
//      EFI_SUCCESS - valid colors returned
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetColorFromAttribute(
    IN  UINT32                         Attribute, 
    OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Foreground, 
    OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Background
)
{
	UINT8 IndexF;
	UINT8 IndexB;

    IndexF = (UINT8)(Attribute & 0x0f);
    IndexB = (UINT8)((Attribute >> 4) & 0x0f);

	*Foreground = ColorArray[IndexF];
	*Background = ColorArray[IndexB];

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: DrawCursor
//
// Description: 
//  This function draws /hides the cursor in the current cursor position
//
// Input: 
//	IN GC_DATA * GcData - Private data structure for SimpleTextOut interface
//  IN BOOLEAN Visible - if TRUE - draws cursor, if FALSE - hides cursor
//
// Output:
//  VOID
//
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID DrawCursor(
    IN GC_DATA *GcData, 
    IN BOOLEAN Visible
)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Fill;

    if(Visible)
    {
        Fill = ColorArray[((GcData->SimpleTextOut.Mode)->Attribute & 0x0f)];    //get foreground color
        GcData->GraphicsOutput->Blt(
            GcData->GraphicsOutput,
            &Fill,
            EfiBltVideoFill,
            0,
            0,                   
            GcData->DeltaX + (GcData->SimpleTextOut.Mode)->CursorColumn * NARROW_GLYPH_WIDTH,
            GcData->DeltaY + (GcData->SimpleTextOut.Mode)->CursorRow * EFI_GLYPH_HEIGHT + CURSOR_OFFSET,
            NARROW_GLYPH_WIDTH,
            CURSOR_THICKNESS,
            0); 
    }
    else
    {
        GcData->GraphicsOutput->Blt(
            GcData->GraphicsOutput,
            GcData->Cursor,
            EfiBltBufferToVideo,
            0,
            0,                   
            GcData->DeltaX + (GcData->SimpleTextOut.Mode)->CursorColumn * NARROW_GLYPH_WIDTH,
            GcData->DeltaY + (GcData->SimpleTextOut.Mode)->CursorRow * EFI_GLYPH_HEIGHT + CURSOR_OFFSET,
            NARROW_GLYPH_WIDTH,
            CURSOR_THICKNESS,
            NARROW_GLYPH_WIDTH * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)); 
    }                                   
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: BlinkCursorEvent
//
// Description: 
//  This is the function that makes the cursor blink. A timer event 
//	is created that will cause this function to be called
//
// Input: 
//	IN EFI_EVENT Event - event that was triggered
//  IN VOID *Context - pointer to the event context
//
// Output:
//  VOID
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EFIAPI BlinkCursorEvent(
	IN EFI_EVENT Event,
	IN VOID      *Context
)
{
	GC_DATA	*GcData;
	
	GcData = (GC_DATA *) Context;

	if (!(GcData->SimpleTextOut.Mode)->CursorVisible)
		return;

	if (GcData->BlinkVisible)
	{
		// remove the cursor from the screen
		DrawCursor(GcData, FALSE);
        GcData->BlinkVisible = FALSE;
	}
	else		
	{
		// put cursor back
		DrawCursor(GcData, TRUE);
        GcData->BlinkVisible = TRUE;
	}
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: GetGraphicsModeNumber
//
// Description: 
//  This function returns graphics mode number, correspondend with given
//  horizontal and vertical resolution. If either HorRes or VerRes are equal
//  to MAX_RES, the highest supported resolution will be returned.
//
// Input: 
//	IN EFI_GRAPHICS_OUTPUT_PROTOCOL	*GraphicsOutput - pointer to Gop driver
//  IN UINT32 HorRes - required horizontal resolution
//  IN UINT32 VerRes - required vertical resolution
//  OUT UINT32 *ModeNum - returned graphics mode number
//  IN  BOOLEAN ExactMatch - TRUE indicates that only an exact match should
//                           succeed. FALSE indicates that if a higher
//                           resolution is supported, it can be substituted.
//  OUT UINT32 *ActualHorRes - On return, holds the actual horizontal
//                             resolution that was found to match
//  OUT UINT32 *ActualVerRes - On return, holds the actual vertical
//                             resolution that was found to match
//      
// Output: 
//  EFI_STATUS
//      EFI_SUCCESS - correct mode number returned
//      EFI_NOT_FOUND - mode number not found for given resolution
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetGraphicsModeNumber (
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput, 
    IN  UINT32                       HorRes,
    IN  UINT32                       VerRes,
	OUT UINT32                       *ModeNum,
    IN  BOOLEAN                      ExactMatch,
    OUT UINT32                       *ActualHorRes OPTIONAL,
    OUT UINT32                       *ActualVerRes OPTIONAL
)
{
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	Info;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION	*pInfo = &Info;
	EFI_STATUS				                Status;
	UINTN							        SizeOfInfo;
	UINT32							        i;
    UINT32                                  MaxHorizontalResolution = 0;
    UINT32                                  MaxVerticalResolution = 0;
    UINT32                                  MaxModeNumber = 0;

	for(i = 0; i < GraphicsOutput->Mode->MaxMode; i++) {
		Status = GraphicsOutput->QueryMode(GraphicsOutput, i, &SizeOfInfo, &pInfo);

		if (!EFI_ERROR(Status)) { 
            if (HorRes == MAX_RES || 
                VerRes == MAX_RES )
            {
                if (pInfo->HorizontalResolution >= MaxHorizontalResolution &&
                    pInfo->VerticalResolution >= MaxVerticalResolution) {
                        MaxHorizontalResolution = pInfo->HorizontalResolution;
                        MaxVerticalResolution = pInfo->VerticalResolution;
                        MaxModeNumber = i;
                    }
                continue;
            }
            if (ExactMatch && 
                pInfo->HorizontalResolution == HorRes && 
                pInfo->VerticalResolution == VerRes )
            {
			    MaxHorizontalResolution = pInfo->HorizontalResolution;
                MaxVerticalResolution = pInfo->VerticalResolution;
                MaxModeNumber = i;
                break;
            }
            if(!ExactMatch && 
                pInfo->HorizontalResolution >= HorRes && 
                pInfo->VerticalResolution >= VerRes )
            {
			    MaxHorizontalResolution = pInfo->HorizontalResolution;
                MaxVerticalResolution = pInfo->VerticalResolution;
                MaxModeNumber = i;
                break;
            }
		}
	}
    
    if (MaxHorizontalResolution == 0 || MaxVerticalResolution == 0)
        return EFI_NOT_FOUND;
        
    if (ActualHorRes != NULL)
        *ActualHorRes = MaxHorizontalResolution;
    if (ActualVerRes != NULL)
        *ActualVerRes = MaxVerticalResolution;
    *ModeNum = MaxModeNumber;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: SetupGraphicsDevice
//
// Description: 
//  This function fills array of supported text modes
//
// Input: 
//	IN GC_DATA *Data - pointer to private protocol data structure
//      
// Output: 
//  EFI_STATUS
//      EFI_SUCCESS - function executed successfully
//      EFI_UNSUPPORTED - no supported modes found
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SetupGraphicsDevice(
    IN GC_DATA *GcData
)
{
    EFI_STATUS Status;
    INT32 i;
    UINT32 GraphicsModeNumber;
    INT32 MaxSupportedModes = 1;
    BOOLEAN DefaultModeNotSupported = FALSE;
    INT32 ModeZeroIndex = 0;
    UINT32 VideoCol, VideoRow;

    for(i = 0; i < MaxTextMode; i++) {
        if (TextModeArray[i].VideoCol == MIN_RES ||
            TextModeArray[i].VideoRow == MIN_RES)
        {
            // Get the minimum video resolution that supports YxZ rows x columns
            Status = GetGraphicsModeNumber (
                GcData->GraphicsOutput,
                TextModeArray[i].Col * NARROW_GLYPH_WIDTH,
                TextModeArray[i].Row * EFI_GLYPH_HEIGHT,
                &GraphicsModeNumber,
                FALSE,
                &VideoCol,
                &VideoRow
            );
        } else
        {
            // Get the requested video resolution
            Status = GetGraphicsModeNumber (
                GcData->GraphicsOutput,
                TextModeArray[i].VideoCol,
                TextModeArray[i].VideoRow,
                &GraphicsModeNumber,
                TRUE,
                &VideoCol,
                &VideoRow
            );
        }
        
        if(!EFI_ERROR(Status)) {
            GcData->SupportedModes[i].Supported = TRUE;
            GcData->SupportedModes[i].GraphicsMode = GraphicsModeNumber;
            GcData->SupportedModes[i].ModeNum = TextModeArray[i].ModeNum;
            
            if( TextModeArray[i].Col == MAX_RES || 
                TextModeArray[i].Col == MIN_RES )
                // Calculate the optimal number of text columns based on horizontal resolution
                GcData->SupportedModes[i].Col = VideoCol / NARROW_GLYPH_WIDTH;
            else
                GcData->SupportedModes[i].Col = TextModeArray[i].Col;

            if( TextModeArray[i].Row == MAX_RES ||
                TextModeArray[i].Row == MIN_RES )
                // Calculate the optimal number of text rows based on vertical resolution
                GcData->SupportedModes[i].Row = VideoRow / EFI_GLYPH_HEIGHT;
            else
                GcData->SupportedModes[i].Row = TextModeArray[i].Row;

            // Save the video resolution for this mode
            GcData->SupportedModes[i].VideoCol = VideoCol;
            GcData->SupportedModes[i].VideoRow = VideoRow;

            MaxSupportedModes = (TextModeArray[i].ModeNum >= MaxSupportedModes) ? 
                                 TextModeArray[i].ModeNum + 1 : MaxSupportedModes;
            
            // Ensure there are enough pixels for the number of rows/cols
            if (VideoCol / NARROW_GLYPH_WIDTH < (UINT32)GcData->SupportedModes[i].Col ||
                VideoRow / EFI_GLYPH_HEIGHT < (UINT32)GcData->SupportedModes[i].Row)
                GcData->SupportedModes[i].Supported = FALSE;
        } else {
            GcData->SupportedModes[i].Supported = FALSE;
            GcData->SupportedModes[i].ModeNum = TextModeArray[i].ModeNum;

            if(TextModeArray[i].ModeNum == 0) {
                ModeZeroIndex = i;
                DefaultModeNotSupported = TRUE;
            }
        }
    }

    if(DefaultModeNotSupported) {
        // Fallback to minimum specified by UEFI spec
        Status = GetGraphicsModeNumber(GcData->GraphicsOutput,
                                       MODE_ZERO_MIN_HOR_RES,
                                       MODE_ZERO_MIN_VER_RES,
                                       &GraphicsModeNumber,
                                       FALSE,
                                       &(GcData->SupportedModes[ModeZeroIndex].VideoCol),
                                       &(GcData->SupportedModes[ModeZeroIndex].VideoRow));
        if(EFI_ERROR(Status)) {
            (GcData->SimpleTextOut.Mode)->MaxMode = 0;
            return EFI_UNSUPPORTED;
        }

        GcData->SupportedModes[ModeZeroIndex].Supported = TRUE;
        GcData->SupportedModes[ModeZeroIndex].GraphicsMode = GraphicsModeNumber;
        GcData->SupportedModes[ModeZeroIndex].Row = 25;
        GcData->SupportedModes[ModeZeroIndex].Col = 80;
		GcData->SupportedModes[ModeZeroIndex].ModeNum = TextModeArray[ModeZeroIndex].ModeNum;
    }

    (GcData->SimpleTextOut.Mode)->MaxMode = MaxSupportedModes;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: ScrollUp
//
// Description: 
//  This function scrolls screen one row up and clears bottom row
//
// Input:       
//  IN GC_DATA *GcData - pointer to private protocol data structure
//      
// Output:      
//  VOID
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ScrollUp(
    IN GC_DATA *GcData
)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Fill;

    GcData->GraphicsOutput->Blt(GcData->GraphicsOutput,
                                &Fill,
                                EfiBltVideoToVideo,
                                GcData->DeltaX,
                                GcData->DeltaY + EFI_GLYPH_HEIGHT,                   
                                GcData->DeltaX,
                                GcData->DeltaY,
                                GcData->MaxColumns * NARROW_GLYPH_WIDTH,
                                (GcData->MaxRows - 1) * EFI_GLYPH_HEIGHT,
                                0);
//clear bottom line
    Fill = ColorArray[(((GcData->SimpleTextOut.Mode)->Attribute >> 4) & 0xf)];
    GcData->GraphicsOutput->Blt(GcData->GraphicsOutput,
                                &Fill,
                                EfiBltVideoFill,
                                0,
                                0,                   
                                GcData->DeltaX,
                                GcData->DeltaY + (GcData->MaxRows - 1) * EFI_GLYPH_HEIGHT,
                                GcData->MaxColumns * NARROW_GLYPH_WIDTH,
                                EFI_GLYPH_HEIGHT,
                                0);
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name:    SaveCursorImage
//
// Description: 
//  This function saves image under cursor to restore, when cursor moves
//
// Input:       
//  IN GC_DATA *GcData - pointer to private protocol data structure
//      
// Output:      
//  VOID
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SaveCursorImage(
    IN GC_DATA *GcData
)
{
    GcData->GraphicsOutput->Blt(
            GcData->GraphicsOutput,
            GcData->Cursor,
            EfiBltVideoToBltBuffer,
            GcData->DeltaX + (GcData->SimpleTextOut.Mode)->CursorColumn * NARROW_GLYPH_WIDTH,
            GcData->DeltaY + (GcData->SimpleTextOut.Mode)->CursorRow * EFI_GLYPH_HEIGHT + CURSOR_OFFSET,
            0,
            0,                   
            NARROW_GLYPH_WIDTH,
            CURSOR_THICKNESS,
            NARROW_GLYPH_WIDTH * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
}    

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name: ShiftCursor
//
// Description: 
//  This function shifts cursor right to number of columns defined in Step
//  If cursor reaches right edge of the screen it moves one line down, scrolling screen
//  if nesessary
//
// Input: 
//	IN GC_DATA *GcData - pointer to private protocol data structure
//  IN UINT16 Step - number of columns to shift cursor right
//      
// Output: 
//  VOID
//		
// Notes:
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ShiftCursor(
    IN GC_DATA *GcData,
    IN UINT16  Step
)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This = (EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *)GcData;

    if(((GcData->SimpleTextOut.Mode)->CursorColumn + Step) >= (INT32)GcData->MaxColumns)
    {
        if((GcData->SimpleTextOut.Mode)->CursorRow == GcData->MaxRows - 1)
        {
            ScrollUp(GcData);
            This->SetCursorPosition(
                              This, 
                              0, 
                              (GcData->SimpleTextOut.Mode)->CursorRow);
        }
        else
        {
            This->SetCursorPosition(
                              This, 
                              0, 
                              (GcData->SimpleTextOut.Mode)->CursorRow + 1);
        }
    }
    else
    {
        This->SetCursorPosition(
                              This, 
                              (GcData->SimpleTextOut.Mode)->CursorColumn + Step, 
                              (GcData->SimpleTextOut.Mode)->CursorRow);
    }
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name:    GcUpdateBltBuffer
//
// Description: 
//  This function is a porting hook to implement specific action on
//  Blt buffer before put it on screen
//
// Input:       
//  IN GC_DATA *GcData - pointer to internal structure
//  IN UINT32 Width - width of passed buffer in pixels
//  IN UINT32 Height - height of passed buffer in pixels
//  IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer - pointer to Blt buffer
//              to perform action upon
//      
// Output:
//  VOID
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID GcUpdateBltBuffer (
	IN     GC_DATA 			             *This,
	IN     UINT32			             Width,
    IN     UINT32                        Height,
	IN OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer
)
{
    return;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Name:    GcInternalClearScreen
//
// Description: 
//  This function is a porting hook to implement specific action when
//  clear screen operation is needed
//
// Input:       
//  IN GC_DATA *This - pointer to private protocol data structure
//      
// Output:      
//  VOID
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID GcInternalClearScreen (
	IN OUT GC_DATA *This
)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Fill;
    EFI_STATUS Status;
    UINT32 StartX;
    UINT32 StartY;
    UINT32 SizeX;
    UINT32 SizeY;

    if(This->MaxColumns == 0 || This->MaxRows == 0) { //this is the first invocation
        StartX = 0;
        StartY = 0;
        SizeX = This->GraphicsOutput->Mode->Info->HorizontalResolution;
        SizeY = This->GraphicsOutput->Mode->Info->VerticalResolution;
    } else {
        StartX = This->DeltaX;
        StartY = This->DeltaY;
        SizeX = This->MaxColumns * NARROW_GLYPH_WIDTH;
        SizeY = This->MaxRows * EFI_GLYPH_HEIGHT;
    }

    Fill = ColorArray[(((This->SimpleTextOut.Mode)->Attribute >> 4) & 0xf)];
    This->GraphicsOutput->Blt(This->GraphicsOutput,
                                &Fill,
                                EfiBltVideoFill,
                                0,
                                0,                   
                                StartX,
                                StartY,
                                SizeX,
                                SizeY,
                                0);

	Status = This->SimpleTextOut.SetCursorPosition(&(This->SimpleTextOut), 0, 0);
    if(EFI_ERROR(Status)) { //on first invocation this failed because MaxRows = MaxCols = 0
        (This->SimpleTextOut.Mode)->CursorColumn = 0;
	    (This->SimpleTextOut.Mode)->CursorRow = 0;
    }
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
