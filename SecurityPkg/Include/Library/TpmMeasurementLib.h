/** @file
  This library is used by other modules to measure data to TPM.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _TPM_MEASUREMENT_LIB_H_
#define _TPM_MEASUREMENT_LIB_H_

#define TPM_MEASUREMENT_LOG_FLAGS_TPM12  BIT0
#define TPM_MEASUREMENT_LOG_FLAGS_TPM20  BIT1

/**
  Measure and log data, and extend the measurement result into a specific PCR.

  @param[in]  PcrIndex          PCR Index.
  @param[in]  EventType         Event type.
  @param[in]  Event             Measurement Event.
  @param[in]  EventSize         Event type.
  @param[in]  Buffer            The content to be measured.  
  @param[in]  BufferSize        The size of content to be measured.  
  @param[in]  Flags             Bitmap providing additional information.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_UNSUPPORTED       TPM device not available.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.
**/
EFI_STATUS
EFIAPI 
TpmMeasureAndLogData (
  IN UINT32             PcrIndex,
  IN UINT32             EventType,
  IN VOID               *EventLog,
  IN UINT32             LogLen,
  IN VOID               *HashData,
  IN UINT64             HashDataLen
  );

#endif
