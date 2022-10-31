//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioSmmController.h 3     6/06/11 4:48a Rajeshms $
//
// $Revision: 3 $
//
// $Date: 6/06/11 4:48a $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name: SdioController.h
//
// Description: SDIO controller definition
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _EFI_SDIO_CONTROLLER_H_
#define _EFI_SDIO_CONTROLLER_H_

#include <Efi.h>
#include <Token.h>
#include <AmiDxeLib.h>
#include "SdioDef.h"

#define     DMAADR      0x0                         //offset 00h     SDMA System Address
#define     BLKSZ       0x4                         //Offset 04h     Block SIZE
#define     BLKCNT      0x6                         //Offset 06h     Block Count
#define     CMDARG      0x8                         //Offset 08h     Argument
#define     XFRMODE     0xC                         //Offset 0Ch     Transfer Mode
#define     CMD         0xE                         //Offset 0Eh     Command
#define     RESP0       0x10                        //Offset 10h     Response0
#define     RESP1       0x14                        //Offset 14h     Response1
#define     RESP2       0x18                        //Offset 18h     Response2
#define     RESP3       0x1C                        //Offset 1Ch     Response3
#define     BUFDATA     0x20                        //Offset 20h     Buffer Data
#define     PSTATE      0x24                        //Offset 24h     Present State
#define     HOSTCTL     0x28                        //Offset 28h     Host Control
#define     PWRCTL      0x29                        //Offset 29h     Power Control
#define     BLKGAPCTL   0x2A                        //Offset 2Ah     Block Gap Control
#define     WAKECTL     0x2B                        //Offset 2Bh     Wakeup Control
#define     CLKCTL      0x2C                        //Offset 2Ch     Clock Control
#define     TOCTL       0x2E                        //Offset 2Eh     Timeout Control
#define     SWRST       0x2F                        //Offset 2Fh     Software Control
#define     NINTSTS     0x30                        //Offset 30h     Normal Interrupt Status
#define     ERINTSTS    0x32                        //Offset 32h     Error Interrupt Status
#define     NINTEN      0x34                        //Offset 34h     Normal Interrupt Enable
#define     ERINTEN     0x36                        //Offset 36h     Error Interrupt Enable
#define     NINTSIGEN   0x38                        //Offset 38h     Normal Interrupt Signal Enable
#define     ERINTSIGEN  0x3A                        //Offset 3Ah     Error Interrupt Signal Enable
#define     AC12ERRSTS  0x3C                        //Offset 3Ch     Auto CMD12 Error Status
#define     Reserved0   0x3C                        //Offset 3Eh
#define     HOSTCTL2    0x3E                        //Offset 3Eh     Host Control 2    
#define     CAP         0x40                        //Offset 40h     Capabilities
#define     dReserved1  0x44                        //Offset 44h
#define     MCCAP       0x48                        //offset 48h     Maximum Current Capabilities
#define     Reserved2   0x4C                        //offset 4Ch
#define     wForceEventForAC12ERSTS     0x50        //offset 50h     Force Event For Auto CMD12 Error Status
#define     wForceEventForERINTSTS      0x52        //offset 52h     Force Event For Error Interrupt Status
#define     ADMAERSTS   0x54                        //offset 54h     ADMA Error Status
#define     Reserved3   0x55                        //offset 55h
#define     Reserved4   0x56                        //offset 56h
#define     ADMAADR     0x58                        //offset 58h     ADMA System Address
#define     PresetVal   0x60                        //Offset 6f-60h                    Preset value Registerl
#define     SharedBusCtl                 0xE0       //Offset E0h     Shared Bus Control Register
#define     SLTINTSTS   0xFC                        //offset 0FCh    Slot Interrupt Status
#define     HCVER       0xFE                        //offset 0FEh    Host Controller Version


