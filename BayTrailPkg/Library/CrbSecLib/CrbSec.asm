        TITLE CRBSEC.ASM -- Chipset Reference Board SEC initialization
;*************************************************************************
;*************************************************************************
;**                                                                     **
;**        (C)Copyright 1985-2009, American Megatrends, Inc.            **
;**                                                                     **
;**                       All Rights Reserved.                          **
;**                                                                     **
;**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
;**                                                                     **
;**                       Phone: (770)-246-8600                         **
;**                                                                     **
;*************************************************************************
;*************************************************************************

;*************************************************************************
; $Header: /Alaska/BIN/Modules/CRB Board/CRBSEC.ASM 4     6/16/09 3:01a Abelwu $
;
; $Revision: 4 $
;
; $Date: 6/16/09 3:01a $
;*************************************************************************
; Revision History
; ----------------
; $Log: /Alaska/BIN/Modules/CRB Board/CRBSEC.ASM $
; 
; 4     6/16/09 3:01a Abelwu
; Fixed the name of the procedure SECCRB_EarlyInit in Header.
; 
; 3     2/19/09 10:02p Abelwu
; Updated for AMI Coding Standard v0.99
; 
; 2     6/04/08 6:04a Abelwu
; Updated the header of the source file.
; 
; 1     6/03/08 2:40a Abelwu
; 
;*************************************************************************
;<AMI_FHDR_START>
;
; Name:         SRBSEC.ASM
;
; Description:  Program any workaround or initialization needed before
;               enabling Cache as memory in the SEC stage for CRB.
;
;<AMI_FHDR_END>
;*************************************************************************

;----------------------------------------------------------------------------
        INCLUDE token.equ
;----------------------------------------------------------------------------

.686p
.xmm
.model small, c
.code

; Externs
EXTERN  SECCRB_EarlyInitEnd:NEAR32

; Define the equates here

;<AMI_PHDR_START>
;----------------------------------------------------------------------------
;
; Procedure:    SECCRB_EarlyInit
;
; Description:  This routine initializes CRB for PEI preparation
;
; Input:        ESP BIST Info
;               EBP Reset ID (EDX at reset)
;               Stack not available
;
; Output:       None
;
; Modified:     All, except EBP and ESP
;
; Notes:        NBSEC & SBSEC initialization is done at this point.
;----------------------------------------------------------------------------
;<AMI_PHDR_END>

SECCRB_EarlyInit     PROC PUBLIC
        jmp SECCRB_EarlyInitEnd
SECCRB_EarlyInit  ENDP

END
;*************************************************************************
;*************************************************************************
;**                                                                     **
;**        (C)Copyright 1985-2009, American Megatrends, Inc.            **
;**                                                                     **
;**                       All Rights Reserved.                          **
;**                                                                     **
;**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
;**                                                                     **
;**                       Phone: (770)-246-8600                         **
;**                                                                     **
;*************************************************************************
;*************************************************************************

