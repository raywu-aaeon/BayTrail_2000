/*++

Copyright (c) 2004-2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTAm.c

Abstract:

  TDT authetication module for using TDT DXE driver.
  This driver uses the TDT protocol, HECI Protocol and TDT Platform Policy to implement Theft Deterrence Technology AM module.

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/



#include "TdtAmHelper.h"
#include "TdtAm.h"
#include <Protocol/Tdt.h>
#include <Protocol/Heci.h>


EFI_STATUS
Base32Encode (
  UINT8 *encodedStr,
  UINTN *encodedLen,
  UINT8 *rawData,
  UINTN rawDataLen
)

/*++

Routine Description:

  Convert Hex values into Base32 values

Arguments:
  Array of encoded BASE values
  Length of encoded values
  array of Hex strings that needs to be encoded into BASE32
  Length of Hex values

Returns:
  Array of converted Hex values

--*/


{
  UINTN i, j;
  UINT8 value;
  int shift;
  int shifttable[] = {3, -2, 1, -4, -1, 2, -3, 0};

  if (encodedLen && *encodedLen < (rawDataLen * 8) / 5) {
    DEBUG((EFI_D_ERROR, "TDTAm::Base32Encode:  Buffer to store encoded string too small"));
    return EFI_BUFFER_TOO_SMALL;
  }
  for (i = 0, j = 0; i < rawDataLen; j++) {
    shift = shifttable[j % 8];
    if (shift > 0) {
      value = (UINT8)(rawData[i] >> shift);
    } else {
      value = (UINT8) (rawData[i] << (-shift));
      if (i++ < rawDataLen - 1) {
        value |= (rawData[i] >> (8 + shift));
      }
    }
    value &= 0x1f;
    if (25 >= value) {
      value += 'A';
    } else {
      value += '2' - 26;
    }
    if (encodedStr && encodedLen) {
      encodedStr[j] = value;
    }
  }
  if (encodedStr && encodedLen) {
    *encodedLen = j;
    encodedStr[j] = L'\0';
  }
  return  EFI_SUCCESS;
}


// RFC 3548 - http://rfc.net/rfc3548.html#p6
int
Base32Decode (
  UINT8 *encodedStr,
  UINT8 *decodedData
)

/*++

Routine Description:

  Convert Base32 values into Hex values

Arguments:
  array of Decimal numbers
  Converted Hex values
  Length of the Hex Strings that expected

Returns:
  Array of converted Hex values

--*/


{

  UINTN i, j;
  int retVal = 1;
  UINT8 value;
  UINTN decodedLen = 0;
  int shifttable[] = {3, -2, 1, -4, -1, 2, -3, 0};
  int shift;
  UINTN encodedLen = AsciiStrLen((char *)encodedStr);
  decodedLen = (encodedLen * 5) / 8;

  decodedData[0] = 0;
  for (i = 0, j = 0; i < encodedLen; i++) {
    if ('A' <= encodedStr[i] && 'Z' >= encodedStr[i]) {
      value = encodedStr[i] - 'A';
    } else if ('a' <= encodedStr[i] && 'z' >= encodedStr[i]) {
      value = encodedStr[i] - 'a';
    } else if ('2' <= encodedStr[i] && '7' >= encodedStr[i]) {
      value = encodedStr[i] - '2' + 26;
    } else {
      //printf("Invalid Base32 character %c\n", encodedStr[i]);
      retVal = 0;
      break;
    }
    shift = shifttable[i % 8];
    if (shift > 0) {
      decodedData[j] |= (value << shift);
    } else {
      decodedData[j] |= (value >> (-shift));
      // Pack the bits that are going to fall off due to right shift in next byte
      // (left justfied) unless decoding the last character. In case of last
      // character, the extra bits are just padding to make the length of input
      // bits a multiple of 5 while encoding - so ignore them
      if (i != encodedLen - 1)
        decodedData[++j] = (UINT8) (value << (8 + shift));
    }
  }
  return retVal;
}


