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

#ifndef __RNG_H__
#define __RNG_H__

#define EFI_RNG_SERVICE_BINDING_PROTOCOL_GUID \
  {0xe417a4a2, 0x0843, 0x4619, 0xbf, 0x11, 0x5c, 0xe8, 0x2a, 0xfc, 0xfc, 0x59}

#define EFI_RNG_PROTOCOL_GUID \
  {0x3152bca5, 0xeade, 0x433d, 0x86, 0x2e, 0xc0, 0x1c, 0xdc, 0x29, 0x1f, 0x44}

typedef struct _EFI_RNG_PROTOCOL EFI_RNG_PROTOCOL;

typedef EFI_GUID EFI_RNG_ALGORITHM;

#define EFI_RNG_ALGORITHM_SP800_90_HASH_256_GUID \
  {0xa7af67cb, 0x603b, 0x4d42, 0xba, 0x21, 0x70, 0xbf, 0xb6, 0x29, 0x3f, 0x96}

#define EFI_RNG_ALGORITHM_SP800_90_HMAC_256_GUID \
  {0xc5149b43, 0xae85, 0x4f53, 0x99, 0x82, 0xb9, 0x43, 0x35, 0xd3, 0xa9, 0xe7}

#define EFI_RNG_ALGORITHM_SP800_90_CTR_256_GUID \
  {0x44f0de6e, 0x4d8c, 0x4045, 0xa8, 0xc7, 0x4d, 0xd1, 0x68, 0x85, 0x6b, 0x9e}

#define EFI_RNG_ALGORITHM_X9_31_3DES_GUID \
  {0x63c4785a, 0xca34, 0x4012, 0xa3, 0xc8, 0x0b, 0x6a, 0x32, 0x4f, 0x55, 0x46}

#define EFI_RNG_ALGORITHM_X9_31_AES_GUID \
  {0xacd03321, 0x777e, 0x4d3d, 0xb1, 0xc8, 0x20, 0xcf, 0xd8, 0x88, 0x20, 0xc9}

#define EFI_RNG_ALGORITHM_RAW \
  {0xe43176d7, 0xb6e8, 0x4827, 0xb7, 0x84, 0x7f, 0xfd, 0xc4, 0xb6, 0x85, 0x61}

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
typedef
EFI_STATUS
(EFIAPI *EFI_RNG_GET_INFO) (
  IN EFI_RNG_PROTOCOL       *This,
  IN OUT UINTN              *RNGAlgorithmListSize,
  OUT EFI_RNG_ALGORITHM     *RNGAlgorithmList
  );


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
typedef
EFI_STATUS
(EFIAPI *EFI_RNG_GET_RNG) (
  IN EFI_RNG_PROTOCOL  *This,
  IN EFI_RNG_ALGORITHM *RNGAlgorithm,
  IN UINTN             RNGValueLength,
  OUT UINT8            *RNGValue
  );

///
///  RNG driver interface
///
struct _EFI_RNG_PROTOCOL {
  EFI_RNG_GET_INFO GetInfo;
  EFI_RNG_GET_RNG  GetRNG;
};

///
/// Global variable containing the Rng protocol GUID
///
extern EFI_GUID gEfiRngProtocolGuid;
extern EFI_GUID gEfiRngServiceBindingProtocolGuid;

extern EFI_GUID gEfiRngAlgorithmSp800_90Hash256Guid;
extern EFI_GUID gEfiRngAlgorithmSp800_90Hmac256Guid;
extern EFI_GUID gEfiRngAlgorithmSp800_90Ctr256Guid;
extern EFI_GUID gEfiRngAlgorithmX9_31_3DesGuid;
extern EFI_GUID gEfiRngAlgorithmX9_31AesGuid;
extern EFI_GUID gEfiRngAlgorithmRawGuid;

#endif
