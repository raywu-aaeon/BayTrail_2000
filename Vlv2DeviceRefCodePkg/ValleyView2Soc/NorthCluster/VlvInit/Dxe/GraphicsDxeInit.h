/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  GraphicsDxeInit.h

Abstract:

  Header file for initialization of GT PowerManagement

--*/

#ifndef _GRAPHICS_DxE_INIT_H_
#define _GRAPHICS_DXE_INIT_H_

#ifndef ECP_FLAG
#include "VlvInit.h"
#endif
#include "McAccess.h"
#ifndef ECP_FLAG
#include "MmioAccess.h"
#endif
#include "VlvAccess.h"
#ifdef SG_SUPPORT  
#include "SwitchableGraphicsInit.h"
#endif

#ifndef ECP_FLAG
#include <Library/S3BootScriptLib.h>
#include <Library/DxeServicesTableLib.h>
#endif

//
// Data definitions
//

#define VLV_X0_IGD_DEVICE_ID 0x0F30
#define VLV_A0_IGD_DEVICE_ID 0x0F31
//
// GT RELATED EQUATES
//
#define GTT_MEM_ALIGN        22
#define GTTMMADR_SIZE_4MB    0x400000

#define GT1_MOBILE_DID       0x106
#define GT2_MOBILE_DID       0x116
#define GT2P_MOBILE_DID      0x126

#define GT1_DESKTOP_DID      0x102
#define GT2_DESKTOP_DID      0x112
#define GT2P_DESKTOP_DID     0x122

#define GT1_MOBILE_DID_IVB   0x156
#define GT2_MOBILE_DID_IVB   0x166

#define GT1_DESKTOP_DID_IVB  0x152
#define GT2_DESKTOP_DID_IVB  0x162

//
#define GT_UPSERVER_WORKSTATION_DID 0x10A

#ifndef TOLUD
#define TOLUD (0xbc)
#endif

#ifndef PCI_EXPRESS_BASE_ADDRESS
#define PCI_EXPRESS_BASE_ADDRESS 0xE0000000
#endif

#define IGD_R_VID                0x00
#define IGD_R_CMD                0x04
#define IGD_R_GTTMMADR           0x10

#define IGD_R_GGC                0x50
#define IGD_R_BDSM               0x5C
#define IGD_R_BGSM               0x70
#define LockBit                  BIT0

#define PAVP_LITE_MODE           1
#define PAVP_SERPENT_MODE        2
#define PAVP_PCM_SIZE_1_MB       1
#ifndef PAVPC
#define PAVPC                    (0x74)
#endif // PAVPC
#define PAVPC_PCME_OFFSET        (0x0)
#define PAVPC_PCME_WIDTH         (0x1)
#define PAVPC_PCME_MASK          (0x1)
#define PAVPC_PCME_DEFAULT       (0x0)
#define PAVPC_PAVPE_OFFSET       (0x1)
#define PAVPC_PAVPE_WIDTH        (0x1)
#define PAVPC_PAVPE_MASK         (0x2)
#define PAVPC_PAVPE_DEFAULT      (0x0)
#define PAVPC_PAVPLCK_OFFSET     (0x2)
#define PAVPC_PAVPLCK_WIDTH      (0x1)
#define PAVPC_PAVPLCK_MASK       (0x4)
#define PAVPC_PAVPLCK_DEFAULT    (0x0)
#define PAVPC_HVYMODSEL_OFFSET   (0x3)
#define PAVPC_HVYMODSEL_WIDTH    (0x1)
#define PAVPC_HVYMODSEL_MASK     (0x8)
#define PAVPC_HVYMODSEL_DEFAULT  (0x0)
#define PAVPC_WOPCMSZ_OFFSET     (0x4)
#define PAVPC_WOPCMSZ_WIDTH      (0x2)
#define PAVPC_WOPCMSZ_MASK       (0x30)
#define PAVPC_WOPCMSZ_DEFAULT    (0x0)
#define PAVPC_PCMBASE_OFFSET     (0x14)
#define PAVPC_PCMBASE_WIDTH      (0xe)
#define PAVPC_PCMBASE_MASK       (0xfffc0000)
#define PAVPC_PCMBASE_DEFAULT    (0x0)
#define MmPciAddress_IgdSv( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )


#define MmPci32Ptr_IgdSv( Segment, Bus, Device, Function, Register ) \
  ( (volatile UINT32 *) MmPciAddress_IgdSv( Segment, Bus, Device, Function, Register ) )

#define MmPci32_IgdSv( Segment, Bus, Device, Function, Register ) \
  *MmPci32Ptr_IgdSv( Segment, Bus, Device, Function, Register )

#define MmPci32Or_IgdSv( Segment, Bus, Device, Function, Register, OrData ) \
  MmPci32_IgdSv( Segment, Bus, Device, Function, Register ) = \
    (UINT32) ( \
      MmPci32_IgdSv( Segment, Bus, Device, Function, Register ) | \
      (UINT32)(OrData) \
    )

#define MmPci32And_IgdSv( Segment, Bus, Device, Function, Register, AndData ) \
  MmPci32_IgdSv( Segment, Bus, Device, Function, Register ) = \
    (UINT32) ( \
      MmPci32_IgdSv( Segment, Bus, Device, Function, Register ) & \
      (UINT32)(AndData) \
    )


EFI_STATUS
PmInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
/*++

Routine Description:

  Initialize GT PowerManagement of SystemAgent.

Arguments:

  ImageHandle         - Handle for the image of this driver
  DxePlatformSaPolicy - SA DxePlatformPolicy protocol

Returns:

  EFI_SUCCESS         - GT Power Management initialization complete


--*/
;

EFI_STATUS
PavpInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  );
EFI_STATUS
GraphicsDxeInit (
  IN EFI_HANDLE                      ImageHandle,
  IN DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformSaPolicy
  )
/*++

Routine Description:

  Initialize GT PowerManagement of SystemAgent.

Arguments:

  ImageHandle         - Handle for the image of this driver
  DxePlatformSaPolicy - SA DxePlatformPolicy protocol

Returns:

  EFI_SUCCESS         - GT Power Management initialization complete

--*/
;

VOID
PollGtReady(
  UINT64 Base,
  UINT32 Offset,
  UINT32 Mask,
  UINT32 Result
  )
/*++

Routine Description:

  "Poll Status" for GT Readiness

Arguments:

  Base            - Base address of MMIO
  Offset          - MMIO Offset
  Mask            - Mask
  Result          - Value to wait for

Returns:

  None

--*/
;
#endif
