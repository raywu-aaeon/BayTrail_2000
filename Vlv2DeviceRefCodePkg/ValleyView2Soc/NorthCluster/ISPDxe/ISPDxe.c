/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  IspDxe.c

Abstract:

  ISP Platform Driver


--*/


#include <ISPDxe.h>
#include <Protocol/GlobalNvsArea.h>
#ifdef ECP_FLAG
EFI_GUID gEfiGlobalNvsAreaProtocolGuid = EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID;
#else
#include <Library/DxeServicesTableLib.h>
#endif
EFI_HANDLE         mImageHandle;

///ISP reserved memory length
#define ISP_MEMORY_LENGTH    0x400000
#define ISP_MMIO_ALIGNMENT   22  //4M alignment
static EFI_ACPI_SUPPORT_PROTOCOL  *mAcpiSupport = NULL;

EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;

EFI_STATUS
UpdateResourceTemplateAslCode (
  IN     UINT32                        AslSignature,
  IN     UINT8                         MacroAmlEncoding,
  IN     UINT8                         MacroEntryNumber,
  IN     UINT8                         Offset,
  IN     VOID                          *Buffer,
  IN     UINTN                         Length
  )
/**
  @brief

  This procedure will update a Resource Descriptor Macro in
  Resrouce Template buffer list.

  @param[in] AslSignature                 - The signature of Operation Region that we want to update.
  @param[in] MacroEntryNumber             - The Resource Descriptor Macro entry number that we want to update.
  @param[in] Offset                       - The offset within Resource Descriptor Macro that we want to update.
  @param[in] Value                        - The value that we want to update.

  @retval EFI_SUCCESS                  - The function completed successfully.

**/
{
  EFI_STATUS                  Status;
  EFI_ACPI_DESCRIPTION_HEADER *Table;
  EFI_ACPI_TABLE_VERSION      Version;
  UINT8                       *CurrPtr;
  UINT8                       *Operation;
  UINT32                      *Signature;
  UINT8                       *DsdtPointer;
  UINT8                       Index;
  UINT8                       *BufferLength;
  UINTN                       Handle;
  UINT16                      AslLength;
  BOOLEAN                     EntryFound;

  ///
  /// Locate table with matching ID
  ///
  Index     = 0;
  AslLength = 0;
  EntryFound = FALSE;

  do {
    Status = mAcpiSupport->GetAcpiTable (mAcpiSupport, Index, (VOID **) &Table, &Version, &Handle);
    if (Status == EFI_NOT_FOUND) {
      break;
    }

    ASSERT_EFI_ERROR (Status);
    Index++;
  } while (Table->Signature != EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE);

  ///
  /// Point to the beginning of the DSDT table
  ///
  Index = 0;
  CurrPtr = (UINT8 *) Table;

  ///
  /// Loop through the ASL looking for values that we must fix up.
  ///
  for (DsdtPointer = CurrPtr; DsdtPointer <= (CurrPtr + ((EFI_ACPI_COMMON_HEADER *) CurrPtr)->Length); DsdtPointer++) {
    ///
    /// Get a pointer to compare for signature
    ///
    Signature = (UINT32 *) DsdtPointer;

    ///
    /// Check if this is the Device Object signature we are looking for
    ///
    if ((*Signature) == AslSignature) {
      ///
      /// Read the Device Object block length
      ///
      if (*(DsdtPointer - 2) == AML_DEVICE_OP) {
        AslLength = *(DsdtPointer - 1);
      } else if (*(DsdtPointer - 3) == AML_DEVICE_OP) {
        AslLength = *(UINT16 *) (DsdtPointer - 2);
        AslLength = (AslLength & 0x0F) + ((AslLength & 0x0FF00) >> 4);
      }

      ///
      /// Conditional match.  Search AML Encoding in Device.
      ///
      for (Operation = DsdtPointer; Operation <= DsdtPointer + AslLength; Operation++) {
        ///
        /// Look for Name Encoding
        ///
        while(Operation <= DsdtPointer + AslLength) {
          if(*Operation == AML_NAME_OP) {
            ///
            /// Found Name AML Encoding
            ///
            Operation++;
            if(*(UINT32 *)(Operation) == SIGNATURE_32 ('S', 'B', 'U', 'F')) {
              ///
              /// Found RBUF Resource Template object name
              ///
              break;
            }
          }
          Operation++;
        }

        if(Operation > DsdtPointer + AslLength ) {
#ifdef ECP_FLAG
          FreePool (Table);
#else
          gBS->FreePool (Table);
#endif
          return EFI_NOT_FOUND;
        }

        ///
        /// Now look for the Resource Template Object buffer opcode
        ///
        while((*Operation) != AML_BUFFER_OP) {
          Operation++;
          if(Operation > DsdtPointer + AslLength) {
#ifdef ECP_FLAG
            FreePool (Table);
#else
            gBS->FreePool (Table);
#endif
            return EFI_NOT_FOUND;
          }
        }

        ///
        /// In a Resource Template Object the length of buffer
        /// list is 3 bytes after the buffer opcode
        ///
        Operation += 3;
        BufferLength = (*Operation) + Operation + 1;

        ///
        /// Now look for the Macro to be updated
        ///
        while(Operation <= BufferLength) {
          if((*Operation == MacroAmlEncoding)) {
            ///
            /// We found a matching encoding however, the buffer list may have "n" number
            /// of same encoding entries. Let's narrow down to the "n"th entry.
            ///
            Index++;
            if(Index == MacroEntryNumber) {
              ///
              /// Get to the starting offset & end offset
              ///
              Operation += Offset;

              ///
              /// Fixup the value at the offset
              ///
              //EFI_DEADLOOP();
              CopyMem ((VOID *) Operation, (VOID *) (Buffer), Length);


              ///
              /// Update the modified ACPI table
              ///
              Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, Table, TRUE, Version, &Handle);
              ASSERT_EFI_ERROR (Status);
#ifdef ECP_FLAG
              FreePool (Table);
#else
              gBS->FreePool (Table);
#endif
              return Status;
            }
          }
          Operation++;
        }

        if(Operation > DsdtPointer + AslLength) {
#ifdef ECP_FLAG
          FreePool (Table);
#else
          gBS->FreePool (Table);
#endif
          return EFI_NOT_FOUND;
        }
      }
    }
  }

  return EFI_NOT_FOUND;
}



