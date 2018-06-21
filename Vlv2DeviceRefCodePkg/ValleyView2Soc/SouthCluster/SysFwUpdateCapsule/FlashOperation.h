/*++

This file contains a 'Sample Header File' and is licensed as such
under the terms of your license agreement with Intel or your
vendor.  This file may be modified by the user, subject to
the additional terms of the license agreement

--*/

/*++
Copyright (c)  2008-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

Abstract:

*/
#ifndef _FLASH_OP_H
#define _FLASH_OP_H

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>


#define SECTOR_SIZE_4KB   0x1000      // Common 4kBytes sector size
#define MAX_FWH_SIZE      0x00300000  // 8Mbit (Note that this can also be used for the 4Mbit )

//
// Prefix Opcode Index on the host SPI controller
//
typedef enum {
  SPI_WREN,             // Prefix Opcode 0: Write Enable
  SPI_EWSR,             // Prefix Opcode 1: Enable Write Status Register
} PREFIX_OPCODE_INDEX;

//
// Opcode Menu Index on the host SPI controller
//
typedef enum {
  SPI_READ_ID,        // Opcode 0: READ ID, Read cycle with addressary
  SPI_READ,           // Opcode 1: READ, Read cycle with address
  SPI_RDSR,           // Opcode 2: Read Status Register, No address
  SPI_WRDI,           // Opcode 3: Write Disable, No address
  SPI_SERASE,         // Opcode 4: Sector Erase (4KB), Write cycle with address
  SPI_BERASE,         // Opcode 5: Block Erase (32KB), Write cycle with address
  SPI_PROG,           // Opcode 6: Byte Program, Write cycle with address
  SPI_WRSR,           // Opcode 7: Write Status Register, No address
} SPI_OPCODE_INDEX;

//
// Serial Flash Status Register definitions
//
#define SF_SR_BUSY        0x01      // Indicates if internal write operation is in progress
#define SF_SR_WEL         0x02      // Indicates if device is memory write enabled
#define SF_SR_BP0         0x04      // Block protection bit 0
#define SF_SR_BP1         0x08      // Block protection bit 1
#define SF_SR_BP2         0x10      // Block protection bit 2
#define SF_SR_BP3         0x20      // Block protection bit 3
#define SF_SR_WPE         0x3C      // Enable write protection on all blocks
#define SF_SR_AAI         0x40      // Auto Address Increment Programming status
#define SF_SR_BPL         0x80      // Block protection lock-down


BOOLEAN
FlashReadCompare (
  IN  UINTN BaseAddress,
  IN  UINT8 *Byte,
  IN  UINTN Length,
  IN  SPI_REGION_TYPE  SpiRegionType
  );



EFI_STATUS
FlashErase (
  IN  UINTN  BaseAddress,
  IN  UINTN NumBytes,
  IN  SPI_REGION_TYPE  SpiRegionType
  );


EFI_STATUS
FlashWrite (
  IN  UINTN DstBufferPtr,
  IN  UINT8 *Byte,
  IN  UINTN Length,
  IN  SPI_REGION_TYPE   SpiRegionType
  );


EFI_STATUS BIOSVerify(
  IN  UINTN                   FileSize,
  IN  UINT8                   *FileBuffer,
  IN  SPI_REGION_TYPE         SpiRegionType
  );

EFI_STATUS BIOSFlash(
  IN  UINTN                   FileSize,
  IN  UINT8                   *FileBuffer,
  IN  SPI_REGION_TYPE         SpiRegionType
  );

EFI_STATUS BIOSErase(
  IN  UINTN                   FileSize,
  IN  SPI_REGION_TYPE         SpiRegionType
  );

EFI_STATUS BIOSVerifyEx(
  IN  UINTN                   FileSize,
  IN  UINT8                   *FileBuffer,
  IN  SPI_REGION_TYPE         SpiRegionType,
  IN  UINTN                   RegionOffset
  );

EFI_STATUS BIOSFlashEx(
  IN UINTN             BufSize,
  IN UINT8             *BufferPtr,
  IN SPI_REGION_TYPE   SpiRegionType,
  IN UINTN             RegionOffset  //Region offset is the offset from beginning of the region.
);

UINTN
GetEraseSize(
  CHAR16 *str
);

#endif
