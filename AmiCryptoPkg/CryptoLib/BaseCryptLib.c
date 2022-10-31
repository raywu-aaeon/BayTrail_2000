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
//----------------------------------------------------------------------------
//
// Name:		BaseCryptLib.h
//
// Description:	Defines base cryptographic library APIs compatible with EDK2 BaseCryptLib.
//      The Base Cryptographic Library provides implementations of basic cryptography
//      primitives (SHA-1, SHA-256, RSA, etc) for UEFI security functionality enabling.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


#include <Token.h>
#include <BaseCryptLib.h>
#include <CryptLib.h>
#include "includes.h"
#include "common.h"
#include "bignum.h"
#include "rsa.h"

// Structure is defined in rsa.c
struct crypto_rsa_key {
    int private_key; /* whether private key is set */
    struct bignum *n; /* modulus (p * q) */
    struct bignum *e; /* public exponent */
    /* The following parameters are available only if private_key is set */
//    struct bignum *d; /* private exponent */
//    struct bignum *p; /* prime p (factor of n) */
//    struct bignum *q; /* prime q (factor of n) */
//    struct bignum *dmp1; /* d mod (p - 1); CRT exponent */
//    struct bignum *dmq1; /* d mod (q - 1); CRT exponent */
//    struct bignum *iqmp; /* 1 / q mod p; CRT coefficient */
};

struct SHA1Context {
	UINT32 state[5];
	UINT32 count[2];
	UINT8 buffer[64];
};
void SHA1Init(struct SHA1Context *context);
void SHA1Update(struct SHA1Context *context, const void *data, UINT32 len);
void SHA1Final(unsigned char digest[20], struct SHA1Context *context);
struct sha256_state {
	UINT64 length;
	UINT32 state[8], curlen;
	UINT8  buf[64];
};
void sha256_init(struct sha256_state *md);
int sha256_process(struct sha256_state *md, const unsigned char *in, unsigned long inlen);
int sha256_done(struct sha256_state *md, unsigned char *out);


//struct Rsa_Key_Context {
//    VOID *CRheap;
//    struct crypto_rsa_key *key;
//};

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-1 hash operations.

  @return  The size, in bytes, of the context buffer required for SHA-1 hash operations.

**/
UINTN Sha1GetContextSize(
    VOID)
{
    return sizeof(struct SHA1Context);
}


/**
  Initializes user-supplied memory pointed by Sha1Context as the SHA-1 hash context for
  subsequent use.

  If Sha1Context is NULL, then ASSERT().

  @param[in, out]  Sha1Context  Pointer to the SHA-1 Context being initialized.

  @retval TRUE  SHA-1 initialization succeeded.

**/
BOOLEAN Sha1Init(
    IN OUT  VOID  *Sha1Context)
{
    SHA1Init((struct SHA1Context*)Sha1Context);
    return TRUE;
}


/**
  Performs SHA-1 digest on a data buffer of the specified length. This function can
  be called multiple times to compute the digest of long or discontinuous data streams.

  If Sha1Context is NULL, then ASSERT().

  @param[in, out]  Sha1Context  Pointer to the SHA-1 context.
  @param[in]       Data         Pointer to the buffer containing the data to be hashed.
  @param[in]       DataLength   Length of Data buffer in bytes.

  @retval TRUE   SHA-1 data digest succeeded.
  @retval FALSE  Invalid SHA-1 context. After Sha1Final function has been called, the
                 SHA-1 context cannot be reused.

**/
BOOLEAN Sha1Update(
  IN OUT  VOID        *Sha1Context,
  IN      CONST VOID  *Data,
  IN      UINTN       DataLength)
{
    SHA1Update((struct SHA1Context*)Sha1Context, (UINT8*)Data, (UINT32)DataLength);
    return TRUE;
}


