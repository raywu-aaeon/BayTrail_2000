/*++

Copyright (c)  1999 - 2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PciLanInfo.h

Abstract:

--*/

#ifndef _PCI_LAN_INFO_GUID_H_
#define _PCI_LAN_INFO_GUID_H_

#pragma pack(1)

// structure used for Pci Lan variable
typedef struct {
    UINT8         PciBus;
    UINT8         PciDevice;
    UINT8         PciFunction;
} PCI_LAN_INFO;

#pragma pack()

#define EFI_PCI_LAN_INFO_GUID \
  {0xd9a1427, 0xe02a, 0x437d, 0x92, 0x6b, 0xaa, 0x52, 0x1f, 0xd7, 0x22, 0xba};

extern EFI_GUID gEfiPciLanInfoGuid;

#endif
