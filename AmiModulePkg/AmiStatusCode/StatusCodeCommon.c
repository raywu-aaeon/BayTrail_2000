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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	StatusCode.c
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include <AmiLib.h>
#include <Token.h>
//#include <StatusCodeELinks.h>
#include "StatusCodeInt.h"
#include <Library/SerialPortLib.h>
#include <Library/PrintLib.h>

#pragma pack(1)
typedef struct
{
    UINT32                        ErrorLevel;
    // 12 * sizeof (UINT64) Var Arg stack
    // ascii EFI_DEBUG () Format string
} EFI_DEBUG_INFO;
#pragma pack()

//----------------------------------------------------------------------------
// GUID Definitions
#define EFI_STATUS_CODE_DATA_TYPE_DEBUG_GUID \
          {0x9A4E9246, 0xD553, 0x11D5, 0x87, 0xE2, 0x00, 0x06, 0x29, 0x45, 0xC3, 0xb9}
#define EFI_STATUS_CODE_DATA_TYPE_ASSERT_GUID \
          {0xDA571595, 0x4D99, 0x487C, 0x82, 0x7C, 0x26, 0x22, 0x67, 0x7D, 0x33, 0x07}
static EFI_GUID gEfiStatusCodeDataTypeDebugGuid = EFI_STATUS_CODE_DATA_TYPE_DEBUG_GUID;
static EFI_GUID gEfiStatusCodeDataTypeAssertGuid = EFI_STATUS_CODE_DATA_TYPE_ASSERT_GUID;
EFI_GUID gDataTypeStringGuid = EFI_STATUS_CODE_DATA_TYPE_STRING_GUID;
EFI_GUID gSpecificDataGuid = EFI_STATUS_CODE_SPECIFIC_DATA_GUID;

extern UINT16 StringMaxSize;

