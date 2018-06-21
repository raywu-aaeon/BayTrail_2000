//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************


/** @file AcpiCore.c
    Main ACPI Driver File. It has ACPI Driver entry point,
    ACPISupport Protocol and ACPITable Protocol.

**/
//**********************************************************************

#include <AmiDxeLib.h>
#include <AcpiRes.h>
#include "AcpiCore.h"
#include <AcpiOemElinks.h>


//--------------------------------------
//Some Global vars

ACPI_DB                 gAcpiData = {0, 0, NULL, 0};
RSDT_PTR_20             *gRsdtPtrStr = NULL;
UINT8                   gForceAcpi1 = 0, gPublishedOnReadyToBoot = 0;


typedef EFI_STATUS (ACPI_OEM_FUNCTION)(IN OUT ACPI_HDR *AcpiHdr);
extern ACPI_OEM_FUNCTION OEM_LIST EndOfOemList;
ACPI_OEM_FUNCTION* AcpiOemPartsList[] = {OEM_LIST NULL};

EFI_STATUS MpsTableBuilderInit(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable);

//------------Move to header------------------------
EFI_STATUS BuildMandatoryAcpiTbls ();
EFI_STATUS AcpiReadyToBootEvent();
EFI_STATUS UpdateFacp ();
EFI_STATUS PublishTbl (IN UINT8 RsdtBuild, IN UINT8 XsdtBuild);


//------------Move to header------------------------
/**
    This function provides platform specific OEM_ID and OEM_TABLE_ID to
    overwrite default ACPI Table header.

         
    @param AcpiHdr ACPI TABLE header

        @return EFI_STATUS  
		@retval EFI_SUCCESS if Values overwtitten.
        @retval EFI_INSUPPORTED if no need to change values - use default.

    @note  UINT8 *AcpiOemId[6]; UINT8 *AcpiOemTblId[8] avoid buffer overrun!
**/

EFI_STATUS OemAcpiSetPlatformId(IN OUT ACPI_HDR *AcpiHdr)
{
    EFI_STATUS       Status=EFI_UNSUPPORTED;
    UINTN i;

    for (i=0; AcpiOemPartsList[i]; i++) Status = AcpiOemPartsList[i](AcpiHdr);

    return Status;
}


/**
    Finds ACPI table by Handle and returns its entry number in
    gAcpiData structure

    @param Handle - Handle (pointer to ACPI table header)


    @retval Entry number in gAcpiData structure

**/
UINTN FindAcpiTblByHandle (UINTN *Handle)
{
    UINTN i;
    
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if (*Handle == (UINTN)gAcpiData.AcpiEntries[i]->BtHeaderPtr)   // Handle is the address of ACPI table
        {
            return i;
        }
    }
    
    return ACPI_TABLE_NOT_FOUND;
}// end of FindAcpiTblByHandle

/**
    Founds ACPI Table with defined Signature (Sig) and Version in gAcpiData structure.


    @param TblPtr pointer to the ACPI entrie in gAcpiData,
        modifided by this function
    @param Sig Signature of a table to search for
        IN EFI_ACPI_TABLE_VERSION - Version of a table to be found

    @retval EFI_SUCCESS table with corresponding signature was found
    @retval EFI_NOT_FOUND otherwise

**/
EFI_STATUS GetBtTable (OUT ACPI_TBL_ITEM **TblPtr, IN UINT32 Sig, IN EFI_ACPI_TABLE_VERSION Versiov)
{
    UINTN       i;
    
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if ((gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == Sig)
                && (gAcpiData.AcpiEntries[i]->AcpiTblVer & Versiov))
        {
            *TblPtr = gAcpiData.AcpiEntries[i];
            return EFI_SUCCESS;
        }
    }
    
    return EFI_NOT_FOUND;
}// end of GetBtTable

