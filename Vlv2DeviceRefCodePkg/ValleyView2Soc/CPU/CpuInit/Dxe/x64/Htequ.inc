
;******************************************************************************
;*
;*        Copyright (c) 2005-2006  Intel Corporation. All rights reserved
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

VacantFlag                    Equ   00h
NotVacantFlag                 Equ   0ffh
BreakToRunApSignal            Equ   6E750000h

MonitorFilterSize             Equ   40h
WakeUpApCounterInit           Equ   0
WakeUpApPerHltLoop            Equ   1
WakeUpApPerMwaitLoop          Equ   2
WakeUpApPerRunLoop            Equ   3
WakeUpApPerMwaitLoop32        Equ   4
WakeUpApPerRunLoop32          Equ   5

LockLocation                  equ   1000h - 0400h
StackStartAddressLocation     equ   LockLocation + 08h
StackSizeLocation             equ   LockLocation + 10h
CProcedureLocation            equ   LockLocation + 18h
GdtrLocation                  equ   LockLocation + 20h
IdtrLocation                  equ   LockLocation + 2Ah
BufferStartLocation           equ   LockLocation + 34h
Cr3OffsetLocation             equ   LockLocation + 38h
InitFlagLocation              equ   LockLocation + 3Ch
WakeUpApManner                equ   LockLocation + 40h
BistBuffer                    equ   LockLocation + 44h

PAUSE32   MACRO
            DB      0F3h
            DB      090h
            ENDM

;-------------------------------------------------------------------------------
