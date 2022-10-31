
#ifndef _PEI_HASHPPI_SHA256_H_
#define _PEI_HASHPPI_SHA256_H_

#define PEI_HASH_SHA256_PPI_GUID \
    {0x878af489, 0xde44, 0x44f1, 0xb6, 0x9e, 0xce, 0xe5, 0x7f, 0x2c, 0xa1, 0x96}

/**
  The Stall() function provides a blocking stall for at least the number 
  of microseconds stipulated in the final argument of the API.

  @param  PeiServices    An indirect pointer to the PEI Services Table
                         published by the PEI Foundation.
  @param  This           Pointer to the local data for the interface.
  @param  Microseconds   Number of microseconds for which to stall.

  @retval EFI_SUCCESS    The service provided at least the required delay.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_PEI_HASH_SHA256_RUN)(
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN UINT32                      Address,
  IN UINT32                      Size,
  IN UINT32                      HashAddress
  );

///
/// This service provides a simple, blocking stall with platform-specific resolution. 
///
typedef struct  {
    EFI_PEI_HASH_SHA256_RUN  HashSha256;
}EFI_HASH_SHA256_PPI;

#endif
