/** @file
  This is the module file for fault tolerant system firmware update driver implementation

  Copyright (c) 2009 - 2012, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Protocol/Spi.h>
#include <Library/PcdLib.h>
#include <Library/ShellLib.h>
#include <Library/DebugLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>

#include <Library/DriverLib.h>

#include <Protocol/I2cBus.h>

#include "FlashOperation.h"
#include "FotaFwUpdate.h"
#include "FotaUpdateCommon.h"


//#define _SHOW_LOG_

extern EFI_SPI_PROTOCOL  *mSpiProtocol;
UINT8  ReadBuffer[2];   //Buffer to receive data from ULPMC.
UINT8  WriteBuffer[2];  //Buffer to send cmd to ULPMC.

BOOLEAN
FlashReadCompare (
  IN  UINTN BaseAddress,
  IN  UINT8 *Byte,
  IN  UINTN Length,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
  EFI_STATUS          Status = EFI_SUCCESS;
  UINT32              SectorSize;
  UINT32              SpiAddress;
  UINT8                Buffer[SECTOR_SIZE_4KB];

  SpiAddress = (UINT32)(UINTN)(BaseAddress);
  SectorSize = SECTOR_SIZE_4KB;

  while ( (Length > 0) && (Length <= MAX_FWH_SIZE) ) {
    Status = mSpiProtocol->Execute (
                             mSpiProtocol,
                             SPI_READ,
                             SPI_WREN,
                             TRUE,
                             TRUE,
                             FALSE,
                             (UINT32) SpiAddress,
                             SectorSize,
                             Buffer,
                             SpiRegionType
                             );

    if (EFI_ERROR (Status)) {
#ifdef _SHOW_LOG_
      Print(L"Read SPI ROM Failed [%08x]\n", SpiAddress);
#endif
      return FALSE;
    }
    SpiAddress += SectorSize;
    Length   -= SectorSize;

    if (CompareMem(Buffer, Byte, SECTOR_SIZE_4KB)) {
      return FALSE;
    }
  }

  return TRUE;
}


EFI_STATUS
FlashErase (
  IN  UINTN  BaseAddress,
  IN  UINTN NumBytes,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
  EFI_STATUS          Status = EFI_SUCCESS;
  UINT32              SectorSize;
  UINT32              SpiAddress;

  SpiAddress = (UINT32)(UINTN)(BaseAddress);
  SectorSize = SECTOR_SIZE_4KB;
  while ( (NumBytes > 0) && (NumBytes <= MAX_FWH_SIZE) ) {
    Status = mSpiProtocol->Execute (
                             mSpiProtocol,
                             SPI_SERASE,
                             SPI_WREN,
                             FALSE,
                             TRUE,
                             FALSE,
                             (UINT32) SpiAddress,
                             0,
                             NULL,
                             SpiRegionType
                             );
    if (EFI_ERROR (Status)) {
      break;
    }
    SpiAddress += SectorSize;
    NumBytes   -= SectorSize;
  }

  return Status;
}

EFI_STATUS
FlashRead (
  IN  UINTN             BaseAddress,
  IN  UINT8             *Byte,
  IN  UINTN             Length,
  IN  SPI_REGION_TYPE   SpiRegionType
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
  UINT32                SectorSize;
  UINT32                SpiAddress;
  UINT8                 Buffer[SECTOR_SIZE_4KB];

  SpiAddress = (UINT32)(UINTN)(BaseAddress);
  SectorSize = SECTOR_SIZE_4KB;

  Status = mSpiProtocol->Execute (
                           mSpiProtocol,
                           SPI_READ,
                           SPI_WREN,
                           TRUE,
                           TRUE,
                           FALSE,
                           (UINT32) SpiAddress,
                           SectorSize,
                           Buffer,
                           SpiRegionType
                           );

  if (EFI_ERROR (Status)) {
#ifdef _SHOW_LOG_
    Print(L"Read SPI ROM Failed [%08x]\n", SpiAddress);
#endif
    return Status;
  }

  CopyMem (Byte, (void *)Buffer, Length);

  return Status;
}


EFI_STATUS
FlashWrite (
  IN  UINTN DstBufferPtr,
  IN  UINT8 *Byte,
  IN  UINTN Length,
  IN  SPI_REGION_TYPE   SpiRegionType
  )
{
  EFI_STATUS                Status;
  UINT32                    NumBytes = (UINT32)Length;
  UINT8*                    pBuf8 = Byte;
  UINT32                    SpiAddress;

  SpiAddress = (UINT32)(UINTN)(DstBufferPtr);
  Status = mSpiProtocol->Execute (
                           mSpiProtocol,
                           SPI_PROG,
                           SPI_WREN,
                           TRUE,
                           TRUE,
                           TRUE,
                           (UINT32)SpiAddress,
                           NumBytes,
                           pBuf8,
                           SpiRegionType
                           );
  return Status;
}


EFI_STATUS BIOSVerify(
  IN  UINTN                   FileSize,
  IN  UINT8                   *FileBuffer,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
  UINTN              DataIndex;
  BOOLEAN         Flag = TRUE;

  for (DataIndex = 0; DataIndex < FileSize; DataIndex += SECTOR_SIZE_4KB) {
    if(FlashReadCompare(DataIndex, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType)) {
      continue;
    }
#ifdef _SHOW_LOG_
    Print(L"Verifying... %d%% Completed   Error at [%08x].  \n", (DataIndex * 100 / FileSize), DataIndex);
#endif
    Flag = FALSE;
    break;
  }
#ifdef _SHOW_LOG_
  Print(L"Flash Verify Complete. ");
#endif
  if(Flag) {
#ifdef _SHOW_LOG_
    Print(L"It's same...!!\n");
#endif
  }

  else {
#ifdef _SHOW_LOG_
    Print(L"It's different as show...!!\n");
#endif
  }
  return EFI_SUCCESS;
}

EFI_STATUS BIOSVerifyEx(
  IN  UINTN                   FileSize,
  IN  UINT8                   *FileBuffer,
  IN  SPI_REGION_TYPE  SpiRegionType,
  IN  UINTN            RegionOffset
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  UINTN              DataIndex;
  BOOLEAN         Flag = TRUE;

  for (DataIndex = 0; DataIndex < FileSize; DataIndex += SECTOR_SIZE_4KB) {
    if(FlashReadCompare(DataIndex + RegionOffset, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType)) {
      continue;
    }
#ifdef _SHOW_LOG_
    Print(L"Verifying... %d%% Completed   Error at [%08x].  \n", (DataIndex * 100 / FileSize), DataIndex);
#endif
    Flag = FALSE;
    break;
  }

  Status = Flag?EFI_SUCCESS:EFI_ABORTED;

  return Status;
}

EFI_STATUS BIOSFlash(
  IN  UINTN                   FileSize,
  IN  UINT8                   *FileBuffer,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
  UINTN                    DataIndex;

  for (DataIndex = 0; DataIndex < FileSize; DataIndex += SECTOR_SIZE_4KB) {
    if(FlashReadCompare(DataIndex, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType)) {
      continue;
    }
    FlashErase(DataIndex, SECTOR_SIZE_4KB, SpiRegionType);
    FlashWrite(DataIndex, FileBuffer + DataIndex, SECTOR_SIZE_4KB, SpiRegionType);
  }

  return EFI_SUCCESS;
}

/**
Extended version of BIOS flash that allows Flash specific SPI region providing a 4K-aligned offset.
**/
EFI_STATUS BIOSFlashEx(
  IN UINTN             BufSize,
  IN UINT8             *BufferPtr,
  IN SPI_REGION_TYPE   SpiRegionType,
  IN UINTN             RegionOffset  //Region offset is the offset from beginning of the region.
  )
{
  EFI_STATUS         Status = EFI_SUCCESS;
  UINTN              DataIndex;

  if((RegionOffset&0xFFF)!=0) {
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }
  for(DataIndex = 0; DataIndex < BufSize; DataIndex += SECTOR_SIZE_4KB) {


    if(FlashReadCompare(DataIndex + RegionOffset, BufferPtr + DataIndex, SECTOR_SIZE_4KB, SpiRegionType)) {
      //
      //Skips over unchanged bytes
      //
      continue;
    }

    Status = FlashErase(DataIndex+RegionOffset, SECTOR_SIZE_4KB, SpiRegionType);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "In BIOSFlashEx: Failed to erase 4K blocks.\n"));
