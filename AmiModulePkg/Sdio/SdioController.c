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
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioSmmController.c 23    4/11/12 1:35a Rajeshms $
//
// $Revision: 23 $
//
// $Date: 4/11/12 1:35a $
//**********************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SdioController.c
//
// Description: Functions to detect the SDIO devie and read/write operation
//
//<AMI_FHDR_END>
//**********************************************************************

#include <Protocol\SdioInt13Protocol.h>
#include "SdioController.h"
#ifdef SDIO_PEI_RECOVERY_PRESENT
#include <PPI/Stall.h>
#endif

#ifndef SDIO_PEI_RECOVERY_PRESENT
extern      SDIO_GLOBAL_DATA *gSdioData;
extern      UINT8 SDIO_Access_Mode;
#else
extern EFI_PEI_STALL_PPI    *gStallPpi;
extern EFI_PEI_SERVICES     **gPeiServices;
extern UINT32   *gRecoveryTransferAddress;
UINT32          gRecoveryTransferAddr;
UINT8           SDIO_Access_Mode = 0;
#endif

BOOLEAN     SDIO_PIOflg=FALSE;
UINT8       *gSdioReadData = NULL;
UINT16      Clock;
UINT8       *SDDevnameForPNMZero  = "SD Memory Card - Device ";
UINT8       *MMCDevnameForPNMZero = "MMC Memory Card - Device ";

UINT8 SDIOGetFormatType(SDIO_DEVICE_INFO*, UINT8, UINT8*, UINT16*);
UINT8 SDIOSetDefaultGeometry(SDIO_DEVICE_INFO*, UINT8);
UINT8 SDIOValidatePartitionTable(UINT8*, UINT32, UINT8**);
UINT8 SDIOUpdateCHSFromBootRecord(SDIO_DEVICE_INFO*, UINT8, UINT8*);
VOID  SdioMassUpdateCylinderInfo(SDIO_DEVICE_INFO*, UINT8);

//
// SD device Block Size. 
//
#define     BLOCK_SIZE  0x200

//
// Maximum buffer allocated for the DMA transfer is 4KB.
//
#define     BLOCK_BOUNTRY   0x1000      

//
// Number of blocks ( Sectors) transferd in single DMA operation.
//
#define     MAXIMUM_NO_BLOCKS_FOR_SINGLE_DMA_TRANSFER   (BLOCK_BOUNTRY / BLOCK_SIZE) 

