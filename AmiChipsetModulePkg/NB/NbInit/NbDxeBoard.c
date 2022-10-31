//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        NbDxeBoard.c
//
// Description: This file contains DXE stage board component code for
//              Template NB
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Protocol/PciRootBridgeIo.h>

#if SMBIOS_SUPPORT
#include <Protocol/SmbiosDynamicData.h>
#endif
#include <Nb.h>
#include <HOB.h>
#include <Protocol/MemInfo.h>
#include <Guid/MemoryConfigData.h>
#include <MemoryInit/Src32/Mrc.h>
#include <Library/MemoryDownLib.h>  //EIP168616

//EIP154389 >>
#if SMBIOS_SUPPORT
#include <Protocol/SmBus.h>

#define MRC_REF_CLOCK_133                 (0)
#define MRC_REF_CLOCK_100                 (1)
#define fNoInit                           (0)
#define f800                              (800)
#define f1000                             (1000)
#define f1067                             (1067)
#define f1200                             (1200)
#define f1333                             (1333)
#define f1400                             (1400)
#define f1600                             (1600)
#define f1800                             (1800)
#define f1867                             (1867)
#define f2000                             (2000)
#define f2133                             (2133)
#define f2200                             (2200)
#define f2400                             (2400)
#define f2600                             (2600)
#define f2667                             (2667)
#define fUnSupport                        (0x7FFFFFFF)

#define MRC_FREQUENCY_MTB_OFFSET          1000000
#define MRC_FREQUENCY_FTB_OFFSET          1000
#define MRC_DDR3_800_TCK_MIN              2500000   /// 1/(800/2) femtoseconds
#define MRC_DDR3_1000_TCK_MIN             2000000   /// 1/(1000/2) femtoseconds
#define MRC_DDR3_1067_TCK_MIN             1875000   /// 1/(1067/2) femtoseconds
#define MRC_DDR3_1200_TCK_MIN             1666666   /// 1/(1200/2) femtoseconds
#define MRC_DDR3_1333_TCK_MIN             1500000   /// 1/(1333/2) femtoseconds
#define MRC_DDR3_1400_TCK_MIN             1428571   /// 1/(1400/2) femtoseconds
#define MRC_DDR3_1600_TCK_MIN             1250000   /// 1/(1600/2) femtoseconds
#define MRC_DDR3_1800_TCK_MIN             1111111   /// 1/(1800/2) femtoseconds
#define MRC_DDR3_1867_TCK_MIN             1071428   /// 1/(1867/2) femtoseconds
#define MRC_DDR3_2000_TCK_MIN             1000000   /// 1/(2000/2) femtoseconds
#define MRC_DDR3_2133_TCK_MIN             937500    /// 1/(2133/2) femtoseconds
#define MRC_DDR3_2200_TCK_MIN             909090    /// 1/(2200/2) femtoseconds
#define MRC_DDR3_2400_TCK_MIN             833333    /// 1/(2400/2) femtoseconds
#define MRC_DDR3_2600_TCK_MIN             769230    /// 1/(2600/2) femtoseconds
#define MRC_DDR3_2667_TCK_MIN             750000    /// 1/(2667/2) femtoseconds
#define MRC_DDR3_2800_TCK_MIN             714285    /// 1/(2800/2) femtoseconds
#define TREFIMULTIPLIER                   1000      /// tREFI value defined in XMP 1.3 spec is actually in thousands of MTB units.
#define MAX(a,b)   (((a) > (b)) ? (a) : (b))
#define MIN(a,b)   (((a) < (b)) ? (a) : (b))

typedef struct {
  UINT32           tCK;
  UINT32           DDRFreq;
  UINT8            RefClkFlag;  // 0 = invalid freq. 1 = valid only at 133 RefClk, 2 = valid only at 100 RefClk, 3 = valid at both.
} NbTRangeTable;