//-----------------------------------------------------------------------;
// Present State Register (Offset 24h)
//-----------------------------------------------------------------------;
#define PSTATE_CmdI                        BIT0
#define PSTATE_DCmdI                       BIT1
#define PSTATE_DLA                         BIT2
#define PSTATE_ReTuneReq                   BIT3
#define PSTATE_WTA                         BIT8
#define PSTATE_RTA                         BIT9
#define PSTATE_BUFWREN                     BIT10
#define PSTATE_BRE                         BIT11
#define PSTATE_CardInsert                  BIT16
#define PSTATE_CSS                         BIT17
#define PSTATE_CD                          BIT18
#define PSTATE_WP                          BIT19
#define PSTATE_DAT0                        BIT20           //busy
#define PSTATE_DAT1                        BIT21
#define PSTATE_DAT2                        BIT22
#define PSTATE_DAT3                        BIT23           //CS
#define PSTATE_CMDLVL                      BIT24

//----------------------------------------------------------------------------
//       Card mode
//----------------------------------------------------------------------------
#define SD_SPI_Mode                         BIT0
#define SD_Stand_CAP                        BIT1
#define SD_High_CAP                         BIT2
#define SD_SDIO_Card                        BIT3
#define MMC_Stand_CAP                       BIT4
#define MMC_High_CAP                        BIT5

#define Mmc_card_capacity_bit               0x40000000
//----------------------------------------------------------------------------
//       Card active
//----------------------------------------------------------------------------
#define NotInUSE                            0
#define InUSE                               1
#define Initilized                          2
#define Initilized_failure                  3

//;-----------------------------------------------------------------------;
//; CURRENT_STATE
//;-----------------------------------------------------------------------;
#define CardStatus_idle                     0
#define CardStatus_ready                    1
#define CardStatus_ident                    2
#define CardStatus_stby                     3
#define CardStatus_tran                     4
#define CardStatus_data                     5
#define CardStatus_rcv                      6
#define CardStatus_prg                      7
#define CardStatus_dis                      8
#define CardStatus_NoDevice                 0ffh

//-----------------------------------------------------------------------;
//Clock Control Register (Offset 02Ch)
//-----------------------------------------------------------------------;
#define CLKCTL_InternalClockEnable          BIT0
#define CLKCTL_InternalClockStable          BIT1
#define CLKCTL_ClockEnable                  BIT2
#define CLKCTL_ClockGeneratorSelect         BIT5
#define CLKCTL_Freq_Divider                 0FF00h  //BIT8~BIT15
#define CLKCTL_Freq_DividerV3               0FFC0h  //BIT6~BIT15

//-----------------------------------------------------------------------;
//Power Control Register (Offset 029h)
//-----------------------------------------------------------------------;
#define PWRCTL_PowerEnable                  BIT0
#define PWRCTL_18V                          (BIT1+BIT3)
#define PWRCTL_30V                          (BIT2+BIT3)
#define PWRCTL_33V                          (BIT1+BIT2+BIT3)

// MMC COMMAND EQUATES
//----------------------------------------------------------------------------
#define SEND_OP_COND                1       //;CMD1         R3
#define SEND_EXT_CSD                8       //;CMD8         R1
#define BUSTEST_READ                14      //;CMD14        R1
#define BUSTEST_WRITE               19      //;CMD19        R1
#define SET_BLOCK_COUNT             23      //;CMD23        R1

// SDIO COMMAND  EQUATES
//----------------------------------------------------------------------------
//Basic Commands (class 0)                 command index   response type
#define GO_IDLE_STATE               0       //;CMD0           -
#define ALL_SEND_CID                2       //;CMD2           R2
#define SEND_RELATIVE_ADDR          3       //;CMD3           R6
#define SET_DSR                     4       //;CMD4           -
#define SELECT_DESELECT_CARD        7       //;CMD7           R1b
#define SEND_IF_COND                8       //;CMD8           R7
#define SEND_CSD                    9       //;CMD9           R2
#define SEND_CID                    10      //;CMD10          R2
#define STOP_TRANSMISSION           12      //;CMD12          R1b
#define SEND_STATUS                 13      //;CMD13          R1
#define GO_INACTIVE_STATE           15      //;CMD15          -

//Block-Oriented Read Commands (class 2)
#define SET_BLOCKLEN               16      //;CMD16          R1
#define READ_SINGLE_BLOCK          17      //;CMD17          R1
#define READ_MULTIPLE_BLOCK        18      //;CMD18          R1

