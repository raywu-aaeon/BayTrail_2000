//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************


/** @file ARM/AcpiBaseLib.c
    Main ACPI Driver File. It has ACPI Driver entry point,
    ACPISupport Protocol and ACPITable Protocol.

**/
//**********************************************************************

#include <AmiDxeLib.h>
#include <Token.h>
#include <AcpiRes.h>
#include "AcpiCore.h"
#include "Acpi50.h"
#include <Protocol/Cpu.h>
#include <Library/AmiSdlLib.h>
#include <AcpiOemElinks.h>
#include <Fid.h>
#include <Library/DebugLib.h>
#include <Guid/ArmMpCoreInfo.h>
#if ATAD_SUPPORT == 1
#include "AtadSmi.h"
#endif

//--------------------------------------
//Some Global vars
extern ACPI_DB          gAcpiData;
FACS_20                 *gxFacs;
extern RSDT_PTR_20      *gRsdtPtrStr;
UINTN                   gAcpiTblPages = 0;
EFI_GUID                gAcpi20TAbleGuid = EFI_ACPI_20_TABLE_GUID;
extern UINT8            gPublishedOnReadyToBoot;
#if ATAD_SUPPORT == 1
    VOID  *AtadBuffPtr = NULL;
#endif


UINT8    ACPI_OEM_ID[6]     = ACPI_OEM_ID_MAK;     
UINT8    ACPI_OEM_TBL_ID[8] = ACPI_OEM_TBL_ID_MAK; 


EFI_STATUS OemAcpiSetPlatformId(IN OUT ACPI_HDR *AcpiHdr);


/**
    This function creates ACPI table v 2.0+ header with specified signature

         
    @param TblSig ACPI table signature
    @param HdrPtr Pointer to memory, where the header should be placed

          
    @retval VOID

**/

void PrepareHdr20(UINT32 TblSig, ACPI_HDR *HeaderPtr, UINTN Vers)
{
    UINTN i;
    EFI_STATUS  Status;
    
    if (HeaderPtr==NULL) return;
    
    HeaderPtr->Signature=TblSig;
    
    //Check what Revision# header needs depends on TABLE we're building
    switch (TblSig)
    {
        case RSDT_SIG:
            HeaderPtr->Revision=ACPI_REV1;
            HeaderPtr->CreatorRev=CREATOR_REV_MS+1;
            break;
            
        case XSDT_SIG:
            HeaderPtr->Revision=ACPI_REV1;
            HeaderPtr->CreatorRev=CREATOR_REV_MS+2;
            break;
            
        case FACP_SIG:
            HeaderPtr->Revision=ACPI_REV3;
            HeaderPtr->CreatorRev=CREATOR_REV_MS+3;
            
            if (Vers > 2)
            {
            
                HeaderPtr->Revision=ACPI_REV4;
                HeaderPtr->CreatorRev++;
            }
            
            break;
        case APIC_SIG:
            HeaderPtr->Revision=ACPI_REV1;
            HeaderPtr->CreatorRev=CREATOR_REV_MS+4;
            
            if (Vers > 2)
            {
            
                HeaderPtr->Revision=ACPI_REV2;
                HeaderPtr->CreatorRev++;
            }
            
            if (Vers > 3)HeaderPtr->Revision=ACPI_REV3;
            
            break;
        case SBST_SIG:
            HeaderPtr->Revision=ACPI_REV1;
            break;
        case SPCR_SIG:
            HeaderPtr->Revision=ACPI_REV1;
            break;
        case ECDT_SIG:
            HeaderPtr->Revision=ACPI_REV1;
            break;
            //case DSDT_SIG:
            //HeaderPtr->CreatorRev=CREATOR_REV_MS+5;
            //HeaderPtr->Revision=ACPI_REV2;
            //break;
    }
    
    //instead of puting fixed revision number
    //HeaderPtr->Revision=ACPI_REV2;
    
    //Dont' touch Creator ID and Creator Revision;
    if (TblSig != DSDT_SIG)
    {
        if (TblSig == RSDT_SIG)
            HeaderPtr->CreatorId = CREATOR_ID_MS;
        else
            HeaderPtr->CreatorId = CREATOR_ID_AMI;
            
        HeaderPtr->CreatorRev = CREATOR_REV_MS;
        HeaderPtr->OemRev = ACPI_OEM_REV;
    }

    //Get the platform specific OEM_IDs
    Status=OemAcpiSetPlatformId(HeaderPtr);
    
    //if platform does not support OEM_ID overwrite
    if (EFI_ERROR(Status))
    {
        for (i=0; i<6; i++) HeaderPtr->OemId[i]=ACPI_OEM_ID[i];
        
        for (i=0; i<8; i++) HeaderPtr->OemTblId[i]=ACPI_OEM_TBL_ID[i];
    }

}  //PrepareHdr20

/**
    This function allocates memory for and fills FIDT struscure. 

    @param 
        IN OUT TablPtr Pointer to memory, where the FACP table will resides.
        Filled by this procedure
    @retval 
  EFI_OUT_OF_RESOURCES - Memory for the table could not be allocated
  EFI_SUCCESS - Table was successfully build

**/

EFI_STATUS BuildFIDT (IN OUT ACPI_HDR **TablPtr)