/**
  Completes SHA-1 hash computation and retrieves the digest value into the specified
  memory. After this function has been called, the SHA-1 context cannot be used again.

  If Sha1Context is NULL, then ASSERT().
  If HashValue is NULL, then ASSERT().

  @param[in, out]  Sha1Context  Pointer to the SHA-1 context
  @param[out]      HashValue    Pointer to a buffer that receives the SHA-1 digest
                                value (20 bytes).

  @retval TRUE  SHA-1 digest computation succeeded.

**/
BOOLEAN Sha1Final(
    IN OUT  VOID   *Sha1Context,
    OUT     UINT8  *HashValue)
{
    SHA1Final(HashValue, (struct SHA1Context *)Sha1Context);
    return TRUE;
}


/**
  Retrieves the size, in bytes, of the context buffer required for SHA-256 operations.

  @return  The size, in bytes, of the context buffer required for SHA-256 operations.

**/
UINTN Sha256GetContextSize(
    VOID)
{
    return sizeof(struct sha256_state);
}


/**
  Initializes user-supplied memory pointed by Sha256Context as SHA-256 hash context for
  subsequent use.

  If Sha256Context is NULL, then ASSERT().

  @param[in, out]  Sha256Context  Pointer to SHA-256 Context being initialized.

  @retval TRUE  SHA-256 context initialization succeeded.

**/
BOOLEAN Sha256Init (
    IN OUT  VOID  *Sha256Context)
{
    sha256_init((struct sha256_state *)Sha256Context);
    return TRUE;
}


/**
  Performs SHA-256 digest on a data buffer of the specified length. This function can
  be called multiple times to compute the digest of long or discontinuous data streams.

  If Sha256Context is NULL, then ASSERT().

  @param[in, out]  Sha256Context  Pointer to the SHA-256 context.
  @param[in]       Data           Pointer to the buffer containing the data to be hashed.
  @param[in]       DataLength     Length of Data buffer in bytes.

  @retval TRUE   SHA-256 data digest succeeded.
  @retval FALSE  Invalid SHA-256 context. After Sha256Final function has been called, the
                 SHA-256 context cannot be reused.

**/
BOOLEAN Sha256Update(
    IN OUT  VOID        *Sha256Context,
    IN      CONST VOID  *Data,
    IN      UINTN       DataLength)
{
    return sha256_process(
        (struct sha256_state *)Sha256Context, 
        (const unsigned char *)Data, 
        (unsigned long)DataLength) == 0?TRUE:FALSE;
}


/**
  Completes SHA-256 hash computation and retrieves the digest value into the specified
  memory. After this function has been called, the SHA-256 context cannot be used again.

  If Sha256Context is NULL, then ASSERT().
  If HashValue is NULL, then ASSERT().

  @param[in, out]  Sha256Context  Pointer to SHA-256 context
  @param[out]      HashValue      Pointer to a buffer that receives the SHA-256 digest
                                  value (32 bytes).

  @retval TRUE  SHA-256 digest computation succeeded.

**/
BOOLEAN Sha256Final(
    IN OUT  VOID   *Sha256Context,
    OUT     UINT8  *HashValue)
{
    return sha256_done(
        (struct sha256_state *)Sha256Context, 
        HashValue) == 0 ? TRUE:FALSE;
}


/**
  Allocates and Initializes one RSA Context for subsequent use.

  @return  Pointer to the RSA Context that has been initialized.
           If the allocations fails, RsaNew() returns NULL.

**/
VOID* RsaNew(
    VOID)
{
    struct crypto_rsa_key *RsaKey;

    ResetCRmm();

    RsaKey = os_malloc(sizeof(*RsaKey));
    if (RsaKey == NULL)
            return FALSE;

    return RsaKey;
}


/**
  Release the specified RSA Context.

  @param[in]  RsaContext  Pointer to the RSA context to be released.

**/
VOID RsaFree (
    IN  VOID  *RsaContext)
{  
// Security concern, memory heap is being cleared on exit 
    ResetCRmm();

    return;
}

