//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/BIN/Core/CORE_DXE/BdsBoard.c 62    11/01/11 1:41p Felixp $
//
// $Revision: 62 $
//
// $Date: 11/01/11 1:41p $
//**********************************************************************


//<AMI_FHDR_START>
//---------------------------------------------------------------------------
// Name:        BdsBoard.C
//
// Description: This file contains BDS/CORE_DXE related OEM code.  There are
//              variables defined in this file that might change for each
//              OEM project
////---------------------------------------------------------------------------
//<AMI_FHDR_END>

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------
#include "BootOptions.h"
#include <Protocol/GraphicsOutput.h>
#include <Protocol/Decompress.h>
//#include <CORE_DXEStrTokens.h>
#include <Setup.h>
//#include <GenericSio.h>
#include <BootOptioneLinks.h>
#ifdef CSM_SUPPORT
#include <Protocol/LegacyBiosExt.h>
#endif

//---------------------------------------------------------------------------
// MACRO Constants
//---------------------------------------------------------------------------
#define _AND_               &       // (EIP7580)+

#ifndef FW_ORPHAN_BOOT_OPTIONS_POLICY
#define FW_ORPHAN_BOOT_OPTIONS_POLICY ORPHAN_BOOT_OPTIONS_POLICY_DELETE
#endif
#ifndef NON_FW_ORPHAN_BOOT_OPTIONS_POLICY
#define NON_FW_ORPHAN_BOOT_OPTIONS_POLICY ORPHAN_BOOT_OPTIONS_POLICY_KEEP
#endif
#ifndef ORPHAN_GROUP_HEADERS_POLICY
#define ORPHAN_GROUP_HEADERS_POLICY ORPHAN_BOOT_OPTIONS_POLICY_DELETE
#endif

//---------------------------------------------------------------------------
// Type definitions
//---------------------------------------------------------------------------
typedef enum{
//    BoTagLegacyXxx
//    BoTagUefiXxx
//    BoTagXxx
    BoTagLegacyFloppy,
    BoTagLegacyHardDisk,
    BoTagLegacyCdrom,
    BoTagLegacyPcmcia,
    BoTagLegacyUsb,
    BoTagLegacyEmbedNetwork,
    BoTagLegacyBevDevice,
    BoTagUefi,
    BoTagEmbeddedShell
} BOOT_OPTION_TAG;

//---------------------------------------------------------------------------
// Constant and Variables declarations
//---------------------------------------------------------------------------
#ifndef DEFAULT_BOOT_TIMEOUT
#define DEFAULT_BOOT_TIMEOUT 1
#endif
const   UINT16  DefaultTimeout  = DEFAULT_BOOT_TIMEOUT;

STRING_REF BbsDevTypeNameToken[] = {
	STRING_TOKEN(STR_FD),
	STRING_TOKEN(STR_HD),
	STRING_TOKEN(STR_CD),
	STRING_TOKEN(STR_PCMCIA),
	STRING_TOKEN(STR_USB),
	STRING_TOKEN(STR_NET),
	STRING_TOKEN(STR_BEV),
	STRING_TOKEN(STR_UNKNOWN)
};

struct{
    UINT8   Type;
    UINT8	SubType; 			
    STRING_REF StrToken;
} DpStrings[] = {   
    {MESSAGING_DEVICE_PATH, MSG_ATAPI_DP, STRING_TOKEN(STR_ATAPI)},
    {MESSAGING_DEVICE_PATH, MSG_SCSI_DP, STRING_TOKEN(STR_SCSI)},
	{MESSAGING_DEVICE_PATH, MSG_USB_DP, STRING_TOKEN(STR_USB)},
	{MESSAGING_DEVICE_PATH, MSG_MAC_ADDR_DP, STRING_TOKEN(STR_NET)},
	{MEDIA_DEVICE_PATH, MEDIA_HARDDRIVE_DP, STRING_TOKEN(STR_HD)},
	{MEDIA_DEVICE_PATH, MEDIA_CDROM_DP, STRING_TOKEN(STR_CD)}
};

