;**********************************************************************
;**********************************************************************
;**                                                                  **
;**        (C)Copyright 1985-2007, American Megatrends, Inc.         **
;**                                                                  **
;**                       All Rights Reserved.                       **
;**                                                                  **
;**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
;**                                                                  **
;**                       Phone: (770)-246-8600                      **
;**                                                                  **
;**********************************************************************
;**********************************************************************

;**********************************************************************
; $Header: /Aptio/Modules/ICBD-eModule/ICBD/DmiNvData/DmiNvData.asm 1     11/11/30 9:10p Chienliho $
;
; $Revision: 1 $
;
; $Date: 11/11/30 9:10p $
;**********************************************************************
; Revision History
; ----------------
; $Log: /Aptio/Modules/ICBD-eModule/ICBD/DmiNvData/DmiNvData.asm $
; 
; 1     11/11/30 9:10p Chienliho
; 
; 1     11/11/30 8:48p Chienliho
; 
; 1     11/07/08 4:47p Chienlih
; 
; 3     3/29/07 6:13p Davidd
; Changed the year in the AMI banner.
; 
; 2     12/15/06 5:48p Davidd
; Code cleanup and reformatted to coding standard.
; 
; 1     4/29/05 2:06p Davidd
; Initial checkin.
;
;**********************************************************************

	INCLUDE token.equ



.586p 
.model	flat,C 
.data
	;Structure ID
	db '$','B','N','F'

	;BIOS Stage 2 Info
	;SPI Offset
	dd MKF_FV_MAIN_OFFSET
	;Size
	dd MKF_FV_MAIN_SIZE
	INCLUDE HashSecondStageKey.inc

	;Recovery Module Info
	;SPI Offset
	dd MKF_FV_BB_OFFSET
	;Size
	dd MKF_FV_BB_SIZE
	INCLUDE HashFvBbKey.inc

	;uCode Info
	;SPI Offset
	dd MKF_MICROCODE_OFFSET
	;Size
	dd MKF_MICROCODE_SIZE

	;Recovery uCode Info
	;SPI Offset
	dd 0h
	;Size
	dd 0h
end

;**********************************************************************
;**********************************************************************
;**                                                                  **
;**        (C)Copyright 1985-2007, American Megatrends, Inc.         **
;**                                                                  **
;**                       All Rights Reserved.                       **
;**                                                                  **
;**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
;**                                                                  **
;**                       Phone: (770)-246-8600                      **
;**                                                                  **
;**********************************************************************
;**********************************************************************