STATIC
VOID
EFIAPI
OnReadyToBootISP(
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS           Status;
  UINT32               Data32;
  EFI_PHYSICAL_ADDRESS  ISPBase;

//Avoid the event will run twice
  gBS->CloseEvent (Event);

///allocated memeory for ISP
  ISPBase = 0xFFFFFFFF;
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  ISP_MMIO_ALIGNMENT,
                  ISP_MEMORY_LENGTH,
                  &ISPBase,
                  mImageHandle,
                  NULL
                  );

  ASSERT_EFI_ERROR (Status);

//Update ISP Base in AcpiTable->ISP0
  Data32 = (UINT32)ISPBase;
  Status = UpdateResourceTemplateAslCode((SIGNATURE_32 ('I', 'S', 'P', '0')),
             AML_MEMORY32_FIXED_OP,
             1,
             0x04,
             &Data32,
             sizeof (Data32)
             );

  ASSERT_EFI_ERROR (Status);

//Update ISP Base in GlobalNvs
  DEBUG ((EFI_D_ERROR, "=============ISPBase=%x==============\n\n", ISPBase));
  GlobalNvsArea->Area->ISPAddr = (UINT32)ISPBase;
  return;
}

EFI_STATUS
EFIAPI
ISPDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Entry point for Acpi platform driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
{
  EFI_STATUS            Status;
  EFI_EVENT             Event;
  UINT32                Buffer32;

  //
  //  Locate the Global NVS Protocol.
  //
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );

  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (GlobalNvsArea->Area->ISPDevSel == 0) {
    DEBUG ((EFI_D_ERROR, "ISP Feature Disable !!\n\n"));
    return EFI_SUCCESS;
  }
// Camera clock workarounds
//vlv.ccu.plt_clk_ctrl_0 -> bit1=0; bit0=1;
//<iosfsb bar:0x0,device:0x0,function:0x0,offset:0x3c,portid:0xa9,readop:0x6,size:0x4,tap:SOCDFX_TAP2IOSFSB_STAP0,writeop:0x7,>

  PchMsgBusAndThenOr32AddToS3Save (
    0xa9, // CCU
    0x3c, // offset
    Buffer32,
    0xFFFFFFFC, // AND
    0x1,  // OR
    0x6, //PCH_LPE_PRIVATE_READ_OPCODE,
    0x7 //PCH_LPE_PRIVATE_WRITE_OPCODE
  );
//vlv.ccu.plt_clk_ctrl_1 -> bit1=0; bit0=1;
// <iosfsb bar:0x0,device:0x0,function:0x0,offset:0x40,portid:0xa9,readop:0x6,size:0x4,tap:SOCDFX_TAP2IOSFSB_STAP0,writeop:0x7,>

  PchMsgBusAndThenOr32AddToS3Save (
    0xa9, // CCU
    0x40, // offset
    Buffer32,
    0xFFFFFFFC, // AND
    0x1,  // OR
    0x6, //PCH_LPE_PRIVATE_READ_OPCODE,
    0x7 //PCH_LPE_PRIVATE_WRITE_OPCODE
  );
//vlv.ccu.plt_clk_ctrl_2 -> bit1=0; bit0=1;
// <iosfsb bar:0x0,device:0x0,function:0x0,offset:0x44,portid:0xa9,readop:0x6,size:0x4,tap:SOCDFX_TAP2IOSFSB_STAP0,writeop:0x7,>

  PchMsgBusAndThenOr32AddToS3Save (
    0xa9, // CCU
    0x44, // offset
    Buffer32,
    0xFFFFFFFC, // AND
    0x1,  // OR
    0x6, //PCH_LPE_PRIVATE_READ_OPCODE,
    0x7 //PCH_LPE_PRIVATE_WRITE_OPCODE
  );

  mImageHandle = ImageHandle;


  if (GlobalNvsArea->Area->ISPDevSel == 1) {
    Status = EfiCreateEventReadyToBootEx (
              TPL_NOTIFY,
              OnReadyToBootISP,
              NULL,
              &Event
              );

    ///
    /// Locate ACPI tables
    ///
    Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, (VOID **) &mAcpiSupport);
  }
  return Status;
}