/**
  Sets the tag-designated RSA key component into the established RSA context from
  the user-specified nonnegative integer (octet string format represented in RSA
  PKCS#1).

  If RsaContext is NULL, then ASSERT().

  @param[in, out]  RsaContext  Pointer to RSA context being set.
  @param[in]       KeyTag      Tag of RSA key component being set.
  @param[in]       BigNumber   Pointer to octet integer buffer.
  @param[in]       BnLength    Length of big number buffer in bytes.

  @return  TRUE   RSA key component was set successfully.
  @return  FALSE  Invalid RSA key component tag.

**/
BOOLEAN RsaSetKey(
    IN OUT VOID         *RsaContext,
    IN     RSA_KEY_TAG  KeyTag,
    IN     CONST UINT8  *BigNumber,
    IN     UINTN        BnLength)
{
    struct crypto_rsa_key *key;
	UINT8 *KeyNmod;

    if(RsaContext==NULL)
        return FALSE;

    key = (struct crypto_rsa_key *)RsaContext;

    if(KeyTag==RsaKeyN) {
// !!! NOTE !!!!
// KeyGen adds a leading ZERO if the msb of the first byte of the n-modulus is ONE. 
// This is to avoid this integer to be treated as negative value.
// If your calculations produce a number with the high bit set to 1, 
// just increase the length by another byte and pad the beginning with 0x00 
// to keep it positive.
    	KeyNmod = (UINT8*)BigNumber;
    	if(BigNumber[0] >> 7 == 1)
    	{
            KeyNmod = os_malloc((unsigned int)(BnLength+1));
    		MemCpy((void*)&KeyNmod[1], (void*)BigNumber, BnLength);
    		KeyNmod[0] = 0;
    		BnLength++;
    	}
    
        key->n = bignum_init();
        if (key->n == NULL)
                goto error; 

//         * RSA2048PublicKey ::= 
//         *     modulus INTEGER, -- n

        if(bignum_set_unsigned_bin(key->n, KeyNmod, BnLength) < 0 )
            goto error;
    } else 
        if(KeyTag==RsaKeyE) {
            key->e = bignum_init();
            if (key->e == NULL)
                goto error; 
//         * RSA2048PublicKey ::= 
//         *     exponent INTEGER, -- e
            if(bignum_set_unsigned_bin(key->e, BigNumber, BnLength) < 0 )
                goto error;
        } else // unknown Key Tag
                return FALSE;

    return TRUE;

error:
    crypto_rsa_free(key);
    return FALSE;
}


/**
  Verifies the RSA-SSA signature with EMSA-PKCS1-v1_5 encoding scheme defined in
  RSA PKCS#1.

  If RsaContext is NULL, then ASSERT().
  If MessageHash is NULL, then ASSERT().
  If Signature is NULL, then ASSERT().
  If HashLength is not equal to the size of MD5, SHA-1 or SHA-256 digest, then ASSERT().

  @param[in]  RsaContext   Pointer to RSA context for signature verification.
  @param[in]  MessageHash  Pointer to octet message hash to be checked.
  @param[in]  HashLength   Length of the message hash in bytes.
  @param[in]  Signature    Pointer to RSA PKCS1-v1_5 signature to be verified.
  @param[in]  SigLength    Length of signature in bytes.

  @return  TRUE   Valid signature encoded in PKCS1-v1_5.
  @return  FALSE  Invalid signature or invalid RSA context.

**/
BOOLEAN RsaPkcs1Verify(
    IN  VOID         *RsaContext,
    IN  CONST UINT8  *MessageHash,
    IN  UINTN        HashLength,
    IN  UINT8        *Signature,
    IN  UINTN        SigLength)
{
    INTN        err;
    size_t     *sig_len;
    UINT8       DecriptedSig[DEFAULT_RSA_SIG_LEN];

    sig_len=(size_t*)&SigLength;

    if(RsaContext == NULL )
        return FALSE;

    err = crypto_rsa_exptmod((const UINT8*)Signature, *sig_len, (UINT8*)&DecriptedSig, sig_len, (struct crypto_rsa_key*)RsaContext, 0);

// locate Hash inside the decrypted signature body and compare it with given Hash;
// !!! only supporting PKCS#1 v1_5 Padding
    if(!err) {
//        err = pkcs_1_v1_5_decode(MessageHash, HashLength, (const UINT8 *)&DecriptedSig, (unsigned long)*sig_len);
        err = MemCmp((void*)MessageHash, (void*)((UINT32)DecriptedSig + (UINT32)(*sig_len - HashLength)), HashLength);
    }

    return !err ? TRUE:FALSE;
}

