//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    <SerialIo.c>
//
// Description: Driver for serial ports (both super io and pci).  
//              Installs EFI_SERIAL_IO_PROTOCOL for a serial device.  
//              Adds UART_DEVICE_PATH and UART_FLOW_CONTROL_DEVICE_PATH 
//              nodes to its device path.
//
//<AMI_FHDR_END>
//**********************************************************************

#include <AmiDxeLib.h>
#include <Protocol/SerialIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName.h>
#include <Protocol/AmiSio.h>
#include <AcpiRes.h>
#include <Protocol/PciIo.h>
#include <Pci.h>
#include <Protocol/TerminalAmiSerial.h>

//
// Serial Driver Defaults
//

extern UINT32               UartDefaultBaudRate;
extern EFI_PARITY_TYPE      UartDefaultParity;
extern UINT8                UartDefaultDataBits;
extern EFI_STOP_BITS_TYPE   UartDefaultStopBits;
extern UINT8                SerialIo_PciSerialSupport;
extern UINTN                SioUartInputClock;
extern UINTN                PciUartInputClock; 
static UINT8                ComSelector = 0;
extern UINT8                PciComMmioWidth;
extern UINT8                MaxFailuresAllowed;
extern UINT32               MaximumSerialWriteErrorCount; 
extern BOOLEAN              SerialWriteErrorCheck;
extern BOOLEAN              SerialMouseDetection;

#define UART_DEFAULT_TYPE   UART16550A

//
// 115200 baud with rounding errors
//
#define UART_PORT_MAX_BAUD_RATE         115400
#define UART_MIN_BAUD_RATE              50

#define UART_DEFAULT_RECEIVE_FIFO_DEPTH 16
#define UART_MAX_RECEIVE_FIFO_DEPTH     16
#define UART_MIN_TIMEOUT                1           // 1 uS
#define UART_MAX_TIMEOUT                100000000   // 100 seconds

#define UART_DEFAULT_TIMEOUT            1000000
#define TIMEOUT_INTERVAL                10

//
// UART Register and bit definitions
//
#define SERIAL_REGISTER_THR 0       // WO   Transmit Holding Register
#define SERIAL_REGISTER_RBR 0       // RO   Receive Buffer Register
#define SERIAL_REGISTER_DLL 0       // R/W  Divisor Latch LSB
#define SERIAL_REGISTER_DLM 1       // R/W  Divisor Latch MSB
#define SERIAL_REGISTER_IER 1       // R/W  Interrupt Enable Register
    #define RAVIE 0x1               //      Bit0: Receiver Data Available 
                                    //            Interrupt Enable
    #define THEIE 0x2               //      Bit1: Transmitter Holding
                                    //      Register Empty Interrupt Enable
    #define RIE 0x4                 //      Bit2: Receiver Interrupt Enable
    #define MIE 0x8                 //      Bit3: Modem Interrupt Enable
#define SERIAL_REGISTER_IIR 2       // RO   Interrupt Identification Register
    #define IPS 0x1                 //      Bit0: Interrupt Pending Status
    #define IIB 0x0e                //      Bit1-3: Interrupt ID Bits
    #define FIFO_ENABLE_STATUS_64B 0x20 //  Bit5: 64 Byte FIFO Enable Status
    #define FIFOES 0xc0             //      Bit6-7: FIFO Mode Enable Status
    #define FIFO_ENABLED 0xc0       //      Bit6 and 7: FIFO enabled, usuable
    #define FIFO_ENABLED_UNUSABLE 0x80 //   Bit7: FIFO enabled but unusable
    #define FIFO_DISABLED 0x0       //      FIFO not enabled
#define SERIAL_REGISTER_FCR 2       // WO   FIFO Cotrol Register
    #define TRFIFOE 0x1             //      Bit0: Transmit and Receive 
                                    //            FIFO Enable
    #define RESETRF 0x2             //      Bit1: Reset Reciever FIFO
    #define RESETTF 0x4             //      Bit2: Reset Transmistter FIFO
    #define DMS 0x8                 //      Bit3: DMA Mode Select
    #define FIFO_ENABLE_64B 0x20    //      Bit5: 64 Byte FIFO Enable   
    #define RTB 0xc0                //      Bit6-7: Receive Trigger Bits
#define SERIAL_REGISTER_LCR 3       // R/W  Line Control Register
    #define SERIALDB 0x3            //      Bit0-1: Number of Serial 
                                    //                 Data Bits
    #define STOPB 0x4               //      Bit2: Number of Stop Bits
    #define PAREN 0x8               //      Bit3: Parity Enable
    #define EVENPAR 0x10            //      Bit4: Even Parity Select
    #define STICPAR 0x20            //      Bit5: Sticky Parity
    #define BRCON 0x40              //      Bit6: Break Control
    #define DLAB 0x80               //      Bit7: Divisor Latch Access Bit
#define SERIAL_REGISTER_MCR 4       // R/W  Modem Control Register
    #define DTRC 0x1                //      Bit0: Data Terminal Ready Control
    #define RTS  0x2                //      Bit1: Request To Send Control
    #define OUT1 0x4                //      Bit2: Output1
    #define OUT2 0x8                //      Bit3: Output2, used to disable 
                                    //            interrupts
    #define LME 0x10                //      Bit4: Loopback Mode Enable
#define SERIAL_REGISTER_LSR 5       // R/W  Line Status Register
    #define DR 0x1                  //      Bit0: Receiver Data Ready Status
    #define OE 0x2                  //      Bit1: Overrun Error Status
    #define PE 0x4                  //      Bit2: Parity Error Status
    #define FE 0x8                  //      Bit3: Framing Error Status
    #define BI 0x10                 //      Bit4: Break Interrupt Status
    #define THRE 0x20               //      Bit5: Transmistter Holding 
                                    //            Register Status
    #define TEMT 0x40               //      Bit6: Transmitter Empty Status
    #define FIFOE 0x80              //      Bit7: FIFO Error Status
#define SERIAL_REGISTER_MSR 6       // R/W  Modem Status Register
    #define DeltaCTS 0x1            //      Bit0: Delta Clear To Send Status
    #define DeltaDSR 0x2            //      Bit1: Delta Data Set Ready Status
    #define TrailingEdgeRI 0x4      //      Bit2: Trailing Edge of Ring 
                                    //            Indicator Status
    #define DeltaDCD 0x8            //      Bit3: Delta Data Carrier 
                                    //            Detect Status
    #define CTS 0x10                //      Bit4: Clear To Send Status
    #define DSR 0x20                //      Bit5: Data Set Ready Status
    #define RI 0x40                 //      Bit6: Ring Indicator Status
    #define DCD 0x80                //      Bit7: Data Carrier Detect Status
#define SERIAL_REGISTER_SCR 7       // R/W  Scratch Pad Register

#define XON 17
#define XOFF 19
//
// Macro definitions for Serial I/O driver signature
//
#define EFI_SIGNATURE_16(A, B)        ((A) | (B << 8))
#define EFI_SIGNATURE_32(A, B, C, D)  (EFI_SIGNATURE_16 (A, B) | (EFI_SIGNATURE_16 (C, D) << 16))
#define EFI_SERIAL_DRIVER_SIGNATURE    EFI_SIGNATURE_32 ('$', 'S', 'I', 'O')
//==========================================================================
// Function Prototypes

//==========================================================================
BOOLEAN LookupHID(
    UINT32 hid);

EFI_STATUS GetSerialIo_DP(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    ACPI_HID_DEVICE_PATH**      SerialIodp,
    UINT32                      Attributes,
    BOOLEAN                     Test);

//
// UART types
//
typedef enum {
    UART8250   = 0,
    UART16450  = 1,
    UART16550  = 2,
    UART16550A = 3
} UART_TYPE;


typedef struct _SERIAL_PARAMETERS{
    EFI_SERIAL_IO_PROTOCOL      SerialIo;
    UINT32                      Signature;
    AMI_SIO_PROTOCOL            *AmiSio;
    EFI_HANDLE                  Handle;
    SERIAL_IO_MODE              SerialMode;
    EFI_DEVICE_PATH_PROTOCOL    *DevPathProtocol;
    EFI_DEVICE_PATH_PROTOCOL    *ParentDevicePath;
    UART_DEVICE_PATH            UartDevicePath;
    UART_FLOW_CONTROL_DEVICE_PATH UartFlowControlDevicePath;
    UINT64                      BaseAddress;
    UART_TYPE                   UartType;
    BOOLEAN                     WaitForXon;
    UINT32                      ErrorCount;
    UINTN                       Fifohead;
    UINTN                       Fifotail;
    UINTN                       DataCount;
    UINT8                       LoopbackBuffer[UART_MAX_RECEIVE_FIFO_DEPTH];
    BOOLEAN                     Started;
    BOOLEAN                     LineStatusRegOverrunErrorBit;
    BOOLEAN                     SerialDevicePresentOnPort;
    CHAR16                      *ControllerName; 
    EFI_PCI_IO_PROTOCOL         *PciIo;
    UINT8                       BarIndex;
    UINT8                       ReadWriteSemaphore; 
    BOOLEAN                     MMIODevice;
    UINT32                      SerialPortErrorCount;
    EFI_EVENT                   SerialErrorReEnableEvent;
    BOOLEAN                     TimerEventActive;
    BOOLEAN                     AmiSerialDetected;
    BOOLEAN                     IsPciAmiSerial;
    BOOLEAN                     FlowCtrlCTSnotSet;
} SERIAL_PARAMETERS;

UART_DEVICE_PATH gExampleUartDevicePath = 
{
    {
        MESSAGING_DEVICE_PATH,
        MSG_UART_DP,
        sizeof (UART_DEVICE_PATH),
    },
    0,  //Reserved
    0,  //BaudRate
    0,  //DataBits
    0,  //Parity
    0   //StopBits
};
 
#define Flow_Control_Map_Hardware   BIT00
#define Flow_Control_Map_Software   BIT01

UART_FLOW_CONTROL_DEVICE_PATH gExampleUartFlowControlDevicePath = 
{
    {
        MESSAGING_DEVICE_PATH,
        MSG_VENDOR_DP,
        sizeof (UART_FLOW_CONTROL_DEVICE_PATH),
    },
    DEVICE_PATH_MESSAGING_UART_FLOW_CONTROL,
    1
};

//
// LOCAL FUNCTION PROTOTYPES
//
UINT8 
SerialReadPort (
    IN SERIAL_PARAMETERS        *Parameters,
    IN UINT32                   Offset
    );

VOID 
SerialWritePort (
    IN SERIAL_PARAMETERS        *Parameters,
    IN UINT32                   Offset,
    IN UINT8                    Data
    );

VOID AddSerialFifo(
    IN SERIAL_PARAMETERS *Parameters, 
    IN UINT8 Data
    );

UINT8 RemoveSerialFifo(
    IN SERIAL_PARAMETERS *Parameters
    );

BOOLEAN CheckSerialFifoEmpty(
    IN SERIAL_PARAMETERS *Parameters
    );

BOOLEAN CheckSerialFifoFull(
    IN SERIAL_PARAMETERS *Parameters
    );

VOID CheckThreBit(
    EFI_EVENT Event, 
    VOID *Context
);

//==========================================================================
// Function Prototypes for Driver Binding Protocol Interface
//==========================================================================
EFI_STATUS EFIAPI SerialIoSupported (
    IN EFI_DRIVER_BINDING_PROTOCOL    *This,
    IN EFI_HANDLE                     ControllerHandle,
    IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    );

EFI_STATUS EFIAPI SerialIoStart (
    IN EFI_DRIVER_BINDING_PROTOCOL    *This,
    IN EFI_HANDLE                     ControllerHandle,
    IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    );

EFI_STATUS EFIAPI SerialIoStop (
    IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
    IN  EFI_HANDLE                     ControllerHandle,
    IN  UINTN                          NumberOfChildren,
    IN  EFI_HANDLE                     *ChildHandleBuffer
    );


EFI_STATUS
EfiSerialReset (
    IN EFI_SERIAL_IO_PROTOCOL      *This
    );

