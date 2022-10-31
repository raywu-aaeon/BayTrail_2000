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
//**********************************************************************
//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: Scrtlib.c -  Standard C Run-time Library(CRT) Interfaces
//
// Description:     _strdup; _stricmp; Atoi_ex 
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>
#include "includes.h"

void *malloc(unsigned int Size);
void free(void *ptr);
//void memcpy(void* pDestination, void* pSource, unsigned long Length);

//extern EFI_RUNTIME_SERVICES		*pRS;

//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:  _strdup
//
// Description:	The strdup() function shall return a pointer to a new string, 
//              which is a duplicate of the string pointed to by s1
//
// Input:  const char *s  - string source 
//
// Output:	pointer to dupplicated string  
//
//**********************************************************************
//<AMI_PHDR_END>
CHAR8 * _strdup(const CHAR8 *s)
{
	CHAR8 *src=(CHAR8*)s, *aa=NULL;
	int size=0, maxsize = 0x1000;

    while(*src++ && size++ < maxsize);
    if(size < maxsize) {
        aa = malloc(size);
        memcpy((void*)aa, (void*)s, size);
    }
    return aa;
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   tolower
//
// Description: Converts Unicode standard European characters to lower case
//
// Input: Data - character to convert
//
// Output: the  character in lower case Unicode format
//
//<AMI_PHDR_END>
//**********************************************************************
//#ifdef PEI_BUILD
CHAR8 _tolower( CHAR8  Data  )
{
    if ( ('A' <= Data && Data <= 'Z')   ||
         (Data >= 0xC0 && Data <= 0xD6) ||
         (Data >= 0xD8 && Data <= 0xDE) )
        return (CHAR8)(Data + 0x20);
    else
        return Data;
}
//#endif
//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:  _stricmp
//
// Description:	Perform a lowercase comparison of strings.
//
// Input:
//  IN CHAR8 *string1
// Pointer to a CHAR8 null-terminated string.
//
//  IN CHAR8 *string2
// Pointer to a CHAR8 null-terminated string.
//
// Output:
//  INT32 non-zero value if both string1 and string2 are not identical.
//  Otherwise, returns 0 if both string1 and string 2 are identical.
//
//          < 0 string1 less than string2 
//          0 string1 identical to string2 
//          > 0 string1 greater than string2 
//
//**********************************************************************
//<AMI_PHDR_END>
int _stricmp( const CHAR8 *string1, const CHAR8 *string2 ) 
{
    CHAR8 *s1, *s2, *pos1, *pos2;
//    unsigned int size1, size2;
    int res;
//    size1 =  (unsigned int)Strlen(string1);
//    size2 =  (unsigned int)Strlen(string2);
//    s1 = malloc(size1);
//    s2 = malloc(size2);
    pos1 = s1 = string1;//s1;
    pos2 = s2 = string2;//s2;
// copying&converting to lower
	while(*string1++ && *string2++) {
        *pos1++=_tolower(*string1);
        *pos2++=_tolower(*string2);
    }

// comparison
	while(*s1 && *s1==*s2) {*s1++;*s2++;}
    res = *s1 - *s2;
//    free(s1);
//    free(s2);
	return res;
}

/* formatted Atoi func. replaces sscanf*/
//*************************************************************************
//<AMI_PHDR_START>
//
// Name: AtoiEX
//
// Description:
//  INT32 AtoiEX(IN CHAR8 *string) converts an ASCII string
//  that represents an integer into an actual INT integer value and returns the result.
//  The input string size is limited by strlen parameter
//
// Input:
//  IN CHAR8 *string
// Pointer to a CHAR8 string that represents an integer number.
//  IN UINT8 string length
// Limits the string length to parse.
//
// Output:
//  INT32 value that the string represented.
//
// Modified:
//
// Referrals:
// 
// Notes:	
// 
//<AMI_PHDR_END>
//*************************************************************************
int AtoiEX(char *s, UINT8 len, int* value)
{
    char ch;
    char str[9];
    if(len>8) len=8; // max int size
    memcpy(str, s, len);
    ch = *(str+len);
    *(str+len) = 0;
    *value = Atoi(str);
    *(str+len) = ch;

    return *value;
}
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
