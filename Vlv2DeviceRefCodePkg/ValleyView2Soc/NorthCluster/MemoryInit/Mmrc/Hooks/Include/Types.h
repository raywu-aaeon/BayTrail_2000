/*++

Copyright (c) 1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

	Types.h

Abstract:

	This file include all the external Data types.

--*/

#ifndef _TYPES_H_
#define _TYPES_H_

#ifndef EFI_MEMORY_INIT
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                int8_t;
typedef uint32_t  uintn_t;
typedef int32_t   intn_t;
//
typedef uint8_t BOOLEAN;
typedef intn_t INTN;
typedef uintn_t UINTN;
typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;

#if defined EFI_MEMORY_INIT && EFI_MEMORY_INIT
typedef int64_t INT64;
typedef uint64_t UINT64;
#endif

typedef int8_t CHAR8;
typedef uint16_t CHAR16;
#endif

#if defined EFI_MEMORY_INIT && EFI_MEMORY_INIT
#ifdef ECP_FLAG
#include "Tiano.h"
#else
#include <PiPei.h>
#endif
#endif

//
// Modifiers to abstract standard types to aid in debug of problems
//
#define CONST     const
#define STATIC    static
#define VOID      void
#define VOLATILE  volatile

//
// Constants. They may exist in other build structures, so #ifndef them.
//
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif
#ifndef TRUE
#define TRUE  ((BOOLEAN) 1 == 1)
#endif

#ifndef FALSE
#define FALSE ((BOOLEAN) 0 == 1)
#endif

#ifndef NULL
#define NULL  ((VOID *) 0)
#endif

typedef UINT32 STATUS;
#define SUCCESS 0
#define FAILURE 0xFFFFFFFF

#ifndef MRC_DEADLOOP
#define MRC_DEADLOOP()    while (TRUE)
#endif

#endif
