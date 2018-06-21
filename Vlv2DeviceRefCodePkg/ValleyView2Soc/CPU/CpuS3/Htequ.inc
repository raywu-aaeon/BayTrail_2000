
;-------------------------------------------------------------------------------
;
; Copyright (c)  1999 - 2007 Intel Corporation. All rights reserved
; This software and associated documentation (if any) is furnished
; under a license and may only be used or copied in accordance
; with the terms of the license. Except as permitted by such
; license, no part of this software or documentation may be
; reproduced, stored in a retrieval system, or transmitted in any
; form or by any means without the express written consent of
; Intel Corporation.
;
;
; Module Name:
;
;   HtEqu.inc
; 
; Abstract:
; 
;   This is the equates file for HT (Hyper-threading) support
;
;-------------------------------------------------------------------------------

VacantFlag             Equ   00h
NotVacantFlag          Equ   0ffh

LockLocation                  equ        1000h - 0200h
StackStartAddressLocation     equ        LockLocation + 04h
StackSizeLocation             equ        LockLocation + 08h
CProcedureLocation            equ        LockLocation + 0Ch
GdtrLocation                  equ        LockLocation + 10h
IdtrLocation                  equ        LockLocation + 16h
BufferStartLocation           equ        LockLocation + 1Ch
PmodeOffsetLocation           equ        LockLocation + 20h
AcpiCpuDataAddressLocation    equ        LockLocation + 24h
MtrrValuesAddressLocation     equ        LockLocation + 28h
FinishedCountAddressLocation  equ        LockLocation + 2Ch
SerializeLockAddressLocation  equ        LockLocation + 30h
MicrocodeAddressLocation      equ        LockLocation + 34h
BootScriptAddressLocation     equ        LockLocation + 38h
StartStateLocation            equ        LockLocation + 3Ch

PAUSE32   MACRO
            DB      0F3h
            DB      090h
            ENDM

;-------------------------------------------------------------------------------
