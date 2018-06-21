//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file Mmc.c
    Functions to handle Mmc related function

**/
//----------------------------------------------------------------------
#include "SdioController.h"
//----------------------------------------------------------------------
extern UINT32           gClock;
extern SDIO_GLOBAL_DATA *gSdioData;

/**
    To program the power and get the OCR

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: Following step 4 to 9 Jedec Electrical standard 4.51 and 5.01
    
**/
EFI_STATUS
MmcGetOcr (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port,
    IN  UINT32              Argument,
    OUT UINT32              *Response
) 
{
    EFI_STATUS  Status;
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    UINT16      i;
    

    //
    // Older version i.e Jedec Electrical standard 4.51 suggest to issue cmd0
    // (as per step 4 from A.6.1 Bus initialization). The second time when 
    // MmcGetOcr funtion is called i.e while programming for  1.8 V switching  
    // the card will be in ready state 
    //
    CommandIndex=GO_IDLE_STATE_CMD0;
    CommandArgument=0;
    Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }


    //
    // Issue CMD1 to get OCR (as per step 4 from Sec A.6.1 Bus initialization)
    // mentioned Jedec Electrical standard 5.01
    //
    CommandIndex = (SEND_OP_COND_CMD1 << 8) | (RESP3_TYPE);
    
    //
    // Issue cmd1 for atleast 1 sec as per (10.1.2 e•MMC power-up guidelines)
    // from Jedec Electrical standard 5.01  
    //
    for(i=0; i <= MMC_POWERUP_TIMEOUT; i++) {
        Status = SdMmcCommand(SdioDevInfo, Port, CommandIndex, Argument);
        if(EFI_ERROR(Status)) {
            return EFI_DEVICE_ERROR;
        }
        
        //
        // Check if busy bit is set.
        //
        *Response = SDIO_REG32(SdioBaseAddr,RESP0);
        if(*Response & CARD_POWERUP_STATUS_BIT) {
            return EFI_SUCCESS;
        }
        
        //
        // Delay for 10 Msec
        // 
        MicroSecondDelay (10000);
    }

    return EFI_TIMEOUT;
}

/**
    Set the Clock value for the MMC device

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note : This function follows :
    1. Jedec eMMC Electrical Standard 5.01
    2. SD Host Controller Simplified Specification Version 3.00 
**/
EFI_STATUS
MmcCard_SetClock (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    UINT32      MaxCardClk=0;
    BOOLEAN     HighSpeedSupport=FALSE;
    UINT32      MaxHstClk;
    EFI_STATUS  Status;
    
    // 
    // This function will check HighSpeedSupport support for host and will 
    // calculate the maximum host controller frequecy and maximum card 
    // frequecy
    //
    CalculateMaxCardAndHostFreq (SdioDevInfo, 
                                Port, 
                                &HighSpeedSupport, 
                                &MaxCardClk,
                                &MaxHstClk);

    //
    // If host support Highspeed and card support High-Speed e•MMC @ 52MHz then set to 52 Mhz
    // else if host support Highspeed and card support High-Speed e•MMC @ 26MHz set to 26 Mhz
    // else set to normal mode
    //
if( (HighSpeedSupport) && (SdioDevInfo->bMmcCardType & MMC_HS_SDR_52 )) {
        Status = ControllerSetClock(SdioDevInfo,Port,52000000);
        if(EFI_ERROR(Status)) {
            return Status;
        }
    } else if((HighSpeedSupport) &&  (SdioDevInfo->bMmcCardType & MMC_HS_SDR_26 )){
        Status = ControllerSetClock(SdioDevInfo,Port,26000000);
        if(EFI_ERROR(Status)) {
            return Status;
        }
    } else {
        //
        // In Normal mode we try to set the clock to 25Mhz or less 
        //
        if( (MaxHstClk <= MaxCardClk ) && (MaxHstClk < 25000000)) {
            Status = ControllerSetClock(SdioDevInfo,Port,MaxHstClk);
            if(EFI_ERROR(Status)) {
                return Status;
            }
        } else if( (MaxCardClk < MaxHstClk ) && (MaxCardClk < 25000000)){
            Status = ControllerSetClock(SdioDevInfo,Port,MaxCardClk);
            if(EFI_ERROR(Status)) {
                return Status;
            }
        } else {
            Status = ControllerSetClock(SdioDevInfo,Port,25000000);
            if(EFI_ERROR(Status)) {
                return Status;
            }
        }
    }
    return EFI_SUCCESS;
}

