/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SataControllerName.c

Abstract:

  This portion is to register the Sata Controller Driver name:
    "SATA Controller Init Driver"

--*/
#include "SataController.h"
#ifndef ECP_FLAG
#include <Library/UefiLib.h>
#endif
//
// Forward reference declaration
//
EFI_STATUS
SataControllerGetDriverName (
#ifdef ECP_FLAG
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
#else
  IN  EFI_COMPONENT_NAME2_PROTOCOL   *This,
#endif
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  );

EFI_STATUS
SataControllerGetControllerName (
#ifdef ECP_FLAG
  IN  EFI_COMPONENT_NAME_PROTOCOL   *This,
#else
  IN  EFI_COMPONENT_NAME2_PROTOCOL   *This,
#endif
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle        OPTIONAL,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  );

//
// EFI Component Name Protocol
// This portion declares a gloabl variable of EFI_COMPONENT_NAME_PROTOCOL type.
//
#ifdef ECP_FLAG
EFI_COMPONENT_NAME_PROTOCOL      mSataControllerName = {
#else
EFI_COMPONENT_NAME2_PROTOCOL     mSataControllerName = {
#endif
  SataControllerGetDriverName,
  SataControllerGetControllerName,
#ifdef ECP_FLAG
  "eng"
#else
  "en"
#endif
};

//
//  Define the Driver's unicode name string
//
static EFI_UNICODE_STRING_TABLE mSataControllerDriverNameTable[] = {
  {"eng;en", L"PCH Serial ATA Controller Initialization Driver"},
  {NULL, NULL}
};

static EFI_UNICODE_STRING_TABLE mSataControllerControllerNameTable[] = {
  {"eng;en", L"PCH Serial ATA Controller"},
  {NULL,  NULL}
};

//
// /////////////////////////////////////////////////////////////////////////////////
//
EFI_STATUS
SataControllerGetDriverName (
#ifdef ECP_FLAG
  IN  EFI_COMPONENT_NAME_PROTOCOL   *This,
#else
  IN  EFI_COMPONENT_NAME2_PROTOCOL   *This,
#endif
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  )
/*++

Routine Description:
  Retrieves a Unicode string that is the user readable name of the EFI Driver.

Arguments:
  This                    A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  Language                A pointer to a three character ISO 639-2 language identifier.
                          This is the language of the driver name that that the caller
                          is requesting, and it must match one of the languages specified
                          in SupportedLanguages.  The number of languages supported by a
                          driver is up to the driver writer.
  DriverName              A pointer to the Unicode string to return.  This Unicode string
                          is the name of the driver specified by This in the language
                          specified by Language.

Returns:
  EFI_SUCCESS             The Unicode string for the Driver specified by This
                          and the language specified by Language was returned
                          in DriverName.
  EFI_INVALID_PARAMETER   Language is NULL.
  EFI_INVALID_PARAMETER   DriverName is NULL.
  EFI_UNSUPPORTED         The driver specified by This does not support the
                          language specified by Language.

--*/
{
#ifdef ECP_FLAG
  return LookupUnicodeString (
           Language,
           mSataControllerName.SupportedLanguages,
           mSataControllerDriverNameTable,
           DriverName
           );
#else
  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mSataControllerDriverNameTable,
           DriverName,
           (BOOLEAN)(This == &mSataControllerName)
           );
#endif
}

EFI_STATUS
SataControllerGetControllerName (
#ifdef ECP_FLAG
  IN  EFI_COMPONENT_NAME_PROTOCOL   *This,
#else
  IN  EFI_COMPONENT_NAME2_PROTOCOL   *This,
#endif
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle        OPTIONAL,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  )
/*++

Routine Description:
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by an EFI Driver.

Arguments:
  This                    A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  ControllerHandle        The handle of a controller that the driver specified by
                          This is managing.  This handle specifies the controller
                          whose name is to be returned.
  ChildHandle             The handle of the child controller to retrieve the name
                          of.  This is an optional parameter that may be NULL.  It
                          will be NULL for device drivers.  It will also be NULL
                          for a bus drivers that wish to retrieve the name of the
                          bus controller.  It will not be NULL for a bus driver
                          that wishes to retrieve the name of a child controller.
  Language                A pointer to a three character ISO 639-2 language
                          identifier.  This is the language of the controller name
                          that that the caller is requesting, and it must match one
                          of the languages specified in SupportedLanguages.  The
                          number of languages supported by a driver is up to the
                          driver writer.
  ControllerName          A pointer to the Unicode string to return.  This Unicode
                          string is the name of the controller specified by
                          ControllerHandle and ChildHandle in the language
                          specified by Language from the point of view of the
                          driver specified by This.

Returns:
  EFI_SUCCESS             The Unicode string for the user readable name in the
                          language specified by Language for the driver
                          specified by This was returned in DriverName.
  EFI_INVALID_PARAMETER   ControllerHandle is not a valid EFI_HANDLE.
  EFI_INVALID_PARAMETER   ChildHandle is not NULL and it is not a valid
                          EFI_HANDLE.
  EFI_INVALID_PARAMETER   Language is NULL.
  EFI_INVALID_PARAMETER   ControllerName is NULL.
  EFI_UNSUPPORTED         The driver specified by This is not currently
                          managing the controller specified by
                          ControllerHandle and ChildHandle.
  EFI_UNSUPPORTED         The driver specified by This does not support the
                          language specified by Language.

--*/
{
  return LookupUnicodeString (
           Language,
           mSataControllerName.SupportedLanguages,
           mSataControllerControllerNameTable,
           ControllerName
           );
}
