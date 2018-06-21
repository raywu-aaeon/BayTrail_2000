/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


--*/


#include "acpitablePlatform.h"
#include "Csrt.h"

//
// Debug Port Table
//
EFI_ACPI_CSRT_TABLE Csrt = {
  {
    //EFI_ACPI_DESCRIPTION_HEADER Start
    EFI_ACPI_5_0_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE,
    sizeof(EFI_ACPI_CSRT_TABLE),
    EFI_ACPI_CSRT_TABLE_REVISION,
    0,                          // to make sum of entire table == 0
    EFI_ACPI_OEM_ID,            // OEMID is a 6 bytes long field
    EFI_ACPI_OEM_TABLE_ID,      // OEM table identification(8 bytes long)
    EFI_ACPI_OEM_REVISION,      // OEM revision
    EFI_ACPI_CREATOR_ID,        // ASL compiler vendor ID
    EFI_ACPI_CREATOR_REVISION   // ASL compiler revision number
  },  //EFI_ACPI_DESCRIPTION_HEADER End
  {
    //
    // LPIO1 DMA   RESOURCE_GROUP_INFO1
    //
    {
      //RESOURCE_GROUP_HEADER Start
      sizeof(RESOURCE_GROUP_INFO1),// sizeof(RESOURCE_GROUP_INFO)
      0x4C544E49,
      0x00000000,
      0x9C60,   // Update per BWG Addendum A.12
      0x0000,
      0x0002,   // Update per BWG Addendum A.12
      0x0000,
      sizeof(SHARED_INFO_SECTION),
      {
        // Shared Info Section
        0x0001,         // Major Version 1
        0x0000,         // Minor Version 0
        0x55AA55AA,     // MMIO Base - Low Part
        0x00000000,     // MMIO Base - High Part
        0x0000002A,     // Interrupt GSI 42
        0x02,           // Interrupt Polarity
        0x00,           // Interrupt Mode
        0x06,           // Number of Channels
        0x20,           // DMA Address Width
        0x0000,         // Base Request Line
        0x0010,         // Number of Handshake Signals
        0x0001000     // Maximum Block Transfer Size
      },
    },  // End of Resource Group header
    {
      {
        // Controller 0
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0001,         // Resource Type
        0x20495053     // UID - SPI
      },
      {
        // Channel 0
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x30414843     // UID - CHA0
      },
      {
        // Channel 1
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x31414843     // UID - CHA1
      },
      {
        // Channel 2
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x32414843     // UID - CHA2
      },
      {
        // Channel 3
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x33414843     // UID - CHA3
      },
      {
        // Channel 4
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x34414843     // UID - CHA4
      },
      {
        // Channel 5
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x35414843     // UID - CHA5
      }
    }
  },
  {
    //
    // LPIO2 DMA   RESOURCE_GROUP_INFO2
    //
    {
      //RESOURCE_GROUP_HEADER Start
      sizeof(RESOURCE_GROUP_INFO2),// sizeof(RESOURCE_GROUP_INFO)
      0x4C544E49,
      0x00000000,
      0x9C60,   // Update per BWG Addendum A.12
      0x0000,
      0x0003,   // Update per BWG Addendum A.12
      0x0000,
      sizeof(SHARED_INFO_SECTION),
      {
        // Shared Info Section
        0x0001,         // Major Version 1
        0x0000,         // Minor Version 0
        0x55AA55AA,     // MMIO Base - Low Part
        0x00000000,     // MMIO Base - High Part
        0x0000002B,     // Interrupt GSI 43
        0x02,           // Interrupt Polarity
        0x00,           // Interrupt Mode
        0x08,           // Number of Channels
        0x20,           // DMA Address Width
        0x0010,         // Base Request Line
        0x0010,         // Number of Handshake Signals
        0x0001000     // Maximum Block Transfer Size
      },
    },  // End of Resource Group header
    {
      {
        // Controller 0
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0001,         // Resource Type
        0x20433249   // UID - I2C
      },
      {
        // Channel 0
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x30414843     // UID - CHA0
      },
      {
        // Channel 1
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x31414843     // UID - CHA1
      },
      {
        // Channel 2
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x32414843     // UID - CHA2
      },
      {
        // Channel 3
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x33414843     // UID - CHA3
      },
      {
        // Channel 4
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x34414843     // UID - CHA4
      },
      {
        // Channel 5
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x35414843     // UID - CHA5
      },
      {
        // Channel 6
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x36414843     // UID - CHA6
      },
      {
        // Channel 7
        0x0000000C,     // Resource Descriptor Length
        0x0003,         // Resource Type
        0x0000,         // Resource Type
        0x37414843     // UID - CHA7
      }
    }
  }//LPIO2 DMA   RESOURCE_GROUP_INFO2 End
};