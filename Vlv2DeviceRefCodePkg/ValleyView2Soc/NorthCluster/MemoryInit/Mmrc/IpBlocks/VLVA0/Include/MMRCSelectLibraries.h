/*++

Copyright (c) 2005-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  MMRCSelectLibraries.h

Abstract:

  Selectable entries.

--*/
#include "MMRC.h"
#include "MMRCProjectLibraries.h"
#include "MMRCLibraries.h"

#ifndef _MMRCSELECTLIBRARIES_H_
#define _MMRCSELECTLIBRARIES_H_

#define _IO_SUPPORTED_
#define _PCI_SUPPORTED_
#define _IOSF_SUPPORTED_

#define VS_ENV

#ifdef PRINTF_SUP
//extern int16_t printf(const char *fmt, ...);
#define DEBUG_ALGOENRANCE			1
#define DEBUG_DUMPPFCT				2
#endif

#ifdef _IO_SUPPORTED_
extern void     	    out16 (uint16_t port, uint16_t val);
extern void     	    out32 (uint16_t port, uint32_t val);
extern uint8_t  	in8 (uint16_t port);
extern uint16_t 	in16 (uint16_t port);
extern uint32_t 	in32 (uint16_t port);
#endif

#ifdef _PCI_SUPPORTED_
extern uint32_t 	pciread32 (uint32_t bus, uint32_t device, uint32_t function, uint32_t reg);
extern uint16_t 	pciread16 (uint32_t bus, uint32_t device, uint32_t function, uint32_t reg);
extern uint8_t  	pciread8 (uint32_t bus, uint32_t device, uint32_t function, uint32_t reg);
extern void     	    pciwrite32 (uint32_t bus, uint32_t device, uint32_t func, uint32_t reg, uint32_t Value);
extern void     	    pciwrite16 (uint32_t bus, uint32_t device, uint32_t func, uint32_t reg, uint16_t Value);
extern void     	    pciwrite8 (uint32_t bus, uint32_t device, uint32_t func, uint32_t reg, uint8_t Value);
#endif

#ifdef _IOSF_SUPPORTED_
extern uint32_t 	iosfreadfield (uint8_t, IOSF_REG );
extern void     	    iosfwritefield (uint8_t, IOSF_REG, uint32_t);
#endif

#endif //_MMRCSELECTLIBRARIES_H_