/*++

Copyright (c)  1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AmtBxLoader.h

Abstract:


--*/

#ifndef __EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL_H__
#define __EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL_H__


#define EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL_GUID \
 {0x45b6f49, 0x9102, 0x4590, 0xa0, 0xa5, 0x35, 0x31, 0x1b, 0xa, 0xef, 0xc3}


//EFI_FORWARD_DECLARATION (EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL);


//
// Prototypes for the Amt BIOS Extensions Loader Driver Protocol
//

typedef
EFI_STATUS
(EFIAPI *EFI_VERIFY_SIGNATURE)(
    IN UINT8  *PublicKeyBuffer,
    IN UINT32 PublicKeyBufferSize,
    IN UINT8  *PublicKeySignatureBuffer
);

typedef struct _EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL {
    EFI_VERIFY_SIGNATURE                                VerifySignature;
} EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL;

extern EFI_GUID gEfiDpsdRSA1024AndSHA256SignatureVerificationProtocolGuid;

#endif
