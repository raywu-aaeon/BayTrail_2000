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
// $Header: /Alaska/SOURCE/Modules/Legacy Serial Redirection/LegacySredir_Setup.C 31    2/27/12 6:04a Jittenkumarp $
//
// $Revision: 31 $
//
// $Date: 2/27/12 6:04a $
//****************************************************************************

//<AMI_FHDR_START>
//****************************************************************************
//
// Name:        LegacySredir_Setup.C
//
// Description: This File is used to get the Terminal Setup values from EFI
//****************************************************************************
//<AMI_FHDR_END>

#pragma warning ( disable : 4214 )

#include "token.h"
#include "Protocol/LegacySredir.h"
#include <Setup.h>
#include <AmiDxeLib.h>          
#include <Protocol/AmiSio.h>
#include <Protocol/DevicePath.h>
#include <AcpiRes.h>
#include <Protocol/PciIo.h>
#include <PCI.h>
#include <Protocol\TerminalAmiSerial.h>
#include <LegacySredirElink.h>

EFI_STATUS GetSetupValuesForLegacySredir(
        OUT EFI_COM_PARAMETERS *EfiComParameters
        );  


#define STOPB   0x4                 //      Bit2: Number of Stop Bits
#define PAREN   0x8                 //      Bit3: Parity Enable
#define EVENPAR 0x10                //      Bit4: Even Parity Select
#define STICPAR 0x20                //      Bit5: Sticky Parity
#define SERIALDB 0x3                //      Bit0-1: Number of Serial 
                                    //                 Data Bits

EFI_COM_PARAMETERS  gEfiComParameters;
BOOLEAN             IsFound = FALSE;
BOOLEAN             IsPciDevice;


#define TERMINAL_VAR_GUID \
{0x560bf58a, 0x1e0d, 0x4d7e, 0x95, 0x3f, 0x29, 0x80, 0xa2, 0x61, 0xe0, 0x31}
EFI_GUID gTerminalVarGuid   = TERMINAL_VAR_GUID;
UINT32 gTotalSioSerialPorts = TOTAL_SIO_SERIAL_PORTS;
#define SIO_SERIAL_PORTS_LOCATION_VAR_C_NAME    L"SioSerialPortsLocationVar"
#define PCI_SERIAL_PORTS_LOCATION_VAR_C_NAME    L"PciSerialPortsLocationVar"
#define SERIAL_PORTS_ENABLED_VAR_C_NAME         L"SerialPortsEnabledVar"