EFI_STATUS
EfiSerialSetAttributes (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    IN UINT64                   BaudRate,
    IN UINT32                   ReceiveFifoDepth,
    IN UINT32                   Timeout,
    IN EFI_PARITY_TYPE          Parity,
    IN UINT8                    DataBits,
    IN EFI_STOP_BITS_TYPE       StopBits
    );

EFI_STATUS
EfiSerialSetControl (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    IN UINT32                   Control
    );

EFI_STATUS
EfiSerialGetControl (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    OUT UINT32                  *Control
    );

EFI_STATUS
EfiSerialRead (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    IN OUT UINTN                *BufferSize,
    OUT VOID                    *Buffer
    );

EFI_STATUS
EfiSerialWrite (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    );

VOID SetUartType(SERIAL_PARAMETERS *Parameters);
VOID EnableFifoBuffers(SERIAL_PARAMETERS *Parameters, UINT32 NewFifoDepth);
BOOLEAN CheckForLoopbackDevice(EFI_SERIAL_IO_PROTOCOL *SerialIo);

//==========================================================================
// Driver binding protocol instance for SerialIo Driver
//==========================================================================
EFI_DRIVER_BINDING_PROTOCOL gSerialIoDriverBinding = {
    SerialIoSupported,
    SerialIoStart,
    SerialIoStop,
    0x10,
    NULL,
    NULL
    };

//==========================================================================
// Supported SerialIo driver table
//==========================================================================
EFI_STATUS SerialIoGetDriverName(
    IN EFI_COMPONENT_NAME_PROTOCOL  *This,
    IN CHAR8                        *Language,
    OUT CHAR16                      **DriverName
);

EFI_STATUS
SerialIoGetControllerName(
    IN EFI_COMPONENT_NAME_PROTOCOL  *This,
    IN EFI_HANDLE                   ControllerHandle,
    IN EFI_HANDLE                   ChildHandle OPTIONAL,
    IN CHAR8                        *Language,
    OUT CHAR16                      **ControllerName
);

CHAR16 *gSerialIoDriverName = L"AMI Serial I/O Driver";
CHAR16 *gSerialIoName[]     = {L"COM1", L"COM2", 
                               L"COM3", L"COM4", 
                               L"COM5", L"COM6", 
                               L"COM7", L"COM8", 
                               L"COM9", L"COM10" };

//==========================================================================
// Driver component name instance for SerialIo Driver
//==========================================================================

extern CHAR8 SupportedLanguages[];
BOOLEAN LanguageCodesEqual(
    IN CONST CHAR8* LangCode1, 
    IN CONST CHAR8* LangCode2
);

EFI_COMPONENT_NAME_PROTOCOL gSerialIoCtlName = {
    SerialIoGetDriverName,
    SerialIoGetControllerName,
    0
};


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SerialZeroMemory
//
// Description: Clears the buffer
//
// Input:       void    *Buffer,
//              UINTN   Size
//
// Output:      None
//
// Modified:
//      
// Referrals: 
//
// Notes:	
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
void
SerialZeroMemory (
    void                            *Buffer,
    UINTN                           Size
 )
{
    UINT8   *Ptr;
    Ptr = Buffer;
    while (Size--) {
        *(Ptr++) = 0;
    }
}