/**
    Checks if Notify function(s) present dispatches it.


    @param TableHeaderPtr - pointer to the ACPI Table header,
        modifided by this function
    @retval EFI_STATAUS From the function dispatched.

**/
EFI_STATUS DispatchSdtNotify(ACPI_TBL_ITEM* TableItemPtr)
{
    EFI_STATUS  Status=EFI_SUCCESS;
    UINTN   i;
//--------
    for(i=0;i<gAcpiData.NotifyFnCount; i++){
        Status=gAcpiData.AcpiNotifyFn[i]((EFI_ACPI_SDT_HEADER*)TableItemPtr->BtHeaderPtr,TableItemPtr->AcpiTblVer,(UINTN)TableItemPtr);        
        
        ASSERT_EFI_ERROR(Status);        
    }    

    return Status;
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ACPI SUPPORT PPROTOCOL function Implementation
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/**
    This function returns ACPI table

         
    @param This pointer to EFI_ACPI_SUPPORT_PROTOCOL instance
    @param Index Index of ACPI table to return
    @param Table Pointer where to place found table
    @param Version requested ACPI table version
    @param Handle requested ACPI table handle

          
    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_OUT_OF_RESOURCES not enough memory to allocate table
    @retval EFI_INVALID_PARAMETER invalid EFI_ACPI_SUPPORT_PROTOCOL pointer
    @retval EFI_NOT_FOUND requested ACPI table not found

**/

EFI_STATUS AcpiSupportGetAcpiTable (
    IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
    IN INTN                                 Index,
    OUT VOID                                **Table,
    OUT EFI_ACPI_TABLE_VERSION              *Version,
    OUT UINTN                               *Handle )
{
    ACPI_HDR        *HdrPtr;
    VOID            *Ptr;
    UINTN           i;
    
//----------------------

    if (This!=&gAcpiData.AcpiSupportProtocol) return EFI_INVALID_PARAMETER;
    
    if (Index > ((INTN)gAcpiData.AcpiEntCount - 1)) return EFI_NOT_FOUND;
    
    for (i = Index; i < (UINTN) gAcpiData.AcpiEntCount; i++)
    {
        if (gAcpiData.AcpiEntries[i]->AcpiTblVer < ACPI_TABLE_NOT_REMOVABLE)
        {
            // means this table was added by EFI_ACPI_TABLE_PROTOCOL.InstallAcpiTable
            // So it is illegal for EFI_ACPI_SUPPORT_PROTOCOL to receive a Handle for it
            HdrPtr = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
            Ptr = Malloc(HdrPtr->Length);
            ASSERT(Ptr);
            
            if (!Ptr) return EFI_OUT_OF_RESOURCES;
            
            *Version = gAcpiData.AcpiEntries[i]->AcpiTblVer;
            *Handle = (UINTN) HdrPtr;
            pBS->CopyMem(Ptr, HdrPtr, HdrPtr->Length);
            *Table = Ptr;
            return EFI_SUCCESS;
        }
    }
    
    return EFI_NOT_FOUND;
}//end of AcpiSupportGetAcpiTable

/**
    This function allows to add, remove of modify ACPI table

         
    @param This pointer to EFI_ACPI_SUPPORT_PROTOCOL instance
    @param Table Pointer to update data. If NULL, corresponded table
        should be removed
    @param Checksum if TRUE, function will recalculate checksum before adding table
    @param Version requested ACPI table version
    @param Handle requested ACPI table handle

          
    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_OUT_OF_RESOURCES not enough memory to perform operation
    @retval EFI_INVALID_PARAMETER invalid EFI_ACPI_SUPPORT_PROTOCOL pointer or ACPI table
        content
    @retval EFI_ABORTED provided ACPI table already present

**/

EFI_STATUS AcpiSupportSetAcpiTable(
    IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
    IN CONST VOID                           *Table    OPTIONAL,
    IN BOOLEAN                              Checksum,
    IN EFI_ACPI_TABLE_VERSION               Version,
    IN OUT UINTN                            *Handle  )
{
    ACPI_TBL_ITEM   *AcpiTableAdded = NULL, *TblDummy = NULL;
    ACPI_HDR        *Hdr1 = NULL, *Hdr2 = NULL, *Dsdt = NULL, *XtDsdt = NULL;
    VOID            *Ptr = NULL;
    EFI_ACPI_TABLE_VERSION XtDsdtVer = 0,  DsdtVer = 0, MultyVer = 0;
    EFI_STATUS      Status = 0;
    UINTN           TblNum = ACPI_TABLE_NOT_FOUND;
    BOOLEAN         CorrectFacp = FALSE, WasChecksummed = FALSE;

    
    //Handle == NULL   Table != NULL add the table
    //Handle != NULL   Table != NULL replace the table
    //Handle != NULL   Table == NULL remove the table
//-----------------------------------------------------------------------------------
    if ((This != &gAcpiData.AcpiSupportProtocol) || (Handle==NULL) || (*Handle == 0 && Table == NULL ))  //---------------------------------------------------------------
    {
        Status = EFI_INVALID_PARAMETER;
        ASSERT_EFI_ERROR(Status);
        return Status;
    }
    
    TRACE((-1, "ACPI: SetAcpiTable() Table=0x%X; Handle=0x%X; *Handle=0x%X\n", Table, Handle, *Handle));
    if (Table != NULL)
    {
        Hdr1 = (ACPI_HDR*) Table;
        
        //Check is we are getting a "special" table that needs a specific care
        if (Hdr1->Signature == FACS_SIG) 
        {
            return EFI_INVALID_PARAMETER;
        }
        if (Hdr1->Signature == FACP_SIG)
        {
            if (*Handle == 0)                                // --- Do not delete or add FACP - only replace it
            {
                Status = GetBtTable(&TblDummy, FACP_SIG, ( EFI_ACPI_TABLE_VERSION_X ));
                ASSERT_EFI_ERROR(Status);                  // --- Or if new version > 2 and old version of FACP = 2
                                                           // --- And action = add table - replace old one with the new one
                if (EFI_ERROR (Status)) 
                {
                    return EFI_ABORTED;
                }
                
                if (TblDummy->AcpiTblVer >= Version) 
                {
                    return EFI_INVALID_PARAMETER;
                }
                else
                {
                    *Handle = (UINTN)TblDummy->BtHeaderPtr;
                    CorrectFacp = TRUE;
                }
            }
        }
        
        if (Hdr1->Signature == DSDT_SIG)
        {
        
            //Just in case reset
            TblDummy=NULL;
            
            if (*Handle == 0)                                // --- Do not delete or add DSDT - only replace it
            {
                Status = GetBtTable(&TblDummy, DSDT_SIG, (EFI_ACPI_TABLE_VERSION_X));
                
                // --- Or if new version > 2 and old version of FACP = 2
                if (!EFI_ERROR (Status))            // --- And action = add table - replace old one with the new one
                {
                    XtDsdt = TblDummy->BtHeaderPtr;
                    XtDsdtVer = TblDummy->AcpiTblVer;
                }
                
                Status = GetBtTable(&TblDummy, DSDT_SIG, EFI_ACPI_TABLE_VERSION_1_0B);
                
                if (!EFI_ERROR (Status))
                {
                    Dsdt = TblDummy->BtHeaderPtr;
                    DsdtVer = TblDummy->AcpiTblVer;
                }
                
                if ((Version == EFI_ACPI_TABLE_VERSION_1_0B) && DsdtVer) 
                {
                    return EFI_INVALID_PARAMETER;
                }
                else
                {
                    if ((Version > DsdtVer) && (Version > XtDsdtVer))
                    {
                        if (XtDsdtVer)
                            *Handle = (UINTN) XtDsdt;
                    }
                    
                    else 
                    {
                        return EFI_INVALID_PARAMETER;
                    }
                }
            }
            
            CorrectFacp = TRUE;
        }
        
        if (Version == EFI_ACPI_TABLE_VERSION_NONE)
            Status = pBS->AllocatePool(EfiACPIMemoryNVS, Hdr1->Length, &Ptr);
        else
            Ptr = Malloc(Hdr1->Length);
            
        ASSERT(Ptr);
        
        if (Ptr==NULL) 
        {
            return EFI_OUT_OF_RESOURCES;
        }
        
        pBS->CopyMem(Ptr, Hdr1, Hdr1->Length);
        AcpiTableAdded = MallocZ (sizeof (ACPI_TBL_ITEM));
        ASSERT(AcpiTableAdded);
        
        if (!AcpiTableAdded) 
        {
            return EFI_OUT_OF_RESOURCES;
        }
        
        AcpiTableAdded->AcpiTblVer = Version;
        AcpiTableAdded->BtHeaderPtr = (ACPI_HDR*) Ptr;
        if (!Checksum)
            if (!ChsumTbl((UINT8*)Ptr, ((ACPI_HDR*) Ptr)->Length))
                WasChecksummed = TRUE;
        //If table was checksumed and Checksum parameter of SetTable function was not set 
        //to TRUE in next string OemAcpiSetPlatformId may modify the table - let's
        //remember was it checksumed or not

        if (EFI_ERROR(OemAcpiSetPlatformId ((ACPI_HDR*) Ptr))) WasChecksummed = FALSE;
        //If OemAcpiSetPlatformId did not modifies table - reset WasChecksummed to FALSE

        Status = DispatchSdtNotify(AcpiTableAdded);
        ASSERT_EFI_ERROR(Status);

        Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTableAdded);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) 
        {
            return EFI_OUT_OF_RESOURCES;
        }
        

        if (Version != EFI_ACPI_TABLE_VERSION_NONE)
            gAcpiData.AcpiLength += ((ACPI_HDR*) Ptr)->Length;
            
        if (Checksum || WasChecksummed)
        {
            ((ACPI_HDR*) Ptr)->Checksum = 0;
            ((ACPI_HDR*) Ptr)->Checksum = ChsumTbl((UINT8*)Ptr, ((ACPI_HDR*) Ptr)->Length);
        }
    }
    
    if (*Handle)
    {
        Status=EFI_SUCCESS;
        
        TRACE((-1, "ACPI: SetAcpiTable() Hnd!=0, Removing Tbl. Ver=0x%X, ",Version));
        
        Hdr2 = (ACPI_HDR*)(*Handle);
        
        //Check is we are getting a "special" table that needs a specific care
        if (Hdr2->Signature == FACS_SIG) 
        {
            return EFI_INVALID_PARAMETER;
        }
        
        if ((Hdr2->Signature == FACP_SIG) && (!Table)) 
        {
            return EFI_INVALID_PARAMETER;
        }
        
        if ((Hdr2->Signature == DSDT_SIG) && (!Table)) 
        {
            return EFI_INVALID_PARAMETER;
        }
        
        TblNum = FindAcpiTblByHandle (Handle);
        TRACE((-1, "TblNum=0x%X ", TblNum));
        
        if (TblNum == ACPI_TABLE_NOT_FOUND) Status=EFI_INVALID_PARAMETER;
        
        TRACE((-1,"Status = %r\n", Status));
        
        // Table with this Handle does not exist
        if (EFI_ERROR(Status))
        {
            return Status;
        }
        
        //if sombody is trying to replace or delete table with version
        //which is a combination of bits (for example V1 and 2 or V2 and 3, etc)
        if ((Version != gAcpiData.AcpiEntries[TblNum]->AcpiTblVer) && (Version != ACPI_TABLE_NOT_REMOVABLE))
        {
            MultyVer = gAcpiData.AcpiEntries[TblNum]->AcpiTblVer;
            
            if ((MultyVer == EFI_ACPI_TABLE_VERSION_1_0B) ||
                    (MultyVer == EFI_ACPI_TABLE_VERSION_2_0) ||
                    (MultyVer == EFI_ACPI_TABLE_VERSION_3_0) || (MultyVer == EFI_ACPI_TABLE_VERSION_4_0))
            {
                return EFI_INVALID_PARAMETER;
            }
            
            else MultyVer ^= Version;
        }
        
        if (Version != EFI_ACPI_TABLE_VERSION_NONE)
            gAcpiData.AcpiLength -= gAcpiData.AcpiEntries[TblNum]->BtHeaderPtr->Length;
            
        Status = DeleteItemLst((T_ITEM_LIST*) &gAcpiData, TblNum, TRUE);
        TRACE((-1,"ACPI:  Deleting Table From Storage: Status = %r\n", Status));
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR (Status)) 
        {
            return EFI_OUT_OF_RESOURCES;
        }
        
        if (MultyVer)
        {
            AcpiTableAdded = MallocZ (sizeof (ACPI_TBL_ITEM));
            ASSERT(AcpiTableAdded);
            
            if (!AcpiTableAdded) 
            {
                return EFI_OUT_OF_RESOURCES;
            }
            
            AcpiTableAdded->AcpiTblVer = MultyVer;
            
            AcpiTableAdded->BtHeaderPtr = Hdr2;
            OemAcpiSetPlatformId(Hdr2);
            Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTableAdded);
            ASSERT_EFI_ERROR(Status);

            Status = DispatchSdtNotify(AcpiTableAdded);
            ASSERT_EFI_ERROR(Status);

            if (EFI_ERROR(Status)) 
            {
                return EFI_OUT_OF_RESOURCES;
            }
            
            gAcpiData.AcpiLength += gAcpiData.AcpiEntries[TblNum]->BtHeaderPtr->Length;
        }
        
        else pBS->FreePool(Hdr2);
    }
    
    //Update Handle with New Table Instance.
    *Handle=(UINTN)Ptr;
    
    if (CorrectFacp)
    {
        Status = UpdateFacp ();
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) 
        {
            return EFI_INVALID_PARAMETER;
        }
    }
    
    TRACE((-1,"ACPI: SetAcpiTable() Exiting... Status = %r\n", Status));

    if (gPublishedOnReadyToBoot)
    {
        if (!gForceAcpi1) Status = PublishTbl (1, 1);
        else Status = PublishTbl (1, 0);
    }
    TRACE((-1,"ACPI: PublishTables in SetAcpiTable() Status = %r\n", Status));
    ASSERT_EFI_ERROR(Status);

    return Status;