//Block-Oriented Write Commands (class 4)
#define WRITE_SIGLE_BLOCK          24      //;CMD24          R1
#define WRITE_MULTIPLE_BLOCK       25      //;CMD25          R1
#define PROGRAM_CSD                27      //;CMD27          R1

//Block Oriented Write Protection Commands (class 6)
#define SET_WRITE_PROT               28      //;CMD28          R1b
#define CLR_WRITE_PROT               29      //;CMD29          R1b
#define SEND_WRITE_PROT              30      //;CMD30          R1

//Erase Commands (class 5)
#define ERASE_WR_BLK_START           32      //;CMD32          R1
#define ERASE_WR_BLK_END             33      //;CMD33          R1
#define ERASE                        38      //;CMD38          R1b

//Lock Card (class 7)
#define LOCK_UNLOCK                  42      //CMD42          R1

//Application-specific Commands (class 8)
#define APP_CMD                     55      //;CMD55          R1
#define GEN_CMD                     56      //;CMD56          R1

//Application Specific Commands used/reserved by SD Memory Card
#define SET_BUS_WIDTH               6       //;ACMD6          R1
#define SD_STATUS                   13      //;ACMD13         R1
#define SEND_NUM_WR_BLOCKS          22      //;ACMD22         R1
#define SET_WR_BLK_ERASE_COUNT      23      //;ACMD23         R1
#define SD_SEND_OP_COND             41      //;ACMD41         R3
#define SET_CLR_CARD_DETECT         42      //;ACMD42         R1
#define SEND_SCR                    51      //;ACMD51         R1

//Switch Function Commands (class 10)
#define SWITCH_FUNC                 06       //;CMD6           R1

//Sd IO Device Commands.
#define IO_SEND_OP_COND             05        
#define IO_RW_DIRECT                52

//-----------------------------------------------------------------------;
// Command Register (Offset 0Eh)
//-----------------------------------------------------------------------;
#define Resp1_Type                          (BIT1+BIT3+BIT4)
#define Resp1b_Type                         (BIT0+BIT1+BIT3+BIT4)
#define Resp2_Type                          (BIT0+BIT3)
#define Resp3_Type                          BIT1
#define Resp4_Type                          BIT1
#define Resp5_Type                          (BIT1+BIT3+BIT4)
#define Resp5b_Type                         (BIT0+BIT1+BIT3+BIT4)
#define Resp6_Type                          (BIT1+BIT3+BIT4)
#define Resp7_Type                          (BIT1+BIT3+BIT4)

#define Data_Present_Select                 BIT5

#define Suspend_cmd                         BIT6
#define Resume_cmd                          BIT7
#define Abort_cmd                           (BIT6+BIT7)

//-----------------------------------------------------------------------;
//Software Reset Register (Offset 02Fh)
//-----------------------------------------------------------------------;
#define ResetAll                             BIT0
#define ResetCMD                             BIT1
#define ResetDAT                             BIT2


//-----------------------------------------------------------------------;
// Operation Conditions Register (OCR)
//-----------------------------------------------------------------------;
#define Card_VoltageWindow                  0xff8000        //;Bit15~ 23
#define Card_CapacityStatus                 BIT30
#define Card_PowerUpStatusBit               BIT31

//-----------------------------------------------------------------------;
// Normal Interrupt Status Register (Offset 30h)
//-----------------------------------------------------------------------;
#define NINTSTS_CC                          BIT0
#define NINTSTS_TC                          BIT1
#define NINTSTS_BGE                         BIT2
#define NINTSTS_DMAINT                      BIT3
#define NINTSTS_BWR                         BIT4
#define NINTSTS_BRR                         BIT5
#define NINTSTS_CIN                         BIT6
#define NINTSTS_CRM                         BIT7
#define NINTSTS_CI                          BIT8
#define NINTSTS_INT_A                       BIT9
#define NINTSTS_INT_B                       BIT10
#define NINTSTS_INT_C                       BIT11
#define NINTSTS_ReTunEvt                    BIT12
#define NINTSTS_EI                          BIT15