{

	FW_VERSION		*FidPtr = NULL;
    UINTN			FidLen = 0;
    UINT8			i;
    ACPI_HDR		*FidTblHdr;
    EFI_GUID    	FwGuid = FW_VERSION_GUID;
    CHAR8 CoreMajVer [3] = THREE_CHAR_ARRAY(CORE_MAJOR_VERSION);
    CHAR8 CoreMinVer [3] = THREE_CHAR_ARRAY(CORE_MINOR_VERSION);
    CHAR8 ProjMajVer [3] = THREE_CHAR_ARRAY(PROJECT_MAJOR_VERSION);
    CHAR8 ProjMinVer [3] = THREE_CHAR_ARRAY(PROJECT_MINOR_VERSION);
 
    FidLen = sizeof(ACPI_HDR) + sizeof(FW_VERSION);
    *TablPtr = MallocZ (FidLen);
    if ((*TablPtr)==NULL)
    {
    	ASSERT(*TablPtr);
    	return EFI_OUT_OF_RESOURCES;
    }
    FidTblHdr = (ACPI_HDR*)*TablPtr;
    PrepareHdr20(FIDT_SIG, FidTblHdr, 4);
    FidTblHdr->Length = (UINT32)FidLen;
    FidTblHdr->Revision = 1;
    
    FidPtr = (FW_VERSION*) ((UINT8*)FidTblHdr + sizeof(ACPI_HDR));
    
    Strcpy (FidPtr->FirmwareID, CONVERT_TO_STRING($FID));
    FidPtr->StructVersion = 4;
    FidPtr->Size = sizeof(FW_VERSION);
    Strcpy (FidPtr->BiosTag, CONVERT_TO_STRING(BIOS_TAG));
    FidPtr->FirmwareGuid = FwGuid;
    MemCpy(FidPtr->CoreMajorVersion, CoreMajVer, 3);
    MemCpy(FidPtr->CoreMinorVersion, CoreMinVer, 3);
    MemCpy(FidPtr->ProjectMajorVersion, ProjMajVer, 3);
    MemCpy(FidPtr->ProjectMinorVersion, ProjMinVer, 3);

    FidPtr->SignOnStringId = 0xffff;
    
    for (i=0; i<6; i++) FidPtr->OemId[i] = ACPI_OEM_ID[i];
    
    for (i=0; i<8; i++) FidPtr->OemTableId[i] = ACPI_OEM_TBL_ID[i];
    
    FidTblHdr->Checksum = ChsumTbl((UINT8*)FidTblHdr, FidTblHdr->Length);

    return EFI_SUCCESS;
}
/**
    This function allocates memory for and fills FACP table v 1.1+ with
    predefined values from SDL tokens
         
    @param TablVer version of FACP table
    @param TablPtr pointer to memory, where the FACP table will resides.
        	Filled by this procedure
    @retval 
  EFI_OUT_OF_RESOURCES - Memory for the table could not be allocated
  EFI_SUCCESS - Table was successfully build

**/


EFI_STATUS BuildFacpAll (IN UINTN TablVer, OUT ACPI_HDR **TablPtr)