//--- !!!!!!!!!!!!!!!!!!!!!!!!!! Version none Done ???? !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}// end of AcpiSupportSetAcpiTable


/**
    Causes one or more versions of the ACPI tables to be published in
    the EFI system configuration tables.

         
    @param This pointer to EFI_ACPI_SUPPORT_PROTOCOL instance
    @param Version ACPI table version

          
    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_ABORTED invalid EFI_ACPI_SUPPORT_PROTOCOL pointer or
        an error occurred and the function could not complete successfully.
    @retval EFI_UNSUPPORTED passed ACPI table version invalid

**/

EFI_STATUS AcpiSupportPublishTables(
    IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
    IN EFI_ACPI_TABLE_VERSION               Version )
{
    EFI_STATUS Status;
    
    if ((Version < EFI_ACPI_TABLE_VERSION_1_0B) || (Version > EFI_ACPI_TABLE_VERSION_ALL) || (This!=&gAcpiData.AcpiSupportProtocol))
        return EFI_UNSUPPORTED;
    if (Version == EFI_ACPI_TABLE_VERSION_1_0B)
    {
        if ((gRsdtPtrStr != NULL) && (gRsdtPtrStr->XsdtAddr != 0)) Status = PublishTbl (1, 1);
        else Status = PublishTbl (1, 0);
    }
    
    if ((Version > EFI_ACPI_TABLE_VERSION_1_0B) && (!gForceAcpi1))
    {
        if ((gRsdtPtrStr != NULL) && (gRsdtPtrStr->RsdtAddr != 0)) Status = PublishTbl (1, 1);
        else Status = PublishTbl (0, 1);
    }
    ASSERT_EFI_ERROR (Status);
    
    if (EFI_ERROR (Status)) return EFI_ABORTED;
    else return EFI_SUCCESS;
}
/**
    Installs an ACPI table into the RSDT/XSDT.

         
    @param This A pointer to a EFI_ACPI_TABLE_PROTOCOL.
    @param AcpiTableBuffer A pointer to a buffer containing the ACPI table to be installed.
    @param AcpiTableBufferSize Specifies the size, in bytes, of the AcpiTableBuffer buffer.
    @param TableKey Returns a key to refer to the ACPI table.

          
    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_INVALID_PARAMETER Either AcpiTableBuffer is NULL, TableKey is NULL, or
        AcpiTableBufferSize and the size field embedded in the ACPI
        table pointed to by AcpiTableBuffer are not in sync
    @retval EFI_OUT_OF_RESOURCES Insufficient resources exist to complete the request.

**/
EFI_STATUS AcpiInstallAcpiTable(
    IN CONST EFI_ACPI_TABLE_PROTOCOL *This,
    IN CONST VOID *AcpiTableBuffer,
    IN UINTN AcpiTableBufferSize,
    OUT UINTN *TableKey)
{
    EFI_STATUS Status;
    UINTN IntTableKey = 0;
    
    
    if ((AcpiTableBuffer == NULL) || (AcpiTableBufferSize != (UINTN)((ACPI_HDR*)AcpiTableBuffer)->Length)
            || (TableKey == NULL)) return EFI_INVALID_PARAMETER;
            
    Status = AcpiSupportSetAcpiTable(&gAcpiData.AcpiSupportProtocol,
                                     (VOID*)AcpiTableBuffer,
                                     TRUE,
                                     // This is the mark, that means, that this table was added by the new protocol
                                     ACPI_TABLE_NOT_REMOVABLE,
                                     &IntTableKey);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    *TableKey = IntTableKey;
    
    if (!gPublishedOnReadyToBoot)
    {
    	if (!gForceAcpi1) Status = PublishTbl (1, 1);
    	else PublishTbl (1, 0);
    }
    return Status;
}// end of AcpiInstallAcpiTable