#ifndef PEI_BUILD

/**
  Verifies the validity of a PKCS#7 signed data as described in "PKCS #7: Cryptographic
  Message Syntax Standard".

  If P7Data is NULL, then ASSERT().

  @param[in]  P7Data       Pointer to the PKCS#7 message to verify.
  @param[in]  P7Length     Length of the PKCS#7 message in bytes.
  @param[in]  TrustedCert  Pointer to a trusted/root certificate encoded in DER, which
                           is used for certificate chain verification.
  @param[in]  CertLength   Length of the trusted certificate in bytes.
  @param[in]  InData       Pointer to the content to be verified.
  @param[in]  DataLength   Length of InData in bytes.

  @return  TRUE  The specified PKCS#7 signed data is valid.
  @return  FALSE Invalid PKCS#7 signed data.

**/
BOOLEAN Pkcs7Verify(
  IN  CONST UINT8  *P7Data,
  IN  UINTN        P7Length,
  IN  CONST UINT8  *TrustedCert,
  IN  UINTN        CertLength,
  IN  CONST UINT8  *InData,
  IN  UINTN        DataLength
  )
{
//    UINT8       Hash[SHA256_DIGEST_SIZE];
//    UINT8       HashAlgo;
    INTN        err, reason;
    struct pkcs7_signed_data_st* PKCS7cert;
    struct x509_certificate *x509SignCert=NULL;//, *x509RootCert;
    struct x509_certificate *x509TrustCert;
    
    err = 0;

    ResetCRmm();

    PKCS7cert = Pkcs7_parse_Authenticode_certificate(P7Data, P7Length);
    if (PKCS7cert) {
        // verify Pkcs7 Signing Cert chain up to the TrustCert
        if(TrustedCert && CertLength) {
            x509TrustCert = x509_certificate_parse(TrustedCert, CertLength);
            if(x509TrustCert)
                err = Pkcs7_x509_certificate_chain_validate(PKCS7cert, x509TrustCert, (int*)&reason);
             else 
                err = -1;
        } 
        //
        // For generic PKCS#7 handling, InData may be NULL if the content is present
        // in PKCS#7 structure. So ignore NULL checking here.
        //
/*
        if(InData && DataLength > SHA256_DIGEST_SIZE) {
            err = Pkcs7_return_digestAlgorithm(PKCS7cert, (UINT8*)&HashAlgo);
            if(HashAlgo == SHA1) {
                sha1_vector( 1, &InData, (const UINTN*)&DataLength, Hash);
                DataLength = SHA1_DIGEST_SIZE;
            } else if(HashAlgo == SHA256) {
                sha256_vector( 1, &InData, (const UINTN*)&DataLength, Hash);
                DataLength = SHA256_DIGEST_SIZE;
            } else {err = -1; goto Done;}

            InData = (UINT8*)&Hash[0];
        }
*/
        // x509SignCert== NULL -> extract SignCert from Pkcs7 crt
        err = Pkcs7_certificate_validate_digest(PKCS7cert, x509SignCert, (UINT8*)InData, DataLength);
     } else 
         err = -1;
//Done:
// Security concern, memory heap is being cleared on exit 
    ResetCRmm();

    return !err ? TRUE:FALSE;
}
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
