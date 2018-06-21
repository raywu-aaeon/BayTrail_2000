/** @file
  Define the data structure and GUID of Capsule Processing Result Variable.

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  @par Revision Reference:
  GUIDs defined in UEFI 2.4 spec.
**/

#ifndef __CAPSULE_PROCESSING_RESULT_H__
#define __CAPSULE_PROCESSING_RESULT_H__

#define EFI_CAPSULE_REPORT_GUID \
  { \
    0x39b68c46, 0xf7fb, 0x441b, { 0xb6, 0xec, 0x16, 0xb0, 0xf6, 0x98, 0x21, 0xf3 } \
  }

///
/// The format of Capsule Processing Result Variable. 
///
#pragma pack(1)

typedef struct {
  //
  // Size in bytes of the variable
  //
  UINT16 VariableTotalSize;
  //
  // Guid from EFI_CAPSULE_HEADER
  //
  EFI_GUID CapsuleGuid;
  //
  // Timestamp using system time when processing completed
  //
  EFI_TIME CapsuleProcessed;
  //
  // Result of the capsule processing
  //
  EFI_STATUS CapsuleStatus;
} EFI_CAPSULE_PROCESSING_RESULT;

#pragma pack()

extern EFI_GUID gEfiCapsuleReportGuid;

#endif