EFI_STATUS
SerialIoGetDriverName(
    IN EFI_COMPONENT_NAME_PROTOCOL  *This,
    IN CHAR8                        *Language,
    OUT CHAR16                      **DriverName
)
{
    if ( !Language || !DriverName ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( !LanguageCodesEqual( Language, This->SupportedLanguages) ) {
        return EFI_UNSUPPORTED;
    }

    *DriverName = gSerialIoDriverName;
    return EFI_SUCCESS;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialIoGetControllerName
//
// Description: Gets the name of the controller.  
//
// Input:
//      IN EFI_COMPONENT_NAME_PROTOCOL  *This
//      IN EFI_HANDLE                   Controller
//      IN EFI_HANDLE                   ChildHandle OPTIONAL
//      IN CHAR8                        *Language
//      OUT CHAR16                      **ControllerName
//
// Output:
//      EFI_STATUS
//
//<AMI_PHDR_END>
//********************************************************************** 
EFI_STATUS
SerialIoGetControllerName(
    IN EFI_COMPONENT_NAME_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_HANDLE                   ChildHandle OPTIONAL,
    IN CHAR8                        *Language,
    OUT CHAR16                      **ControllerName
)
{
    EFI_STATUS              Status=EFI_SUCCESS; 
    EFI_SERIAL_IO_PROTOCOL  *SerialIo=NULL;
    SERIAL_PARAMETERS       *Parameters=NULL;

    if ( !Language || !ControllerName || !Controller) {
        return EFI_INVALID_PARAMETER;
    }

    if ( !LanguageCodesEqual( Language, This->SupportedLanguages) ) {
        return EFI_UNSUPPORTED;
    }

    //
    // Find the last device node in the device path and return "Supported"
    // for mouse and/or keyboard depending on the SDL switches.
    //
    if (ChildHandle) {
        Status = pBS->OpenProtocol (ChildHandle,
                                      &gEfiSerialIoProtocolGuid,
                                      (VOID **)&SerialIo,
                                      &gSerialIoDriverBinding,
                                      ChildHandle,
                                      EFI_OPEN_PROTOCOL_GET_PROTOCOL
                                      );

        if ( EFI_ERROR( Status )) {
            return Status;
        }

        Parameters = (SERIAL_PARAMETERS *) SerialIo;
        *ControllerName = Parameters->ControllerName; 
        return EFI_SUCCESS;
    }

    return EFI_UNSUPPORTED;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialIoEntryPoint
//
// Description: Installs the driver binding protocol for this driver. 
//
// Input:
//  IN EFI_HANDLE        ImageHandle,
//  IN EFI_SYSTEM_TABLE  *SystemTable
//
// Output:
//  EFI_STATUS
//      
// Referrals: InitAmiLib InstallMultipleProtocolInterfaces 
//
// Notes:   
//  Here is the control flow of this function:
//  1. Initialize Ami Lib.
//  2. Install Driver Binding Protocol
//  3. Return EFI_SUCCESS.
//
//<AMI_PHDR_END>
//********************************************************************** 
EFI_STATUS  SerialIoEntryPoint(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS  Status;

    InitAmiLib(ImageHandle,SystemTable);

    //
    // Iitiaize the ImageHandle and DriverBindingHandle
    //
    gSerialIoDriverBinding.DriverBindingHandle = NULL;
    gSerialIoDriverBinding.ImageHandle = ImageHandle;

    gSerialIoCtlName.SupportedLanguages = SupportedLanguages;
    
    Status = pBS->InstallMultipleProtocolInterfaces(
                    &gSerialIoDriverBinding.DriverBindingHandle,
                    &gEfiDriverBindingProtocolGuid, &gSerialIoDriverBinding,
                    &gEfiComponentNameProtocolGuid, &gSerialIoCtlName,
                    NULL);

    return Status;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialIoSupported
//
// Description: This driver supports the given controller if 
//              (1) the EISA UID and HID are values are correct
//              or (2) the controller supports the PciIo Protocol and the 
//              Revision Id field in the configuration space indicates that
//              the device is a serial port.  
//              
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
//  IN EFI_HANDLE                       Controller,
//  IN EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath
//
// Output:
//  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//<AMI_PHDR_END>
//********************************************************************** 
EFI_STATUS EFIAPI SerialIoSupported (
    IN EFI_DRIVER_BINDING_PROTOCOL    *This,
    IN EFI_HANDLE                     Controller,
    IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )
{
    ACPI_HID_DEVICE_PATH    *acpiDP=NULL;
 
    EFI_STATUS              Status;
    EFI_PCI_IO_PROTOCOL     *PciIo=NULL;
    UINT8                   RevisionId[4];
    UINT64                  CommandReg=0; 
    UINT64                  Supports=0;
    ASLR_QWORD_ASD          *Resources=NULL;
    UINT8                   i;
    EFI_DEVICE_PATH_PROTOCOL    *tmpdp=NULL;
    EFI_HANDLE              TempHandle;
    EFI_PCI_IO_PROTOCOL     *RootPciIo=NULL;
    EFI_DEVICE_PATH_PROTOCOL *TruncatedDevPath = NULL;
    EFI_DEVICE_PATH_PROTOCOL *TempDevPath = NULL;
    AMI_SIO_PROTOCOL            *AmiSio=NULL;
    AMI_SERIAL_PROTOCOL      *AmiSerialProtocol=NULL;
    BOOLEAN                  Pci;
    BOOLEAN                  Mmio;


    //
    // Check whether Ami Serial Protocol is installed.
    //
    Status = pBS->OpenProtocol (Controller, 
                                &gAmiSerialProtocolGuid, 
                                (VOID**)&AmiSerialProtocol, 
                                This->DriverBindingHandle, 
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER
                                );

    if (!EFI_ERROR(Status)) {

        Status = pBS->CloseProtocol (Controller,
                            &gAmiSerialProtocolGuid,
                            This->DriverBindingHandle,
                            Controller
                            );

        if (EFI_ERROR(Status)) {
            return EFI_UNSUPPORTED;
        }

        AmiSerialProtocol->CheckPciMmio(AmiSerialProtocol,&Pci,&Mmio);

        if((Pci == TRUE) && (SerialIo_PciSerialSupport != 1)) {
            return EFI_UNSUPPORTED;
        }
 
        return EFI_SUCCESS;
    }

    // Find the last device node in the device path and return "Supported" 
    if(!EFI_ERROR(GetSerialIo_DP(This, Controller, &acpiDP, 
                                    EFI_OPEN_PROTOCOL_BY_DRIVER,TRUE)) &&
        LookupHID(acpiDP->HID)) {
        if (RemainingDevicePath && 
            ((RemainingDevicePath->Type != MESSAGING_DEVICE_PATH) ||
             (RemainingDevicePath->SubType != MSG_UART_DP))) {
            return EFI_UNSUPPORTED;
        }

        Status  = pBS->OpenProtocol(Controller,
                                &gEfiAmiSioProtocolGuid,
                                (VOID **)&AmiSio,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER);

        if ( EFI_ERROR( Status )) {
            goto Error;
        }
    
        Status = pBS->CloseProtocol (Controller,
                                     &gEfiAmiSioProtocolGuid,
                                     This->DriverBindingHandle,
                                     Controller
                                     );
        if ( EFI_ERROR( Status )) {
            goto Error;
        }

        return EFI_SUCCESS;

    } else if(SerialIo_PciSerialSupport == 1){
        Status = pBS->OpenProtocol (Controller, 
                                    &gEfiDevicePathProtocolGuid, 
                                    (VOID**)&tmpdp, 
                                    This->DriverBindingHandle, 
                                    Controller,
                                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                                    );

        if ( EFI_ERROR( Status )) {
            goto Error;
        }

        Status = pBS->OpenProtocol (Controller, 
                                    &gEfiPciIoProtocolGuid,
                                    (VOID**)&PciIo,
                                    This->DriverBindingHandle,
                                    Controller,
                                    EFI_OPEN_PROTOCOL_BY_DRIVER
                                    );
        if ( EFI_ERROR( Status )) {
            goto Error;
        }

        PciIo->Pci.Read (PciIo,
                        EfiPciIoWidthUint32,
                        PCI_REV_ID_OFFSET,
                        1,
                        &RevisionId
                        );

        Status = pBS->CloseProtocol (Controller,
                                     &gEfiPciIoProtocolGuid,
                                     This->DriverBindingHandle,
                                     Controller
                                     );
        if ( EFI_ERROR( Status )) {
            goto Error;
        }

        if ((RevisionId[3] == PCI_CL_COMM) &&
            ((RevisionId[2] == PCI_CL_COMM_CSL_SERIAL) || (RevisionId[2] == PCI_CL_COMM_CSL_OTHER))) {

            Supports = 1;

            for (i = 0; i < PCI_MAX_BAR_NO; i++) {
                Status = PciIo->GetBarAttributes (
                                                PciIo,
                                                i,
                                                &Supports,
                                                (VOID**)&Resources
                                                );

                //
                //Find the Serial device Resource type. Based on that Enable the IO and Memory 
                //
                if (Resources->Type == ASLRV_SPC_TYPE_IO) {
                    CommandReg = EFI_PCI_IO_ATTRIBUTE_IO;
                    pBS->FreePool(Resources);
                    break;
                } else if(Resources->Type == ASLRV_SPC_TYPE_MEM) {
                    CommandReg = EFI_PCI_IO_ATTRIBUTE_MEMORY;
                    pBS->FreePool(Resources);
                    break;
                }
            }

            if(CommandReg != 0) {
                Status = PciIo->Attributes (PciIo,
                                        EfiPciIoAttributeOperationEnable,
                                        CommandReg,
                                        NULL);              

                TruncatedDevPath=tmpdp;
                do {
                    TruncatedDevPath = DPCut(TruncatedDevPath);
                    if(TruncatedDevPath == NULL ) {
                        break;
                    }

                    //
                    // Locate handle using device path
                    //
                    TempHandle= NULL;    
                    TempDevPath=TruncatedDevPath;
                    Status = pBS->LocateDevicePath(
                                                &gEfiPciIoProtocolGuid,
                                                &TempDevPath,
                                                &TempHandle);

                    if(Status == EFI_SUCCESS) {

                        RootPciIo=NULL;
                        Status = pBS->HandleProtocol (
                                                    TempHandle,
                                                    &gEfiPciIoProtocolGuid,
                                                    (VOID**)&RootPciIo); // Get Device path protocol

                        if(Status == EFI_SUCCESS) {

                            Status = RootPciIo->Attributes (RootPciIo,
                                                        EfiPciIoAttributeOperationEnable,
                                                        CommandReg,
                                                        NULL);   

                        }
                
                    }
                
                }while(TruncatedDevPath != NULL);

            }

            if (RemainingDevicePath && 
                ((RemainingDevicePath->Type != MESSAGING_DEVICE_PATH) ||
                 (RemainingDevicePath->SubType != MSG_UART_DP))) {
                return EFI_UNSUPPORTED;
            }
            return EFI_SUCCESS; 
        }
    }

Error:
//EIP212613 >>
	if( Status == EFI_ALREADY_STARTED || Status == EFI_ACCESS_DENIED ) {
	    return Status;
	}
	return EFI_UNSUPPORTED;
//EIP212613 <<
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialIoStart
//
// Description: Starts the driver.
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
//  IN EFI_HANDLE                       Controller,
//  IN EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath
//
// Output:
//  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:
//
//<AMI_PHDR_END>
//********************************************************************** 
EFI_STATUS EFIAPI SerialIoStart (
    IN EFI_DRIVER_BINDING_PROTOCOL    *This,
    IN EFI_HANDLE                     Controller,
    IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath)
{
    EFI_STATUS                  Status=EFI_SUCCESS;
    SERIAL_PARAMETERS           *Parameters=NULL;
    T_ITEM_LIST                 *ResList=NULL;
    AMI_SIO_PROTOCOL            *AmiSio=NULL;
    ACPI_HID_DEVICE_PATH        *acpiDP=NULL;
    UART_DEVICE_PATH            *UartDevicePath=NULL; 
    EFI_DEVICE_PATH_PROTOCOL    *tmpdp=NULL;
    UINTN                       i=0;
    ASLRF_S_HDR                 *hdr=NULL;
    EFI_PCI_IO_PROTOCOL         *PciIo=NULL;
    UINT64                      Supports=0;
    ASLR_QWORD_ASD              *Resources=NULL;
    UINTN                       Segment=0; 
    UINTN                       Bus=0; 
    UINTN                       Device=0; 
    UINTN                       Function=0;
    EFI_DEVICE_PATH_PROTOCOL    *TempDevPathProtocol;
    AMI_SERIAL_PROTOCOL         *AmiSerialProtocol=NULL;
    BOOLEAN                     Pci;
    UINT8                       Port=0;
    static UINT8                AmiSerialUid = 10;
    ACPI_HID_DEVICE_PATH        DummySioDevicePath = {
                                {ACPI_DEVICE_PATH,
                                ACPI_DP,
                                sizeof(ACPI_HID_DEVICE_PATH)},
                                EISA_PNP_ID(0x501),
                                0};

    AMI_SERIAL_VENDOR_DEVICE_PATH AmiSerialVendorDevicePath = {
                                    {{HARDWARE_DEVICE_PATH,
                                    HW_VENDOR_DP,
                                    sizeof(AMI_SERIAL_VENDOR_DEVICE_PATH)},
                                    AMI_SERIAL_VENDOR_DEVICE_PATH_GUID},
                                    0,
                                    0,
                                    0,
                                    0};

//------------------------------------
 
    Parameters = MallocZ(sizeof(SERIAL_PARAMETERS));
    if ( !Parameters ) {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = pBS->OpenProtocol (Controller,
                                &gAmiSerialProtocolGuid,
                               (VOID**) &AmiSerialProtocol,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER
                                );

    if (!EFI_ERROR(Status)) {
        //
        // Get the Base address and also check whether its a PCI device.
        //
        AmiSerialProtocol->GetBaseAddress(AmiSerialProtocol,&(Parameters->BaseAddress));
        AmiSerialProtocol->CheckPciMmio(AmiSerialProtocol,&Pci,&(Parameters->MMIODevice));

        if(Pci) {
            AmiSerialProtocol->GetPciLocation(AmiSerialProtocol,&Bus,&Device,&Function,&Port);
            Parameters->ControllerName = (CHAR16 *) MallocZ(75); 
            Swprintf(Parameters->ControllerName, L"COM (Pci Dev%X, Func%X, Port%X)", 
                                                        Device, Function, Port);
            //
            // Install AMI serial Vendor device path.
            //
            AmiSerialVendorDevicePath.Bus      = (UINT8)Bus;
            AmiSerialVendorDevicePath.Device   = (UINT8)Device;
            AmiSerialVendorDevicePath.Function = (UINT8)Function;
            AmiSerialVendorDevicePath.Port     = Port;
            Parameters->ParentDevicePath = DPAddNode(NULL,NULL);
            Parameters->ParentDevicePath = DPAddNode(Parameters->ParentDevicePath, 
                                            (EFI_DEVICE_PATH_PROTOCOL *)&AmiSerialVendorDevicePath);
            Parameters->IsPciAmiSerial = TRUE;
        } else {
            
            Parameters->ControllerName = gSerialIoName[ComSelector];
            ComSelector++;
            DummySioDevicePath.UID = AmiSerialUid;
            AmiSerialUid++;
            //
            // Install Dummy ACPI device path for non-pci device. 
            //
            Parameters->ParentDevicePath = DPAddNode(NULL,NULL);
            Parameters->ParentDevicePath = DPAddNode(Parameters->ParentDevicePath,
                                             (EFI_DEVICE_PATH_PROTOCOL *)&DummySioDevicePath);

        }

        Parameters->AmiSerialDetected = TRUE;

        goto SerialTest;
    }
        


    Status = pBS->OpenProtocol(Controller, &gEfiDevicePathProtocolGuid,
                                (VOID**)&tmpdp, This->DriverBindingHandle, 
                                Controller, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if ( EFI_ERROR( Status )) {
        goto Error;
    }

    //1. Make a copy of the device path of the UART Parent 
    //- it has to be ACPI device path
    Parameters->ParentDevicePath = DPCopy(tmpdp);

    //2. Open AmiSio Protocol BY_DRIVER .
    //Don't close it.
    Status  = pBS->OpenProtocol(Controller,
                                &gEfiAmiSioProtocolGuid,
                                (VOID **)&AmiSio,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER);
    if(!EFI_ERROR(Status)) {
    
        Status = AmiSio->CurrentRes(AmiSio, 0, &ResList);
        if ( EFI_ERROR( Status )) {
            goto Error;         
        }
    
        if (ResList){
            for(i = 0; i < ResList->ItemCount; i++) {
                //We only need Base Addr 
                hdr = (ASLRF_S_HDR*)ResList->Items[i];
                switch(hdr->Name){
                    case ASLV_RT_FixedIO: 
                        Parameters->BaseAddress = ((ASLR_FixedIO*)hdr)->_BAS;
                        break;
                    case ASLV_RT_IO: 
                        Parameters->BaseAddress = ((ASLR_IO*)hdr)->_MIN;
                        break;
                }
            }
        }

        if(!EFI_ERROR(GetSerialIo_DP(&gSerialIoDriverBinding, Controller,
                                    &acpiDP, EFI_OPEN_PROTOCOL_GET_PROTOCOL,
                                    TRUE)) &&
            LookupHID(acpiDP->HID) )
        {            
            Parameters->ControllerName = gSerialIoName[ComSelector];
            ComSelector++;
        }        
    } else {  
        //
        // Check for PCI serial Device.
        //
        Status = pBS->OpenProtocol (
                      Controller,
                      &gEfiPciIoProtocolGuid,
                      (VOID **)&PciIo,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                      );
        if (EFI_ERROR (Status)) {
            Parameters->PciIo = NULL;
            goto Error;
        }

        Parameters->PciIo = PciIo;

        Status = PciIo->GetLocation(PciIo, &Segment, &Bus, 
                                    &Device, &Function);
        if (EFI_ERROR(Status)) {
            Parameters->PciIo = NULL;
            goto Error;
        }

        Parameters->ControllerName = (CHAR16 *) MallocZ(50); 
        Swprintf(Parameters->ControllerName, L"COM (Pci Dev%X, Func%X)", 
                                                        Device, Function);
       
        Supports = 1;

        Status = EFI_DEVICE_ERROR; // UEFI 2.2 spec -- return proper error coce
        for (i = 0; i < PCI_MAX_BAR_NO; i++) {
    
            Status = Parameters->PciIo->GetBarAttributes (
                                                        PciIo,
                                                        (UINT8) i,
                                                        &Supports,
                                                        (VOID**)&Resources
                                                       );
            if (Resources->Type == ASLRV_SPC_TYPE_IO) {
                Parameters->BarIndex = (UINT8)i;
                Status = EFI_SUCCESS;
                pBS->FreePool(Resources);
                break;
            }
            if (Resources->Type == ASLRV_SPC_TYPE_MEM) {
                Parameters->MMIODevice=TRUE;
                Parameters->BarIndex = (UINT8)i;
                Status = EFI_SUCCESS;
                pBS->FreePool(Resources);
                break;
            } 
            pBS->FreePool(Resources);
        }
        if (EFI_ERROR(Status)) {
            Parameters->PciIo = NULL;
            goto Error;
        }
    }

SerialTest:
    //
    // Check for a valid Baseaddress.
    //
    if ( (!Parameters->PciIo) && (Parameters->BaseAddress == 0) ) {
        goto Error;
    }

    //
    // Check whether a port is there
    //
    SerialWritePort(Parameters, SERIAL_REGISTER_SCR, 0xaa);
    if ( SerialReadPort( Parameters, SERIAL_REGISTER_SCR ) != 0xaa ) {
        goto Error;
    }

    SetUartType(Parameters);

    Parameters->WaitForXon = FALSE; // Allow input flow until halted.
    Parameters->ErrorCount = 0;
    Parameters->Fifohead = 0;
    Parameters->Fifotail = 0;
    Parameters->DataCount = 0;
    SerialZeroMemory(&Parameters->LoopbackBuffer[0],UART_MAX_RECEIVE_FIFO_DEPTH);
    //
    // Initialize Serial I/O
    //
    Parameters->SerialIo.Revision       = EFI_SERIAL_IO_PROTOCOL_REVISION;
    Parameters->SerialIo.Reset          = EfiSerialReset;
    Parameters->SerialIo.SetAttributes  = EfiSerialSetAttributes;
    Parameters->SerialIo.SetControl     = EfiSerialSetControl;
    Parameters->SerialIo.GetControl     = EfiSerialGetControl;
    Parameters->SerialIo.Write          = EfiSerialWrite;
    Parameters->SerialIo.Read           = EfiSerialRead;
    Parameters->SerialIo.Mode           = &(Parameters->SerialMode);
    //
    //  Initialize the SerialIo driver Signature 
    //

    Parameters->Signature = EFI_SERIAL_DRIVER_SIGNATURE;
    Parameters->UartDevicePath = gExampleUartDevicePath;

    Status = Parameters->SerialIo.SetAttributes(&Parameters->SerialIo,
                    0,              //Parameters->SerialMode.BaudRate
                    0,              //Parameters->SerialMode.ReceiveFifoDepth
                    0,              //Parameters->SerialMode.Timeout
                    DefaultParity,  //Parameters->SerialMode.Parity
                    0,              //Parameters->SerialMode.DataBits
                    DefaultStopBits); //Parameters->SerialMode.StopBits

    if ( EFI_ERROR( Status )) {
        goto Error;
    }

    

    if (RemainingDevicePath) {
        UartDevicePath = (UART_DEVICE_PATH *)RemainingDevicePath;
        UartDevicePath->BaudRate = Parameters->SerialMode.BaudRate; 
        UartDevicePath->Parity = Parameters->SerialMode.Parity; 
        UartDevicePath->DataBits = Parameters->SerialMode.DataBits; 
        UartDevicePath->StopBits = Parameters->SerialMode.StopBits; 
    }
    //
    //Create This UART Device Device Path
    //
    TempDevPathProtocol = 
        DPAddNode( (EFI_DEVICE_PATH_PROTOCOL*) Parameters->ParentDevicePath,
                (EFI_DEVICE_PATH_PROTOCOL*) &Parameters->UartDevicePath );
    
    Parameters->UartFlowControlDevicePath = 
                                        gExampleUartFlowControlDevicePath;

    Parameters->DevPathProtocol = 
        DPAddNode( (EFI_DEVICE_PATH_PROTOCOL*) TempDevPathProtocol,
                (EFI_DEVICE_PATH_PROTOCOL*) &Parameters->UartFlowControlDevicePath );
    
    pBS->FreePool(TempDevPathProtocol);

    //
    // Install SerialIO protocol interfaces for the serial device.
    //
    Status = pBS->InstallMultipleProtocolInterfaces (
                    &Parameters->Handle,
                    &gEfiSerialIoProtocolGuid, &Parameters->SerialIo,
                    &gEfiDevicePathProtocolGuid, Parameters->DevPathProtocol,
                    NULL);
    
    if (EFI_ERROR(Status)) {
        goto Error;
    }

    //
    // Open the protocol for the created child handle.
    //
    if (Parameters->AmiSerialDetected) {
        //
        // Open the Ami Serial protocol as child for the Serial IO.
        //
        Status=pBS->OpenProtocol(Controller, 
                                &gAmiSerialProtocolGuid,
                                (VOID**)&AmiSerialProtocol, 
                                This->DriverBindingHandle, 
                                Parameters->Handle,
                                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
    
    } else if (!Parameters->PciIo) {
        Status=pBS->OpenProtocol(Controller, 
                                &gEfiAmiSioProtocolGuid,
                                (VOID**)&Parameters->AmiSio, 
                                This->DriverBindingHandle, 
                                Parameters->Handle, 
                                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER);
    } else { 

        Status = pBS->OpenProtocol (Controller,
                                    &gEfiPciIoProtocolGuid,
                                    (VOID**)&PciIo,
                                    This->DriverBindingHandle,
                                    Parameters->Handle,
                                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                                    );
    }

    if (EFI_ERROR(Status)) {
        goto Error;
    }

    Parameters->Started=TRUE;
    
    Parameters->LineStatusRegOverrunErrorBit = FALSE; 

    //
    // Create a Event to check the Serial Port status.
    //
    Status = pBS->CreateEvent(
        EVT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
        TPL_NOTIFY,
        CheckThreBit,
        (VOID*)Parameters,
        &Parameters->SerialErrorReEnableEvent
    );
    ASSERT_EFI_ERROR(Status);
    
    Parameters->TimerEventActive=FALSE;

    return EFI_SUCCESS;

Error:
    
    pBS->CloseProtocol(Controller,
                       &gEfiAmiSioProtocolGuid,
                       This->DriverBindingHandle,
                       Controller
                       );

    pBS->CloseProtocol(Controller,
                       &gEfiPciIoProtocolGuid,
                       This->DriverBindingHandle,
                       Controller
                       );
    
    pBS->CloseProtocol(Controller,
                       &gAmiSerialProtocolGuid,
                       This->DriverBindingHandle,
                       Controller
                       );

    if ( Parameters ) {
        pBS->FreePool(Parameters);
    }

    return EFI_DEVICE_ERROR;

}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialIoStop
//
// Description: Stops the driver.
//
// Input:
//  IN EFI_DRIVER_BINDING_PROTOCOL      *This,
//  IN EFI_HANDLE                       Controller,
//  IN  UINTN                           NumberOfChildren,
//  IN  EFI_HANDLE                      *ChildHandleBuffer
//
// Output:
//  EFI_STATUS
//
// Modified:
//      
// Referrals: 
//
// Notes:   
//
//<AMI_PHDR_END>
//********************************************************************** 
EFI_STATUS EFIAPI
SerialIoStop (
    IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
    IN  EFI_HANDLE                      Controller,
    IN  UINTN                           NumberOfChildren,
    IN  EFI_HANDLE                      *ChildHandleBuffer
    )
{
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_SERIAL_IO_PROTOCOL  *SerialIo = NULL;
    SERIAL_PARAMETERS       *Parameters = NULL;
    
    if (!NumberOfChildren) {
        Status = pBS->CloseProtocol(
                        Controller,
                        &gEfiAmiSioProtocolGuid,
                        This->DriverBindingHandle,
                        Controller
                        );
        if (EFI_ERROR(Status)) {
            Status = pBS->CloseProtocol(
                        Controller,
                        &gEfiPciIoProtocolGuid,
                        This->DriverBindingHandle,
                        Controller
                        );
            if ( EFI_ERROR( Status )) {
                Status = pBS->CloseProtocol(
                                Controller,
                                &gAmiSerialProtocolGuid,
                                This->DriverBindingHandle,
                                Controller
                                );
                if ( EFI_ERROR( Status )) {
                    return EFI_DEVICE_ERROR;
                }
            }
        }
        return EFI_SUCCESS;
    }
    //
    // Uninstall all protocols installed installed in DriverBindingStart
    //

    //
    // open the Serial Io protocol
    //
    Status = pBS->OpenProtocol (
                    *ChildHandleBuffer,
                    &gEfiSerialIoProtocolGuid,
                    (VOID **)&SerialIo,
                    This->DriverBindingHandle,
                    *ChildHandleBuffer,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );

    if ( EFI_ERROR( Status )) {
        return EFI_DEVICE_ERROR;
    }

    
    // Find the pointer to the parent structure
    //  in this case, SerialIo is the first element in the data structure
    Parameters = (SERIAL_PARAMETERS *) SerialIo;

    if(Parameters->AmiSerialDetected) {
        Status = pBS->CloseProtocol(Controller, 
                                    &gAmiSerialProtocolGuid,
                                    This->DriverBindingHandle, 
                                    Parameters->Handle
                                    );
    } else if (Parameters->PciIo) {
        Status = pBS->CloseProtocol(Controller, 
                                    &gEfiPciIoProtocolGuid,
                                    This->DriverBindingHandle, 
                                    Parameters->Handle
                                    );
    } else {
        Status = pBS->CloseProtocol(Controller, 
                                    &gEfiAmiSioProtocolGuid,
                                    This->DriverBindingHandle, 
                                    Parameters->Handle
                                    );
    }

    if ( EFI_ERROR( Status )) {
        goto Error;
    }

    //
    // Uninstall the protocol interface
    //
    Status = pBS->UninstallMultipleProtocolInterfaces (
                                                Parameters->Handle,
                                                &gEfiSerialIoProtocolGuid,
                                                &Parameters->SerialIo,
                                                &gEfiDevicePathProtocolGuid,
                                                Parameters->DevPathProtocol,
                                                NULL);

Error:

    pBS->FreePool(Parameters);

    if ( EFI_ERROR( Status )) {
        return EFI_DEVICE_ERROR;
    }
    
    return EFI_SUCCESS;
}


//
// PROTOCOL INTERFACE FUNCTIONS
//

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:	EfiSerialReset
//
// Description: Resets the serial device.
//
//  Arguments:
//    This        - A Pointer to the Serial IO Interface.
//
//  Returns:
//    EFI_SUCCESS      - The serial device was reset.
//    EFI_DEVICE_ERROR - The serial device could not be reset.
//
// Output:
//  EFI_STATUS
//
//<AMI_PHDR_END>
//********************************************************************** 
EFI_STATUS
EfiSerialReset (
    IN EFI_SERIAL_IO_PROTOCOL  *This
    )
{
    EFI_STATUS              Status;
    SERIAL_PARAMETERS       *Parameters = (SERIAL_PARAMETERS *)This;
    UINT8                   Temp;
    
    //
    // Divisor Access Latch must be 0 to program UART.
    //
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_LCR);
    Temp &= ~DLAB;
    SerialWritePort(Parameters, SERIAL_REGISTER_LCR, Temp);
    
    //
    // Turn off interrupts via Interrupt Enable Register.
    //
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_IER);
    Temp &= ~(RAVIE | THEIE | RIE | MIE);
    SerialWritePort(Parameters, SERIAL_REGISTER_IER, Temp);
    
    //
    // Reset the FIFO.
    //
    Temp = RESETRF | RESETTF;
    SerialWritePort(Parameters, SERIAL_REGISTER_FCR, Temp);
    
    EnableFifoBuffers(Parameters, UART_MAX_RECEIVE_FIFO_DEPTH); 

    //
    // Use Modem Control Register to disable interrupts and loopback.
    //
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_MCR);
    Temp &= ~(OUT1 | OUT2 | LME);
    SerialWritePort(Parameters, SERIAL_REGISTER_MCR, Temp);
    
    //
    // Clear the scratch pad register
    //
    SerialWritePort(Parameters, SERIAL_REGISTER_SCR, 0);
    
    //
    // Go set the current attributes
    //
    Status = This->SetAttributes(
        This,
        This->Mode->BaudRate,
        This->Mode->ReceiveFifoDepth,
        This->Mode->Timeout,
        This->Mode->Parity,
        (UINT8)This->Mode->DataBits,
        This->Mode->StopBits);
    
    if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }
    
    //
    // Go set the current control bits
    //
    Status = This->SetControl(
        This,
        This->Mode->ControlMask);
    
    if (EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }
    
    Parameters->WaitForXon = FALSE; // Allow input flow until halted.
    Parameters->ErrorCount = 0;
    Parameters->Fifohead = 0;
    Parameters->Fifotail = 0;
    Parameters->DataCount = 0;
    SerialZeroMemory(&Parameters->LoopbackBuffer[0],UART_MAX_RECEIVE_FIFO_DEPTH);

  
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_MCR);
  	Temp |= DTRC;  // Tell Hyperterminal that it can write to us now.
    SerialWritePort( Parameters, SERIAL_REGISTER_MCR, Temp );
    //
    // Empty input
    //
    SerialReadPort( Parameters, SERIAL_REGISTER_RBR );
    //
    // Device reset is complete
    //
    return EFI_SUCCESS;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:	EfiSerialSetAttributes
//
// Description: Protocol interface to set the serial device attributes.
//      Sets the baud rate, receive FIFO depth, transmit/receice time out,
//      parity, data buts, and stop bits on a serial device.
//
// Input:
//  EFI_SERIAL_IO_INTERFACE       *This - Pointer to the Serial IO Interface
//  UINT64                        BaudRate - The requested baudrate
//  UINT32                        ReceiveFifoDepth - Requested depth of 
//                                                   receive FIFO
//  UINT32                        Timeout - Single character timeout in 
//                                          miliseconds
//  EFI_PARITY_TYPE               Parity - The type of parity to use
//  UINT8                         DataBits - The number of data bits to use
//  EFI_STOP_BITS_TYPE            StopBits - The number of stopbits to use
//
// Output:
//  EFI_STATUS
//    EFI_SUCCESS       - The device attributes were set.
//    EFI_DEVICE_ERROR  - The device attributes could not be set.
//    EFI_INVALID_PARAMETER - A parameter was invalid.
//
// Modified:
//      
// Referrals: SerialReadPort, SerialWritePort, AppendDevicePathNode
//
// Notes:   
//
//<AMI_PHDR_END>
//**********************************************************************

EFI_STATUS
EfiSerialSetAttributes (
    IN EFI_SERIAL_IO_PROTOCOL           *This,
    IN UINT64                           BaudRate,
    IN UINT32                           ReceiveFifoDepth,
    IN UINT32                           Timeout,
    IN EFI_PARITY_TYPE                  Parity,
    IN UINT8                            DataBits,
    IN EFI_STOP_BITS_TYPE               StopBits
    )
{
    EFI_STATUS                  Status;
    SERIAL_PARAMETERS           *Parameters = (SERIAL_PARAMETERS *)This;
    UINT32                      Divisor;
    UINTN                       Remainder;
    UINT8                       Temp;
    EFI_DEVICE_PATH_PROTOCOL    *NewDevicePath;
    UINTN                       UartInputClock;
    static UINT32               BaudRateTable[] = { 50, 75, 110, 134, 
                                                    150, 300, 600, 1200,
                                                    1800, 2000, 2400, 3600,
                                                    4800, 7200, 9600, 19200,
                                                    38400, 57600, 115200, 230400 };
    //
    // Check for PCI or SIO for the UART Input Clock. 
    //
    if( (Parameters->PciIo != NULL) || (Parameters->IsPciAmiSerial == TRUE) ) {
        UartInputClock = PciUartInputClock;
    } else {
        UartInputClock = SioUartInputClock;
    }    

    //
    // If anything is zero, fill in the default value.
    //
    if ( BaudRate == 0 ) {
        BaudRate = UartDefaultBaudRate;
    }

    if ( ReceiveFifoDepth == 0 ) {
        ReceiveFifoDepth = UART_DEFAULT_RECEIVE_FIFO_DEPTH;
    }

    if ( Timeout == 0 ) {
        Timeout = UART_DEFAULT_TIMEOUT;
    }

    if ( Parity == DefaultParity ) {
        Parity = UartDefaultParity;
    }

    if ( DataBits == 0 ) {
        DataBits = UartDefaultDataBits;
    }

    if ( StopBits == DefaultStopBits ) {
        StopBits = UartDefaultStopBits;
    }

    //
    // Check for valid parameters.
    //
    // Don't allow less than 7 data bits on 16550A.
    if ( Parameters->UartType == UART16550A ) {
        if ( DataBits < 7 ) {
            return EFI_INVALID_PARAMETER;
        }
    }

    // Check baud rate
    if ( BaudRate < 50 ) {
        return EFI_INVALID_PARAMETER;
    }

    for (Temp = 0; Temp < (sizeof(BaudRateTable)/sizeof(BaudRateTable[0]))-1; Temp++)
    {
        if ( BaudRate < BaudRateTable[Temp + 1] ) {
            BaudRate = BaudRateTable[Temp];
            break;
        }
    }

    if ( BaudRate >= 115200 ) {
        BaudRate = 115200;
    }
     
    if ((ReceiveFifoDepth < 1) || 
        (ReceiveFifoDepth > UART_MAX_RECEIVE_FIFO_DEPTH)) {
        return EFI_INVALID_PARAMETER;
    }
    
    if ((Timeout < UART_MIN_TIMEOUT) || (Timeout > UART_MAX_TIMEOUT)) {
        return EFI_INVALID_PARAMETER;
    }
    
    if ((Parity < NoParity) || (Parity > SpaceParity)) {
        return EFI_INVALID_PARAMETER;
    }
    
    if ((DataBits < 5) || (DataBits > 8)) {
        return EFI_INVALID_PARAMETER;
    }
    
    if ((StopBits < OneStopBit) || (StopBits > TwoStopBits)) {
        return EFI_INVALID_PARAMETER;
    }
    
    //
    // Do not allow OneFiveStopBits for 6, 7, or 8 data bits.
    //
    if ((DataBits >=6) && (DataBits <=8) && (StopBits == OneFiveStopBits)) {
        return EFI_INVALID_PARAMETER;
    }
    
    //
    // If the new atributes are the same as the current ones, exit with 
    // success.
    //
    if (Parameters->UartDevicePath.BaudRate     == BaudRate         &&
        Parameters->UartDevicePath.DataBits     == DataBits         &&
        Parameters->UartDevicePath.Parity       == Parity           &&
        Parameters->UartDevicePath.StopBits     == StopBits         &&
        This->Mode->ReceiveFifoDepth            == ReceiveFifoDepth &&
        This->Mode->Timeout                     == Timeout) {
        return EFI_SUCCESS;
    }

    if (This->Mode->ReceiveFifoDepth != ReceiveFifoDepth) {
        EnableFifoBuffers(Parameters, ReceiveFifoDepth); 
    }

    //
    // Compute the baud rate divisor.
    //
    Divisor = (UINT32) Div64 (UartInputClock,
                                ((UINT32)BaudRate * 16), 
                                &Remainder);
    if ( Remainder ) {
        Divisor += 1;
    }
    
    if ((Divisor == 0) || (Divisor & 0xffff0000)) {
        return EFI_INVALID_PARAMETER;
    }
    
    //
    // The actual rate is the input clock divided by the divisor 
    // and then by 16.
    //
    BaudRate = UartInputClock / Divisor / 16;
    
    //
    // Read the Line Control Register and save it
    //
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_LCR);
    
    //
    // Put serial port in Divisor Latch Mode
    //
    Temp |= DLAB;
    SerialWritePort(Parameters, SERIAL_REGISTER_LCR, Temp);
    
    //
    // Write the baud rate divisor to the serial port
    //
    SerialWritePort(Parameters, 
                    SERIAL_REGISTER_DLL, 
                    (UINT8)(Divisor & 0xff));
    SerialWritePort(Parameters, 
                    SERIAL_REGISTER_DLM, 
                    (UINT8)((Divisor>>8) & 0xff));
    
    //
    // Reset divisor latch bit in order to set remaining attributes.
    //
    Temp &= ~DLAB;    // Kill divisor latch bit.
    
    // Set parity bits.
    switch (Parity) {
        case NoParity:
            Temp &= ~(PAREN | EVENPAR | STICPAR);
        break;
        case EvenParity:
            Temp |= (PAREN | EVENPAR);
            Temp &= ~STICPAR;
        break;
        case OddParity:
            Temp |= PAREN;
            Temp &= ~(EVENPAR | STICPAR);
        break;
        case SpaceParity:
            Temp |= (PAREN | EVENPAR | STICPAR);
        break;
        case MarkParity:
            Temp |= (PAREN | STICPAR);
            Temp &= ~EVENPAR;
        break;
        default:
        break;
    }
    //
    // Set stop bits.
    //
    switch (StopBits) {
        case OneStopBit :
            Temp &= ~STOPB;
        break;
        case OneFiveStopBits :
        case TwoStopBits :
            Temp |= STOPB;
        break;
        default:
        break;
    }
    //
    // Set data bits.
    //
    Temp &= ~SERIALDB;
    Temp |= (UINT8)((DataBits - 5) & 0x03);
    
    //
    // Now write the LCR register back.
    //
    SerialWritePort(Parameters, SERIAL_REGISTER_LCR, Temp);
    
    //
    //Set the Serial Mode parameters.
    //
    This->Mode->BaudRate         = BaudRate;
    This->Mode->ReceiveFifoDepth = ReceiveFifoDepth;
    This->Mode->Timeout          = Timeout;
    This->Mode->Parity           = Parity;
    This->Mode->DataBits         = DataBits;
    This->Mode->StopBits         = StopBits;
    
    //
    // If the parameters in the device path node are the same as the 
    // current ones, return success.
    //
    if (Parameters->UartDevicePath.BaudRate     == BaudRate &&
        Parameters->UartDevicePath.DataBits     == DataBits &&
        Parameters->UartDevicePath.Parity       == Parity   &&
        Parameters->UartDevicePath.StopBits     == StopBits    ) {
        return EFI_SUCCESS;
    }
    
    //
    // If the parameters have changed, update the device path.
    //
    Parameters->UartDevicePath.BaudRate = BaudRate;
    Parameters->UartDevicePath.DataBits = DataBits;
    Parameters->UartDevicePath.Parity   = (UINT8)Parity;
    Parameters->UartDevicePath.StopBits = (UINT8)StopBits;
    
    NewDevicePath = DPAddNode (
                    Parameters->ParentDevicePath,
                    (EFI_DEVICE_PATH_PROTOCOL *)&Parameters->UartDevicePath
                    );
	//
	// Adding the UartFlowControlDevicePath Node with Updated UartDevicePath Node.
	//
	NewDevicePath = DPAddNode(
					NewDevicePath,
					(EFI_DEVICE_PATH_PROTOCOL*) &Parameters->UartFlowControlDevicePath
					);					
    if (NewDevicePath == NULL) {
        return EFI_DEVICE_ERROR;
    }
    
    if (Parameters->Handle != NULL) {
        Status = pBS->ReinstallProtocolInterface (
                    Parameters->Handle,                 
                    &gEfiDevicePathProtocolGuid, 
                    Parameters->DevPathProtocol, 
                    NewDevicePath
                    );
        if ( EFI_ERROR( Status )) {
            return Status;
        }
    }
    
    if (Parameters->DevPathProtocol) {
        pBS->FreePool (Parameters->DevPathProtocol);
    }
    Parameters->DevPathProtocol = NewDevicePath;
    return EFI_SUCCESS;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EfiSerialSetControl
//
// Description: Protocol interface to set the serial device control bits.
//
// Input:
//  EFI_SERIAL_IO_INTERFACE      *This - Pointer to the Serial IO Interface
//  UINT32                       Control - Contains the new control bit 
//                                         settings
//
// Output:
//  EFI_STATUS
//    EFI_SUCCESS       - The new control bits were set on the serial device.
//    EFI_UNSUPPORTED   - The serial device does not support this operation.
//    EFI_DEVICE_ERROR  - The serial device is not functioning correctly.
//
// Modified:
//      
// Referrals: SerialReadPort, SerialWritePort
//
// Notes:	
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EfiSerialSetControl (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    IN UINT32                   Control
    )
{
    SERIAL_PARAMETERS       *Parameters = (SERIAL_PARAMETERS *)This;
    UINT8                   Temp;
    
    //
    // Check for valid parameters. Only the controls checked here can be set.
    //
    if (Control & ~(EFI_SERIAL_DATA_TERMINAL_READY |
                    EFI_SERIAL_REQUEST_TO_SEND |
                    EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE |
                    EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE |
                    EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE)) {
        return EFI_UNSUPPORTED;
    }
    
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_MCR);
    Temp &= ~(DTRC | RTS | LME);    // Kill DTRC, RTS, and LME bits.
    
    if (Control & EFI_SERIAL_DATA_TERMINAL_READY) {
        Temp |= DTRC;
    }
    
    if (Control & EFI_SERIAL_REQUEST_TO_SEND) {
        Temp |= RTS;
    }
    
    if (Control & EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE) {
        Temp |= LME;
    }
    
  SerialWritePort(Parameters, SERIAL_REGISTER_MCR, Temp);
    
    Parameters->SerialMode.ControlMask = 0;
    if (Control & EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) {
        Parameters->SerialMode.ControlMask |= 
                                EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
    }
    
    if (Control & EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE) {
        Parameters->SerialMode.ControlMask |= 
                                    EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE;
    } else if (Control & EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) {
        Parameters->SerialMode.ControlMask |= 
                                    EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE;
    } 
    
    return EFI_SUCCESS;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EfiSerialGetControl
//
// Description: Protocol interface to get the serial device control bits.
//
// Input:
//  EFI_SERIAL_IO_INTERFACE         *This - Pointer to the Serial IO Interface
//  OUT UINT32                      *Control
//
// Output:
//  EFI_STATUS
//    EFI_SUCCESS       - The new control bits were set on the serial device.
//    EFI_DEVICE_ERROR  - The serial device is not functioning correctly.
//  *Control            - Control bit settings
//
// Modified:
//      
// Referrals: SerialReadPort, SerialWritePort, SerialReceiveTransmit
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EfiSerialGetControl (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    OUT UINT32                  *Control
    )
{
    SERIAL_PARAMETERS       *Parameters = (SERIAL_PARAMETERS *)This;
    
    UINT8                   Temp;
    UINT8                   LSRValue; 
    
    *Control = 0;
    
    //
    // Set the Modem Status Register controls.
    //
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_MSR);
    
    if (Temp & CTS) {
        *Control |= EFI_SERIAL_CLEAR_TO_SEND;
    }
    
    if (Temp & DSR) {
        *Control |= EFI_SERIAL_DATA_SET_READY;
    }
    
    if (Temp & RI) {
        *Control |= EFI_SERIAL_RING_INDICATE;
    }
    
    if (Temp & DCD) {
        *Control |= EFI_SERIAL_CARRIER_DETECT;
    }
    
    //
    // Set the Modem Control Register controls.
    //
    Temp = SerialReadPort(Parameters, SERIAL_REGISTER_MCR);
    
    if (Temp & DTRC) {
        *Control |= EFI_SERIAL_DATA_TERMINAL_READY;
    }
    
    if (Temp & RTS) {
        *Control |= EFI_SERIAL_REQUEST_TO_SEND;
    }
    
    if (Temp & LME) {
        *Control |= EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE;
    }
    
    if (Parameters->SerialMode.ControlMask & 
                                EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) {
        *Control |= EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE;
    }
    
    //
    // If the Transmit FIFO is empty, set the control bit for that.
    //
    LSRValue = SerialReadPort(Parameters, SERIAL_REGISTER_LSR);
    if (LSRValue & THRE) {
        *Control |= EFI_SERIAL_OUTPUT_BUFFER_EMPTY;
    }
    
    //
    // If the Receive FIFO is empty, set the control bit for that.
    //

    if (LSRValue & OE) {
        Parameters->LineStatusRegOverrunErrorBit = TRUE; 
    }
    if (!(LSRValue & DR)) {
        *Control |= EFI_SERIAL_INPUT_BUFFER_EMPTY;
    }

    //
    // If loopback has been enabled, set the control bit for that.
    //
    if (Parameters->SerialMode.ControlMask 
                                & EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) {
        *Control |= EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE;
    }
    
    return EFI_SUCCESS;
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EfiSerialWrite
//
// Description: Protocol interface to write data to a serial device.
//              We use a semaphore Parameters->ReadWriteSemaphore.
//
// Input:
//  EFI_SERIAL_IO_INTERFACE       *This - Pointer to the Serial IO Interface
//  UINTN                         *BufferSize - Number of character to 
//                                              write/number written
//  VOID                          *Buffer - Data buffer
//
// Output:
//  EFI_STATUS
//    EFI_SUCCESS       - The data was written.
//    EFI_DEVICE_ERROR  - The device reported an error.
//    EFI_TIMEOUT       - The data write was stopped due to a timeout.
//  *BufferSize         - Number of bytes written
//
// Modified: *BufferSize
//      
// Referrals: SerialReceiveTransmit, SerialFifoPut, CheckSerialFifoEmpty
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EfiSerialWrite (
    IN EFI_SERIAL_IO_PROTOCOL   *This,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    )
{
    SERIAL_PARAMETERS       *Parameters = (SERIAL_PARAMETERS *)This;
    UINT8                   *ByteBuffer;
    UINT8                   Byte = 0;
    UINT32                  Index;
    UINTN                   WaitTime = 0;
    UINTN                   BytesWritten = 0;
    BOOLEAN                 Handshaking = FALSE;
    
    BOOLEAN                 HardwareLoopbackMode = 
                                (Parameters->SerialMode.ControlMask & 
                                    EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE) ?
                                        TRUE : FALSE;
    EFI_STATUS              HardwareLoopbackModeStatus = EFI_SUCCESS; 
    EFI_STATUS              Status; 
    UINTN                   DataSize=0;
    
    if (*BufferSize == 0) {
        return EFI_SUCCESS;
    }
    
    if (!Buffer) {
        return EFI_DEVICE_ERROR;
    }
    
    if (Parameters->SerialDevicePresentOnPort) {
        *BufferSize = 0;
        return EFI_DEVICE_ERROR;
    }
    //
    // if the Serial Write Error check enabled and if it reaches the maximum error count
    // return serial port write as error always and return error if host is not ready
    // to accept data(i.e CTS is not Set) when flow control is enabled.
    //
    if((SerialWriteErrorCheck && Parameters->SerialPortErrorCount >= MaximumSerialWriteErrorCount) || (Parameters->FlowCtrlCTSnotSet)) {
        *BufferSize = 0;

        if( Parameters->TimerEventActive == FALSE) {

            //
            // Enable the Port active checking for every one second. This event will
            // also check if CTS is Set(host ready to accept data).
            //
            pBS->SetTimer(Parameters->SerialErrorReEnableEvent,
                            TimerPeriodic,
                            10000000);
    
            Parameters->TimerEventActive=TRUE;
        }

        return EFI_DEVICE_ERROR;
    }
    
    ByteBuffer = (UINT8 *)Buffer;
    
    if (HardwareLoopbackMode) {
        if (*BufferSize > 16) {
            *BufferSize = 16;
            HardwareLoopbackModeStatus = EFI_TIMEOUT; 
        } else {
            HardwareLoopbackModeStatus = EFI_SUCCESS; 
        }
    }
    
    if ( Parameters->ReadWriteSemaphore ) {
        return EFI_TIMEOUT;
    }
    Parameters->ReadWriteSemaphore = 1;

    //
    // Software loopback section. Minimum code to pass SCT test.
    //
    if (Parameters->SerialMode.ControlMask & 
        EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) {
        while(DataSize < *BufferSize) {
            if(CheckSerialFifoFull(Parameters)) {
                AddSerialFifo(Parameters,ByteBuffer[DataSize]);
            } else {
                *BufferSize=DataSize;
                Status = EFI_OUT_OF_RESOURCES;
                goto ReleaseSemaphore;
            }
            DataSize++;
        }
        Status = EFI_SUCCESS;
        goto ReleaseSemaphore;
    }
    
    // Software flow control section.
    // Software flow control path
    if (Parameters->SerialMode.ControlMask & 
        EFI_SERIAL_SOFTWARE_FLOW_CONTROL_ENABLE) { 
        if (Parameters->WaitForXon) {
            // read in loop until Xon received
            do
            {
                Byte = SerialReadPort( Parameters, SERIAL_REGISTER_LSR );

                if ( Byte & OE ) {
                    Parameters->LineStatusRegOverrunErrorBit = TRUE;
                }
            } while ((Byte & DR) == FALSE );
            Byte =     SerialReadPort( Parameters, SERIAL_REGISTER_RBR );

            if ( Byte == XON ) {
                Parameters->WaitForXon = FALSE;
            }
            *BufferSize = BytesWritten;
            Status = EFI_TIMEOUT;
            goto ReleaseSemaphore;
        }
    }
    
    // Hardware flow control section.
    // Note: This code takes in to consideration two facts about hardware 
    // handshaking:
    // 1. Hardware handshaking is for the benefit of the terminal, not the 
    //    serial port.
    //    Therefore, the handshaking loop is outside the port write loop.
    // 2. Hardware handshaking is NOT standardized. What is done here works
    //    with Hyperterminal, and it is not known if it will work with other
    //    terminal programs or terminals.
    // Hardware flow control path
    if ((Parameters->SerialMode.ControlMask & 
        EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) && !(Parameters->FlowCtrlCTSnotSet)) {   
        Handshaking = TRUE;
        WaitTime = 0;
        Parameters->ErrorCount = 0;
        Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MCR);
        Byte |= RTS; // Set RTS
        SerialWritePort(Parameters, SERIAL_REGISTER_MCR, Byte);
        
        do { // Wait for CTS
            Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MSR);
            
            if ((Byte & CTS) == FALSE) {
                pBS->Stall(1);
                WaitTime += 1;
                if (WaitTime == 100000) { // 1/10 sec
                    WaitTime = 0;
                    Parameters->ErrorCount += 1;
                    // Turn off Redirection if too many errrors.
                    if (Parameters->ErrorCount == MaxFailuresAllowed) {
                        //
                        // Disbale Redirection if host is not ready to accept data.
                        // Redirection will be enabled again once CTS is set.
                        //
                        Parameters->FlowCtrlCTSnotSet = TRUE;
                        Handshaking = FALSE;
                        Status = EFI_DEVICE_ERROR;
                        goto ReleaseSemaphore;
                    }
                }
            }
        } while ((Byte & CTS) == FALSE);
    }
    
    // Serial port write loop.
    for (Index = 0; Index < *BufferSize; Index ++) {
        WaitTime = 0;        
        do {
            pBS->Stall(1);
            WaitTime+= 1;
            if (WaitTime == 100000) { // 1/10 sec
                *BufferSize = BytesWritten;
                Status = EFI_SUCCESS;
                //
                //Increase the Error Count on Serial Write.
                //                    
                Parameters->SerialPortErrorCount++;
                goto ReleaseSemaphore;
            }
            Byte = SerialReadPort(Parameters, SERIAL_REGISTER_LSR);
            if (Byte & OE) {
                Parameters->LineStatusRegOverrunErrorBit = TRUE; 
            }
            // Loop until the serial port is ready for the next byte
        } while ((Byte & THRE) == FALSE);

        //
        // Reset the Error Count When there is a successfull Serial Write.
        //
        Parameters->SerialPortErrorCount=0;
        SerialWritePort(Parameters, SERIAL_REGISTER_THR, ByteBuffer[Index]);
        BytesWritten++; // Count bytes written
        
        if (HardwareLoopbackMode) {
            WaitTime = 0;        
            do {
                pBS->Stall(1);
                WaitTime+= 1;
                if (WaitTime == 100000) { // 1/10 sec
                    *BufferSize = BytesWritten;
                    Status = EFI_TIMEOUT;
                    goto ReleaseSemaphore;
                }
                Byte = SerialReadPort(Parameters, SERIAL_REGISTER_LSR);
                if (Byte & OE) {
                    Parameters->LineStatusRegOverrunErrorBit = TRUE; 
                }    
            } while ((Byte & DR) == FALSE);
        }
        
    } // end of index loop
    
    if ((Parameters->SerialMode.ControlMask & 
        EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE) ||
        Handshaking) {
        Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MCR); 
        Byte &= ~(RTS); // Clear RTS (signal write is done)
        SerialWritePort(Parameters, SERIAL_REGISTER_MCR, Byte);
        pBS->Stall(1); // Let him see it.
    }
    
    // Hyperterminal could be set to hardware handshaking even if we are not.
    // By setting DTR and RTS here unconditionally, we can still get input.
    Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MCR); 
    //
    // Tell Hyperterminal that it can write to us now.
    //
    Byte |= (DTRC | RTS);  
    SerialWritePort( Parameters, SERIAL_REGISTER_MCR, Byte );

    //
    // Return bytes written IAW spec
    //
    *BufferSize = BytesWritten;  

    if ( HardwareLoopbackMode ) {
        Status = HardwareLoopbackModeStatus;
    } else {
        Status = EFI_SUCCESS;
    }

ReleaseSemaphore:
    Parameters->ReadWriteSemaphore = 0;   
    return Status; 
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EfiSerialRead
//
// Description: Protocol interface to read data from a serial device.
//              In the case of a buffer overrun error, we dump the receive 
//              FIFO buffer.
//              We use a semaphore (Parameters->ReadWriteSemaphore).
//
// Input:
//  EFI_SERIAL_IO_INTERFACE       *This - Pointer to the Serial IO Interface
//  UINTN                         *BufferSize - Size of input buffer/
//                                              characters received
//  VOID                          *Buffer - Data buffer
//
// Output:
//  EFI_STATUS
//    EFI_SUCCESS       - The data was read.
//    EFI_DEVICE_ERROR  - The device reported an error.
//    EFI_TIMEOUT       - The data read was stopped due to a timeout.
//  *BufferSize         - Number of characters received
//
// Modified: *BufferSize
//      
// Referrals: SerialReceiveTransmit, SerialFifoGet
//
// Notes:   
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EfiSerialRead (
    IN EFI_SERIAL_IO_PROTOCOL           *This,
    IN OUT UINTN                        *BufferSize,
    OUT VOID                            *Buffer
    )

{
    EFI_STATUS              Status;
    SERIAL_PARAMETERS       *Parameters = (SERIAL_PARAMETERS *)This;
    UINT32                  Index;
    UINT8                   *ByteBuffer;
    UINT8                   LSRValue; 
    
    UINTN                   PassedInBufferSize = 0; 
    
    ByteBuffer = (UINT8 *)Buffer;
    
    if (*BufferSize == 0) {
        return EFI_SUCCESS;
    }
    
    if (!Buffer) {
        return EFI_DEVICE_ERROR;
    }
    
    if (Parameters->ReadWriteSemaphore) {
      *BufferSize = 0;
      return EFI_TIMEOUT;
    }

    //
    // if the Serial Error check enabled and if it reaches the maximum error count
    // return serial port Read as error always.
    //
    if( (SerialWriteErrorCheck && Parameters->SerialPortErrorCount >= MaximumSerialWriteErrorCount) || (Parameters->FlowCtrlCTSnotSet)) {
        *BufferSize = 0;
        if( Parameters->TimerEventActive == FALSE) {

            //
            // Enable the Port active checking for every one second
            //
            pBS->SetTimer(Parameters->SerialErrorReEnableEvent,
                            TimerPeriodic,
                            10000000);
    
            Parameters->TimerEventActive=TRUE;
        }
        return EFI_DEVICE_ERROR;
    }

    Parameters->ReadWriteSemaphore = 1; 

    //
    // Software loopback section. Minimum code to pass SCT test.
    //
    if (Parameters->SerialMode.ControlMask & 
        EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE) {

        while(PassedInBufferSize < *BufferSize) {
            if(!CheckSerialFifoEmpty(Parameters)) {
                ByteBuffer[PassedInBufferSize]= RemoveSerialFifo(Parameters);
            } else {
                *BufferSize=PassedInBufferSize;
                Status = EFI_TIMEOUT;
                goto ReleaseSemaphore;
            }
            PassedInBufferSize++;
        }
        if ( PassedInBufferSize != *BufferSize ) {
            Status = EFI_TIMEOUT;
        } else {
            Status = EFI_SUCCESS;
        }
        goto ReleaseSemaphore; 
    }
    
DumpFIFOBufferBecauseOfOverrunError:
    if (Parameters->LineStatusRegOverrunErrorBit) {
        //dump everything in the FIFO buffer.  
        for (Index = 0; Index < This->Mode->ReceiveFifoDepth; Index++) {
            SerialReadPort(Parameters, SERIAL_REGISTER_RBR);
        }
        Parameters->LineStatusRegOverrunErrorBit = FALSE; 
        *BufferSize = 0; 
        Status =  EFI_DEVICE_ERROR;
        goto ReleaseSemaphore; 
    }
    
    Index = 0;
    PassedInBufferSize = *BufferSize;
    while (TRUE) {
        LSRValue = SerialReadPort(Parameters, SERIAL_REGISTER_LSR);
        
        if (LSRValue & OE) {
            Parameters->LineStatusRegOverrunErrorBit = TRUE; 
            goto DumpFIFOBufferBecauseOfOverrunError;
        }
        
        if (LSRValue & DR) { 
            ByteBuffer[Index++] = 
                        SerialReadPort(Parameters, SERIAL_REGISTER_RBR);
            if (Index == *BufferSize) {
                Status = EFI_SUCCESS; 
                goto ReleaseSemaphore; 
            }
        } else {
            *BufferSize = Index;
            if ( PassedInBufferSize != *BufferSize ) {
                Status = EFI_TIMEOUT;
            } else {
                Status = EFI_SUCCESS;
            }
            goto ReleaseSemaphore;
        }
    }
ReleaseSemaphore:
    Parameters->ReadWriteSemaphore = 0;     
    return Status; 
}

//
// LOCAL FUNCTIONS
//

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   AddSerialFifo
//
// Description: Add the data to the loopback buffer
//
// Input:
//          IN SERIAL_PARAMETERS *Parameters
//
// Output:
//          None
// Modified:
//      
// Referrals: 
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
VOID AddSerialFifo(
    IN SERIAL_PARAMETERS *Parameters, 
    IN UINT8 Data
    ) 
{
    Parameters->LoopbackBuffer[Parameters->Fifotail] = Data;
    Parameters->Fifotail++;
    Parameters->DataCount++;
    if(Parameters->Fifotail >= Parameters->SerialIo.Mode->ReceiveFifoDepth) {
        Parameters->Fifotail = 0;
    }
    return;
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   RemoveSerialFifo
//
// Description: Remove the data from the loopback buffer
//
// Input:
//          IN SERIAL_PARAMETERS *Parameters
//
// Output:
//          Data
// Modified:
//      
// Referrals: 
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
UINT8 RemoveSerialFifo(
    IN SERIAL_PARAMETERS *Parameters 
    ) 
{
    UINT8   Data;
    Data=Parameters->LoopbackBuffer[Parameters->Fifohead];
    Parameters->Fifohead++;
    Parameters->DataCount--;
    if(Parameters->Fifohead >= Parameters->SerialIo.Mode->ReceiveFifoDepth) {
        Parameters->Fifohead = 0;
    }
    return Data;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   CheckSerialFifoEmpty
//
// Description: Check the loopback buffer empty or not
//
// Input:
//          IN SERIAL_PARAMETERS *Parameters
//
// Output:
//          TRUE - if empty
//          FALSE- if not emptry
// Modified:
//      
// Referrals: 
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN CheckSerialFifoEmpty(
    IN SERIAL_PARAMETERS *Parameters 
    ) 
{
    if(Parameters->DataCount == 0) {
        return TRUE;
    }
    return FALSE;
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   RemoveSerialFifo
//
// Description: check the loop back buffer full or not
//
// Input:
//          IN SERIAL_PARAMETERS *Parameters
//
// Output:
//          TRUE -Not Full
//          FLASE - FULL
// Modified:
//      
// Referrals: 
//
// Notes:
//
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN CheckSerialFifoFull(
    IN SERIAL_PARAMETERS *Parameters 
    ) 
{
    //
    // count of free items in the queue
    //
    if( Parameters->DataCount >= Parameters->SerialIo.Mode->ReceiveFifoDepth ) {
        return FALSE;
    }

    return TRUE;
}

//
// Read and write functions
//
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialReadPort.
//
// Description: Reads a byte from the serial port.
//  
// Input:
//      IN SERIAL_PARAMETERS *Parameters: We use Parameters->BaseAddress.
//      IN UINT32 Offset: The offset from Parameters->BaseAddress.
//
// Output: UINT8 (The data read from the port).
//      
//
//<AMI_PHDR_END>
//***************************************************************
UINT8 
SerialReadPort (
    IN SERIAL_PARAMETERS    *Parameters,
    IN UINT32               Offset
    )

{
    UINT8 Data; 

    if(Parameters->AmiSerialDetected) {
        if(!Parameters->MMIODevice) {
            return IoRead8((UINT32)Parameters->BaseAddress + Offset);
        } else {
            if(PciComMmioWidth == 4) {
                UINT32  TempData;
                Offset = Offset * 4;
                TempData = *(volatile UINT32*)(Parameters->BaseAddress + Offset);
                Data=(UINT8)TempData;
                return Data;
            } else if(PciComMmioWidth == 2) {
                UINT16  TempData;
                Offset = Offset * 2;
                TempData = *(volatile UINT16*)(Parameters->BaseAddress + Offset);
                Data=(UINT8)TempData;
                return Data;
            } else {
                return *(volatile UINT8*)(Parameters->BaseAddress + Offset);
            }
        }        
    }

    if (!Parameters->PciIo) {
        return IoRead8((UINT32)Parameters->BaseAddress + Offset);
    }
    if (Parameters->MMIODevice == TRUE) {

        if(PciComMmioWidth == 4) {
            UINT32  TempData;
            //
            // Mmio access is Dword Length.
            //
            Offset=Offset * 4;
            Parameters->PciIo->Mem.Read (Parameters->PciIo, 
                                EfiPciIoWidthUint32, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &TempData
                                );
            Data=(UINT8)TempData;
            return Data;
        } else if(PciComMmioWidth == 2) {
            UINT16  TempData;
            //
            // Mmio access is Word Length.
            //
            Offset=Offset * 2;
            Parameters->PciIo->Mem.Read (Parameters->PciIo, 
                                EfiPciIoWidthUint16, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &TempData
                                );
            Data=(UINT8)TempData;
            return Data;
        } else {
            //
            // Mmio access is Byte Length.
            //
            Parameters->PciIo->Mem.Read (Parameters->PciIo, 
                                EfiPciIoWidthUint8, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &Data
                                );
            return Data;
        }
    }
    Parameters->PciIo->Io.Read (Parameters->PciIo, 
                                EfiPciIoWidthUint8, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &Data
                                );
    return Data;

}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SerialWritePort.
//
// Description: Write the byte "Data" to the serial port.
//  
// Input:
//      IN SERIAL_PARAMETERS *Parameters: We use Parameters->BaseAddress.
//      IN UINT32 Offset: The offset from Parameters->BaseAddress.
//      IN UINT8 Data
//
// Output: VOID
//      
//
//<AMI_PHDR_END>
//***************************************************************
VOID 
SerialWritePort (
    IN SERIAL_PARAMETERS    *Parameters,
    IN UINT32               Offset,
    IN UINT8                Data
    )

{

    if(Parameters->AmiSerialDetected) {
        if(!Parameters->MMIODevice) {
            IoWrite8((UINT32)Parameters->BaseAddress + Offset, Data);
        } else {
            if(PciComMmioWidth == 4) {
                UINT32  TempData=(UINT32)Data;
                Offset=Offset * 4;
                *(volatile UINT32*)(Parameters->BaseAddress + Offset)= TempData;
            } else if(PciComMmioWidth == 2) {
                UINT16  TempData=(UINT16)Data;               
                Offset=Offset * 2;
                *(volatile UINT16*)(Parameters->BaseAddress + Offset) = TempData;
            } else {
                *(volatile UINT8*)(Parameters->BaseAddress + Offset) = Data;
            }
        }
        return;
    }      

    if (!Parameters->PciIo) {
        IoWrite8((UINT32)Parameters->BaseAddress + Offset, Data);
    } else if (Parameters->MMIODevice == TRUE) {
        if(PciComMmioWidth == 4) {
            UINT32  TempData=(UINT32)Data;
            //
            // Mmio access is Dword Length.
            //
            Offset=Offset * 4;
            Parameters->PciIo->Mem.Write (Parameters->PciIo, 
                                EfiPciIoWidthUint32, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &TempData
                                );
            return;
        } else if(PciComMmioWidth == 2) {
            UINT16  TempData=(UINT16)Data;
            //
            // Mmio access is Word Length.
            //
            Offset=Offset * 2;
            Parameters->PciIo->Mem.Write (Parameters->PciIo, 
                                EfiPciIoWidthUint16, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &TempData
                                );
            return ;
        } else {
            //
            // Mmio access is Byte Length.
            //
            Parameters->PciIo->Mem.Write (Parameters->PciIo, 
                                EfiPciIoWidthUint8, 
                                Parameters->BarIndex,
                                (UINT16) Offset, 
                                1, 
                                &Data
                                );
            return;
        }

    } else {
        Parameters->PciIo->Io.Write (Parameters->PciIo, 
                                    EfiPciIoWidthUint8, 
                                    Parameters->BarIndex,
                                    (UINT16) Offset, 
                                    1, 
                                    &Data
                                    );
    }

    return;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   LookupHID
//
// Description: "hid" should match EISA_PNP_ID(0x501).
//
// Input:
//  UINT32      hid - HID to look for
//  UINT32      uid - UID to look for
//
// Output:
//      TRUE if match is found, FALSE otherwise
//
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN LookupHID(
    UINT32          hid)
{
    return hid == EISA_PNP_ID(0x501);
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetSerialIo_DP
//
// Description: This fuction returns the last node in the device 
//              path for the given controller, which is the Acpi device path
//              for the Serial port.  
//
// Input: 
//      IN EFI_DRIVER_BINDING_PROTOCOL *This
//      IN EFI_HANDLE                  Controller
//      IN OUT ACPI_HID_DEVICE_PATH**  SerialIodp
//      IN UINT32                      Attributes
//      IN BOOLEAN                     Test
//
// Output:
//      EFI_SUCCESS or EFI_UNSUPPORTED
//
//<AMI_PHDR_END>
//**********************************************************************                
EFI_STATUS GetSerialIo_DP(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN OUT ACPI_HID_DEVICE_PATH**  SerialIodp,
    IN UINT32                      Attributes,
    IN BOOLEAN                     Test)
{   
    EFI_STATUS Status;
    ACPI_HID_DEVICE_PATH        *acpiDP;
    EFI_DEVICE_PATH_PROTOCOL    * SerialIoDevPath; 
    
    // Get device path from Controller handle.
    //
    Status = pBS->OpenProtocol (
                    Controller,
                    &gEfiDevicePathProtocolGuid,
                    (void **)&SerialIoDevPath,
                    This->DriverBindingHandle,
                    Controller,   
                    Attributes
                    );
    
    if (EFI_ERROR(Status)) {
        return EFI_UNSUPPORTED;
    }
    
    acpiDP = *SerialIodp = 
                    (ACPI_HID_DEVICE_PATH *)DPGetLastNode(SerialIoDevPath);
    
    Status = (acpiDP->Header.Type == ACPI_DEVICE_PATH &&
                acpiDP->Header.SubType == ACPI_DP) ? 
                EFI_SUCCESS : EFI_UNSUPPORTED;
    if(Test == TRUE){
        pBS->CloseProtocol (
            Controller,
            &gEfiDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
            );
    }
    return Status;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   SetUartType
//
// Description: In our implementation, we check the characteristics of 
//              the FIFO buffers and set the Uart type accordingly to 
//              UART16550A, UART16550, or UART16450.
//
// Input:
//      IN SERIAL_PARAMETERS *Parameters
//
// Output: VOID
//
//<AMI_PHDR_END>
//**********************************************************************  
VOID SetUartType(SERIAL_PARAMETERS *Parameters)
{

    UINT8 IIRValue = 0;
    UINT8 FifoEnableStatus = 0; 

    SerialWritePort(Parameters, SERIAL_REGISTER_FCR, TRFIFOE);
    IIRValue = SerialReadPort(Parameters, SERIAL_REGISTER_IIR);
    FifoEnableStatus = IIRValue & FIFOES;

    if (FifoEnableStatus == FIFO_ENABLED) {
        Parameters->UartType = UART16550A;
    } else if (FifoEnableStatus == FIFO_ENABLED_UNUSABLE) {
        Parameters->UartType = UART16550;
    } else if (FifoEnableStatus == FIFO_DISABLED) {
        Parameters->UartType = UART16450; //UartType is 16450 or 8250
    }

}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   EnableFifoBuffers
//
// Description: If the UART is of type 16550A or a later one, 
//                  if "NewFifoDepth" is 1, the FIFO buffers are disabled;
//                  else they are enabled.
//              If the UART is of a earlier type than 16550A 
//              (in which case there are no FIFO buffers), 
//              the FIFO buffers are disabled. 
//
// Input:
//      IN SERIAL_PARAMETERS *Parameters
//      IN UINT32 NewFifoDepth
//
// Output: VOID
//
//<AMI_PHDR_END>
//**********************************************************************  
VOID EnableFifoBuffers(IN SERIAL_PARAMETERS *Parameters, IN UINT32 NewFifoDepth)
{
    if (Parameters->UartType < UART16550A) {
        SerialWritePort(Parameters, SERIAL_REGISTER_FCR, ~TRFIFOE);
    } else {
        if (NewFifoDepth == 1) {
            SerialWritePort(Parameters, SERIAL_REGISTER_FCR, ~TRFIFOE);
        } else {
            SerialWritePort(Parameters, SERIAL_REGISTER_FCR, 
                            TRFIFOE|RESETRF|RESETTF);
        }
    }

}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   CheckForLoopbackDevice
//
// Description: Checks if a hardware loopback plug is attached and sets 
//              the result in Parameters->SerialDevicePresentOnPort. 
//
// Input:       IN EFI_SERIAL_IO_PROTOCOL *SerialIo
//
// Output: VOID
//
//<AMI_PHDR_END>
//**********************************************************************  
BOOLEAN CheckForLoopbackDevice(IN EFI_SERIAL_IO_PROTOCOL *SerialIo)
{
    SERIAL_PARAMETERS       *Parameters=(SERIAL_PARAMETERS *)SerialIo;
    UINT8   Byte;
    UINT8   Byte2;
    UINT8   FcrValue, McrValue;

    //
    //  If the Signature doesn't match with the Serial Driver Signature,
    //  return FALSE
    //
    if(Parameters->Signature!=EFI_SERIAL_DRIVER_SIGNATURE){
        return FALSE;
    }
    FcrValue = SerialReadPort(Parameters, SERIAL_REGISTER_FCR);

    //
    // Program the FCR
    //
    SerialWritePort(Parameters, SERIAL_REGISTER_FCR, 
                            TRFIFOE|RESETRF|RESETTF);

    Byte = SerialReadPort(Parameters, SERIAL_REGISTER_FCR);
    if(Byte == 0xff) {
        SerialWritePort(Parameters, SERIAL_REGISTER_FCR, FcrValue); 
        Parameters->SerialDevicePresentOnPort = TRUE;
        return TRUE;        
    }
    
    //
    // Wait for 2ms is sufficient for the next byte
    //
    pBS->Stall(2000);

    //
    //check if RECV DATA AVAILABLE IS SET. If Available,Read the data till all data is read
    //
    do {
        Byte = SerialReadPort( Parameters, SERIAL_REGISTER_LSR );
        if(Byte & DR) {
            Byte2 = SerialReadPort( Parameters, SERIAL_REGISTER_RBR );
        }
    } while ((Byte & DR) == TRUE );

    //
    // Write into THR
    //
    SerialWritePort(Parameters, SERIAL_REGISTER_THR,0x80); 

    //
    // Send BackSpace to clear the character(0x80) sent for testing
    // the presence of Loop Back Device.
    //
    SerialWritePort(Parameters, SERIAL_REGISTER_THR,0x08);
    SerialWritePort(Parameters, SERIAL_REGISTER_THR,0x20);
    SerialWritePort(Parameters, SERIAL_REGISTER_THR,0x08);

    //
    // Wait for 5ms is sufficient for the next byte
    //
    pBS->Stall(50000);
   
    Byte = SerialReadPort( Parameters, SERIAL_REGISTER_LSR );

    if(Byte & DR) {
        Byte2 = SerialReadPort( Parameters, SERIAL_REGISTER_RBR );
        if(Byte2 == 0x80) {
            SerialWritePort(Parameters, SERIAL_REGISTER_FCR, FcrValue); 
            Parameters->SerialDevicePresentOnPort = TRUE;
            return TRUE ;  
        }
    }

    if(SerialMouseDetection) {
        //
        // Check for Mouse device on the Serial Port.
        // If we Toggle the DTR BIT, mouse sends data to RCVR BUFFER.
        //
    
        //
        // Program the FCR
        //
        SerialWritePort(Parameters, SERIAL_REGISTER_FCR, 
                                TRFIFOE|RESETRF|RESETTF);
    
        //
        // Wait for 5ms is sufficient for the next byte
        //
        pBS->Stall(5000);
    
        //
        //check if RECV DATA AVAILABLE IS SET. If Available,Read the data till all data is read
        //
        do {
            Byte = SerialReadPort( Parameters, SERIAL_REGISTER_LSR );
            if(Byte & DR) {
                Byte2 = SerialReadPort( Parameters, SERIAL_REGISTER_RBR );
            }
        } while ((Byte & DR) == TRUE );
    
        McrValue = SerialReadPort( Parameters, SERIAL_REGISTER_MCR );
        Byte = SerialReadPort( Parameters, SERIAL_REGISTER_MCR );
    
        //
        // Wait for 2ms is sufficient for the next byte
        //
        pBS->Stall(2000);
    
        if(Byte & DTRC) {
            Byte &= ~(DTRC);
        } else {
            Byte |= DTRC;
        }
    
        SerialWritePort(Parameters, SERIAL_REGISTER_MCR, Byte);
    
        //
        // Wait for 5ms is sufficient for the next byte
        //
        pBS->Stall(5000);
    
        Byte = SerialReadPort( Parameters, SERIAL_REGISTER_LSR );
    
        SerialWritePort(Parameters, SERIAL_REGISTER_FCR, FcrValue); 
        SerialWritePort(Parameters, SERIAL_REGISTER_MCR, McrValue);
        
        //
        // If mouse device connected in Serial port, will be sending the data.
        //
        if(Byte & DR) {
            Parameters->SerialDevicePresentOnPort = TRUE;
            return TRUE; 
        }
    }

    // Check for hardware loopback plug...
    Byte2 = SerialReadPort(Parameters, SERIAL_REGISTER_MCR);
    if (!(Byte2 & LME)) { // if hardware loopback not enabled...
        // Test for loopback plug and return error if detected
        // read MSR
        Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MSR); 
        Byte &= (CTS | DSR | DCD | RI); 
        // check these bits 
        if ((Byte == (CTS | DSR | DCD)) || (Byte == (CTS | DSR | DCD | RI))) {              // check these bits
            // change MCR
            SerialWritePort(Parameters, SERIAL_REGISTER_MCR, 0);
            // read MSR again 
            Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MSR);
            // restore MCR
            SerialWritePort(Parameters, SERIAL_REGISTER_MCR, Byte2);
             // if change loops back...
            if ((Byte & 0xf0) == 0) {
                Parameters->SerialDevicePresentOnPort = TRUE;
                return TRUE;
            }
        }
    }
    return FALSE;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   CheckThreBit
//
// Description: Serial Read and Write will be failed incase if it's reaches the maximum error count
//              Once it reaches the error count, this event will be activated to monitor the serial port
//              working state. If Transmistter Holding Register ready to accept the data, Error count will 
//              be reset so that redirection will start working again.
//
//              Also this event fucntion will check if host can accept data(CTS is Set)
//              when flow control is enabled. If Set it resets FlowCtrlCTSnotSet so that
//              redirection will start working again.
//
// Input:
//              IN EFI_EVENT    Event
//              IN VOID         *Context
//
// Output:
//              VOID
//
//<AMI_PHDR_END>
//**********************************************************************
VOID CheckThreBit(
    EFI_EVENT Event, 
    VOID *Context
)
{
    SERIAL_PARAMETERS   *Parameters=(SERIAL_PARAMETERS*)((UINTN*)Context);
    UINT8               Byte=0;

    if(Parameters == NULL) {
        return;
    }

    if( !Parameters->FlowCtrlCTSnotSet ) {

        Byte = SerialReadPort(Parameters, SERIAL_REGISTER_LSR);

        //
        // Check the Transmistter Holding Register Empty Status
        //
        if(!(Byte & THRE)) {
            return;
        }
        
        //  
        // Serial Port ready to accept the data. 
        //                    
        Parameters->SerialPortErrorCount=0;
    } else {

        Byte = SerialReadPort(Parameters, SERIAL_REGISTER_MSR);

        //
        // Check whether Host can accept data.
        //
        if(!(Byte & CTS)) {
            return;
        }  
        Parameters->FlowCtrlCTSnotSet = FALSE;
		
    }
        
    //
    // Disable the Event 
    //
    pBS->SetTimer(Parameters->SerialErrorReEnableEvent,
                        TimerCancel,
                        0);

    Parameters->TimerEventActive=FALSE;


    return;
}


//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
