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
// Name: TerminalSetupVar.h
//
// Description: Contains definition of PCI_SERIAL_PORTS_LOCATION_VAR 
//		and of varstore declaration SERIAL_PORTS_ENABLED_VAR.
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _TERMINAL_SETUP_VAR_H
#define _TERMINAL_SETUP_VAR_H
#ifdef __cplusplus
extern "C" {
#endif

#include <Token.h>

#define TERMINAL_VAR_GUID \
{0x560bf58a, 0x1e0d, 0x4d7e, 0x95, 0x3f, 0x29, 0x80, 0xa2, 0x61, 0xe0, 0x31}

#define SERIAL_PORTS_ENABLED_VAR_C_NAME         L"SerialPortsEnabledVar"
#define SIO_SERIAL_PORTS_LOCATION_VAR_C_NAME    L"SioSerialPortsLocationVar"
#define PCI_SERIAL_PORTS_LOCATION_VAR_C_NAME    L"PciSerialPortsLocationVar"
#define DEBUGGER_SERIAL_PORTS_ENABLED_VAR_C_NAME         L"DebuggerSerialPortsEnabledVar"

#define DEBUGGER_TERMINAL_VAR_GUID \
{0x97ca1a5b, 0xb760, 0x4d1f, 0xa5, 0x4b, 0xd1, 0x90, 0x92, 0x3, 0x2c, 0x90}

#pragma pack(1)

#ifndef TYPEDEF_DEBUGGER_SERIAL_PORTS_ENABLED_VAR
#define TYPEDEF_DEBUGGER_SERIAL_PORTS_ENABLED_VAR
    typedef struct { 
        UINT8 PortEnabled;
    } DEBUGGER_SERIAL_PORTS_ENABLED_VAR;
#endif

#if (TOTAL_SERIAL_PORTS > 0)
#ifndef TYPEDEF_SERIAL_PORTS_ENABLED_VAR
#define TYPEDEF_SERIAL_PORTS_ENABLED_VAR
    typedef struct { 
        UINT8 PortsEnabled[ TOTAL_SERIAL_PORTS ];
    } SERIAL_PORTS_ENABLED_VAR;
#endif
#endif

#if (TOTAL_SIO_SERIAL_PORTS > 0)
#ifndef TYPEDEF_SIO_SERIAL_PORTS_LOCATION_VAR
#define TYPEDEF_SIO_SERIAL_PORTS_LOCATION_VAR
    typedef struct { 
        UINT8 PortUid[ TOTAL_SIO_SERIAL_PORTS ];
        UINT8 Valid[ TOTAL_SIO_SERIAL_PORTS ];
    } SIO_SERIAL_PORTS_LOCATION_VAR;
#endif
#endif

#if (TOTAL_PCI_SERIAL_PORTS > 0)
#ifndef TYPEDEF_PCI_SERIAL_PORTS_LOCATION_VAR
#define TYPEDEF_PCI_SERIAL_PORTS_LOCATION_VAR
    typedef struct { 
        UINT8 Segment[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 Bus[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 Device[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 Function[ TOTAL_PCI_SERIAL_PORTS ]; 
        UINT8 AmiPciSerialPresent[ TOTAL_PCI_SERIAL_PORTS ];
        UINT8 Port[ TOTAL_PCI_SERIAL_PORTS ]; 
    } PCI_SERIAL_PORTS_LOCATION_VAR;
#endif
#endif

#pragma pack()

#ifdef __cplusplus
}
#endif
#endif

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