{
    FACP_50     *Facp;
    ACPI_HDR    *FACP_Hdr;
    UINT32       SizeOfFacp = sizeof(FACP_20);
//-----------------------------

    if (TablVer<1 || TablVer>4) return EFI_INVALID_PARAMETER;
    
    if (TablVer == 1) SizeOfFacp = 0x84;// size of compatability 1.1 structure
    if (ACPI_BUILD_TABLES_5_0 == 1) SizeOfFacp = sizeof(FACP_50);
    *TablPtr = MallocZ (SizeOfFacp);
    if ((*TablPtr)==NULL)
    {
        ASSERT(*TablPtr);
        return EFI_OUT_OF_RESOURCES;
    }
    
    FACP_Hdr = *TablPtr;
    
    if (TablVer == 1)
    {
        PrepareHdr20(FACP_SIG, FACP_Hdr, 2);
        FACP_Hdr->Revision = ACPI_REV2;// compatability 1.1 structure
    }
    
    else
        PrepareHdr20(FACP_SIG, FACP_Hdr, TablVer);
        
    Facp = (PFACP_50) *TablPtr;
    
    Facp->Reserved1     = ACPI_INT_MODEL;
    
    Facp->PM_PPROF      = ACPI_PM_PROFILE;
    
    Facp->FLAGS = 0;
    
#if HW_REDUCED_ACPI == 0
    Facp->SCI_INT       = ACPI_SCI_INT;
    Facp->SMI_CMD       = SW_SMI_IO_ADDRESS;
    Facp->ACPI_ENABLE_CMD   = SW_SMI_ACPI_ENABLE;

    Facp->ACPI_DISABLE_CMD  = SW_SMI_ACPI_DISABLE;

    Facp->S4BIOS_REQ    = SW_SMI_S4BIOS; 
    
    Facp->PSTATE_CNT    = SW_SMI_PSTATE_CNT;
    
    if (PM1A_EVT_BLK_ADDRESS > 0xffffffff)
        Facp->PM1a_EVT_BLK  = 0;
    else
        Facp->PM1a_EVT_BLK  = PM1A_EVT_BLK_ADDRESS;
        
    if (PM1B_EVT_BLK_ADDRESS > 0xffffffff)
        Facp->PM1b_EVT_BLK  = 0;
    else
        Facp->PM1b_EVT_BLK  = PM1B_EVT_BLK_ADDRESS;
        
    if (PM1A_CNT_BLK_ADDRESS > 0xffffffff) 
        Facp->PM1a_CNT_BLK  = 0;
    else
        Facp->PM1a_CNT_BLK  = PM1A_CNT_BLK_ADDRESS;
        
    if (PM1B_CNT_BLK_ADDRESS > 0xffffffff)
        Facp->PM1b_CNT_BLK  = 0;
    else
        Facp->PM1b_CNT_BLK  = PM1B_CNT_BLK_ADDRESS;
        
    if (PM2_CNT_BLK_ADDRESS > 0xffffffff)
        Facp->PM2_CNT_BLK   = 0;
    else
        Facp->PM2_CNT_BLK   = PM2_CNT_BLK_ADDRESS;
        
    if (PM_TMR_BLK_ADDRESS > 0xffffffff)
        Facp->PM_TMR_BLK    = 0;
    else
        Facp->PM_TMR_BLK    = PM_TMR_BLK_ADDRESS;
        
    if (GPE0_BLK_ADDRESS > 0xffffffff)
        Facp->GPE0_BLK  = 0;
    else
        Facp->GPE0_BLK  = GPE0_BLK_ADDRESS;
        
    if (GPE1_BLK_ADDRESS > 0xffffffff) 
        Facp->GPE1_BLK  = 0;
    else
        Facp->GPE1_BLK  = GPE1_BLK_ADDRESS;
        
    Facp->GPE0_BLK_LEN          = GPE0_BLK_LENGTH; 
    Facp->GPE1_BLK_LEN          = GPE1_BLK_LENGTH; 
    Facp->GPE1_BASE             = GPE1_BASE_OFFSET; 
    Facp->PM1_EVT_LEN   = PM1_EVT_LENGTH;
    Facp->PM1_CNT_LEN   = PM1_CNT_LENGTH;
    Facp->PM2_CNT_LEN   = PM2_CNT_LENGTH;
    Facp->PM_TM_LEN     = PM_TMR_LENGTH;
    
    Facp->CST_CNT   = SW_SMI_CST_CNT;
    Facp->P_LVL2_LAT    = P_LVL2_LAT_VAL;
    Facp->P_LVL3_LAT    = P_LVL3_LAT_VAL;
    Facp->FLUSH_SIZE    = FLUSH_SIZE_VAL;
    Facp->FLUSH_STRIDE  = FLUSH_STRIDE_VAL;
    Facp->DUTY_OFFSET   = DUTY_OFFSET_VAL;
    Facp->DUTY_WIDTH    = DUTY_WIDTH_VAL; 
    Facp->DAY_ALRM      = ACPI_ALARM_DAY_CMOS;
    Facp->MON_ALRM      = ACPI_ALARM_MONTH_CMOS;
    Facp->CENTURY       = ACPI_CENTURY_CMOS;
    Facp->IAPC_BOOT_ARCH = ACPI_IA_BOOT_ARCH;
    
    //--------Filling Flags for V.1----------------------
    
    if (FACP_FLAG_WBINVD_FLUSH) Facp->FLAGS |= 1<<1;
    
    if (FACP_FLAG_PROC_C1)      Facp->FLAGS |= 1<<2;
    
    if (FACP_FLAG_P_LVL2_UP)    Facp->FLAGS |= 1<<3;
    
    if (FACP_FLAG_RTC_S4)       Facp->FLAGS |= 1<<7;
    
    if (FACP_FLAG_TMR_VAL_EXT)  Facp->FLAGS |= 1<<8;
    
    if (FACP_FLAG_HEADLESS)         Facp->FLAGS |= 1<<12;
    
    if (FACP_FLAG_CPU_SW_SLP)       Facp->FLAGS |= 1<<13;
#if ACPI_BUILD_TABLES_3_0    
    
    if (FACP_FLAG_S4_RTC_STS_VALID)             Facp->FLAGS |= 1<<16;
    
    if (FACP_FLAG_REMOTE_POWER_ON_CAPABLE)  	Facp->FLAGS |= 1<<17;

    if (FACP_FLAG_PCI_EXP_WAK)      			Facp->FLAGS |= 1<<14;
    
#endif //#if ACPI_BUILD_TABLES_3_0 
    
#endif //#if HW_REDUCED_ACPI == 0
    
    if (FACP_FLAG_WBINVD)       Facp->FLAGS |= 1;
    
    if (FACP_FLAG_PWR_BUTTON)   Facp->FLAGS |= 1<<4;
    
    if (FACP_FLAG_SLP_BUTTON)   Facp->FLAGS |= 1<<5;
    
    if (FACP_FLAG_FIX_RTC)      Facp->FLAGS |= 1<<6;
    
    if (FACP_FLAG_DCK_CAP)      Facp->FLAGS |= 1<<9;
    
    
    //--------Filling Flags for V.2 and GAS compat structure for v.1----------------------
    
    if (FACP_FLAG_RESET_REG_SUP)    Facp->FLAGS |= 1<<10;
    
    if (FACP_FLAG_SEALED_CASE)      Facp->FLAGS |= 1<<11;
    

    
#if ACPI_BUILD_TABLES_3_0
    
    if (FACP_FLAG_USE_PLATFORM_CLOCK)                   Facp->FLAGS |= 1<<15;
    
    if (TablVer > 2)
    {
        if (FACP_FLAG_FORCE_APIC_CLUSTER_MODEL)             Facp->FLAGS |= 1<<18;
        
        if (FACP_FLAG_FORCE_APIC_PHYSICAL_DESTINATION_MODE) Facp->FLAGS |= 1<<19;
        
    }
    
#endif
    
    // RESET_REG GAS_20 structure and value
    Facp->RESET_REG.AddrSpcID   = ACPI_RESET_REG_TYPE;
    Facp->RESET_REG.RegBitWidth = ACPI_RESET_REG_BITWIDTH;
    Facp->RESET_REG.RegBitOffs  = ACPI_RESET_REG_BITOFFSET;
    Facp->RESET_REG.Address     = ACPI_RESET_REG_ADDRESS;
    Facp->RESET_VAL             = ACPI_RESET_REG_VALUE;
    
    if (ACPI_RESET_REG_ADDRESS)
    {
        // Set FACP flag
        Facp->FLAGS |= 1<<10;
    }
    
    if (TablVer == 1)
    {
        Facp->Header.Length     = 0x84;
        Facp->Header.Checksum = 0;
        Facp->Header.Checksum = ChsumTbl((UINT8*)Facp, Facp->Header.Length);
        return EFI_SUCCESS;
    }
    
    //--------This is all for V.1-----------------------
#if HW_REDUCED_ACPI == 0    
    // PM1a_EVT_BLK GAS_20 structure
    Facp->X_PM1a_EVT_BLK.AddrSpcID  = PM1A_EVT_BLK_TYPE;
    Facp->X_PM1a_EVT_BLK.RegBitWidth= PM1A_EVT_BLK_BITWIDTH;
    Facp->X_PM1a_EVT_BLK.RegBitOffs = PM1A_EVT_BLK_BITOFFSET;
    Facp->X_PM1a_EVT_BLK.AccessSize  = 2;
    Facp->X_PM1a_EVT_BLK.Address    = PM1A_EVT_BLK_ADDRESS;
    
    // PM1a_CNT_BLK GAS_20 structure
    Facp->X_PM1a_CNT_BLK.AddrSpcID  = PM1A_CNT_BLK_TYPE;
    Facp->X_PM1a_CNT_BLK.RegBitWidth= PM1A_CNT_BLK_BITWIDTH;
    Facp->X_PM1a_CNT_BLK.RegBitOffs = PM1A_CNT_BLK_BITOFFSET;
    Facp->X_PM1a_CNT_BLK.AccessSize  = 2;
    Facp->X_PM1a_CNT_BLK.Address    = PM1A_CNT_BLK_ADDRESS;
    
    // PM1b_EVT_BLK GAS_20 structure
    Facp->X_PM1b_EVT_BLK.AddrSpcID  = PM1B_EVT_BLK_TYPE;
    Facp->X_PM1b_EVT_BLK.RegBitWidth= PM1B_EVT_BLK_BITWIDTH;
    Facp->X_PM1b_EVT_BLK.RegBitOffs = PM1B_EVT_BLK_BITOFFSET;
    Facp->X_PM1b_EVT_BLK.AccessSize  = 2;
    Facp->X_PM1b_EVT_BLK.Address    = PM1B_EVT_BLK_ADDRESS;
    
    // PM1b_CNT_BLK GAS_20 structure
    Facp->X_PM1b_CNT_BLK.AddrSpcID  = PM1B_CNT_BLK_TYPE;
    Facp->X_PM1b_CNT_BLK.RegBitWidth= PM1B_CNT_BLK_BITWIDTH;
    Facp->X_PM1b_CNT_BLK.RegBitOffs = PM1B_CNT_BLK_BITOFFSET;
    Facp->X_PM1b_CNT_BLK.AccessSize    = 2;
    Facp->X_PM1b_CNT_BLK.Address    = PM1B_CNT_BLK_ADDRESS;
    
    // PM1b_CNT_BLK GAS_20 structure
    Facp->X_PM2_CNT_BLK.AddrSpcID   = PM2_CNT_BLK_TYPE;
    Facp->X_PM2_CNT_BLK.RegBitWidth = PM2_CNT_BLK_BITWIDTH;
    Facp->X_PM2_CNT_BLK.RegBitOffs  = PM2_CNT_BLK_BITOFFSET;
    Facp->X_PM2_CNT_BLK.AccessSize    = 1;
    Facp->X_PM2_CNT_BLK.Address     = PM2_CNT_BLK_ADDRESS;
    
    Facp->X_PM_TMR_BLK.AddrSpcID    = PM_TMR_BLK_TYPE;
    Facp->X_PM_TMR_BLK.RegBitWidth  = PM_TMR_BLK_BITWIDTH;
    Facp->X_PM_TMR_BLK.RegBitOffs   = PM_TMR_BLK_BITOFFSET;
    Facp->X_PM_TMR_BLK.AccessSize    = 3;
    Facp->X_PM_TMR_BLK.Address      = PM_TMR_BLK_ADDRESS;
    
    Facp->X_GPE0_BLK.AddrSpcID      = GPE0_BLK_TYPE;
    Facp->X_GPE0_BLK.RegBitWidth    = GPE0_BLK_BITWIDTH;
    Facp->X_GPE0_BLK.RegBitOffs     = GPE0_BLK_BITOFFSET;
    Facp->X_GPE0_BLK.AccessSize    = 1;
    Facp->X_GPE0_BLK.Address        = GPE0_BLK_ADDRESS;
    
    Facp->X_GPE1_BLK.AddrSpcID      = GPE1_BLK_TYPE;
    Facp->X_GPE1_BLK.RegBitWidth    = GPE1_BLK_BITWIDTH;
    Facp->X_GPE1_BLK.RegBitOffs     = GPE1_BLK_BITOFFSET;
    Facp->X_GPE1_BLK.AccessSize    = 1;
    Facp->X_GPE1_BLK.Address        = GPE1_BLK_ADDRESS;
#endif //#if HW_REDUCED_ACPI == 0   
    if (ACPI_BUILD_TABLES_5_0 == 1) 
    {
#if HW_REDUCED_ACPI      

        Facp->FLAGS |= 1<<20;

        Facp->SLEEP_CONTROL_REG.AddrSpcID   = SLEEP_CONTROL_REG_TYPE;
        Facp->SLEEP_CONTROL_REG.RegBitWidth = SLEEP_CONTROL_REG_BITWIDTH;
        Facp->SLEEP_CONTROL_REG.RegBitOffs  = SLEEP_CONTROL_REG_BITOFFSET;
        Facp->SLEEP_CONTROL_REG.AccessSize  = SLEEP_CONTROL_REG_ACCESS_SIZE;
        Facp->SLEEP_CONTROL_REG.Address     = SLEEP_CONTROL_REG_ADDRESS;

        Facp->SLEEP_STATUS_REG.AddrSpcID    = SLEEP_STATUS_REG_TYPE;
        Facp->SLEEP_STATUS_REG.RegBitWidth  = SLEEP_STATUS_REG_BITWIDTH;
        Facp->SLEEP_STATUS_REG.RegBitOffs   = SLEEP_STATUS_REG_BITOFFSET;
        Facp->SLEEP_STATUS_REG.AccessSize   = SLEEP_STATUS_REG_ACCESS_SIZE;
        Facp->SLEEP_STATUS_REG.Address      = SLEEP_STATUS_REG_ADDRESS;
#endif
    
        if (LOW_POWER_S0_IDLE_CAPABLE)       Facp->FLAGS |= 1<<21;
        Facp->Header.Revision = 5; // ACPI 5.0 revision
    }
    Facp->Header.Length     = SizeOfFacp;
    Facp->Header.Checksum = 0;
    Facp->Header.Checksum = ChsumTbl((UINT8*)Facp, Facp->Header.Length);
    
    return EFI_SUCCESS;
    
}// end of BuildFacpAll