VOID
DecimalToHexString (
  UINT8 *decStr,
  UINT8 *u8Hex,
  UINTN hexIndex
)

/*++

Routine Description:

  Decimal large (BASE10) into hex value

Arguments:
  array of Decimal numbers
  Converted Hex values
  Length of the Hex Strings that expected

Returns:
  Array of converted Hex values

--*/


{

  UINTN i, j;
  UINTN Remainder;
  CHAR8 Quotient;
  UINTN num;
  UINTN len;
  UINTN leadingZero = 1;

  len = AsciiStrLen((CHAR8 *)decStr);
  Remainder = decStr[0] - '0';

  if (len > 1)
    Remainder = Remainder * 10 + decStr[1] - '0';

  if (len < 3) {
    u8Hex[hexIndex] = (UINT8) Remainder;
    return;
  }

  i = 2;
  j = 0;
  while(i < len) {
    num = Remainder * 10 + decStr[i] - '0';
    Quotient = (CHAR8) (num / 256);
    if (!leadingZero || Quotient) {
      decStr[j++] = '0' + Quotient;
      leadingZero = 0;
    }
    Remainder = num % 256;
    i++;

  }

  decStr[j] = L'\0';
  u8Hex[hexIndex--] = (UINT8)Remainder;

  if (decStr[0] != L'\0') {
    DecimalToHexString(decStr, u8Hex, hexIndex);
  }

  return;

}

VOID
Uint8ToUnicode (
  IN  UINT8     *AsciiString,
  OUT CHAR16    *UnicodeString
)
/*++

Routine Description:

  Convert the CHAR8 ASCII into CHAR16 Unicode strings

Arguments:
  Ascii String
  Buffer for converted Unicode string

Returns:
  Unicode String

--*/
{
  UINT8 Index = 0;

  while (AsciiString[Index] != 0) {
    UnicodeString[Index] = (CHAR16) AsciiString[Index];
    Index++;
  }
}



/**
 * DecodePEMChar
 *
 * Decode a PEM-encoded character.
 *
 * Parameters
 *  IN      pemChar                 PEM-encoded character.
 *
 * Return Value:
 *  Value orresponding to the character, or PEM_INVALID_CHARACTER if the
 *  input character could not be decoded.
 */


UINT8
DecodePEMChar (
  UINT8 pemChar
  )
{
  if ((pemChar >= 'A') && (pemChar <= 'Z')) return (pemChar - 'A');
  if ((pemChar >= 'a') && (pemChar <= 'z')) return ((pemChar - 'a') + 26);
  if ((pemChar >= '0') && (pemChar <= '9')) return ((pemChar - '0') + 52);
  if (pemChar == '-') return 62;
  if (pemChar == '!') return 63;
  if (pemChar == '=') return PEM_PAD_CHAR;

  DEBUG((EFI_D_ERROR, "DecodePEMChar: Invalid  character %d\n", pemChar));
  return PEM_INVALID_CHAR;
}


UINT8
DecodeBase16Char (
  UINT8 base16char
  )
{
  if (base16char >= '0' && base16char <= '9') {
    return base16char - '0';
  } else if (base16char >= 'A' && base16char <= 'F') {
    return base16char - 'A' + 0xA;
  } else if (base16char >= 'a' && base16char <= 'f') {
    return base16char - 'a' + 0xA;
  }
  return 0;
}