// Timing Range table
const NbTRangeTable NbRange[] = {
  { 0xFFFFFFFF,            fUnSupport, (0 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_800_TCK_MIN,  f800,       (1 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1000_TCK_MIN, f1000,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1067_TCK_MIN, f1067,      (1 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1200_TCK_MIN, f1200,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1333_TCK_MIN, f1333,      (1 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1400_TCK_MIN, f1400,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1600_TCK_MIN, f1600,      (1 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1800_TCK_MIN, f1800,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_1867_TCK_MIN, f1867,      (1 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_2000_TCK_MIN, f2000,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_2133_TCK_MIN, f2133,      (1 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_2200_TCK_MIN, f2200,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_2400_TCK_MIN, f2400,      (1 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_2600_TCK_MIN, f2600,      (0 << MRC_REF_CLOCK_133) | (1 << MRC_REF_CLOCK_100) },
  { MRC_DDR3_2667_TCK_MIN, f2667,      (1 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) },
  { 0,                     fNoInit,    (0 << MRC_REF_CLOCK_133) | (0 << MRC_REF_CLOCK_100) }
};
#endif
//EIP154389 <<

// GUID Definitions

// Produced Protocols
//EIP154389 >>
#if SMBIOS_SUPPORT
EFI_SMBUS_HC_PROTOCOL   *gSmbusProtocol 		 = NULL;
#endif 
//EIP154389 <<

EFI_GUID    			EfiMemoryConfigDataGuid  = EFI_MEMORY_CONFIG_DATA_GUID;
EFI_GUID    			HobListGuid 			 = HOB_LIST_GUID;

// Portable Constants

// Function Prototypes

// PPI interface definition

// Protocols that are installed
MEM_INFO_PROTOCOL             mMemInfoHobProtocol;
MRC_PARAMS_SAVE_RESTORE       *MemInfoHob;

// Function Definition

EFI_STATUS
SaveMemoryInformation(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
);

VOID CreateMemoryDataForSMBios(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbDxeBoardInit
//
// Description: This function initializes the board specific component in
//              in the chipset north bridge
//
// Input:       ImageHandle Image handle
//              SystemTable Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
NbDxeBoardInit(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable)
{
    EFI_STATUS         Status = EFI_SUCCESS;

    SaveMemoryInformation(ImageHandle, SystemTable);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SaveMemoryInformation
//
// Description: This function initializes the board specific component in
//              in the chipset north bridge
//
// Input:       ImageHandle Image handle
//              SystemTable Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
SaveMemoryInformation(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable)
{
    EFI_STATUS                    Status = EFI_SUCCESS;
    VOID                          *Hob;
    EFI_HANDLE                    Handle = NULL;
    UINT8                                         Channel_local;
    UINT8                                         Slot_num;
//EIP154389 >>	
#if SMBIOS_SUPPORT
    EFI_EVENT                       SmbusEvent;
    VOID                            *SmbusRegistration = NULL;
#endif
//EIP154389 <<
    
    InitAmiLib(ImageHandle, SystemTable);

    // Get the HOB list and install MemInfo protocol
    Hob = GetEfiConfigurationTable(SystemTable, &HobListGuid);
    if(!Hob) {
        return EFI_INVALID_PARAMETER;
    }

    Status = FindNextHobByGuid(&gEfiMemoryConfigDataGuid, &Hob);
    if(Status == EFI_SUCCESS) {
        MemInfoHob = (MRC_PARAMS_SAVE_RESTORE *)((UINT8 *)(Hob) + sizeof(EFI_HOB_GUID_TYPE));
        mMemInfoHobProtocol.MemInfoData.memSize = 0;
        for(Channel_local = 0; Channel_local < MEM_CHANNEL_NUM; Channel_local++) {
            if(!MemInfoHob->Channel[Channel_local].Enabled) continue;
            mMemInfoHobProtocol.MemInfoData.ddrType     = MemInfoHob->DDRType;
            for(Slot_num = 0; Slot_num < DIMM_SLOT_NUM; Slot_num++) { //P20130624 
    		  if(MemInfoHob->Channel[Channel_local].DimmPresent[Slot_num]) {
    			  mMemInfoHobProtocol.MemInfoData.memSize += (UINT32) (MemInfoHob->Channel[Channel_local].SlotMem[Slot_num]);
    			  mMemInfoHobProtocol.MemInfoData.dimmSize[Channel_local*DIMM_SLOT_NUM+Slot_num] = MemInfoHob->Channel[Channel_local].SlotMem[Slot_num]; //P20130624 //EIP126797 - WHCK test: "SMBIOS HCT"
    		  }
            }
            mMemInfoHobProtocol.MemInfoData.ddrFreq     = MemInfoHob->DdrFreq;
        }

        TRACE((-1, "Install memory protocol:\n"));

        switch(mMemInfoHobProtocol.MemInfoData.ddrType) {
        case DDRType_DDR3:
            TRACE((-1, "	DDR3 SDRAM Memory type.\n"));
            break;
        case DDRType_DDR3L:
            TRACE((-1, "	DDR3L SDRAM Memory type.\n"));
            break;
        case DDRType_DDR3ECC:
            TRACE((-1, "	DDR3ECC SDRAM Memory type.\n"));
            break;
        case DDRType_LPDDR2:
            TRACE((-1, "	LPDDR2 SDRAM Memory type.\n"));
            break;
        case DDRType_LPDDR3:
            TRACE((-1, "	LPDDR3 SDRAM Memory type.\n"));
            break;
        case DDRType_DDR4:
            TRACE((-1, "	DDR4 SDRAM Memory type.\n"));
            break;
        default:
            TRACE((-1, "	Unknown SDRAM Memory type.\n"));
            break;
        }

        TRACE((-1, "	Total memory size = %x \n", mMemInfoHobProtocol.MemInfoData.memSize));
        for(Slot_num = 0; Slot_num < MEM_SOCKETS_NUM; Slot_num++) //P20130624 
            TRACE((-1, "	Dimm%x = %x,", Slot_num, mMemInfoHobProtocol.MemInfoData.dimmSize[Slot_num]));
        TRACE((-1, "\n"));

        switch(mMemInfoHobProtocol.MemInfoData.ddrFreq) {
        case DDRFREQ_800:
            TRACE((-1, "	Memory Frequency is 800.\n"));
            break;
        case DDRFREQ_1066:
            TRACE((-1, "	Memory Frequency is 1066.\n"));
            break;
        case DDRFREQ_1333:
            TRACE((-1, "	Memory Frequency is 1333.\n"));
            break;
        case DDRFREQ_1600:
            TRACE((-1, "	Memory Frequency is 1600.\n"));
        }

        Status = pBS->InstallMultipleProtocolInterfaces(
                     &Handle,
                     &gMemInfoProtocolGuid,
                     &mMemInfoHobProtocol,
                     NULL
                 );

#if SMBIOS_SUPPORT
//EIP154389 >>
        Status = RegisterProtocolCallback(
                  &gEfiSmbusHcProtocolGuid,
                  CreateMemoryDataForSMBios,
                  NULL,
                  &SmbusEvent,
                  &SmbusRegistration
                  );    
//EIP154389 <<
#endif

    } else {
        TRACE((TRACE_ALWAYS, "Cannot find MemInfoHob... \n"));
        ASSERT_EFI_ERROR(Status);
    }

    return Status;
}

#if SMBIOS_SUPPORT
//EIP154389 >>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadSpdData
//
// Description: Returns the length of the Dimm Spd
//              
// Input:       	UINT8	                SpdSalveAddr,
// 	                UINT8					Offset,
// 	                UINTN					Count,
// Output:      	UINT8					*Buffer
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadSpdData (
	IN  UINT8	                SpdSalveAddr,
	IN	UINT8					Offset,
	IN	UINTN					Count,
	OUT	UINT8					*Buffer
	)
{
	EFI_STATUS					Status;
	UINTN						Index;
	UINTN						Length;
	EFI_SMBUS_OPERATION			Operation;
	EFI_SMBUS_DEVICE_COMMAND	Command;
    EFI_SMBUS_DEVICE_ADDRESS    SlaveAddress;

    if(gSmbusProtocol == NULL) 	return EFI_UNSUPPORTED;

	SlaveAddress.SmbusDeviceAddress = SpdSalveAddr;

	for (Index = 0; Index < Count; Index++) 
	{
		Command = Offset + Index;

		Length = 1;
		Operation = EfiSmbusReadByte;
		Status = gSmbusProtocol->Execute (gSmbusProtocol, 
							SlaveAddress,
							Command, 
							Operation, 
							FALSE,
							&Length,
							&Buffer[Index] );
		if (EFI_ERROR(Status)) return Status;
	}

	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbGetDimmFrequency
//
// Description: Returns Dimm Frequency
//              
//
// Input:       UINT32 tCK
//
// Output:      UINT32 XmpFrequency
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static
UINT32
NbGetDimmFrequency (
  IN     UINT32             tCK
  )
{
  UINT32                              Index;
  UINT32                              XmpFrequency = fNoInit;
  UINT32                              NbRangeSize        = (sizeof (NbRange) / sizeof (NbTRangeTable)) - 1;

  if(tCK == 0 || tCK == 0xffffffff) return fNoInit;

     for (Index = 0; Index < NbRangeSize; Index++) {
       if ((tCK <= NbRange[Index].tCK) && (tCK > NbRange[Index + 1].tCK)) {
         XmpFrequency = NbRange[Index].DDRFreq;
         break;
       }
     }
     
     /*
     while (Index) {
       if (NbRange[Index].RefClkFlag & (1 << MRC_REF_CLOCK_133)) {
         XmpFrequency = NbRange[--Index].DDRFreq;
       } else break;
     }
 	 */
  return XmpFrequency;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbGetDimmTCK
//
// Description: Returns the Dimm tCK Timing
//              
//
// Input:       UINT32 tCK
//
// Output:      TRUE  - Have tCK Timing
//              FALSE - Not have tCK Timing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static BOOLEAN
NbGetDimmTCK (
  UINT8                        SpdFtbDividend,
  UINT8                        SpdFtbDivisor,
  UINT8                        SpdMtbDividend,
  UINT8                        SpdMtbDivisor,
  UINT8                        tCKminMtb,
  UINT8                        tCKminFine,
  OUT     UINT32               *tCK
  )
{
    INT32              MediumTimebase = 0;
    INT32              FineTimebase = 0;

    FineTimebase  = (SpdFtbDivisor == 0) ? 0 : (SpdFtbDividend * MRC_FREQUENCY_FTB_OFFSET) / SpdFtbDivisor;
    MediumTimebase  = (SpdMtbDivisor == 0) ? 0 : (SpdMtbDividend * MRC_FREQUENCY_MTB_OFFSET) / SpdMtbDivisor;
     *tCK = (MediumTimebase * tCKminMtb) + (FineTimebase * tCKminFine);

     return (MediumTimebase == 0) ? FALSE : TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbSmbiosType17Speed
//
// Description: To update Dimm Frequency
//              
// Input:       SMBIOS_MEMORY_DEVICE_INFO     *TypeBuffer
//
// Output:      SMBIOS_MEMORY_DEVICE_INFO     *TypeBuffer
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 NbSmbiosType17Speed (
  IN UINT8                    DimmNumber,
//EIP159928 >>
  IN SMBUS_DEVICE_ADDRESS     SpdSmBusAddr,
  IN OUT MEMORY_DEVICE        *TypeBuffer
//EIP159928 <<  
  ) 
{
//EIP168616 >>
#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
    MRC_DRAM_INPUT                      Input_Struct;
#else
    UINT8                               VoltageCap = 0; //EIP159928
    UINT8                               SpdFtbDivisor = 0;
    UINT8                               SpdFtbDividend = 0;
    UINT8                               SpdMtbDividend = 0;
    UINT8                               SpdMtbDivisor = 0;
    UINT32                              tCK = 0;
    UINT8                               tCKminMtb = 0;
    UINT8                               tCKminFine = 0;
    UINT8                               SpdData[18] = {6, 9, 10, 11, 12, 34, 176, 177, 178, 180, 181, 182, 183, 184, 186, 211, 221, 246};
    UINT8                               *DimmData = NULL;  //EIP168616
    UINT8                               i;
    EFI_STATUS                          Status;
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
    TypeBuffer->MinimumVoltage  = OEM_DRAM_MIN_VOL;
    TypeBuffer->MaximumVoltage  = OEM_DRAM_MAX_VOL;
    FillMemoryDownParam (&Input_Struct);
    switch (Input_Struct.DRAM_Speed) {
    case 0x00:
      TypeBuffer->Speed = 800;
      break;
    case 0x01:
      TypeBuffer->Speed = 1066;
      break;
    case 0x02:
      TypeBuffer->Speed = 1333;
      break;
    case 0x03:
      TypeBuffer->Speed = 1600;
      break;
    default:
      TypeBuffer->Speed = 1333;
      break;
    }
#else
//EIP168616 <<

    Status = pBS->LocateProtocol( &gEfiSmbusHcProtocolGuid, NULL, (void **)&gSmbusProtocol );

    // Get Spd data
    for (i = 0; i < sizeof(SpdData) ; i++) { 
      //EIP168616 >>
      #if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
      DimmData = GetDimmSpdTbl (DimmNumber/2, DimmNumber);
      //try memory down mode
      SpdData[i] = DimmData[SpdData[i]];
      #else
      Status = ReadSpdData((UINT8)SpdSmBusAddr.SmbusDeviceAddress, SpdData[i], 1, &SpdData[i]);
      #endif
      //EIP168616 <<
    }

    //EIP159928 >>
    VoltageCap = SpdData[0];
    if (VoltageCap & 0x04) { // 1.25 v
        TypeBuffer->MinimumVoltage = 1250;
    } else if (VoltageCap & 0x02) { // 1.35 v
        TypeBuffer->MinimumVoltage = 1350; 
    } else if (!(VoltageCap & 0x01)) { // 1.5 v  bit0 = 0 
        TypeBuffer->MinimumVoltage = 1500; 
    }

    if (!(VoltageCap & 0x01)) { // 1.5 v  bit0 = 0 
        TypeBuffer->MaximumVoltage = 1500;
    } else if (VoltageCap & 0x02) { // 1.35 v
        TypeBuffer->MaximumVoltage = 1350; 
    } else if (VoltageCap & 0x04) { // 1.25 v
        TypeBuffer->MaximumVoltage = 1250; 
    }      
	//EIP159928 <<

           // Calculate Dimm STD Profile
    SpdFtbDivisor = SpdData[1];
    SpdFtbDividend = (UINT8)(SpdFtbDivisor >> 4);
    SpdFtbDivisor &= 0x0f;
    SpdMtbDividend  = SpdData[2];
    SpdMtbDivisor  = SpdData[3];
    tCKminMtb  = SpdData[4];
    tCKminFine  = SpdData[5];
           
    // Get tCK Timing
    if(NbGetDimmTCK(SpdFtbDividend, SpdFtbDivisor, SpdMtbDividend, SpdMtbDivisor, tCKminMtb, tCKminFine, &tCK)){
    	// Get Dimm Frequency
      TypeBuffer->Speed = NbGetDimmFrequency(tCK); //EIP159928 
    
#if defined DCLK_FREQUENCY && DCLK_FREQUENCY == 1
        // Get Dimm Frequency
        if((NbGetDimmFrequency(tCK)%5) == 3) {
          TypeBuffer->Speed = (NbGetDimmFrequency(tCK)/2)+1; //EIP159928 
        } else {
          TypeBuffer->Speed = (NbGetDimmFrequency(tCK)/2); //EIP159928 
        }
#endif  
    }
#endif  //EIP168616
    return EFI_SUCCESS; //EIP159928
}
//EIP154389 <<

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//                     ***** PORTING REQUIRED *****
//               (if SMBIOS module is part of the project)
//----------------------------------------------------------------------------
// Procedure: CreateMemoryDataForSMBios
//
// Description: This function gathers the System Memory information and
//              saves them in a variable named "SmbiosMemVar". This variable
//              with the slot information is needed by the SMBIOS module to
//              create the "Type 16, 17, 18, 19" structure.
//
// Input:  None
//
// Output: Creates variable named "SmbiosMemVar" with System Memory
//         information
//
// Note: Refer to SYSTEM_MEM_ARRAY_DYNAMIC_DATA in SmbiosDynamicData.h for
//       structure information.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CreateMemoryDataForSMBios(
//EIP154389 >>
	    IN EFI_EVENT      Event,
	    IN VOID           *Context
//EIP154389 <<
)
{
    //
    //  Sample code for Blackford CRB. Needs to be changed for other platforms.
    //
    EFI_STATUS                      Status;
    SYSTEM_MEM_ARRAY_DYNAMIC_DATA   Memory;
    UINTN                           VarSize;
    UINT32                          Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS;
    UINT8                           PMAIndex, MemDevIndex, Channel_local; //EIP126797
    UINT32                          Type16StartingAddr = 0;
    UINT32                          Type20StartingAddr = 0;
    SMBUS_DEVICE_ADDRESS			SlvAddrReg[]= SALVE_ADDRESS; //EIP130719
    MEMORY_DEVICE                   *TypeBuffer; //EIP159928 >>
// Parse Hobs to get SmbiosMemConfig Hob.

    // Allocate temporary buffer
    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(MEMORY_DEVICE)+0x100, &TypeBuffer); //EIP159928 >>

    for(PMAIndex = 0; PMAIndex < MEMORY_ARRAY_NUM; PMAIndex++) { //P20130624 
        // Type 16
        Memory.PhysicalMemArray[PMAIndex].MaxCapacity = (0x1000000000 >> 13);       // 64GB //CSP20140331_22
        Memory.PhysicalMemArray[PMAIndex].ExtMaxCapacity = 0; //EIP153754
        Memory.PhysicalMemArray[PMAIndex].MemErrInfoHandle = 0xFFFF;        // 0xFFFE if not supported
        // 0xFFFF if supported but no error

#if MEMORY_ERROR_INFO
        // Type 18
        Memory.PhysicalMemArray[PMAIndex].ArrayMemoryError.ErrorType = 3;
        Memory.PhysicalMemArray[PMAIndex].ArrayMemoryError.ErrorGranularity = 2;
        Memory.PhysicalMemArray[PMAIndex].ArrayMemoryError.ErrorOperation = 2;
        Memory.PhysicalMemArray[PMAIndex].ArrayMemoryError.MemArrayErrorAddress = 0x80000000;
        Memory.PhysicalMemArray[PMAIndex].ArrayMemoryError.DeviceErrorAddress = 0x80000000;
        Memory.PhysicalMemArray[PMAIndex].ArrayMemoryError.ErrorResolution = 0x80000000;
#endif

//EIP126797 >>
        Type20StartingAddr = Type16StartingAddr;
        for(Channel_local = 0; Channel_local < MEM_CHANNEL_NUM; Channel_local++) {
          for(MemDevIndex = 0; MemDevIndex < DIMM_SLOT_NUM; MemDevIndex++) { //P20130624 
            // Type 17
            TRACE((TRACE_ALWAYS, "MemInfoHob->Channel[%X].SlotMem[%X]: %X\n",Channel_local,MemDevIndex,MemInfoHob->Channel[Channel_local].SlotMem[MemDevIndex]));
            if(MemInfoHob->Channel[Channel_local].DimmPresent[MemDevIndex]) {
            	
             	Memory.PhysicalMemArray[PMAIndex].SpdSmBusAddr[Channel_local*DIMM_SLOT_NUM+MemDevIndex] = SlvAddrReg[Channel_local*DIMM_SLOT_NUM+MemDevIndex]; //EIP130719
            	
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.TotalWidth = 8 * (1 << (MemInfoHob->Channel[Channel_local].DimmBusWidth[MemDevIndex] & 0x07));
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Size = MemInfoHob->Channel[Channel_local].SlotMem[MemDevIndex];
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.MemoryType = 0x18;

                switch(MemInfoHob->DdrFreq) {
                case DDRFREQ_800:
//EIP154389 >>
                    //Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = 800;
#if defined DCLK_FREQUENCY && DCLK_FREQUENCY == 1
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 800/2;
#else
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 800;
#endif
//EIP154389 <<
                    break;
                case DDRFREQ_1066:
//EIP154389 >>
                    //Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = 1066;
#if defined DCLK_FREQUENCY && DCLK_FREQUENCY == 1
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 1066/2;
#else
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 1066;
#endif
//EIP154389 <<
                	break;
                case DDRFREQ_1333:
//EIP154389 >>
                    //Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = 1333;
#if defined DCLK_FREQUENCY && DCLK_FREQUENCY == 1
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 1333/2;
#else
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 1333;
#endif
//EIP154389 <<
                    break;
                case DDRFREQ_1600:
//EIP154389 >>
                    //Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = 1600;
#if defined DCLK_FREQUENCY && DCLK_FREQUENCY == 1
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 1600/2;
#else
                	Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 1600;
#endif
//EIP154389 <<
                    break;
                default:
                    //Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = 0; //EIP154389
                    Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 0;
                }
				//EIP159928 >>
                NbSmbiosType17Speed(Channel_local*DIMM_SLOT_NUM+MemDevIndex, SlvAddrReg[Channel_local*DIMM_SLOT_NUM+MemDevIndex], TypeBuffer); //EIP154389
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = TypeBuffer->Speed;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.MinimumVoltage = TypeBuffer->MinimumVoltage;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.MaximumVoltage = TypeBuffer->MaximumVoltage;
                switch(MemInfoHob->DDRType) {
                case DDRType_DDR3L:
                    Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfiguredVoltage = 1350;
                    break;
                case DDRType_LPDDR3:
                    Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfiguredVoltage = 1250;
                    break;
                case DDRType_DDR3:
                case DDRType_DDR3ECC:
                case DDRType_LPDDR2:
                case DDRType_DDR4:
                default:
                    Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfiguredVoltage = 1500;
                    break;
                }
                ;
                //EIP159928 <<    
            } else {
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.TotalWidth = 0xFFFF;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Size = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Speed = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.MemoryType = 0x02;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfMemClkSpeed = 0;
				//EIP159928 >>
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.MinimumVoltage = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.MaximumVoltage = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ConfiguredVoltage = 0;
				//EIP159928 <<
            }
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.ExtendedSize = 0;
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.DeviceSet = ((MemDevIndex / DIMM_SLOT_NUM) + (MemDevIndex % DIMM_SLOT_NUM)); //P20130624 

#if defined AMI_SMBIOS_MODULE_VERSION && AMI_SMBIOS_MODULE_VERSION >= 100
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.TypeDetail = BIT02;
#endif
            
#if MEMORY_ERROR_INFO
            // Type 18
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type18.ErrorType = 3;
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type18.ErrorGranularity = 2;
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type18.ErrorOperation = 2;
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type18.MemArrayErrorAddress = 0x80000000;
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type18.DeviceErrorAddress = 0x80000000;
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type18.ErrorResolution = 0x80000000;
#endif

            // Type 20
            if(Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Size == 0) {
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.StartingAddress = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.EndingAddress = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.InterleavePosition =  0; //EIP148189
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.InterleaveDataDepth = 0; //EIP148189           
            } else {
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.StartingAddress = Type20StartingAddr;
                Type20StartingAddr += (Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type17.Size << 10);  // Size in MB, convert to KB
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.EndingAddress = Type20StartingAddr - 1;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.InterleavePosition = Channel_local+1;  //EIP148189
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.InterleaveDataDepth = MEM_CHANNEL_NUM;   //EIP148189            
            }
            Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.PartitionRowPosition = SMBIOS_PARTITION_ROW_POSITION;  //EIP148189
            
            //EIP142202 >>
            if (((Type20StartingAddr - 1)/1024) >= 0xFFFFFFFF)
            {
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.ExtendedEndAddr = ((Type20StartingAddr - 1)/1024);
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.EndingAddress   = 0xFFFFFFFF;
            }
            else
            {
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.ExtendedStartAddr  = 0;
                Memory.PhysicalMemArray[PMAIndex].MemoryDeviceData[Channel_local*DIMM_SLOT_NUM+MemDevIndex].Type20.ExtendedEndAddr    = 0;
            }
            //EIP142202 <<
          }
        }
//EIP126797 <<

        // Type 19
        if(Type16StartingAddr == Type20StartingAddr) {
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.StartingAddress = 0;
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.EndingAddress = 0;
        } else {
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.StartingAddress = Type16StartingAddr;
            Type16StartingAddr = Type20StartingAddr;
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.EndingAddress = Type16StartingAddr - 1;
        }
        Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.PartitionWidth = 2;

        //EIP142202 >>
        if (((Type16StartingAddr - 1)/1024) >= 0xFFFFFFFF)
        {
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.ExtendedEndAddr = ((Type16StartingAddr - 1)/1024);
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.EndingAddress   = 0xFFFFFFFF;
        }
        else
        {
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.ExtendedStartAddr  = 0;
            Memory.PhysicalMemArray[PMAIndex].MemArrayMapAddr.ExtendedEndAddr    = 0;
        }
        //EIP142202 <<
    }
    VarSize = sizeof(SYSTEM_MEM_ARRAY_DYNAMIC_DATA);
    Status = pRS->SetVariable(SmbiosMemVar, &gAmiSmbiosDynamicDataGuid,
                              Attributes, VarSize, &Memory);

    if(EFI_ERROR(Status)) {
        TRACE((-1, "SetVariable:%r \n", Status));
    }
    
    pBS->FreePool(TypeBuffer); //EIP159928 
    pBS->CloseEvent(Event); //EIP154389
}
#endif


//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
