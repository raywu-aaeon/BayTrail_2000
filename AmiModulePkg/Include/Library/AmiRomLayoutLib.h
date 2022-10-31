//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file
  @brief Defines AmiRomLayout library class.

  ROM Layout Processing Functions.
**/
#ifndef __AMI_ROM_LAYOUT_LIB__H__
#define __AMI_ROM_LAYOUT_LIB__H__
#include <AmiRomLayout.h>

EFI_STATUS AmiGetRomLayoutProperties(
	OUT UINT32 *Version OPTIONAL, OUT UINT32 *RomAreaSize OPTIONAL, OUT UINT32 *RomLayoutSize OPTIONAL
);

/*
  Return a pointer to the first entry of AMI_ROM_AREA structure.

  @retparam AMI_ROM_AREA*       Pointer to the first entry of the AMI_ROM_AREA structure
  @retparam NULL                The AMI_ROM_AREA structure could not be found.
*/
AMI_ROM_AREA* AmiGetFirstRomArea(VOID);

/*
  Using the passed parameter as the starting entry, return a pointer to the next AMI_ROM_AREA entry.

  @param[in] AMI_ROM_AREA *     Pointer to the starting entry

  @retparam  AMI_ROM_AREA *     Pointer to the next entry after the passed starting entry
  @retparam  NULL               There were no more entries after the starting entry, or the starting
                                entry was not a valid pointer.
 */
AMI_ROM_AREA* AmiGetNextRomArea(AMI_ROM_AREA *Start);

/*
  Return a pointer to the single ROM_AREA structure that covers the address passed into this
  function. If the address does not correspond to an address in the ROM_AREA structures, NULL will
  be returned

  @param[in] EFI_PHYSICAL_ADDRESS   The address of the region in question being queried
  @param[out] ROM_AREA **           The pointer to the ROM_AREA structure that describes the region
                                    which encompasses the address

  @retparam EFI_SUCCESS             The region was found, and returned in the RomArea parameter
  @retparam EFI_NOT_FOUND           The region was not found.
*/
AMI_ROM_AREA* AmiGetRomAreaByAddress(IN EFI_PHYSICAL_ADDRESS Address);

/*
  Return a pointer to the first ROM_AREA structure with the AreaGuid.

  @param[in] AreaGuid               The address of the region in question being queried
  @param[out] ROM_AREA **           The pointer to the ROM_AREA structure that describes the region
                                    which encompasses the address

  @retparam EFI_SUCCESS             The region was found, and returned in the RomArea parameter
  @retparam EFI_NOT_FOUND           The region was not found.
*/
AMI_ROM_AREA* AmiGetFirstRomAreaByGuid(IN EFI_GUID *AreaGuid);

// There may be more than one area with the same GUID.
// Set PreviousRomArea to NULL to get the first area.
AMI_ROM_AREA* AmiGetNextRomAreaByGuid(IN EFI_GUID *AreaGuid, IN AMI_ROM_AREA *PreviousRomArea);

// Searches ROM image at ImageAddress for a ROM Layout table.
EFI_STATUS AmiGetImageRomLayout(
	IN VOID *ImageAddress, IN UINTN ImageSize,
	OUT AMI_ROM_AREA **RomLayout, OUT UINT32 *Version OPTIONAL, 
	OUT UINT32 *RomAreaSize OPTIONAL, OUT UINT32 *RomLayoutSize OPTIONAL
);

EFI_STATUS AmiPublishFv(IN EFI_GUID *FvName);

EFI_STATUS AmiPublishFvArea(IN AMI_ROM_AREA *FvArea);

#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
