//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: IA32math.c -  64-bit Math Functions to patch up 32bit MS compiler
//                      
//
// Description:     The 32-bit versions of C compiler generate calls to library routines
//                  to handle 64-bit math. These functions use non-standard calling conventions.
//                  _allmul(IA32 use)
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>
#include "AmiLib.h"

#if defined(_WIN64)
#else
//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:  _allmul
//
// Description:	64x32 bit multiplication
//              long multiply routine
//              Does a long multiply (same for signed/unsigned)
//
// Input:
//  Value64  multiplier (QWORD)
//  Value32  multiplicand (QWORD)
//
// Output:	  UINT64 - 64bit product of multiplier and multiplicand
//
//**********************************************************************
//<AMI_PHDR_END>
UINT64 _allmul(IN UINT64 Value64,IN UINTN Value32)
{
    return Mul64(Value64, Value32);
/*
//; EXIT PARAMETERS.
//;    EDX:EAX - Result.
    UINT32 MA=(UINT32)(Value64>>32);
    UINT32 MB=(UINT32)Value64;
    UINT32 MC(UINT32)Value32>>32;
    UINT32 MD(UINT32)Value32;
    _asm {
// MA EQU DWORD PTR Value64 [4]
// MB EQU DWORD PTR Value64
// MC EQU DWORD PTR Value32 [4]
// MD EQU DWORD PTR Value32

 mov eax, MA
 mov ecx, MC
 or  ecx, eax    ; both zero?
 mov ecx, MD
 .if zero?      ; yes, use shortcut.
   mov eax, MB
   mul ecx      ; EDX:EAX = DB[0:63].
 .else
   mov eax, MA
   mul ecx      ; EDX:EAX = DA[0:63].
   mov esi, eax ; ESI = DA[0:31].

   mov eax, MB 
   mul MC       ; EDX:EAX = CB[0:63]
   add esi, eax ; ESI = DA[0:31] + CB[0:31]


   mov eax, MB
   mul ecx      ; EDX:EAX = BD[0:63]
   add edx, esi ; EDX = DA[0:31] + CB[0:31] + DB[31:63]
                ; EAX = DB[0:31]
 .endif

    }
*/
}
//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:  _aullshr
//
// Description:	Does a Long Shift Rightt (signed and unsigned are identical)
//              Shifts a long right any number of bits.
//
// Input:
//          EDX:EAX - long value to be shifted
//          CL    - number of bits to shift by
//
// Output:	 
//          EDX:EAX - shifted value
//
//**********************************************************************
//<AMI_PHDR_END>
_aullshr( IN UINT64 Value, IN UINT8 Shift )
{
	_asm {
			xor	ebx, ebx
			test cl, 32
			cmovnz   eax, edx
			cmovnz   edx, ebx
			shrd    eax, edx, cl
			shr     edx, cl
	}
}

//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:  _allshl
//
// Description:	Does a Long Shift Left (signed and unsigned are identical)
//              Shifts a long left any number of bits.
//
// Input:
//  Value64  long value to be shifted
//  Value8   number of bits to shift by
//
// Output:	  UINT64 - 64bit product of multiplier and multiplicand
//
//**********************************************************************
//<AMI_PHDR_END>
/*
_allshl( IN UINT64 Value, IN UINT8 Shift )
{
	_asm {
    			xor	ebx, ebx
                test cl, 32
                cmovz   eax, edx
                cmovz   edx, ebx
                shld    edx, eax, cl
                shl     eax, cl
	}
}
*/

//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:  _aulldiv
//
// Description:	Does a unsigned long divide of the arguments.
//
// Input:
//  Value64  dividend (QWORD)
//  Value64  divisor (QWORD)
//
// Output:	  UINT64 - 64bit product of multiplier and multiplicand
//
//**********************************************************************
//<AMI_PHDR_END>
UINT64 _aulldiv(IN UINT64 Dividend, IN UINTN Divisor)	//Can only be 31 bits for IA-32
{
    UINTN	*Remainder=0;
    return Div64(Dividend, Divisor, Remainder);
}
#endif //#if defined(_WIN64)
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