//-----------------------------------------------------------------------;
// Capabilities Register (Offset 40h)
//-----------------------------------------------------------------------;
#define Timeout_Clock_Frequency             (BIT0+BIT1+BIT2+BIT3+BIT4+BIT5)
#define Timeout_Clock_Unit                  BIT7
#define Base_Clock_Frequency_For_SD_Clock   (BIT8+BIT9+BIT10+BIT11+BIT12+BIT13)
#define Base_Clock_Frequency_For_SD_ClockV3   (BIT8+BIT9+BIT10+BIT11+BIT12+BIT13+BIT14+BIT15)

#define Max_Block_Length                    (BIT16+BIT17)
#define Embedded_Device_Support             BIT18
#define ADMA2_Support                       BIT19
#define High_Speed_Support                  BIT21
#define SDMA_Support                        BIT22
#define SuspendResume_Support               BIT23
#define Voltage_Support_33                  BIT24
#define Voltage_Support_30                  BIT25
#define Voltage_Support_18                  BIT26
#define Bit64_System_Bus_Support            BIT28
#define Asyn_Int_Support                    BIT29
#define Slot_Type                           (BIT32+BIT31)
#define SDR50_Support                       BIT32
#define SDR104_Support                      BIT33
#define DDR50_Support                       BIT34
#define Drvier_Type_A_Support               BIT36
#define Drvier_Type_C_Support               BIT37
#define Drvier_Type_D_Support               BIT38
#define TimerCount_ReTune                   (BIT40+BIT41+BIT42+BIT43)
#define Tune_SDR50                          BIT45
#define ReTuningModes                       (BIT46+BIT47)
#define Clock_Multiplier                    (BIT48+BIT49+BIT50+BIT51+BIT52+BIT53+BIT54+BIT55)


//-----------------------------------------------------------------------;
// Host Control Register (Offset 28h)
//-----------------------------------------------------------------------;
#define LED_Control                         BIT0
#define DATA_Transfer_4BitMode              BIT1
#define High_Speed_Enable                   BIT2
#define DAM_Select                          (BIT3+BIT4)
#define Card_Detect_Transfer_Level          BIT6
#define Card_Detect_Signal_Selection        BIT7

//-----------------------------------------------------------------------;
// Host Control 2 Register (Offset 3Eh)
//-----------------------------------------------------------------------;

#define UHS_Mode_Select                     (BIT0+BIT2+BIT1)
#define Signaling_Enable_1_8V               BIT3
#define Driver_Strength_Select              (BIT5+BIT4)
#define Execute_Tuning                      BIT6
#define Sampling_Clock_Select               BIT7
#define Asynchronous_Interrupt_Enable       BIT14
#define Preset_Value_Enable                 BIT15


//-----------------------------------------------------------------------;
// Card-Specific Data Register (CSR) the response3 (119:0)
//-----------------------------------------------------------------------;
//0: CSD_ver1.0   1:CSD_ver2.0 (bit30~31offset 96bit)
#define CSD_Ver20                           BIT31
#define partial_blocks_for_write_allowed    BIT21                           //; offset 0bit
#define max_write_block_length              (BIT22+BIT23+BIT24+BIT25)       //; offset 0bit
#define partial_blocks_for_read_allowed     BIT15                           ///; bit 79(offset 64bit)
#define max_read_block_length               (BIT16+BIT17+BIT18+BIT19)       //;bit 80 81 82 83(offset 64bit)

//CSD Version 1.0
#define device_size_1h						0x3ff							//;bit 64~73 ( bit0~9 offset 64bit )
#define device_size_1l						0xc0000000						//;bit 62~63 ( bit30~31 offset 48bit )
#define device_size_1                       0x3ffc000                       //;bit 62~73 ( bit14~25 offset 48bit )
#define device_szie_mul_1                   BIT15+BIT16+BIT17               //;bit 47~49    ( bit15~17 offset 32bit )
//CSD Version 2.0
#define device_size_2                       0x3fffff                        //;bit 48~69 ( bit0~22  offset 48bit )

