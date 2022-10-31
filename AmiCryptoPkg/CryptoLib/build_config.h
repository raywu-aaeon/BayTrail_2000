#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H
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
// Name: build_config.h -  Aptio build defines. Fine tuning of wpa-supplicant crypto lib
//                      
//
// Description:     crypto lib configuration defines, e.g., #define CONFIG_CRYPTO_INTERNAL
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

//#pragma warning( disable : 4002 4267 4090 /*4334*/)

/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */
// build_config.h
#define PKCS12_FUNCS
#define INTERNAL_SHA1
#define INTERNAL_SHA256
#define CONFIG_INTERNAL_X509        // x509
#ifndef PEI_BUILD
//#define INTERNAL_AES
#define INTERNAL_MD5
#else // PEI_BUILD
#if CONFIG_PEI_PKCS7 == 0
#define CONFIG_NO_INTERNAL_PEI_PKCS7        //  x509 & PKCS7 
#endif
#endif
#define CONFIG_CRYPTO_INTERNAL      // sha1
#define CONFIG_INTERNAL_LIBTOMMATH  // bignum
#ifndef DEBUG_TRACE
#define CONFIG_NO_STDOUT_DEBUG
#endif

#define __BYTE_ORDER __BIG_ENDIAN

// bignum math
#define MP_PREC                 64     /* default digits of precision */

void *malloc(unsigned int Size);
void free(void *ptr);
void *realloc(void *OldPtr, unsigned int NewSize);
//void memset(void* pBuffer, UINT8 Value, UINTN Size);
//void memcpy(void* pDestination, void* pSource, unsigned long Length);

int _stricmp( const CHAR8 *string1, const CHAR8 *string2 ); //for build error, conflictiong types, Scrtlib.c : 106
char *_strdup(const CHAR8 *s);

//#define memcmp(s1, s2, n) MemCmp((s1), (s2), (n))
#define memcmp MemCmp
//#define memcpy(d, s, n) MemCpy((d), (s), (n))
#define memcpy MemCpy
#define memmove MemCpy
//#define memset MemSet
#define memset(d, n, s) MemSet((d), (s), (n))

// defines for x509v3.c
int sscanf (const char *buffer, const char *format, ...);
int AtoiEX(char *s, UINT8 s_len, int* value);
EFI_STATUS GetTime(EFI_TIME *Time, EFI_TIME_CAPABILITIES *Capabilities);
#define os_snprintf (int)Sprintf_s
#if defined(_WIN64)
//typedef unsigned __int64        size_t;
#else

#ifndef __size_t__
#define __size_t__
typedef unsigned int            size_t;
#endif // __size_t__

#endif

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
#endif /* BUILD_CONFIG_H */

