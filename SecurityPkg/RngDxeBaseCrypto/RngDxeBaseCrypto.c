/** @file

  Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
  This software and associated documentation
  (if any) is furnished under a license and may only be used or
  copied in accordance with the terms of the license. Except as
  permitted by such license, no part of this software or
  documentation may be reproduced, stored in a retrieval system, or
  transmitted in any form or by any means without the express
  written consent of Intel Corporation.

**/

#include <Uefi.h>
#include <Protocol/RngProtocol.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseCryptLib.h>

typedef
BOOLEAN
(EFIAPI *EFI_RNG_RANDOM_SEED) (
  IN  CONST  UINT8  *Seed  OPTIONAL,
  IN  UINTN         SeedSize
  );

typedef
BOOLEAN
(EFIAPI *EFI_RNG_RANDOM_BYTES) (
  OUT  UINT8  *Output,
  IN   UINTN  Size
  );

typedef struct {
  EFI_RNG_ALGORITHM          *Guid;
  EFI_RNG_RANDOM_SEED        RandomSeed;
  EFI_RNG_RANDOM_BYTES       RandomBytes;
  BOOLEAN                    SeedGen;
} EFI_RNG_INFO;

EFI_RNG_INFO  mRngInfo[] = {
  //
  // Comment them because current openssl-0.9.8l does not support SP800-90.
  // SP800-90 support is initial added at 04-Mar-2011.
  //
//{&gEfiRngAlgorithmSp800_90Hash256Guid, Sp800_90Hash256RamdomSeed, Sp800_90Hash256RandomBytes},
//{&gEfiRngAlgorithmSp800_90Hmac256Guid, Sp800_90Hmac256RamdomSeed, Sp800_90Hmac256RandomBytes},
//{&gEfiRngAlgorithmSp800_90Ctr256Guid,  Sp800_90Ctr256RamdomSeed,  Sp800_90Ctr256RandomBytes},
//{&gEfiRngAlgorithmX9_31_3DesGuid,      X9_31_3DesRamdomSeed,      X9_31_3DesRandomBytes},
//{&gEfiRngAlgorithmX9_31AesGuid,        X9_31AesRamdomSeed,        X9_31AesRandomBytes},
  {&gEfiRngAlgorithmRawGuid,             RandomSeed,                RandomBytes},
};

EFI_RNG_INFO *
GetRngInfo (
  IN CONST EFI_GUID              *RngAlgorithm
  )
{
  UINTN      Index;

  if (RngAlgorithm == NULL) return (&mRngInfo[0]);
  for (Index = 0; Index < sizeof(mRngInfo)/sizeof(mRngInfo[0]); Index++) {
  	DEBUG((EFI_D_ERROR, "GetRngInfo %x %x %r\n", Index, mRngInfo[Index].Guid, mRngInfo[Index].Guid));
    if (CompareGuid (RngAlgorithm, mRngInfo[Index].Guid)) {
      return &mRngInfo[Index];
    }
  }
  return NULL;
}


/**
  Returns information about the random number generation implementation.

  This function returns information about supported RNG algorithms. 
  Note: A driver need not support more than one RNG Algorithm.
 
  @param [in]     This                  Points to this instance of EFI_RNG_PROTOCOL.
  @param [in,out] RNGAlgorithmListSize  On input, the size in bytes of RNGAlgorithmList. 
                                        On output, the size in bytes of the data returned in
                                        RNGAlgorithmList (if the buffer was large enough) or the size, 
                                        in bytes, of the RNGAlgorithmList needed to obtain the array 
                                        (if the buffer was not large enough). In the latter case, 
                                        the function will return EFI_BUFFER_TOO_SMALL.
  @param [out]    RNGAlgorithmList      Buffer filled with one EFI_RNG_ALGORITHM element for each supported 
                                        RNG Algorithm. The list must not change across multiple calls to the 
                                        same driver. The first algorithm in the list is the default algorithm 
                                        for the driver.

  @retval   EFI_SUCCESS                 RNG algorithm list returned successfully. 
  @retval   EFI_UNSUPPORTED             The service is not supported by this driver.
  @retval   EFI_DEVICE_ERROR            A list of algorithms could not be retrieved due to a hardware or firmware error.
  @retval   EFI_BUFFER_TOO_SMALL        The buffer RNGAlgorithmList is too small to hold the result.
**/
EFI_STATUS
EFIAPI
BaseCryptoGetInfo (
  IN EFI_RNG_PROTOCOL       *This,
  IN OUT UINTN              *RNGAlgorithmListSize,
  OUT EFI_RNG_ALGORITHM     *RNGAlgorithmList
  )
{
  UINTN        Index;

  if (*RNGAlgorithmListSize < sizeof(mRngInfo)/sizeof(mRngInfo[0]) * sizeof(*RNGAlgorithmList)) {
    *RNGAlgorithmListSize = sizeof(mRngInfo)/sizeof(mRngInfo[0]) * sizeof(*RNGAlgorithmList);
    return EFI_BUFFER_TOO_SMALL;
  }

  for (Index = 0; Index < sizeof(mRngInfo)/sizeof(mRngInfo[0]); Index++) {
    CopyMem (&RNGAlgorithmList[Index], mRngInfo[Index].Guid, sizeof(RNGAlgorithmList[Index]));
  }

  *RNGAlgorithmListSize = sizeof(mRngInfo)/sizeof(mRngInfo[0]) * sizeof(*RNGAlgorithmList);
  return EFI_SUCCESS;
}

/**
 Produces an RNG value.
 
 This function fills the RNGValue buffer with random bytes from the specified RNG algorithm
 
 @param [in]    This                  Points to this instance of EFI_RNG_PROTOCOL.
 @param [in]    RNGAlgorithm          Points to the EFI_RNG_ALGORITHM which identifies the RNG algorithm to use. 
                                      May be NULL in which case the function will use its default RNG Algorithm.
 @param [in]    RNGValueLength        The length in bytes of the buffer pointed to by RNGValue.
 @param [out]   RNGValue              Pointer to a buffer that will hold the resulting RNG value.
 
  @retval   EFI_SUCCESS               RNG value returned successful
  @retval   EFI_UNSUPPORTED           The algorithm specified by RNGAlgorithm is not supported by this driver.
  @retval   EFI_DEVICE_ERROR          An RNG value could not be retrieved due to a hardware or firmware error.
  @retval   EFI_NOT_READY             There is no (not enough) entropy data available.
 **/
EFI_STATUS
EFIAPI
BaseCryptoGetRNG (
  IN EFI_RNG_PROTOCOL  *This,
  IN EFI_RNG_ALGORITHM *RNGAlgorithm,
  IN UINTN             RNGValueLength,
  OUT UINT8            *RNGValue
  )
{
  EFI_RNG_INFO             *RngInfo;

  RngInfo = GetRngInfo (RNGAlgorithm);
  if (RngInfo == NULL) {
    return EFI_UNSUPPORTED;
  }

  if (!RngInfo->SeedGen) {
    RngInfo->RandomSeed (NULL, 0);
    RngInfo->SeedGen = TRUE;
  }
  RngInfo->RandomBytes (RNGValue, RNGValueLength);

  return EFI_SUCCESS;
}

EFI_RNG_PROTOCOL mRngProtocol = {
  BaseCryptoGetInfo,
  BaseCryptoGetRNG
};

EFI_STATUS
EFIAPI
UefiMain(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiRngProtocolGuid,
                  &mRngProtocol,
                  NULL
                  );

  return Status;
}



