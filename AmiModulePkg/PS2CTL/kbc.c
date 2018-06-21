//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Core/CORE_DXE/PS2CTL/kbc.c 39    6/05/12 9:30a Lavanyap $
//
// $Revision: 39 $
//
// $Date: 6/05/12 9:30a $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: kbc.c
//
// Description: PS/2 Controller I/O support fuctions
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

//----------------------------------------------------------------------

#include "ps2ctl.h"
#include "kbc.h"
#include "GenericSio.h"


//----------------------------------------------------------------------

extern  BOOLEAN MouseResetRequired;
extern  BOOLEAN DetectPs2KeyboardValue;
extern  BOOLEAN InstallKeyboardMouseAlways;
extern  UINT32  IbFreeTimeoutValue;
extern  UINT32  IbFreeMaxTimeoutValue;
BOOLEAN MouseEnableState = FALSE;
BOOLEAN KBDEnableState = TRUE;
BOOLEAN Ps2KbdDetected = FALSE;
BOOLEAN Ps2KbdMouseDetected=FALSE;
extern  BOOLEAN Ps2MouseDetected;
extern  BOOLEAN KbdIrqSupport;
extern  UINT8   gKeyboardIrqInstall;
extern  EFI_LEGACY_8259_PROTOCOL *mLegacy8259;
BOOLEAN InsidePS2DataDispatcher = FALSE;
//----------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       KBCDummyGetData
//
// Description:     Reads keyboard data port to clear it
//
// Parameters:      VOID *Context - Pointer to context
//
// Output:          UINT8 - Keyboard data port data
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
KBCDummyGetData (
    VOID *Context )
{
    UINT8   bData;
    bData = IoRead8(KBC_DATA_PORT);
    TRACEKBD((-1,"KD %X ", bData));
    return bData;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       MouseDummyGetData
//
// Description:     Reads keyboard data port to clear it
//
// Parameters:      VOID *Context - Pointer to context
//
// Output:          UINT8 - Keyboard data port data
//
// Modified:        MouseResetRequired
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
MouseDummyGetData (
    VOID *Context )
{
	UINT8	bData;
    bData = IoRead8(KBC_DATA_PORT);
    TRACEKBD((-1,"MD %X ", bData));
    MouseResetRequired = TRUE;          // Since a orphan mouse data is
                                        // received, mouse packet will be out
                                        // of sync.
    return bData;

}


//----------------------------------------------------------------------
// The following two fuction pointers are initialized with dummy
// routines; they will be updated with the real routine pointers
// in the corresponding device drivers' start functions.
//----------------------------------------------------------------------

STATEMACHINEPROC DrivePS2KbdMachine = KBCDummyGetData;
STATEMACHINEPROC DrivePS2MouseMachine = MouseDummyGetData;

extern KEYBOARD gKbd;


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       KBCBatTest
//
// Description:     Runs Basic Assurance Test on KBC.
//
// Parameters:      None
//
// Output:          EFI_SUCCESS or EFI_DEVICE_ERROR
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS KBCBatTest()
{
    UINT16 wCounter = 0xFFFF;
  PROGRESS_CODE(DXE_KEYBOARD_SELF_TEST);
    //
    // Empty KBC before BAT
    //
    for (; wCounter; wCounter--) {
        IoRead8(KBC_DATA_PORT);
        IoDelay();
        if (!(IoRead8(KBC_CMDSTS_PORT) & (KBC_OBF | KBC_IBF))) {
            break;
        }
    }
    if (!wCounter) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Perform BAT
    //
    if (Read8042(0xAA) != 0x55) {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       AutodetectKbdMousePorts
//
// Description:     Auto detection of KB/MS using AMI KB-5.  This code will
//                  allow the connector swap of Keyboard and PS2 Mouse i.e.
//                  keyboard can be connected to PS2 Mouse connector and
//                  vice-versa.
//
// Parameters:      None. AMI KB-5 present in the system, keyboard controller
//                  BAT is complete.
//
// Output:          None
//
//
// Notes:            This code should be used only if the motherboard has
//                  AMI KB-5 which is also available in IO chipsets having KBC
//                  e.g. SMC932, etc.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID AutodetectKbdMousePorts()
{
    UINT8 bData, Index;

    EFI_STATUS              Status;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    WriteKeyboardCommand(0x60);         // Lock KBD
    IoRead8(KBC_DATA_PORT);             // Discard any data

    Write8042CommandByte(0x74);         // KBD and Aux device disabled

//  Check for KBC version
    IoRead8(KBC_DATA_PORT);             // Discard any data
    WriteKeyboardCommand(0xa1);         //
    if (!ObFullReadTimeout(&bData, 20, TRUE) && bData == 0x35) {

        WriteKeyboardCommand(0x60);
        WriteKeyboardData(4);

        for (Index = 6; Index; Index--){  // Read max. 6 data
            if (ObFullReadTimeout(&bData, 10, TRUE)) break;
        }

        WriteKeyboardCommand(0xa7);         // Disable Mouse
        WriteKeyboardCommand(0xc8);         //  Select Primary

        WriteKeyboardData(rKeyboardID);     // Read KBD ID

        ObFullReadTimeout(&bData, 1000, TRUE);      // Read ACK

        if (bData == rKeyboardID) goto PortSwap;

        if (bData == KB_ACK_COM) {
            ObFullReadTimeout(&bData, 100, TRUE);
// When Mouse is connected to KBD port, control goes to PortSwap here
            if (!bData) goto PortSwap;
            ObFullReadTimeout(&bData, 100, TRUE);
        }
        bData = IoRead8(KBC_CMDSTS_PORT);
// When KBD is connected to the KBD port, control returns here
        if (!(bData & KBC_TIMEOUT_ERR)) return;

        WriteKeyboardCommand(0xD4);         // Secondary Port
        WriteKeyboardData(rKeyboardID);     // Read KBD ID
        ObFullReadTimeout(&bData, 1000, TRUE);
        if (bData == rKeyboardID) return;
        if (bData == KB_ACK_COM) {
// When Mouse alone is connected to Mouseport, control returns here
            if (!ObFullRead()) return;
            bData = ObFullRead();
        }
        bData = IoRead8(KBC_CMDSTS_PORT);
// When KBD alone is connected to Mouse port, no time out error and control
//  goes to portswap.
        if (bData & KBC_TIMEOUT_ERR) return;

PortSwap:
        WriteKeyboardCommand(0xC9);
        return;
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       AuxDeviceCommand
//
// Description:     This routine issues AuxDevice command, and returns the
//                  from the AUX device
//                  the connector swap of Keyboard and PS2 Mouse i.e. keyboard
//                  can be connected to PS2 Mouse connector and vice-versa.
//
// Parameters:      UINT8 bCmd - AUX device command
//
// Output:          UINT8 Data from AUX device
//
// Notes:            Only AUX commands that expect the response from AUX device
//                  can be executed using this function; otherwise the code will
//                  be stuck waiting for OBF
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 AuxDeviceCommand (
    UINT8   bCmd )
{
    EFI_STATUS              Status;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return (UINT8)Status;
    }
    IoWrite8(KBC_CMDSTS_PORT, 0xD4);

    return IssueCommand(bCmd);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       IssueCommand
//
// Description:     Helper function to read the data after executing AUX
//                  device command.
//
// Parameters:      UINT8 bCmd - AUX device command
//
// Output:          UINT8 Data from AUX device
//
// Notes:           Only AUX commands that expect the response from AUX device
//                  can be executed using this function; otherwise the code will
//                  be stuck waiting for OBF
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 IssueCommand (
    UINT8   bCmd )
{
    EFI_STATUS              Status;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return (UINT8)Status;
    }
    IoWrite8(KBC_DATA_PORT, bCmd);
    IbFree();
    for (;;)
    {
        if (IoRead8(KBC_CMDSTS_PORT) & KBC_OBF) {
            return IoRead8(KBC_DATA_PORT);
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       Read8042
//
// Description:     Sends the given command to KBC, reads and returns the
//                  acknowledgement byte returned from KBC.
//
// Parameters:      UINT8 bCmd - Command to send to KBC
//
// Output:          UINT8 Acknowledgment byte
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 Read8042 (
    UINT8   bCmd )
{

    UINT8 bData = 0xFE;
    WriteKeyboardCommand(bCmd);
    ObFullReadTimeout(&bData, 40, FALSE);
    return bData;

}



//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       ReadDevice
//
// Description:     Sends the given command to KBD, reads and returns the
//                  acknowledgement byte returned from KBD.
//
// Parameters:      UINT8 bCmd - Command to send to KBC
//                  UINT8 *Data - Pointer to data buffer
//                  UINT8 Response - Response expected
//
// Output:          EFI_SUCCESS - Data == Response
//                  EFI_DEVICE_ERROR - Data != Response
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS ReadDevice (
    UINT8   bCmd,
    UINT8   *Data,
    UINT8   Response )
{

    WriteKeyboardData(bCmd);
    if (ObFullReadTimeout(Data, 40, FALSE)) return EFI_DEVICE_ERROR;
    if (*Data == Response) return EFI_SUCCESS;
    return EFI_DEVICE_ERROR;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       WriteKeyboardCommand
//
// Description:     Writes command to KBC.
//
// Parameters:      UINT8 bCmd - Command to send to KBC
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID WriteKeyboardCommand (
    UINT8   bCmd )
{
    EFI_STATUS              Status;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    IoWrite8(KBC_CMDSTS_PORT, bCmd);
    IbFree();
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       WriteKeyboardData
//
// Description:     Writes data to KBC.
//
// Parameters:      UINT8 bCmd - Data to send to KBC
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID WriteKeyboardData (
    UINT8   bCmd )
{
    EFI_STATUS              Status;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    IoWrite8(KBC_DATA_PORT, bCmd);
    IbFree();
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       Write8042CommandByte
//
// Description:     Writes CCB to KBC
//
// Parameters:      UINT8 bCCB - Command byte to send to KBC
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID Write8042CommandByte (
    UINT8   bCCB )
{
    EFI_STATUS              Status;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    WriteKeyboardCommand(0x60);     // CMD to send command byte
    IoWrite8(KBC_DATA_PORT, bCCB);  // Write command byte into KBC
    IbFree();                       // Wait until input buffer is free
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       ObFullRead
//
// Description:     Waits for Output Buffer Full and then reads the data port
//
// Parameters:      None
//
// Output:          UINT8 KBC Data port data
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 ObFullRead()
{
    for (;;) {
        if (IoRead8(KBC_CMDSTS_PORT) & KBC_OBF) {
            return IoRead8(KBC_DATA_PORT);
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       IbFree
//
// Description:     Waits for Iutput Buffer to be empty
//
// Parameters:      None
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IbFree()
{
    for (;;) {
        if (!(IoRead8(KBC_CMDSTS_PORT) & KBC_IBF)) {
            break;
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	    IbFreeTimeout
//
// Description:	    Waits a specified timeout for Iutput Buffer to be empty
//
// Parameters:		UINT32 TimeoutValue
//
// Return value:	EFI_STATUS (EFI_SUCCESS or EFI_TIMEOUT)
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS IbFreeTimeout( UINT32 TimeoutValue)
{
    UINTN   i;

    for (i = 0; i < TimeoutValue; i++) {
        if (!(IoRead8(KBC_CMDSTS_PORT) & KBC_IBF)) {
            return EFI_SUCCESS;
        }
        gSysTable->BootServices->Stall(1000);   // 1 ms
    }
    return EFI_TIMEOUT;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       IoDelay
//
// Description:     Performs IO delay by executing IO read.
//
// Parameters:      None
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID IoDelay()
{
    IoRead8(0x61);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       ObFullReadTimeout
//
// Description:     This routine checks for the data availbility in output
//                  buffer for a short period of time, if data is available
//                  within this time, it reads and returns the data from
//                  output buffer.
//
// Paremeters:      UINT8* data - Pointer to the byte to be updated
//                  UINT32 msec - Milliseconds timeout
//                  BOOLEAN ONLYOBF - Only waits for OBF if true
//
// Output:          BOOLEAN - Returns FALSE if data is successfully updated
//                  (no timeout), data is updated
//                  Returns TRUE if time-out
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
ObFullReadTimeout (
    UINT8*      data,
    UINT32      msec,
    BOOLEAN     ONLYOBF )
{

    UINT8       bData;
    UINT32      loopcount = msec << 1;


    for (; loopcount; loopcount--) {

        bData = IoRead8(KBC_CMDSTS_PORT);

        if (ONLYOBF && (bData & KBC_OBF)) {
            *data = IoRead8(KBC_DATA_PORT);
            return FALSE;
        }

        if ((bData & (KBC_OBF|KBC_AUX_OBF)) == KBC_OBF) {
            *data = IoRead8(KBC_DATA_PORT);
            if (bData & 0x40) {
                TRACEKBD((-1, "Status Reg K : %x, %x\n", bData, *data));
                return TRUE;
            }
            else return FALSE;  // No timeout
        }

        if ((bData & (KBC_OBF|KBC_AUX_OBF)) == (KBC_OBF | KBC_AUX_OBF)){
            TRACEKBD((-1, "AUX OBF inside KBD"));
            return TRUE;
        }

        gSysTable->BootServices->Stall(500);        // 0.5msec

    }
    TRACEKBD((-1, "KBD data not available"));
    return TRUE;    // Timeout
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       OutToKb
//
// Description:     Send the given command to KDB during runtime.
//
// Parameters:      KEYBOARD* kbd - Pointer to keyboard buffer
//                  UINT8 bCmd - Command to send to keyboard
//
// Output:          EFI_SUCCESS or EFI_DEVICE_ERROR
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
OutToKb (
    KEYBOARD*   kbd,
    UINT8       bCmd )
{
    UINT8 bCounter1, bData;
    UINT32  Counter;
    EFI_STATUS              Status;

    //
    // If Keyboard irq is supported, device acknowlegement is prossed by IRQ
    // Handler. The acknowledgement data is stored in Kbd->CommandResponded
    //
    if(KbdIrqSupport && gKeyboardIrqInstall){
        if(KBDEnableState) {
            Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
            if (EFI_ERROR(Status)) {
                return Status;
            }
            IoWrite8(KBC_DATA_PORT, bCmd);
            IbFree();
            for (Counter = 1000; Counter > 0; Counter--) {
                if (kbd->CommandResponded == KB_ACK_COM){
                    kbd->CommandResponded =0;
                    return EFI_SUCCESS;
                }
                if (kbd->CommandResponded == KB_RSND_COM){
                    kbd->CommandResponded =0;
                    break;
                }
                gSysTable->BootServices->Stall(1000);
            }
            return EFI_DEVICE_ERROR;
        } 
    }


    for (bCounter1 = 3; bCounter1 > 0; bCounter1--) {
        IbFree();
        IoWrite8(KBC_DATA_PORT, bCmd);
        IbFree();

        for (Counter = 1000; Counter > 0; Counter--) {
            if (IoRead8(KBC_CMDSTS_PORT) & KBC_OBF) {
                bData = IoRead8(KBC_DATA_PORT);
                if (bData == 0xFA) {
                    return EFI_SUCCESS;
                } else if (bData == 0xFE) {
					break;
				} else {
                    if (IoRead8(KBC_CMDSTS_PORT) & KBC_TIMEOUT_ERR) break;
                    //
                    // Insert the key into the buffer
                    //
                    if (kbd) {
                        HandleKBDData(kbd, bData);
                        if (kbd->KeyIsReady) {
                            ProcessHotKey(kbd->KeyData.PS2ScanCode, kbd->KeyData.KeyState.KeyShiftState);
                            InsertKeyToBuffer(kbd, &kbd->KeyData);
                            kbd->KeyIsReady = FALSE;
                        }
                    }
                }
            }
            gSysTable->BootServices->Stall(1000);           // 1msec
        }
    }

    return EFI_DEVICE_ERROR;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       DisableKeyboard
//
// Description:     Disables KBD interface and reads the data from KBC
//                  data port.
//
// Modified:        KBDEnableState
//
// Referral(s):     KBDEnableState
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID DisableKeyboard()
{
    EFI_STATUS  Status;

    if (!KBDEnableState) return;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    IoWrite8(KBC_CMDSTS_PORT, 0xAD);
    IbFree();
    KBDEnableState = FALSE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       EnableKeyboard
//
// Description:     Enables KBD interface.
//
// Paremeters:      None
//
// Output:          Status
//
// Modified:        KBDEnableState
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EnableKeyboard()
{
    EFI_STATUS Status=EFI_SUCCESS;

    if (KBDEnableState) {
        return EFI_SUCCESS;
    }
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    IoWrite8(KBC_CMDSTS_PORT, 0xAE);
    Status = IbFreeTimeout(IbFreeTimeoutValue);
	KBDEnableState = TRUE;
    if (EFI_ERROR(Status)) {
        KBDEnableState = FALSE;
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       DisableAuxDevice
//
// Description:     Disables Aux interface.
//
// Paremeters:      None
//
// Output:          None
//
// Modified:        MouseEnableState
//
// Referrals:       MouseEnableState
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID DisableAuxDevice()
{
    EFI_STATUS  Status;
    if (!MouseEnableState) return;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    IoWrite8(KBC_CMDSTS_PORT, 0xA7);
    IbFree();
    MouseEnableState = FALSE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       EnableAuxDevice
//
// Description:     Enables Aux interface.
//
// Paremeters:      None
//
// Output:          None
//
// Modified:        MouseEnableState
//
// Referrals:       MouseEnableState
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID EnableAuxDevice()
{
    EFI_STATUS  Status;
    if (MouseEnableState) return;
    Status = IbFreeTimeout(IbFreeMaxTimeoutValue);
    if (EFI_ERROR(Status)) {
        return;
    }
    IoWrite8(KBC_CMDSTS_PORT, 0xA8);
    IbFree();
    MouseEnableState = TRUE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       DetectPS2KeyboardAndMouse
//
// Description:     Detects the presence of Keyboard and Mouse in KBC port.
//
// Parameters:      None. Keyboard interface is disabled.
//
// Output:          Ps2KbdDetected and Ps2MouseDetected variable set accorinding 
//                  the device presence 
//
// Modified:        Ps2KbdDetected, Ps2MouseDetected
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS DetectPS2KeyboardAndMouse()
{
    UINT16      wCount;
    UINT8       bData;
    BOOLEAN bAck = FALSE;

    if(Ps2KbdMouseDetected) {
        //
        // Ps2Keyboard and Mouse Detected already
        //
        return EFI_SUCCESS;
    }
            
    Ps2KbdMouseDetected=TRUE;

    DetectPs2Mouse();


    if ( DetectPs2KeyboardValue ) {

        PROGRESS_CODE(DXE_KEYBOARD_DETECT);
        Write8042CommandByte (0x6d);
        KBCGetData();                                       // Dummy read

        for (wCount = 0; wCount < 3; wCount++) {
            // Disable Scanning
            if (!ReadDevice(KBD_DISABLE_SCANNING, &bData, KB_ACK_COM)) break;
            if (IoRead8(KBC_CMDSTS_PORT) & 0x40) {              // Time out error
                gSysTable->BootServices->Stall(6000);           // 6 msec
                // After power-up some junk data comes from KBD. If not eaten
                // other command will fail.
                KBCGetData();
            }
        }

        DisableKeyboard();
        KBCGetData();   

        //
        // 3 times retry on keyboard reset
        //
        for (wCount = 0; wCount < 3; wCount++) {
            if (!ReadDevice(KBD_RESET, &bData, KB_ACK_COM)) {   // ACK received
                TRACEKBD((-1,"KBD Reset Response %X ", bData));
                bAck = TRUE;
                break;
            } else {        
                KBCGetData();                       // Dummy read
            }
        }

        if (bAck) {                             //If not not Keyboard
            if (ObFullRead() == 0xAA) {     // Reset successful
                Ps2KbdDetected=TRUE;
            } else if (Read8042(0xAB)) {       // On Success returns 0
                //
                // 0x01 if Clock line stuck low, 0x02 if clock line stuck high, 
                // 0x03 if data line stuck low, and 0x04 if data line stuck high       
                //
                Ps2KbdDetected=FALSE;
            }
	    }									

        //
        // Check for lock key
        //
        if (!(IoRead8(KBC_CMDSTS_PORT) & 0x10)) {
            //
            // Keyboard is locked, we can report it here
            //
            Ps2KbdDetected=FALSE;
        }
    } else { 
        Ps2KbdDetected = TRUE;
        KBDEnableState = TRUE;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       UpdateSioVariableForKeyboardMouse
//
// Description:     Update the SIO variable in the ACPI name space depend on the 
//                  Ps2keyboard and Mouse Present state.
//
// Parameters:      None
//
// Output:          None
//                  
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateSioVariableForKeyboardMouse(
    EFI_EVENT   Event,
    VOID        *Context
)
{
    static EFI_GUID     SioDevStatusVarGuid = SIO_DEV_STATUS_VAR_GUID;
    UINTN               SioDevStatusVarSize = sizeof(SIO_DEV_STATUS);
    SIO_DEV_STATUS      SioDevStatus;
    UINT32              SioDevStatusVarAttributes = 0;
    EFI_STATUS Status;

    //
    // Get the SIO variable.
    //
    Status = pRS->GetVariable( SIO_DEV_STATUS_VAR_NAME, 
                                &SioDevStatusVarGuid, 
                                &SioDevStatusVarAttributes,
                                &SioDevStatusVarSize, 
                                &SioDevStatus.DEV_STATUS);

    //
    // If variable not found return without updating it.
    //
    if(EFI_ERROR(Status)) {
        SioDevStatus.DEV_STATUS = 0;
		SioDevStatusVarAttributes = EFI_VARIABLE_BOOTSERVICE_ACCESS;
        SioDevStatus.Res3 = 1;   // To indicate that PS2 state vas updated
    }

    //
    // Set the flag based on the Ps2 keyboard presence state
    //
    if(Ps2KbdDetected) {
        SioDevStatus.Key60_64 = 1;
    } else {
        SioDevStatus.Key60_64 = 0;
    }


    //
    // Set the Mouse flag based on the Mouse Presence state.
    //
    if(Ps2MouseDetected) {
        SioDevStatus.Ps2Mouse = 1;
    } else {
        SioDevStatus.Ps2Mouse = 0;
    }

    //
    // Set the SIO variable.
    //
    Status = pRS->SetVariable(  SIO_DEV_STATUS_VAR_NAME, 
                                &SioDevStatusVarGuid, 
                                SioDevStatusVarAttributes,
                                SioDevStatusVarSize, 
                                &SioDevStatus);
    return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       DetectPS2Keyboard
//
// Description:     Detects the presence of Keyboard in KBC port.
//
// Parameters:      None
//
// Output:          EFI_SUCCESS if mouse is detected
//                  EFI_NOT_FOUND if mouse is not detected
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  DetectPS2Keyboard( )
{
    if ( InstallKeyboardMouseAlways ) {
        return EFI_SUCCESS;
    } else {
        return Ps2KbdDetected ? EFI_SUCCESS : EFI_NOT_FOUND;
    }
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       PS2DataDispatcher
//
// Description:     This fuction checks whether data is available in the PS2
//                  controller output buffer. If so, it gives control to the
//                  corresponding state machine executor.
//
// Parameters:      VOID *Context - Pointer to the context for this function
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID PS2DataDispatcher (
    VOID    *Context )
{
    UINT8 data;
    KEYBOARD *kbd = &gKbd;
    UINT8  bIndicators;
	
    bIndicators = kbd->KeyData.KeyState.KeyToggleState & 7;    // SCRL/NUM/CPSL 
    if(KbdIrqSupport){
        
        //
        // if keyboard irq supported check status of SCRL/NUM/CPSL keys
        // and send the command to Keyboard to update the LED status
        // 
        UINT8 bIndicators = kbd->KeyData.KeyState.KeyToggleState & 7; 
        if (bIndicators != kbd->Indicators && kbd->LEDCommandState == 0){
            LEDsOnOff(kbd);
        }
        return;
    }

    if (InsidePS2DataDispatcher) return;
    InsidePS2DataDispatcher = TRUE;

    for(data = IoRead8(KBC_CMDSTS_PORT); data & KBC_OBF; data = IoRead8(KBC_CMDSTS_PORT)) {
        if (data & KBC_AUX_OBF) {
            DrivePS2MouseMachine(Context);
        }
        else {
            //
            // Removed the DisableKeyboard() as to read multiple data from port60h,
            // If any valid key is received we break out of the loop. 
            //
            DrivePS2KbdMachine(Context);
            if (CheckKeyinBuffer(kbd) ) {
                break;
            }
        }
    }

    //
    // Check LED state before issuing ED command
    //
    if (bIndicators != kbd->Indicators && kbd->LEDCommandState == 0) {
	    //
		// Disable the keyboard before issuing ED command 
		//
        DisableKeyboard();
        CheckIssueLEDCmd(kbd);
    }

    //
    //Process the led command and data
    //
    ProcessLEDCommandData(kbd);
    EnableKeyboard();
    InsidePS2DataDispatcher = FALSE;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       CheckIssueLEDCmd
//
// Description:     This function check if KBD LED command ED needs to be
//                  issued.
//                  If 'yes', sends ED command. No data is read.
//
// Parameters:      KEYBOARD *kbd - Pointer to keyboard buffer
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CheckIssueLEDCmd (
    KEYBOARD    *Kbd )
{

    UINT8 bIndicators = Kbd->KeyData.KeyState.KeyToggleState & 7;    // SCRL/NUM/CPSL

    if (bIndicators != Kbd->Indicators && Kbd->LEDCommandState == 0) {
        //
        // Don't issue LED command when data is pending
        //
        if (IoRead8(KBC_CMDSTS_PORT) & KBC_OBF) return;
        Kbd->LEDCommandState = ED_COMMAND_ISSUED;
        WriteKeyboardData(0xED);
    }

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       ProcessKBDResponse
//
// Description:     If 0xFA is received as data, check for any pending ACK
//                  and take necessary action.
//
// Parameters:      KEYBOARD* kbd - Pointer to keyboard buffer
//                  UINT8 bData - Data received from keyboard
//
// Output:          None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProcessKBDResponse (
    KEYBOARD    *Kbd,
    UINT8       Data )
{

    UINT8 bIndicators = Kbd->KeyData.KeyState.KeyToggleState & 7;    // SCRL/NUM/CPSL

    switch (Data) {
        case 0xFA:
            if (Kbd->LEDCommandState == ED_COMMAND_ISSUED) {
                Kbd->LEDCommandState = ED_DATA_ISSUED;
                Kbd->Indicators = bIndicators;
                WriteKeyboardData(bIndicators);
                break;
            }

            if (Kbd->LEDCommandState == ED_DATA_ISSUED) {
                Kbd->LEDCommandState = 0;
                break;
            }


        case 0xFE:
            if (Kbd->LEDCommandState == ED_COMMAND_ISSUED || Kbd->LEDCommandState == ED_DATA_ISSUED) {
//                  Error occured. Clear out the current indicator bits.
//                  Modifiers will have the correct bits that needs to be set.
//                  Next Call to CheckIssueLEDCmd will detect the mismatch
//                  and start the LED sequence.
                    WriteKeyboardData(0xF4);
                    Kbd->LEDCommandState = 0;
                    bIndicators = Kbd->KeyData.KeyState.KeyToggleState & 7;
                    Kbd->KeyData.KeyState.KeyToggleState &=
                        ~(SCROLL_LOCK_ACTIVE | NUM_LOCK_ACTIVE | CAPS_LOCK_ACTIVE);
                    Kbd->Indicators &= 0xf0;
                    break;
            }

        case 0xFF:
                Kbd->LEDCommandState = 0;
                break;
        default:  break;
    }

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       KBCGetData
//
// Description:     Reads and returns byte of data from KBC data port. Also
//                  used as dummy KBC data process routine.
//
// Parameters:      VOID *Context - Pointer to the context of this function
//
// Output:          UINT8 Data read from KBC Data port.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 KBCGetData ()
{
    UINT8   Data;
    Data = IoRead8(KBC_DATA_PORT);

    return Data;
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
