#include <Token.h>
#include <AmiPeiLib.h>
#include <RomLayout.h>

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BlockReportFvBB
//
// Description: Function to block publishing FV_BB when FV_BB_BACKUP hob exist
//              ELINK into ProcessFvBeforePublishing
//
// Input:       IN EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//              IN OUT ROM_AREA *Area - pointer to ROM Area, which contains FV
//                                      to be processed
//              IN UINT32 FvType - FV type
//
// Output:      TRUE  - There is no FvHob for FV_BB_BACKUP
//              FALSE - FV_BB_BACKUP shoub be published, block publishing FV_BB
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
BlockReportFvBB(
   IN EFI_PEI_SERVICES **PeiServices,
   IN OUT ROM_AREA *Area,
   IN UINT32 FvType
)
{
    EFI_STATUS              Status;
    EFI_HOB_FIRMWARE_VOLUME *FvHob = NULL;
    if( Area->Address != HASH_FV_BB_ADDRESS ) return TRUE;
    Status = (*PeiServices)->GetHobList(PeiServices, &FvHob);
    if( EFI_ERROR(Status) ) return TRUE;
    while(!EFI_ERROR(FindNextHobByType(EFI_HOB_TYPE_FV, &FvHob)))
    {
        if ( FvHob->BaseAddress == HASH_FV_BB_BACKUP_ADDRESS ) 
        {
            //SMDbgPrint("Already publish the FV_BB_BACKUP\n");
            return FALSE;
        }
    }
    return TRUE;
}