/**
    Get Extended CSD from MMC device if version is 4.0 and higher

        
    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note : This function follows This function follows step 23of Sec  A.6.1 
          Bus initialization from Jedec Electrical standard 5.01  
          specification

**/
EFI_STATUS
MmcCardGetExtCSD (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    EFI_STATUS  Status;
    UINT8      TransferAddress[512];
    UINT32     *Temp;

    Status = SdMmcReadWriteData (SdioDevInfo,Port,SEND_EXT_CSD_CMD8,0,(UINT32*)&TransferAddress[0],0,512,RESP1_TYPE,FALSE,TRUE);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    // Get the card type and Sector count from EXT_CSD register
    //
    SdioDevInfo->bMmcCardType = TransferAddress[196] & 0xff;  // CARD_TYPE [196]
    Temp = (UINT32*)&TransferAddress[212];
    SdioDevInfo->bMmcSecCount = *Temp;        // SEC_COUNT [215:212]
    InternalMemCopyMem(&SdioDevInfo->Ext_Csd, &TransferAddress[0], sizeof(EXT_CSD));
#if SDMMC_VERBOSE_PRINT
    DEBUG((EFI_D_VERBOSE ,"SdMmc: EXT_CSD Device type=0x%x \n",SdioDevInfo->bMmcCardType));
    DEBUG((EFI_D_VERBOSE ,"SdMmc: EXT_CSD Max Sector Count =0x%x \n",SdioDevInfo->bMmcSecCount));
#endif
    if( SdioDevInfo->bCsdStruct == 3 ) {
        SdioDevInfo->bCsdStruct = (TransferAddress[48] & 0xff0000 ) >> 16 ; // CSD_STRUCTURE [194]
    }

    return Status;

}