#ifndef SDIO_PEI_RECOVERY_PRESENT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FixedDelay
//
// Description: This routine delays for specified number of micro seconds
//
// Input:   wCount      Amount of delay (count in 15microsec)
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
FixedDelay(UINT32 dCount)
{
    UINT8   Reference = IoRead8(0x61) & 0x10;
    UINT8   Current;

    while(dCount)
    {
        Current = IoRead8(0x61) & 0x10;
        if (Reference != Current)
        {
            Reference = Current;
            dCount--;
        }
    }
    return;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CheckDevicePresence_Controller
//
// Description: Check if any device is connected to the port
//
// Input:
//  IN SATA_DEVICE_INTERFACE            *SataDevInterface,
//  IN EFI_IDE_CONTROLLER_INIT_PROTOCOL    *IdeControllerInterface,
//  IN UINT8                               Port,
//  IN UINT8                               PMPort
//
// Output:
//   EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CheckDevicePresence_Controller (
    IN SDIO_DEVICE_INFO                *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      PresenceState=0;


    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0xFF);

    PresenceState=SDIO_REG32(SdioBaseAddr,PSTATE);

    //
    // Set the Clock divisor as 128, slower clock for identification mode.
    //
    Clock = 0x4000;

    if(PresenceState & PSTATE_CardInsert) {
        SdioDevInfo->bMode=SD_Stand_CAP;
        SdioDevInfo->bState=CardStatus_idle;
        SdioDevInfo->bActive=InUSE;
        SdioDevInfo->bHostControllerVersion= (UINT8)(0xff&(SDIO_REG32(SdioBaseAddr,HCVER)));
        return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConfigureMassDevice_Controller
//
// Description: Configure the Sdio device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//
// Output:
//    EFI_STATUS
//
// Modified:
//
// Referrals: GetIdentifyData, GeneratePortReset, ExecuteNonDataCommand, InitAcousticSupport
//
// Notes:
//      1. Get the Identify data command.
//      2. From the IdeControllerInit protocol, get the DMA & PIO supported
//      3. Issue Set feature command to set PIO, DMA and multiple mode
//      4. Initialize Acoustic, SMART, Power Management features.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
ConfigureMassDevice_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    EFI_STATUS      Status;

    Status= SDCard_InitEnvironment(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    Status= SdCard_GetOCR(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetDeviceInformation
//
// Description: Configure the Sdio device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//
// Output:
//    EFI_STATUS
//
// Modified:
//
// Referrals: GetIdentifyData, GeneratePortReset, ExecuteNonDataCommand, InitAcousticSupport
//
// Notes:
//      1. Get the Identify data command.
//      2. From the IdeControllerInit protocol, get the DMA & PIO supported
//      3. Issue Set feature command to set PIO, DMA and multiple mode
//      4. Initialize Acoustic, SMART, Power Management features.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetDeviceInformation (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    EFI_STATUS                          Status;
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    UINT32                      CapValue=0;

    Status= SDCard_GetRCA(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    Status= SDCard_GetCSD(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    Status= SDCard_Select(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    if ( (SdioDevInfo->bMode == MMC_Stand_CAP) ||
            (SdioDevInfo->bMode == MMC_High_CAP) ) {
        //
        // Get extended CSD register if MMC 4.0 or higher
        //
        if (SdioDevInfo->bSpecVers >= 4) {
            Status = SDCard_GetExtCSD(SdioDevInfo, Port);
            if (EFI_ERROR(Status)) {
                return Status;
            }
            //
            // Set to High speed interface by enabling HS_TIMING in extended CSD reg.
            // if host controller supports High Speed.
            CapValue=SDIO_REG32(SdioDevInfo->SdioBaseAddress,CAP);
            if ( CapValue & High_Speed_Support ) {                                 
                CommandIndex = ((SWITCH_FUNC<< 8) | Resp1_Type);
                CommandArgument = 0 | (1 << 8) | (185 << 16) | (3 << 24);
                Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
                if(EFI_ERROR(Status)) {
                    return Status;
                }
            }
        }
    }
    
    Status= SDCard_SetClock(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Give a delay of 10Msec after setting clock for e-MMC device.
    //
    if ( (SdioDevInfo->bMode == MMC_Stand_CAP) ||
            (SdioDevInfo->bMode == MMC_High_CAP) ) {
#ifndef SDIO_PEI_RECOVERY_PRESENT    
        FixedDelay (10*1000/15);  // 1000 usec * 10 /15 = 10Msec
#else
        gStallPpi->Stall( gPeiServices, gStallPpi, 10000 );
#endif
    }
	
	//
	// Set the Default MMC Bus width as 4-bit.
	//
#ifndef SDIO_PEI_RECOVERY_PRESENT 
    if(!SdioDevInfo->MmcBusWidth) {
        SdioDevInfo->MmcBusWidth = MMC_4_BIT_BUS_WIDTH;
    }
#else
        SdioDevInfo->MmcBusWidth = MMC_1_BIT_BUS_WIDTH;
#endif

    Status= SDCard_Buswidth(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Give a delay of 10Msec after setting buswidth for e-MMC device.
    //
    if ( (SdioDevInfo->bMode == MMC_Stand_CAP) ||
            (SdioDevInfo->bMode == MMC_High_CAP) ) {
#ifndef SDIO_PEI_RECOVERY_PRESENT
        FixedDelay (10*1000/15);  // 1000 usec * 10 /15 = 10Msec
#else
        gStallPpi->Stall( gPeiServices, gStallPpi, 10000 );
#endif
    }

    SdioDevInfo->bActive=Initilized;


    Status= SDCard_DeviceIdentify(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_InitEnvironment
//
// Description: Initilize the Sd card
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_InitEnvironment (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      ClkControl=0;
    UINT32                      Temp32;

// Sey dbg
//while(1);
    
    SdioDevInfo->bActive=Initilized_failure;

    //
    //reset all
    //
    SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetAll);
    
#ifndef SDIO_PEI_RECOVERY_PRESENT    
    FixedDelay (5 * 1000/15);  // 1000 usec/15 = 5Msec
#else
    gStallPpi->Stall( gPeiServices, gStallPpi, 5000 );
#endif

    //
    //clear interrupt Status
    //
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,0xFFFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSTS,0xFFFF);


    //
    // Set INT parameter
    //
    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0xFF);

	#ifndef SDIO_PEI_RECOVERY_PRESENT    
			FixedDelay (1000/15);  // 1000 usec/15 = 1Msec
	#else
			gStallPpi->Stall( gPeiServices, gStallPpi, 1000 );
	#endif
    //
    //Set Clock register
    //set divisor as 8 first, set slower clock for get OCR
    //
    SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,Clock);
    
    SDIO_REG16_OR(SdioBaseAddr,CLKCTL,CLKCTL_InternalClockEnable);
    
#ifndef SDIO_PEI_RECOVERY_PRESENT    
    FixedDelay (5 * 1000/15);  // 1000 usec/15 = 5Msec
#else
    gStallPpi->Stall( gPeiServices, gStallPpi, 5000 );
#endif

    do {
        ClkControl=SDIO_REG16(SdioBaseAddr,CLKCTL);
    } while(!(ClkControl & CLKCTL_InternalClockStable));

    SDIO_REG16_OR(SdioBaseAddr,CLKCTL,CLKCTL_ClockEnable);
    //
    //set power based on power support in capabilities register.
    //
    Temp32=SDIO_REG32(SdioBaseAddr,CAP);
    
    if (Temp32 & Voltage_Support_18 ) {
        SDIO_REG8_OR(SdioBaseAddr,PWRCTL,PWRCTL_18V);
    } else if( Temp32 & Voltage_Support_33 ) {
        SDIO_REG8_OR(SdioBaseAddr,PWRCTL,PWRCTL_33V);
    } else {
        SDIO_REG8_OR(SdioBaseAddr,PWRCTL,PWRCTL_30V);
    }
            
    SDIO_REG8_OR(SdioBaseAddr,PWRCTL,PWRCTL_PowerEnable);
    
#ifndef SDIO_PEI_RECOVERY_PRESENT    
    FixedDelay (5 * 1000/15);  // 1000 usec/15 = 5Msec
#else
    gStallPpi->Stall( gPeiServices, gStallPpi, 5000 );
#endif
    
    do {
        ClkControl=SDIO_REG16(SdioBaseAddr,PWRCTL);
    } while(!(ClkControl & PWRCTL_PowerEnable));

    //
    //set time out control register
    //
    SDCard_SetTimeout(SdioDevInfo,Port);

    //
    //check SPI OR SD MODE
    //
    Temp32=SDIO_REG32(SdioBaseAddr,PSTATE);
    if(!(Temp32 & PSTATE_DAT3)) {
        SdioDevInfo->bMode=SD_SPI_Mode;
    }

    //
    //delay 100ms for power on stable
    //
#ifndef SDIO_PEI_RECOVERY_PRESENT
    FixedDelay (100*1000/15);  // 1000 usec * 100 = 100Msec
#else
    gStallPpi->Stall( gPeiServices, gStallPpi, 100000 );
#endif

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_SetTimeout
//
// Description: Set the SD device timeout value
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_SetTimeout (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);

    //
    // Set the Maximum Timeout Value
    //
    SDIO_WRITE_REG8(SdioBaseAddr,TOCTL,0x0E);
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_CheckIO
//
// Description: Get the SD device information. It return how many memory and 
//              IO functions are present in the device.
//
// Input:
//    IN SDIO_DEVICE_INFO                 *SdioDevInfo,
//    IN UINT8                            Port,
//    IN UINT8                            *NumberOfIo,
//    IN UINT8                            *MemoryPresent,
//    IN UINT32                           *IoOcr
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_CheckIO (
    IN SDIO_DEVICE_INFO                 *SdioDevInfo,
    IN UINT8                            Port,
    IN UINT8                            *NumberOfIo,
    IN UINT8                            *MemoryPresent,
    IN UINT32                           *IoOcr
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      ResponceData=0;
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;

    //
    //Issue CMD5
    //

    CommandIndex=(IO_SEND_OP_COND << 8) | (Resp4_Type);
    CommandArgument= 0x000000;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    ResponceData=SDIO_REG32(SdioBaseAddr,RESP0);
    *NumberOfIo= ((ResponceData >> 28) & 07);
    *MemoryPresent= ((ResponceData >> 27) & 01);
    *IoOcr=(ResponceData & 0xFFFFFF);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:	ReturnMSBset
//
// Description:	Returns the MOST significant Bit set.
//
// Input:
//	UINT32		Data
//
// Output:
//		UINT8
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 ReturnMSBset(
    UINT32 Data )
{
    UINT8 Index;
    UINT8 Value = 0; 

    for ( Index = 0; Index < 32; Index++ )
    {
        if ( Data & 1 ) {
            Value = Index;
        }
        Data >>= 1;
    }

    return Value;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_SetIOVoltage
//
// Description: Set the SD IO Device Voltage to the device 
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//    IN UINT32                           IoOcr
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_SetIOVoltage (
    IN SDIO_DEVICE_INFO                 *SdioDevInfo,
    IN UINT8                            Port,
    IN UINT32                           IoOcr
)
{

    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;


    IoOcr=ReturnMSBset(IoOcr);

    //
    //Issue CMD5 to set the Voltage Level.
    //
    CommandIndex=(IO_SEND_OP_COND << 8) | (Resp4_Type);
    CommandArgument = 1 << IoOcr;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    SdioDevInfo->bState=CardStatus_ident;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_SendCmd52
//
// Description: Set the CMD 52 and read or Write to the CIA regsiters 
//
// Input:
//    IN SDIO_DEVICE_INFO                 *SdioDevInfo,
//    IN UINT8                            Port,
//    IN UINT32                           ReadWriteStartOffset,
//    IN UINT32                           NumberOfBytes,
//    IN BOOLEAN                          ReadWrite,
//    IN OUT UINT8                        *DataBuffer
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_SendCmd52 (
    IN SDIO_DEVICE_INFO                 *SdioDevInfo,
    IN UINT8                            Port,
    IN UINT32                           ReadWriteStartOffset,
    IN UINT32                           NumberOfBytes,
    IN BOOLEAN                          ReadWrite,
    IN OUT UINT8                        *DataBuffer
)
{

    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;
    UINT32                      Index=0;
    UINT8                       StatusFlag=0;
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      ResponceData;


    for(Index=0;Index<NumberOfBytes;Index++) {

        CommandIndex=(IO_RW_DIRECT << 8) | (Resp5_Type);
        if(ReadWrite == TRUE) {
            //
            // Write Command
            //
            CommandArgument= (UINT32)((1 << 30) + ((ReadWriteStartOffset+Index) << 9) 
                                        + (0x1 << 8) + DataBuffer[Index]);    
        } else {
            //
            // Read Command
            //
            CommandArgument= (UINT32)(((ReadWriteStartOffset+Index) << 9) + (0x1 << 8) );    
        }

        //
        // Send the Command to the controller
        //
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
        if(EFI_ERROR(Status)) {
            return Status;
        }

        //
        // Get the Responce Data 
        //
        ResponceData=SDIO_REG32(SdioBaseAddr,RESP0);

        //
        // Check for the Error.
        //
        StatusFlag=(UINT8)(ResponceData >> 8);
        if(StatusFlag & (IO_OUT_OF_RANGE + IO_INVALID_FUNCTION_NO + IO_SDIO_DEVICE_ERROR +
                        IO_ILLEGAL_COMMAND + IO_COM_CRC_ERROR) ) {

            //
            // Error Found. Return with Error Status
            //
            return EFI_DEVICE_ERROR;
        }

        //
        // For Read Command, Save the data in the buffer
        //
        if(ReadWrite == FALSE) {
            DataBuffer[Index]=(UINT8)(ResponceData);
        }

    }

    return EFI_SUCCESS;
}

#if CHECK_FOR_SD_IO_DEVICE
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SdCard_CheckSdIoDevice
//
// Description: Check the Connected SD device is I/O device or not 
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                          Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdCard_CheckSdIoDevice (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                           Port
)
{

    EFI_STATUS                  Status;
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    UINT8                       MemoryPresent;
    UINT8                       NumberOfIO;
    UINT32                      IoOcr;
    UINT32                      CisAreaIndex;
    UINT8                       TupleCode=0;
    UINT8                       TupleLink=0;
    UINT8                       TupleSize=0;
    UINT8                       Cmd52Data[3];

    Status= SDCard_InitEnvironment(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    CommandIndex=GO_IDLE_STATE;
    CommandArgument=0;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Check , SD device has only IO function.
    //
    Status= SDCard_CheckIO(SdioDevInfo,Port,&NumberOfIO,&MemoryPresent,&IoOcr);

    if((EFI_ERROR(Status)) || MemoryPresent != 0 || NumberOfIO == 0) {
        return EFI_NOT_FOUND;
    }

    //
    // It's an SD I/O Device and only read the Manufature name and display it.
    //      
    SdioDevInfo->IODevice=TRUE;

    //
    // Set the New voltage
    //
    Status= SDCard_SetIOVoltage(SdioDevInfo,Port,IoOcr);

    //
    // Ask the card to publish a new relative address 
    //
    Status= SDCard_GetRCA(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Select the card
    //
    Status= SDCard_Select(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Get the Straing CIS Pointer Area
    //            
    Status=SDCard_SendCmd52(SdioDevInfo,Port,COMMON_CIS_POINTER,3, FALSE, &Cmd52Data[0]);

    if(EFI_ERROR(Status)) {
        return Status;
    }

    CisAreaIndex =(UINT32)( (UINT32)Cmd52Data[0] + (UINT32)(Cmd52Data[1] << 8) 
                            + (UINT32)(Cmd52Data[2] << 16)); 

    if(CisAreaIndex < CIS_AREA_START || CisAreaIndex > CIS_AREA_END) {
        //
        // Invalid CIS area. 
        //
        return EFI_DEVICE_ERROR;
    }

    //
    //Get the SD IO Device Manufature ID 
    //

    do {

        //
        // Get the Tuple Code
        //            
        Status=SDCard_SendCmd52(SdioDevInfo,Port,CisAreaIndex,1, FALSE, &TupleCode);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        //
        // Skip the Tuple Code 
        //
        CisAreaIndex++;

        //
        // If it's Manufacture Tuple or End of Tuple , Get out
        //
        if(TupleCode == CISTPL_MANFID || TupleCode == CISTPL_END) {
            break;
        }
        
        //
        // Read the Next Tuple Link
        //
        Status=SDCard_SendCmd52(SdioDevInfo,Port,CisAreaIndex,1, FALSE, &TupleLink);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        //
        // Try to get the Next Tuple Code.
        // Skip the TupleLink + TupleLink Size
        //
        CisAreaIndex+=TupleLink+1;

    } while(CisAreaIndex <= CIS_AREA_END);

    //
    // if Manufacture Tuple found, Read the Manufature ID
    //
    if(TupleCode == CISTPL_MANFID) {

        //
        // Get the Manufacture tuple size
        //            
        Status=SDCard_SendCmd52(SdioDevInfo,Port,CisAreaIndex,1, FALSE, &TupleSize);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        //
        // Read all the data in the Manufacture Tuple
        //
        Status=SDCard_SendCmd52(SdioDevInfo,Port,CisAreaIndex+1,TupleSize, 
                                FALSE, &SdioDevInfo->SdIOManufactureId[0]);
        return Status;
    }
        
    return EFI_NOT_FOUND;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SdCard_GetOCR
//
// Description: Get the OCR value from the Sd device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdCard_GetOCR (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                           Port
)
{

    EFI_STATUS                  Status;
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    UINT32                      CommandA41Argument=0;
    UINT8                       i,j;
    UINT32                      Temp32;
   
    //
    // As some cards needs higher clock speed in identification mode, we increase the
    // clock speed when the card fails for CMD0/CMD2, till the maximum host clock.
    //
    for( j=0 ; j < 8 ; j++ ) {

        if ( j ) {
            Clock = Clock >> 1;
            Clock &= 0xFF00;
                
            Status= SDCard_InitEnvironment(SdioDevInfo,Port);
            if(EFI_ERROR(Status)) {
                return Status;
            }
        }

        CommandIndex=GO_IDLE_STATE;
        CommandArgument=0;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
        if(EFI_ERROR(Status)) {
            if( j == 7 ) {
                return Status;
            }
            continue;
        }

        //
        // It can be SD card.
        //
        CommandIndex=(SEND_IF_COND << 8) | (Resp7_Type);
        CommandArgument=0x1aa;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
        if(!EFI_ERROR(Status)) {
            SdioDevInfo->bMode=SD_High_CAP;
        } else {
            SdioDevInfo->bMode=SD_Stand_CAP;
            SDIO_WRITE_REG16(SdioBaseAddr,ERINTSTS,0xFFFF);
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,0xFFFF);
            SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetCMD);

            CommandIndex=GO_IDLE_STATE;
            CommandArgument=0;
            Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

            if(EFI_ERROR(Status)) {
                // If SEND_IF_COND fails for more than 3 times, then it can be MMC card.
                if( j == 2 ) {
                    goto CheckMMC;
                }                
                continue;
            }
        }

        //
        //delay 50ms for reset
        //
#ifndef SDIO_PEI_RECOVERY_PRESENT
        FixedDelay (50*1000/15);  // 1000 usec * 50 /15 = 50Msec
#else
        gStallPpi->Stall( gPeiServices, gStallPpi, 50000 );
#endif


        //
        //repeat issue cmd55 and acmd41, until  OCR ready
        //

        for(i=0;i<=50;i++) {
            //
            // Issue CMD 55. CMD55 will fail for MMC card until the card
            // is in stby state(i.e after CMD3(SEND_RELATIVE_ADDR) is sent).
            // Please check the MMCA 4.1(JESD84-A441) Specification.
            //
            CommandIndex=(APP_CMD << 8) | (Resp1_Type);
            CommandArgument=0;
            Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
            if(EFI_ERROR(Status)) {
                goto CheckMMC;
            }

            //
            //Issue ACMD41
            //

            CommandIndex=(SD_SEND_OP_COND << 8) | (Resp3_Type);
            CommandArgument=0;
            Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandA41Argument);
            if(EFI_ERROR(Status)) {
                goto CheckMMC;
            }

            CommandA41Argument=SDIO_REG32(SdioBaseAddr,RESP0);
            if(CommandA41Argument & Card_PowerUpStatusBit) {
                break;
            }

            CommandA41Argument&=Card_VoltageWindow;
            if(SdioDevInfo->bMode == SD_High_CAP) {
                CommandA41Argument= (CommandA41Argument | Card_CapacityStatus);
            }

            //
            //delay 50ms
            //
#ifndef SDIO_PEI_RECOVERY_PRESENT
            FixedDelay (50*1000/15);  // 1000 usec * 50 /15
#else
            gStallPpi->Stall( gPeiServices, gStallPpi, 50000 );
#endif

        }

        if(i>20) {
            return EFI_NOT_FOUND;
        }

        if(CommandA41Argument & Card_CapacityStatus) {
            SdioDevInfo->bMode=SD_High_CAP;
        } else {
            SdioDevInfo->bMode=SD_Stand_CAP;
        }

        SdioDevInfo->dOCR=CommandA41Argument;
        SdioDevInfo->bState=CardStatus_ready;

        Status= SDCard_GetCID(SdioDevInfo,Port);
        if(EFI_ERROR(Status)) {
            if( j == 7 ) {
                return Status;
            }
            continue;
        }
        
        return EFI_SUCCESS;
    }

CheckMMC:
    //
    // As some controller/cards needs higher clock speed in identification mode,
    // we increase the clock speed when the card fails for CMD's in identification mode.
    // Some eMMC still need low clock in identification mode, reduce more if SD 3.0 controller
    if(SdioDevInfo->bHostControllerVersion >= HOST_CONTROLLER_Ver3) {
        Clock = 0x40;
    } else {
        Clock = 0x8000;
    }
    for ( j=0 ; j < 8; j++ ) {

        if ( j ) {
          Clock = Clock >> 1;
          Clock &= 0xFF00;
          if(j==1) {
            if(SdioDevInfo->bHostControllerVersion >= HOST_CONTROLLER_Ver3) {
                Clock = 0x8000;
            }
          }
        }
        
        Status= SDCard_InitEnvironment(SdioDevInfo,Port);
        if(EFI_ERROR(Status)) {
            if( j == 7 ) {
                return Status;
            }
            continue;            
        }
	
        CommandIndex=GO_IDLE_STATE;
        CommandArgument=0;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
        if(EFI_ERROR(Status)) {
            if( j == 7 ) {
                return Status;
            }
            continue;            
        }

        //
        // Check if it is MMC card by issuing cmd1. For e-MMC device the access mode
        // support is also sent as command argument.
        //
        CommandIndex = (SEND_OP_COND << 8) | (Resp3_Type);
        CommandArgument = (Mmc_card_capacity_bit | Card_VoltageWindow);

        for(i=0; i < 50; i++) {
            Status = SDCard_CommandCMD(SdioDevInfo, Port, CommandIndex, CommandArgument);
            Temp32 = SDIO_REG32(SdioBaseAddr,RESP0);
            if(!EFI_ERROR(Status) && (Temp32 & Card_PowerUpStatusBit)) {
                break;
            }
        }

        if(i > 50) {
            return EFI_NOT_FOUND;
        }

        SdioDevInfo->dOCR = Temp32;
        if((Temp32 & Mmc_card_capacity_bit )) { 
            SdioDevInfo->bMode = MMC_High_CAP;
        } else {
            SdioDevInfo->bMode = MMC_Stand_CAP;
        }
        SdioDevInfo->bState=CardStatus_ready;

        Status= SDCard_GetCID(SdioDevInfo,Port);
        if(EFI_ERROR(Status)) {
            if( j == 7 ) {
                return Status;
            }
            continue;            
        }
        break;
    } 

    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_CommandCMD
//
// Description: Send the Command to Sd device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//    IN UINT16                               CommandIndex,
//    IN UINT32                               CommandArgument
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_CommandCMD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT16                               CommandIndex,
    IN UINT32                               CommandArgument
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    EFI_STATUS                  Status;

    //
    // Removed the resetting the CMD line code as some controller fail if it is resetted.
    //

    SDIO_WRITE_REG32(SdioBaseAddr,CMDARG,CommandArgument);

    SDIO_WRITE_REG16(SdioBaseAddr,CMD,CommandIndex);

    Status=SDCard_WaitCMDComplete(SdioDevInfo,Port);

    return Status;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_WaitCMDComplete
//
// Description: Waits Until command complete.
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_WaitCMDComplete (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      Temp16;
    UINT16                      TempValue;
    UINT8                       i;


    for(i=0;i<=32;i++) {

        Temp16=SDIO_REG16(SdioBaseAddr,NINTSTS);
        if(Temp16 & NINTSTS_EI) {
            return EFI_DEVICE_ERROR;
        }
        if(Temp16 & NINTSTS_CC) {
            break;
        }
        //
        //delay 1ms delay
        //
#ifndef SDIO_PEI_RECOVERY_PRESENT
        FixedDelay (1*1000/15);
#else
        gStallPpi->Stall( gPeiServices, gStallPpi, 1000 );
#endif
    }

    if(i>32) {
        return EFI_DEVICE_ERROR;
    }

    Temp16=SDIO_REG16(SdioBaseAddr,CMD);
    TempValue=Temp16;
    Temp16=(Temp16 >> 8);

    if(Temp16 == READ_MULTIPLE_BLOCK || Temp16 == WRITE_MULTIPLE_BLOCK) {
        SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_CC);
    } else {
        TempValue &= (BIT0+BIT1);
        if(TempValue == (BIT0+BIT1)) {
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,(NINTSTS_TC | NINTSTS_CC));
        } else {
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_CC);
        }

    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_SetClock
//
// Description: Set the Clock value for the Sd device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_SetClock (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      CapValue=0;
    UINT32                      TempValue=0;
    UINT16                      Temp16;
    UINT32                      MaxCardClk=0;
    UINT32                      MaxHstClk=0;
    UINT8                       FreqUnitIndex;
    UINT8                       MultFactIndex;
    UINT16                      FreqUnit[] = {0, 10, 100, 1000};
    UINT8                       MultFactor[] = {10, 12, 13, 15, 20, 26, 30, \
                                                35, 40, 45, 52, 55, 60, 70, 80};
    UINT16                      ClockDivident=0;
    UINT8                       UpperDiv;
    BOOLEAN                     HighSpeedSupport=FALSE;
    BOOLEAN                     SetHighSpeedSupport=FALSE;
    //
    //disable clock first
    //
    SDIO_REG16_AND(SdioBaseAddr,CLKCTL,(~CLKCTL_ClockEnable));
    Temp16=SDIO_REG16(SdioBaseAddr,CLKCTL);
    Temp16&=0xFF;
    SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,Temp16);

    CapValue=SDIO_REG32(SdioBaseAddr,CAP);
    if ( CapValue & High_Speed_Support ) {
        HighSpeedSupport = TRUE;
    }
    TempValue=CapValue;
    if(SdioDevInfo->bHostControllerVersion >= HOST_CONTROLLER_Ver3) {
        CapValue &= Base_Clock_Frequency_For_SD_ClockV3;
    } else {
        CapValue &= Base_Clock_Frequency_For_SD_Clock;
    }

    CapValue = (CapValue >> 8);

    if(CapValue) {
        MaxHstClk = CapValue * 1000000;
    }

    //
    // Calculate the maximum card frequency in the CSD register
    //
    FreqUnitIndex = SdioDevInfo->d4CSD[3] & 7;
    MultFactIndex = (SdioDevInfo->d4CSD[3] >> 3) & 0xf;
    MaxCardClk = 10000 * FreqUnit[FreqUnitIndex] * MultFactor[MultFactIndex-1];

    if( (SdioDevInfo->bMode == MMC_Stand_CAP) || \
      (SdioDevInfo->bMode == MMC_High_CAP) ) {
        // Check Controller if High Speed supported before checking CARD_TYPE.
        if( HighSpeedSupport ) {
            // If BIT1 is clock can be set to 52MHz.
            if( SdioDevInfo->bMmcCardType & BIT1 ) {
                if( MaxHstClk <= 52000000) {
                    SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x000);                    
                } else {                
                    if( ( TempValue = (MaxHstClk / 2) ) <= 52000000) {
                        SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x100);
                    } else if( ( TempValue = (MaxHstClk / 4 )) <= 52000000) {
                        SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x200);
                    } else {
                        SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x400);
                    }
                }
                SetHighSpeedSupport = TRUE;
            } else {
                goto SetNormalClock;
            }
        } else {
SetNormalClock:
            if( MaxHstClk <= MaxCardClk ) {
                SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x000);
                if(MaxHstClk >= 25000000) {
                    SetHighSpeedSupport = TRUE;
                }
            } else {
                if( ( TempValue = (MaxHstClk / 2) ) < MaxCardClk) {
                    SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x100);
                } else if( ( TempValue = (MaxHstClk / 4 )) < MaxCardClk) {
                    SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x200);
                } else {
                    SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x400);
                }
                
                if(TempValue >= 25000000) {
                    SetHighSpeedSupport = TRUE;
                }
            }    
        }

    } else {
        //
        // SD Card.
        //
        if((SdioDevInfo->bHostControllerVersion >= HOST_CONTROLLER_Ver3) ) {
            if(MaxHstClk%(2*MaxCardClk)) {
                ClockDivident =(MaxHstClk/(2*MaxCardClk))+1;
            }else{
                ClockDivident =MaxHstClk/(2*MaxCardClk);
            }
            
            if ( (MaxHstClk/(2*ClockDivident)) >= 25000000 ) {
                SetHighSpeedSupport = TRUE;
            }
            
            if(ClockDivident<=0x3FF && ClockDivident> 0xFF) {
                UpperDiv = ((ClockDivident>>2)&0xc0); 
                SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,UpperDiv);
                ClockDivident &= 0xff;
                ClockDivident = ClockDivident<<8;          
                SDIO_REG16_OR(SdioBaseAddr,CLKCTL,ClockDivident);
            }else if (  ClockDivident<0xFF ){
                ClockDivident = ClockDivident<<8;          
                SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,ClockDivident);
            }

        } else {
            if( ( TempValue = (MaxHstClk / 2) ) < MaxCardClk) {
                SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x100);
            } else if( ( TempValue = (MaxHstClk / 4 )) < MaxCardClk) {
                SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x200);
            } else {
                SDIO_WRITE_REG16(SdioBaseAddr,CLKCTL,0x400);
            }
            
            if(TempValue >= 25000000) {
                SetHighSpeedSupport = TRUE;
            }
        }
    }

    if ( HighSpeedSupport && SetHighSpeedSupport==TRUE) {
        SDIO_REG8_OR(SdioBaseAddr,HOSTCTL,High_Speed_Enable);
    }
    
    SDIO_REG16_OR(SdioBaseAddr,CLKCTL,CLKCTL_InternalClockEnable);
    
    do {
        Temp16=SDIO_REG16(SdioBaseAddr,CLKCTL);
    } while(!(Temp16 & CLKCTL_InternalClockStable));
    
    SDIO_REG16_OR(SdioBaseAddr,CLKCTL,CLKCTL_ClockEnable);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_GetCID
