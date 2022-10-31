#include <Token.h> //EIP144758
#include <Base.h>
#include <Protocol/AmiSio.h>
#include <Library/PlatformHookLib.h>
#include <Library/AmiSioPeiLib.h>
#include <Library\PciCf8Lib.h>

//#include <Library\SioLibExt.h>
//EIP144758 >>
IO_DECODE_DATA SioSerialPortDebugDecodeTable[]={
    #include "build\PrivateSioPeiInitTable.h"
};

SIO_DEVICE_INIT_DATA SioSerialPortDebugInitTable[]={
    #include "build\PrivateSioPeiInitTable.h"
};
//EIP144758 <<

/**
  Performs platform specific initialization required for the CPU to access
  the hardware associated with a SerialPortLib instance.  This function does
  not intiailzie the serial port hardware itself.  Instead, it initializes
  hardware devices that are required for the CPU to access the serial port
  hardware.  This function may be called more than once.

  @retval RETURN_SUCCESS       The platform specific initialization succeeded.
  @retval RETURN_DEVICE_ERROR  The platform specific initialization could not be completed.

**/
RETURN_STATUS
EFIAPI
PlatformHookSerialPortInitialize (
  VOID
  )
{
	//1. Decode
	UINT8 index;

	for(index=0; index<sizeof(SioSerialPortDebugDecodeTable)/sizeof(IO_DECODE_DATA); index++)
		AmiSioLibSetLpcDeviceDecoding(NULL, \
									SioSerialPortDebugDecodeTable[index].BaseAdd, \
									SioSerialPortDebugDecodeTable[index].UID, \
									SioSerialPortDebugDecodeTable[index].Type);
	//2. Enable IO
	//3. Set Serial port address
	ProgramRtRegisterTable(0x00, SioSerialPortDebugInitTable, sizeof(SioSerialPortDebugInitTable)/sizeof(SIO_DEVICE_INIT_DATA));

	return  RETURN_SUCCESS;
}

