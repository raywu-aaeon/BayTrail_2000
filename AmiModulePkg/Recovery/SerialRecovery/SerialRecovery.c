//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//<AMI_FHDR_START>
//**********************************************************************
//
// Name: SerialRecovery.c - PEI driver
//
// Description: Implements Serial Recovery Functions
//
//**********************************************************************
//<AMI_FHDR_END>

//----------------------------------------------------------------------
#include <Library/SerialPortLib.h>
#include <AmiPeiLib.h>
#include <Guid/AmiRecoveryDevice.h>
#include <Ppi/PeiRecoverySerialModePpi.h>
#include <Ppi/Stall.h>
#include <Ppi/DeviceRecoveryModule.h>
//----------------------------------------------------------------------

#define XMODEM_SOH              0x01        // Start Header
#define XMODEM_EOT              0x04        // End of Transfer
#define XMODEM_ACK              0x06        // Acknowledge
#define XMODEM_NAK              0x15        // Negative Acknowledge
#define XMODEM_CAN              0x18        // Cancel Transfer

EFI_PEI_SERVICES    **mPeiServices;

BOOLEAN GetSerialData (
    UINT8*    ReadData 
);
VOID SendSerialData (
    UINT8*    d 
);
VOID SendSerialString (
    CHAR8*    Str 
);

VOID PrintCopyRightMessage ( );

EFI_STATUS
EFIAPI GetNumberRecoveryCapsules (
    IN EFI_PEI_SERVICES                      **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *This,
    OUT UINTN                                *NumberRecoveryCapsules );

EFI_STATUS
EFIAPI GetRecoveryCapsuleInfo (
    IN EFI_PEI_SERVICES                      **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *This,
    IN UINTN                                 CapsuleInstance,
    OUT UINTN                                *Size,
    OUT EFI_GUID                             *CapsuleType );

EFI_STATUS
EFIAPI LoadRecoveryCapsule (
    IN OUT EFI_PEI_SERVICES                  **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *This,
    IN UINTN                                 CapsuleInstance,
    OUT VOID                                 *Buffer );

BOOLEAN ReadFileFromSerial (
    CHAR8 *    Buffer,
    UINT32*    Size,
    UINT8      *result );

BOOLEAN ReadSerialBlock (
    UINT8*    Buffer );

BOOLEAN SendNakAck (
    UINT8    * Data,
    UINT8    *XCommand );

EFI_PEI_STALL_PPI    *mStallPpi;

EFI_PEI_DEVICE_RECOVERY_MODULE_PPI SerialRecoveryModule = {
    GetNumberRecoveryCapsules, GetRecoveryCapsuleInfo, LoadRecoveryCapsule
};

// PPI to be installed
EFI_PEI_PPI_DESCRIPTOR  SerialRecoveryPpiList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiDeviceRecoveryModulePpiGuid, &SerialRecoveryModule
    }
};

PEI_RECOVERY_SERIAL_MODE_PPI ComSerialDevice = {
    0x3f8
};

// PPI to be installed
EFI_PEI_PPI_DESCRIPTOR SerialDevicePpiList[] = { 
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gSerialRecoveryDevicePpiGuid, &ComSerialDevice
    }
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   SerialRecoveryPeimEntry
//
// Description: Serial Recovery PEI entry Function
//
//  Input:       IN EFI_FFS_FILE_HEADER *FfsHeader- FFSHeader
//               IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI SerialRecoveryPeimEntry (
    IN EFI_PEI_FILE_HANDLE    FfsHeader,
    IN CONST EFI_PEI_SERVICES       **PeiServices )
{
    EFI_STATUS    Status;

    mPeiServices = PeiServices;

    Status = (*PeiServices)->LocatePpi(
                            PeiServices,
                            &gEfiPeiStallPpiGuid,
                            0,
                            NULL,
                            &mStallPpi
                            );
    ASSERT_PEI_ERROR( PeiServices, Status );

    Status = (*PeiServices)->InstallPpi( PeiServices, SerialRecoveryPpiList );

    //Install the Serial Device for Com port
    ComSerialDevice.SerialDeviceBaseAddress = (UINT16)PcdGet64 (PcdSerialRegisterBase);
    Status = (*PeiServices)->InstallPpi (PeiServices,SerialDevicePpiList);
    ASSERT_PEI_ERROR( PeiServices, Status );

    return EFI_SUCCESS;
}

