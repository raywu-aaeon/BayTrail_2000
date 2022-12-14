/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
  --*/
/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


--*/

/*++

Change Log:
1445-18032013: removed extra '0x' from LPI_RES_COUNTERx descriptor
1300-18032013: Changed Res Frequency from 0x0 (TSC) to 0x8000 (32768)
1200-18032013: Set all Residency/Latency values to match S0i3

--*/

#ifndef _LPIT_H

#define _LPIT_H

//
// Include files
//
#include <Library/UefiLib.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiSupport.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DriverLib.h>
#include <IndustryStandard/Acpi50.h>
#include <AcpiOemElinks.h>  //EIP134732



//
// LPIT Definitions
//
#define EFI_ACPI_LOW_POWER_IDLE_TABLE_REVISION 0x1
#define EFI_ACPI_OEM_REVISION     0x00000003
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32('V','L','V','2')
#define EFI_ACPI_CREATOR_REVISION 0x0100000D  

//
// Ensure proper structure formats
//
#pragma pack(1)

typedef union _EFI_ACPI_LPI_STATE_FLAGS {
  struct {
    UINT32 Disabled           :1;
    UINT32 CounterUnavailable :1;
    UINT32 Reserved           :30;
  };
  UINT32 AsUlong;
} EFI_ACPI_LPI_STATE_FLAGS, *PEFI_ACPI_LPI_STATE_FLAGS;

// Only Mwait LPI here:

typedef struct _EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR {
  UINT32 Type;        // offset: 0
  UINT32 Length;      // offset: 4                    
  UINT16 UniqueId;    // offset: 8
  UINT8 Reserved[2]; // offset: 9
  EFI_ACPI_LPI_STATE_FLAGS Flags; // offset: 12
  EFI_ACPI_2_0_GENERIC_ADDRESS_STRUCTURE EntryTrigger; // offset: 16
  UINT32 Residency;  // offset: 28
  UINT32 Latency; // offset: 32
  EFI_ACPI_2_0_GENERIC_ADDRESS_STRUCTURE ResidencyCounter; // offset: 36 
  UINT64 ResidencyCounterFrequency; //offset: 48
} EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR;


//
// Defines for LPIT table, some are VLV specific
//


// signature "LPIT"
#define EFI_ACPI_LOW_POWER_IDLE_TABLE_SIGNATURE  0x5449504c 

#define EFI_ACPI_OEM_LPIT_REVISION                      0x00000000

#define EFI_ACPI_LOW_POWER_IDLE_MWAIT_TYPE    0x0
#define EFI_ACPI_LOW_POWER_IDLE_DEFAULT_FLAG  0x0
#define EFI_ACPI_LOW_POWER_IDLE_RES_FREQ_8K   0x8000  // 32768

//
// LPI state count (4 on VLV: S0ir, S0i1, S0i2, S0i3)
//

#define EFI_ACPI_VLV_LPI_STATE_COUNT          0x4 

//
// LPI TRIGGER (HW C7 on VLV),  
// TOFIX!!!
//
#define EFI_ACPI_VLV_LPI_TRIGGER {0x7F,0x1,0x2,0x0,0x64}

//
// LPI residency counter (MMIO)
//
#define  EFI_ACPI_VLV_LPI_RES_COUNTER0   {0x0,32,0x0,0x03,0xFED03080}
#define  EFI_ACPI_VLV_LPI_RES_COUNTER1   {0x0,32,0x0,0x03,0xFED03084}     
#define  EFI_ACPI_VLV_LPI_RES_COUNTER2   {0x0,32,0x0,0x03,0xFED03088}     
#define  EFI_ACPI_VLV_LPI_RES_COUNTER3   {0x0,32,0x0,0x03,0xFED0308C}     

//
// LPI break-even residency in us - all match S0i3 residency
// Residency estimate: Latency x 3
//
#define  EFI_ACPI_VLV_LPI_MIN_RES0   15000
#define  EFI_ACPI_VLV_LPI_MIN_RES1   15000
#define  EFI_ACPI_VLV_LPI_MIN_RES2   15000
#define  EFI_ACPI_VLV_LPI_MIN_RES3   15000

//
// LPI latency in us - all match S0i3 latency
//  
#define  EFI_ACPI_VLV_LPI_LATENCY0   5000
#define  EFI_ACPI_VLV_LPI_LATENCY1   5000
#define  EFI_ACPI_VLV_LPI_LATENCY2   5000
#define  EFI_ACPI_VLV_LPI_LATENCY3   5000


