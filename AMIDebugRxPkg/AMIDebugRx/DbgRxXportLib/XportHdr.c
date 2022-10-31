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
// $Header: /AptioV/BIN/AMIDebugRx/DbgRxXportLib/XportHdr.c 1     11/02/12 10:14a Sudhirv $
//
// $Revision: 1 $
//
// $Date: 11/02/12 10:14a $
//*****************************************************************
//*****************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:		XportHdr.c
//
// Description:	File containing the globals for CAR base address & size.
//				Also the code which make use of this so that it can be
//				integrated as it is Debugger eModule of binary.
//
//<AMI_FHDR_END>
//**********************************************************************
#include "efi.h"
#include "Pei.h"

//#include "misc.h"
#ifndef	EFIx64
#include <Library\AMIPeiDebug.h>
#else
#include <Library\AMIPeiDebugX64.h>
#endif
#include <Library\AMIPeiGUIDs.h>

#include "token.h"
#include "timestamp.h"
#include <Library/IoLib.h>

#define CONVERT_TO_STRING(a) #a
#define STR(a) CONVERT_TO_STRING(a)

UINTN	AMI_PEIDEBUGGER_DS1_SIZE = 0x400;

EFI_GUID  mPeiDebugDataGuidXp = PEI_DBGSUPPORT_DATA_GUID;
EFI_GUID  mPeiDbgBasePpiGuidXp = EFI_PEI_DBG_BASEADDRESS_PPI_GUID;
const char *sTargetProjectTag= STR(TARGET_PROJECT_TAG);

#ifdef REDIRECTION_ONLY_MODE
#if REDIRECTION_ONLY_MODE
volatile UINTN gRedirectionOnlyEnabled = 1;
#else
volatile UINTN gRedirectionOnlyEnabled = 0;
#endif
#endif

#ifdef DEBUG_PORT_DETECTION_MODE
UINTN gDebugPortDetection = DEBUG_PORT_DETECTION_MODE;
#else
UINTN gDebugPortDetection = 0;
#endif

#ifdef DEBUG_PORT_DETECTION_TIMEOUT
UINTN gDebugPortDetectionTimeout = DEBUG_PORT_DETECTION_TIMEOUT;
#else
UINTN gDebugPortDetectionTimeout = 0;
#endif

volatile UINTN USB_DEBUGGER_ENABLED = USB_DEBUG_TRANSPORT;

//volatile UINTN gDbgWriteIO80Support = DBG_WRITE_IO_80_SUPPORT;

typedef struct {
	UINT16		Year;
	UINT8		Month;
	UINT8		Day;
	UINT8		Hour;
	UINT8		Minute;
	UINT8		Second;
} DateTime_T;

//<AMI_PHDR_START>
//--------------------------------------------------------------------
// Procedure:	IoRead32
//
// Description:	Internal Helper function. Reads 32-bit value from IO port
//
// Input:		UINT16 Port
//
// Output:		UINT32 - port data
//
//--------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 DbgIoRead32(UINT16 Port)
{
#ifdef	EFIx64
	return IoRead32(Port);
#else
	_asm {
		mov dx, Port
		in eax, dx
	}
#endif
}

//<AMI_PHDR_START>
//--------------------------------------------------------------------
// Procedure:	Stall
//
// Description:	Internal Helper function.
//
// Input:		UINTN Usec (microseconds)
//
// Output:		void
//
//--------------------------------------------------------------------
//<AMI_PHDR_END>
void Stall (UINTN Usec)
{
    UINTN   Counter = (Usec * 7)/2; // 3.58 count per microsec
    UINTN   i;
    UINT32  Data32;
    UINT32  PrevData;

    PrevData = DbgIoRead32(PM_BASE_ADDRESS + 8);
    for (i=0; i < Counter; ) {
       Data32 = DbgIoRead32(PM_BASE_ADDRESS + 8);
        if (Data32 < PrevData) {        // Reset if there is a overlap
            PrevData=Data32;
            continue;
        }
        i += (Data32 - PrevData);
		PrevData=Data32; // FIX need to find out the real diff betweek different count.
    }
}


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

