//**********************************************************************
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
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
// $Header: /Alaska/BIN/Core/Library/Tokens.c 19    4/27/11 4:47a Lavanyap $
//
// $Revision: 19 $
//
// $Date: 4/27/11 4:47a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	Tokens.c
//
// Description:	
//    This file contains constants and variables corresponding to some of the SDL tokens.
//    The constants and variables are used by modules that are distributed in binary 
//    and cannot directly consume token.h
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Ffs.h>
#include <AmiLib.h>
#include <Token.h>

// Constants
/*
const char *sAmiRomFile = CONVERT_TO_STRING(AMI_ROM);

const UINTN FlashSize = FLASH_SIZE;
*/
#ifdef FLASH_ERASE_POLARITY
const UINT32 FlashBlockSize = FLASH_BLOCK_SIZE;
/*
const UINT32 FlashRetries = FLASH_RETRIES;
*/
const UINTN FlashEmpty = (UINTN)(-FLASH_ERASE_POLARITY);
const UINT32 FlashEmptyNext = (FLASH_ERASE_POLARITY ? 0xffffff : 0);
#ifdef NVRAM_ADDRESS
const UINTN NvramAddress = NVRAM_ADDRESS;
#endif
#endif
#if FAULT_TOLERANT_NVRAM_UPDATE
const UINTN NvramBackupAddress = NVRAM_BACKUP_ADDRESS;
#else
const UINTN NvramBackupAddress = 0;
#endif
//	#if RUNTIME_TRACE_SUPPORT
//	const BOOLEAN HideComPort = TRUE;
//	#else
const BOOLEAN HideComPort = FALSE;
//	#endif
#ifdef FLASH_ERASE_POLARITY
#ifdef NVRAM_SIZE
const UINTN NvramSize = NVRAM_SIZE;
#endif
#ifdef NV_SIMULATION
const BOOLEAN NvSimulation = NV_SIMULATION;
#endif
#ifdef NVRAM_HEADER_SIZE
const UINT32 NvramHeaderLength = NVRAM_HEADER_SIZE;
#endif
#ifdef NVRAM_MONOTONIC_COUNTER_SUPPORT
const BOOLEAN NvramMonotonicCounterSupport = NVRAM_MONOTONIC_COUNTER_SUPPORT;
#endif
#ifdef NVRAM_RECORD_CHECKSUM_SUPPORT
const BOOLEAN NvramChecksumSupport = NVRAM_RECORD_CHECKSUM_SUPPORT;
#endif
#ifdef DEFAULT_LANGUAGE_CODE
const char *DefaultLanguageCode = CONVERT_TO_STRING(DEFAULT_LANGUAGE_CODE);
#endif
/*const UINT32 TraceLevelMask = TRACE_LEVEL_MASK;
*/
// Variables
#define FLASH_DEVICE_BASE_ADDRESS (FLASH_BASE)
UINTN FlashDeviceBase = FLASH_DEVICE_BASE_ADDRESS;
UINTN FwhFeatureSpaceBase = FLASH_DEVICE_BASE_ADDRESS & ~(UINTN)0x400000;
#endif
#if NVRAM_RT_GARBAGE_COLLECTION_SUPPORT
const BOOLEAN NvramRtGarbageCollectionSupport = NVRAM_RT_GARBAGE_COLLECTION_SUPPORT;
#else
const BOOLEAN NvramRtGarbageCollectionSupport = 0;
#endif
#if NO_MMIO_FLASH_ACCESS_DURING_UPDATE
const BOOLEAN FlashNotMemoryMapped = NO_MMIO_FLASH_ACCESS_DURING_UPDATE;
#else
const BOOLEAN FlashNotMemoryMapped = 0;
#endif
/*
#ifdef USE_RECOVERY_IMAGE_ON_FLASH_UPDATE
const BOOLEAN UseNewImage = USE_RECOVERY_IMAGE_ON_FLASH_UPDATE;
#else
const BOOLEAN UseNewImage = TRUE;
#endif

#ifdef FORCE_SETUP_ON_FAILED_RECOVERY
const BOOLEAN ForceSetupOnFailedRecovery = FORCE_SETUP_ON_FAILED_RECOVERY;
#else
const BOOLEAN ForceSetupOnFailedRecovery = TRUE;
#endif

typedef struct _TEXT_MODE  {
	INT32	ModeNum;
	INT32	Col;
	INT32	Row;
	UINT32	VideoCol; // horizontal pixels
	UINT32	VideoRow; // vertical pixels
} TEXT_MODE;

const TEXT_MODE TextModeArray[] = {GC_MODE_LIST};
const INT32 MaxTextMode=(sizeof(TextModeArray)/sizeof(TEXT_MODE));


//Ps2Ctl driver global variables

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbdIrqSupport
//
// Description:	Variable to replace KB_IRQ_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbdIrqSupport = 
#ifdef KB_IRQ_SUPPORT
    KB_IRQ_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    MsIrqSupport
//
// Description:	Variable to replace MS_IRQ_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN MsIrqSupport = 
#ifdef MS_IRQ_SUPPORT
    MS_IRQ_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    Ps2MouseSupport
//
// Description:	Variable to replace PS2MOUSE_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN Ps2MouseSupport = 
#ifdef PS2MOUSE_SUPPORT
    PS2MOUSE_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    Ps2KbdSupport
//
// Description:	Variable to replace PS2KBD_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN Ps2KbdSupport = 
#ifdef PS2KBD_SUPPORT
PS2KBD_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbRdBeforeInstall
//
// Description:	Variable to replace KBD_READ_BEFORE_INSTALL token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbRdBeforeInstall = 
#ifdef KBD_READ_BEFORE_INSTALL
    KBD_READ_BEFORE_INSTALL
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbcAutoDetectPorts
//
// Description:	Variable to replace KBC_AUTODETECT_PORTS token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbcAutoDetectPorts = 
#ifdef KBC_AUTODETECT_PORTS
KBC_AUTODETECT_PORTS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    BlockKbcPin2223Bit
//
// Description:	Variable to replace BLOCK_KBC_PIN_22_23_BIT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN BlockKbcPin2223Bit = 
#ifdef BLOCK_KBC_PIN_22_23_BIT
BLOCK_KBC_PIN_22_23_BIT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    LedsAtStartup
//
// Description:	Variable to replace LEDS_AT_STARTUP token.
//
// Notes: UINT8
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT8 LedsAtStartup = 
#ifdef LEDS_AT_STARTUP
LEDS_AT_STARTUP
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    MaxHotKeys
//
// Description:	Variable to replace MAX_HOTKEYS token.
//
// Notes: UINT8
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT8 MaxHotKeys = 
#ifdef MAX_HOTKEYS
MAX_HOTKEYS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    InitDefaultHotKeys
//
// Description:	Variable to replace INIT_DEFAULT_HOTKEYS token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN InitDefaultHotKeys = 
#ifdef INIT_DEFAULT_HOTKEYS
INIT_DEFAULT_HOTKEYS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbcBasicAssuranceTest
//
// Description:	Variable to replace KBC_BASIC_ASSURANCE_TEST token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbcBasicAssuranceTest = 
#ifdef KBC_BASIC_ASSURANCE_TEST
KBC_BASIC_ASSURANCE_TEST
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    DetectPs2KeyboardValue
//
// Description:	Variable to replace DETECT_PS2_KEYBOARD token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN DetectPs2KeyboardValue = 
#ifdef DETECT_PS2_KEYBOARD
DETECT_PS2_KEYBOARD
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    DetectPs2MouseValue
//
// Description:	Variable to replace DETECT_PS2_MOUSE token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN DetectPs2MouseValue = 
#ifdef DETECT_PS2_MOUSE
DETECT_PS2_MOUSE
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    InstallKeyboardMouseAlways
//
// Description:	Variable to replace INSTALL_KEYBOARD_MOUSE_ALWAYS token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN InstallKeyboardMouseAlways = 
#ifdef INSTALL_KEYBOARD_MOUSE_ALWAYS
INSTALL_KEYBOARD_MOUSE_ALWAYS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    IbFreeTimeoutValue
//
// Description:	Variable to replace IBFREE_TIMEOUT token.
//
// Notes: UINT32
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT32 IbFreeTimeoutValue = 
#ifdef IBFREE_TIMEOUT
IBFREE_TIMEOUT
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    IbMaxFreeTimeoutValue
//
// Description:	Variable to replace MAXIMUM_TIMEOUT_FOR_IBFREE token.
//
// Notes: UINT32
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT32 IbFreeMaxTimeoutValue = 
#ifdef MAXIMUM_TIMEOUT_FOR_IBFREE
MAXIMUM_TIMEOUT_FOR_IBFREE
#else
    0
#endif
    ;

//Decompression
BOOLEAN GetDecompressInterface(
    UINT8 CompressionType, GET_INFO *GetInfoPtr, DECOMPRESS *DecompressPtr
)
{
    if (CompressionType==EFI_STANDARD_COMPRESSION){
        *GetInfoPtr=GetInfo;
        *DecompressPtr=Decompress;
        return TRUE;
    }
#if LZMA_SUPPORT==1
    else if (CompressionType==EFI_CUSTOMIZED_COMPRESSION){
        *GetInfoPtr=LzmaGetInfo;
        *DecompressPtr=LzmaDecompress;
        return TRUE;
    }
#endif
    return FALSE;
}
*/
const UINT32 TraceLevelMask = 0xFFFFFFFF;
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