//-----------------------------------------------------------------------;
// Card Status
//-----------------------------------------------------------------------;
#define AKE_SEQ_ERROR                       BIT3
#define APP_CMD_STS                         BIT5
#define READY_FOR_DATA                      BIT8
#define CURRENT_STATE                       (BIT9+BIT10+BIT11+BIT12)
#define ERASE_RESET                         BIT13
#define CARD_ECC_DISABLE                    BIT14
#define WP_ERASE_SKIP                       BIT15
#define CID_CSD_OVERWRITE                   BIT16
#define OVERRUN                             BIT17
#define UNDERRUN                            BIT18
#define ERROR                               BIT19
#define CC_ERROR                            BIT20
#define CARD_ECC_FAILED                     BIT21
#define ILLEGAL_COMMAND                     BIT22
#define COM_CRC_ERROR                       BIT23
#define LOCK_UNLOCK_FAILED                  BIT24
#define CARD_IS_LOCK                        BIT25
#define WP_VIOLATION                        BIT26
#define ERASE_PARAM                         BIT27
#define ERASE_SEQ_ERROR                     BIT28
#define BLOCK_LEN_ERROR                     BIT29
#define ADDRESS_ERROR                       BIT30
#define OUT_OF_RANGE                        BIT31

//;-----------------------------------------------------------------------;
//; Transfer Mode Register (Offset 0Ch)
//;-----------------------------------------------------------------------;
#define DMA_Enable                          BIT0
#define Block_Count_Enable                  BIT1
#define Auto_CMD12_Enable                   BIT2
#define Auto_CMD12_EnableV3                   (BIT2+BIT3)
#define Data_Transfer_Card2Host             BIT4
#define Multi_Block_Select                  BIT5
#define CMC_COMP_ATA                        BIT6

//;-----------------------------------------------------------------------;
//; Block Size Register (Offset 04h)
//;-----------------------------------------------------------------------;
#define BLKSize_Size                        0x0fff   //;Bit0 ~ Bit11
#define BLKSize_DAMBufBoundary              0x7000   //;Bit12~ Bit14

#define COMMON_CIS_POINTER                  0x9

//
// IO Card CMD 52 Status
//
#define CIS_AREA_START                      0x1000
#define CIS_AREA_END                        0x17FFF

#define IO_OUT_OF_RANGE                     0x1
#define IO_INVALID_FUNCTION_NO              0x2
#define IO_SDIO_DEVICE_ERROR                0x8
#define IO_ILLEGAL_COMMAND                  0x40
#define IO_COM_CRC_ERROR                    0x80

#define CISTPL_MANFID                       0x20
#define CISTPL_END                          0xFF
#define CISTPL_NULL                         0x00

//-----------------------------------------------------------------------;
//Host Controller Version
//-----------------------------------------------------------------------;
#define HOST_CONTROLLER_Ver3                0x2

//-----------------------------------------------------------------------;
// MMC BIT BUS WIDTH
//-----------------------------------------------------------------------;
#define MMC_1_BIT_BUS_WIDTH                 1
#define MMC_4_BIT_BUS_WIDTH                 4
#define MMC_8_BIT_BUS_WIDTH                 8


#define MAX_SDIO_DEVICES                    8
#define MAXIMUM_SIZE_FOR_FLOPPY_EMULATION   530
#define MAX_LBA_FOR_FLOPPY_EMULATION        0x109000    // 530MB

#define TWO_GB                              0x80000000
#define THIRTY_TWO_GB                       0x800000000



//-----------------------------------------------------------------------;
// SDIO_DEVICE_INFO
//-----------------------------------------------------------------------;
typedef struct _SDIO_DEVICE_INFO{
    BOOLEAN                             DevEntryUsed;
    UINT8                               DeviceAddress;
    UINT64                              SdioBaseAddress;
    UINT8                               PortNumber;
    UINT8                               DeviceState;
    BOOLEAN                             IODevice;
    UINT8                               bMode;
    UINT8                               bState;
    UINT8                               bActive;
    UINT32                              dOCR;
    UINT32                              d4CID[4];
    UINT16                              wRCA;
    UINT32                              d4CSD[4];
    BOOLEAN                             bWrite_Bl_Partial;
    UINT8                               bWrite_Bl_Len;
    BOOLEAN                             bRead_Bl_Partial;
    UINT8                               bRead_Bl_Len;
    UINT8                               d2SCR[8];
    UINT16                              wEmulationOption;
    UINT8                               bHiddenSectors;
    UINT8                               NumHeads;
    UINT8                               LBANumHeads;
    UINT16                              NumCylinders;
    UINT16                              LBANumCyls;
    UINT8                               NumSectors;
    UINT8                               LBANumSectors;
    UINT32                              dMaxLBA;
    UINT16                              wBlockSize;
    UINT8                               bStorageType;
    UINT8                               bEmuType;
    UINT8                               PNM[27];
    UINT8                               SdIOManufactureId[SDIO_MANUFACTUREID_LENGTH];
    UINT8                               bCsdStruct;
    UINT8                               bSpecVers;
    UINT8                               bTranSpeed;
    UINT8                               bMmcCardType;
    UINT32                              bMmcSecCount;
    UINT8                               MmcBusWidth;
    UINT8                               bHostControllerVersion;
}SDIO_DEVICE_INFO;