BOOLEAN
Base16Decode (
  UINT8   *pString,
  UINT32  stringLen,
  UINT8   *pDecodedData,
  UINT32  *pBufLength
  )
{
  int i;
  if (stringLen % 2 != 0) {
    DEBUG((EFI_D_ERROR, "Cannot decode hexadecimal string -- invalid length\n"));
  } else if (*pBufLength < stringLen / 2) {
    DEBUG((EFI_D_ERROR, "Base16 decode - output buffer too small\n"));
  } else {
    for (i = 0; i < (int)stringLen / 2; i++) {
      pDecodedData[i] = DecodeBase16Char(pString[2 * i]) << 4;
      pDecodedData[i] |= DecodeBase16Char(pString[2 * i + 1]);
    }
    *pBufLength = i;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN
Base16Encode (
  UINT8  *pData,
  UINT32 dataLength,
  UINT8  *pEncodedData,
  UINT32 *pBufLength
  )
{
  UINT32 i;
  if (*pBufLength < 2 * dataLength) {
    DEBUG((EFI_D_ERROR, "Cannot encode data -- output buffer too small\n"));
    return FALSE;
  }
  DEBUG((EFI_D_ERROR, "Base16Encode dataLength : %d\n", dataLength));

  for (i = 0; i < dataLength; i++) {
    AsciiSPrint((CHAR8 *) &pEncodedData[2 * i], 3 * sizeof(UINT8), "%02x", pData[i] & 0xFF);
  }
  DEBUG((EFI_D_ERROR, "Base16Encode AsciiSPrint : %s\n", (char *)pEncodedData));
  DEBUG((EFI_D_ERROR, "Base16Encode Sprint : %s\n", (char *)pEncodedData));

  *pBufLength = 2 * dataLength;

  return TRUE;
}

/**
 * PEMSMSDecode
 *
 * Decode PEM-Encoded data.
 *
 * Parameters
 *  IN      pPEMString              Pointer to PEM-Encoded buffer.
 *  OUT     pDecodedData            Pointer to buffer to receive decoded bytes.
 *  IN-OUT  pBufLength              Pointer to the size of the output buffer.  Upon return this will contain
 *                                  the length of the decoded data.
 *
 * Returns status code indicating outcome of operation.
 */

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize ("O0")
#else
#pragma optimize("", off)
#endif
BOOLEAN
PEMSMSDecode (
  UINT8  *pPEMString,
  UINT32 lineLength,
  UINT8  *pDecodedData,
  UINT32 *pBufLength
  )
{
  UINT8 *pStr = pPEMString;
  UINT32 curOutLen = *pBufLength;
  UINT8 *pOut = pDecodedData;
  UINT32 i;
  UINT32 padBytes = 0;
  UINT8 val1, val2;
  UINT8 decodedValue[4];
  UINT32 ii;
  UINT32 unitCount;
  UINT32 decodedBytes;

  if (pStr != NULL) {
    if ((lineLength % 4) != 0) {
      DEBUG((EFI_D_ERROR, "Cannot decode PEM string -- invalid length -1\n"));
      return FALSE;
    }

    unitCount = lineLength / 4;

    for (i = 0; (i < unitCount) && (padBytes == 0); i++) {
      val1 = DecodePEMChar(*pStr++);
      val2 = DecodePEMChar(*pStr++);

      if ((val1 == PEM_INVALID_CHAR) || (val2 == PEM_INVALID_CHAR)) {
        DEBUG((EFI_D_ERROR, "Cannot decode PEM string -- invalid character -2\n"));
        return FALSE;
      }

      decodedValue[0] = ((val1 << 2) | (val2 >> 4));
      val1 = DecodePEMChar(*pStr++);
      if (val1 == PEM_INVALID_CHAR) {
        DEBUG((EFI_D_ERROR, "Cannot decode PEM string -- invalid character -3\n"));
        return FALSE;
      }

      if (val1 != PEM_PAD_CHAR) {
        decodedValue[1] = ((val2 << 4) | (val1 >> 2));
        val2 = DecodePEMChar(*pStr++);
        if (val2 == PEM_INVALID_CHAR) {
          DEBUG((EFI_D_ERROR, "Cannot decode PEM string -- invalid character -4\n"));
          return FALSE;
        }
        if (val2 == PEM_PAD_CHAR) {
          padBytes = 1;
        } else {
          decodedValue[2] = ((val1 << 6) | val2);
        }
      } else {
        val2 = DecodePEMChar(*pStr++);
        if (val2 != PEM_PAD_CHAR) {
          DEBUG((EFI_D_ERROR, "Cannot decode PEM string -- invalid encoding -4\n"));
          return FALSE;
        }
        padBytes = 2;
      }
      decodedBytes = (3 - padBytes);
      if (curOutLen < decodedBytes) {
        DEBUG((EFI_D_ERROR, "Cannot decode PEM string -- out buffer too small-5\n"));
        return FALSE;
      }
      for (ii = 0; ii < decodedBytes; ii++)
        pOut[ii] = decodedValue[ii];

      curOutLen -= decodedBytes;
      pOut += decodedBytes;
    }
  }

  *pBufLength = *pBufLength - curOutLen;
  return TRUE;
}
#ifdef __GNUC__
#pragma GCC pop_options
#else
#pragma optimize("", on)
#endif

BOOLEAN
PEMSMSEncode (
  UINT8  *pData,
  UINT32 dataLength,
  UINT8  *pPEMString,
  UINT32 *pBufLength
  )
{
  UINT32 encodeUnits = (dataLength + 2) / 3;
  UINT32 paddedInputLength = encodeUnits * 3;
  UINT32 padBytes = paddedInputLength - dataLength;
  UINT32 outLength = encodeUnits * 4;
  UINT8  encodeBuf[3];
  UINT32 i;
  UINT8  *pOctet = pData;
  UINT32 ipos = 0;
  UINT32 opos = 0;
  UINT32 index;

  outLength = encodeUnits * 4;
  if (outLength % 64)
    outLength += ((outLength / 64) + 1);
  else
    outLength += (outLength / 64);

  outLength++;
  DEBUG((EFI_D_ERROR, "Pbuflength: %d OutLength: %d \n", *pBufLength, outLength));
  if (*pBufLength < outLength) {
    DEBUG((EFI_D_ERROR, "Heeeeeeeeeee failing Here \n"));
    return FALSE;
  }
  for (i = 0; i < encodeUnits; i++) {
    if ((i == encodeUnits - 1) && (padBytes > 0)) {
      // Encode last piece, with padding
      encodeBuf[0] = *pOctet++;
      encodeBuf[1] = (padBytes == 1) ? *pOctet++ : 0;
      encodeBuf[2] = 0;
      ipos += (3 - padBytes);

      index = encodeBuf[0] >> 2;
      pPEMString[opos++] = PEMCodes[index];
      index = ((encodeBuf[0] & 0x03) << 4) | (encodeBuf[1] >> 4);
      pPEMString[opos++] = PEMCodes[index];

      if (padBytes == 1) {
        index = ((encodeBuf[1] & 0x0f) << 2) | (encodeBuf[2] >> 6);
        pPEMString[opos++] = PEMCodes[index];
        pPEMString[opos++] = '=';
      } else {
        pPEMString[opos++] = '=';
        pPEMString[opos++] = '=';
      }
    } else {
      // Encode next 3 bytes
      encodeBuf[0] = *pOctet++;
      encodeBuf[1] = *pOctet++;
      encodeBuf[2] = *pOctet++;
      ipos += 3;
      index = encodeBuf[0] >> 2;
      pPEMString[opos++] = PEMCodes[index];
      index = ((encodeBuf[0] & 0x03) << 4) | (encodeBuf[1] >> 4);
      pPEMString[opos++] = PEMCodes[index];
      index = ((encodeBuf[1] & 0x0f) << 2) | (encodeBuf[2] >> 6);
      pPEMString[opos++] = PEMCodes[index];
      index = encodeBuf[2] & 0x3f;
      pPEMString[opos++] = PEMCodes[index];
    }
#if 0
    if (((i % PEM_UNITS_PER_LINE) == (PEM_UNITS_PER_LINE - 1)) || (i == encodeUnits - 1)) {
      // Add line feed after each 64 bytes and at end.  Not sure if it is
      // needed at end, but add it for now, and remove after integration
      // with PS if necessary.
      pPEMString[opos++] = '\n';
    }
#endif
  }

  pPEMString[opos] = 0;
  *pBufLength = opos;

  return TRUE;
}