EFI_STATUS GetRecoveryFileInfo (
    IN EFI_PEI_SERVICES    **PeiServices,
    IN OUT VOID            **pCapsuleName,
    IN OUT UINTN           *pCapsuleSize,
    OUT    BOOLEAN         *ExtendedVerification )
{
    if(pCapsuleName != NULL)
        *pCapsuleName = "AMI.ROM";

    if(pCapsuleSize != NULL)
        *pCapsuleSize = (UINTN) PcdGet32 (PcdRecoveryImageSize);

    if(ExtendedVerification != NULL)
        *ExtendedVerification = FALSE;

    return EFI_SUCCESS; 
    //OemGetRecoveryFileInfo(PeiServices, pCapsuleName, \
    //pCapsuleSize, ExtendedVerification);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetNumberRecoveryCapsules
//
// Description: Get the Number of Recovery Capsules
//
//  Input:      IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//              IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This
//              OUT UINTN  *NumberRecoveryCapsules- Number of Recovery Capsule
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI GetNumberRecoveryCapsules (
    IN EFI_PEI_SERVICES                      **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *This,
    OUT UINTN                                *NumberRecoveryCapsules )
{
    *NumberRecoveryCapsules = 1;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetRecoveryCapsuleInfo
//
// Description: Get the Recovery Capsule Information
//
//  Input:      IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//              IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This
//              IN UINTN                              CapsuleInstance,
//              OUT UINTN                             *Size,
//              OUT EFI_GUID                          *CapsuleType
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI GetRecoveryCapsuleInfo (
    IN EFI_PEI_SERVICES                      **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *This,
    IN UINTN                                 CapsuleInstance,
    OUT UINTN                                *Size,
    OUT EFI_GUID                             *CapsuleType )
{

    if(CapsuleType)
        *CapsuleType = gSerialCapsuleGuid;

    if(Size)
        *Size = (UINTN) PcdGet32 (PcdRecoveryImageSize);

    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   PrintBaseAddress
//
// Description: Print the Serial Port base address
//
// Input:       IN UINT16 BaseAddress
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PrintBaseAddress (
    IN UINT64    BaseAddress )
{
    CHAR8    TempBuffer[5]={0,0,0,0,0};
    UINTN    ValueCharNum=0;
    UINTN    Remainder;
    CHAR8    TempPrintBuffer[5]={0,0,0,0,0};
    UINTN    Index=0;

    do {
        BaseAddress = (UINT64)Div64 ((UINT64)BaseAddress, 0x10, &Remainder);

        if(Remainder < 0xa) {
            TempBuffer[ValueCharNum] = (CHAR8)(Remainder + '0');
        } else {
            TempBuffer[ValueCharNum] = (CHAR8)(Remainder + 'A' - 0xa);
        }
        ValueCharNum++;
    } while (BaseAddress != 0);

    // Reverse temp string into Buffer.
    while (ValueCharNum) {
        TempPrintBuffer[Index] = TempBuffer[ValueCharNum-1];
        ValueCharNum--;
        Index++;
    }
    SendSerialString(&TempPrintBuffer[0]);
    return;
}       
//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   LoadRecoveryCapsule
//
// Description: Download the Capsule file from the Serial 
//              device and returns the buffer
//
//  Input:  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//          IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This
//          IN UINTN                       CapsuleInstance,
//          OUT VOID                      *Buffer- Recovery Capsule Data
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI LoadRecoveryCapsule (
    IN OUT EFI_PEI_SERVICES                  **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *This,
    IN UINTN                                 CapsuleInstance,
    OUT VOID                                 *Buffer )
{
    CHAR8                           *pBuff;
    UINT32                          FileSize;
    UINT8                           Result   = 0xFF;
    CHAR8                           *message;
    UINTN                           Number;
    CHAR8                           Char      = 0;
    CHAR8                           *ReadChar = &Char;
    UINT32                          waitCount;
    BOOLEAN                         Recovery = FALSE;
    PEI_RECOVERY_SERIAL_MODE_PPI    *ComDevice=NULL;
    UINT8                           Index=0;
    EFI_STATUS                      Status;
    UINTN                           RecoveryCapsuleSize;
    BOOLEAN                         ExtendedVerification;

    Status = GetRecoveryFileInfo (
                    PeiServices, 
                    NULL, 
                    &RecoveryCapsuleSize, 
                    &ExtendedVerification
                    );
    if(EFI_ERROR(Status)) {
        return Status;
    }

    mPeiServices = PeiServices;
    Index=0;

    do {
        // Locate the Serial Device PPI
        Status = (**PeiServices).LocatePpi (
                                    PeiServices, 
                                    &gSerialRecoveryDevicePpiGuid, 
                                    Index, 
                                    NULL, 
                                    &ComDevice
                                    );

        // If Found Proceed with Reading recovery file. If not break the 
        // loop and return the status

        if ((EFI_ERROR (Status))) {
            break;   
        }

        FileSize = (UINT32)RecoveryCapsuleSize;

        GetNumberRecoveryCapsules(PeiServices, This, &Number);
        if(Number == 0) {
            return EFI_UNSUPPORTED;
        }

        PcdSet64 (PcdSerialRegisterBase, ComDevice->SerialDeviceBaseAddress);

        Status = SerialPortInitialize();
        if (EFI_ERROR(Status)) {
            return Status;
        }

        *ReadChar = '\n';
        SerialPortWrite(ReadChar, 0x01);
        PrintCopyRightMessage();

        message="Press space to start recovery from Serial Address 0x";

        // Display the Serial Port Base address

        SendSerialString(message);
        PrintBaseAddress((UINT64)ComDevice->SerialDeviceBaseAddress);
        message="\n\r or 'N' to try Next Serial Device or 'Q' to quit\n\r";
        SendSerialString(message);
        do {
            waitCount=0xF;        // EKCheck
            while ( waitCount-- ) {
                if ( GetSerialData ( ReadChar ) == TRUE ) {
                    Recovery = TRUE;
                    break;
                }
            }

            if(*ReadChar ==' ') {
                break;
            }

            if (*ReadChar=='N' || *ReadChar=='n') {
                // Try the Next Serial Device
                message="Trying next serial device if avilable.\n\r";
                SendSerialString(message);
                Recovery=FALSE;
                break;
            }
            if ( *ReadChar == 'Q' || *ReadChar == 'q' ) {
                // Exit from Serial Recovery
                message = "Serial Recovery is aborted by user.\n\r";
                SendSerialString( message );
                return EFI_ABORTED;
            }
        } while(Recovery==TRUE);

        if(Recovery == FALSE) {
            // Try Next Device
            Index++;
            continue;
        }
        message="Send BIOS image using XMODEM protocol\n\r";
        SendSerialString( message );

        // Wait for 3-5 Seconds before User Sends a file
        mStallPpi->Stall( mPeiServices, mStallPpi, 20000000 );

        pBuff = Buffer;

        if ( !ReadFileFromSerial( pBuff, &FileSize, &Result )) {
            PEI_TRACE((-1, PeiServices, "\nRead File Failed Capsule Size= %d, FileRead Result=%d\n", FileSize, Result));

            if ( Result == 1 ) {
                PEI_TRACE((-1, PeiServices, "\nNo Responce from Terminal\n"));
            }

            if ( Result == 3 ) {
                PEI_TRACE((-1, PeiServices, "\nFile Size over Run\n"));
            }

            // Error getting Recovery file from current serial device. 
            // So try another Serial Device
            Index++;
            continue;
        } else {
            // File got successfully. So Return
            if(ExtendedVerification || FileSize == (UINT32)RecoveryCapsuleSize)
                return EFI_SUCCESS;
        }
    } while(TRUE);

    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReadFileFromSerial
//
// Description: Read the Recovery file through Serial Device
//
// Input:       CHAR8 * Buffer- Data buffer
//              UINT32* Size- Size
//              UINT8 * result- Result Status
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN ReadFileFromSerial (
    CHAR8 *    Buffer,
    UINT32*    Size,
    UINT8 *    result )
{
    // Read recovery file through serial port, check checksum, signature etc.,
    // and copy it to the buffer ImageStart.

    UINT32    NumBlocks = *Size / 128;
    UINT32    count     = 10;
    UINT32    BlockID   = 1;
    UINT32    TimeOut   = 10;    // 1 second (in .1 sec units) timeout
    UINT8     BlockNum  = 1;
    UINT8   Command   = XMODEM_NAK;
    UINT8     Data      = 0;
    CHAR8*    InChar  = '\0';
    BOOLEAN    Status;

    NumBlocks++;
    while ( NumBlocks ) {
        if ( !SendNakAck( &Data, &Command )) {     // send NAK
            *result = 1;
            return FALSE;
        }

        if ( Data == XMODEM_EOT ) { // If end of transfer, exit with success
            Command = XMODEM_ACK;
            SendSerialData( &Command );
            BlockID--;
            *Size   = BlockID * 128;
            *result = 0;
            return TRUE;
        }

        if ( Data == XMODEM_SOH ) {
            // Start header received, get rest of the packet
            Status = GetSerialData( &Data );
            // Read block#
            if ( Status && (Data == BlockNum)) {
                Status = GetSerialData( &Data );
                if ( Status ) {
                    // If FF-block number doesn't match, retry
                    if ( ReadSerialBlock( Buffer)) {     
                        // Read 128 byte packet
                        BlockNum++;
                        BlockID++;
                        NumBlocks--;
                        Command = XMODEM_ACK;
                        continue;
                    }
                }
            }
            while ( GetSerialData( &Data )) {
                ;
            }
        }
    }

    // No more block to read, No EOT byte received, 
    // terminate xfer and exit with error
    *result = 3;
    while ( GetSerialData( &Data )) {
        ;    // If abort, read and discard
    }
    Command = XMODEM_CAN;
    while ( count-- ) {
        SendSerialData( &Command );    // Send CANCEL Command
    }
    return FALSE;
}

UINT32 CurrentIndex = 0;  // Global variable to hold current buffer index

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReadSerialBlock
//
// Description: Read the Serial Block
//
// Input:       CHAR8 * Buffer- Data buffer
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN ReadSerialBlock (
    UINT8*    Buffer )
{
    UINT8     CheckSum     = 0;
    UINT8     DataByte     = 0;
    UINT8     * DataBuffer = Buffer;
    UINT32    TempIndex    = CurrentIndex;
    UINT8     bytecount;

    // Read 128 byte packet
    for ( bytecount = 1; bytecount <= 128; bytecount++ ) {
        // If error  reading serial port, retry packet
        if ( !(GetSerialData( DataBuffer + CurrentIndex ))) {
            CurrentIndex = TempIndex;
            return FALSE;
        }
        CheckSum += *(DataBuffer + CurrentIndex++);
    }

    // Get Checksum byte
    if ( !(GetSerialData( &DataByte ))) {
        CurrentIndex = TempIndex;
        return FALSE;
    }


    // If byte checksum doesn't match, retry
    if ( DataByte != CheckSum ) {
        CurrentIndex = TempIndex;
        return FALSE;
    }
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   SendNakAck
//
// Description: Send the Ack
//
// Input:           UINT8  *Data,
//                  UINT8  *XCommand
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN SendNakAck (
    UINT8    * Data,
    UINT8    *XCommand )
{
    UINT8    AbortCount = 12, count = 24; // Number of NAKs or ACKs to send
                                          // before abort
    CHAR8    * SerialMsg;

    while ( count-- ) {
        SendSerialData( XCommand );
        if ( GetSerialData( Data )) {
            return TRUE;
        }
    }
    // If timeout, cancel the transfer and
    // return false, with abort message
    while ( GetSerialData( Data )) {      //If abort, read and discard
        ;
    }
    *XCommand = XMODEM_CAN;
    while ( AbortCount-- ) {
        SendSerialData( XCommand );   // Send CANCEL Command
    }
    SerialMsg = "\n\rAborting Serial Recovery";
    SendSerialString( SerialMsg );
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetSerialData
//
// Description: Get the Data from Serial Port
//
// Input:       UINT8* ReadData - Data Got from Serial Device
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN GetSerialData (
    UINT8*    ReadData )
{
    UINTN    Result;
    UINTN    MaxDelay = 0x1046A;

    while(!SerialPortPoll() && MaxDelay)
        MaxDelay--;

    if ( MaxDelay <= 0 ) {
        return FALSE;
    }
     
    Result = SerialPortRead(ReadData, 0x01);

    if (Result < 0x01 ) {
        return FALSE;
    } else {
        return TRUE;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   SendSerialData
//
// Description: Send the Data to serial Device
//
// Input:       UINT8 d - Data to be send it to Serial Device
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SendSerialData (
    UINT8*    d )
{

    SerialPortWrite(d, 0x01);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   SendSerialString
//
// Description: Send the String to serial Device
//
// Input:       CHAR8* Str - String to be send it to Serial Device
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SendSerialString (
    CHAR8*    Str )
{
    UINTN       Count = 0;
    UINT8       *TempBuffer ;

    TempBuffer = (UINT8*) Str;

     // Lets count the size of the string
    while (*(TempBuffer + Count)) {
        Count++;
    }

    if (Count)  
        SerialPortWrite(TempBuffer, Count); 
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   PrintCopyRightMessage
//
// Description: Display the CopyRight Message in Serial Device
//
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PrintCopyRightMessage( )
{
    static CHAR8
    *Message =  "\n\r*********************************************************\n\r"
               "(C)Copyright 1985-2014, American Megatrends, Inc.\n\r"
               "               All Rights Reserved.\n\r"
               "               AMI  Serial Recovery.\n\r"
               "*********************************************************\n\r";

    SendSerialString( Message );
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
