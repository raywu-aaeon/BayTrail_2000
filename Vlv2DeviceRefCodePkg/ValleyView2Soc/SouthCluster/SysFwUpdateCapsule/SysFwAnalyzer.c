/** @file
  This is the helper for SysFwUpdate DXE to analyze the layout of IFWI.

  Copyright (c) 2009 - 2013, Intel Corporation. All rights reserved.<BR>
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
#include <Library/FmiLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseCryptLib.h>
#include "FlashOperation.h"
#include "SysFwUpdate.h"

#define _SHOW_LOG_

extern EFI_SPI_PROTOCOL  *mSpiProtocol;

EFI_STATUS
SysFwVerify(UINT8 *pSysFwBuffer, UINT64 FwBufferSize, PSYS_FW_SFIH_HEADER pSfihHdr, PIFWI_AUTH_HEADER pIfwiAuthHdr)
{
  EFI_STATUS      Status = EFI_SUCCESS;
  VOID            *HashContext = NULL;
  UINTN           ContextSize;
  UINT8           Digest[SHA256_DIGEST_SIZE];
#ifndef SPI_KEY_VALIDATION
  UINT8           RsaE[] = {0x11};
  VOID            *Rsa = NULL;
  BOOLEAN         RsaVerified = FALSE;
  UINTN           SigSize = RSA1_KEY_SIZE;
#else
  VOID   *RsaModulusExponentBufferPtr = NULL;             // Pointer to the Buffer having Modulus & Exponent of RSA Key used for sigining the Manifest(Little-Endian)
  VOID   *BigEndianRsaModulusExponentBufferPtr = NULL;    // Pointer to the Buffer having Modulus & Exponent of RSA Key used for sigining the Manifest(Big-Endian)
  UINT32  RsaExponent;                                    // 4 Byte Exponent of the RSA key used for sigining the Manifest(Little-Endian)
  UINT32  BigEndianRsaExponent;                   // 4 Byte Exponent of the RSA key used for sigining the Manifest(Little-Endian)
  UINTN   KeyIndex = 0;
#endif

#ifdef _SHOW_LOG_
  UINTN           Index=0;
#endif

  UINTN IFWIBinSize = pSfihHdr->IFWILength;
  if(pSfihHdr->IFWILength <= IFWI_SFIH_HEADER_SIZE || FwBufferSize < pSfihHdr->IFWILength + IFWI_AUTH_HEADER_SIZE) {
    Status = EFI_ABORTED;
    goto _exit;
  }

  ContextSize = Sha256GetContextSize();
  HashContext = AllocatePool(ContextSize);

  if(HashContext == NULL) {
    Status =EFI_ABORTED;
    goto _exit;
  }

  Status = Sha256Init(HashContext);
  if(EFI_ERROR(Status)) {
    goto _exit;
  }
  //
  //SHA256 compare
  //
  Status = Sha256Update(HashContext, pSysFwBuffer+IFWI_AUTH_HEADER_SIZE, IFWIBinSize);
  if(EFI_ERROR(Status)) {
    goto _exit;
  }

  ZeroMem(Digest, SHA256_DIGEST_SIZE);
  Status = Sha256Final(HashContext, Digest);
  if(EFI_ERROR(Status)) {
    goto _exit;
  }

  FreePool(HashContext);
  HashContext = NULL;


  if(CompareMem(Digest, pIfwiAuthHdr->IFWIHash, SHA256_DIGEST_SIZE) != 0) {
    Status = EFI_ABORTED;
    goto _exit;
  }


#ifdef _SHOW_LOG_
  Print(L"Digest calucated:\n");
  for(Index = 0; Index < SHA256_DIGEST_SIZE; Index ++) {
    if(0 == Index % 16) Print(L"\r\n0x%04x: ",Index);
    Print(L"%02x ",Digest[Index]);
  }
  Print(L"\r\nDigest in IFWI:\n");
  for(Index = 0; Index < SHA256_DIGEST_SIZE; Index ++) {
    if(0 == Index % 16) Print(L"\r\n0x%04x: ",Index);
    Print(L"%02x ",pIfwiAuthHdr->IFWIHash[Index]);
  }
  Print(L"\r\n");
#endif

#ifndef SPI_KEY_VALIDATION
  //
  //Proceed with RSA PKCS#1 v1.5 verification of Sfih
  //
  Rsa = RsaNew();
  if(Rsa == NULL) {
    Status = EFI_ABORTED;
    goto _exit;
  }
  Status = RsaSetKey(Rsa, RsaKeyN, pIfwiAuthHdr->IFWISignature, RSA1_KEY_SIZE);
  if(EFI_ERROR(Status)) {
#ifdef _SHOW_LOG_
    Print(L"Failed to set N.\r\n");
#endif
    Status = EFI_ABORTED;
    goto _exit;
  }

  Status = RsaSetKey(Rsa, RsaKeyE, RsaE, sizeof(RsaE));
  if(EFI_ERROR(Status)) {
#ifdef _SHOW_LOG_
    Print(L"Failed to set E.\r\n");
#endif
    Status = EFI_ABORTED;
    goto _exit;
  }

  RsaVerified = RsaPkcs1Verify(Rsa, Digest, SHA256_DIGEST_SIZE, pIfwiAuthHdr->IFWISignature+256, SigSize);
  if(!RsaVerified) {
#ifdef _SHOW_LOG_
    Print(L"Failed RsaPkcs1Verify.\r\n");
#endif
    Status = EFI_ABORTED;
    goto _exit;
  } else {
    Status = EFI_SUCCESS;
  }
#else
  RsaModulusExponentBufferPtr = AllocatePool(sizeof(SECURE_BOOT_MANIFEST_MODULUS_EXPONENT_FIELDS));
  if(RsaModulusExponentBufferPtr == NULL) {
    DEBUG((EFI_D_ERROR, "In SysFwVerify: Failed to get the Memory for Modulus & Exponent of RSA Key.\n"));
    Status = EFI_ABORTED;
    goto _exit;
  }

  BigEndianRsaModulusExponentBufferPtr = AllocatePool(sizeof(SECURE_BOOT_MANIFEST_MODULUS_EXPONENT_FIELDS));
  if(BigEndianRsaModulusExponentBufferPtr == NULL) {
    DEBUG((EFI_D_ERROR, "In SysFwVerify: Failed to get the Memory for Modulus & Exponent of RSA Key(Big-Endian).\n"));
    Status = EFI_ABORTED;
    goto _exit;
  }

  // Locate SPI Protocol
  Status = gBS->LocateProtocol( &gEfiSpiProtocolGuid, NULL, (VOID **)&mSpiProtocol);
  if(!EFI_ERROR(Status)) {
    Status = mSpiProtocol->Execute( 
                             mSpiProtocol,
                             SPI_READ,
                             SPI_WREN,
                             TRUE,
                             TRUE,
                             FALSE,
                             VLV_MANIFEST_ADDRESS + sizeof(SECURE_BOOT_MANIFEST_SIGNED_FIELDS),
                             sizeof(SECURE_BOOT_MANIFEST_MODULUS_EXPONENT_FIELDS),
                             RsaModulusExponentBufferPtr,
                             EnumSpiRegionAll
                             );
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "In SysFwVerify: Error in Reading RSA Modulus & Exponent from SPI\n"));
      Status = EFI_ABORTED;
      goto _exit;
    }
  } else {
    DEBUG((EFI_D_ERROR, "In SysFwVerify: Unable to Locate SPI Protocol %d:\n", Status));
    Status = EFI_ABORTED;
    goto _exit;
  }

  // Change the Endianess of RSA Modulus as FMI expect that as Big-Endian
  for(KeyIndex = 0; KeyIndex < RSA1_KEY_SIZE; KeyIndex ++) {
    *((UINT8*)BigEndianRsaModulusExponentBufferPtr + KeyIndex) = *((UINT8*)RsaModulusExponentBufferPtr + (RSA1_KEY_SIZE -1) - KeyIndex);
  }

  RsaExponent = ((SECURE_BOOT_MANIFEST_MODULUS_EXPONENT_FIELDS *)RsaModulusExponentBufferPtr)->Exponent;

  // Change the Endianess of RSA Exponent as FMI expect that as Big-Endian
  BigEndianRsaExponent = ((RsaExponent>>24)&0xff) | ((RsaExponent<<8)&0xff0000) | ((RsaExponent>>8)&0xff00) | ((RsaExponent<<24)&0xff000000);

  Status  = FmiInit();
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "In SysFwVerify: Failed FmiInit %d\n", Status));
    Status = EFI_ABORTED;
    goto _exit;
  }

  Status = FmiRSAVerify(PKCS_VERSION_1_5, (UINT8*)BigEndianRsaModulusExponentBufferPtr, BigEndianRsaExponent, pIfwiAuthHdr->IFWISignature+256, Digest);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "In SysFwVerify: Failed FmiRSAVerify %d\n", Status));
    Status = EFI_ABORTED;
    goto _exit;
  }
#endif

_exit:
  if(HashContext != NULL) {
    FreePool(HashContext);
    HashContext = NULL;
  }
#ifndef SPI_KEY_VALIDATION
  if(Rsa != NULL) {
    RsaFree(Rsa);
    Rsa = NULL;
  }
#else
  if(RsaModulusExponentBufferPtr != NULL) {
    FreePool(RsaModulusExponentBufferPtr);
    RsaModulusExponentBufferPtr = NULL;
  }

  if(BigEndianRsaModulusExponentBufferPtr != NULL) {
    FreePool(BigEndianRsaModulusExponentBufferPtr);
    BigEndianRsaModulusExponentBufferPtr = NULL;
  }
  FreeFmiResource();
#endif
  return Status;

}


EFI_STATUS
GetSysFwLayOutInfo(UINT8 *pSysFwBuffer, UINT64 FwBufferSize, PSYS_FW_SFIH_HEADER pSfihHdr, PIFWI_AUTH_HEADER pIfwiAuthHdr)
{
  EFI_STATUS Status = EFI_SUCCESS;

#ifndef _BY_PASS_VERIFICATION
  UINT64 SfihSignature = SIGNATURE_32('S','F','I','H');
#endif
  //
  //the buffer must contain at least AUTH header and SFIH header
  //
  if(pSysFwBuffer == NULL || FwBufferSize < IFWI_AUTH_HEADER_SIZE + IFWI_SFIH_HEADER_SIZE) {
    Status = EFI_BAD_BUFFER_SIZE;
    goto _exit;
  }
  CopyMem (pIfwiAuthHdr, pSysFwBuffer, IFWI_AUTH_HEADER_SIZE);
  CopyMem (pSfihHdr, pSysFwBuffer+IFWI_AUTH_HEADER_SIZE, IFWI_SFIH_HEADER_SIZE);

#ifdef _SHOW_LOG_
  Print(L"SFIH header content: ------------------\n");
  Print(L"Signature: 0x%x:\n", pSfihHdr->Signature);
  Print(L"IFWILength: 0x%x:\n", pSfihHdr->IFWILength);
  Print(L"IBBOffset: 0x%x:\n", pSfihHdr->IBBOffset);
  Print(L"SecondStgOffset: 0x%x:\n", pSfihHdr->SecondStgOffset);
  Print(L"SecUpdOffset: 0x%x:\n", pSfihHdr->SecUpdOffset);
#endif
  //
  //Check if the SFIH header is with valid SFIH signature
  //


#ifndef _BY_PASS_VERIFICATION
  if(pSfihHdr->Signature != SfihSignature) {
    Status = EFI_UNSUPPORTED;
    goto _exit;
  }

  Status = SysFwVerify(pSysFwBuffer, FwBufferSize, pSfihHdr, pIfwiAuthHdr);
#endif

#ifdef _SHOW_LOG_
  Print(L"Rsa Verify status:%r: ------------------\n", Status);

#endif

_exit:
  return Status;
}