//**********************************************************************
// Status Code Library (functions used throughout the module)
//**********************************************************************
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindByteCode
//
// Description: Match "Value" to Map->Value and return Map->Byte.
//
// Input:
//      STATUS_CODE_TO_BYTE_MAP *Map  - Pointer to a table (array of structures) that maps status code value (EFI_STATUS_CODE_VALUE) to a byte (UINT8) domain specific value.
//      EFI_STATUS_CODE_VALUE Value  - status code to map
//
// Output:
//      UINT8 - domain specific mapping of the Value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 FindByteCode(STATUS_CODE_TO_BYTE_MAP *Map, EFI_STATUS_CODE_VALUE Value)
{
    while (Map->Value!=0)
    {
        if (Map->Value==Value)
        {
            return Map->Byte;
        }

        Map++;
    }

    return 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReportAssertString
//
// Description: This function creates a string from the ASSERT data
//
// Input:
//      IN EFI_STATUS_CODE_DATA *Data - Pointer to the beginning of the ASSERT data
//      OUT UINT8 *String - Pointer to the beginning of a byte String array
//
// Output: *String - Contains information from the ASSERT data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReportAssertString(IN EFI_STATUS_CODE_DATA *Data, OUT UINT8 *String)
{
    EFI_DEBUG_ASSERT_DATA *AssertData;
    CHAR8 *FileName;
    UINT32 LineNum;
    CHAR8 *Desc;

    // gather the data for the different parts of the Assert Info
	//The commented out line below is correct (based on PI spec), however, 
	//the EDKII (multiple instances of the DebugLib/ReportStatusCode libraries) does not properly
	//construct the data structure. The EDKII puts EFI_STATUS_CODE_DATA header twice.
	// The +1 below is indended to skit the dummy header (work around for the EDKII bug).
    //AssertData = (EFI_DEBUG_ASSERT_DATA *)Data;
	AssertData = (EFI_DEBUG_ASSERT_DATA *)(Data+1);

    FileName = (CHAR8 *) (AssertData + 1);

    // get line number
    LineNum = AssertData->LineNumber;

// TODO: Change this to use the FileNameSize in the Data Structure.
//  Intel does not fill this in their worker function, so it is a junk value

    // get description
//  Desc = Filename + AssertData->FileNameSize;
    Desc = FileName + Strlen(FileName) + 1;

    Sprintf_s(String, StringMaxSize, "ASSERT in %s on %i: %s\n", FileName, LineNum, Desc);
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReportData
//
// Description: This function creates a string for error types that have not
//      been predefined by the spec.  This string contains the information in
//      the Type and Value parameters.
//
// Input:
//      IN EFI_STATUS_CODE_TYPE Type - Contains the type and severity of the error
//      IN EFI_STATUS_CODE_VALUE Value - Contains the Class, SubClass, and Operation that caused
//          the error
//      IN EFI_STATUS_CODE_DATA *Data - Data field that contains strings and values to be reported
//      OUT UINT8 *String - Pointer to the beginning of a byte String array
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReportData(
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
    IN EFI_STATUS_CODE_DATA *Data, OUT UINT8 *String
)
{
    if (STATUS_CODE_TYPE(Type)!=EFI_ERROR_CODE) return EFI_UNSUPPORTED;

    Sprintf_s(
        String, StringMaxSize,
        "ERROR: Type:%X; Severity:%X; Class:%X; Subclass:%X; Operation: %X\n",
        Type&0xFF, Type>>24, Value >> 24, (Value >> 16)& 0xFF, Value & 0xFFFF
    );
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReportDebugInfo
//
// Description: This function creates a string from the debug informatoin packaged into
//   Intel-specific(not defined in the standard spec) EFI_DEBUG_INFO structure.
//
// Input: IN EFI_STATUS_CODE_DATA *Data - Pointer to the beginning of the ASSERT data
//
// Output: OUT UINT8 *String - Contains information in the ASSERT data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReportDebugInfo(IN EFI_STATUS_CODE_DATA *Data, OUT UINT8 *String)
{
    BASE_LIST Marker;
    CHAR8 *Format;
    EFI_DEBUG_INFO *DebugInfo = (EFI_DEBUG_INFO *)(Data + 1);

    // The first 12 * UINTN bytes of the string are really an
    // argument stack to support varargs on the Format string.
    Marker = (BASE_LIST) (DebugInfo + 1);
    Format = (CHAR8 *)(((UINT64 *)Marker) + 12);

    AsciiBSPrint((CHAR8*)String, StringMaxSize, Format, Marker);
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CreateString
//
// Description: This function converts the Type, Value, and Data into a string
//      that can be easily output to any console: serial, screen, data storage, etc.
//
// Input:
//      IN  EFI_STATUS_CODE_TYPE Type - Contains the type and severity of the error
//      IN  EFI_STATUS_CODE_VALUE Value - Contains the Class, SubClass, and Operation that caused
//          the error
//      IN  EFI_STATUS_CODE_DATA *Data - Data field that contains strings and values to be reported
//      OUT UINT8 *String - Pointer to the beginning of a byte String array
//
// Output: EFI_STATUS
//      *String - points to the string that was created
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateString(
    IN  EFI_STATUS_CODE_TYPE    Type,
    IN  EFI_STATUS_CODE_VALUE   Value,
    IN  EFI_STATUS_CODE_DATA    *Data,
    OUT UINT8                   *String
)
{
    EFI_STATUS_CODE_STRING_DATA     *StrData;
    CHAR8                           *StrAscii;
    CHAR16                          *StrUnicode;
    UINT16                          Count = 0;

    if (Data != NULL)
    {
        // Check to see if the optional Data parameter is a standard string or
        //  if it is a more complex data structure
        if (!guidcmp(&(Data->Type), &gDataTypeStringGuid))
        {
            // set pointers to different parts of the data structure for
            //  easier access to the data it contains
            StrData = ( EFI_STATUS_CODE_STRING_DATA *)Data;

            // Set up pointer to the beginning of both  parts of the union
            StrAscii = StrData->String.Ascii;
            StrUnicode = StrData->String.Unicode;

            // check for end of string or max length of output string
            while ( ( ((StrData->StringType == EfiStringAscii)
                       && (*StrAscii != '\0'))
                      || ((StrData->StringType == EfiStringUnicode)
                          && (*StrUnicode != '\0')) )
                    && (Count < (StringMaxSize -1))
                  )
            {
                if (StrData->StringType == EfiStringAscii)
                {
                    String[Count++] = *(StrAscii++);
                }

                else if (StrData->StringType == EfiStringUnicode)
                {
                    String[Count] = (UINT8)(*(StrUnicode++));
                    // advance to the next character in the string
                    Count+=2;
                }

                else
                {
                    return EFI_UNSUPPORTED;
                }
            }

            String[Count] = '\0';
            return EFI_SUCCESS;
        }

        else if ( !guidcmp (&Data->Type, &gEfiStatusCodeDataTypeDebugGuid) )
        {
            ReportDebugInfo(Data, String);
        }

        else if (!guidcmp(&(Data->Type), &gSpecificDataGuid))
        {
            // This checks for an assert statement.  Make a string of the data
            //  field
            if ( ((Type & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE)
                    && ((Type & EFI_STATUS_CODE_SEVERITY_MASK) == EFI_ERROR_UNRECOVERED)
                    && ((Value & EFI_STATUS_CODE_OPERATION_MASK)== EFI_SW_EC_ILLEGAL_SOFTWARE_STATE)
                    && (Data != NULL)
               )
            {
                ReportAssertString(Data, String);
            }

            else
            {
                ReportData(Type, Value, Data, String);
            }
        }

        else if ( !guidcmp (&Data->Type, &gEfiStatusCodeDataTypeAssertGuid) )
        {
            //this is Intel specific GUID
            //we have to use Data+1 because of Intel specific bug
            ReportAssertString(Data+1, String);
        }

        else
        {
            ReportData(Type, Value, Data, String);
        }
    }

    else   // if (Data != NULL)
    {
        // If Data is NULL just report required data
        ReportData(Type, Value, NULL, String);
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Halt
//
// Description: Error Code Action.
//              Halts the execution.
//
// Input:
//      VOID *PeiServices - not used
//      EFI_STATUS_CODE_VALUE Value - Value of the error code that triggered the action.
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID Halt(
    IN VOID *PeiServices, IN EFI_STATUS_CODE_VALUE Value
)
{
    EFI_DEADLOOP();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ResetOrHalt
//
// Description: Error Code Action.
//              Attempts to perform a system reset. If reset fails, halts the execution (calls Halt function).
//
// Input:
//      VOID *PeiServices - not used
//      EFI_STATUS_CODE_VALUE Value - Value of the error code that triggered the action.
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ResetOrHalt(
    IN VOID *PeiServices, IN EFI_STATUS_CODE_VALUE Value
)
{
    ResetOrResume(PeiServices,Value);
    Halt(PeiServices, Value);
}


extern INIT_FUNCTION* InitPartsList[];

extern STRING_FUNCTION* StringList[];

extern SIMPLE_FUNCTION* SimpleList[];

extern MISC_FUNCTION* MiscList[];

extern CHECKPOINT_FUNCTION* CheckpointList[];

typedef STATUS_CODE_TO_BYTE_MAP STATUS_CODE_TO_BYTE_MAP_ARRAY[];

extern STATUS_CODE_TO_BYTE_MAP_ARRAY
CHECKPOINT_PROGRESS_CODES_MAP, CHECKPOINT_ERROR_CODES_MAP,
BEEP_PROGRESS_CODES_MAP, BEEP_ERROR_CODES_MAP
;

STATUS_CODE_TO_BYTE_MAP* CheckPointStatusCodes[] =
{
    //#define EFI_PROGRESS_CODE 0x00000001
    CHECKPOINT_PROGRESS_CODES_MAP,
    //#define EFI_ERROR_CODE 0x00000002
    CHECKPOINT_ERROR_CODES_MAP
    //#define EFI_DEBUG_CODE 0x00000003
};

STATUS_CODE_TO_BYTE_MAP* BeepStatusCodes[] =
{
    //#define EFI_PROGRESS_CODE 0x00000001
    BEEP_PROGRESS_CODES_MAP,
    //#define EFI_ERROR_CODE 0x00000002
    BEEP_ERROR_CODES_MAP
    //#define EFI_DEBUG_CODE 0x00000003
};

#define ERROR_ACTION(Value,Action) Action
//#ifdef PEI_STATUS_CODE
extern ERROR_CODE_ACTION PEI_ERROR_CODE_ACTIONS EndOfActionList;
//#else
//extern ERROR_CODE_ACTION DXE_ERROR_CODE_ACTIONS EndOfActionList;
//#endif
#undef ERROR_ACTION
#define ERROR_ACTION(Value,Action) {Value,&Action}

ERROR_CODE_ACTION_MAP ErrorCodeActions[] =
{
//#ifdef PEI_STATUS_CODE
    PEI_ERROR_CODE_ACTIONS
//#endif
//#ifdef DXE_STATUS_CODE
 //   DXE_ERROR_CODE_ACTIONS
//#endif
    {0,0}
};
//--------------------Serial Status Begin----------------------------

//----------------------------------------------------------------------------
// Module specific type definitions


//----------------------------------------------------------------------------
// Function Prototypes
EFI_STATUS SerialData(
    IN  EFI_STATUS_CODE_TYPE    Type,
    IN  EFI_STATUS_CODE_VALUE   Value,
    IN  EFI_STATUS_CODE_DATA    *Data,
    OUT UINT8                   *String
);
VOID Delay(
		VOID **PeiServices,
		UINT32 Microseconds
		);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SerialOutput
//
// Description: Writes the given string to the serial port byte by byte
//              using the function SendByte.
//
// Input:
//      IN EFI_PEI_SERVICES **PeiServices - pointer to the PEI Boot Services table
//      CHAR8 *String - String to be sent over the serial port
//
// Output: EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SerialOutput(
    IN VOID **PeiServices,
    IN CHAR8 *String
)
{

    UINT8       *Buffer;
    UINTN       Count = 0;


    if (String == NULL) return EFI_SUCCESS;
    // first make data useable
    Buffer = (UINT8*)String;

    // Lets count the size of the string
    while (*(Buffer + Count))
    {
    	    if (*(Buffer + Count) =='\r' && *(Buffer + Count + 1) =='\n') Count+=2;
    	    else if (*(Buffer + Count) =='\n') 	//we need to add \r before \n so if we found \n send all that before it 
            {
        	    if (Count)  
        		    SerialPortWrite(Buffer, Count);	// replace "\n" with "\r\n" and send 2 symbols
        	    SerialPortWrite((UINT8*)"\r\n", 2);
        	    Buffer =  Buffer + Count + 1; 	// init Buffer with nex position after \n
        	    Count = 0;	// and reset Counter
            }
            else 
	    	    Count++;
    }
    if (Count)  
    	SerialPortWrite(Buffer, Count); 

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SerialStatusInit
//
// Description:
//   The function programs the stop bits, data bits, and baud rate
//   of the com port and outputs the debug message "Status Code Available."
//   NOTE: This function does not assumes Super I/O has already been initialized.
//
// Input:
//      IN EFI_FFS_FILE_HEADER *FfsHeader
//      IN EFI_PEI_SERVICES **PeiServices - pointer to the PEI Boot Services table
// Output:
//      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SerialStatusInit(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN VOID **PeiServices
)
{
   SerialPortInitialize();
   if (StringMaxSize < 550)
	   SerialOutput(PeiServices, "Status Code Available\n");
   else
	   SerialOutput(PeiServices, "DXE Status Code Available\n");
    return EFI_SUCCESS;
}

VOID SerialCheckpoint(VOID **PeiServices, UINT8 Checkpoint)
{
    char s[20];
    Sprintf_s(s, sizeof(s), "Checkpoint %X\n",Checkpoint);
    SerialOutput(PeiServices, s);
}
//--------------------Serial Status End-------------------------------
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitStatusCodeParts
//
// Description: calls initialization routines of status code subcomponents
//              registered under StatusCodeInitialize eLink.
//
// Input:
//  IN VOID *ImageHandler
//  IN VOID *ServicesTable
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InitStatusCodeParts(IN VOID *ImageHandler, IN VOID *ServicesTable)
{
    UINTN i;
    
    for (i=0; InitPartsList[i]; i++) InitPartsList[i](ImageHandler,ServicesTable);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   StringStatusReport
//
// Description: Calls the string satus code handlers registered under ReportStringStatus eLink.
//
// Input:
//  IN EFI_FFS_FILE_HEADER *FfsHeader
//  IN CHAR8 *String
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID StringStatusReport(IN VOID *PeiServices, IN CHAR8 *String)
{
    UINTN i;
    
    for (i=0; StringList[i]; i++) StringList[i](PeiServices,String);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SimpleStatusReport
//
// Description: Calls simple status code handlers registered under ReportSimpleStatus eLink
//
// Input:
//  IN EFI_PEI_SERVICES *PeiServices -  pointer to Pei Services
//  IN  EFI_STATUS_CODE_TYPE Type - Contains the type and severity of the error
//  IN  EFI_STATUS_CODE_VALUE Value - Contains the Class, SubClass, and Operation that caused
//          the error
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SimpleStatusReport(
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value
)
{
    UINTN i;

    for (i=0; SimpleList[i]; i++) SimpleList[i](PeiServices,Type,Value);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MiscStatusReport
//
// Description: Calls Misc. status code handlers
//  (handlers that consume all available status code data)
//  registered under ReportMiscStatus eLink
//
// Input:
//  IN EFI_PEI_SERVICES *PeiServices -  pointer to Pei Services
//  IN  EFI_STATUS_CODE_TYPE Type - Contains the type and severity of the error
//  IN  EFI_STATUS_CODE_VALUE Value - Contains the Class, SubClass, and Operation that caused
//          the error
//  IN  UINT32 Instance - Instance
//  IN EFI_GUID *CallerId OPTIONAL - The GUID of the caller function
//  IN EFI_STATUS_CODE_DATA *Data OPTIONAL - the extended data field that contains additional info
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID MiscStatusReport(
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
    IN UINT32 Instance, IN CONST EFI_GUID *CallerId OPTIONAL,
    IN EFI_STATUS_CODE_DATA *Data OPTIONAL
)
{
    UINTN i;
    
    for (i=0; MiscList[i]; i++) MiscList[i](PeiServices,Type,Value,Instance,CallerId,Data);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckpointStatusReport
//
// Description: Calls checkpoint status code handlers registered under ReportCheckpoint eLink.
//
// Input:
//  IN EFI_PEI_SERVICES *PeiServices -  pointer to Pei Services
//  IN UINT8              Checkpoint - checkpoint
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CheckpointStatusReport(IN VOID *PeiServices, IN UINT8 Checkpoint)
{
    UINTN i;
    
    for (i=0; CheckpointList[i]; i++) CheckpointList[i](PeiServices,Checkpoint);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PerformErrorCodeAction
//
// Description:
//  Scans list of error actions defined by the PeiErrorCodeActions/DxeErrorCodeActions lists.
//  If action corresponding to the passed in Value is found, performs the action.
//
// Input:
//  IN EFI_PEI_SERVICES *PeiServices -  pointer to Pei Services
//  IN  EFI_STATUS_CODE_TYPE Type - Contains the type and severity of the error
//  IN  EFI_STATUS_CODE_VALUE Value - Contains the Class, SubClass, and Operation that caused
//          the error
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PerformErrorCodeAction(
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value
)
{
    ERROR_CODE_ACTION_MAP *Map = ErrorCodeActions;
    
    while (Map->Value!=0)
    {
        if (Map->Value==Value)
        {
            if (Map->Action!=NULL)
            {
                Map->Action(PeiServices,Value);
                return;
            }
        }
        
        Map++;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReportStatusCode
//
// Description:
//  Top level status code reporting routine exposed by the status code protocol/PPI.
//  Calls the various types of status code handlers
//  (SimpleStatusReport, StringStatusReport, MiscStatusReport, PerformErrorCodeAction)
//  Generates string from the status code data to pass to StringStatusReport function.
//
// Input:
//      IN VOID *PeiServices - pointer to the PEI Boot Services table
//      IN EFI_STATUS_CODE_TYPE Type - the type and severity of the error that occurred
//      IN EFI_STATUS_CODE_VALUE Value - the Class, subclass and Operation that caused the error
//      IN UINT32 Instance -
//      IN EFI_GUI *CallerId OPTIONAL - The GUID of the caller function
//      IN EFI_STATUS_CODE_DATA *Data OPTIONAL - the extended data field that contains additional info
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiReportStatusCode (
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
    IN UINT32 Instance, IN  EFI_GUID *CallerId OPTIONAL,
    IN EFI_STATUS_CODE_DATA *Data OPTIONAL, UINT8 *String
)
{

    SimpleStatusReport(PeiServices,Type,Value);
//#if STRING_STATUS_SUPPORT
    String[0] = '\0';
    CreateString(Type, Value, Data, String);
    
    // step through the useable data and display information as needed
    // Serial data
    
    if (String[0] != '\0') StringStatusReport(PeiServices,String);

//#endif
    MiscStatusReport(PeiServices,Type,Value,Instance,CallerId,Data);
    
    if (STATUS_CODE_TYPE(Type)==EFI_ERROR_CODE)
    PerformErrorCodeAction(PeiServices,Type,Value);
    return EFI_SUCCESS;
}

//**********************************************************************
// Status Code Transports
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Port80Checkpoint
//
// Description: Outputs checkpoint value to port 0x80
//  To support different checkpoint displaying hardware do not port this function.
//  Use ReportCheckpoint eLink based interface instead.
//
// Input:
//  VOID *PeiServices - pointer to the PEI Boot Services table,
//  IN  UINTN Checkpoint - checkpoint to display
//
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID Port80Checkpoint(IN VOID *PeiServices, IN UINT8 Checkpoint)
{
    checkpoint(Checkpoint);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckPointStatus
//
// Description:
//  Looks up the checkpoint for the given status code Type
//  and Value and calls CheckpointStatusReport function to invoke all checkpoint handlers.
//
// Input:
//      IN VOID *PeiServices - pointer to the PEI Boot Services table
//      IN EFI_STATUS_CODE_TYPE Type - the type and severity of the error that occurred
//      IN EFI_STATUS_CODE_VALUE Value - the Class, subclass and Operation that caused the error
//
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CheckPointStatus(
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value
)
{
    UINT8 n;
    UINT32 CodeTypeIndex = STATUS_CODE_TYPE(Type) - 1;
    
    if (CodeTypeIndex >= sizeof(CheckPointStatusCodes)/sizeof(STATUS_CODE_TO_BYTE_MAP*) )
        return EFI_SUCCESS;
        
    n=FindByteCode(CheckPointStatusCodes[CodeTypeIndex],Value);
    
    if (n>0) CheckpointStatusReport(PeiServices,n);
    
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BeepOn
//
// Description: Initiates generation of the sound corresponding
//  to the passed in musical note/octave pair using legacy PC/AT speaker.
//
// Input:
//  UINT8 note - a note to play
//  UINT8 octave - an octave for the note
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID BeepOn(UINT8 note, UINT8 octave)
{
    // beep tones
#define note(x) ((119318200+(x)/2)/(x))
    UINT16 tones[8]={note(26163),note(29366),note(32963),note(34923),note(39200),note(44000),note(49388),note(26163*2)};
    UINT16 Frequency=tones[(note%8)];
    
    if (octave-1>=0) Frequency>>=octave-1;
    else             Frequency<<=1-octave;
    
    //set up channel 1 (used for delays)
    IoWrite8(0x43,0x54);
    IoWrite8(0x41,0x12);
    //set up channel 2 (used by speaker)
    IoWrite8(0x43,0xb6);
    IoWrite8(0x42,(UINT8)Frequency);
    IoWrite8(0x42,(UINT8)(Frequency>>8));
    //turn the speaker on
    IoWrite8(0x61,IoRead8(0x61)|3);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BeepOff
//
// Description: Stops generation of the legacy PC/AT speaker sound started by the BeepOn routine.
//
// Input:
//      VOID
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID BeepOff()
{
    IoWrite8(0x61,IoRead8(0x61)&0xfc);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Beep
//
// Description: Generates sound of the specified duration corresponding
//  to the passed in musical note/octave pair using legacy PC/AT speaker.
//  Control flow:
//    BeepOn
//    Delay
//    BeepOff
//
// Input:
//  VOID *PeiServices - pointer to the PEI Boot Services table
//  UINT8 note - a note to play
//  UINT8 octave - an octave for the note
//  UINT32 duration - sound duration in microseconds.
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID Beep(VOID *PeiServices, UINT8 note, UINT8 octave, UINT32 duration)
{
    BeepOn(note,octave);
    Delay(PeiServices, duration);
    BeepOff();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BeepStatus
//
// Description: Simple Status Code handler that generates beep codes for a certain types of
//  progress and error codes defined by the BEEP_PROGRESS_CODES_MAP and BEEP_ERROR_CODES_MAP mapping tables.
//
// Input:
//  EFI_PEI_SERVICES *PeiServices, - pointer to the PEI Boot Services table
//  IN EFI_STATUS_CODE_TYPE Type - the type and severity of the error that occurred
//  IN EFI_STATUS_CODE_VALUE Value - the Class, subclass and Operation that caused the error
//
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS BeepStatus(
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value
)
{
    UINT32 CodeTypeIndex = STATUS_CODE_TYPE(Type) - 1;
    UINT8 i,n;
    
    if (CodeTypeIndex >= sizeof(BeepStatusCodes)/sizeof(STATUS_CODE_TO_BYTE_MAP*) ) return EFI_SUCCESS;
    
    n = FindByteCode(BeepStatusCodes[CodeTypeIndex],Value);
    
    if (n>0)
    {
        for (i=0; i<n; i++)
        {
            Beep(PeiServices,1,2,400000);
            Delay(PeiServices,100000);
        }
    }
    
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BeepService
//
// Description:
//  This function plays Value quantity of beeps
//
// Input:
//      IN VOID *PeiServices - pointer to the PEI Boot Services table
//      IN EFI_STATUS_CODE_TYPE Type - the type and severity of the error that occurred
//      IN EFI_STATUS_CODE_VALUE Value - the Class, subclass and Operation that caused the error
//      IN UINT32 Instance -
//      IN EFI_GUI *CallerId OPTIONAL - The GUID of the caller function
//      IN EFI_STATUS_CODE_DATA *Data OPTIONAL - the extended data field that contains additional info
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS BeepService(
    IN VOID *PeiServices,
    IN EFI_STATUS_CODE_TYPE Type, IN EFI_STATUS_CODE_VALUE Value,
    IN UINT32 Instance, IN EFI_GUID *CallerId OPTIONAL,
    IN EFI_STATUS_CODE_DATA *Data OPTIONAL
)
{
    UINT8 n;
    
    if ((Value&0xFF00)!=AMI_STATUS_CODE_BEEP_CLASS) return EFI_SUCCESS;
    
    n = (UINT8)Value;
    
    if (n)
    {
        UINT8 i;
        
        for (i=0; i<n; i++) { Beep(PeiServices,0,4,40000); Delay(PeiServices,100000);}
    }
    
    return EFI_SUCCESS;
}


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
