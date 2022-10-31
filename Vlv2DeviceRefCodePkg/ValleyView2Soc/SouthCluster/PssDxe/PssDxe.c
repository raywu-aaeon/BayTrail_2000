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

  PssDxe.c

Abstract:

  PSS DXE Driver

--*/

#include "PssDxe.h"
#include "Key.h"
#include <Setup.h>    // AMI_OVERRIDE - EIP140009 Support Pss
#include <Library/BaseCryptLib.h>
#include <Protocol/DataHub.h>
#include <Guid/PlatformInfo.h>

PSS_DRIVER_CONTEXT      *DriverContext;
EFI_EVENT               mReadyToBootEvent;
UINTN                   gRow,gColumn;
EFI_PLATFORM_INFO_HOB   *mPlatformInfo = NULL;

BOOLEAN   mModeInitialized = FALSE;
//
// Boot video resolution and text mode.
//
UINT32    mBootHorizontalResolution    = 0;
UINT32    mBootVerticalResolution      = 0;
UINT32    mBootTextModeColumn          = 0;
UINT32    mBootTextModeRow             = 0;
//
// BIOS setup video resolution and text mode.
//
UINT32    mSetupTextModeColumn         = 0;
UINT32    mSetupTextModeRow            = 0;
UINT32    mSetupHorizontalResolution   = 0;
UINT32    mSetupVerticalResolution     = 0;


BOOLEAN
StringToArray(
  CONST CHAR16 *pString,
  UINT8  *pValue
  )
{
  UINT8     Value;
  UINT8     Digit;
  UINT8     Index;

  //
  // Convert the value to hex
  //
  while ( 0 != *pString ) {
    Value = 0;

    for (Index = 0; Index < 2; Index ++) {
      //
      // Upper case the digit
      //
      Digit = (UINT8)(* pString);
      if ( 'a' <= Digit ) {
        Digit &= ~ 0x20;
      }

      //
      // Validate the digit
      //
      if (( '0' <= Digit ) && ( '9' >= Digit )) {
        Digit -= '0';
      } else if (( 'A' <= Digit ) && ( 'F' >= Digit )) {
        Digit -= 'A' - 10;
      }

      Value <<= 4;
      Value += Digit;
      *pValue = Value;
      pString += 1;
    }

    //
    // Point to next Value
    //
    pValue += 1;
  }

  return TRUE;
}