//
// Description: Get the Sd card CID
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_GetCID (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;
    UINT8                       i;
    UINT8                       Temp8;

    if(SdioDevInfo->bState != CardStatus_ready) {
        return EFI_DEVICE_ERROR;
    }

    //
    //Issue CMD2
    //

    CommandIndex=((ALL_SEND_CID << 8) | Resp2_Type);
    CommandArgument=0;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    SdioDevInfo->d4CID[0]=SDIO_REG32(SdioBaseAddr,RESP0);
    SdioDevInfo->d4CID[1]=SDIO_REG32(SdioBaseAddr,RESP1);
    SdioDevInfo->d4CID[2]=SDIO_REG32(SdioBaseAddr,RESP2);
    SdioDevInfo->d4CID[3]=SDIO_REG32(SdioBaseAddr,RESP3);

    //
    // We get bits 0-119 in response, shift left by one byte.
    //
    for(i=3; i>0; i--) {
        SdioDevInfo->d4CID[i] = (SdioDevInfo->d4CID[i] << 8);
        Temp8 = (SdioDevInfo->d4CID[i-1] >> 24) & 0xff;
        SdioDevInfo->d4CID[i] += Temp8;
    }

    SdioDevInfo->d4CID[i] = (SdioDevInfo->d4CID[i] << 8);

    SdioDevInfo->bState=CardStatus_ident;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_GetRCA
//
// Description: Get the RCA value from the Sd device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_GetRCA (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      Temp32;
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;

    if((SdioDevInfo->bState != CardStatus_ident) &&
        (SdioDevInfo->bState != CardStatus_stby)) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Issue CMD3 for all MMC devices
    //
    if ( (SdioDevInfo->bMode == MMC_Stand_CAP) || \
          (SdioDevInfo->bMode == MMC_High_CAP)  ) {
        CommandIndex = ((SEND_RELATIVE_ADDR << 8) | Resp1_Type);
        CommandArgument = 1 << 16;
        Status = SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
        if (EFI_ERROR(Status)) {
            return Status;
        }
        Temp32 = CommandArgument >> 16;
    } else {
        //
        // Issue CMD3 for all SD devices
        //  
        CommandIndex=((SEND_RELATIVE_ADDR << 8) | Resp6_Type);
        CommandArgument=0;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

        if(EFI_ERROR(Status)) {
            return Status;
        }

    Temp32 = SDIO_REG32(SdioBaseAddr,RESP0);
    Temp32 = (Temp32 >> 16);
    }

    SdioDevInfo->wRCA=(UINT16)Temp32;
    SdioDevInfo->bState=CardStatus_stby;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_GetCSD
//
// Description: Get the CSD from Sd device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_GetCSD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      Temp32;
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;
    UINT8                       Temp8;
    UINT8                       i;


    if(SdioDevInfo->bState != CardStatus_stby) {
        return EFI_DEVICE_ERROR;
    }


    //
    //Issue CMD9
    //

    CommandIndex=((SEND_CSD << 8) | Resp2_Type);
    CommandArgument=(UINT32)SdioDevInfo->wRCA;
    CommandArgument=CommandArgument << 16;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

    if(EFI_ERROR(Status)) {
        return Status;
    }

    SdioDevInfo->d4CSD[0] = SDIO_REG32(SdioBaseAddr, RESP0);
    SdioDevInfo->d4CSD[1] = SDIO_REG32(SdioBaseAddr, RESP1);
    SdioDevInfo->d4CSD[2] = SDIO_REG32(SdioBaseAddr, RESP2);
    SdioDevInfo->d4CSD[3] = SDIO_REG32(SdioBaseAddr, RESP3);
    //
    // We get bits 0-119 in response, shift left by one byte.
    //
    for(i=3; i>0; i--) {
        SdioDevInfo->d4CSD[i] = (SdioDevInfo->d4CSD[i] << 8);
        Temp8 = (SdioDevInfo->d4CSD[i-1] >> 24) & 0xff;
        SdioDevInfo->d4CSD[i] += Temp8;
    }

    SdioDevInfo->d4CSD[i] = (SdioDevInfo->d4CSD[i] << 8);

    if(SdioDevInfo->d4CSD[0] & partial_blocks_for_write_allowed) {
        SdioDevInfo->bWrite_Bl_Partial=TRUE;
    } else {
        SdioDevInfo->bWrite_Bl_Partial=FALSE;
    }

    Temp32= SdioDevInfo->d4CSD[0] & max_write_block_length;
    Temp32=Temp32 >> 22;
    SdioDevInfo->bWrite_Bl_Len=(UINT8)Temp32;

    if(SdioDevInfo->d4CSD[2] & partial_blocks_for_read_allowed) {
        SdioDevInfo->bRead_Bl_Partial=TRUE;
    } else {
        SdioDevInfo->bRead_Bl_Partial=FALSE;
    }

    Temp32= SdioDevInfo->d4CSD[2] & max_read_block_length;
    Temp32=Temp32 >> 16;
    SdioDevInfo->bRead_Bl_Len=(UINT8)Temp32;

    Temp8 = (SdioDevInfo->d4CSD[3] >> 24) & 0xff;
    SdioDevInfo->bCsdStruct = (Temp8 & 0xc0) >> 6;
    SdioDevInfo->bSpecVers = (Temp8 & 0x3c) >> 2;
    SdioDevInfo->bTranSpeed = SdioDevInfo->d4CSD[3] & 0xff;
    
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_GetExtCSD
//
// Description: Get Extended CSD from MMC device if version is 4.0 and higher
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_GetExtCSD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    EFI_STATUS  Status;
    UINT32      *TransferAddress = 0;
    UINT32      NumBlks=0;

#ifndef SDIO_PEI_RECOVERY_PRESENT 
    //
    //  Get the TransferBufferAddress
    //
    TransferAddress = (UINT32*)(gSdioData->TransferBufferAddress);
#else
    gRecoveryTransferAddr = (UINT32)gRecoveryTransferAddress;
    if((gRecoveryTransferAddr & 0x0FFF) != 0) {
        //
        // Make as 4Kb alligned address.
        //      
        gRecoveryTransferAddr = gRecoveryTransferAddr + (0x1000 - (gRecoveryTransferAddr & 0x00FFF));
    }
    TransferAddress = (UINT32 *)gRecoveryTransferAddr;
#endif

    Status = SDCard_ReadWriteData_Controller (SdioDevInfo,Port,SEND_EXT_CSD,0,TransferAddress,0,512,Resp1_Type,FALSE,TRUE);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    // Get the card type and Sector count from EXT_CSD regiser
    //
    SdioDevInfo->bMmcCardType = *(TransferAddress + 49) & 0xf;  // CARD_TYPE [196]
    SdioDevInfo->bMmcSecCount = *(TransferAddress + 53);        // SEC_COUNT [215:212]

    if( SdioDevInfo->bCsdStruct == 3 ) {
        SdioDevInfo->bCsdStruct = (*(TransferAddress + 48) & 0xff0000 ) >> 16 ; // CSD_STRUCTURE [194]
    }

    return Status;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_ReadWriteData_Controller
//
// Description: This function read/write from/to the card. 
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_ReadWriteData_Controller (
    IN SDIO_DEVICE_INFO *SdioDevInfo,
    IN UINT8            Port,
    IN UINT8            Command,
    IN UINT16           CommandArgument,
    IN UINT32           *TransferAddress,
    IN UINT32           NumBlks,
    IN UINT32           BlockSize,
    IN UINT16           ResponseType,
    IN BOOLEAN          AppCmd, 
    IN BOOLEAN          Read
)
{

    UINT32      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    EFI_STATUS  Status;
    UINT16      CommandIndex;
    UINT32      CommandArgument1;

    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0x1FF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0x1FF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0);

    Status= SDCard_GetStatus(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    if(SdioDevInfo->bState != CardStatus_tran) {
        return EFI_DEVICE_ERROR;
    }
    //
    // Issue CMD55 if ACMD need to be sent.
    //
    if(AppCmd) {
        CommandIndex=((APP_CMD << 8) | Resp1_Type);
        CommandArgument1=(UINT32)SdioDevInfo->wRCA;
        CommandArgument1=CommandArgument1 << 16;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument1);

        if(EFI_ERROR(Status)) {
            return Status;
        }
    }

    SDIO_WRITE_REG16(SdioBaseAddr,BLKSZ,BlockSize);
    SDIO_WRITE_REG16(SdioBaseAddr,BLKCNT,NumBlks);
    if(Read) {
        SDIO_WRITE_REG16(SdioBaseAddr,XFRMODE,Data_Transfer_Card2Host);
    } else {
        SDIO_WRITE_REG16(SdioBaseAddr,XFRMODE,0);
    }
    SDIO_WRITE_REG32(SdioBaseAddr,CMDARG,CommandArgument);
    SDIO_WRITE_REG16(SdioBaseAddr,CMD,((Command << 8) | ResponseType+Data_Present_Select));

    Status= SDCard_WaitCMDComplete(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto Error;
    }

    Status= SDCard_WaitXferComplete(SdioDevInfo,Port,&TransferAddress,TransferAddress,&NumBlks,Read);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto Error;
    }

    Status = SDCard_RestoreStatus(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
    }