//-----------------------------------------------------------------------;
// SDIO_GLOBAL_DATA
//-----------------------------------------------------------------------;
typedef struct{
    UINT32                              TransferBufferAddress;
    UINT8                               SDIOMassEmulationOptionTable[MAX_SDIO_DEVICES];
    SDIO_DEVICE_INFO                    SdioDev[MAX_SDIO_DEVICES];
}SDIO_GLOBAL_DATA;



EFI_STATUS
ConfigureMassDevice_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
CheckDevicePresence_Controller (
    IN SDIO_DEVICE_INFO                *SdioDevInfo,
    IN UINT8                           Port
);

EFI_STATUS
SDCard_InitEnvironment (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SdCard_GetOCR (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);


EFI_STATUS
SDCard_SetClock (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);


EFI_STATUS
SDCard_GetCID (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);


EFI_STATUS
SDCard_GetRCA (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);


EFI_STATUS
SDCard_GetCSD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_Select (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_Buswidth (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_CommandCMD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port,
    IN UINT16                         CommandIndex,
    IN UINT32                         CommandArgument
);

EFI_STATUS
SDCard_WaitCMDComplete (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_SetTimeout (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_GetStatus (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_GetCSD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_GetExtCSD (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_GetSCR (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_DMAINTHandle (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port,
    IN UINT32                         **BufferAddress,
    IN UINT32                         *DmaAddress,
    IN UINT32                         *NumBlks,
    IN BOOLEAN                        SdioRead
);

EFI_STATUS
SDCard_XferWdata2Buff (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port,
    IN UINT32                         **BufferAddress,
    IN UINT32                         *DmaAddress,
    IN UINT32                         *NumBlks
);

EFI_STATUS
SDCard_BRINTHandle (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port,
    IN UINT32                         **TransferAddress
);

EFI_STATUS
SDCard_RestoreStatus (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDCard_WaitXferComplete (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port,
    IN UINT32                         **BufferAddress,
    IN UINT32                         *DmaAddress,
    IN UINT32                         *TotalSec,
    IN BOOLEAN                        SdioRead
);

EFI_STATUS
SDCard_DeviceIdentify (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SDIOMassGetCapacity (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                           Port
);

EFI_STATUS
SDIOMassGetPNMWithDeviceClass (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                          Port
);

EFI_STATUS
SdioDetectDevice_Controller (
    IN SDIO_DEVICE_INFO                   *SdioDevInfo,
    IN UINT8                               Port
);

EFI_STATUS
SDIOAPI_ReadCard_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
);

EFI_STATUS
SDCard_RWCMD_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress,
    BOOLEAN                                 ReadWrite
);

EFI_STATUS
SDCard_XferRemainderSector_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN UINT32                               *BufferAddress,
    IN UINT32                               *DmaAddress,
    IN UINT32                               NumBlks
);

EFI_STATUS
SDIOAPI_WriteCard_Controller (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
);

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
);

EFI_STATUS
GetDeviceInformation (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
);



EFI_STATUS
SDIOAPI_ResetCard (
    IN SDIO_DEVICE_INFO             *SdioDevInfo,
    IN UINT8                        Port
);

EFI_STATUS
SDIO_ResetAll (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                                Port
);

VOID
SDIOMassIdentifyDeviceType(
    IN SDIO_DEVICE_INFO             *SdioDevInfo,
    IN UINT8                        Port
);

EFI_STATUS
SdCard_CheckSdIoDevice (
    IN SDIO_DEVICE_INFO               *SdioDevInfo,
    IN UINT8                           Port
);

//-----------------------------------------------------------------------;
//MMIO Access
//-----------------------------------------------------------------------;

#define     MmAddress( BaseAddr, Register ) \
            ((UINTN)(BaseAddr) + \
            (UINTN)(Register) \
             )

#define     Mm32Ptr( BaseAddr, Register ) \
            ((volatile UINT32 *)MmAddress (BaseAddr, Register ))

#define     Mm16Ptr( BaseAddr, Register ) \
            ((volatile UINT16 *)MmAddress (BaseAddr, Register ))

#define     Mm8Ptr( BaseAddr, Register ) \
            ((volatile UINT8 *)MmAddress (BaseAddr, Register ))

//-----------------------------------------------------------------------;
// SDIO Generic
//-----------------------------------------------------------------------;

#define     SDIO_REG8( BaseAddr, Register ) \
            (*Mm8Ptr ((BaseAddr), (Register)))

#define     SDIO_REG8_OR( BaseAddr, Register, OrData) \
            (SDIO_REG8 ((BaseAddr), (Register))) |= ((UINT8) (OrData))

#define     SDIO_REG16( BaseAddr, Register ) \
            (*Mm16Ptr ((BaseAddr), (Register)))

#define     SDIO_WRITE_REG8( BaseAddr, Register, Data ) \
            (SDIO_REG8 ((BaseAddr), (Register))) = ((UINT8) (Data))

#define     SDIO_WRITE_REG16( BaseAddr, Register, Data ) \
            (SDIO_REG16 ((BaseAddr), (Register))) = ((UINT16) (Data))

#define     SDIO_WRITE_REG32( BaseAddr, Register, Data ) \
            (SDIO_REG32 ((BaseAddr), (Register))) = ((UINT32) (Data))

#define     SDIO_REG16_OR( BaseAddr, Register, OrData) \
            (SDIO_REG16 ((BaseAddr), (Register))) |= ((UINT16) (OrData))

#define     SDIO_REG32( BaseAddr, Register ) \
            (*Mm32Ptr ((BaseAddr), (Register)))

#define     SDIO_REG32_OR( BaseAddr, Register, OrData) \
            (SDIO_REG32 ((BaseAddr), (Register))) = (SDIO_REG32 ((BaseAddr), (Register))) | ((UINT32) (OrData))

#define     SDIO_REG8_AND( BaseAddr, Register, AndData) \
            (SDIO_REG8 ((BaseAddr), (Register))) = (SDIO_REG8 ((BaseAddr), (Register))) & ((UINT8) (AndData))

#define     SDIO_REG16_AND( BaseAddr, Register, AndData) \
            (SDIO_REG16 ((BaseAddr), (Register))) &= ((UINT16) (AndData))

#define     SDIO_REG32_AND( BaseAddr, Register, AndData) \
            (SDIO_REG32 ((BaseAddr), (Register))) = (SDIO_REG32 ((BaseAddr), (Register))) & ((UINT32) (AndData))

#define     SDIO_REG8_AND_OR( BaseAddr, Register, AndData, OrData) \
            (SDIO_REG8 ((BaseAddr), (Register)) = \
                (((SDIO_REG8 ((BaseAddr), (Register))) & ((UINT8) (AndData))) | ((UINT8) (OrData))))

#define     SDIO_REG16_AND_OR( BaseAddr, Register, AndData, OrData) \
            (SDIO_REG16 ((BaseAddr), (Register)) = \
                (((SDIO_REG16 ((BaseAddr), (Register))) & ((UINT16) AndData)) | ((UINT16) (OrData))))

#define     SDIO_REG32_AND_OR( BaseAddr, Register,AndData,  OrData) \
            (SDIO_REG32 ((BaseAddr), (Register)) = \
                (((SDIO_REG32 ((BaseAddr), (Register))) & ((UINT32) (AndData))) | ((UINT32) (OrData))))


#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