#ifdef _SHOW_LOG_
      Print(L"In BIOSFlashEx: failed to erase 4K blocks.\n");
#endif
      return Status;
    }

    Status=FlashWrite(DataIndex+RegionOffset, BufferPtr + DataIndex, SECTOR_SIZE_4KB, SpiRegionType);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "In BIOSFlashEx: Failed to write 4K blocks.\n"));
      return Status;
    }
  }

  return Status;
}

EFI_STATUS BIOSErase(
  IN  UINTN                   FileSize,
  IN  SPI_REGION_TYPE  SpiRegionType
  )
{
  UINTN                    DataIndex;

  for (DataIndex = 0; DataIndex < FileSize; DataIndex += SECTOR_SIZE_4KB) {
    FlashErase(DataIndex, SECTOR_SIZE_4KB, SpiRegionType);
  }

  return EFI_SUCCESS;
}




UINTN
GetEraseSize(
  CHAR16 *str
  )
{
  UINTN Size = 0x100000;
  UINTN temp = 1;
  temp = str[0] - '0';
  Size = Size * temp;
  return Size;
}



EFI_STATUS
PreUpdateCheck(UINT8 *PwrStatus)
{
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE *HandleArray = NULL;
  UINTN HandleArrayCount = 0;
  UINTN Index = 0;

  EFI_I2C_BUS_PROTOCOL *I2cBusProtocol = NULL;
  EFI_I2C_REQUEST_PACKET Request;
  CHAR8 AcpiID[I2C_ACPI_ID_LEN+1];

  UINTN FCap=0;
  UINTN RCap=0;

  if(PwrStatus == NULL) {
    return EFI_ABORTED;
  }
  *PwrStatus = PRE_CHECK_PASS;

  //
  //Compose the device path to be check by DevicePath lib.
  //
  AsciiStrCpy(AcpiID, DID_ACPI_ID_PREFIX);
  AcpiID[4] = '0'+ ULPMC_I2C_CONTROLLER_ID;
  AsciiStrCpy(AcpiID+5, DID_ACPI_ID_SUFFIX);
  AsciiStrCpy(AcpiID+11, DID_ACPI_ID_SUFFIX_400K);

  Status = gBS->LocateHandleBuffer( ByProtocol,
                  &gEfiI2cBusProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray);
  if(EFI_ERROR(Status)) {
#ifdef _SHOW_LOG_
    DEBUG((EFI_D_ERROR, "Failed to locate i2c bus protocol.\n"));
#endif
    Status = EFI_ABORTED;
    goto _exit;
  }

  for ( Index = 0; HandleArrayCount > Index; Index ++ ) {
    //
    //  Determine if the device is available
    //
    if ( NULL != DlAcpiFindDeviceWithMatchingCid ( HandleArray [ Index ],
         0,
         (CONST CHAR8 *)AcpiID
         )) {
      //
      //  The device was found
      //
      Status = gBS->OpenProtocol ( HandleArray [ Index ],
                      &gEfiI2cBusProtocolGuid,
                      (VOID **)&I2cBusProtocol,
                      NULL,
                      NULL,
                      EFI_OPEN_PROTOCOL_GET_PROTOCOL );
      break;
    }
  }
  //
  //  Done with the handle array
  //
  gBS->FreePool ( HandleArray );

  if(NULL == I2cBusProtocol) {
    DEBUG((EFI_D_ERROR, "Failed to locate i2c device.\n"));
    Status = EFI_ABORTED;
    goto _exit;
  }
  //
  //TODO:Check power source. If on AC, pass directly.
  //If not on AC, and no battery.then return PRE_CHECK_NO_AC.


  ReadBuffer[0] = ReadBuffer[1] =0;

  WriteBuffer[0] = ULPMC_FG_REMAINCAP_CMD_0;

  Request.ReadBytes = 2;
  Request.ReadBuffer = &ReadBuffer[0];
  Request.WriteBytes=1;
  Request.WriteBuffer = &WriteBuffer[0];
  Request.Timeout = I2C_TIMEOUT_DEFAULT;

  //Send request to FG for remaining capacity.
  Status = I2cBusProtocol->StartRequest ( I2cBusProtocol,
                             ULPMC_I2C_SLAVE_ADDR|0x400,
                             NULL,
                             &Request,
                             NULL );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get remaining capacity from ULPMC.\n"));
    Status = EFI_ABORTED;
    goto _exit;
  }

  RCap = (ReadBuffer[1]<<8)|ReadBuffer[0];

  ReadBuffer[0] = ReadBuffer[1] =0;

  WriteBuffer[0] = ULPMC_FG_FULLCAP_CMD_0;

  Request.ReadBytes = 2;
  Request.ReadBuffer = &ReadBuffer[0];
  Request.WriteBytes=1;
  Request.WriteBuffer = &WriteBuffer[0];
  Request.Timeout = I2C_TIMEOUT_DEFAULT;

  //Send request to FG for full capacity.
  Status = I2cBusProtocol->StartRequest ( I2cBusProtocol,
                             ULPMC_I2C_SLAVE_ADDR|0x400,
                             NULL,
                             &Request,
                             NULL );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get remaining capacity from ULPMC.\n"));
    goto _exit;
  }

  FCap = (ReadBuffer[1]<<8)|ReadBuffer[0];
  if( 0 == FCap) {
    Status = EFI_ABORTED;
    goto _exit;
  }
#ifdef _SHOW_LOG_
  Print(L"Fuel guage detection- remaining capactiy: %d%%\n", RCap*100/FCap);
#endif

  //Calculate the battery level to ensure it's not lower than 25%.
  if((RCap * 100 / FCap) < 25) {
    *PwrStatus = PRE_CHECK_LOW_BAT;
    Status = EFI_NOT_READY;
    goto _exit;
  }

_exit:
  return Status;
}