/**
    Function allows a caller to remove an ACPI table.

         
    @param This A pointer to a EFI_ACPI_TABLE_PROTOCOL.
    @param TableKey Specifies the table to uninstall. The key was returned from
        InstallAcpiTable().

          
    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_NOT_FOUND TableKey does not refer to a valid key for a table entry.
    @retval EFI_OUT_OF_RESOURCES Insufficient resources exist to complete the request.

**/

EFI_STATUS AcpiUninstallAcpiTable(
    IN CONST EFI_ACPI_TABLE_PROTOCOL *This,
    IN UINTN TableKey)
{
    EFI_STATUS Status;
    
    Status = AcpiSupportSetAcpiTable(&gAcpiData.AcpiSupportProtocol,
                                     NULL,
                                     FALSE,
                                     // This is the mark, that means, that this table was added by the new protocol
                                     ACPI_TABLE_NOT_REMOVABLE,
                                     &TableKey);
    if (EFI_ERROR(Status))
        return (EFI_NOT_FOUND);
    else
        return Status;
}//end of AcpiUninstallAcpiTable


//
// ACPI SDT Protocol functions implementation. 
//

/**
    Returns a requested ACPI table.
    The GetAcpiTable() function returns a pointer to a buffer containing the ACPI table associated
    with the Index that was input. The following structures are not considered elements in the list of
    ACPI tables:
    - Root System Description Pointer (RSD_PTR)
    - Root System Description Table (RSDT)
    - Extended System Description Table (XSDT)
    Version is updated with a bit map containing all the versions of ACPI of which the table is a
    member.
  
    
    @param Index The zero-based index of the table to retrieve.
    @param Table Pointer for returning the table buffer.
    @paramVersion     On return, updated with the ACPI versions to which this table belongs. Type
        EFI_ACPI_TABLE_VERSION is defined in "Related Definitions" in the
        EFI_ACPI_SDT_PROTOCOL.    
    @param TableKey    On return, points to the table key for the specified ACPI system definition table. This
        is identical to the table key used in the EFI_ACPI_TABLE_PROTOCOL.  
     
    @retval EFI_SUCCESS       The function completed successfully.
    @retval EFI_NOT_FOUND     The requested index is too large and a table was not found.                                  
**/
EFI_STATUS GetAcpiTable2 (
  IN  UINTN                               Index,
  OUT EFI_ACPI_SDT_HEADER                 **Table,
  OUT EFI_ACPI_TABLE_VERSION              *Version,
  OUT UINTN                               *TableKey)
{

    return AcpiSupportGetAcpiTable (&gAcpiData.AcpiSupportProtocol, Index,
                                (VOID **)Table, Version,TableKey);

}

