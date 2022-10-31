
;******************************************************************************
;*
;*        Copyright (c) 2000-2006  Intel Corporation. All rights reserved
;*
;*    This software and associated documenation (if any) is furnished
;*    under a license and may only be used or copied in accordance
;*    with the terms of the license. Except as permitted by such
;*    license, no part of this software or documentation may be
;*    reproduced, stored in a retrieval system, or transmitted in any
;*    form or by any means without the express written consent of
;*    Intel Corporation.
;*
;******************************************************************************

VacantFlag             Equ   00h
NotVacantFlag          Equ   0ffh
StartupApSignal        Equ   6E750000h
MonitorFilterSize      Equ   10h
ApCounterInit          Equ   0
ApInHltLoop            Equ   1
ApInMwaitLoop          Equ   2
ApInRunLoop            Equ   3

LockLocation           equ   1000h - 0400h
StackStart             equ   LockLocation + 4h
StackSize              equ   LockLocation + 8h
RendezvousProc         equ   LockLocation + 0Ch
GdtrProfile            equ   LockLocation + 10h
IdtrProfile            equ   LockLocation + 16h
BufferStart            equ   LockLocation + 1Ch
Cr3Location            equ   LockLocation + 20h
InitFlag               equ   LockLocation + 24h
WakeUpApManner         equ   LockLocation + 28h
BistBuffer             equ   LockLocation + 2Ch
PAUSE32   MACRO
            DB      0F3h
            DB      090h
          ENDM

;-------------------------------------------------------------------------------