#if (TOTAL_PCI_SERIAL_PORTS > 0)
    typedef struct { 
        UINT8 Segment[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 Bus[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 Device[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 Function[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 AmiPciSerialPresent[ TOTAL_PCI_SERIAL_PORTS ];
        UINT8 Port[ TOTAL_PCI_SERIAL_PORTS ];
    } PCI_SERIAL_PORTS_LOCATION_VAR;
#endif
#if (TOTAL_SIO_SERIAL_PORTS > 0)
    typedef struct { 
        UINT8 PortUid[ TOTAL_SIO_SERIAL_PORTS ];
        UINT8 Valid[ TOTAL_SIO_SERIAL_PORTS ];
    } SIO_SERIAL_PORTS_LOCATION_VAR;
#endif

#if (TOTAL_SERIAL_PORTS > 0)
    typedef struct { 
        UINT8 PortsEnabled[ TOTAL_SERIAL_PORTS ];
    } SERIAL_PORTS_ENABLED_VAR;
#endif

UINT32 gComBaudRates[8] = {0, 0, 0, 9600, 19200, 38400, 57600, 115200};
#define SERIAL_REGISTER_SCR 7    // R/W  Scratch Pad Register

#pragma pack(1)
typedef struct { 
    UINT16 VendorId; 
    UINT16 DeviceId; 
} INVALID_PCICOM;
#pragma pack()

INVALID_PCICOM InvalidPciCom[] = {INVALID_PCICOM_DEVICELIST
                                  {0xFFFF, 0xFFFF}
                                 };

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
// Name:        InitilizeNonCommonSerialRegsiters	
//
// Description: This Function is used to Initilize the Non Standard 
//              Serial Port Registers 
//
// Input:       EfiComParameters-Address of the COM port parameters Structure
//	
// Output:      None 
//
// Notes:       Some of the MMIO com ports has non Standard bits in Registers
//              Those regsiters are all initilized on this function.
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InitilizeNonCommonSerialRegsiters(
    IN EFI_COM_PARAMETERS *EfiComParameters
)
{
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:        ValidateComPort
//
// Description: Validate the COM port using Scratch Pad Registers. 
//
// Input:       IN  UINT32   BaseAddress, 
//              IN  BOOLEAN  MmioDevice
//
// Output:      Comport number
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
ValidateComPort(
    IN  UINT32  BaseAddress, 
    IN  BOOLEAN MmioDevice
)
{
    UINT32  TempData32=0xAA;
    UINT8   TempData8=0xAA;

    if(MmioDevice) {
        //
        // Write into the Scratch Pad Reg
        //

#if COM_MMIO_WIDTH == 4
        *(UINT32*)(BaseAddress +(SERIAL_REGISTER_SCR*COM_MMIO_WIDTH))=TempData32;
        TempData32=*(UINT32*)(BaseAddress +(SERIAL_REGISTER_SCR*COM_MMIO_WIDTH));
#else
    #if COM_MMIO_WIDTH == 2
        *(UINT16*)(BaseAddress +(SERIAL_REGISTER_SCR*COM_MMIO_WIDTH))=(UINT16)TempData32;
        TempData32=*(UINT16*)(BaseAddress +(SERIAL_REGISTER_SCR*COM_MMIO_WIDTH));
    #else
        *(UINT8*)(BaseAddress +(SERIAL_REGISTER_SCR*COM_MMIO_WIDTH))=(UINT8)TempData32;
        TempData32=*(UINT8*)(BaseAddress +(SERIAL_REGISTER_SCR*COM_MMIO_WIDTH));
    #endif
#endif 
        //
        // Compare the read value Write Value, Both are same, Port is Valid
        //
        if((UINT8)TempData32 == 0xAA) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        //
        // Write into Scratch Pad ISA Com port 
        //
        IoWrite8(BaseAddress + SERIAL_REGISTER_SCR, TempData8);
        TempData8=IoRead8(BaseAddress + SERIAL_REGISTER_SCR);
        //
        // Compare the read value Write Value, Both are same, Port is Valid
        //
        if(TempData8 == 0xAA) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:        GetPciSerialComPortNumber	
//
// Description: This Function is used to get the Comport number to map Terminal Driver Setup values
//
// Input:       IN  UINT8   Device, 
//              IN  UINT8   Function
//
// Output:      Comport number
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
GetPciSerialComPortNumber(
IN  UINT8   Device, 
IN  UINT8   Function,
IN  UINT8    PortNo
)
{
#if (TOTAL_PCI_SERIAL_PORTS > 0)
    UINT32      ComPort;
    UINT32      i = 0;
    UINTN       PciSerialPortsLocationVarSize = 
                                    sizeof(PCI_SERIAL_PORTS_LOCATION_VAR);
    UINT32      PciSerialPortsLocationVarAttributes=0;
    PCI_SERIAL_PORTS_LOCATION_VAR PciSerialPortsLocationVar; 
    EFI_STATUS  Status;

    UINTN SerialPortsEnabledVarSize = sizeof(SERIAL_PORTS_ENABLED_VAR); 
    UINT32 SerialPortsEnabledVarAttributes=0;
    SERIAL_PORTS_ENABLED_VAR SerialPortsEnabledVar; 

    Status = pRS->GetVariable(SERIAL_PORTS_ENABLED_VAR_C_NAME, 
                                &gTerminalVarGuid,
                                &SerialPortsEnabledVarAttributes, 
                                &SerialPortsEnabledVarSize, 
                                &SerialPortsEnabledVar);

    if(EFI_ERROR(Status)) {
        return 0xFF;
    }

    Status = pRS->GetVariable(PCI_SERIAL_PORTS_LOCATION_VAR_C_NAME, 
                                &gTerminalVarGuid, 
                                &PciSerialPortsLocationVarAttributes, 
                                &PciSerialPortsLocationVarSize, 
                                &PciSerialPortsLocationVar);

    if(EFI_ERROR(Status)) {
        return 0xFF;
    }

    ComPort = 0xFF; 

    for (i = 0; i < TOTAL_PCI_SERIAL_PORTS; i++) {
        if ((SerialPortsEnabledVar.PortsEnabled[gTotalSioSerialPorts+i]) && 
            (PciSerialPortsLocationVar.Device[i] == Device) && 
            (PciSerialPortsLocationVar.Function[i] == Function)) {
                if( PciSerialPortsLocationVar.AmiPciSerialPresent[i] ) {
                     if( PciSerialPortsLocationVar.Port[i] != PortNo ) continue;
                }
                ComPort = gTotalSioSerialPorts+i; 
                break;            
        }
    }	
    return ComPort;
#else 
    return 0xFF;
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:        SkipInvalidPciComDevice
//
// Description: Skip the Invalid PCI COM device that is provided in the 
//              InvalidPciComDeviceList 
//
// Input:       IN  UINT16 VendorId, 
//              IN  UINT16 DeviceId,
//
// Output:      TRUE - If the device has to be skipped
//              FALSE - Don't Skip the device
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
SkipInvalidPciComDevice(
    IN  UINT16 VendorId, 
    IN  UINT16 DeviceId 
)
{
    UINT8   i=0;

    while(  InvalidPciCom[i].VendorId != 0xFFFF && 
            InvalidPciCom[i].DeviceId != 0xFFFF ) {

        if( InvalidPciCom[i].VendorId == VendorId && 
            InvalidPciCom[i].DeviceId == DeviceId ) {

            return TRUE;
        }
        i++;
    }

    return FALSE;
}
	

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:        GetSetupValuesForLegacySredir	
//
// Description: This Function is used to get the Setup Values of Terminal Module
//
// Input:       Nothing
//	
//
// Output:      EfiComparameters - SerialPort, Baudrate, Terminal Type, Flowcontrol
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetSetupValuesForLegacySredir(
OUT EFI_COM_PARAMETERS *EfiComParameters)

{
#if (TOTAL_SERIAL_PORTS == 0)
    return EFI_NOT_FOUND;
#else
    EFI_STATUS          Status;
    EFI_GUID            SetupGuid = SETUP_GUID; 
    SETUP_DATA          SetupData; 
    EFI_DEVICE_PATH_PROTOCOL    *SerialDevicePath = NULL;
    EFI_DEVICE_PATH_PROTOCOL    *AmiSioDevicePath = NULL;
    EFI_DEVICE_PATH_PROTOCOL    *TruncatedSerialDevicePath = NULL;
    ACPI_HID_DEVICE_PATH        *AcpiPrevDPNodePtr = NULL;
    AMI_SIO_PROTOCOL    *AmiSioProtocol=NULL;
    EFI_HANDLE          *HandleBuffer, AmiSioProtocolHandle=0;
    UINTN               HandleCount, Index;
    ASLRF_S_HDR         *Header=NULL;
    T_ITEM_LIST         *ResourcesList=NULL;
    UINTN               IrqInBitPosition=0;
    UINT32              SetupDataAttributes = 0;
    UINTN               SetupDataSize = sizeof(SETUP_DATA); 
    UINT8               TerminalTypes[4] = {1, 2, 2, 0};
    UINT8               DataParityStopBit=0;
    UINT8               TempSetupValue;
	EFI_PCI_IO_PROTOCOL		*PciIo;
    EFI_HANDLE              *PciHandleBuffer;
    UINTN                   PciHandleCount;
    UINT8                   RevisionId[4];
    UINT64                  Supports=0;
    ASLR_QWORD_ASD          *Resources=NULL;
    EFI_DEVICE_PATH_PROTOCOL    *tmpdp=NULL;
    EFI_HANDLE              TempHandle;
    EFI_PCI_IO_PROTOCOL     *RootPciIo=NULL;
    EFI_DEVICE_PATH_PROTOCOL *TruncatedDevPath = NULL;
    EFI_DEVICE_PATH_PROTOCOL *TempDevPath = NULL;
    UINT16                  CommandRegValue; 
    UINT8                   ResIndex;
    UINT16                  CommandReg=0; 
    UINTN                   Bus=0, Device=0, Function=0, SegNum=0;
    UINT8                   Port=0;
    UINT32                  ComPort=0;
    UINT16                  VendorId,DeviceId; 
#if (TOTAL_SIO_SERIAL_PORTS > 0)
    UINTN                   i;
    UINTN                   k;
    UINTN                   ComPortNo;
    UINTN                   SioSerialPortsLocationVarSize = 
                                    sizeof(SIO_SERIAL_PORTS_LOCATION_VAR);
    UINT32                  SioSerialPortsLocationVarAttributes=0;
    SIO_SERIAL_PORTS_LOCATION_VAR SioSerialPortsLocationVar;
#endif

    AMI_SERIAL_PROTOCOL      *AmiSerialProtocol=NULL;
    BOOLEAN                  Pci;
    BOOLEAN                  Mmio;
    UINT64                  BaseAddress;

    if (IsFound) {
        *EfiComParameters = gEfiComParameters;
        return EFI_SUCCESS;
    }

    Status = pRS->GetVariable(L"Setup", &SetupGuid, &SetupDataAttributes,
                                &SetupDataSize, &SetupData);

    if (EFI_ERROR (Status)) {
        return EFI_NOT_FOUND;
    }

    //
    // Handle AMI_SERIAL_PROTOCOL.
    //
    Status = pBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gAmiSerialProtocolGuid,
                                    NULL,
                                    &PciHandleCount,
                                    &PciHandleBuffer);

    for (Index = 0; Index < PciHandleCount; Index++) {
        Status = pBS->HandleProtocol (
                                    PciHandleBuffer[Index],
                                    &gAmiSerialProtocolGuid,
                                    &AmiSerialProtocol);
        
         if (EFI_ERROR (Status)) {
            continue;
        }

             AmiSerialProtocol->GetPciLocation(AmiSerialProtocol,&Bus,&Device,&Function,&Port);

            ComPort=GetPciSerialComPortNumber((UINT8)Device, (UINT8)Function,Port);

            if(ComPort==0xFF) {
                continue;
            }
            if (!SetupData.ConsoleRedirectionEnable[ComPort]) {
                continue;
            }

            pBS->SetMem(&gEfiComParameters, (sizeof(EFI_COM_PARAMETERS)), 0);

            AmiSerialProtocol->CheckPciMmio(AmiSerialProtocol,&Pci,&Mmio);
            AmiSerialProtocol->GetBaseAddress(AmiSerialProtocol,&BaseAddress);
            
            if(Mmio) {
                gEfiComParameters.MMIOBaseAddress= (UINT32)BaseAddress;
            } else {    
                gEfiComParameters.BaseAddress= (UINT16)BaseAddress;
            }

            if(Pci) {
                IsPciDevice = TRUE;
            } else {
                IsPciDevice = FALSE;
            }
        
             AmiSerialProtocol->GetSerialIRQ(AmiSerialProtocol,&(gEfiComParameters.SerialIRQ)); 

                //
                //Other settings from Termial Redirection driver
                //
                gEfiComParameters.Baudrate      = gComBaudRates[SetupData.BaudRate[ComPort]];
                gEfiComParameters.TerminalType  = TerminalTypes[SetupData.TerminalType[ComPort]];
                gEfiComParameters.FlowControl   = SetupData.FlowControl[ComPort];
                gEfiComParameters.LegacyOsResolution   = SetupData.LegacyOsResolution[ComPort];
                gEfiComParameters.RecorderMode   = SetupData.RecorderMode[ComPort];
                gEfiComParameters.VtUtf8   = SetupData.VtUtf8[ComPort];
                gEfiComParameters.PuttyKeyPad   = SetupData.PuttyFunctionKeyPad[ComPort];
#if  (INSTALL_LEGACY_OS_THROUGH_REMOTE == 1)
                gEfiComParameters.InstallLegacyOSthroughRemote  = SetupData.InstallLegacyOSthroughRemote[ComPort];
#endif
                gEfiComParameters.RedirectionAfterBiosPost = SetupData.RedirectionAfterBiosPost[ComPort] ;


                TempSetupValue=SetupData.Parity[ComPort];
                // Set parity bits.
                switch (TempSetupValue) {
                    case NoParity:
                            DataParityStopBit &= ~(PAREN | EVENPAR | STICPAR);
                            break;
                    case EvenParity:
                            DataParityStopBit |= (PAREN | EVENPAR);
                            DataParityStopBit &= ~STICPAR;
                            break;
                    case OddParity:
                            DataParityStopBit |= PAREN;
                            DataParityStopBit &= ~(EVENPAR | STICPAR);
                            break;
                    case SpaceParity:
                            DataParityStopBit |= (PAREN | EVENPAR | STICPAR);
                            break;
                    case MarkParity:
                            DataParityStopBit |= (PAREN | STICPAR);
                            DataParityStopBit &= ~EVENPAR;
                            break;
                }
    
                TempSetupValue=SetupData.StopBits[ComPort];

                // Set stop bits.
                switch (TempSetupValue) {
                    case OneStopBit :
                            DataParityStopBit &= ~STOPB;
                            break;
                     case OneFiveStopBits :
                     case TwoStopBits :
                            DataParityStopBit |= STOPB;
                            break;
                }

                TempSetupValue=SetupData.DataBits[ComPort];
   
                // Set data bits.
                DataParityStopBit &= ~SERIALDB;
                DataParityStopBit |= (UINT8)((TempSetupValue - 5) & 0x03);
                gEfiComParameters.DataParityStop= DataParityStopBit;

                IsFound = TRUE;
             	*EfiComParameters = gEfiComParameters;
                return EFI_SUCCESS;
    }

    //
    // Handle PCI COM port.
    //

    //
    //Locate All devicepath handles
    //
    Status = pBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &PciHandleCount,
                                    &PciHandleBuffer);

    if (EFI_ERROR (Status)) {
        return Status;
    }

    for (Index = 0; Index < PciHandleCount; Index++) {
        Status = pBS->HandleProtocol (
                                    PciHandleBuffer[Index],
                                    &gEfiPciIoProtocolGuid,
                                    &PciIo); 


        PciIo->Pci.Read (PciIo,
                        EfiPciIoWidthUint32,
                        PCI_REV_ID_OFFSET,
                        1,
                        &RevisionId
                        );

        if ((RevisionId[3] == PCI_CL_COMM) &&
            ((RevisionId[2] == PCI_CL_COMM_CSL_SERIAL) || (RevisionId[2] == PCI_CL_COMM_CSL_OTHER))) {

            Status = PciIo->GetLocation(PciIo, &SegNum, &Bus,&Device,&Function);
            if (EFI_ERROR (Status)) {
                continue;
            }

            PciIo->Pci.Read (PciIo,
                             EfiPciIoWidthUint16,
                             PCI_VID,
                             1,
                             &VendorId
                             );
            PciIo->Pci.Read (PciIo,
                             EfiPciIoWidthUint16,
                             PCI_DID,
                             1,
                             &DeviceId
                             );

            if(SkipInvalidPciComDevice(VendorId,DeviceId)) {
                continue;
            }

            ComPort=GetPciSerialComPortNumber((UINT8)Device, (UINT8)Function, Port);

            if(ComPort==0xFF) {
                continue;
            }
            if (!SetupData.ConsoleRedirectionEnable[ComPort]) {
                continue;
            }

            Supports = 1;

            pBS->SetMem(&gEfiComParameters, (sizeof(EFI_COM_PARAMETERS)), 0);

            for (ResIndex = 0; ResIndex < PCI_MAX_BAR_NO; ResIndex++) {
                Status = PciIo->GetBarAttributes (
                                                PciIo,
                                                ResIndex,
                                                &Supports,
                                                &Resources
                                                );

                //
                //Find the Serial device Resource type. Based on that Enable the IO and Memory 
                //
                if (Resources->Type == ASLRV_SPC_TYPE_IO) {
                    gEfiComParameters.BaseAddress= (UINT16)Resources->_MIN;	   
                    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0x3c, 1, &gEfiComParameters.SerialIRQ);
                    CommandReg = PCI_CMD_IO_SPACE;
                    pBS->FreePool(Resources);
                    break;
                } else if(Resources->Type == ASLRV_SPC_TYPE_MEM) {
                    gEfiComParameters.MMIOBaseAddress= (UINT32)Resources->_MIN;	   
                    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0x3c, 1, &gEfiComParameters.SerialIRQ);
                    CommandReg = PCI_CMD_MEMORY_SPACE;
                    pBS->FreePool(Resources);
                    break;
                }
            }

            if(CommandReg != 0) {

                PciIo->Pci.Read (PciIo,
                                EfiPciIoWidthUint16,
                                PCI_COMMAND_REGISTER_OFFSET,
                                1,
                                &CommandRegValue
                                );
                CommandRegValue|=CommandReg;                    
                PciIo->Pci.Write (PciIo,
                                EfiPciIoWidthUint16,
                                PCI_COMMAND_REGISTER_OFFSET,
                                1,
                                &CommandRegValue
                                );

                Status = pBS->HandleProtocol (
                                    PciHandleBuffer[Index],
                                    &gEfiDevicePathProtocolGuid,
                                    &tmpdp); 


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
                                                    &RootPciIo); // Get Device path protocol

                        if(Status == EFI_SUCCESS) {

                            RootPciIo->Pci.Read (RootPciIo,
                                                    EfiPciIoWidthUint16,
                                                    PCI_COMMAND_REGISTER_OFFSET,
                                                    1,
                                                    &CommandRegValue
                                                    );
                            CommandRegValue|=CommandReg;
                            RootPciIo->Pci.Write (RootPciIo,
                                                    EfiPciIoWidthUint16,
                                                    PCI_COMMAND_REGISTER_OFFSET,
                                                    1,
                                                    &CommandRegValue
                                                    );
                        }
                
                    }
                
                }while(TruncatedDevPath != NULL);

                //
                // Check the Port Presence
                //
                if(gEfiComParameters.MMIOBaseAddress != 0) {
                    if(ValidateComPort(gEfiComParameters.MMIOBaseAddress, TRUE) == FALSE) {
                        continue;
                    }
                } else {
                    if(ValidateComPort(gEfiComParameters.BaseAddress, FALSE) == FALSE) {
                        continue;
                    }
                }

                //
                //Other settings from Termial Redirection driver
                //
                gEfiComParameters.Baudrate      = gComBaudRates[SetupData.BaudRate[ComPort]];
                gEfiComParameters.TerminalType  = TerminalTypes[SetupData.TerminalType[ComPort]];
                gEfiComParameters.FlowControl   = SetupData.FlowControl[ComPort];
                gEfiComParameters.LegacyOsResolution   = SetupData.LegacyOsResolution[ComPort];
                gEfiComParameters.RecorderMode   = SetupData.RecorderMode[ComPort];
                gEfiComParameters.VtUtf8   = SetupData.VtUtf8[ComPort];
                gEfiComParameters.PuttyKeyPad   = SetupData.PuttyFunctionKeyPad[ComPort];
#if  (INSTALL_LEGACY_OS_THROUGH_REMOTE == 1)
                gEfiComParameters.InstallLegacyOSthroughRemote  = SetupData.InstallLegacyOSthroughRemote[ComPort];
#endif
                gEfiComParameters.RedirectionAfterBiosPost = SetupData.RedirectionAfterBiosPost[ComPort] ;
                TempSetupValue=SetupData.Parity[ComPort];
                // Set parity bits.
                switch (TempSetupValue) {
                    case NoParity:
                            DataParityStopBit &= ~(PAREN | EVENPAR | STICPAR);
                            break;
                    case EvenParity:
                            DataParityStopBit |= (PAREN | EVENPAR);
                            DataParityStopBit &= ~STICPAR;
                            break;
                    case OddParity:
                            DataParityStopBit |= PAREN;
                            DataParityStopBit &= ~(EVENPAR | STICPAR);
                            break;
                    case SpaceParity:
                            DataParityStopBit |= (PAREN | EVENPAR | STICPAR);
                            break;
                    case MarkParity:
                            DataParityStopBit |= (PAREN | STICPAR);
                            DataParityStopBit &= ~EVENPAR;
                            break;
                }
    
                TempSetupValue=SetupData.StopBits[ComPort];

                // Set stop bits.
                switch (TempSetupValue) {
                    case OneStopBit :
                            DataParityStopBit &= ~STOPB;
                            break;
                     case OneFiveStopBits :
                     case TwoStopBits :
                            DataParityStopBit |= STOPB;
                            break;
                }

                TempSetupValue=SetupData.DataBits[ComPort];
   
                // Set data bits.
                DataParityStopBit &= ~SERIALDB;
                DataParityStopBit |= (UINT8)((TempSetupValue - 5) & 0x03);
                gEfiComParameters.DataParityStop= DataParityStopBit;

                IsFound = TRUE;
                IsPciDevice = TRUE;
             	*EfiComParameters = gEfiComParameters;
                return EFI_SUCCESS;
            }
        }
    }

    //
    // Handle ISA COM port.
    //


    //
    //Locate All devicepath handles
    //
    Status = pBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiDevicePathProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer);

    if (EFI_ERROR (Status)) {
        return Status;
    }
    
#if (TOTAL_SIO_SERIAL_PORTS > 0)
    Status = pRS->GetVariable(SIO_SERIAL_PORTS_LOCATION_VAR_C_NAME, 
                                &gTerminalVarGuid, 
                                &SioSerialPortsLocationVarAttributes, 
                                &SioSerialPortsLocationVarSize, 
                                &SioSerialPortsLocationVar);

    if (EFI_ERROR (Status)) {
        return Status;
    }

    for (Index = 0; Index < HandleCount; Index++) {
        Status = pBS->HandleProtocol (
                                    HandleBuffer[Index],
                                    &gEfiDevicePathProtocolGuid,
                                    &SerialDevicePath); // Get Device path protocol

    if (EFI_ERROR (Status) || (SerialDevicePath == NULL)) continue;

        TruncatedSerialDevicePath = SerialDevicePath;
        //
        //Check for the Serial Port device path
        //
        for (;!isEndNode(TruncatedSerialDevicePath);TruncatedSerialDevicePath = NEXT_NODE(TruncatedSerialDevicePath)) {
            AcpiPrevDPNodePtr = (ACPI_HID_DEVICE_PATH *)(TruncatedSerialDevicePath); //get ACPI device path
            if ((AcpiPrevDPNodePtr->Header.Type == ACPI_DEVICE_PATH) 
                && (AcpiPrevDPNodePtr->Header.SubType == ACPI_DP)
                && (AcpiPrevDPNodePtr->HID == EISA_PNP_ID(0x501))) {

                if(TOTAL_SIO_SERIAL_PORTS == 0) {
                    continue;
                }                                                       

                //
                // Find the Port number ( Setup option offset)
                //
                for (k = 0; k < TOTAL_SIO_SERIAL_PORTS; k++) {
                    if ((SioSerialPortsLocationVar.PortUid[k] == (AcpiPrevDPNodePtr->UID) ) &&
                        (SioSerialPortsLocationVar.Valid[k] == 0xFF)) {                                
                        ComPortNo = k; 
                        break;            
                    }
                }

                if(k == TOTAL_SIO_SERIAL_PORTS) {
                    continue;
                }
                //
                //Check Terminal redirection device status.
                //
                if (!SetupData.ConsoleRedirectionEnable[ComPortNo]) continue;

                //
                //Locate AmiSioProtocol form this handle to get current resource of this device.
                //
                AmiSioDevicePath = SerialDevicePath;		//Truncate End device path

                Status = pBS->LocateDevicePath(&gEfiDevicePathProtocolGuid, 
                                               &AmiSioDevicePath, 
                                               &AmiSioProtocolHandle);

                if (EFI_ERROR (Status)) continue;

                Status = pBS->HandleProtocol (
                                                AmiSioProtocolHandle,
                                                &gEfiAmiSioProtocolGuid,
                                                &AmiSioProtocol);

                if (EFI_ERROR (Status) || (AmiSioProtocol == NULL)) continue;

                Status = AmiSioProtocol->CurrentRes(AmiSioProtocol, 
                                                    FALSE, 
                                                    &ResourcesList);
                if (EFI_ERROR(Status)) continue;
                
                if(ResourcesList){
                    for(i=0; i<ResourcesList->ItemCount; i++){
                        Header=(ASLRF_S_HDR*)ResourcesList->Items[i];
                        switch(Header->Name) {
                            case ASLV_RT_FixedIO: 
                                gEfiComParameters.BaseAddress=((ASLR_FixedIO*)Header)->_BAS;
                                break;
                            case ASLV_RT_IO: 
                                gEfiComParameters.BaseAddress=((ASLR_IO*)Header)->_MIN;
                                break;
                            case ASLV_RT_IRQ:
                                gEfiComParameters.SerialIRQ = (UINT8)((ASLR_IRQNoFlags*)Header)->_INT;
                                break; 
                        }
                    }
                }
                if(ValidateComPort(gEfiComParameters.BaseAddress, FALSE) == FALSE) {
                    continue;
                }
                //
                //Other settings from Termial Redirection driver
                //
                gEfiComParameters.Baudrate      = gComBaudRates[SetupData.BaudRate[ComPortNo]];
                gEfiComParameters.TerminalType  = TerminalTypes[SetupData.TerminalType[ComPortNo]];
                gEfiComParameters.FlowControl   = SetupData.FlowControl[ComPortNo];
                gEfiComParameters.LegacyOsResolution   = SetupData.LegacyOsResolution[ComPortNo];
                gEfiComParameters.RecorderMode   = SetupData.RecorderMode[ComPortNo];
                gEfiComParameters.VtUtf8   = SetupData.VtUtf8[ComPortNo];
                gEfiComParameters.PuttyKeyPad   = SetupData.PuttyFunctionKeyPad[ComPortNo];
#if  (INSTALL_LEGACY_OS_THROUGH_REMOTE == 1)
                gEfiComParameters.InstallLegacyOSthroughRemote  = SetupData.InstallLegacyOSthroughRemote[ComPortNo];
#endif
                gEfiComParameters.RedirectionAfterBiosPost = SetupData.RedirectionAfterBiosPost[ComPortNo] ;
                TempSetupValue=SetupData.Parity[ComPortNo];
                // Set parity bits.
                switch (TempSetupValue) {
                    case NoParity:
                        DataParityStopBit &= ~(PAREN | EVENPAR | STICPAR);
                    break;
                    case EvenParity:
                        DataParityStopBit |= (PAREN | EVENPAR);
                        DataParityStopBit &= ~STICPAR;
                    break;
                    case OddParity:
                        DataParityStopBit |= PAREN;
                        DataParityStopBit &= ~(EVENPAR | STICPAR);
                        break;
                    case SpaceParity:
                        DataParityStopBit |= (PAREN | EVENPAR | STICPAR);
                        break;
                    case MarkParity:
                        DataParityStopBit |= (PAREN | STICPAR);
                        DataParityStopBit &= ~EVENPAR;
                    break;
                }
    
                TempSetupValue=SetupData.StopBits[ComPortNo];

                // Set stop bits.
                switch (TempSetupValue) {
                    case OneStopBit :
                        DataParityStopBit &= ~STOPB;
                    break;
                    case OneFiveStopBits :
                    case TwoStopBits :
                        DataParityStopBit |= STOPB;
                        break;
                }

                TempSetupValue=SetupData.DataBits[ComPortNo];
   
                // Set data bits.
                DataParityStopBit &= ~SERIALDB;
                DataParityStopBit |= (UINT8)((TempSetupValue - 5) & 0x03);
                gEfiComParameters.DataParityStop= DataParityStopBit;

                IsFound = TRUE;
                IsPciDevice = FALSE;
                break;
            }
        }
    }

#else
    return EFI_NOT_FOUND;
#endif

	if (!IsFound) {
        return EFI_NOT_FOUND;
    }
	*EfiComParameters = gEfiComParameters;
	return EFI_SUCCESS;
#endif
}

//
//TERMINAL TYPE:
//              db      0               ;ANSI
//              db      1               ;VT100
//              db      2               ;VT-UTF8
//FLOWCONTROLlIST:
//              db      0               ;none
//              db      1               ;hardware Flow control
//              db      2               ;software

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