#if GTDT_BUILD
/**
    This function allocates memory for and fills GTDT struscure.
         
    @param TablVer Version of GTDT table
    @param TablPtr - Pointer to memory, where the GTDT table will resides.
        	Filled by this procedure
    @retval 
  EFI_OUT_OF_RESOURCES - Memory for the table could not be allocated
  EFI_SUCCESS - Table was successfully build

**/

EFI_STATUS BuildGTDT (IN UINTN TablVer, OUT ACPI_HDR **TablPtr)

{
    EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE    *GTDT;
    UINTN	i;

    if (TablVer<1 || TablVer>4) return EFI_INVALID_PARAMETER;
    *TablPtr = MallocZ (sizeof(EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE));

    if ((*TablPtr)==NULL)
    {
        ASSERT(*TablPtr);
        return EFI_OUT_OF_RESOURCES;
    }

    GTDT = (EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE*)*TablPtr;
    GTDT->Header.Signature = EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE_SIGNATURE;
	GTDT->Header.Length = sizeof(EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE);
	GTDT->Header.Revision = EFI_ACPI_5_0_GENERIC_TIMER_DESCRIPTION_TABLE_REVISION;
	
	for (i=0; i<6; i++) GTDT->Header.OemId[i] = ACPI_OEM_ID[i];

	GTDT->Header.CreatorId = CREATOR_ID_AMI;
	GTDT->Header.CreatorRevision = CREATOR_REV_MS;
	
	//for (i=0; i<8; i++) GTDT->Header.OemTableId[i] = ACPI_OEM_TBL_ID[i];
	MemCpy(&GTDT->Header.OemTableId, &ACPI_OEM_TBL_ID, sizeof(ACPI_OEM_TBL_ID));

	GTDT->Header.OemRevision = 1;

	//Fill GTDT members
	GTDT->PhysicalAddress			= GTDT_PHYSICAL_ADDRESS; 
	GTDT->GlobalFlags				= GTDT_GLOBAL_FLAGS; 
	GTDT->SecurePL1TimerGSIV		= GTDT_SECURE_PL1_TIMER_GSIV; 
	GTDT->SecurePL1TimerFlags		= GTDT_SECURE_PL1_TIMER_FLAGS; 
	GTDT->NonSecurePL1TimerGSIV		= GTDT_NON_SECURE_PL1_TIMER_GSIV; 
	GTDT->NonSecurePL1TimerFlags    = GTDT_NON_SECURE_PL1_TIMER_FLAGS;
	GTDT->VirtualTimerGSIV			= GTDT_VIRTUAL_TIMER_GSIV;
	GTDT->VirtualTimerFlags			= GTDT_VIRTUAL_TIMER_FLAGS;
	GTDT->NonSecurePL2TimerGSIV		= GTDT_NON_SECURE_PL2_TIMER_GSIV;
	GTDT->NonSecurePL2TimerFlags    = GTDT_NON_SECURE_PL2_TIMER_FLAGS;
	
	GTDT->Header.Checksum = ChsumTbl((UINT8*)GTDT, GTDT->Header.Length);

    return EFI_SUCCESS;
}
#endif
#if ARM_MADT_BUILD

EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER		*Hdr;
EFI_ACPI_5_0_GIC_STRUCTURE      						*Gic;
EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE 					*GicDistributor;

/**
    This function allocates memory for and fills MADT structure.
         
    @param TablVer Version of MADT table
    @param TablPtr Pointer to memory, where the MADT table will resides.
        	Filled by this procedure
    @retval 
  EFI_OUT_OF_RESOURCES - Memory for the table could not be allocated
  EFI_SUCCESS - Table was successfully build

**/

EFI_STATUS BuildMADT (IN UINTN TablVer, OUT ACPI_HDR **TablPtr)

{
    UINT8 					i;
    UINTN					Index;
    ARM_PROCESSOR_TABLE   	*ArmProcessorTable;
	ARM_CORE_INFO    	    *ArmCoreInfoTable;
	UINT8					*MADT;
	
	for (Index=0; Index < pST->NumberOfTableEntries; Index++) {
		// Check for correct GUID type
		if (CompareGuid (&gArmMpCoreInfoGuid, &(pST->ConfigurationTable[Index].VendorGuid))) {
			// Get pointer to ARM processor table
			ArmProcessorTable = (ARM_PROCESSOR_TABLE *)pST->ConfigurationTable[Index].VendorTable;
			ArmCoreInfoTable = ArmProcessorTable->ArmCpus;
			break;
		}
	}

    if (TablVer<1 || TablVer>4) return EFI_INVALID_PARAMETER;
    
	*TablPtr = MallocZ (sizeof(EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER) + sizeof(EFI_ACPI_5_0_GIC_STRUCTURE) * ArmProcessorTable->NumberOfEntries + sizeof(EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE) );
    if ((*TablPtr)==NULL)
    {
        ASSERT(*TablPtr);
        return EFI_OUT_OF_RESOURCES;
    }

	MADT = (UINT8*)*TablPtr;	
    Hdr = (EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)MADT;
    PrepareHdr20(APIC_SIG, &(Hdr->Header), TablVer);
	Hdr->Header.Length = sizeof(EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER) + sizeof(EFI_ACPI_5_0_GIC_STRUCTURE) * ArmProcessorTable->NumberOfEntries + sizeof(EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE) ;
    Hdr->LocalApicAddress = 0;
    Hdr->Flags = 1;

	MADT = MADT + sizeof(EFI_ACPI_5_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER);
    //Fill GIC members
    for(i=0; i< ArmProcessorTable->NumberOfEntries; i++)
    {
		Gic = (EFI_ACPI_5_0_GIC_STRUCTURE *)MADT;
        Gic->Type = EFI_ACPI_5_0_GIC;
        Gic->Length = sizeof(EFI_ACPI_5_0_GIC_STRUCTURE);
        Gic->GicId = i;
        Gic->AcpiProcessorUid = i;
        Gic->Flags = ARM_MADT_GIC_FLAGS;
        Gic->ParkedAddress = ArmCoreInfoTable[i].MailboxSetAddress; 
        Gic->PhysicalBaseAddress = (UINT64) PcdGet32(PcdGicInterruptInterfaceBase);
		MADT = MADT + sizeof(EFI_ACPI_5_0_GIC_STRUCTURE);
    }

    //Fill GIC distrubutor
    GicDistributor = (EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE *)MADT;
    GicDistributor->Type = EFI_ACPI_5_0_GICD;
    GicDistributor->Length = sizeof(EFI_ACPI_5_0_GIC_DISTRIBUTOR_STRUCTURE);
    GicDistributor->GicId = ARM_MADT_GIC_DISTR_ID;
    GicDistributor->PhysicalBaseAddress = (UINT64) PcdGet32(PcdGicDistributorBase);
    
    Hdr->Header.Checksum = ChsumTbl((UINT8*)*TablPtr, Hdr->Header.Length);
    
    return EFI_SUCCESS;
}
#endif
/**
    Allocates ACPI NVS memory and builds FACS table from values,
    defined by SDL tokens

    @param VOID



    @retval EFI_OUT_OF_RESOURCES not enough memory
    @retval EFI_SUCCESS FACS table were successfully build

**/

