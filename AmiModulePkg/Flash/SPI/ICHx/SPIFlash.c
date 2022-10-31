//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    FlashWrite.c
//
// Description: Flash update routines
//
//<AMI_FHDR_END>
//**********************************************************************
//----------------------------------------------------------------------------
// Includes
#include <Efi.h>
#include <AmiDxeLib.h>
#include <Library/TimerLib.h>
#include "FlashPart.h"
#include "SpiFlash.h"
#include "Token.h"
//----------------------------------------------------------------------------
// Local defines for transaction types
// ICH8/9/10 SPI register defines.
#define     SPI_HSTS                    0x04    // Hardware Sequencing Flash Status Register (16bits)
#define     SPI_STS                     0x90    //  SPI Status
#define     SPI_CTL                     0x91    //  SPI Control
#define     SPI_ADR                     0x08    //  SPI Address
#define     SPI_DAT0                    0x10    //  SPI Data 0
#define     SPI_PREOP                   0x94    //  Prefix Opcode Configuration
#define     SPI_OPTYPE                  0x96    //  Opcode Type Configuration
#define     SPI_OPMENU                  0x98    //  Opcode Menu Configuration
//  SPI default opcode slots
#define     SPI_OPCODE_WRITE_INDEX      0x0
#define     SPI_OPCODE_READ_INDEX       0x1
#define     SPI_OPCODE_ERASE_INDEX      0x2
#define     SPI_OPCODE_READ_S_INDEX     0x3
#define     SPI_OPCODE_READ_ID_INDEX    0x4
#define     SPI_OPCODE_WRITE_S_INDEX    0x5
#define     SPI_OPCODE_WRITE_S_E_INDEX  0x6
#define     SPI_OPCODE_WRITE_E_INDEX    0x7
#define     SPI_OPCODE_AAI_INDEX        0x6
#define     SPI_OPCODE_WRITE_D_INDEX    0x7
#define     SPI_PREFIX_WRITE_S_EN       0x1
#define     SPI_PREFIX_WRITE_EN         0x0
#define     SPI_MAX_DATA_TRANSFER       0x40
#define     ICHX_FDOC                   0xb0    // Flash Descriptor Observability Control Register
#define     ICHX_FDOD                   0xb4    // Flash Descriptor Observability Data Register
#define     FLASH_VALID_SIGNATURE       0x0ff0a55a
#define     NO_ADDRESS_UPDATE           0
#ifndef     SPI_OPCODE_TYPE_READ_NO_ADDRESS
#define     SPI_OPCODE_TYPE_READ_NO_ADDRESS     0x0
#define     SPI_OPCODE_TYPE_WRITE_NO_ADDRESS    0x1
#define     SPI_OPCODE_TYPE_READ_WITH_ADDRESS   0x2
#define     SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS  0x3
#endif
//----------------------------------------------------------------------------
// Module level global data
extern UINT16       gFlashId;
extern FLASH_PART   *FlashAPI;
//----------------------------------------------------------------------------
// Function Externs
extern
VOID
SpiChipsetVirtualFixup  (
    IN EFI_RUNTIME_SERVICES *pRS
);
//----------------------------------------------------------------------------
// Local prototypes
VOID
CommonSpiEraseCommand   (
    volatile UINT8          *pBlockAddress
);
VOID
CommonSpiProgramCommand (
    volatile UINT8          *pByteAddress,
    UINT8                   *Byte,
    UINT32                  *Length
);
VOID
CommonSpiReadCommand    (
    volatile UINT8          *pByteAddress,
    UINT8                   *Byte,
    UINT32                  *Length
);
BOOLEAN
CommonSpiIsEraseCompleted   (
    volatile UINT8          *pBlockAddress,
    BOOLEAN                 *pError,
    UINTN                   *pStatus
);
BOOLEAN
CommonSpiIsProgramCompleted (
    volatile UINT8          *pByteAddress,
    UINT8                   *Byte,
    UINT32                  Length,
    BOOLEAN                 *pError,
    UINTN                   *pStatus
);
VOID
CommonSpiBlockWriteEnable   (
    volatile UINT8          *pBlockAddress
);
VOID
CommonSpiBlockWriteDisable  (
    volatile UINT8          *pBlockAddress
);
VOID
CommonSpiDeviceWriteEnable  (
    VOID
);
VOID
CommonSpiDeviceWriteDisable (
    VOID
);
VOID
CommonSpiDeviceVirtualFixup (
    EFI_RUNTIME_SERVICES    *pRS
);
//----------------------------------------------------------------------------
// Local Variables
FLASH_PART mCommonSpiFlash ={
    CommonSpiReadCommand,
    CommonSpiEraseCommand,
    CommonSpiProgramCommand,
    CommonSpiIsEraseCompleted,
    CommonSpiIsProgramCompleted,
    CommonSpiBlockWriteEnable,
    CommonSpiBlockWriteDisable,
    CommonSpiDeviceWriteEnable,
    CommonSpiDeviceWriteDisable,
    CommonSpiDeviceVirtualFixup,
    1,                      // default value, should be changed in Init function
    SECTOR_SIZE_4KB
};
EX_FLASH_PART mExFlashPart = {
      {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},0,0},
      FLASH_SIZE,           // flash size, should be changed in Init function
      0,                    // flash part id, should be changed in Init function
      0                     // flash part string, should be changed in
                            // Init function
};
#if defined SPI_BASE && SPI_BASE != 0
volatile UINT8  *gSPIBASE = (UINT8*)(UINTN)SPI_BASE;
#else
volatile UINT8  *gSPIBASE = (UINT8*)(UINTN)(SB_RCBA + SPI_BASE_ADDRESS);
#endif
UINT8           gbDeviceVirtual = 0;
UINT8           gbDeviceWriteEnabled = 0;
//----------------------------------------------------------------------------
// Function Definitions

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IoDelay
//
// Description:
//
// Input:
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
SpiIoDelay (VOID)
{
//-    UINT8               bTimeout;
//-    for ( bTimeout = 0; bTimeout < 33; bTimeout++ ) {
//-        IoWrite8( 0xEB, 0x55 );
//-        IoWrite8( 0xEB, 0xAA );
//-    }
    MicroSecondDelay(10);
    return ;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   WaitForSpiCycleDone
//
// Description:
//
// Input:
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID WaitForSpiCycleDone (UINT8 HwCycle, UINT8 SwCycle)
{
    UINT32              dTimeout;
    UINT8               bCyclyDone;

    if (HwCycle) {
        for (dTimeout = 0, bCyclyDone = 0; dTimeout < 0x4000000; dTimeout++) {
            bCyclyDone = *(volatile UINT8*)( gSPIBASE + SPI_HSTS );
            if ((bCyclyDone & BIT00) || ((bCyclyDone & BIT05) == 0)) break;
        }
        // write BIT2 to clear CycleDone status
        *(volatile UINT8*)( gSPIBASE + SPI_HSTS ) = BIT00 + BIT01 + BIT02;
    }
    if (SwCycle) {
        for (dTimeout = 0, bCyclyDone = 0; dTimeout < 0x4000000; dTimeout++) {
            bCyclyDone = *(volatile UINT8*)( gSPIBASE + SPI_STS );
            if ((bCyclyDone & BIT02) || ((bCyclyDone & BIT00) == 0)) break;
        }
        // write BIT2 to clear CycleDone status
        *(volatile UINT8*)( gSPIBASE + SPI_STS ) = BIT02 + BIT03 + BIT04;
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SpiWriteDisable
//
// Description: This procedure issuess SPI Write Disable if AAIWordProgram. 
//
// Input:       None.
//
// Output:      None.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SpiWriteDisable (VOID)
{
    // Opcode menu slot 7 is configured as "Write Disable" if AAIWordProgram.
    *(volatile UINT16*)(gSPIBASE + SPI_CTL) = \
                                ( SPI_OPCODE_WRITE_D_INDEX << 4) + BIT01;
    // Wait for spi software sequence cycle completed.
    WaitForSpiCycleDone (FALSE, TRUE);
    return ;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CheckAaiWordProram
//
// Description: This procedure checks whether issues the AAIWordProgram command. 
//
// Input:       dAddr   - Start address to be written.
//              dLength - Number of bytes to be written.
//
// Output:      TRUE    - Yes
//              FALSE   - No
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
CheckAaiWordProram (
    IN UINT32               dAddr,
    IN UINT32               dLength
)
{
    if ((mExFlashPart.AAIWordProgram != 0) && !(dAddr & 1) && (dLength >= 2))
        return TRUE;
    return FALSE;    
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CommonSpiReadStatus
//
// Description:
//
// Input:       None.
//
// Output:      Status Register which is read from SPI flash.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
CommonSpiReadStatus   (
    IN UINT32               dUpdateAddr
)
{
    UINT16  wSpiCmd;

    if( dUpdateAddr ) *(volatile UINT32*)( gSPIBASE + SPI_ADR ) = dUpdateAddr;
    // Opcode menu slot 3 is configured as "Read Status Register"
    wSpiCmd = SPI_OPCODE_READ_S_INDEX << 4;
    // indicate that data phase is required
    wSpiCmd += (1 << 14);
    // Set BIT1 (Go)
    *(volatile UINT16*)( gSPIBASE + SPI_CTL ) =  wSpiCmd + BIT01;
    // Wait for spi software sequence cycle completed.
    WaitForSpiCycleDone (FALSE, TRUE);
    // return status register.
    return ( *(volatile UINT8*)( gSPIBASE + SPI_DAT0 ) );
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   WaitForWriteOperationCompleted
//
// Description:
//
// Input:       None.
//
// Output:      None.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
WaitForWriteOperationCompleted (VOID)
{
    UINT16              wWaitStsRetry;
    UINT8               bStatus;

    for( wWaitStsRetry = 0; wWaitStsRetry < 0xFFFF; wWaitStsRetry++ ) {
        // read flash status register.
        bStatus = CommonSpiReadStatus( NO_ADDRESS_UPDATE );
        // Is operation busy ?
        if( !( bStatus & 0x1 ) ) break;
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CommonSpiWriteStatus
//
// Description: Routine for Write SPI Status Register.
//
// Input:       None.
//
// Output:      Status Register which is read from SPI flash.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiWriteStatus    (
    IN UINT8                bWriteData,
    IN UINT8                bOpcodeIndex,
    IN UINT8                bIsDataPhase,
    IN UINT8                bPrefixOp,
    IN UINT32               dSectorAddress
)
{
    UINT16  wSpiCmd;

    // Wait for spi hardware/software sequence cycle completed.
    WaitForSpiCycleDone (TRUE, TRUE);
    *(volatile UINT8*)( gSPIBASE + SPI_DAT0 ) = bWriteData;
    *(volatile UINT32*)( gSPIBASE + SPI_ADR ) = dSectorAddress;
    // Opcode menu slot 3 is configured as "Read Status Register"
    wSpiCmd = bOpcodeIndex << 4;
    // indicate that data phase is required
    wSpiCmd += ( bIsDataPhase << 14 );
    // BIT3(Preop 1)
    wSpiCmd += ( bPrefixOp << 3 );
    // Set BIT1 (Go), BIT2(Atomic w/ Prefix),
    *(volatile UINT16*)( gSPIBASE + SPI_CTL ) =  wSpiCmd + BIT01 + BIT02;
    // Wait for spi software sequence cycle completed.
    WaitForSpiCycleDone (FALSE, TRUE);
    // wait for SPI flash operation completed.
    WaitForWriteOperationCompleted();
    // return status register.
    return ;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CommonSpiReadByte
//
// Description:
//
// Input:       dByteAddress    Address that need to be read.
//
// Output:      BYTE            Value which is read from SPI flash.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
CommonSpiReadByte   (
    IN UINT32               dByteAddress
)
{
    UINT8           bData;
    UINT32          dLength = sizeof(UINT8);
    CommonSpiReadCommand((volatile UINT8*)dByteAddress, &bData, &dLength);
    return bData;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CommonConvertSpiAddress
//
// Description:
//
// Input:
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
CommonConvertSpiAddress   (
    IN volatile UINT8       *pAddress
)
{
    UINT32              dIchSpiFDOD;
    static UINT32       gBiosRegionBase = 0;

    // if flash identified, here checks the BIOS region of Flash
    // Descriptor Table.
    if ( mExFlashPart.FlashVenDevId ) {
        if ( !gBiosRegionBase ) {
            *(volatile UINT32*)( gSPIBASE + ICHX_FDOC ) = 0;
            dIchSpiFDOD = *(volatile UINT32*)( gSPIBASE + ICHX_FDOD );
            if ( dIchSpiFDOD == FLASH_VALID_SIGNATURE ) {
                *(volatile UINT32*)( gSPIBASE + ICHX_FDOC ) = ( BIT13 + BIT02 );
                do {
                    dIchSpiFDOD = *(volatile UINT32*)( gSPIBASE + ICHX_FDOD );
                } while( dIchSpiFDOD == FLASH_VALID_SIGNATURE );
                gBiosRegionBase = ( ( (dIchSpiFDOD >> 16) + 1 ) << 12 );
            }
            else gBiosRegionBase = mExFlashPart.FlashCapacity;
        }
    }
    if ( gbDeviceVirtual ) {
        // pAddress - offset from Flash Device Base.
        pAddress -= FlashDeviceBase;
        // pAddress - 32bit memory mapping address.
        pAddress += (0xFFFFFFFF - FLASH_SIZE) + 1;
    }
    // pAddress - physical address in flash.
    pAddress += gBiosRegionBase;
    return ((UINT32)pAddress);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitializeSpiEnvironment
//
// Description:
//
// Input:
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
InitializeSpiEnvironment (
    IN OUT  FLASH_INFO      *FlashInfo
)
{
    UINT32	Buffer32;	// EIP137034

    //Program first DWORD of opcode commands
    *((volatile UINT32*)( gSPIBASE + R_RCRB_SPI_OPMENU + 0 )) = (UINT32)
        // Write Byte
        ( (FlashInfo->Write.Opcode << (SPI_OPCODE_WRITE_INDEX * 8)) | \
        // Read Data
          (FlashInfo->Read.Opcode << (SPI_OPCODE_READ_INDEX * 8)) | \
        // Erase 64k Sector
          (FlashInfo->Erase.Opcode << (SPI_OPCODE_ERASE_INDEX * 8)) | \
        // Read Device Status Reg
          (FlashInfo->ReadStatus.Opcode << (SPI_OPCODE_READ_S_INDEX * 8)) );

    //Program second DWORD of Opcode commands
    *((volatile UINT32*)( gSPIBASE + R_RCRB_SPI_OPMENU + 4 )) = (UINT32)
        // Read device ID
      ((FlashInfo->ReadId.Opcode << ((SPI_OPCODE_READ_ID_INDEX - 4) * 8)) | \
        // Write Status Register
       (FlashInfo->WriteStatus.Opcode << \
                                    ((SPI_OPCODE_WRITE_S_INDEX - 4) * 8)) | \
        // Write Status Enable Register
       (FlashInfo->WriteStatusEnable.Opcode << \
                                    ((SPI_OPCODE_WRITE_S_E_INDEX - 4) * 8)));

    //Program opcode types
    *((volatile UINT16*)( gSPIBASE + R_RCRB_SPI_OPTYPE )) = (UINT16)
        // write with address.
        (FlashInfo->Write.OpcodeType << (SPI_OPCODE_WRITE_INDEX * 2) | \
        // read with address.
         FlashInfo->Read.OpcodeType << (SPI_OPCODE_READ_INDEX * 2) | \
        // write with address.
         FlashInfo->Erase.OpcodeType << (SPI_OPCODE_ERASE_INDEX * 2) | \
        // read w/o no adress.
         FlashInfo->ReadStatus.OpcodeType << (SPI_OPCODE_READ_S_INDEX * 2) | \
        // read with address.
         FlashInfo->ReadId.OpcodeType << (SPI_OPCODE_READ_ID_INDEX * 2) | \
        // write w/o address.
         FlashInfo->WriteStatus.OpcodeType << (SPI_OPCODE_WRITE_S_INDEX * 2) | \
        // write w/o address.
         FlashInfo->WriteStatusEnable.OpcodeType << \
                                    (SPI_OPCODE_WRITE_S_E_INDEX * 2));

    //set up the prefix opcodes for commands
    *((volatile UINT16*)( gSPIBASE + R_RCRB_SPI_PREOP )) = (UINT16)
        ( ( FlashInfo->WriteStatusEnable.Opcode << 8 ) | \
                                    ( FlashInfo->WriteEnable.Opcode ) );

    //set up Program Opcode and Optype if AAIWordProgram.
    if (mExFlashPart.AAIWordProgram != 0) {
        UINT8  bOpType = 0;
        *((volatile UINT16*)(gSPIBASE + R_RCRB_SPI_OPMENU + 6)) = 0x4ad; 
        bOpType = *((volatile UINT8*)(gSPIBASE + R_RCRB_SPI_OPTYPE + 1));
        bOpType = ((bOpType & 0xf) | \
                 (SPI_OPCODE_TYPE_WRITE_NO_ADDRESS << 6) | \
                 (SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS << 4));
        *((volatile UINT8*)(gSPIBASE + R_RCRB_SPI_OPTYPE + 1)) = bOpType;
    }
    
#if defined FAST_READ_SUPPORT && FAST_READ_SUPPORT != 0
#if defined SPI_CYCLE_FREQUENCY && SPI_CYCLE_FREQUENCY != 0
    // Here to sets frequency to use for all SPI software sequencing cycles.
    {
        UINT32  SwStatusRegr = 0;
        SwStatusRegr = *((volatile UINT32*)(gSPIBASE + 0x90));
        SwStatusRegr &= ~ (BIT26 | BIT25 | BIT24);
        SwStatusRegr |= (SPI_CYCLE_FREQUENCY << 24);
        *((volatile UINT32*)(gSPIBASE + 0x90)) = SwStatusRegr;
    }
#endif    
#endif    
// EIP137034 >>
    //
    // Program SPI Cycle Frequency bits in R_PCH_SPI_SSFCS 
    // (Software Sequencing Flash Control Status Register)
    //
    Buffer32 = *((volatile UINT32*)(gSPIBASE + SPI_STS));
    Buffer32 &= ~ (BIT26 | BIT25 | BIT24);
#ifdef PROGRAM_SPI_CYCLE_FREQ
    Buffer32 |= PROGRAM_SPI_CYCLE_FREQ << 24;
#else
    Buffer32 |= BIT26;
#endif    
    *((volatile UINT32*)(gSPIBASE + SPI_STS)) = Buffer32;
// EIP137034 +<<
    
    return ;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SerialFlashDiscoveryForQuadRead
//
// Description: This procedure checks SPI Quad Read support through SFDP.
//
// Input:       None.
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SerialFlashDiscoveryForQuadRead (VOID)
{
#if defined SPI_INITIALIZE_WITH_VSCC && SPI_INITIALIZE_WITH_VSCC == 1
#if defined SPI_QUAD_MODE_IDENTIFICATION && SPI_QUAD_MODE_IDENTIFICATION == 1 //EIP160504
    UINT8               SfdpData[256]; 
    UINT16              wSpiCmd, wOffset, j;
    UINT32              Index;
    // Serial Flash Discoverable Parameters Command "0x5A".
  	*(volatile UINT8*)(gSPIBASE + SPI_OPMENU + 7) = 0x5a;
  	// Do nothing if SPI Registers is locked.
  	if (*(volatile UINT8*)(gSPIBASE + SPI_OPMENU + 7) != 0x5a) 
  	    return EFI_WRITE_PROTECTED;
  	// Update SPI OpMenu[7] to "READ WITH ADDRESS".
  	SfdpData[0] = *(volatile UINT8*)(gSPIBASE + SPI_OPTYPE + 1);
    *(volatile UINT8*)(gSPIBASE + SPI_OPTYPE + 1) = (SfdpData[0] & 0x3f) | 0x80;
   	MemSet(SfdpData, 256, 0);
    for (Index = 0; Index < 256; Index += 64) {
    	// Set SPI read-address = 0, 64, 128, 256 
    	*(volatile UINT32*)(gSPIBASE + SPI_ADR) = Index;
    	// set opcode for "5A"
    	wSpiCmd = SPI_OPCODE_WRITE_E_INDEX << 4;
    	// set transaction = 64 bytes
    	wSpiCmd += ((64 - 1) << 8);
    	// indicate that data phase is required
    	wSpiCmd += (1 << 14);
    	// Go (BIT1)
    	*(volatile UINT16*)(gSPIBASE + SPI_CTL) =  wSpiCmd + BIT01;
        // Wait for spi software sequence cycle completed.
    	WaitForSpiCycleDone (FALSE, TRUE);
    	SpiIoDelay();
    	for (j = 0; j < 64; j++)
    	    *(SfdpData + Index + j) = *(volatile UINT8*)(gSPIBASE + SPI_DAT0 + j);
  	    // Serial Flash Discoverable Parameters (SFDP) Signature = 50444653h
    	if (Index == 0) {
    	    wOffset = 0;
    	    if (*(UINT32*)&SfdpData[wOffset] != 0x50444653) {
                if (*(UINT32*)&SfdpData[++wOffset] != 0x50444653) {
// EIP137034 >>
/*#if defined SPI_PARAMETER_TABLE_INDEX && SPI_PARAMETER_TABLE_INDEX != 0
                    // Try to read the "SFDP" from SPI Parameter Table Register
                    UINT32  ParaTblData = 0;
                    *(volatile UINT32*)(gSPIBASE + SPI_PARAMETER_TABLE_INDEX) = 0;
                    MicroSecondDelay(1);
                    ParaTblData = *(volatile UINT32*)(gSPIBASE + SPI_PARAMETER_TABLE_INDEX + 4);
                    if(ParaTblData == 0x50444653) return EFI_SUCCESS;
#endif // #if defined SPI_PARAMETER_TABLE_INDEX && SPI_PARAMETER_TABLE_INDEX != 0*/
// EIP137034 <<
                    return EFI_UNSUPPORTED;
    	        }
            }        
        }            
    }
  	// SFDP opode at address Ch bits 23:00 = Parameter ID 0 table Address
  	Index = (*(UINT32*)&SfdpData[wOffset + 0xC] & 0x00FFFFFF);
  	// SFDP opode at address 05h(SFPD Major Revisions) must = 0001h
  	// SFDP opode at address 0Ah(Serial Flash Basic Major Revisions) must = 0001h
  	if ((SfdpData[wOffset + 5] != 0x1) || (SfdpData[wOffset + 0xA] != 0x1) || \
        ((Index + 0x10) > 256)) return EFI_UNSUPPORTED;
    // Parameter ID 0 Table BIT[21] - Fast Read Quad I/O.
    // Parameter ID 0 Table BIT[22] - Fast Read Quad Output.
    if (*(UINT32*)&SfdpData[Index + 1] & (BIT21 + BIT22)) return EFI_SUCCESS;
#endif  // #if SPI_QUAD_MODE_IDENTIFICATION == 1 //EIP160504
#endif  // #if SPI_INITIALIZE_WITH_VSCC == 1
    return EFI_UNSUPPORTED;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ReinitializeSpiEnvironment
//
// Description:
//
// Input:
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ReinitializeSpiEnvironment (
    IN OUT  FLASH_INFO      *FlashInfo
)
{
    UINT32	Buffer32;	// EIP137034

#if defined SPI_INITIALIZE_WITH_VSCC && SPI_INITIALIZE_WITH_VSCC == 1
    // Program UPPER/LOWER VSCC register.
    UINT32          dVSCC = 0;
    EFI_STATUS      Status;
    // Erase Opcode. 
    dVSCC = FlashInfo->Erase.Opcode << 8;
    // Block/Sector Erase Size.
    if (FlashInfo->SectorSize == SECTOR_SIZE_4KB) dVSCC |= BIT00;
    else if (FlashInfo->SectorSize == SECTOR_SIZE_8KB) dVSCC |= BIT01;
    else if (FlashInfo->SectorSize == SECTOR_SIZE_64KB) dVSCC |= (BIT00 + BIT01);
    // Write Granularity.
    if (FlashInfo->PageSize != 1) dVSCC |= BIT02;
    // Write Status Required.
    if (FlashInfo->WriteStatusEnable.Opcode == 0x50) dVSCC |= BIT03;
    // Write Enable On Write Status.
    if (FlashInfo->WriteStatusEnable.Opcode == 0x39) dVSCC |= BIT04;
    Status = SerialFlashDiscoveryForQuadRead();
    if (!EFI_ERROR(Status)) {
        switch ((UINT8)mExFlashPart.FlashVenDevId) {
            case 0xEF : // Winbond         
            case 0x37 : // AMIC
            case 0x01 : // Spansion
                dVSCC |= BIT5;  break;
            case 0xC2 : // MXIC
                dVSCC |= BIT6;  break;
            case 0x1F : // Atmel
                dVSCC |= (BIT5 + BIT6); break;
            case 0xBF : // SST/Microchip
                dVSCC |= BIT7;  break;
                break;
            default : break;
        }
    }    
    if (Status != EFI_WRITE_PROTECTED) {
#if LOWER_VSCC_REG != 0
        *(volatile UINT32*)(gSPIBASE + LOWER_VSCC_REG) = dVSCC;
#endif 
#if UPPER_VSCC_REG != 0
        *(volatile UINT32*)(gSPIBASE + UPPER_VSCC_REG) = dVSCC;
#endif 
    }
#endif 
//-    GlobalBlockProtectionUnlock();
    InitializeSpiEnvironment ( FlashInfo );
// EIP137034 >>
    //
    // Program B_PCH_SPI_TCGC_FCGDIS (Functional Clock Gating Disable) bit in 
    // R_PCH_SPI_TCGC (Trunk Clock Gating Control Register)
    //
    Buffer32 = *((volatile UINT32*)(gSPIBASE + 0x100));
    Buffer32 &= ~BIT10;
    *((volatile UINT32*)(gSPIBASE + 0x100)) = Buffer32;

    //
    // Program B_PCH_SPI_BCR_SRC (SPI Read Configuration) bits in
    // R_PCH_SPI_BCR (BIOS Control Register)
    //
    Buffer32 = *((volatile UINT32*)(gSPIBASE + 0xfc));
    Buffer32 &= ~(BIT3 | BIT2);
    // BIT3 = 1, BIT2 = 0 : Prefetch Enable, Cache Enable
    // BIT3 = 0, BIT2 = 1 : Prefetch Disable, Cache Disable
    // BIT3 = 0, BIT2 = 0 : Prefetch Disable, Cache Enable
    Buffer32 |= 0x00;
    *((volatile UINT32*)(gSPIBASE + 0xfc)) = Buffer32;
// EIP137034 <<
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CommonSpiReadId
//
// Description:
//
// Input:
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
CommonSpiReadId (
    IN      FLASH_INFO      *FlashInfo,
    IN OUT  UINT32          *dFlashId
)
{
    UINT16              wSpiCmd = 0xFFFF;

    InitializeSpiEnvironment( FlashInfo );
    // Set SPI read-address = 0
    *(volatile UINT32*)( gSPIBASE + SPI_ADR ) = 0;
    // set opcode for "Read ID"
    wSpiCmd = SPI_OPCODE_READ_ID_INDEX << 4;
    // set transaction = 3 bytes
    wSpiCmd += ( ( 3 - 1 ) << 8 );
    // indicate that data phase is required
    wSpiCmd += ( 1 << 14 );
    // Go (BIT1)
    *(volatile UINT16*)( gSPIBASE + SPI_CTL ) =  wSpiCmd + BIT01;
    // Wait for spi software sequence cycle completed.
    WaitForSpiCycleDone (FALSE, TRUE);
    SpiIoDelay();
    *dFlashId = *(volatile UINT32*)( gSPIBASE + SPI_DAT0 ) & 0x00FFFFFF;
    return  TRUE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiEraseCommand
//
// Description: This API function erases a block in the flash. Flash model
//              specific code will branch out from this routine
//
// Input:       pBlockAddress   Block that need to be erased
//
// Output:      Nothing
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiEraseCommand   (
    IN volatile UINT8*      pBlockAddress
)
{
    volatile UINT32     dSectorAddr, dPhyAddress;
    UINT32              dNByte;
    UINT16              wEraseRetry, wNumSectors, wSector;
    UINT16              wSpiCmd;

    // These parts only erase in 64K sectors
    InitializeSpiEnvironment( &mExFlashPart.FlashCommandMenu );
    wNumSectors = ( FlashBlockSize / FlashAPI->FlashSectorSize );
    for ( wSector = 0; wSector < wNumSectors ; wSector++ ) {
        dSectorAddr = (UINT32)(pBlockAddress + \
                                (wSector * FlashAPI->FlashSectorSize));
        dPhyAddress = CommonConvertSpiAddress( (volatile UINT8*)dSectorAddr );
        for ( dNByte = 0; dNByte < FlashAPI->FlashSectorSize; dNByte++ ) {
            if (0xFF != CommonSpiReadByte(dSectorAddr + dNByte)) break;
        }
        if  ( dNByte == FlashAPI->FlashSectorSize )   continue;
        for ( wEraseRetry = 0; wEraseRetry < FLASH_RETRIES; wEraseRetry++ ) {
            // Wait for spi hardware/software sequence cycle completed.
            WaitForSpiCycleDone (TRUE, TRUE);
            *(volatile UINT32*)( gSPIBASE + SPI_ADR ) = dPhyAddress;
            // opcode index 2 is programmed for erase command.
            // Set BIT1 (Go), BIT2(Atomic w/ Prefix)
            wSpiCmd = ( SPI_OPCODE_ERASE_INDEX << 4) + BIT01 + BIT02;
            *(volatile UINT16*)( gSPIBASE + SPI_CTL ) = wSpiCmd;
            // Wait for spi software sequence cycle completed.
            WaitForSpiCycleDone (FALSE, TRUE);
            // wait for SPI flash operation completed.
            WaitForWriteOperationCompleted();
            // write operation appeared to succeed, now read back byte
            // and compare.
            if (CommonSpiReadByte(dSectorAddr) == 0xFF)   break;
        }
        // According to Intel Server Platform Services Integration Guide, 
        // Section#5.3.5, Waits 10 milliseconds delay while erase blocks.
        MicroSecondDelay(10);
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiProgramCommand
//
// Description: This function programs a byte data to the specified location
//
// Input:   *pByteAddress   Location where the data to be written
//          Bytes - data to be written.
//          Length - number of bytes to write
//
// Output:  Length - number of bytes that were not written
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiProgramCommand (
    IN volatile UINT8       *pByteAddress,
    IN UINT8                *Byte,
    IN OUT UINT32           *Length
)
{
    UINT8               bFlashRetry = 0;
    UINT16              wProgBytes = 0, wNumBytes = 0;
    UINT16              wSpiCmd = 0, wRetry = 0, wMaxNumBytes = 0;
    UINT32              dPhyAddress = 0;


    InitializeSpiEnvironment( &mExFlashPart.FlashCommandMenu );
    wProgBytes = mCommonSpiFlash.FlashProgramSize;
    if ( mCommonSpiFlash.FlashProgramSize != 1 ) {
        // Limit the max transfer to the number of bytes the chipset can
        // transfer per cycle
        if ( *Length >= SPI_MAX_DATA_TRANSFER )
            wProgBytes = SPI_MAX_DATA_TRANSFER;
        else wProgBytes = *Length;
        // this is currently for the WINBOND parts only
        // mask off lowest 8 bits of address so that we can determine how
        // many bytes we can write before we hit the end of a page
        wMaxNumBytes = 0x100 - ((UINT32)pByteAddress & 0xFF);
        if ( (UINT32)pByteAddress & 0x1 )   wProgBytes = 1;
        else if ( wProgBytes > wMaxNumBytes ) wProgBytes = wMaxNumBytes;
    } else if(CheckAaiWordProram((UINT32)pByteAddress, *Length)) wProgBytes = 2;
    for ( bFlashRetry = 0; bFlashRetry < FLASH_RETRIES; bFlashRetry++ ) {
        // check if do the data need to be programmed ?
        for ( wNumBytes = 0; wNumBytes < wProgBytes; wNumBytes++ ) {
            if ( *( Byte + wNumBytes ) != 0xFF )    break;
        }
        // The data is empty and don't need to be programmed.
        if ( wNumBytes == wProgBytes )  break;
        // update data to chipset SPI data transfer registers.
        for ( wNumBytes = 0; wNumBytes < wProgBytes; wNumBytes++ ) {
            for ( wRetry = 0; wRetry < 0x400; wRetry ++ ) {
                *(volatile UINT8*)( gSPIBASE + SPI_DAT0 + wNumBytes ) = \
                                                         *( Byte + wNumBytes );
                // verified for checking the data is correct.
                if ( *( Byte + wNumBytes ) == \
                        *(volatile UINT8*)( gSPIBASE + SPI_DAT0 + wNumBytes ) )
                    break;
            }
        }
        // Wait for spi hardware/software sequence cycle completed.
        WaitForSpiCycleDone (TRUE, TRUE);
        dPhyAddress = CommonConvertSpiAddress ( pByteAddress );
        *(volatile UINT32*)( gSPIBASE + SPI_ADR ) = dPhyAddress;
        // BIT14 - indicate that it's data cycle.
        wSpiCmd = ( 1 << 14 );
        // BIT[8..13] - update the number of bytes to be written.
        wSpiCmd += ( wProgBytes - 1 ) << 8;
        // opcode index 0 is programmed for program command.
        // Set BIT1 (Go), BIT2(Atomic w/ Prefix)
        if (CheckAaiWordProram(dPhyAddress, wProgBytes))
            wSpiCmd += ( SPI_OPCODE_AAI_INDEX << 4) + BIT01 + BIT02;
        else wSpiCmd += ( SPI_OPCODE_WRITE_INDEX << 4) + BIT01 + BIT02;
        *(volatile UINT16*)( gSPIBASE + SPI_CTL ) = wSpiCmd;
        // Wait for spi software sequence cycle completed.
        WaitForSpiCycleDone (FALSE, TRUE);
        // wait for chipset SPI flash operation completed.
        WaitForWriteOperationCompleted();
        // Issue SPI Write Disable if SST AAIWordProgram supported.
        if (CheckAaiWordProram(dPhyAddress, wProgBytes)) SpiWriteDisable();
        // write operation appeared to succeed, now read back byte and compare
        // set control for 1-byte data read, no prefix
        for ( wNumBytes = 0; wNumBytes < wProgBytes; wNumBytes++ ) {
            if (*(Byte + wNumBytes) != \
                CommonSpiReadByte((UINT32)(pByteAddress + wNumBytes))) break;
        }
        if ( wNumBytes == wProgBytes )  break;
    }
    // Don't forget to return the number of bytes not written
    *Length = *Length - (UINT32)wProgBytes;
    return;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiReadCommand
//
// Description: This function programs a byte data to the specified location
//
// Input:   *pByteAddress   Location where the data to be written
//          Bytes - data to be written.
//          Length - number of bytes to write
//
// Output:  Length - number of bytes that were not written
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CommonSpiReadCommand   (
    IN  volatile UINT8      *pByteAddress,
    OUT UINT8               *Byte,
    IN  OUT UINT32          *Length
)
{
    UINT32              dReadAddress = 0; 
    UINT16              wSpiCmd, i, wMaxRead = 0;

    InitializeSpiEnvironment( &mExFlashPart.FlashCommandMenu );
    dReadAddress = CommonConvertSpiAddress ( pByteAddress );
    wMaxRead = 0x100 - (dReadAddress & 0xff);
    if (wMaxRead > SPI_MAX_DATA_TRANSFER) wMaxRead = SPI_MAX_DATA_TRANSFER;
    if (wMaxRead > *Length) wMaxRead = (UINT16)*Length;
    // update the read address.
    *(volatile UINT32*)( gSPIBASE + SPI_ADR ) = dReadAddress;
    // Opcode menu slot 1 is configured as "Read Flash"
    wSpiCmd = ( SPI_OPCODE_READ_INDEX << 4 ) + BIT01;
    // indicate that data phase is required
    wSpiCmd += (1 << 14);
    // BIT[8..13] - update the number of bytes to be read.
    wSpiCmd += (wMaxRead - 1) << 8;
    *(volatile UINT16*)( gSPIBASE + SPI_CTL ) = wSpiCmd;
    // Wait for spi software sequence cycle completed.
    WaitForSpiCycleDone (FALSE, TRUE);
    // read data
    for (i = 0; i < wMaxRead; i++) 
        *(Byte + i) = *(volatile UINT8*)(gSPIBASE + SPI_DAT0 + i);
    *Length = *Length - (UINT32)wMaxRead;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiIsEraseCompleted
//
// Description: This function verifies whether the block erase
//      command is completed and returns the status of the command
//
// Input:   *pBlockAddress  Location of the block erase
//
// Output:  *pError         True on error and false on success
//          *Status         Error status code (0 - Success)
//
// Return:  TRUE        If operation completed successfully
//                          *pError = FALSE, *pStatus = 0
//          FALSE       If operation failed
//                          *pError = TRUE, *pStatus = error status
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
CommonSpiIsEraseCompleted   (
    IN  volatile UINT8          *pBlockAddress,
    OUT BOOLEAN                 *pError,
    OUT UINTN                   *pStatus
)
{
    UINT32                      dNumBytes;

    for ( dNumBytes = 0; dNumBytes < FlashBlockSize; dNumBytes++ ) {
        if ( *(volatile UINT8*)( pBlockAddress + dNumBytes ) != 0xFF ) {
            if ( pStatus ) *pStatus = EFI_NOT_READY;
            if ( pError ) *pError = TRUE;
            return FALSE;
        }
    }
    if ( pError ) *pError = FALSE;
    if ( pStatus ) *pStatus = EFI_SUCCESS;
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiIsProgramCompleted
//
// Description: This function verifies whether the program (page or byte)
//      command is completed and returns the status of the command
//
// Input:   *pByteAddress   Location of the program command
//          Byte            Last data byte written
//
// Output:  *pError         True on error and false on success
//          *Status         Error status code (0 - Success)
//
// Return:  TRUE        If operation completed successfully
//                          *pError = FALSE, *pStatus = 0
//          FALSE       If operation failed
//                          *pError = TRUE, *pStatus = error status
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
CommonSpiIsProgramCompleted (
    IN  volatile UINT8          *pByteAddress,
    IN  UINT8                   *Byte,
    IN  UINT32                  Length,
    OUT BOOLEAN                 *pError,
    OUT UINTN                   *pStatus
)
{
    UINT32                      dNumBytes;
    UINT8                       bByte;

    for ( dNumBytes = 0; dNumBytes < Length; dNumBytes++ ) {
        bByte = * ( Byte + dNumBytes );
        if ( bByte != *(volatile UINT8*)( pByteAddress + dNumBytes ) ) {
            if ( pStatus ) *pStatus = EFI_NOT_READY;
            if ( pError ) *pError = TRUE;
            return FALSE;
        }
    }
    if ( pError ) *pError = FALSE;
    if ( pStatus ) *pStatus = EFI_SUCCESS;
    return TRUE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SpiBlockProtectUpdate
//
// Description: This function writes the status register.
//
// Input:   *pBlockAddress - Address within the block to write enable.
//          bStatusData - Value to be written to Status Register.
//
// Output:      None
//
// Return:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
SpiBlockProtectUpdate (
    IN volatile UINT8           *pBlockAddress,
    IN UINT8                    bStatusData
)
{
    UINT8                   bStatusReg = 0, bPrefixOp, bDataPhase = 1, bBPbits;
    UINT32                  dSectorAddr = 0;

    dSectorAddr = CommonConvertSpiAddress (pBlockAddress);
    bStatusReg = CommonSpiReadStatus (dSectorAddr);
    bBPbits = (BIT02 + BIT03 + BIT04);
    switch ( (UINT8)mExFlashPart.FlashVenDevId ) {
        // if SST flash, prefix 1 w/o address
        case 0xBF :
            bPrefixOp = SPI_PREFIX_WRITE_S_EN;
            break;
        // if ATMEL flash, prefix 0 w/ address
        case 0x1F :
            bBPbits = (BIT02 + BIT03);
        default :
        // default flash, prefix 0 w/o address
            bPrefixOp = SPI_PREFIX_WRITE_EN;
    }
    bStatusReg &= bBPbits;              // keep valid bits.
    bStatusData &= bBPbits;             // keep valid bits.
    if (bStatusReg != bStatusData) {
        CommonSpiWriteStatus (  bStatusData, \
                                SPI_OPCODE_WRITE_S_INDEX, \
                                bDataPhase, \
                                bPrefixOp, \
                                dSectorAddr );
    }                                
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiBlockWriteEnable
//
// Description: This function contains any flash specific code need to
//              enable a particular flash block write
//
// Input:   *pBlockAddress - Address within the block to write enable
//
// Output:      None
//
// Return:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiBlockWriteEnable   (
    IN volatile UINT8       *pBlockAddress
)
{
    SpiBlockProtectUpdate (pBlockAddress, 0);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiBlockWriteDisable
//
// Description: This function contains any flash specific code need to
//              disable a particular flash block write
//
// Input:   *pBlockAddress - Address within the block to write disable
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiBlockWriteDisable  (
    IN volatile UINT8           *pBlockAddress
)
{
#if BLOCK_PROTECT_ENABLE
    SpiBlockProtectUpdate (pBlockAddress, 0xff);
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiDeviceWriteEnable
//
// Description: This function contains any flash specific code need to
//              enable flash write
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiDeviceWriteEnable (VOID)
{
    // check is DeviceWrite enabled, if yes, don't enable it again,
    // else, enable it.
    if ( !gbDeviceWriteEnabled ) {
        gbDeviceWriteEnabled = 1;
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiDeviceWriteDisable
//
// Description: This function contains any flash specific code need to
//              disable flash write
//
// Input:       None
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
CommonSpiDeviceWriteDisable (VOID)
{
    // check is DeviceWrite enabled, if yes, disable it,
    // if no, don't disable it.
    if ( gbDeviceWriteEnabled ) {
        gbDeviceWriteEnabled = 0;
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CommonSpiDeviceVirtualFixup
//
// Description: This function will be invoked by the core to convert
//              runtime pointers to virtual address
//
// Input:       *pRS    Pointer to runtime services
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
CommonSpiDeviceVirtualFixup (
    IN EFI_RUNTIME_SERVICES  *pRS
)
{

//  // Following is an example code for virtual address conversion
//  pRS->ConvertPointer(0, (VOID**)&FlashDeviceBase);

//-    SpiChipsetVirtualFixup(pRS);
    pRS->ConvertPointer(0, (VOID **)&gSPIBASE);
    gbDeviceVirtual = 1;

    return;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
