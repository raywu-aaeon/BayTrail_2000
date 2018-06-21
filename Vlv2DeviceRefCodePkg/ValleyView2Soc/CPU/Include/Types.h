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

    Types.h

Abstract:

    This file include all the external data types.

--*/

#ifndef _TYPES_H_
#define _TYPES_H_



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