EFI_STATUS BuildFacs ()
{
    EFI_STATUS      Status;
    UINTN           Size = sizeof(FACS_20);
    
    

    Status = pBS->AllocatePool(EfiACPIMemoryNVS, Size+64, (VOID **)&gxFacs);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    
	gxFacs = (FACS_20*)((UINTN) gxFacs + 64);
    gxFacs = (FACS_20*)((UINTN) gxFacs & (~0x3F));
    pBS->SetMem(gxFacs, Size, 0);
    gxFacs->Signature=(UINT32)FACS_SIG;
    gxFacs->Length=sizeof(FACS_20);
    
    if (ACPI_BUILD_TABLES_4_0 != 1)
    {
        gxFacs->Flags=FACS_FLAG_S4BIOS;
        gxFacs->Version=ACPI_REV1;
    }
    else
    {
        gxFacs->Flags = FACS_FLAG_S4BIOS | (FACS_FLAG_64BIT_WAKE_SUPPORTED << 1);
        gxFacs->Version=ACPI_REV2;
    }
    
    return EFI_SUCCESS;
}// end of  BuildFacs ---------------------------------------------

/**
    This function finds DSDT table in firmvare volume

         
    @param Dsdt1 pointer to memory where DSDT v1.1 table will be stored
    @param Dsdt2 pointer to memory where DSDT v2.0+ table will be stored

          
    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_ABORTED ACPI storage file not found
    @retval EFI_NOT_FOUND DSDT table not found in firmware volume

**/

EFI_STATUS GetDsdtFv(OUT ACPI_HDR **Dsdt2)
{
	EFI_STATUS	Status;
	
    //In current AmiBoardInfo implementation separate instance of
    //DSDT for ACPI version 1.1b  DOES OT SUPPORTED!
	Status=AmiSdlInitBoardInfo();
	if(EFI_ERROR(Status)){
		TRACE((-1,"AcpiCore: No AmiBoardInfo Found: Status=%r\n", Status));
		return EFI_NOT_FOUND;
	}

    *Dsdt2 = Malloc(((ACPI_HDR*)gAmiBoardInfo2Protocol->BrdAcpiInfo)->Length);
    ASSERT(*Dsdt2);
    
    if (*Dsdt2==NULL) return EFI_OUT_OF_RESOURCES;
    
    pBS->CopyMem(*Dsdt2, gAmiBoardInfo2Protocol->BrdAcpiInfo,
                 ((ACPI_HDR*)gAmiBoardInfo2Protocol->BrdAcpiInfo)->Length);

    
    
    if  (*Dsdt2 == NULL)
    {
        TRACE((-1,"Acpi: No DSDT was FOUND: Status=EFI_NOT_FOUND\n"));
        return EFI_NOT_FOUND;
    }
    
    PrepareHdr20(DSDT_SIG, (ACPI_HDR*)(*Dsdt2),2);
    
    return EFI_SUCCESS;
    
}// end of GetDsdtFv -----------------------------------------------

/**
    This function Updates FACP with the new values for DSDT and Facs
    pointers

    @param VOID

    @retval EFI_SUCCESS Function executed successfully
    @retval EFI_ABORTED Error


    @note    Modifies  gAcpiData

**/

EFI_STATUS UpdateFacp () 
{
    ACPI_HDR    *Facp2 = NULL, *Dsdt2 = NULL;
    UINTN       i;
    EFI_STATUS  Status = EFI_SUCCESS;
    
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG)
        {
        	Facp2 = gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find FACP for V 2+
        }
        
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG)
        {
        	Dsdt2 = gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find DSDT for V 2+
        }
    }
    
    
    if ((Facp2 == NULL) || (Dsdt2 == NULL)) Status = EFI_ABORTED;
    
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR (Status)) return Status;
    
    ((FACP_20*) Facp2)->FIRMWARE_CTRL = 0;
    
    ((FACP_20*) Facp2)->DSDT = 0;
    
    ((FACP_20*) Facp2)->X_FIRMWARE_CTRL = (UINT64) ((UINTN) gxFacs);
    
    ((FACP_20*) Facp2)->X_DSDT = (UINT64) ((UINTN)Dsdt2);
    Facp2->Checksum = 0;
    Facp2->Checksum = ChsumTbl((UINT8*)Facp2, Facp2->Length);
    return Status;
}// end of UpdateFacp

/**
    Copys table from position pointed by FromPtr to a position pointed
    by ToPtr (which is in EfiACPIReclaimMemory) and fills RSDT and XSDT
    pointers with ToPtr value


    @param RsdtPtr pionter to RSDT
    @param XsdtPtr pionter to XSDT
    @param FromPtr pointer to a table which should be copyed
    @param ToPtr pointer to EfiACPIReclaimMemory where table should be placed

    @retval  Pointer to the next avaiable space in allocated EfiACPIReclaimMemory
        right after copyed table (alligned already)
        If NUUL - Invalid parameters.

**/

UINT8 *FillAcpiStr (RSDT32 *RsdtPtr, XSDT_20 *XsdtPtr, VOID *FromPtr, VOID *ToPtr)
{
    UINT8       *NextPtr;
    UINTN       i;
    
    
    if ((RsdtPtr == NULL) && (XsdtPtr == NULL)) return NULL;
    
    if ((FromPtr == NULL) || (ToPtr == NULL)) ASSERT(0);
    
    pBS->CopyMem(ToPtr, FromPtr, ((ACPI_HDR*)FromPtr)->Length);
    
    if (RsdtPtr != NULL)
    {
        for (i = 0; RsdtPtr->Ptrs[i] != 0; i++); // Find first unfilled (last) entry in RSDT
        
        RsdtPtr->Ptrs[i] = (UINT32) ToPtr;
    }
    
    if (XsdtPtr != NULL)
    {
        for (i = 0; XsdtPtr->Ptrs[i] != 0; i++); // Find first unfilled (last) entry in XSDT
        
        XsdtPtr->Ptrs[i] = (UINT64) ((UINTN)ToPtr);
    }
    
    NextPtr = (UINT8*) ((UINT8*)ToPtr+((ACPI_HDR*)ToPtr)->Length + 7);
    
    NextPtr = (UINT8 *)((UINTN)NextPtr & (~0x7));
    
    return NextPtr;
    
}//end of FillAcpiStr