/**
    Set the RCA value from the MMC card

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Noate: This function follows step 17 of Sec A.6.1 Bus initialization
           from Jedec Electrical standard 5.01  specification.
           1. Issue CMD3 with new RCA.
           2. Receive R1, and put card to stand by stae.
**/
EFI_STATUS
MmcCardSetRCA (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{

    UINT16      CommandIndex;
    UINT32      CommandArgument;
    EFI_STATUS  Status;

    if((SdioDevInfo->bState != CARDSTATUS_IDENT) &&
        (SdioDevInfo->bState != CARDSTATUS_STBY)) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Issue CMD3 for all MMC devices
    //
    CommandIndex = ((SEND_RELATIVE_ADDR_CMD3 << 8) | RESP1_TYPE);
    CommandArgument = (Port+2) << 16;
    Status = SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    SdioDevInfo->wRCA=(UINT16)(Port+2);
    SdioDevInfo->bState=CARDSTATUS_STBY;

    return EFI_SUCCESS;
}

/**
    This function checks whether  1- bit mode is supported
     by the MMC card

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
TestMmc1BitSupport (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
) 
{

    EFI_STATUS  Status;
    UINT8       TransferBuffer[8]= {0};
    UINT8       Temp;

    //
    // Send command 19 to test the 1 bit bus with
    //
    TransferBuffer[0] = 0x40;
    Status = SdMmcReadWriteData (SdioDevInfo,
                                     Port,
                                     BUSTEST_WRITE_CMD19,
                                     0,
                                     (UINT32*)&TransferBuffer[0],
                                     1,
                                     8,
                                     RESP1_TYPE,
                                     FALSE,
                                     FALSE);
    if(EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }
    
    //
    // Verify with command 14 whether the bit has been toggled
    //
    Temp = SdioDevInfo->MmcBusWidth;
    SdioDevInfo->MmcBusWidth = BUSTEST_READ_CMD14;
    Status = SdMmcReadWriteData (SdioDevInfo,
                                     Port,
                                     BUSTEST_READ_CMD14,
                                     0,
                                     (UINT32*)&TransferBuffer[0],
                                     1,
                                     8,
                                     RESP1_TYPE,
                                     FALSE,
                                     TRUE);
    SdioDevInfo->MmcBusWidth = Temp;    
    if((EFI_ERROR(Status))&& (TransferBuffer[0]& 0xc0)!=0x80 ) {
        return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
}

/**
    This function checks whether 4-bit bus mode is supported by the
    MMC card

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
TestMmc4BitSupport (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    EFI_STATUS  Status;
    UINT8       TransferBuffer[8]= {0};
    UINT8       Temp;
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    
    
    
    //
    // Send command 19 to test the 4 bit bus with
    //
    TransferBuffer[0] = 0x5a;
    Status = SdMmcReadWriteData (SdioDevInfo,
                                     Port,
                                     BUSTEST_WRITE_CMD19,
                                     0,
                                     (UINT32*)&TransferBuffer[0],
                                     1,
                                     8,
                                     RESP1_TYPE,
                                     FALSE,
                                     FALSE);
    if(EFI_ERROR(Status)) {
        SDIO_REG8_AND(SdioBaseAddr,HOSTCTL,~(DATA_TRANSFER_4BITMODE));
        SdioDevInfo->MmcBusWidth = MMC_1_BIT_BUS_WIDTH;
        return TestMmc1BitSupport(SdioDevInfo,Port);
    }

    //
    // Verify with command 14 if the bit has been toggled
    //
    Temp = SdioDevInfo->MmcBusWidth;
    SdioDevInfo->MmcBusWidth = BUSTEST_READ_CMD14;
    Status = SdMmcReadWriteData (SdioDevInfo,
                                     Port,
                                     BUSTEST_READ_CMD14,
                                     0,
                                     (UINT32*)&TransferBuffer[0],
                                     1,
                                     8,
                                     RESP1_TYPE,
                                     FALSE,
                                     TRUE);
    SdioDevInfo->MmcBusWidth = Temp;
    if((EFI_ERROR(Status))|| TransferBuffer[0]!=0xa5 ) {
        SDIO_REG8_AND(SdioBaseAddr,HOSTCTL,~(DATA_TRANSFER_4BITMODE));
        SdioDevInfo->MmcBusWidth = MMC_1_BIT_BUS_WIDTH;
        return TestMmc1BitSupport(SdioDevInfo,Port);
    }
    return EFI_SUCCESS;
}

/**
    This function checks whether 8-bit or 4-bit or 1- bit mode 
    is supported by the MMC card

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
MmcBusWidthTest (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    EFI_STATUS  Status;
    UINT8       TransferBuffer[8]= {0};
    UINT8       Temp;
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    
    if(SdioDevInfo->MmcBusWidth == MMC_8_BIT_BUS_WIDTH ) {
        
        TransferBuffer[0] = 0x55;
        TransferBuffer[1] = 0xaa;
        
        //
        // Send command 19 to test the 8 bit bus with
        //
        Status =SdMmcReadWriteData (SdioDevInfo,
                                     Port,
                                     BUSTEST_WRITE_CMD19,
                                     0,
                                     (UINT32*)&TransferBuffer[0],
                                     1,
                                     8,
                                     RESP1_TYPE,
                                     FALSE,
                                     FALSE);
        if(EFI_ERROR(Status)) {
            SDIO_REG8_AND_OR(SdioBaseAddr,HOSTCTL,~(DATA_TRANSFER_8BITMODE),DATA_TRANSFER_4BITMODE);
            SdioDevInfo->MmcBusWidth = MMC_4_BIT_BUS_WIDTH;
            return TestMmc4BitSupport(SdioDevInfo,Port);
        }

        //
        // Verify with command 14 if the bit has been toggled
        //
        Temp = SdioDevInfo->MmcBusWidth;
        SdioDevInfo->MmcBusWidth = BUSTEST_READ_CMD14;
        Status  = SdMmcReadWriteData (SdioDevInfo,
                                     Port,
                                     BUSTEST_READ_CMD14,
                                     0,
                                     (UINT32*)&TransferBuffer[0],
                                     1,
                                     8,
                                     RESP1_TYPE,
                                     FALSE,
                                     TRUE);
        SdioDevInfo->MmcBusWidth = Temp;
        if((EFI_ERROR(Status))|| TransferBuffer[0]!=0xaa || TransferBuffer[1]!=0x55) {
            SDIO_REG8_AND_OR(SdioBaseAddr,HOSTCTL,~(DATA_TRANSFER_8BITMODE),DATA_TRANSFER_4BITMODE);
            SdioDevInfo->MmcBusWidth = MMC_4_BIT_BUS_WIDTH;
            return TestMmc4BitSupport(SdioDevInfo,Port);
        }
        return EFI_SUCCESS;
        
    }else if (SdioDevInfo->MmcBusWidth == MMC_4_BIT_BUS_WIDTH) {
        return TestMmc4BitSupport(SdioDevInfo,Port);
    }else {
        return TestMmc1BitSupport(SdioDevInfo,Port);
    }
    
}

/**
    Set the Bus width for the card

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: This function follows the Section A.6.2 Switching to high-speed 
        mode of Jedec Electrical standard 5.01  
**/
EFI_STATUS
MmcCardBuswidth (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    EFI_STATUS  Status;
    UINT32      CapValue=0;
    
    //
    // Currently only 4-bit bus width is set for the MMC/eMMC device.
    //
    if (SdioDevInfo->bSpecVers < 4) {
        //
        // Old MMC Card's does not support Bus Width command.
        //
        SdioDevInfo->MmcBusWidth = MMC_1_BIT_BUS_WIDTH;
        return EFI_SUCCESS;
    }

    //
    // Clear the Bus with Bits
    //
    SDIO_REG8_AND(SdioBaseAddr,HOSTCTL,~(DATA_TRANSFER_8BITMODE+DATA_TRANSFER_4BITMODE));
    
    CapValue = SDIO_REG32(SdioBaseAddr,CAP);
    if ((SdioDevInfo->bHostControllerVersion >= HOST_CONTROLLER_VER3) &&((CapValue & BIT18) && ((CapValue>>30)==1))) {
        //
        // Select the 8 bit mode in host controller
        //
        SDIO_REG8_OR(SdioBaseAddr,HOSTCTL,DATA_TRANSFER_8BITMODE);
        SdioDevInfo->MmcBusWidth = MMC_8_BIT_BUS_WIDTH;
    } else {
        //
        // Select the 4 bit mode in host controller
        //
        SDIO_REG8_OR(SdioBaseAddr,HOSTCTL,DATA_TRANSFER_4BITMODE);
        SdioDevInfo->MmcBusWidth = MMC_4_BIT_BUS_WIDTH;
    }
    
#if BUS_TEST_SUPPORT == 1
    //
    // This function send cmd 19 and cmd14 to test bus with
    // as per step 26 to 29 of A.6.2 Switching to high-speed 
    //
    Status= MmcBusWidthTest(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        SdioDevInfo->MmcBusWidth = MMC_1_BIT_BUS_WIDTH;
        return EFI_SUCCESS;
    }
#endif
    
    CommandIndex = ((SWITCH_FUNC_CMD6<< 8) | RESP1B_TYPE);
    if(SdioDevInfo->MmcBusWidth == MMC_1_BIT_BUS_WIDTH) {
        CommandArgument = 0 | (0 << 8) | (183 << 16) | (3 << 24);   // 1-bit bus width
    } else if(SdioDevInfo->MmcBusWidth == MMC_4_BIT_BUS_WIDTH) {
        CommandArgument = 0 | (1 << 8) | (183 << 16) | (3 << 24);   // 4-bit bus width  
    } else if(SdioDevInfo->MmcBusWidth == MMC_8_BIT_BUS_WIDTH) {
        CommandArgument = 0 | (2 << 8) | (183 << 16) | (3 << 24);   // 8-bit bus width
    } else {
        return EFI_SUCCESS;
    }

    Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }
	
    //
    // Check if SWITCH_ERROR occured
    //
    Status=SdMmcCardGetStatus(SdioDevInfo,Port);
    if(EFI_ERROR(Status) && (SdioDevInfo->CurrentState != SWITCH_ERROR)) {
        return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
}

/**
    Set the Sd device Bus width

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: This function follows Section 6.6.4 "HS200" timing mode selection
    from Jedec Electrical standard 5.01  specification. Please note we are 
    not programming driver strength. It is kept to default
    

**/
EFI_STATUS
MmcCardHS200 (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    EFI_STATUS  Status;
    UINT32      MaxHstClk=0;
    UINT32      CapValue=0;

    
    //
    // AMI SDIO driver follows SD Host Controller Simplified Specification
    // Version 3.00. So 1.8V IO volatage only supported for HS200 emmc.
    //
    CapValue=SDIO_REG32(SdioBaseAddr,CAP);
    
    if(!((CapValue & BIT26)&&(CapValue & BIT21))) {
        return EFI_SUCCESS;
    }
    
    MaxHstClk = CapValue;
    if(SdioDevInfo->bHostControllerVersion == HOST_CONTROLLER_VER3) {
        MaxHstClk &= BASE_CLOCK_VER3;
    } else {
        MaxHstClk &= BASE_CLOCK;
    }

    MaxHstClk = (MaxHstClk >> 8);
    if(MaxHstClk <= 52) {
        return EFI_SUCCESS;
    }

    //
    // Check CARD_TYPE to confirm if it supports HS200 at 1.8V and Bus width
    // is not set to one as per step 2 of 6.6.4 "HS200" timing mode selection
    //
    if( ((!(SdioDevInfo->bMmcCardType & MMC_HS200_SDR_18V))|| (SdioDevInfo->MmcBusWidth == MMC_1_BIT_BUS_WIDTH))  ) {    
        return EFI_SUCCESS;
    }
    
    //
    // Check if 1.8V switching in controller was successful
    //
    if(SdioDevInfo->VoltageSwitch == FALSE) {
        return EFI_SUCCESS;
    }
       
    //
    // Set HS200 in HS_TIMING to the eMMC device. As per step 4 of 
    // 6.6.4 "HS200" timing mode selection
    //
    CommandIndex = ((SWITCH_FUNC_CMD6<< 8) | RESP1B_TYPE);
    CommandArgument = 0 | (2 << 8) | (185 << 16) | (3 << 24);
    Status=SdMmcCommand(SdioDevInfo, Port,CommandIndex,CommandArgument);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    
    //
    // As per 4 of 6.6.4 "HS200" timing mode selection send command 13
    //
    Status=SdMmcCardGetStatus(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }
    
    //
    // As per step 4 of 6.6.4 "HS200" timing mode selection the state of the
    // card should be tran state
    //
    if(SdioDevInfo->bState != CARDSTATUS_TRAN) {
        return EFI_DEVICE_ERROR;
    }
    
    //
    // For the tuning for HS200 the SDR104 should 
    // be enabled or the tuning will fail at 200mhz
    //
    SDIO_REG8_OR(SdioBaseAddr,HOSTCTL2, SDR104_UHS_SELECT);
    
    //
    // As per ste 5 nd 6 of "6.6.4 "HS200" timing mode selection" set the
    // clock frequency to 200 Mhz and do tuning.
    //
    Status=HS200ClockTunning(SdioDevInfo,Port,MaxHstClk);
    if(EFI_ERROR(Status)) { 
        return Status;
    }
    return EFI_SUCCESS;

}


/**
    This function issues tuning command to MMC card

    @param  SdioDevInterface 
    @param  Port
    @param  BufferAddress 
    @param  Size
    @retval EFI_STATUS

**/
EFI_STATUS
MmcSendTuningCommand (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port,
    IN  VOID                *BufferAddress,
    IN  UINT16              Size
)
{
    EFI_STATUS              Status;
    UINT16                  CommandIndex;
    UINT64                  SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    
    //
    // Configure the transfer mode register, Bloack size register and block
    // count register for the transfer
    //
    SDIO_WRITE_REG16(SdioBaseAddr,BLKSZ,Size);
    SDIO_WRITE_REG16(SdioBaseAddr,BLKCNT,1);

    SDIO_WRITE_REG16(SdioBaseAddr,XFRMODE,DATA_TRANSFER_CARD2HOST);
    //
    // Send the Command argument and command
    //
    CommandIndex = (SEND_TUNING_MMC_CMD21 << 8) | RESP1_TYPE+DATA_PRESENT_SELECT;
    Status=SdMmcCommand (SdioDevInfo,Port,CommandIndex,0);
    if(EFI_ERROR(Status)) {
        return EFI_DEVICE_ERROR;
    }   
    //
    // Wait for the transfer to complete
    //
    Status= SdMmcWaitXferComplete(SdioDevInfo,Port,(UINT32**)&BufferAddress,Size,1);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    return Status;
}
/**
    Clock tunning for HS200

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: This function follows setion 6.6.8.1 Sampling Tuning Sequence for HS200
**/
EFI_STATUS
HS200ClockTunning (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port,
    IN  UINT32              MaxHstClk
)
{
    UINT16      TuningBlockSize = 0;
    UINT8       Buffer[128];
    UINT32      OriginalClock;
    EFI_STATUS  Status;
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    UINT16      Temp16;
    UINT8       i=80;
    UINT32      CapValue;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    
    //
    // Save the orginal clock
    //
    OriginalClock = gClock;    
    //
    // Set the new clock to 200Mhz   
    //
    // If bus width set to 4bit the data pattern is in Tuning_block_4bit
    // else If bus width set to 8bit the data pattern is in Tuning_block_8bit
    //
    if(SdioDevInfo->MmcBusWidth == MMC_4_BIT_BUS_WIDTH) {
        TuningBlockSize = 64;
    } else if(SdioDevInfo->MmcBusWidth == MMC_8_BIT_BUS_WIDTH){
        TuningBlockSize = 128;
    }     
    //
    //disable clock first.Set the new clock to 200Mhz
    //
    ControllerSetClock(SdioDevInfo,Port,200000000);
    SDIO_REG16_AND(SdioBaseAddr,HOSTCTL2, ~SAMPLING_CLOCK_SELECT);
    MicroSecondDelay (1000);
    SDIO_REG16_OR(SdioBaseAddr,HOSTCTL2, EXECUTE_TUNING);
    do {
        // Send tuning Command 
        //
        Status  = MmcSendTuningCommand (SdioDevInfo,
                                     Port,
                                     (UINT32*)&Buffer[0],
                                     TuningBlockSize
                                     );
        if(EFI_ERROR(Status)) {
            //
            // HS200 failed clear the error status for  data and cmd.
            //
            SDIO_WRITE_REG16(SdioBaseAddr,ERINTSTS,0xFFFF);
            SDIO_WRITE_REG16(SdioBaseAddr,NINTSTS,0xFFFF);
            continue;
        }        
        Temp16 = SDIO_REG16(SdioBaseAddr,HOSTCTL2);
        if(!(Temp16&EXECUTE_TUNING )) {
            break;
        }
        MicroSecondDelay (3000);
    }while(i--);
    
    Temp16 = SDIO_REG16(SdioBaseAddr,HOSTCTL2);    
    if(Temp16&SAMPLING_CLOCK_SELECT) {
#if SDMMC_VERBOSE_PRINT
        DEBUG((EFI_D_VERBOSE ,"SdMmc: tuning successful i =%x\n",i));
#endif
        return EFI_SUCCESS;
    } else {
#if SDMMC_VERBOSE_PRINT
        DEBUG((EFI_D_VERBOSE ,"SdMmc: tuning Failed reverting to previous mode \n"));
#endif        
        //
        // If tunning failed then revert back to High speed
        //        
        SDIO_REG8_AND(SdioBaseAddr,HOSTCTL2, ~SDR104_UHS_SELECT);
        ControllerSetClock(SdioDevInfo,Port,OriginalClock);
        
        CapValue=SDIO_REG32(SdioDevInfo->SdioBaseAddress,CAP);
        if ( (CapValue & HIGH_SPEED_SUPPORT) && (OriginalClock > 26000000) ) {
            
            CommandIndex = ((SWITCH_FUNC_CMD6<< 8) | RESP1B_TYPE);
            CommandArgument = 0 | (1 << 8) | (185 << 16) | (3 << 24);
            Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
            if(EFI_ERROR(Status)) {
                return Status;
            }
            //
            // As per 6.6.2 High-speed modes selection send command 13
            //
            Status=SdMmcCardGetStatus(SdioDevInfo,Port);
            if(EFI_ERROR(Status) && (Status != EFI_CRC_ERROR)) {
                return EFI_DEVICE_ERROR;
            }
        }
        return EFI_DEVICE_ERROR;
    }
    return EFI_SUCCESS;
}

/**
    Switching to High speed

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: This function follows step 21-24 of Sec A.6.1 Bus initialization
           from Jedec Electrical standard 5.01  specification
**/
EFI_STATUS
MmcSetHighSpeed (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
) {
    UINT32      CapValue;
    EFI_STATUS  Status;
    UINT16      CommandIndex;
    UINT32      CommandArgument;
    
    //
    // Send CMD7 with the Device’s RCA to place the Device in tran state
    // as per step 21 of A.6.1 Bus initialization
    //
    Status= SdMmcCardSelect(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    // Get extended CSD register if MMC 4.0 or higher as per step 22 and 23
    // of A.6.1 Bus initialization
    //
    if (SdioDevInfo->bSpecVers >= 4) {
        Status = MmcCardGetExtCSD(SdioDevInfo, Port);
        if (EFI_ERROR(Status)) {
            return Status;
        }
        
        //
        // Set to High speed interface by enabling HS_TIMING in extended CSD reg.
        // if host controller supports High Speed. As per step 23 of A.6.1 Bus 
        // initialization
        //
        CapValue=SDIO_REG32(SdioDevInfo->SdioBaseAddress,CAP);
        if ( CapValue & HIGH_SPEED_SUPPORT ) {                                 
            CommandIndex = ((SWITCH_FUNC_CMD6<< 8) | RESP1B_TYPE);
            CommandArgument = 0 | (1 << 8) | (185 << 16) | (3 << 24);
            Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,CommandArgument);
            if(EFI_ERROR(Status)) {
                return Status;
            }
            
            //
            // As per 6.6.2 High-speed modes selection send command 13
            //
            Status=SdMmcCardGetStatus(SdioDevInfo,Port);
            if(EFI_ERROR(Status) && (Status != EFI_CRC_ERROR)) {
                return EFI_DEVICE_ERROR;
            }
        }
    }
    
    //
    // Set the clock as per step 24 of A.6.1 Bus initialization
    //
    Status= MmcCard_SetClock(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    return EFI_SUCCESS;

}
/**
    Initialize the eMMC card.

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS
    Note: This function is as per Jedec eMMC Electrical Standard 5.01
          (Sec A.6.1 Bus initialization step 4 to 36). The step 1 to
          3 is already taken care in ControllerInitEnvironment function
**/
EFI_STATUS
MmcCardInit (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{

    EFI_STATUS  Status;
    UINT32      CommandArgument;
    UINT32      Response;
#if HS200_MMC_SUPPORT == 1
    UINT64      SdioBaseAddr = SdioDevInfo->SdioBaseAddress;
    UINT32      CapValue;
    UINT32      PresentState;
    UINT32      Temp32;
#endif
    SdioDevInfo->VoltageSwitch = FALSE;
#if SDMMC_VERBOSE_PRINT
    DEBUG((EFI_D_VERBOSE,"SdMmc: Mmc Card Found.. Initializing Mmc Card\n"));
#endif
    //
    // This function is to get OCR.
    // As per step 4-8 from sec Sec A.6.1 Bus initialization
    //
    CommandArgument = (CARD_POWERUP_STATUS_BIT| CARD_CAPACITY_STATUS | CARD_VOLTAGE_WINDOW);
    Status = MmcGetOcr(SdioDevInfo,Port,CommandArgument,&Response);
    if(EFI_ERROR(Status)) {
        return EFI_NOT_FOUND;
    }
    SdioDevInfo->dOCR = Response;
    
#if HS200_MMC_SUPPORT == 1
    
    CapValue=SDIO_REG32(SdioBaseAddr,CAP);
    
    //
    // Set 1.8V Programming in card as per  step 10-14 of Sec A.6.1 Bus 
    // initialization of Jedec Electrical standard 5.01
    //
    if((CapValue & VOLTAGE_SUPPORT_18)&&((SdioDevInfo->dOCR)&CARD_LOW_VOLTAGE_WINDOW)) {

        SDIO_REG8_AND(SdioBaseAddr,PWRCTL,~PWRCTL_POWER_ENABLE);
        SDIO_WRITE_REG8(SdioBaseAddr,PWRCTL,PWRCTL_18V);
        SDIO_REG8_OR(SdioBaseAddr,PWRCTL,PWRCTL_POWER_ENABLE);
        
        //
        // Delay mentioned in step 11 of A.6.1 Bus initialization of JEDEC 
        // Ver5.01
        //
        MicroSecondDelay (1000);
        
        //
        // Stop providing SD clock to the card.As mentioned in step 4 of sec 
        // Signal Voltage Switch Procedure of Sd host controller 3.00
        // 
        SDIO_REG8_AND(SdioBaseAddr,CLKCTL,~CLKCTL_CLOCK_ENABLE);
        
        //
        // As mentioned in step 6 of sec Signal Voltage Switch Procedure
        // of Sd host controller 3.00
        //
        SDIO_REG8_OR(SdioBaseAddr,HOSTCTL2,SIGNALING_ENABLE_1_8V);
        
        //
        // Delay is mentioned in 3.6.1 Signal Voltage Switch Procedure of step 7.
        //
        MicroSecondDelay (5000);
        Temp32 = SDIO_REG32(SdioBaseAddr,HOSTCTL2);
        
        //
        // Check 1.8V set as per step 8 of sec 3.6.1 Signal Voltage Switch 
        // Procedure of Sd host controller 3.00
        //
        if(Temp32 & SIGNALING_ENABLE_1_8V) {
            //
            // Set the clock to 400KHZ
            //
            ControllerSetClock(SdioDevInfo,Port,400000);
            
            //
            // Delay is mentioned in 3.6.1 Signal Voltage Switch Procedure 
            // of step 10.
            // 
            MicroSecondDelay (1000);
            
            //
            // Check DAT[3:0] is 1111b as per step 11 of sec 3.6.1 Signal 
            // Voltage Switch Procedure of Sd host controller 3.00
            //
            PresentState=SDIO_REG32(SdioBaseAddr,PSTATE);
        
            PresentState = PresentState >> 20;
            PresentState &= 0xf;
            
            //
            // DAT[3:0] should 1111b
            //
            if(PresentState == 0xf) {
                
                //
                // Send CMD1 with argument 0x00000080 and receive R3 
                // as per step12 of A.6.1 Bus initialization
                //
                CommandArgument = (CARD_POWERUP_STATUS_BIT| CARD_CAPACITY_STATUS | CARD_LOW_VOLTAGE_WINDOW);
                Status = MmcGetOcr(SdioDevInfo,Port,CommandArgument,&Response);
                if((!EFI_ERROR(Status)) && ((Response&(~CARD_CAPACITY_STATUS)) == (CARD_POWERUP_STATUS_BIT| CARD_LOW_VOLTAGE_OUTPUT_WINDOW))) {
                    SdioDevInfo->dOCR = Response;
                    SdioDevInfo->VoltageSwitch = TRUE;
                }
            } 
        }
        
        //
        // If switching to 1.8V failed then revert back to the Host supported 
        // higher voltage
        //
        if(SdioDevInfo->VoltageSwitch== FALSE) {
            //
            // As per Step8 of A.6.1 Bus initialization "if Device is not 
            // compliant (since it should have put itself into inactive
            // state, due to voltage incompatibility, and not respond); in
            // such a case the host must power down the bus and start its 
            // error recovery procedure. As the Host's Error Recovery method
            // uses Abort command (which is executed in tran state) and 
            // Powering down necessitate Voltage and clock reprogramming. So 
            // better to call ControllerInitEnvironment.
            //
            SDIO_REG8_AND(SdioBaseAddr,PWRCTL,~PWRCTL_POWER_ENABLE);
            Status= ControllerInitEnvironment(SdioDevInfo,Port);
            if(EFI_ERROR(Status)) {
                return Status;
            }
            
            CommandArgument = (CARD_POWERUP_STATUS_BIT| CARD_CAPACITY_STATUS | CARD_VOLTAGE_WINDOW);
            Status = MmcGetOcr(SdioDevInfo,Port,CommandArgument,&Response);
            SdioDevInfo->dOCR = Response;
            if(EFI_ERROR(Status)) {
               return Status;
            }
        }
    } 
#endif
    
    if((SdioDevInfo->dOCR & MMC_CARD_CAPACITY_BIT )) { 
        SdioDevInfo->bMode = MMC_HIGH_CAP;
    } else {
        SdioDevInfo->bMode = MMC_STAND_CAP;
    }
    SdioDevInfo->bState=CARDSTATUS_READY;
    
    //
    // Get CID from card as per  step 15-16 of Sec A.6.1 Bus initialization
    // of Jedec Electrical standard 5.01
    //
    Status= SdMmcCardGetCID(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    
    //
    // Set RCA for card as per  step 17 of Sec A.6.1 Bus initialization
    // of Jedec Electrical standard 5.01
    //
    Status= MmcCardSetRCA(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Get CID from card as per  step 18-20 of Sec A.6.1 Bus initialization
    // of Jedec Electrical standard 5.01
    //
    Status= SdMmcCardGetCSD(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Set high Speed from card as per  A.6.2 Switching to high-speed modeof 
    // Jedec Electrical standard 5.01
    //
    Status= MmcSetHighSpeed (SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    
    //
    // Set the bus width as per Section A.6.3 Changing the data bus width
    //
    Status= MmcCardBuswidth(SdioDevInfo,Port);
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    //Get the device capacity
    //
    Status= MmcMassGetCapacity(SdioDevInfo,Port);
#if HS200_MMC_SUPPORT == 1
    //
    // Set HS200 if the controller and card support it. If error device will work in normal mode
    //
    MmcCardHS200(SdioDevInfo,Port);
#endif
    

    return Status;
}

/**
    Get the MMC device capacity

    @param  SdioDevInfo 
    @param  Port 

    @retval EFI_STATUS

**/
EFI_STATUS
MmcMassGetCapacity (
    IN  SDIO_DEVICE_INFO    *SdioDevInfo,
    IN  UINT8               Port
)
{
    UINT32      CSize;
    UINT8       BlockLength;
    UINT16      CommandIndex;
    UINT32      Mult;
    EFI_STATUS  Status;
    //
    // Bit80-83 is Max. read data block length for the MMC.
    // BLOCK_LEN = 2^(READ_BL_LEN), (READ_BL_LEN < 12)
    //
    BlockLength=(SdioDevInfo->d4CSD[2] & MAX_READ_BLOCK_LENGTH) >> 16;
    SdioDevInfo->wBlockSize=(1 << BlockLength);
    
    //
    // MMC(following spec > 4.0) should have C_SIZE as 0xFFF and
    // dMaxLBA is calculated from SEC_COUNT register.
    //
    CSize = ((SdioDevInfo->d4CSD[1] & DEVICE_SIZE_1L) >> 30) | \
                ((SdioDevInfo->d4CSD[2] & DEVICE_SIZE_1H) << 2);
    if(CSize == 0xfff) {
        SdioDevInfo->dMaxLBA = SdioDevInfo->bMmcSecCount;
    } else {
        //
        // Bit47-49 is Device size multipier
        // MULT = 2^(C_SIZE_MULT+2) (C_SIZE_MULT < 8)
        // 
        //
        Mult = (SdioDevInfo->d4CSD[1] & DEVICE_SZIE_MUL_1)>>15;
        Mult= (1<<(Mult+2));
        
        //
        // BLOCKNR(number of block) = (C_SIZE+1) * MULT
        //
        SdioDevInfo->dMaxLBA = (CSize+1)*Mult;
        // If the BlockSize exceeds 512 bytes for MMC card, , then Set the Blocklength to 
		// default blocksize (512) using CMD16 (SET_BLOCK_LEN). 
        if (SdioDevInfo->wBlockSize > BLOCK_SIZE) {            
            CommandIndex = (SET_BLOCKLEN << 8) | (RESP1_TYPE);
            Status=SdMmcCommand(SdioDevInfo,Port,CommandIndex,BLOCK_SIZE);
            if(EFI_ERROR(Status)) {
                return EFI_DEVICE_ERROR;
            }
			// Modify the MaxLBA as per new BLOCK_SIZE (512)
            SdioDevInfo->dMaxLBA *= (SdioDevInfo->wBlockSize / BLOCK_SIZE);
            SdioDevInfo->wBlockSize = BLOCK_SIZE;
        }                
    }
    //
    // LBA address starts from 0 so the maximum accessible address should
    // be one less than the max.
    //
    SdioDevInfo->dMaxLBA -= 1;
    return EFI_SUCCESS;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