EFI_STATUS RegisterNotify(
  IN BOOLEAN                    Register,
  IN EFI_ACPI_NOTIFICATION_FN   Notification)
{
    if(Notification == NULL) return EFI_INVALID_PARAMETER;

    if(Register){
        return AppendItemLst((T_ITEM_LIST*)&gAcpiData.NotifyInitCount, Notification);
    } else {
        UINTN   i;
    //-----------------
        for(i=0; i<gAcpiData.NotifyFnCount; i++){
            if(gAcpiData.AcpiNotifyFn[i]==Notification){
                return DeleteItemLst((T_ITEM_LIST*)&gAcpiData.NotifyInitCount,i,FALSE);
            }
        }  
        //can't found matching notify function.
        return EFI_INVALID_PARAMETER; 
    }
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Driver entry point
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/**
    This function is ACPI driver entry point

         
    @param ImageHandle Image handle
    @param SystemTable pointer to system table

          
    @retval EFI_SUCCESS Function executed successfully, ACPI_SUPPORT_PROTOCOL installed
    @retval EFI_ABORTED Dsdt table not found or table publishing failed
    @retval EFI_ALREADY_STARTED driver already started
    @retval EFI_OUT_OF_RESOURCES not enough memory to perform operation

    @note  
  This function also creates ReadyToBoot event to update AML objects before booting

**/

EFI_STATUS AcpiNewCoreEntry (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS      Status = EFI_SUCCESS;
    static EFI_GUID Acpisupguid = EFI_ACPI_SUPPORT_GUID;
    VOID            *DummyPtr;


//------------------------
    InitAmiLib(ImageHandle,SystemTable);
    PROGRESS_CODE(DXE_ACPI_INIT);
    //it must be ony one instance of this protocol
    Status = pBS->LocateProtocol(&Acpisupguid,NULL,&DummyPtr);
    TRACE((-1,"IN ACPI Start: %x\n", Status));
    
    if (!EFI_ERROR(Status)) 
    	return EFI_ALREADY_STARTED;

    Status = BuildMandatoryAcpiTbls (); 
    //Call Architecture Specific Library function to build a set of mandatory tables
    if (EFI_ERROR(Status)) 
    	return Status;
    
#if MPS_TABLE_SUPPORT == 1
    Status = MpsTableBuilderInit(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR(Status);
#endif
    
    Status = AcpiReadyToBootEvent();
    //Call Architecture Specific Library function to craeate Ready to Boot event, if necessary 
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) 
    	return Status;
    
    
    gAcpiData.AcpiSupportProtocol.GetAcpiTable = AcpiSupportGetAcpiTable;
    gAcpiData.AcpiSupportProtocol.SetAcpiTable = AcpiSupportSetAcpiTable;
    gAcpiData.AcpiSupportProtocol.PublishTables = AcpiSupportPublishTables;
    
    gAcpiData.AcpiTableProtocol.InstallAcpiTable = AcpiInstallAcpiTable;
    gAcpiData.AcpiTableProtocol.UninstallAcpiTable = AcpiUninstallAcpiTable;

    gAcpiData.AcpiSdtProtocol.GetAcpiTable=GetAcpiTable2;
    gAcpiData.AcpiSdtProtocol.RegisterNotify=RegisterNotify;
    gAcpiData.AcpiSdtProtocol.Open=Open;
    gAcpiData.AcpiSdtProtocol.OpenSdt=OpenSdt;
    gAcpiData.AcpiSdtProtocol.Close=Close;
    gAcpiData.AcpiSdtProtocol.GetChild=GetChild;
    gAcpiData.AcpiSdtProtocol.GetOption=GetOption;
    gAcpiData.AcpiSdtProtocol.SetOption=SetOption;
    gAcpiData.AcpiSdtProtocol.FindPath=FindPath;
    gAcpiData.AcpiSdtProtocol.AcpiVersion=EFI_ACPI_TABLE_VERSION_ALL;

    gAcpiData.AcpiSupportHandle = NULL;
    //Instasll ProtocolInterface;
    Status=pBS->InstallMultipleProtocolInterfaces(
               &gAcpiData.AcpiSupportHandle,
               &Acpisupguid,
               &gAcpiData.AcpiSupportProtocol,
               &gEfiAcpiTableProtocolGuid,
               &gAcpiData.AcpiTableProtocol,
               &gEfiAcpiSdtProtocolGuid,
               &gAcpiData.AcpiSdtProtocol, 
               NULL);
    ASSERT_EFI_ERROR(Status);
    
    return Status;
    
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