//these GUIDs are used by BDS.c
EFI_GUID    SetupEnterProtocolGuid=AMITSE_SETUP_ENTER_GUID;
EFI_GUID    SecondBootOptionProtocolGuid=AMITSE_AFTER_FIRST_BOOT_OPTION_GUID;
EFI_GUID    BeforeBootProtocolGuid = AMITSE_EVENT_BEFORE_BOOT_GUID;
#ifndef EFI_AMI_LEGACYBOOT_PROTOCOL_GUID
#define EFI_AMI_LEGACYBOOT_PROTOCOL_GUID            \
  {0x120d28aa, 0x6630, 0x46f0, 0x81, 0x57, 0xc0, 0xad, 0xc2, 0x38, 0x3b, 0xf5}
#endif
EFI_GUID    BeforeLegacyBootProtocolGuid = EFI_AMI_LEGACYBOOT_PROTOCOL_GUID;
EFI_GUID    ShellFfsFileNameGuid = SHELL_GUID;
#if Shell_SUPPORT
EFI_GUID    *DefaultAppFfsGuidPtr = &ShellFfsFileNameGuid;
#else
UINT8    Edk2ShellGuid[] =
    { 0xB7, 0xD6, 0x7A, 0xC5, 0x15, 0x05, 0xA8, 0x40, 0x9D, 0x21, 0x55, 0x16, 0x52, 0x85, 0x4E, 0x37 };
EFI_GUID    *DefaultAppFfsGuidPtr = (EFI_GUID*)&Edk2ShellGuid;
#endif

//EFI_GUID    SetupVariableGuid = SETUP_GUID;

struct {
	VENDOR_DEVICE_PATH media;
	MEDIA_FW_VOL_FILEPATH_DEVICE_PATH ffs;
	EFI_DEVICE_PATH_PROTOCOL end;
} ShellDp = {
	{
        {
            MEDIA_DEVICE_PATH, MEDIA_VENDOR_DP,
            sizeof(VENDOR_DEVICE_PATH)
        },
        AMI_MEDIA_DEVICE_PATH_GUID
    },
	{
        {
            MEDIA_DEVICE_PATH, MEDIA_FV_FILEPATH_DP,
            sizeof(MEDIA_FW_VOL_FILEPATH_DEVICE_PATH)
        },
        SHELL_GUID
    },
	{
        END_DEVICE_PATH, END_ENTIRE_SUBTYPE,
        sizeof(EFI_DEVICE_PATH_PROTOCOL)
    }
};

const STRING_REF UnknownDeviceToken = STRING_TOKEN(STR_UNKNOWN);

BOOT_OPTION_TAG LegacyBootOptionTags[] = {
    BoTagLegacyFloppy,
    BoTagLegacyHardDisk,
    BoTagLegacyCdrom,
    BoTagLegacyPcmcia,
    BoTagLegacyUsb,
    BoTagLegacyEmbedNetwork,
    BoTagLegacyBevDevice,
    UNASSIGNED_HIGHEST_TAG
};

BOOT_OPTION_TAG BootOptionTagPriorities[] = {
    BOOT_OPTION_TAG_PRIORITIES,
    UNASSIGNED_HIGHEST_TAG
};

BOOLEAN NormalizeBootOptionName = NORMALIZE_BOOT_OPTION_NAME;
BOOLEAN NormalizeBootOptionDevicePath = NORMALIZE_BOOT_OPTION_DEVICE_PATH;