EFI_STATUS
EFIAPI
SetConsoleMode (
  BOOLEAN  IsSetupMode
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsOutput;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL       *SimpleTextOut;
  UINTN                                 SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;
  UINT32                                MaxGopMode = 0;
  UINT32                                MaxTextMode = 0;
  UINT32                                ModeNumber;
  UINT32                                NewHorizontalResolution;
  UINT32                                NewVerticalResolution;
  UINT32                                NewColumns;
  UINT32                                NewRows;
  UINTN                                 HandleCount;
  EFI_HANDLE                            *HandleBuffer;
  EFI_STATUS                            Status;
  UINTN                                 Index;
  UINTN                                 CurrentColumn;
  UINTN                                 CurrentRow;
  UINTN                                 BootTextColumn;
  UINTN                                 BootTextRow;

  MaxGopMode  = 0;
  MaxTextMode = 0;

  //
  // Get current video resolution and text mode
  //
  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID**)&GraphicsOutput
                  );
  if (EFI_ERROR (Status)) {
    GraphicsOutput = NULL;
  }

  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiSimpleTextOutProtocolGuid,
                  (VOID**)&SimpleTextOut
                  );
  if (EFI_ERROR (Status)) {
    SimpleTextOut = NULL;
  }

  if ((GraphicsOutput == NULL) || (SimpleTextOut == NULL)) {
    return EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_ERROR, "IsSetupMode = %d\n",IsSetupMode));

  //
  //Need to initialze for fast boot
  //
  if (!mModeInitialized) {
    if (GraphicsOutput != NULL) {
      //
      // Get current video resolution and text mode.
      //
      mBootHorizontalResolution = GraphicsOutput->Mode->Info->HorizontalResolution;
      mBootVerticalResolution   = GraphicsOutput->Mode->Info->VerticalResolution;
    }

    if (SimpleTextOut != NULL) {
      Status = SimpleTextOut->QueryMode (
                                SimpleTextOut,
                                SimpleTextOut->Mode->Mode,
                                &BootTextColumn,
                                &BootTextRow
                                );
      mBootTextModeColumn = (UINT32)BootTextColumn;
      mBootTextModeRow    = (UINT32)BootTextRow;
    }

    //
    // Get user defined text mode for setup.
    //
    mSetupHorizontalResolution = PcdGet32 (PcdSetupVideoHorizontalResolution);
    mSetupVerticalResolution   = PcdGet32 (PcdSetupVideoVerticalResolution);
    mSetupTextModeColumn       = PcdGet32 (PcdSetupConOutColumn);
    mSetupTextModeRow          = PcdGet32 (PcdSetupConOutRow);

    mModeInitialized           = TRUE;
  }

  if (IsSetupMode) {
    //
    // The requried resolution and text mode is setup mode.
    //
    NewHorizontalResolution = mSetupHorizontalResolution;
    NewVerticalResolution   = mSetupVerticalResolution;
    NewColumns              = mSetupTextModeColumn;
    NewRows                 = mSetupTextModeRow;
  } else {
    //
    // The required resolution and text mode is boot mode.
    //
    NewHorizontalResolution = mBootHorizontalResolution;
    NewVerticalResolution   = mBootVerticalResolution;
    NewColumns              = mBootTextModeColumn;
    NewRows                 = mBootTextModeRow;
  }

  if (GraphicsOutput != NULL) {
    MaxGopMode  = GraphicsOutput->Mode->MaxMode;
  }

  if (SimpleTextOut != NULL) {
    MaxTextMode = SimpleTextOut->Mode->MaxMode;
  }

  //
  // 1. If current video resolution is same with required video resolution,
  //    video resolution need not be changed.
  //    1.1. If current text mode is same with required text mode, text mode need not be changed.
  //    1.2. If current text mode is different from required text mode, text mode need be changed.
  // 2. If current video resolution is different from required video resolution, we need restart whole console drivers.
  //
  for (ModeNumber = 0; ModeNumber < MaxGopMode; ModeNumber++) {
    Status = GraphicsOutput->QueryMode (
                               GraphicsOutput,
                               ModeNumber,
                               &SizeOfInfo,
                               &Info
                               );
    if (!EFI_ERROR (Status)) {
      if ((Info->HorizontalResolution == NewHorizontalResolution) &&
          (Info->VerticalResolution == NewVerticalResolution)) {
        if ((GraphicsOutput->Mode->Info->HorizontalResolution == NewHorizontalResolution) &&
            (GraphicsOutput->Mode->Info->VerticalResolution == NewVerticalResolution)) {
          //
          // Current resolution is same with required resolution, check if text mode need be set
          //
          Status = SimpleTextOut->QueryMode (SimpleTextOut, SimpleTextOut->Mode->Mode, &CurrentColumn, &CurrentRow);
          ASSERT_EFI_ERROR (Status);
          if (CurrentColumn == NewColumns && CurrentRow == NewRows) {
            //
            // If current text mode is same with required text mode. Do nothing
            //
            FreePool (Info);
            return EFI_SUCCESS;
          } else {
            //
            // If current text mode is different from requried text mode.  Set new video mode
            //
            for (Index = 0; Index < MaxTextMode; Index++) {
              Status = SimpleTextOut->QueryMode (SimpleTextOut, Index, &CurrentColumn, &CurrentRow);
              if (!EFI_ERROR(Status)) {
                if ((CurrentColumn == NewColumns) && (CurrentRow == NewRows)) {
                  //
                  // Required text mode is supported, set it.
                  //
                  Status = SimpleTextOut->SetMode (SimpleTextOut, Index);
                  ASSERT_EFI_ERROR (Status);
                  //
                  // Update text mode PCD.
                  //
                  PcdSet32 (PcdConOutColumn, mSetupTextModeColumn);
                  PcdSet32 (PcdConOutRow, mSetupTextModeRow);
                  FreePool (Info);
                  return EFI_SUCCESS;
                }
              }
            }
            if (Index == MaxTextMode) {
              //
              // If requried text mode is not supported, return error.
              //
              FreePool (Info);
              return EFI_UNSUPPORTED;
            }
          }
        } else {
          //
          // If current video resolution is not same with the new one, set new video resolution.
          // In this case, the driver which produces simple text out need be restarted.
          //
          Status = GraphicsOutput->SetMode (GraphicsOutput, ModeNumber);
          if (!EFI_ERROR (Status)) {
            FreePool (Info);
            break;
          }
        }
      }
      FreePool (Info);
    }
  }

  if (ModeNumber == MaxGopMode) {
    //
    // If the resolution is not supported, return error.
    //
    return EFI_UNSUPPORTED;
  }

  //
  // Set PCD to Inform GraphicsConsole to change video resolution.
  // Set PCD to Inform Consplitter to change text mode.
  //
  PcdSet32 (PcdVideoHorizontalResolution, NewHorizontalResolution);
  PcdSet32 (PcdVideoVerticalResolution, NewVerticalResolution);
  PcdSet32 (PcdConOutColumn, NewColumns);
  PcdSet32 (PcdConOutRow, NewRows);

  DEBUG ((EFI_D_ERROR, "NewHorizontalResolution = %d, NewVerticalResolution = %d \n",NewHorizontalResolution,NewVerticalResolution ));
  DEBUG ((EFI_D_ERROR, "PcdVideoHorizontalResolution = %d, PcdVideoVerticalResolution = %d \n",  PcdGet32 (PcdVideoHorizontalResolution),PcdGet32 (PcdVideoVerticalResolution) ));

  //
  // Video mode is changed, so restart graphics console driver and higher level driver.
  // Reconnect graphics console driver and higher level driver.
  // Locate all the handles with GOP protocol and reconnect it.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleTextOutProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleCount; Index++) {
      gBS->DisconnectController (HandleBuffer[Index], NULL, NULL);
    }
    for (Index = 0; Index < HandleCount; Index++) {
      gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
    }
    if (HandleBuffer != NULL) {
      FreePool (HandleBuffer);
    }
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
DrawPopUp (
  )
{
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *ConOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE      SavedConsoleMode;
  UINTN                            Rows;
  UINTN                            Columns;
  UINTN                            Row;
  UINTN                            Column;
  UINTN                            NumberOfRow;
  UINTN                            NumberOfColumn;
  UINTN                            Length;
  CHAR16                           *Line1;
  CHAR16                           *Line2;
  CHAR16                           *StringBuffer;

  DEBUG ((EFI_D_ERROR, "DrawPopUp...\n"));

  //
  // Cache a pointer to the Simple Text Output Protocol in the EFI System Table
  //
  ConOut = gST->ConOut;

  //
  // Save the current console cursor position and attributes
  //
  CopyMem (&SavedConsoleMode, ConOut->Mode, sizeof (SavedConsoleMode));

  //
  // Retrieve the number of columns and rows in the current console mode
  //
  ConOut->QueryMode (ConOut, SavedConsoleMode.Mode, &Columns, &Rows);
  ConOut->ClearScreen (gST->ConOut);

  //
  // Enable cursor and set the foreground and background colors specified by Attribute
  //
  ConOut->EnableCursor (ConOut, TRUE);
  ConOut->SetAttribute (ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE);

  //
  // Compute the starting row and starting column for the popup
  //
  NumberOfRow    = PUBLIC_KEY_SETUP_SIZE/MAX_INLINE_COUNT + 5;
  NumberOfColumn = MAX_INLINE_COUNT*4 + 4;

  Row    = (Rows - NumberOfRow)/2;
  Column = (Columns - NumberOfColumn)/2;

  //
  // Allocate a buffer for a single line of the popup with borders and a Null-terminator
  //
  Line1 = AllocateZeroPool ((NumberOfColumn + 1) * sizeof (CHAR16));
  ASSERT (Line1 != NULL);

  //
  // Draw top of popup box
  //
  SetMem16 (Line1, NumberOfColumn * 2, BOXDRAW_HORIZONTAL);
  Line1[0]                  = BOXDRAW_DOWN_RIGHT;
  Line1[NumberOfColumn - 1] = BOXDRAW_DOWN_LEFT;
  Line1[NumberOfColumn]     = L'\0';
  ConOut->SetCursorPosition (ConOut, Column, Row++);
  ConOut->OutputString (ConOut, Line1);

  // Draw middle of the popup with strings
  StringBuffer = AllocateZeroPool (MAX_STRING_LEN * sizeof (CHAR16));
  ASSERT(StringBuffer != NULL);
  StrCpy (StringBuffer, L"Enter PSS Verification Key");
  Length = StrLen (StringBuffer);

  SetMem16 (Line1, NumberOfColumn * 2, L' ');
  Line1[0]                  = BOXDRAW_VERTICAL;
  Line1[NumberOfColumn - 1] = BOXDRAW_VERTICAL;
  Line1[NumberOfColumn]     = L'\0';
  CopyMem (Line1 + 1 + (NumberOfColumn - Length) / 2, StringBuffer , Length * sizeof (CHAR16));
  ConOut->SetCursorPosition (ConOut, Column, Row++);
  ConOut->OutputString (ConOut, Line1);

  //Draw vertical lines for popup
  while ((NumberOfRow - 3) > 0) {
    SetMem16 (Line1, NumberOfColumn * 2, L' ');
    Line1[0]                  = BOXDRAW_VERTICAL;
    Line1[NumberOfColumn - 1] = BOXDRAW_VERTICAL;
    Line1[NumberOfColumn]     = L'\0';
    ConOut->SetCursorPosition (ConOut, Column, Row++);
    ConOut->OutputString (ConOut, Line1);
    NumberOfRow--;
  }

  // Draw bottom of popup box
  SetMem16 (Line1, NumberOfColumn * 2, BOXDRAW_HORIZONTAL);
  Line1[0]                  = BOXDRAW_UP_RIGHT;
  Line1[NumberOfColumn - 1] = BOXDRAW_UP_LEFT;
  Line1[NumberOfColumn]     = L'\0';
  ConOut->SetCursorPosition (ConOut, Column, Row++);
  ConOut->OutputString (ConOut, Line1);

  // Free the allocated line buffer
  FreePool (Line1);
  FreePool (StringBuffer);

  //
  // Draw top of inside box
  //
  ConOut->SetAttribute (ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

  NumberOfRow    = PUBLIC_KEY_SETUP_SIZE/MAX_INLINE_COUNT;
  NumberOfColumn = MAX_INLINE_COUNT*4;

  Line2 = AllocateZeroPool ((NumberOfColumn*4 + 1) * sizeof (CHAR16));
  ASSERT (Line2 != NULL);

  // These are for inside popup
  Row    = Row - NumberOfRow - 2;
  Column = Column + 2;

  gRow    = Row;
  gColumn = Column;

  // Draw top of inside popup box
  while ( NumberOfRow > 0) {
    SetMem16 (Line2, NumberOfColumn * 2, L' ');
    Line2[0]              = L' ';
    Line2[NumberOfColumn] = L'\0';

    ConOut->SetCursorPosition (ConOut, Column, Row++);
    ConOut->OutputString (ConOut, Line2);
    NumberOfRow--;
  }

  // Free the allocated buffer
  FreePool (Line2);

  //
  // Set the cursor position
  //
  ConOut->SetCursorPosition (ConOut, gColumn, gRow);
}

EFI_STATUS
ReadKeyStrokes(
  OUT  EFI_INPUT_KEY        *Key
  )
{
  EFI_STATUS                Status;
  //EFI_EVENT                 TimerEvent;
  //EFI_EVENT                 WaitList[2];
  //UINTN                     Index;
  EFI_TPL                   OldTpl;

  OldTpl = gBS->RaiseTPL (TPL_APPLICATION);

  do {
    /*
    do {
      Status = gBS->CreateEvent (EFI_EVENT_TIMER, 0, NULL, NULL, &TimerEvent);

      //
      // Set a timer event of 1 second expiration
      //
      gBS->SetTimer (
          TimerEvent,
          TimerRelative,
          ONE_SECOND
          );

      //
      // Wait for the keystroke event or the timer
      //
      WaitList[0] = gST->ConIn->WaitForKey;
      WaitList[1] = TimerEvent;
      Status      = gBS->WaitForEvent (2, WaitList, &Index);

      //
      // Check for the timer expiration
      //
      if (!EFI_ERROR (Status) && Index == 1) {
        Status = EFI_TIMEOUT;
      }

      gBS->CloseEvent (TimerEvent);
    } while (Status == EFI_TIMEOUT);
    */
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
  } while (EFI_ERROR(Status));

  gBS->RestoreTPL (OldTpl);

  return Status;
}

EFI_STATUS
ReadStrings(
  IN CHAR16                 *StringPtr
  )
{
  EFI_STATUS                Status;
  UINTN                     Index;
  CHAR16                    KeyPad[2];
  EFI_INPUT_KEY             Key;
  CHAR16                    *BufferedString;
  UINTN                     Column;
  UINTN                     Row;

  Row    = gRow;
  Column = gColumn;

  BufferedString = AllocateZeroPool((MAX_INLINE_COUNT*4 + 1)* sizeof (CHAR16));
  ASSERT (BufferedString);

  do {
    Status = ReadKeyStrokes(&Key);
    if (!(((Key.UnicodeChar >= L'0') && (Key.UnicodeChar <= L'9')) ||
          ((Key.UnicodeChar >= L'a') && (Key.UnicodeChar <= L'f')) ||
          ((Key.UnicodeChar >= L'A') && (Key.UnicodeChar <= L'F')) ||
          (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) ||
          (Key.UnicodeChar == CHAR_BACKSPACE) ||
          (Key.ScanCode == SCAN_ESC))) {
      //
      // Invalid Key
      //
      continue;
    }

    switch (Key.UnicodeChar) {
      case CHAR_NULL:
        switch (Key.ScanCode) {
          case SCAN_ESC:
            FreePool(BufferedString);
            BufferedString = NULL;
            return EFI_DEVICE_ERROR;
            break;

          default:
            break;
        }
        break;

      case CHAR_CARRIAGE_RETURN:
        FreePool(BufferedString);
        BufferedString = NULL;
        return EFI_SUCCESS;
        break;

      case CHAR_BACKSPACE:
        if (StringPtr[0] != CHAR_NULL) {
          for (Index = 0; (StringPtr[Index] != CHAR_NULL) && (Index < PUBLIC_KEY_SETUP_SIZE*2); Index++) {
          }
          //
          // Effectively truncate string by 1 character
          //
          StringPtr[Index - 1] = L'\0';
        }

      default:
        //
        // If it is the beginning of the string, don't worry about checking maximum limits
        //
        if ((StringPtr[0] == CHAR_NULL)&&(Key.UnicodeChar!= CHAR_BACKSPACE)) {
          StrnCpy (StringPtr, &Key.UnicodeChar, 1);
        } else if (((StrLen(StringPtr)) < PUBLIC_KEY_SETUP_SIZE*2)&&(Key.UnicodeChar!= CHAR_BACKSPACE)) {
          KeyPad[0] = Key.UnicodeChar;
          KeyPad[1] = CHAR_NULL;
          StrCat (StringPtr, KeyPad);
        }

        for(Index =0; Index<MAX_INLINE_COUNT*4; Index++) {
          BufferedString[Index] = L' ';
        }
        gST->ConOut->SetCursorPosition (gST->ConOut, Column, Row);

        for(Index =0; Index<StrLen(StringPtr) - (Row - gRow)*MAX_INLINE_COUNT*2; Index += 2) {
          BufferedString[Index*2] = *(StringPtr + (Row - gRow)*MAX_INLINE_COUNT*2 + Index);
          if (*(StringPtr + (Row - gRow)*MAX_INLINE_COUNT*2 + Index + 1) != 0) {
            BufferedString[Index*2 + 1] = *(StringPtr + (Row - gRow)*MAX_INLINE_COUNT*2 + Index + 1);
            BufferedString[Index*2 + 2] = L'h';
            BufferedString[Index*2 + 3] = L' ';
          }
        }
        gST->ConOut->OutputString (gST->ConOut, BufferedString);

        //
        // Jump to Next Line
        //
        if ((((StrLen(StringPtr) + MAX_INLINE_COUNT*2) % (MAX_INLINE_COUNT*2)) == 0) &&
            (Row > gRow) && (Key.UnicodeChar == CHAR_BACKSPACE)) {
          Row --;
        } else if ((((StrLen(StringPtr) + MAX_INLINE_COUNT*2) % (MAX_INLINE_COUNT*2)) == 0) &&
            (Row < (gRow + PUBLIC_KEY_SETUP_SIZE/MAX_INLINE_COUNT - 1)) && (Key.UnicodeChar != CHAR_BACKSPACE)) {
          Row ++;
        }
        break;
    }

    gST->ConOut->SetCursorPosition (
                   gST->ConOut,
                   Column + (StrLen(StringPtr) - (Row - gRow)*MAX_INLINE_COUNT*2)*2 - (StrLen(StringPtr) - (Row - gRow)*MAX_INLINE_COUNT*2)%2,
                   Row
                   );
  } while(TRUE);

  FreePool (BufferedString);
  BufferedString = NULL;

  return Status;
}


EFI_STATUS
EFIAPI
ReadPssData (
  UINT8     *Buffer,
  UINT32    Address,
  UINT32    Size
  )
{
  EFI_STATUS                Status;
  EFI_I2C_REQUEST_PACKET    Request;
  UINT8                     WriteBuffer[1];  //Buffer to send cmd to PSS.

  if (DriverContext == NULL || (DriverContext->I2cBusProtocol==NULL)) {
    return EFI_NOT_READY;
  }

  if(Size == 0) {
    return EFI_SUCCESS;
  }

  //
  // Prepare the request parameter
  //
  WriteBuffer[0] = (UINT8)Address;

  Request.ReadBytes = Size;
  Request.ReadBuffer = Buffer;
  Request.WriteBytes = 1;
  Request.WriteBuffer = &WriteBuffer[0];
  Request.Timeout = I2C_TIMEOUT_DEFAULT;

  Status = DriverContext->I2cBusProtocol->StartRequest (
                                            DriverContext->I2cBusProtocol,
                                            (PSS_I2C_SLAVE_ADDR + Address/0x100) | 0x400,
                                            NULL,
                                            &Request,
                                            NULL
                                            );

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to Read Data PSS.\n"));
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
GetTID(
  UINT8                 *Buffer
  )
{
  EFI_STATUS                Status;
  EFI_I2C_REQUEST_PACKET    Request;
  UINT8                     WriteBuffer[1];  //Buffer to send cmd to PSS.
  UINT8                     ReadBuffer[24];  //24 byte read for 2K; 12 byte for 8K part.
  UINT32                     BaseAddress;

  if ((DriverContext == NULL) || (DriverContext->I2cBusProtocol==NULL) || (Buffer == NULL)) {
    return EFI_NOT_READY;
  }

  if(DriverContext->Is2KPart) {
    //
    // Prepare the request parameter
    //
    BaseAddress = BASE_ADDRESS_TID_2K;
    WriteBuffer[0] = (UINT8)BaseAddress;

    Request.ReadBytes = 24;
    Request.ReadBuffer = &ReadBuffer[0];
    Request.WriteBytes = 1;
    Request.WriteBuffer = &WriteBuffer[0];
    Request.Timeout = I2C_TIMEOUT_DEFAULT;
  } else {
    BaseAddress = BASE_ADDRESS_TID_8K;
    WriteBuffer[0] = (UINT8)BaseAddress;

    Request.ReadBytes = 12;
    Request.ReadBuffer = &ReadBuffer[0];
    Request.WriteBytes = 1;
    Request.WriteBuffer = &WriteBuffer[0];
    Request.Timeout = I2C_TIMEOUT_DEFAULT;
  }

  Status = DriverContext->I2cBusProtocol->StartRequest (
                                            DriverContext->I2cBusProtocol,
                                            (PSS_I2C_SLAVE_ADDR + BaseAddress/0x100) | 0x400,
                                            NULL,
                                            &Request,
                                            NULL
                                            );

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to Read TID Bank.\n"));
    return EFI_ABORTED;

  }

  if(DriverContext->Is2KPart) {
    CopyMem(Buffer+4, &ReadBuffer[22], 2);
    CopyMem(Buffer, &ReadBuffer[0],4);
  } else {
    CopyMem(Buffer, &ReadBuffer[6], 6);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
GetPublicKey (
  IN OUT UINT8  *PublicKey,
  IN     UINTN  *VarSize
  )
{
  EFI_STATUS        Status;

  Status = gRT->GetVariable(
                  PUBLIC_KEY_SETUP_NAME,
                  &gEfiNormalSetupGuid,
                  NULL,
                  VarSize,
                  PublicKey
                  );
  //
  //Public key needs to be 128-byte long.
  //
  if(!EFI_ERROR(Status)) {
    if(*VarSize!=128) {
      DEBUG((EFI_D_ERROR, "Invalid public key.\n"));
      Status = EFI_ABORTED;
    }
  }

  return Status;
}


UINT16
Monzax_Detect_Chip (
  VOID
  )
{
  EFI_STATUS    Status;
  UINT8         PssData[4];

  //
  // Read the chip's Class ID from the TID bank, it should be 0xE2 (Gen2)
  // And the TID Module should be 0x140
  // Check whether the PSS IC is Monza X-2K
  //
  Status = ReadPssData(&PssData[0], BASE_ADDRESS_CLASSID_2K, 4);
  if (!(EFI_ERROR(Status)) &&
      (PssData[0] == 0xE2) &&
      ((PssData[2] & 0x0F) == 0x01) &&
      (PssData[3] == 0x40)
     ) {
    //
    // Monza X-2K
    //
    return MonzaX_2K_Dura;
  }

  //
  // Read the chip's Class ID from the TID bank, it should be 0xE2 (Gen2)
  // And the TID Module should be 0x150
  // Check whether the PSS IC is Monza X-8K
  //
  Status = ReadPssData(&PssData[0], BASE_ADDRESS_CLASSID_8K, 4);
  if (!(EFI_ERROR(Status)) &&
      (PssData[0] == 0xE2) &&
      ((PssData[2] & 0x0F) == 0x01) &&
      (PssData[3] == 0x50)
     ) {
    //
    // Monza X-8K
    //
    return MonzaX_8K_Dura;
  }

  return 0;
}


EFI_STATUS
EFIAPI
PssDriverInit()
{
  EFI_STATUS                    Status;
  EFI_I2C_BUS_PROTOCOL          *I2cBusProtocol = NULL;
  SETUP_DATA                    mSystemConfiguration;     // AMI_OVERRIDE - EIP140009 Support Pss
  UINTN                         VarSize = 0;
  UINT16                        PssModule;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
  EFI_HANDLE                    *HandleArray = NULL;
  UINTN                         HandleArrayCount = 0;
  UINTN                         Index = 0;
  CHAR8                         AcpiID[I2C_ACPI_ID_LEN + 1];

  DriverContext = AllocatePool(sizeof(PSS_DRIVER_CONTEXT));
  if(DriverContext == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    return Status;
  }

  DriverContext->I2cBusProtocol = NULL;
  DriverContext->PssVerified = FALSE;

  //
  // Compose the device path to be check by DevicePath lib.
  //
  AsciiStrCpy(AcpiID, DID_ACPI_ID_PREFIX);
  AcpiID[4] = '0'+ PSS_I2C_CONTROLLER_ID;
  AsciiStrCpy(AcpiID+5, DID_ACPI_ID_SUFFIX);
  AsciiStrCpy(AcpiID+11, DID_ACPI_ID_SUFFIX_400K);

  Status = gBS->LocateHandleBuffer(
                  ByProtocol,
                  &gEfiI2cBusProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to locate I2C bus protocol.\n"));
    return Status;
  }

  for ( Index = 0; HandleArrayCount > Index; Index ++ ) {
    //
    // Determine if the device is available
    //
    if ( NULL != DlAcpiFindDeviceWithMatchingCid ( HandleArray [ Index ],
                    0,
                    (CONST CHAR8 *)AcpiID
                  )) {
      //
      // The device was found
      //
      Status = gBS->OpenProtocol (
                      HandleArray [Index],
                      &gEfiI2cBusProtocolGuid,
                      (VOID **)&I2cBusProtocol,
                      NULL,
                      NULL,
                      EFI_OPEN_PROTOCOL_GET_PROTOCOL
                      );
      if(!EFI_ERROR(Status)) {
        break;
      }
    }
  }

  //
  // Done with the handle array
  //
  gBS->FreePool (HandleArray);

  if (I2cBusProtocol == NULL) {
    DEBUG((EFI_D_ERROR, "Failed to locate i2c device.\n"));
    return Status;
  }

  DriverContext->I2cBusProtocol = I2cBusProtocol;

  //
  // Detect PSS Chip
  //
  PssModule = Monzax_Detect_Chip();
  if (PssModule == MonzaX_2K_Dura) {
    DEBUG((EFI_D_ERROR, "PSS is a MonzaX_2K_Dura\n"));
    DriverContext->Is2KPart = TRUE;
  } else if (PssModule == MonzaX_8K_Dura) {
    DEBUG((EFI_D_ERROR, "PSS is a MonzaX_8K_Dura\n"));
    DriverContext->Is2KPart = FALSE;
  } else {
    //
    // The PSS doesn't exist, system will be shutdown.
    //
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  if (!EFI_ERROR(Status)) {
    if (PssModule == MonzaX_2K_Dura) {
      GlobalNvsArea->Area->PssDeveice = 1;
    } else if (PssModule == MonzaX_8K_Dura) {
      GlobalNvsArea->Area->PssDeveice = 2;
    } else {
      GlobalNvsArea->Area->PssDeveice = 0;
    }
  }

  //
  // Check the Setup setting
  //
  VarSize = sizeof(SYSTEM_CONFIGURATION);

  Status = gRT->GetVariable(L"Setup",
                            &gEfiNormalSetupGuid,
                            NULL,
                            &VarSize,
                            &mSystemConfiguration);

  if (mSystemConfiguration.PssEnabled == 0) {
    DriverContext->PssEnabled = FALSE;
  } else {
    DriverContext->PssEnabled = TRUE;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
VerifyKey(
  IN  UINT8                 *VerificationKey
  )
{
  EFI_STATUS                Status;
  BOOLEAN                   Verified = FALSE;
  BOOLEAN                   Set = FALSE;
  UINT8                     PublicKey[PUBLIC_KEY_SETUP_SIZE];
  UINTN                     PublicKeySize;
  UINT8                     PssTID[6]= {0,0,0,0,0,0};
  UINT8                     HashValue[SHA256_DIGEST_SIZE];
  UINTN                     HashSize;
  UINTN                     CtxSize;
  VOID                      *Rsa = NULL;
  VOID                      *Sha256Ctx = NULL;
  UINT8 DefaultUnlockPublicKey[PUBLIC_KEY_SETUP_SIZE]= {
    0xBB, 0xF8, 0x2F, 0x09, 0x06, 0x82, 0xCE, 0x9C, 0x23, 0x38, 0xAC, 0x2B, 0x9D, 0xA8, 0x71, 0xF7,
    0x36, 0x8D, 0x07, 0xEE, 0xD4, 0x10, 0x43, 0xA4, 0x40, 0xD6, 0xB6, 0xF0, 0x74, 0x54, 0xF5, 0x1F,
    0xB8, 0xDF, 0xBA, 0xAF, 0x03, 0x5C, 0x02, 0xAB, 0x61, 0xEA, 0x48, 0xCE, 0xEB, 0x6F, 0xCD, 0x48,
    0x76, 0xED, 0x52, 0x0D, 0x60, 0xE1, 0xEC, 0x46, 0x19, 0x71, 0x9D, 0x8A, 0x5B, 0x8B, 0x80, 0x7F,
    0xAF, 0xB8, 0xE0, 0xA3, 0xDF, 0xC7, 0x37, 0x72, 0x3E, 0xE6, 0xB4, 0xB7, 0xD9, 0x3A, 0x25, 0x84,
    0xEE, 0x6A, 0x64, 0x9D, 0x06, 0x09, 0x53, 0x74, 0x88, 0x34, 0xB2, 0x45, 0x45, 0x98, 0x39, 0x4E,
    0xE0, 0xAA, 0xB1, 0x2D, 0x7B, 0x61, 0xA5, 0x1F, 0x52, 0x7A, 0x9A, 0x41, 0xF6, 0xC1, 0x68, 0x7F,
    0xE2, 0x53, 0x72, 0x98, 0xCA, 0x2A, 0x8F, 0x59, 0x46, 0xF8, 0xE5, 0xFD, 0x09, 0x1D, 0xBD, 0xCB
  };
  UINT8 RsaE[] = {0x11};

  //
  // Get Public Key, if error, use default public key.
  //
  PublicKeySize = PUBLIC_KEY_SETUP_SIZE;
  Status = GetPublicKey(
             &PublicKey[0],
             &PublicKeySize
             );
  DEBUG((EFI_D_ERROR, "PublicKey: %x\n", PublicKey));
  if ((EFI_ERROR(Status)) || (PublicKey == 0)) {
    //
    // Public Key hasn't been set, Use Default key.
    //
    DEBUG((EFI_D_ERROR, "Failed to get public key.\n"));
    CopyMem(PublicKey, DefaultUnlockPublicKey, PUBLIC_KEY_SETUP_SIZE);
  }

  //
  // Get TID
  //
  ZeroMem(PssTID, sizeof(PssTID));

  Status = GetTID(&PssTID[0]);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get TID.\n"));
    goto _verify_failure;
  } else {
    DEBUG((EFI_D_ERROR,"TID is:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\r\n",
           PssTID[0], PssTID[1], PssTID[2], PssTID[3], PssTID[4], PssTID[5]));
  }

  //
  // Sha Hash Init
  //
  RandomSeed(NULL, 0);

  HashSize = SHA256_DIGEST_SIZE;
  ZeroMem (HashValue, HashSize);
  CtxSize = Sha256GetContextSize ();
  Sha256Ctx = AllocatePool (CtxSize);

  if(Sha256Ctx == NULL) {
    goto _verify_failure;
  }

  Status = Sha256Init (Sha256Ctx);
  if (!Status) {
    if(Sha256Ctx != NULL) {
      FreePool(Sha256Ctx);
    }
    goto _verify_failure;
  }

  Status = Sha256Update (Sha256Ctx, PssTID, sizeof(PssTID));
  if (!Status) {
    if(Sha256Ctx != NULL) {
      FreePool(Sha256Ctx);
    }
    goto _verify_failure;
  }

  Status = Sha256Final (Sha256Ctx, HashValue);
  if (!Status) {
    if(Sha256Ctx != NULL) {
      FreePool(Sha256Ctx);
    }
    goto _verify_failure;
  }

  FreePool (Sha256Ctx);

  //
  // Generate & Initialize RSA Context.
  //
  Rsa = RsaNew();
  if(Rsa == NULL) {
    DEBUG((EFI_D_ERROR, "Failed to generate Rsa context.\r\n"));
    Status = EFI_ABORTED;
    goto _verify_failure;
  }

  Set = RsaSetKey(Rsa, RsaKeyN, PublicKey, sizeof(PublicKey));
  if(Set == FALSE) {
    DEBUG((EFI_D_ERROR, "Failed to Set PSS Public KeyN.\r\n"));
    Status = EFI_ABORTED;
    goto _verify_failure;
  }

  Set = RsaSetKey(Rsa, RsaKeyE, RsaE, sizeof(RsaE));
  if(Set == FALSE) {
    DEBUG((EFI_D_ERROR, "Failed to Set PSS Public KeyE.\r\n"));
    Status = EFI_ABORTED;
    goto _verify_failure;
  }

  //
  // Verify Key
  //
  Verified = RsaPkcs1Verify (Rsa, HashValue, HashSize, VerificationKey, PUBLIC_KEY_SETUP_SIZE);
  if(Verified == FALSE) {
    DEBUG((EFI_D_ERROR, "Failed to Set PSS Public KeyN.\r\n"));
    Status = EFI_ABORTED;
  }

_verify_failure:
  if(Rsa != NULL) {
    RsaFree(Rsa);
  }

  if (!Verified) {
    //
    // The PSS key not match, system will be shutdown.
    //
    DriverContext->PssVerified = FALSE;
    DEBUG((EFI_D_ERROR, "PSS verification failed.\r\n"));
  } else {
    DriverContext->PssVerified = TRUE;
    DEBUG((EFI_D_ERROR, "PSS verification succeed.\r\n"));
  }

  return Status;
}


VOID
EFIAPI
OnReadyToBoot(
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
{
  PSS_DRIVER_CONTEXT        *pDriverContext = (PSS_DRIVER_CONTEXT*)(*((PSS_DRIVER_CONTEXT**)Context));
  CHAR16                    *VerificationKeySting;
  UINT8                     *VerificationKey;
  BOOLEAN                   ParseResult = FALSE;
  EFI_STATUS                Status;

  DEBUG ((EFI_D_ERROR, "PromptForVerificationKey...\n"));

  if(pDriverContext != NULL && gST->ConOut!= NULL) {
    if(pDriverContext->PssEnabled&&!((pDriverContext)->PssVerified)) {
      //
      // Set Console Mode
      //
      Status = SetConsoleMode(TRUE);

      //
      // Draw Popup for Verification Key prompt
      //
      DrawPopUp ();
      VerificationKeySting = AllocateZeroPool ((PUBLIC_KEY_SETUP_SIZE*2 +1)* sizeof (CHAR16));
	  if (VerificationKeySting == NULL) {
	  	return;
	  }
      VerificationKey      = AllocateZeroPool ((PUBLIC_KEY_SETUP_SIZE +1)* sizeof (UINT8));
	  if (VerificationKey == NULL) {
	  	return;
	  }
      Status = ReadStrings(VerificationKeySting);
      ParseResult = StringToArray(VerificationKeySting, VerificationKey);

      //
      // Verify again
      //
      Status = VerifyKey(VerificationKey);

      if (!((pDriverContext)->PssVerified)) {
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->OutputString(gST->ConOut, L"Failied to verify PSS. System will shutdown.\r\n");
        gBS->Stall (5000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      }

      //
      // Restore to original mode before launching boot option.
      //
      Status = SetConsoleMode (FALSE);
    }

    gST->ConOut->ClearScreen (gST->ConOut);
    gST->ConOut->OutputString(gST->ConOut, L"Succeed to verify PSS.\r\n");
  }

  return;
}


EFI_STATUS
EFIAPI
PssDxeEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Entry point for PSS DXE driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
{
  EFI_STATUS                    Status;
  UINT8                         PssKeyOffset = 0;
  EFI_PEI_HOB_POINTERS          GuidHob;
  UINT8                         PssTID[6]= {0,0,0,0,0,0};
  UINT8                         VerificationKey[PUBLIC_KEY_SETUP_SIZE];

  DEBUG((EFI_D_ERROR, "PssDxeEntryPoint ++\n"));

  //
  // Get the HOB list.  If it is not present, then return.
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
      mPlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  if ( mPlatformInfo != NULL) {
    if ((mPlatformInfo->BoardId != BOARD_ID_BL_FFRD) || (mPlatformInfo->BoardRev < PR1)) {
      DEBUG((EFI_D_ERROR, "Platform is not FFRD and PR1+. PSS driver aborted.\r\n"));
      return EFI_SUCCESS;
    }
  }

  //
  // Initialize PSS
  //
  Status = PssDriverInit();
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to initialize PSS device.\r\n"));
    return Status;
  }

  if(!DriverContext->PssEnabled) {
    DEBUG((EFI_D_ERROR, "PSS not enabled.\r\n"));
    return EFI_SUCCESS;
  }

  if (DriverContext->Is2KPart == TRUE) {
    PssKeyOffset = BASE_ADDRESS_USER_2K;
  } else {
    PssKeyOffset = BASE_ADDRESS_USER_8K;
  }

  //
  // Check the TID
  //
  ZeroMem(PssTID, sizeof(PssTID));

  Status = GetTID(&PssTID[0]);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get TID.\n"));
    return Status;
  } else {
    DEBUG((EFI_D_ERROR,"TID is:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\r\n",
           PssTID[0], PssTID[1], PssTID[2], PssTID[3], PssTID[4], PssTID[5]));
  }

  //
  // Get the VerificationKey
  //
  Status = ReadPssData(VerificationKey, PssKeyOffset, PUBLIC_KEY_SETUP_SIZE);
  if(EFI_ERROR(Status)) {
    DriverContext->PssVerified = FALSE;
    DEBUG((EFI_D_ERROR, "Failed to read PSS private key.\r\n"));
    goto _GetPssInfo_failure;
  }

  //
  // Verify Key
  //
  Status = VerifyKey(VerificationKey);

_GetPssInfo_failure:
  //
  // Creat a event
  //
  Status = EfiCreateEventReadyToBootEx(
             TPL_CALLBACK,
             OnReadyToBoot,
             (VOID*)&DriverContext,
             &mReadyToBootEvent
             );

  DEBUG((EFI_D_ERROR, "PssDxeEntryPoint --\n"));
  return Status;
}
