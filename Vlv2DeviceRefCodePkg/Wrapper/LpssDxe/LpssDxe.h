/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  LpssDxe.h

Abstract:


--*/

#ifndef _LPSS_DXE_H_
#define _LPSS_DXE_H_

#include <PiDxe.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/PciEnumerationComplete.h>
#include <Protocol/GlobalNvsArea.h>
#include <Protocol/PciIo.h>
#include <Protocol/PchPlatformPolicy.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/Pci22.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <PchRegs.h>
#include <pchregs/pchregslpss.h>
#include <pchregs/pchregsScc.h>
#include <ValleyView.h>
#include <VlvAccess.h>
#include <Library/IoLib.h> //EIP132398
#include <Library/S3BootScriptLib.h> //EIP132398

typedef struct _LPSS_DEVICE_INFO {
    UINTN        Segment;
    UINTN        BusNum;
    UINTN        DeviceNum;
    UINTN        FunctionNum;
    UINTN        ReportBarIndex;
    EFI_PHYSICAL_ADDRESS    ReportBar;
    UINT64                  ReportBarLen;
    UINTN                   AddrOffset;
    UINTN                   LenOffset;
    UINTN                   ReportBarIndex1;
    EFI_PHYSICAL_ADDRESS    ReportBar1;
    UINT64                  ReportBarLen1;
    UINTN                   AddrOffset1;
    UINTN                   LenOffset1;
    UINT32                  AcpiModeRegOffset;
    UINT32                  AcpiModeRegValue;
} LPSS_DEVICE_INFO;

#define GLOBAL_NVS_OFFSET(Field)    (UINTN)((CHAR8*)&((EFI_GLOBAL_NVS_AREA*)0)->Field - (CHAR8*)0)

//(CSP20130313E+)>>
#define SccMsgBusRead32(Register, Dbuff) \
{ \
    Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( EC_BASE, MC_MCR ) = (UINT32)( (PCH_SCC_EP_PRIVATE_READ_OPCODE << 24) | (PCH_SCC_EP_PORT_ID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN); \
    (UINT32)(Dbuff) = Mmio32( EC_BASE, MC_MDR ); \
}
//(CSP20130313E+)<<

#define SccMsgBus32AndThenOr( Register, Dbuff, AndData, OrData ) \
  { \
    Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( EC_BASE, MC_MCR ) = (UINT32)( (PCH_SCC_EP_PRIVATE_READ_OPCODE << 24) | (PCH_SCC_EP_PORT_ID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN); \
    (UINT32)(Dbuff) = Mmio32( EC_BASE, MC_MDR ); \
    Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( EC_BASE, MC_MDR ) = ((Dbuff & AndData) | OrData); \
    Mmio32( EC_BASE, MC_MCR ) = (UINT32)( (PCH_SCC_EP_PRIVATE_WRITE_OPCODE << 24) | (PCH_SCC_EP_PORT_ID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN); \
  }


extern EFI_GUID gLpssDummyProtocolGuid;

#endif