/**
    Creates or rewrites RSDT_PTR structure and copys tables, stored
    in gAcpiData structure in allocated EfiACPIReclaimMemory.


    @param RsdtBuild if 1 - Build RSDT and copy tables of Ver 1.1
    @param XsdtBuild if 1 - Build XSDT and copy tables of Ver 2+

    @retval EFI_OUT_OF_RESOURCES not enough memory
    @retval EFI_ABORTED invalid parameters
    @retval EFI_SUCCESS RSDT_PTR structure was successfully build

**/
EFI_STATUS PublishTbl (IN UINT8 RsdtBuild, IN UINT8 XsdtBuild)
{
    ACPI_HDR    *FacpX = NULL, *DsdtX = NULL;
    RSDT32      *Rsdt = NULL;
    XSDT_20     *Xsdt = NULL;
    UINTN       i, XsdtCnt = 0, SpaceNeeded = 0, XsdtSize = 0;
    EFI_STATUS  Status;
    UINT8       *Ptr = NULL, *Dummy = NULL;
    EFI_PHYSICAL_ADDRESS Memory;
    
    if ((XsdtBuild == 0)  || (XsdtBuild > 1)) return EFI_ABORTED;

    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
    	XsdtCnt ++;
            
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG)
        	FacpX = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
                
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG)
            DsdtX = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
    }
        
    
    XsdtSize = XsdtBuild * (sizeof(ACPI_HDR) + (XsdtCnt-1) * sizeof(UINT64) + 7); //DSDT does not goes in XSDT (XsdtCnt-1)
    SpaceNeeded = sizeof(RSDT_PTR_20) + XsdtSize + gAcpiData.AcpiLength + (gAcpiData.AcpiEntCount + 1) * 8;
    
    gAcpiTblPages = EFI_SIZE_TO_PAGES(SpaceNeeded);

    Status = pBS->AllocatePages(AllocateAnyPages, EfiACPIReclaimMemory, gAcpiTblPages , &Memory);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    if (gRsdtPtrStr != NULL) 
    {
    	Status = pBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN) gRsdtPtrStr, gAcpiTblPages);
        ASSERT_EFI_ERROR(Status);
    }

    gRsdtPtrStr = (RSDT_PTR_20*)Memory;
    
    pBS->SetMem(gRsdtPtrStr, SpaceNeeded, 0);    
    
 

    Dummy = (UINT8*) ((UINT8*)gRsdtPtrStr + sizeof(RSDT_PTR_20) + 7);
    Dummy = (UINT8*)((UINTN) Dummy & (~0x7));
    Xsdt = (XSDT_20*) Dummy;
        
    Ptr = (UINT8*) ((UINT8*)gRsdtPtrStr + sizeof(RSDT_PTR_20) + XsdtSize + 7);
    Ptr = (UINT8*)((UINTN) Ptr & (~0x7));

    Dummy = FillAcpiStr (NULL, Xsdt, (VOID*) FacpX,( VOID*) Ptr);
    FacpX = (ACPI_HDR*) Ptr;
    Ptr = Dummy;
        
    pBS->CopyMem((VOID*)Ptr, (VOID*)DsdtX, DsdtX->Length);
    DsdtX = (ACPI_HDR*) Ptr;
    Ptr += (DsdtX->Length + 7);
    Ptr = (UINT8*)((UINTN) Ptr &(~0x7));

    ((FACP_20*)FacpX)->FIRMWARE_CTRL = 0;

    ((FACP_20*)FacpX)->DSDT = 0;
        
    ((FACP_20*)FacpX)->X_FIRMWARE_CTRL = (UINT64) ((UINTN)gxFacs);
        
    ((FACP_20*)FacpX)->X_DSDT = (UINT64) ((UINTN)DsdtX);
    FacpX->Checksum = 0;
    FacpX->Checksum = ChsumTbl((UINT8*)FacpX, FacpX->Length);

    
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if ((gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG) ||
                (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG) ||
                (gAcpiData.AcpiEntries[i]->AcpiTblVer < EFI_ACPI_TABLE_VERSION_1_0B)) 
        	continue;
        
        Dummy = FillAcpiStr (NULL, Xsdt, (VOID*) gAcpiData.AcpiEntries[i]->BtHeaderPtr,( VOID*) Ptr);        
        if (Dummy != NULL) Ptr = Dummy;
  
    }
    
    if (Ptr > ((UINT8*)gRsdtPtrStr + SpaceNeeded))  Status = EFI_OUT_OF_RESOURCES;
    
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return Status;
    
    gRsdtPtrStr->Signature = RSDP_SIG;

    gRsdtPtrStr->Revision = ACPI_REV2; // 2 for ver ACPI 2.0 and up
    
    gRsdtPtrStr->Length = sizeof(RSDT_PTR_20);//this is the length of entire structure
    
    for (i=0; i<6; i++) gRsdtPtrStr->OEMID[i]=ACPI_OEM_ID[i];
    
    gRsdtPtrStr->XsdtAddr = (UINT64)(UINTN) Xsdt;
    gRsdtPtrStr->Checksum = ChsumTbl((UINT8*)gRsdtPtrStr, 20);
    gRsdtPtrStr->XtdChecksum = ChsumTbl((UINT8*)gRsdtPtrStr, sizeof (RSDT_PTR_20));

    PrepareHdr20 (XSDT_SIG, (ACPI_HDR*) Xsdt, 3);

    Xsdt->Header.Length = (UINT32) (XsdtSize - 7);
    Xsdt->Header.Checksum = 0;
    Xsdt->Header.Checksum = ChsumTbl((UINT8*)Xsdt, Xsdt->Header.Length);

    Status = pBS->InstallConfigurationTable(&gAcpi20TAbleGuid, (VOID*) gRsdtPtrStr);

    
    ASSERT_EFI_ERROR(Status);
    
    return Status;

    
}// end of PublishTbl