//---------------------------------------------------------------------------
// External variables
//---------------------------------------------------------------------------
extern  EFI_GUID        EfiVariableGuid;
/*
#if SIO_SUPPORT										// (EIP8888)+
extern  SPIO_LIST_ITEM  SIO_DEVICE_LIST EndOfList;  // (EIP7580)+
#endif												// (EIP8888)-
//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------

#if SIO_SUPPORT	// (EIP8888)+
SPIO_LIST_ITEM  *gSpioList[] = {SIO_DEVICE_PTR_LIST NULL};  // (EIP7580)+
#endif			// (EIP8888)-

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------
*/
//---------------------------------------------------------------------------
// Function Implementations
//---------------------------------------------------------------------------
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DevicePathNodeToStrRef
//
// Description: This function converts node from the device path to a string.
//              Once the whole device path is converted the string is used as
//              as a boot opton name.
//              This function is only used is component name protocol does not report device name.
//
// Input:       EFI_DEVICE_PATH_PROTOCOL *Dp  - pointer to the device path node
//
// Output:      STRING_REF - string token (-1, if the string token is not available)
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
STRING_REF DevicePathNodeToStrRef(EFI_DEVICE_PATH_PROTOCOL *Dp){
    UINTN i;
    //Built in Shell is a special case
    if (   NODE_LENGTH(Dp)==NODE_LENGTH(&ShellDp.media.Header)
        && MemCmp(Dp,&ShellDp.media.Header,NODE_LENGTH(Dp))==0
    ) return STRING_TOKEN(STR_SHELL);
    //BBS device path is another special case
#ifdef CSM_SUPPORT
    if ( Dp->Type==BBS_DEVICE_PATH ){
        return BbsDevTypeNameToken[BBS_DEVICE_TYPE_TO_INDEX(((BBS_BBS_DEVICE_PATH*)Dp)->DeviceType)];
    }
#endif
    for (i=0; i < sizeof(DpStrings)/sizeof(DpStrings[0]); i++){
        if ((Dp->Type==DpStrings[i].Type) && (Dp->SubType==DpStrings[i].SubType)){
            return DpStrings[i].StrToken;
        }		
    }//for i

    return INVALID_STR_TOKEN;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetBbsEntryDeviceTypeDefault
//
// Description: Using the passed BBS_TABLE entry pointer, determine the device
//              type of the associated device
//
// Input:       BBS_TABLE *BbsEntry - pointer to a BBS_TABLE entry
//
// Output:      UINT16 - device type of the BBS entry, see LegacyBios.h for actual values
//                  BBS_FLOPPY
//                  BBS_HARDDISK
//                  BBS_CDROM
//                  BBS_PCMCIA
//                  BBS_USB
//                  BBS_EMBED_NETWORK
//                  BBS_BEV_DEVICE
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 GetBbsEntryDeviceTypeDefault(BBS_TABLE *BbsEntry){
#ifdef CSM_SUPPORT
    UINT16 DeviceType = BbsEntry->DeviceType;
#if BBS_USB_DEVICE_TYPE_SUPPORT
    if (   BbsEntry->Class == PCI_CL_SER_BUS
        && BbsEntry->SubClass == PCI_CL_SER_BUS_SCL_USB
    ) return BBS_USB;
#endif
#if BBS_NETWORK_DEVICE_TYPE_SUPPORT
    if (   BbsEntry->Class == PCI_CL_NETWORK
        && BbsEntry->DeviceType == BBS_BEV_DEVICE
    ) return BBS_EMBED_NETWORK;
#endif
    return DeviceType;
#else
    return 0;
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetLegacyDevOrderType
//
// Description: Using the passed BOOT_OPTION structure, which should correspond to
//              a legacy device entry, determine the device type 
//
// Input:       BOOT_OPTION *Option - pointer to the BOOT_OPTION structure for the 
//                  device in question
//
// Output:      UINT16 - device type of the BOOT_OPTION item, see LegacyBios.h for actual values
//                  BBS_FLOPPY
//                  BBS_HARDDISK
//                  BBS_CDROM
//                  BBS_PCMCIA
//                  BBS_USB
//                  BBS_EMBED_NETWORK
//                  BBS_BEV_DEVICE
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 GetLegacyDevOrderType(BOOT_OPTION *Option){
    return ((BBS_BBS_DEVICE_PATH*)Option->FilePathList)->DeviceType;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindTagPriority
//
// Description: For the passed Tag entry, return the correct boot priority based on 
//              the BootOptionTagPriorities global variable that is filled out
//              based on the SDL Token BOOT_OPTION_TAG_PRIORITIES.
//
// Input:       UINT16 Tag - one of the following items of the BOOT_OPTION_TAG enum:
//                  BoTagLegacyFloppy
//                  BoTagLegacyHardDisk
//                  BoTagLegacyCdrom
//                  BoTagLegacyPcmcia
//                  BoTagLegacyUsb
//                  BoTagLegacyEmbedNetwork
//                  BoTagLegacyBevDevice
//                  BoTagUefi
//                  BoTagEmbeddedShell
//
// Output:      The index of this item in the BootOptionTagPriorities structure, which
//              also corresponds to the boot priority that should be assigned to this
//              class of device
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 FindTagPriority(UINT16 Tag){
    UINT32 i;
    for(i=0; BootOptionTagPriorities[i]!=UNASSIGNED_HIGHEST_TAG; i++)
        if (Tag==BootOptionTagPriorities[i]) return i;
    return UNASSIGNED_HIGHEST_TAG;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsShellBootOption
//
// Description: Determine if the passed BOOT_OPTION is the built in EFI Shell
//
// Input:       BOOT_OPTION *Option - the boot option in question
//
// Output:      BOOLEAN -   TRUE - this boot option represent the built in EFI Shell
//                          FALSE - this is not the built in EFI Shell
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsShellBootOption(BOOT_OPTION *Option){
    EFI_DEVICE_PATH_PROTOCOL *Dp = Option->FilePathList;

    if (Dp==NULL) return FALSE;
    if (   NODE_LENGTH(Dp)==NODE_LENGTH(&ShellDp.media.Header)
        && MemCmp(Dp,&ShellDp.media.Header,NODE_LENGTH(Dp))==0
    ) return TRUE;

    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetBootOptionTags
//
// Description: Go through the entire boot option list and Apply priorities for 
//              each entry in the list.
//
// Input:       DLIST *BootOptionList - the entire Boot Option List
//
// Output:      none
//
// Note:        To change boot order priorities, do not modify this function,
//              modify the SDL Token BootOptionTagPriorities.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetBootOptionTags(){
	DLINK *Link;
    BOOT_OPTION *Option;
    UINT32 UefiBootOptionsInc = 0x100;

    FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option){
        UINT32 TagPriority;
        UINT32 BaseTag = UNASSIGNED_HIGHEST_TAG;
#ifdef CSM_SUPPORT
        if (IsLegacyBootOption(Option)){
            UINT16 DeviceType;
            DeviceType = ((BBS_BBS_DEVICE_PATH*)Option->FilePathList)->DeviceType;
            BaseTag = LegacyBootOptionTags[BBS_DEVICE_TYPE_TO_INDEX(DeviceType)];
        }else
#endif
        if (IsShellBootOption(Option)) BaseTag = BoTagEmbeddedShell;
        else BaseTag = BoTagUefi;
        if (BaseTag == UNASSIGNED_HIGHEST_TAG) continue;
        TagPriority = FindTagPriority(BaseTag);
        //UEFI boot options must have unique tags, otherwise then will be groupped when 
        //GROUP_BOOT_OPTIONS_BY_TAG tokens is enabled
        if (BaseTag == BoTagUefi) BaseTag += UefiBootOptionsInc++;
        Option->Tag = BootOptionTag(BaseTag, TagPriority);
	}
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CreateGroupHeader
//
// Description: Create a Group Header Entry for the passed BOOT_OPTION and add the
//              group header to the master boot options list
//
// Input:       DLIST *BootOptionList - the master boot options list to add the newly
//                  created group item
//              BOOT_OPTION *FirstGroupOption - the boot option which needs linked to a 
//                  group item
//
// Output:      none
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CreateGroupHeader(DLIST *BootOptionList, BOOT_OPTION *FirstGroupOption){
#ifdef CSM_SUPPORT
    static struct {
		BBS_BBS_DEVICE_PATH bbs;
		EFI_DEVICE_PATH_PROTOCOL end;
	} BbsDpTemplate =  {
		{
            {BBS_DEVICE_PATH,BBS_BBS_DP,sizeof(BBS_BBS_DEVICE_PATH)},
            BBS_HARDDISK,0,0
        },
		{END_DEVICE_PATH,END_ENTIRE_SUBTYPE,sizeof(EFI_DEVICE_PATH_PROTOCOL)}
	};

    BOOT_OPTION *Option;

	if (!IsLegacyBootOption(FirstGroupOption)) return;
	Option = CreateBootOption(BootOptionList);
	Option->BootOptionNumber = FirstGroupOption->BootOptionNumber;
	Option->Priority = FirstGroupOption->Priority;
	Option->Tag = FirstGroupOption->Tag;
	Option->FwBootOption = TRUE;
    Option->GroupHeader = TRUE;
    BbsDpTemplate.bbs.DeviceType=GetBbsEntryDeviceType(FirstGroupOption->BbsEntry);
	Option->FilePathList = DPCopy(&BbsDpTemplate.bbs.Header);
	Option->FilePathListLength = DPLength(Option->FilePathList);
    ConstructBootOptionName(Option);
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetBootOptionPriorities
//
// Description: Go through the boot option list and set the priorities of each 
//              group of devices
//
// Input:       DLIST *BootOptionList - the master list of boot options
//
// Output:      none
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetBootOptionPriorities(){
	DLINK *Link;
#if GROUP_BOOT_OPTIONS_BY_TAG	
    UINT16 PreviousBootOptionNumber = INVALID_BOOT_OPTION_NUMBER;
    UINT32 PreviousTag = UNASSIGNED_HIGHEST_TAG;
#endif    
    UINT32 PreviousPriority=0;
    BOOT_OPTION *Option;
    UINT16 NextOptionNumber;

    //Detect first unused boot option number
    NextOptionNumber = 0;
    if (!DListEmpty(BootOptionList)){
        FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option){
            if (   Option->BootOptionNumber != INVALID_BOOT_OPTION_NUMBER 
                && Option->BootOptionNumber > NextOptionNumber 
            ) NextOptionNumber = Option->BootOptionNumber;
        }
        NextOptionNumber++;
    }

	SortList(BootOptionList, CompareTagThenPriority);
    FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option){
#if GROUP_BOOT_OPTIONS_BY_TAG
        if (Option->BootOptionNumber==INVALID_BOOT_OPTION_NUMBER){
            if (   PreviousTag != Option->Tag 
                || Option->Tag==UNASSIGNED_LOWEST_TAG 
                || Option->Tag==UNASSIGNED_HIGHEST_TAG
            ) PreviousBootOptionNumber=NextOptionNumber++;
            Option->BootOptionNumber=PreviousBootOptionNumber;
            Option->Priority=++PreviousPriority;
        }else{
            PreviousBootOptionNumber = Option->BootOptionNumber;
            PreviousPriority = Option->Priority;
        }
		PreviousTag = Option->Tag;
#else
        if (Option->BootOptionNumber==INVALID_BOOT_OPTION_NUMBER){
            Option->BootOptionNumber=(NextOptionNumber)++;
            Option->Priority=++PreviousPriority;
        }else{
            PreviousPriority = Option->Priority;
        }
#endif
	}
    DUMP_BOOT_OPTION_LIST(BootOptionList,"After Setting Priorities");
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CreateGroupHeaders
//
// Description: Go throuhg the the boot option list and  
//
// Input:       
//
// Output:      
//
// Note: This function assums that the BootOptionList is already sorted by tag and
//          then by priority.  failure to adhere to those assumptions will cause
//          undesired behavoir
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CreateGroupHeaders(DLIST *BootOptionList){
    DLINK *Link;
    BOOT_OPTION *Option;
    UINT32 PreviousTag = UNASSIGNED_LOWEST_TAG;

    //PRECONDITION: Boot Option List is sorted by tag then by priority

    DUMP_BOOT_OPTION_LIST(BootOptionList,"Before Adding Group Headers");
    FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option){
        if (PreviousTag == Option->Tag) continue;
        PreviousTag = Option->Tag;
        if (   Option->Tag==UNASSIGNED_LOWEST_TAG 
            || Option->Tag==UNASSIGNED_HIGHEST_TAG
        ) continue;
        if (!Option->GroupHeader) CreateGroupHeader(BootOptionList,Option);
	}
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ApplyOrphanBootOptionPolicy
//
// Description: Apply the correct policy to the passed orphaned boot iptions
//
// Input:       DLIST *BootOptionList - the master boot option list
//              BOOT_OPTION *Option - The orphaned boot option
//              UINTN Policy - the policy to follow, valid values are 
//                  ORPHAN_BOOT_OPTIONS_POLICY_DELETE
//                  ORPHAN_BOOT_OPTIONS_POLICY_DISABLE
//                  ORPHAN_BOOT_OPTIONS_POLICY_KEEP
//
// Output:      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ApplyOrphanBootOptionPolicy(
    DLIST *BootOptionList, BOOT_OPTION *Option, const int Policy
){
    if (Policy==ORPHAN_BOOT_OPTIONS_POLICY_DELETE){
        if (Option->BootOptionNumber!=INVALID_BOOT_OPTION_NUMBER){
            CHAR16 BootStr[9];
    		Swprintf(BootStr,L"Boot%04X",Option->BootOptionNumber);
    		pRS->SetVariable(
    			BootStr, &EfiVariableGuid, 0, 0, NULL
    		);
        }    
        DeleteBootOption(BootOptionList, Option);
    }else if (Policy==ORPHAN_BOOT_OPTIONS_POLICY_DISABLE){
        Option->Attributes &= ~LOAD_OPTION_ACTIVE;
    }else if (Policy==ORPHAN_BOOT_OPTIONS_POLICY_HIDE){
        Option->Attributes |= LOAD_OPTION_HIDDEN;
        MaskFilePathList(Option);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PreProcessBootOptions
//
// Description: Attempts to find stale boot options in the master boot option
//              list, and apply the specified policy to them. 
//
//              Policy is based on SDL tokens: FW_ORPHAN_BOOT_OPTIONS_POLICY and
//              NON_FW_ORPHAN_BOOT_OPTIONS_POLICY
//
// Input:       DLIST *BootOptionList - master boot option list
//
// Output:      none
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PreProcessBootOptions(){
    DLINK *Link;
    BOOT_OPTION *Option;

    // Process boot options not associated with the particular boot device.
    // We can't process group headers just yet because groups that
    // are non-empty now, can become empty at the end of this loop
    // once their orphan members are deleted.
    FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option){
        //skip group headers for now; we'll process them later 
		//in the PostProcessBootOptions
        if (Option->GroupHeader || IsBootOptionWithDevice(Option))
            continue;
        if (Option->FwBootOption) 
            ApplyOrphanBootOptionPolicy(
                BootOptionList, Option, FW_ORPHAN_BOOT_OPTIONS_POLICY
            );
        else
            ApplyOrphanBootOptionPolicy(
                BootOptionList, Option, NON_FW_ORPHAN_BOOT_OPTIONS_POLICY
            );
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PostProcessBootOptions
//
// Description: Go through the master boot option list and apply orphan boot option
//              policy to the boot option groups
//
// Input:       DLIST *BootOptionList - the master boot option list
//
// Output:      none
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PostProcessBootOptions(){
    DLINK *Link;
    BOOT_OPTION *Option;

    SortList(BootOptionList, CompareTagThenPriority);
    //Now we are ready for the processing of orphan group headers.
    //process empty groups ( groups with just the header)
    FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option){
        if (!Option->GroupHeader) continue;
        if (   Link==NULL 
            || OUTTER(Link,Link,BOOT_OPTION)->Tag != Option->Tag
        ) ApplyOrphanBootOptionPolicy(
            BootOptionList, Option, ORPHAN_GROUP_HEADERS_POLICY
          );
    }

#if GROUP_BOOT_OPTIONS_BY_TAG
    CreateGroupHeaders(BootOptionList);
#endif
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConstructBootOptionNamePrefixDefault
//
// Description: Based on the passed boot option, determine if a prefix
//              needs prepended to the front of the boot option name.  If return
//              value is non-zero, then the returned number of CHAR16s from the 
//              Name buffer should prepended to the front of the boot option name
//
// Input:       BOOT_OPTION *Option - boot option in question
//              CHAR16 *Name - pointer to the buffer in which to return the prefix
//              UINTN NameSize - size of the buffer being passed
//
// Output:      UINTN - size of the string being returned
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINTN ConstructBootOptionNamePrefixDefault(BOOT_OPTION *Option, CHAR16 *Name, UINTN NameSize){
    if (IsLegacyBootOption(Option)) return 0;
    //TODO: use string token
    return Swprintf(Name, L"UEFI: ");
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConstructBootOptionNameSuffixDefault
//
// Description: Based on the passed boot option, determine if anything needs appended
//              to the boot option name string.  If return value is not zero, then the
//              returned number of characers should be appended to the end of the name
//              string buffer.
//
// Input:       BOOT_OPTION *Option - the boot option in question
//              CHAR16 *Name - pointer to the buffer to returne the append string
//              UINTN NameSize - the size of the buffer being passed
//
// Output:      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINTN ConstructBootOptionNameSuffixDefault(
    BOOT_OPTION *Option, CHAR16 *Name, UINTN NameSize
){
    return 0;
}

extern DEVICE_PATH_TO_DEVICE_MATCH_TEST BOOT_OPTION_DP_MATCHING_FUNCTIONS EndOfDpMatchingFunctions;
DEVICE_PATH_TO_DEVICE_MATCH_TEST *DpMatchingFunction[] = {
    BOOT_OPTION_DP_MATCHING_FUNCTIONS NULL
};

extern BOOT_OPTION_TO_DEVICE_MATCH_TEST BOOT_OPTION_MATCHING_FUNCTIONS EndOfMatchingFunctions;
BOOT_OPTION_TO_DEVICE_MATCH_TEST *MatchingFunction[] = {
    BOOT_OPTION_MATCHING_FUNCTIONS NULL
};

extern FILTER_BOOT_DEVICE_TEST BOOT_OPTION_BOOT_DEVICE_FILTERING_FUNCTIONS EndOfFilteringFunctions;
FILTER_BOOT_DEVICE_TEST *FilteringFunction[] = {
    BOOT_OPTION_BOOT_DEVICE_FILTERING_FUNCTIONS NULL
};

extern CONSTRUCT_BOOT_OPTION_NAME BOOT_OPTION_BUILD_NAME_FUNCTIONS EndOfBuildNameFunctions;
CONSTRUCT_BOOT_OPTION_NAME *BuildNameFunctions[] = {
    BOOT_OPTION_BUILD_NAME_FUNCTIONS NULL
};

extern BUILD_BOOT_OPTION_FILE_PATH_LIST BOOT_OPTION_BUILD_FILE_PATH_FUNCTIONS EndOfBuildFilePathFunctions;
BUILD_BOOT_OPTION_FILE_PATH_LIST *BuildFilePathFunctions[] = {
  BOOT_OPTION_BUILD_FILE_PATH_FUNCTIONS NULL
};

extern CONSTRUCT_BOOT_OPTION_NAME BOOT_OPTION_NAME_PREFIX_FUNCTION;
CONSTRUCT_BOOT_OPTION_NAME *ConstructBootOptionNamePrefix = BOOT_OPTION_NAME_PREFIX_FUNCTION;

extern CONSTRUCT_BOOT_OPTION_NAME BOOT_OPTION_NAME_SUFFIX_FUNCTION;
CONSTRUCT_BOOT_OPTION_NAME *ConstructBootOptionNameSuffix = BOOT_OPTION_NAME_SUFFIX_FUNCTION;

#ifndef BOOT_OPTION_GET_BBS_ENTRY_DEVICE_TYPE_FUNCTION
#define BOOT_OPTION_GET_BBS_ENTRY_DEVICE_TYPE_FUNCTION GetBbsEntryDeviceTypeDefault
#endif
extern GET_BBS_ENTRY_DEVICE_TYPE BOOT_OPTION_GET_BBS_ENTRY_DEVICE_TYPE_FUNCTION;
GET_BBS_ENTRY_DEVICE_TYPE *GetBbsEntryDeviceType = BOOT_OPTION_GET_BBS_ENTRY_DEVICE_TYPE_FUNCTION;

#if FAST_BOOT_SUPPORT
VOID FastBoot();
BOOLEAN IsFastBoot();

VOID FastBootHook(){
    if(IsFastBoot()) FastBoot();
}
#endif

extern BDS_CONTROL_FLOW_FUNCTION BDS_CONTROL_FLOW EndOfBdsControlFlowFunctions;
BDS_CONTROL_FLOW_FUNCTION *BdsControlFlowFunctions[] = {
    BDS_CONTROL_FLOW NULL
};
CHAR8 *BdsControlFlowFunctionNames[] = {
    BDS_CONTROL_FLOW_NAMES NULL
};

CONST CHAR16 *FirmwareVendorString = CONVERT_TO_WSTRING(CORE_VENDOR);
CONST UINT32 FirmwareRevision = CORE_COMBINED_VERSION;

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