//
// LPI ID
// 
#define  EFI_ACPI_VLV_LPI_UNIQUE_ID0   0
#define  EFI_ACPI_VLV_LPI_UNIQUE_ID1   1
#define  EFI_ACPI_VLV_LPI_UNIQUE_ID2   2
#define  EFI_ACPI_VLV_LPI_UNIQUE_ID3   3

//
//  LPI ACPI table header
//


typedef struct _EFI_ACPI_LOW_POWER_IDLE_TABLE {
  EFI_ACPI_DESCRIPTION_HEADER             Header;
  EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR     LpiStates[EFI_ACPI_VLV_LPI_STATE_COUNT];
} EFI_ACPI_LOW_POWER_IDLE_TABLE;

#pragma pack()

EFI_ACPI_LOW_POWER_IDLE_TABLE Lpit = {

  //
  // Header
  //


  EFI_ACPI_LOW_POWER_IDLE_TABLE_SIGNATURE,
  sizeof (EFI_ACPI_LOW_POWER_IDLE_TABLE),
  EFI_ACPI_LOW_POWER_IDLE_TABLE_REVISION ,

  //
  // Checksum will be updated at runtime
  //
  0x00,

  //
  // It is expected that these values will be updated at runtime
  //
  ' ', ' ', ' ', ' ', ' ', ' ',

  0,
  EFI_ACPI_OEM_LPIT_REVISION,
  0,
  0,



  //
  // Descriptor
  //      
  {
    {         
      EFI_ACPI_LOW_POWER_IDLE_MWAIT_TYPE,
      sizeof(EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR),
      EFI_ACPI_VLV_LPI_UNIQUE_ID0,
      {0,0},
      {EFI_ACPI_LOW_POWER_IDLE_DEFAULT_FLAG},   // Flags 
      EFI_ACPI_VLV_LPI_TRIGGER, //EntryTrigger
      EFI_ACPI_VLV_LPI_MIN_RES0, //Residency
      EFI_ACPI_VLV_LPI_LATENCY0, //Latency
      EFI_ACPI_VLV_LPI_RES_COUNTER0, //ResidencyCounter
      EFI_ACPI_LOW_POWER_IDLE_RES_FREQ_8K //Residency counter frequency
    },
    {         
      EFI_ACPI_LOW_POWER_IDLE_MWAIT_TYPE,
      sizeof(EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR),
      EFI_ACPI_VLV_LPI_UNIQUE_ID1,
      {0,0},
      {EFI_ACPI_LOW_POWER_IDLE_DEFAULT_FLAG},   // Flags 
      EFI_ACPI_VLV_LPI_TRIGGER, //EntryTrigger
      EFI_ACPI_VLV_LPI_MIN_RES1, //Residency
      EFI_ACPI_VLV_LPI_LATENCY1, //Latency
      EFI_ACPI_VLV_LPI_RES_COUNTER1, //ResidencyCounter
      EFI_ACPI_LOW_POWER_IDLE_RES_FREQ_8K //Residency counter frequency
    },
    {         
      EFI_ACPI_LOW_POWER_IDLE_MWAIT_TYPE,
      sizeof(EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR),
      EFI_ACPI_VLV_LPI_UNIQUE_ID2,
      {0,0},
      {EFI_ACPI_LOW_POWER_IDLE_DEFAULT_FLAG},   // Flags 
      EFI_ACPI_VLV_LPI_TRIGGER, //EntryTrigger
      EFI_ACPI_VLV_LPI_MIN_RES2, //Residency
      EFI_ACPI_VLV_LPI_LATENCY2, //Latency
      EFI_ACPI_VLV_LPI_RES_COUNTER2, //ResidencyCounter
      EFI_ACPI_LOW_POWER_IDLE_RES_FREQ_8K //Residency counter frequency
    },
    {         
      EFI_ACPI_LOW_POWER_IDLE_MWAIT_TYPE,
      sizeof(EFI_ACPI_MWAIT_LPI_STATE_DESCRIPTOR),
      EFI_ACPI_VLV_LPI_UNIQUE_ID3,
      {0,0},
      {EFI_ACPI_LOW_POWER_IDLE_DEFAULT_FLAG},   // Flags 
      EFI_ACPI_VLV_LPI_TRIGGER, //EntryTrigger
      EFI_ACPI_VLV_LPI_MIN_RES3, //Residency
      EFI_ACPI_VLV_LPI_LATENCY3, //Latency
      EFI_ACPI_VLV_LPI_RES_COUNTER3, //ResidencyCounter
      EFI_ACPI_LOW_POWER_IDLE_RES_FREQ_8K //Residency counter frequency
    }
  }

};

#endif //_LPIT_H