Error:

    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0xFF);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_Select
//
// Description: Select the Device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_Select (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;

    if((SdioDevInfo->bState != CardStatus_stby) &&
        (SdioDevInfo->bState != CardStatus_tran) &&
        (SdioDevInfo->bState != CardStatus_data) &&
        (SdioDevInfo->bState != CardStatus_prg) &&
        (SdioDevInfo->bState != CardStatus_dis)
        ) {
        return EFI_DEVICE_ERROR;
    }

    //
    //Issue CMD7
    //

    CommandIndex=((SELECT_DESELECT_CARD << 8) | Resp1b_Type);
    CommandArgument=(UINT32)SdioDevInfo->wRCA;
    CommandArgument=CommandArgument << 16;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

    if(EFI_ERROR(Status)) {
        return Status;
    }

    if(SdioDevInfo->IODevice == TRUE) {

        //
        //CMD 13 is not supported by IO device.
        //
        return EFI_SUCCESS;
    }

    Status=SDCard_GetStatus(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        return Status;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_GetStatus
//
// Description: Get the Sd device current Status
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_GetStatus (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{


    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      Temp32;
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;

    //
    //if auto cmd12 enable bit set, will return R1b response REP[96:127]
    //To prevent confusion, clear this bit before issue CMD13
    //
    SDIO_WRITE_REG16(SdioBaseAddr,XFRMODE,0);


    //
    //Issue CMD13
    //

    CommandIndex=((SEND_STATUS << 8) | Resp1_Type);
    CommandArgument=(UINT32)SdioDevInfo->wRCA;
    CommandArgument=CommandArgument << 16;
    Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

    if(EFI_ERROR(Status)) {
        return Status;
    }


    Temp32=SDIO_REG32(SdioBaseAddr,RESP0);
    Temp32 &=CURRENT_STATE;
    Temp32 = (Temp32 >> 9);

    SdioDevInfo->bState=(UINT8)Temp32;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_Buswidth
//
// Description: Set the Sd device Bus width
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_Buswidth (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;

    if( (SdioDevInfo->bMode == MMC_Stand_CAP) || \
          (SdioDevInfo->bMode == MMC_High_CAP) ){
        //
        // Currently only 4-bit bus width is set for the MMC/eMMC device.
        //
        if (SdioDevInfo->bSpecVers < 4) {
            //
            // Old MMC Card's does not support Bus Width command.
            //
            return EFI_SUCCESS;
        }

        CommandIndex = ((SWITCH_FUNC<< 8) | Resp1_Type);
        if(SdioDevInfo->MmcBusWidth == MMC_1_BIT_BUS_WIDTH) {
            CommandArgument = 0 | (0 << 8) | (183 << 16) | (3 << 24);   // 1-bit bus width
        } else if(SdioDevInfo->MmcBusWidth == MMC_4_BIT_BUS_WIDTH) {
            CommandArgument = 0 | (1 << 8) | (183 << 16) | (3 << 24);   // 4-bit bus width  
        } else {
            return EFI_SUCCESS;
        }

        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
        if(EFI_ERROR(Status)) {
            return Status;
        }

        if(SdioDevInfo->MmcBusWidth == MMC_4_BIT_BUS_WIDTH) {
            SDIO_REG8_OR(SdioBaseAddr,HOSTCTL,DATA_Transfer_4BitMode);
        }

        return EFI_SUCCESS;
    }

    Status= SDCard_GetSCR(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Check the Buswidth. Bit 48-51 gives the bus width in SCR field.
    //
    if(SdioDevInfo->d2SCR[6] & BIT2) {

        //
        //Issue CMD55
        //

        CommandIndex=((APP_CMD << 8) | Resp1_Type);
        CommandArgument=(UINT32)SdioDevInfo->wRCA;
        CommandArgument=CommandArgument << 16;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        //
        //Issue ACMD56
        //

        CommandIndex=((SET_BUS_WIDTH << 8) | Resp1_Type);
        CommandArgument=2;
        Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        SDIO_REG8_OR(SdioBaseAddr,HOSTCTL,DATA_Transfer_4BitMode);

    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_GetSCR
//
// Description: Get Sd card SCR value
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_GetSCR (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    EFI_STATUS                  Status;
    UINT32                      *TransferAddress=0;
    UINT8                       *OrgTransferAddress=0;
    UINT32                      NumBlks=0;
    UINT8                       i;

#ifndef SDIO_PEI_RECOVERY_PRESENT
    //
    //  Get the TransferBufferAddress
    //
    TransferAddress = (UINT32*)(gSdioData->TransferBufferAddress);
#else
    gRecoveryTransferAddr = (UINT32)gRecoveryTransferAddress;
    if((gRecoveryTransferAddr & 0x0FFF) != 0) {
        //
        // Make is 4Kb alligned address.
        //      
        gRecoveryTransferAddr = gRecoveryTransferAddr + (0x1000 - (gRecoveryTransferAddr & 0x00FFF));
    }
    TransferAddress = (UINT32 *)gRecoveryTransferAddr;
#endif

    OrgTransferAddress =(UINT8*)TransferAddress;


    Status = SDCard_ReadWriteData_Controller (SdioDevInfo,Port,SEND_SCR,0,TransferAddress,0,8,Resp1_Type,TRUE,TRUE);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // The output data format: MSB comes first for the ACMD51. So reverse the 
    // Data to get the proper SCR fileds.
    //
    for(i=0;i<8;i++) {
        SdioDevInfo->d2SCR[i]=OrgTransferAddress[7-i];
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_WaitXferComplete
//
// Description: Wait until Transfer Complete and gets the data from SD device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//    IN UINT32                               *BufferAddress,
//    IN UINT32                               *DmaAddress
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_WaitXferComplete (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT32                               **BufferAddress,
    IN UINT32                               *DmaAddress,
    IN UINT32                               *NumBlks,
    IN BOOLEAN                              SdioRead
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    EFI_STATUS                  Status;
    UINT16                      Temp16;
    UINT8                       i;


    Status=EFI_DEVICE_ERROR;
    for(i=0;i<=5000;i++) {

        Temp16=SDIO_REG16(SdioBaseAddr,NINTSTS);
        if(Temp16 & NINTSTS_DMAINT) {
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_DMAINT);
            SDCard_DMAINTHandle(SdioDevInfo,Port,BufferAddress,DmaAddress,NumBlks,SdioRead);
        } else if(Temp16 & NINTSTS_BRR) {
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_BRR);
            SDCard_BRINTHandle(SdioDevInfo,Port,BufferAddress);
        } else if(Temp16 & NINTSTS_BWR) {
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_BWR);
            SDCard_BRINTHandle(SdioDevInfo,Port,BufferAddress);
        } else if(Temp16 & NINTSTS_TC) {
            Status=EFI_SUCCESS;
            break;
        } else if(Temp16 & NINTSTS_EI) {
            Status=EFI_DEVICE_ERROR;
            break;
        }

        //
        //delay 1ms
        //
#ifndef SDIO_PEI_RECOVERY_PRESENT
        FixedDelay (1*1000/15);
#else
        gStallPpi->Stall( gPeiServices, gStallPpi, 1000 );
#endif
    }

    if(Status==EFI_SUCCESS || (Temp16 & NINTSTS_TC)) {
        SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_TC+NINTSTS_DMAINT+ \
                                NINTSTS_BWR+NINTSTS_BRR);
        return EFI_SUCCESS;
    }

    return EFI_DEVICE_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_DMAINTHandle
//
// Description: DMA interrupt handler
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_DMAINTHandle (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT32                               **BufferAddress,
    IN UINT32                               *DmaAddress,
    IN UINT32                               *NumBlks,
    IN BOOLEAN                              SdioRead
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      Temp16;
    UINT32                      BlocksToBeTransferd=0;

    Temp16=SDIO_REG16(SdioBaseAddr,XFRMODE);

    if((Temp16 & Data_Transfer_Card2Host) || SdioRead) {
        //
        // DMA Read 
        //

        //
        // In Single DMA transfer right now we support maximum of 4KB 
        //            
        if((*NumBlks) > MAXIMUM_NO_BLOCKS_FOR_SINGLE_DMA_TRANSFER) {
            //
            // Total Sector is more than 4Kb, transfer 4KB
            //    
            BlocksToBeTransferd=MAXIMUM_NO_BLOCKS_FOR_SINGLE_DMA_TRANSFER;
        } else {
            //
            // Total sector is less than 4Kb, transfer only the total Sector value.
            //
            BlocksToBeTransferd=*NumBlks;
        }
        
        //
        // Decrease the Total number sector based on the Sectors to be transfered.
        //
        *NumBlks=(*NumBlks) - BlocksToBeTransferd;

        BlocksToBeTransferd=BlocksToBeTransferd * BLOCK_SIZE;

        MemCpy((VOID*)*BufferAddress,(VOID*)DmaAddress, (UINTN)BlocksToBeTransferd);
        *BufferAddress=*BufferAddress+(BlocksToBeTransferd/4);

    } else {
        //
        // DMA Write
        //

        SDCard_XferWdata2Buff(SdioDevInfo,Port,BufferAddress,DmaAddress,NumBlks);
    }

    SDIO_WRITE_REG32(SdioBaseAddr,DMAADR,DmaAddress);

    SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,NINTSTS_DMAINT);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_XferWdata2Buff
//
// Description: DMA interrupt handler
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_XferWdata2Buff (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT32                               **BufferAddress,
    IN UINT32                               *DmaAddress,
    IN UINT32                               *NumBlks
)
{

    UINT32                      BlocksToBeTransferd=0;

    if((*NumBlks) > MAXIMUM_NO_BLOCKS_FOR_SINGLE_DMA_TRANSFER) {
        BlocksToBeTransferd=MAXIMUM_NO_BLOCKS_FOR_SINGLE_DMA_TRANSFER;
    } else {
        BlocksToBeTransferd=*NumBlks;
    }
    *NumBlks=(*NumBlks) - BlocksToBeTransferd;

    BlocksToBeTransferd=BlocksToBeTransferd * BLOCK_SIZE;

    MemCpy((VOID*)DmaAddress,(VOID*)*BufferAddress, (UINTN)BlocksToBeTransferd);
    *BufferAddress=*BufferAddress+(BlocksToBeTransferd/4);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_XferRemainderSector_Controller
//
// Description: Transfer the Remaining Sector.
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_XferRemainderSector_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT32                               *BufferAddress,
    IN UINT32                               *DmaAddress,
    IN UINT32                               NumBlks
)
{

    UINT32                      TotalSec=BLOCK_BOUNTRY / BLOCK_SIZE;
    UINT32                      Temp32;


    Temp32=NumBlks & (~TotalSec);

    Temp32=Temp32 * BLOCK_SIZE;

    //
    // Read the reamaining data.
    //
    MemCpy((VOID*)BufferAddress,(VOID*)DmaAddress,(UINTN)Temp32);


    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_BRINTHandle
//
// Description: BR interrupt Handler
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_BRINTHandle (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT32                               **TransferAddress
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT16                      BlockSize;
    UINT16                      Temp16;

    BlockSize=SDIO_REG16(SdioBaseAddr,BLKSZ);
    BlockSize=(BlockSize & BLKSize_Size) >> 2;

    Temp16=SDIO_REG16(SdioBaseAddr,XFRMODE);

    if(Temp16 & Data_Transfer_Card2Host) {
        //
        // Read
        //
        do {
            (**TransferAddress)=SDIO_REG32(SdioBaseAddr,BUFDATA);
            (*TransferAddress)++;
            BlockSize--;
        }while(BlockSize);

    } else {
        //
        // Write
        //
        do {
            SDIO_WRITE_REG32(SdioBaseAddr,BUFDATA,(**TransferAddress));
            (*TransferAddress)++;
            BlockSize--;
        }while(BlockSize);
    }
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_RestoreStatus
//
// Description: Restore the SD card Status
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_RestoreStatus (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    UINT16                      CommandIndex;
    UINT32                      CommandArgument;
    EFI_STATUS                  Status;

    do {
        Status=SDCard_GetStatus(SdioDevInfo,Port);

        if(EFI_ERROR(Status)) {
            return Status;
        }

        if(SdioDevInfo->bState == CardStatus_tran) {
            return EFI_SUCCESS;
        }

        if( (SdioDevInfo->bState == CardStatus_data) ||
            (SdioDevInfo->bState == CardStatus_prg)||
            (SdioDevInfo->bState == CardStatus_rcv) ) {

            //
            //Issue CMD12
            //

            CommandIndex=((STOP_TRANSMISSION << 8) | Resp1b_Type);
            CommandArgument=0;
            Status=SDCard_CommandCMD(SdioDevInfo,Port,CommandIndex,CommandArgument);
            if(EFI_ERROR(Status)) {
                return Status;
            }
            if(SdioDevInfo->bHostControllerVersion >= HOST_CONTROLLER_Ver3) {
                //FixedDelay (1*5000/15);
#ifndef SDIO_PEI_RECOVERY_PRESENT    
        FixedDelay (1*5000/15);  // 1000 usec * 10 /15 = 10Msec
#else
        gStallPpi->Stall( gPeiServices, gStallPpi, 5000 );
#endif

            }
            continue;

        } else {
            return EFI_DEVICE_ERROR;
        }
    }while(1);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_DeviceIdentify
//
// Description: Identify the Device
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_DeviceIdentify (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    EFI_STATUS                  Status;

    //
    //Get the device capacity
    //
    Status= SDIOMassGetCapacity(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

#ifndef SDIO_PEI_RECOVERY_PRESENT
    //
    //Get the SD card device name
    //
    Status= SDIOMassGetPNMWithDeviceClass(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    // Identify and Initialize Mass storage device Information
    //
    SDIOMassIdentifyDeviceType(SdioDevInfo, Port);
#endif

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOMassGetCapacity
//
// Description: Get the Sd device capacity
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOMassGetCapacity (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{
    UINT32                      Temp32;
    UINT32                      MaxLba;
    UINT8                       BlockLength;
    UINT8                       *TempUint8=(UINT8*)&SdioDevInfo->d4CSD[0];
    UINT32                      *TempUint32;

    if((SdioDevInfo->bCsdStruct > 0) && (SdioDevInfo->bMode == SD_High_CAP)) { 
        //
        //CSD20
        //
        TempUint32=(UINT32*)&TempUint8[6];
        Temp32=*TempUint32;
        Temp32&=device_size_2;
        Temp32++;
        SdioDevInfo->dMaxLBA=Temp32 << 10;
    } else {
        TempUint32=(UINT32*)&TempUint8[8];
        Temp32=*TempUint32;
        Temp32=(Temp32 & max_read_block_length) >> 16;
        BlockLength=(UINT8)Temp32;
        //
        // MMC(following spec > 4.0) should have C_SIZE as 0xFFF and
        // dMaxLBA is calculated from SEC_COUNT register.
        //
        Temp32 = ((SdioDevInfo->d4CSD[1] & device_size_1l) >> 30) | \
                    ((SdioDevInfo->d4CSD[2] & device_size_1h) << 2);

        if( ((SdioDevInfo->bMode == MMC_Stand_CAP) || (SdioDevInfo->bMode == MMC_High_CAP))\
            && (Temp32 == 0xfff) ) {
            SdioDevInfo->dMaxLBA = SdioDevInfo->bMmcSecCount;
        } else {

            TempUint32=(UINT32*)&TempUint8[6];
            Temp32=*TempUint32;
            Temp32=(Temp32 & device_size_1) >> 14;
            Temp32++;

            TempUint32=(UINT32*)&TempUint8[4];
            MaxLba=*TempUint32;
            BlockLength=((MaxLba & device_szie_mul_1) >> 15)+BlockLength;
            BlockLength-=7;

            MaxLba=(1 << BlockLength);
            SdioDevInfo->dMaxLBA= MaxLba * Temp32;
        }
    }

    // LBA address starts from 0 so the maximum accessible 
    // address should be one less than the max.

    SdioDevInfo->dMaxLBA -= 1;

    SdioDevInfo->wBlockSize=0x200;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOMassGetPNMWithDeviceClass
//
// Description: Get the Sd/MMC card Device Name and concatenate it with the
//              string formed based on the size of SD card if PNM is not Zero.
//              
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                          Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
// Notes:
//    If PNM is zero then SD Memory Card - Device (Index) is displayed for SD.
//    MMC Memory Card - Device (Index) is displayed for MMC device. Index is 
//    varied for the no. of devices detected without PNM.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOMassGetPNMWithDeviceClass (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
)
{
    UINT8                       *TempUint8;
    UINT8                       i;
    UINT8                       j;
    UINT64                      CardSize;
    BOOLEAN                     IsPNMZero = FALSE;
    static                      UINT8 SDDevIndex='0';
    static                      UINT8 MMCDevIndex='0';
    BOOLEAN                     SDCardWithOutPNMDetected=FALSE;

    TempUint8=(UINT8*)&SdioDevInfo->d4CID[3];

    //
    // Check if PNM is Zero.
    //
    for(j=0 ; j<5 ; j++,TempUint8--) {
        if( *TempUint8 != '0' ) {
            break;
        }
    }

    if( j == 5 ) {
       IsPNMZero = TRUE;
    }  

    if ( (SdioDevInfo->bMode == MMC_Stand_CAP) || \
            (SdioDevInfo->bMode == MMC_High_CAP) ) {
        //
        // If PNM is Zero display String MMCDevnameForPNMZero
        //
        if(IsPNMZero) {
            TempUint8 = MMCDevnameForPNMZero;
        } else {
            TempUint8 = "MMC - ";
        }
    } else {
        //
        // If PNM is Zero display String SDDevnameForPNMZero
        //
        if(IsPNMZero) {
            TempUint8 = SDDevnameForPNMZero;
            SDCardWithOutPNMDetected = TRUE;
        } else {
            //
            //Calculate the SD Card Size.
            //
            CardSize = ((UINT64)(SdioDevInfo->dMaxLBA) * 512);
            //
            //Assign name to display in setup based on the Size.
            //
            if( CardSize <= TWO_GB ) {
                TempUint8 = "SDSC - ";
            } else if( CardSize <= THIRTY_TWO_GB ) {
                TempUint8 = "SDHC - ";
            } else {
                TempUint8 = "SDXC - ";
            }
        }
    }

    if(!IsPNMZero) {

        for(i=0; *TempUint8 != 0; i++,TempUint8++) {
            SdioDevInfo->PNM[i]=*TempUint8;
        }

        TempUint8=(UINT8*)&SdioDevInfo->d4CID[3];
        //
        // Concatenate product name with above assigned name.
        //
        for(j=0;j<5;j++) {
            SdioDevInfo->PNM[i+j]=*TempUint8;
            TempUint8--;
        }

        if ( (SdioDevInfo->bMode == MMC_Stand_CAP) || \
            (SdioDevInfo->bMode == MMC_High_CAP)) {
            SdioDevInfo->PNM[i+j] = *TempUint8;
            j++;
        }
        SdioDevInfo->PNM[i+j]=0;
    
    } else {

        for(i=0; *TempUint8 != 0; i++,TempUint8++) {
            SdioDevInfo->PNM[i]=*TempUint8;
        }

        //
        // Append the Index to the string to differentiate between two
        // Cards with PNM equal to Zero.
        //
        if(SDCardWithOutPNMDetected) {
            SdioDevInfo->PNM[i++] = SDDevIndex++;
        } else {
            if ( SdioDevInfo->MmcBusWidth == MMC_1_BIT_BUS_WIDTH ) {
                SdioDevInfo->PNM[i++] = MMCDevIndex - 1;
            } else {
                SdioDevInfo->PNM[i++] = MMCDevIndex++;
            } 
        }
        
        SdioDevInfo->PNM[i]=0;
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOAPI_ReadCard_Controller
//
// Description: Read the Data from Sd card
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOAPI_ReadCard_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    EFI_STATUS                  Status;
    UINT32                      *TransferAddress=0;
    UINT32                      Capability=0;
    BOOLEAN                     SdioRead=TRUE;
    SDIO_PIOflg=FALSE;

    //
    //If the SDIO access mode is Auto, based on capabilities device PIO and DMA
    //
    if(SDIO_Access_Mode == 0 ) {
        Capability=SDIO_REG32(SdioBaseAddr,CAP);
        if(Capability & SDMA_Support) {
            SDIO_PIOflg=FALSE;
        }else {
            SDIO_PIOflg=TRUE;
        }
    } else if(SDIO_Access_Mode == 1) {
        SDIO_PIOflg=FALSE;
    } else if(SDIO_Access_Mode == 2) {
        SDIO_PIOflg=TRUE;
    }

#ifndef SDIO_PEI_RECOVERY_PRESENT
    //
    //  Get the TransferBufferAddress
    //
    TransferAddress = (UINT32*)(gSdioData->TransferBufferAddress);
#else
    gRecoveryTransferAddr = (UINT32)gRecoveryTransferAddress;
    if((gRecoveryTransferAddr & 0x0FFF) != 0) {
        //
        // Make is 4Kb alligned address.
        //      
        gRecoveryTransferAddr = gRecoveryTransferAddr + (0x1000 - (gRecoveryTransferAddr & 0x00FFF));
    }
    TransferAddress = (UINT32 *)gRecoveryTransferAddr;
#endif

    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0x1FF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0x1FF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0x0);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0x0);

    Status=SDCard_GetStatus(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto ReadError;
    }

    if(SdioDevInfo->bState != CardStatus_tran) {
        Status=EFI_DEVICE_ERROR;
        goto ReadError;
    }

    //
    //Release DAT line before transfer
    //
    //SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetDAT);

    //
    //release CMD line before issue CMD
    //
    //SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetCMD);

    //
    //Send the command to controller
    //
    Status=SDCard_RWCMD_Controller(SdioDevInfo,Port,LBA, NumBlks, TransferAddress,TRUE);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto ReadError;
    }

    //
    //Wait for the command compelete
    //
    Status= SDCard_WaitCMDComplete(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto ReadError;
    }

    //
    //Wait for the transfer complete. Data copied to Bufferaddress
    //
    Status= SDCard_WaitXferComplete(SdioDevInfo,Port,(UINT32**)&BufferAddress,TransferAddress,&NumBlks,SdioRead);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto ReadError;
    }

    Status= SDCard_RestoreStatus(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto ReadError;
    }

    //
    // For Dma mode remaining data is copied to input buffer
    //
    if(SDIO_PIOflg == FALSE) {
            Status= SDCard_XferRemainderSector_Controller(SdioDevInfo,Port,(UINT32*)BufferAddress,TransferAddress,NumBlks);

        if(EFI_ERROR(Status)) {
            Status=EFI_DEVICE_ERROR;
            goto ReadError;
        }
    }

ReadError:

    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0xFF);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOAPI_WriteCard_Controller
//
// Description: Read the Data from Sd card
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOAPI_WriteCard_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    EFI_STATUS                  Status;
    UINT32                      *TransferAddress=0;
    UINT32                      BlockToWrite=NumBlks;
    UINT32                      Capability=0;
    BOOLEAN                     SdioRead=FALSE;
    SDIO_PIOflg=FALSE;

    //
    //If the SDIO access mode is Auto, based on capabilities device PIO and DMA
    //
    if(SDIO_Access_Mode == 0 ) {
        Capability=SDIO_REG32(SdioBaseAddr,CAP);
        if(Capability & SDMA_Support) {
            SDIO_PIOflg=FALSE;
        }else {
            SDIO_PIOflg=TRUE;
        }
    } else if(SDIO_Access_Mode == 1) {
        SDIO_PIOflg=FALSE;
    } else if(SDIO_Access_Mode == 2) {
        SDIO_PIOflg=TRUE;
    }

#ifndef SDIO_PEI_RECOVERY_PRESENT
    //
    //  Get the TransferBufferAddress
    //
    TransferAddress = (UINT32*)(gSdioData->TransferBufferAddress);
#else
    gRecoveryTransferAddr = (UINT32)gRecoveryTransferAddress;
    if((gRecoveryTransferAddr & 0x0FFF) != 0) {
        //
        // Make is 4Kb alligned address.
        //      
        gRecoveryTransferAddr = gRecoveryTransferAddr + (0x1000 - (gRecoveryTransferAddr & 0x00FFF));
    }
    TransferAddress = (UINT32 *)gRecoveryTransferAddr;
#endif

    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0x1FF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0x1FF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0x0);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0x0);

    Status=SDCard_GetStatus(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto WriteError;
    }

    if(SdioDevInfo->bState != CardStatus_tran) {
        Status=EFI_DEVICE_ERROR;
        goto WriteError;
    }

    //
    //For DMA mode access the Copy the data from Input buffer to DMA buffer
    //
    if(SDIO_PIOflg == FALSE) {
        SDCard_XferWdata2Buff(SdioDevInfo,Port,(UINT32**)&BufferAddress,TransferAddress,&NumBlks);
        if(EFI_ERROR(Status)) {
            Status=EFI_DEVICE_ERROR;
            goto WriteError;
        }
    }

    //
    //Release DAT line before transfer
    //
    //SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetDAT);

    //
    //release CMD line before issue CMD
    //
    //SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetCMD);

    //
    //Send the command to controller
    //
    Status = SDCard_RWCMD_Controller(SdioDevInfo,Port,LBA, BlockToWrite, TransferAddress,FALSE);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto WriteError;
    }

    //
    //Wait for the command complete command
    //
    Status= SDCard_WaitCMDComplete(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto WriteError;
    }

    //
    //Wait for the transfer complete and move the data from Input buffer to Dma buffer.
    //
    Status= SDCard_WaitXferComplete(SdioDevInfo,Port,(UINT32**)&BufferAddress,TransferAddress,&NumBlks,SdioRead);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto WriteError;
    }

    Status= SDCard_RestoreStatus(SdioDevInfo,Port);

    if(EFI_ERROR(Status)) {
        Status=EFI_DEVICE_ERROR;
        goto WriteError;
    }

WriteError:

    //
    //Clear the status.
    //
    SDIO_WRITE_REG16(SdioBaseAddr,NINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,NINTSIGEN,0xFF);
    SDIO_WRITE_REG16(SdioBaseAddr,ERINTSIGEN,0xFF);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDCard_RWCMD_Controller
//
// Description: Sends the Read or Write command to the SD controller
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDCard_RWCMD_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress,
    BOOLEAN                                 ReadWrite
)
{
    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);
    UINT32                      DmaAddress=(UINT32)BufferAddress;
    UINT32                      BlockLength=0;
    UINT16                      TransferMode=0;
    UINT16                      CmdData=0;
    UINT32                      BlockSize=0;
    UINT32                      Data32=0;
    UINT32                      Timeout = 1000;
    
    //
    // Check CMD INHIBIT and DATA INHIBIT before send command
    // 
      do {
        Data32 = SDIO_REG32(SdioBaseAddr,PSTATE);
      } while (Timeout-- > 0 && Data32 & BIT0);
      
      Timeout = 1000;
      do {
        Data32 = SDIO_REG32(SdioBaseAddr,PSTATE);    
      } while (Timeout-- >0 && Data32 & BIT1);

//;=================================================================
//; 1.Set SDMA Register
//;=================================================================
    if(SDIO_PIOflg == FALSE) {
        SDIO_WRITE_REG32(SdioBaseAddr,DMAADR,DmaAddress);
    }

//;=================================================================
//; 2.Set Argument Register (read write address from card)
//;=================================================================
    if(!((SdioDevInfo->bMode == SD_High_CAP) || 
           (SdioDevInfo->bMode == MMC_High_CAP)) ) {
        if(LBA >= 0x400000) {
            return EFI_DEVICE_ERROR;
        }
        LBA=(UINT32)LBA * BLOCK_SIZE;
    }

    SDIO_WRITE_REG32(SdioBaseAddr,CMDARG,LBA);

//;=================================================================
//; 3.Set Block Size Register, Default 4KB block
//;=================================================================

    if(SdioDevInfo->bMode & (SD_Stand_CAP+SD_High_CAP+\
                 MMC_High_CAP+MMC_Stand_CAP)) {
        BlockLength=BLOCK_SIZE;
    }

//;************************************************
//;resvered for SDIO and SPI mode  here
//;************************************************

    //
    // Right now we support only 4KB DMA buffer
    //
    BlockSize=(SDIO_Transfer_Buffer << 12) + BlockLength;
    SDIO_WRITE_REG16(SdioBaseAddr,BLKSZ,BlockSize);


//;=================================================================
//; 4.Set Block Count Register
//;=================================================================

    if(NumBlks == 0) {
        return EFI_DEVICE_ERROR;
    }

    SDIO_WRITE_REG16(SdioBaseAddr,BLKCNT,NumBlks);

    if(ReadWrite) {
    //
    //Read
    //
    if(NumBlks == 1) {
        TransferMode=Data_Transfer_Card2Host;
        CmdData=(READ_SINGLE_BLOCK << 8);
    } else {
        TransferMode= (Data_Transfer_Card2Host  + \
                    Block_Count_Enable + Multi_Block_Select + Auto_CMD12_Enable);
        CmdData=(READ_MULTIPLE_BLOCK << 8);
    }
    } else {
        //
        //Write
        //
        if(NumBlks == 1) {
            CmdData=(WRITE_SIGLE_BLOCK << 8);
        } else {          
            TransferMode= (Block_Count_Enable + Multi_Block_Select + Auto_CMD12_Enable);
            CmdData=(WRITE_MULTIPLE_BLOCK << 8);
        }

    }

    //
    //Enable DMA mode if PIO mode is false
    //
    if(SDIO_PIOflg == FALSE) {
        TransferMode|=DMA_Enable;
    }

    SDIO_WRITE_REG16(SdioBaseAddr,XFRMODE,TransferMode);

//;=================================================================
//; 6.Set CMD Index
//;=================================================================
    CmdData=CmdData + Resp1_Type+Data_Present_Select;

    SDIO_WRITE_REG16(SdioBaseAddr,CMD,CmdData);

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOAPI_ResetCard
//
// Description: Reset the SD card
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIOAPI_ResetCard (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);

    //
    //Release DAT line
    //
    SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetDAT);

    //
    //release CMD 
    //
    SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetCMD);


    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIO_ResetAll
//
// Description: Reset the SD card
//
// Input:
//    IN SDIO_DEVICE_INFO               *SdioDevInfo
//    IN UINT8                                Port
//
// Output:
//    EFI_STATUS
//
// Modified:
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SDIO_ResetAll (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
)
{

    UINT32                      SdioBaseAddr = (UINT32)(SdioDevInfo->SdioBaseAddress);

    //
    //Send Reset All command 
    //
    SDIO_WRITE_REG8(SdioBaseAddr,SWRST,ResetAll);

    return EFI_SUCCESS;
}

#ifndef SDIO_PEI_RECOVERY_PRESENT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOMassIdentifyDeviceType
//
// Description: This function identifies the device type and fill the 
//              geometry parameter into SDIO_DEVICE_INFO
//
// Input:  
//    SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
//    Port        - SDIO Port number
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SDIOMassIdentifyDeviceType (
    IN SDIO_DEVICE_INFO *SdioDevInfo,
    IN UINT8            Port )
{
    EFI_STATUS    Status;
    UINT16        wEmulationType;
    UINT16        wForceEmulationType = 0;
    static UINT16 SDIOMassEmulationTypeTable[4] = {
        0,                                                    // Auto
        (SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD,     // Floppy
        (SDIO_EMU_FORCED_FDD << 8) + SDIO_MASS_DEV_ARMD,      // Forced floppy
        (SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD };       // HDD

    if ( SdioDevInfo->wEmulationOption ) {                        // non Auto
        wEmulationType = SDIOMassEmulationTypeTable[SdioDevInfo->wEmulationOption];
        wForceEmulationType = SDIOMassEmulationTypeTable[SdioDevInfo->wEmulationOption];
    }

    if (gSdioReadData == NULL) {
    	Status = pBS->AllocatePool(EfiBootServicesData, 0x1000, &gSdioReadData);
    	ASSERT_EFI_ERROR(Status);
    }

    //
    // Assume floppy
    //
    wEmulationType = (UINT16)(SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD;

    if (SDIOGetFormatType( \
           SdioDevInfo, Port, gSdioReadData, &wEmulationType) == SDIO_ERROR) {
        //
        // Find the device type by size
        //
        if ((SdioDevInfo->dMaxLBA >> 11) > MAXIMUM_SIZE_FOR_FLOPPY_EMULATION ) {
            //
            // Assume HDD
            //
            wEmulationType = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
        }
    }

    if (wForceEmulationType) wEmulationType = wForceEmulationType;
    SdioDevInfo->bStorageType = (UINT8)wEmulationType;
    SdioDevInfo->bEmuType = (UINT8)(wEmulationType >> 8);

	if (gSdioReadData != NULL) {
		pBS->FreePool(gSdioReadData);
		gSdioReadData = NULL;
	}

    return;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOGetFormatType
//
// Description: This function reads the first sector from the mass storage
//              device and identifies the formatted type, Sets the Emulation
//              type accordingly.
//
// Input:  
//    IN  *SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
//    IN  Port        - SDIO Port number
//    IN  SdioReadData - Pointer to Buffer
//    OUT EmuType - Pointer to location that contains Emulation type of the 
//                  device
//
// Output: 
//       SDIO_ERROR   If could not identify the format type
//       SDIO_SUCCESS If formatted type is identified and EmuType is updated
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
SDIOGetFormatType (
    IN SDIO_DEVICE_INFO *SdioDevInfo,
    IN UINT8            Port,
    IN UINT8            *SdioReadData,
    OUT UINT16          *EmuType )
{
    EFI_STATUS Status;
    UINT16     Emu_Type;
    UINT8      *ActPartAddr;
    UINT32     dHS;

    //
    // Read the first sector of the device
    //
    Status = SDIOAPI_ReadCard_Controller(SdioDevInfo, Port, 0, 1, SdioReadData);

    if ( EFI_ERROR(Status) ) {
        //
        // If the Read from some MMC card is failed, then it may be
        // a problem of setting Bus Width to 4-bit. Reinitialize the
        // card and set the Bus width to 1-bit.
        //


        if(SdioDevInfo->MmcBusWidth == MMC_1_BIT_BUS_WIDTH) {
            return SDIO_ERROR;
        }
        Clock = 0x400;
        SdioDevInfo->MmcBusWidth = MMC_1_BIT_BUS_WIDTH;

        Status = ConfigureMassDevice_Controller(SdioDevInfo,Port);
        if(EFI_ERROR(Status)) {
            return SDIO_ERROR;
        }

        Status = GetDeviceInformation(SdioDevInfo,Port);
        if(EFI_ERROR(Status)) {
            return SDIO_ERROR;
        }

        Status = SDIOAPI_ReadCard_Controller(SdioDevInfo, Port, 0, 1, SdioReadData);
        if(EFI_ERROR(Status)) {
            return SDIO_ERROR;
        }
    }

    SdioDevInfo->bHiddenSectors = 0;

    //
    // Check for validity of Boot Record
    //
    if ( *(UINT16*)(SdioReadData + 0x1FE) != 0xAA55 ) {
        SDIOSetDefaultGeometry(SdioDevInfo, Port);
        return SDIO_ERROR;
    }

    //
    // Check for validity of the partition table
    //
    if ( SDIOValidatePartitionTable( \
            SdioReadData, SdioDevInfo->dMaxLBA, &ActPartAddr) == SDIO_SUCCESS ) {
        //
        // Only one partition present, check the device size, if the device size
        // is less than 530 MB assume FDD or else assume the emulation as HDD
        //
        if ( SdioDevInfo->dMaxLBA < MAX_LBA_FOR_FLOPPY_EMULATION ) {
            Emu_Type = (UINT16)(SDIO_EMU_FORCED_FDD << 8) + SDIO_MASS_DEV_ARMD;
        } else {
            Emu_Type = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
        }

        //
        // Read boot sector, set the LBA number to boot record LBA number
        //
        dHS = *((UINT32*)(ActPartAddr + 8));
        SdioDevInfo->bHiddenSectors = (UINT8)dHS; // Save hidden sector value

        Status = SDIOAPI_ReadCard_Controller(SdioDevInfo, Port, dHS, 1, SdioReadData);

        if ( EFI_ERROR(Status) ) return SDIO_ERROR;

        if ( SDIOUpdateCHSFromBootRecord( \
                SdioDevInfo, Port, SdioReadData) == SDIO_SUCCESS) {

            *EmuType = Emu_Type;
             return SDIO_SUCCESS;
        } else {

            SDIOSetDefaultGeometry(SdioDevInfo, Port);
            *EmuType = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
            SdioDevInfo->bHiddenSectors = 0;      // Reset hidden sector value
            return SDIO_SUCCESS;
        }
    }

    if ( SDIOUpdateCHSFromBootRecord(
            SdioDevInfo, Port, SdioReadData) == SDIO_SUCCESS) {

        // Assume the emulation as floppy
        // If boot record is a valid FAT/NTFS file system
        *EmuType = (UINT16)(SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD;
        return SDIO_SUCCESS;
    }
    
    SDIOSetDefaultGeometry(SdioDevInfo, Port);  
    Emu_Type = (UINT16)(SDIO_EMU_FLOPPY_ONLY << 8) + SDIO_MASS_DEV_ARMD;

    if ( SdioDevInfo->dMaxLBA >= MAX_LBA_FOR_FLOPPY_EMULATION ) {

        // Assume the emulation as HDD
        // If the device size greater than 530MB
        SdioDevInfo->bHiddenSectors = 0;
        Emu_Type = (UINT16)(SDIO_EMU_HDD_ONLY << 8) + SDIO_MASS_DEV_HDD;
    }

    *EmuType = Emu_Type;
    return SDIO_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SdioMassUpdateCylinderInfo
//
// Description: This procedure updates cylinder parameter for device geometry.
//              head and sector paramaters are required before invoking this
//              function.
//
// Input:  
//    *SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
//    Port        - SDIO Port number
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SdioMassUpdateCylinderInfo (
    IN SDIO_DEVICE_INFO *SdioDevInfo,
    IN UINT8            Port )
{
    UINT32 data = SdioDevInfo->dMaxLBA /(SdioDevInfo->NumSectors * SdioDevInfo->NumHeads);

    if (data <= 1) data++;
    if (data > 0xFFFF) data = 0xFFFF;   // DOS workaround

    SdioDevInfo->NumCylinders = (UINT16)data;
    SdioDevInfo->LBANumCyls = (UINT16)data;
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOSetDefaultGeometry
//
// Description: This procedure sets the  default geometry for mass 
//              storage devices.
//
// Input:  
//    *SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
//    Port         - SDIO Port number
//
// Output:  SDIO_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 
SDIOSetDefaultGeometry ( 
    IN SDIO_DEVICE_INFO *SdioDevInfo,
    IN UINT8            Port )
{
    SdioDevInfo->NumHeads = 0xFF;
    SdioDevInfo->LBANumHeads = 0xFF;
    SdioDevInfo->NumSectors = 0x3F;
    SdioDevInfo->LBANumSectors = 0x3F;

    SdioMassUpdateCylinderInfo(SdioDevInfo, Port);
    return SDIO_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOValidatePartitionTable
//
// Description: This procedure checks whether the partition table is valid.
//
// Input:   IN  *Buffer - Pointer to the Boot Record of the device
//          IN  dMaxLBA - Max LBA of the device
//          OUT **ActPartAddr - Pointer to a pointer that 
//                            contains Active Partition offset
//
// Output:  SDIO_SUCCESS - partion table is valid:
//              ValidEntryCount - Possible valid entry count ( 1-based )
//              ActPartAddr - Active entry offset ( Absolute offset )
//          SDIO_ERROR - Invalid partition table
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
SDIOValidatePartitionTable (
    IN  UINT8  *Buffer,
    IN  UINT32 dMaxLBA,
    OUT UINT8  **ActPartAddr )
{

    UINT8  *PartPtr;
    UINT8  PartNo = 0;
    UINT8  *ActPart = NULL;
    UINT8  ValidEntryCount = 0;

    //
    // Drive has a partition table, start from 1st bootable partition
    //
    PartPtr = Buffer + 0x1BE;

    for (; PartNo<4; PartNo++, PartPtr+=0x10 ) {

        if (*PartPtr & 0x7F ) return SDIO_ERROR; //BootFlag should be 0x0 or 0x80
        //
        // Check whether beginning LBA is reasonable
        //
        if (*(UINT32*)(PartPtr + 8) > dMaxLBA) return SDIO_ERROR;
        
        ValidEntryCount++;                      // Update valid entry count
        //
        // Update active entry offset
        //
        if (!(*PartPtr & 0x80)) continue;
        if (ActPart) continue;
        ActPart = PartPtr;
    }

    if (ValidEntryCount < 1) return SDIO_ERROR; // Atleast one valid partition is found
    //
    // If no active partition table entry found use first entry
    //
    if (ActPart == NULL) ActPart = Buffer + 0x1BE;

    *ActPartAddr = ActPart;

    return SDIO_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SDIOUpdateCHSFromBootRecord
//
// Description: This function parses the boot record and extract the CHS
//      information of the formatted media from the boot record.
//      This routine checks for DOS & NTFS formats only
//
// Input:
//    *SdioDevInfo - Pointer to SDIO_DEVICE_INFO structure
//    Port         - SDIO Port number
//    *SdBootRecord - Pointer to Boot record of the device
//
// Output:  SDIO_ERROR - If the boot record is un-recognizable and CHS info
//                       is not extracted
//          SDIO_SUCCESS - If the boot record is recognizable and CHS info
//                         is extracted. CHS information is updated in the
//                         SDIO_DEVICE_INFO structure
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
SDIOUpdateCHSFromBootRecord (
    IN SDIO_DEVICE_INFO*   SdioDevInfo,
    IN UINT8               Port,
    IN UINT8*              SdBootRecord )
{
    UINT32      OemName;
    UINT8       Heads;
    UINT8       SecPerTrack;
    UINT16      SecTimesHeads;
    UINT16      TotalSect;

    if (*((UINT16*)(SdBootRecord + 0x1FE)) != 0xAA55) return SDIO_ERROR;

    //
    // Read succeeded so the drive is formatted
    // Check for valid MSDOS/MSWIN/NTFS boot record
    //
    OemName = *(UINT32*)(SdBootRecord + 3);
    
    if ((OemName != 0x4F44534D) &&   // 'ODSM' for MSDO
        (OemName != 0x4957534D) &&   // 'IWSM' for MSWI
        (OemName != 0x5346544E)) {   // 'SFTN' for NTFS
        //
        // Check for valid FAT,FAT16,FAT32 boot records
        //
        *(SdBootRecord + 0x36 + 3) = 0x20;              // Ignore the 4th byte and fill it with space
        if ((*(UINT32*)(SdBootRecord + 0x36) != 0x20544146) &&      // " TAF" for FATx
            (*(UINT32*)(SdBootRecord + 0x52) != 0x33544146)) {      // "3TAF" for FAT3
            //
            // Boot Record is invalid. Return with error
            //
            return SDIO_ERROR;
        }
    }

    Heads = *(SdBootRecord + 0x1A);         // Number of heads
    SecPerTrack = *(SdBootRecord + 0x18);   // Sectors/track
    SecTimesHeads = Heads * SecPerTrack;

    // Zero check added to prevent invalid sector/head information in BPB
    if (SecTimesHeads == 0) {
        return SDIO_ERROR;
    }

    TotalSect = *(UINT16*)(SdBootRecord + 0x13);
    if ( TotalSect ) {
        SdioDevInfo->dMaxLBA = TotalSect;
    }

    SdioDevInfo->NumHeads = Heads;
    SdioDevInfo->LBANumHeads = Heads;
    SdioDevInfo->NumSectors = SecPerTrack;
    SdioDevInfo->LBANumSectors = SecPerTrack;

    SdioMassUpdateCylinderInfo(SdioDevInfo, Port);

    return  SDIO_SUCCESS;
}

#endif
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