/**
    This function builds mandatory ACPI tables

         
    @param VOID

          
    @retval EFI_SUCCESS Function executed successfully, ACPI_SUPPORT_PROTOCOL installed
    @retval EFI_ABORTED Dsdt table not found or table publishing failed
    @retval EFI_ALREADY_STARTED driver already started
    @retval EFI_OUT_OF_RESOURCES not enough memory to perform operation

**/

EFI_STATUS BuildMandatoryAcpiTbls ()
{
    EFI_STATUS      Status = EFI_SUCCESS;
    ACPI_HDR        *Dsdt2Ptr = NULL;

    static EFI_GUID Acpisupguid = EFI_ACPI_SUPPORT_GUID;
//	    static EFI_GUID AcpiTableProtocolGuid = EFI_ACPI_TABLE_PROTOCOL_GUID;
    UINTN           AcpiVer;
    EFI_ACPI_TABLE_VERSION  EfiAcpiVer;
    ACPI_TBL_ITEM   *AcpiTable = NULL;
#if FORCE_TO_ACPI1_SETUP_ENABLE
    SETUP_DATA      *SetupData = NULL;
    UINTN           SetupSize = 0;
#endif
#if ATAD_SUPPORT == 1
	EFI_GUID    AtadSmiGuid = ATAD_SMI_GUID;
	UINTN AtadVarSize = sizeof(AtadBuffPtr);
#endif
//------------------------
  
	AcpiVer = 4;
    EfiAcpiVer = EFI_ACPI_TABLE_VERSION_4_0;


#if ATAD_SUPPORT == 1

    Status = pBS->AllocatePool(EfiRuntimeServicesData, 4, &AtadBuffPtr);
    if (!EFI_ERROR(Status) && AtadBuffPtr)
    {
        Status = pRS->SetVariable ( L"AtadSmiBuffer",
        							&AtadSmiGuid,
        							EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        							AtadVarSize,
                                    &AtadBuffPtr );
        ASSERT_EFI_ERROR(Status);
    }

#endif
    Status = BuildFacs ();
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR (Status)) return EFI_OUT_OF_RESOURCES;
    
    AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
    ASSERT(AcpiTable);
    
    if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
    
    Status = BuildFacpAll (AcpiVer, &AcpiTable->BtHeaderPtr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR (Status)) return EFI_OUT_OF_RESOURCES;
    
    AcpiTable->AcpiTblVer = EFI_ACPI_TABLE_VERSION_2_0;
    Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    
    gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
    
    TRACE((-1,"IN ACPI 1: %x\n", Status));
    
    Status = GetDsdtFv(&Dsdt2Ptr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_ABORTED;
    
    if (Dsdt2Ptr != NULL)
    {
        TRACE((-1,"DSDT21 addres 0x%lX; -> %r \n", Dsdt2Ptr, Status));
        Dsdt2Ptr->Checksum = 0;
        Dsdt2Ptr->Checksum = ChsumTbl((UINT8*)Dsdt2Ptr, Dsdt2Ptr->Length);
        AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
        ASSERT(AcpiTable);
        
        if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
        
        AcpiTable->BtHeaderPtr = Dsdt2Ptr;
        AcpiTable->AcpiTblVer = EFI_ACPI_TABLE_VERSION_2_0;
        Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
        
        gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
    }
    else return EFI_ABORTED;
    
    Status = UpdateFacp ();
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_ABORTED;
 
//------ Generic Timer Description Table ---------------------------
#if GTDT_BUILD
   AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
   ASSERT(AcpiTable);

   if (AcpiTable)
   {
       Status = BuildGTDT (AcpiVer, &AcpiTable->BtHeaderPtr);
       if (!EFI_ERROR(Status))
       {
    	   AcpiTable->AcpiTblVer = EfiAcpiVer;
           Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
           ASSERT_EFI_ERROR(Status);
           gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
       }
   }
#endif
//------ Generic Timer Description Table End ------------------------

//------ Multiple APIC Description Table ---------------------------
#if ARM_MADT_BUILD
   AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
   ASSERT(AcpiTable);

   if (AcpiTable)
   {
	   Status = BuildMADT(AcpiVer, &AcpiTable->BtHeaderPtr);
       if (!EFI_ERROR(Status))
       {
    	   AcpiTable->AcpiTblVer = EfiAcpiVer;
           Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
           ASSERT_EFI_ERROR(Status);
           gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
       }
   }
#endif
//------ Multiple APIC Description Table End ------------------------

    
//------ FID Table Start -------------------------------------------
    AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
    ASSERT(AcpiTable);
    
    if (AcpiTable)
    {
        Status = BuildFIDT (&AcpiTable->BtHeaderPtr);

        if (!EFI_ERROR(Status)) 
        {
            AcpiTable->AcpiTblVer = EfiAcpiVer;
            Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
            ASSERT_EFI_ERROR(Status);
            gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
        }
    }
    return EFI_SUCCESS;
}

/**
    This function will be called when ReadyToBoot event will be signaled and
    will publish all ACPI tables 

         
    @param Event signalled event
    @param Context calling context

          
    @retval VOID

**/
VOID ArmAcpiReadyToBootFunction(EFI_EVENT Event, VOID *Context)
{
	EFI_STATUS          Status;
	Status = PublishTbl (0, 1);
	
	ASSERT_EFI_ERROR(Status);
	gPublishedOnReadyToBoot = 1;
}

/**
    This function craeate Ready to Boot event

         
    @param VOID

    @retval 
        EFI_STATUS, based on result


**/

EFI_STATUS AcpiReadyToBootEvent() 
{
	EFI_EVENT               EvtReadyToBoot;
    
    return  CreateReadyToBootEvent(
                 TPL_CALLBACK, ArmAcpiReadyToBootFunction, NULL, &EvtReadyToBoot
             );
}   

/**
    Library constructor - Init Ami Lib

         
    @param ImageHandle Image handle
    @param SystemTable pointer to system table

    @retval 
        EFI_SUCCESS


**/
EFI_STATUS EFIAPI AmiAcpiLibConstructor (IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable)
{
	  InitAmiLib(ImageHandle,SystemTable);
	  return EFI_SUCCESS;
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-20114, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
