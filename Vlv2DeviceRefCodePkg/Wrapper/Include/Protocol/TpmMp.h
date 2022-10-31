/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Tpm.h

Abstract:


--*/

#ifndef __EFI_TPM_MP_DRIVER_PROTOCOL_H__
#define __EFI_TPM_MP_DRIVER_PROTOCOL_H__


#define EFI_TPM_MP_DRIVER_PROTOCOL_GUID \
  { 0xde161cfe, 0x1e60, 0x42a1, 0x8c, 0xc3, 0xee, 0x7e, 0xf0, 0x73, 0x52, 0x12 }


EFI_FORWARD_DECLARATION(EFI_TPM_MP_DRIVER_PROTOCOL);

#define TPM_DRIVER_STATUS         0
#define TPM_DEVICE_STATUS         1

#define TPM_DRIVER_OK             0
#define TPM_DRIVER_FAILED         1
#define TPM_DRIVER_NOT_OPENED     2
#define TPM_DEVICE_OK             0
#define TPM_DEVICE_UNRECOVERABLE  1
#define TPM_DEVICE_RECOVERABLE    2
#define TPM_DEVICE_NOT_FOUND      3

//
// Prototypes for the TPM MP Driver Protocol
//

typedef
EFI_STATUS
(EFIAPI *EFI_TPM_MP_INIT)(
    IN EFI_TPM_MP_DRIVER_PROTOCOL   *This
);
/*++

  Routine Description:
    This service Open the TPM interface

  Arguments:
    This              - A pointer to the EFI_TPM_MP_DRIVER_PROTOCOL.

  Returns:
    EFI_SUCCESS       - Operation completed successfully
    EFI_DEVICE_ERROR  - The command was unsuccessful
    EFI_NOT_FOUND     - The component was not running

--*/

typedef
EFI_STATUS
(EFIAPI *EFI_TPM_MP_CLOSE)(
    IN EFI_TPM_MP_DRIVER_PROTOCOL   *This
);
/*++

  Routine Description:
    This service close the TPM interface and deactivate TPM

  Arguments:
    This              - A pointer to the EFI_TPM_MP_DRIVER_PROTOCOL.

  Returns:
    EFI_SUCCESS       - Operation completed successfully
    EFI_DEVICE_ERROR  - The command was unsuccessful
    EFI_NOT_FOUND     - The component was not running

--*/

typedef
EFI_STATUS
(EFIAPI *EFI_TPM_MP_GET_STATUS_INFO)(
    IN EFI_TPM_MP_DRIVER_PROTOCOL   *This,
    IN UINT32				                ReqStatusType,
    OUT UINT32				              *Status
);
/*++

  Routine Description:
    This service get the current status infomation of TPM

  Arguments:
    This              - A pointer to the EFI_TPM_MP_DRIVER_PROTOCOL.
    ReqStatusType	    - Requested type of status information, driver or device.
    Status	          - Pointer to the returned status.

  Returns:
    EFI_SUCCESS           - Operation completed successfully
    EFI_DEVICE_ERROR      - The command was unsuccessful
    EFI_INVALID_PARAMETER - One or more of the parameters are incorrect
    EFI_BUFFER_TOO_SMALL  - The receive buffer is too small
    EFI_NOT_FOUND         - The component was not running

--*/


typedef
EFI_STATUS
(EFIAPI *EFI_TPM_MP_TRANSMIT)(
    IN EFI_TPM_MP_DRIVER_PROTOCOL   *This,
    IN UINT8				  	            *TransmitBuffer,
    IN UINT32			  	              TransmitBufferLen,
    OUT UINT8				  	            *ReceiveBuf,
    IN OUT UINT32			  	          *ReceiveBufLen
);
/*++

  Routine Description:
    This service transmit data to the TPM and get response from TPM

  Arguments:
    This            - A pointer to the EFI_TPM_MP_DRIVER_PROTOCOL.
    TransmitBuf	    - Pointer to a buffer containing TPM transmit data.
    TransmitBufLen	- Sizeof TPM input buffer in bytes.
    ReceiveBuf	    - Pointer to a buffer containing TPM receive data.
    ReceiveBufLen	  - On input, size of TPM receive buffer in bytes.
                      On output, number of bytes written.

  Returns:
    EFI_SUCCESS           - Operation completed successfully
    EFI_DEVICE_ERROR      - The command was unsuccessful
    EFI_INVALID_PARAMETER - One or more of the parameters are incorrect
    EFI_BUFFER_TOO_SMALL  - The receive buffer is too small
    EFI_NOT_FOUND         - The component was not running

--*/


typedef struct _EFI_TPM_MP_DRIVER_PROTOCOL {
    EFI_TPM_MP_INIT			              Init;
    EFI_TPM_MP_CLOSE			            Close;
    EFI_TPM_MP_GET_STATUS_INFO 		    GetStatusInfo;
    EFI_TPM_MP_TRANSMIT		            Transmit;
} EFI_TPM_MP_DRIVER_PROTOCOL;

extern EFI_GUID gEfiTpmMpDriverProtocolGuid;

#endif
