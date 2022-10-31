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

//**********************************************************************
// $Header: /Alaska/BIN/Modules/ACPI/Template/Core/ACPICORE.C 78    3/16/11 12:07p Oleksiyy $
//
// $Revision: 78 $
//
// $Date: 3/16/11 12:07p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    <AcpiCore.c>
//
// Description: Main ACPI Driver File. It has ACPI Driver entry point,
//              ACPISupport Protocol and ACPITable Protocol.
//
//<AMI_FHDR_END>
//**********************************************************************

#include <Library/AmiAcpiCpuLib.h>
#include <AmiDxeLib.h>
#include <Protocol/SuperIo.h>
#include <Protocol/DevicePath.h>
#include <token.h>
#include <Dxe.h>
#include <AcpiRes.h>
#include "AcpiCore.h"
#include <Setup.h>
#include "ACPI50.h"
#include <Protocol/Cpu.h>

#include <Protocol/AmiBoardInfo2.h>
#include <Library/AmiSdlLib.h>
#include <AcpiOemElinks.h>

//--------------------------------------
//Some Global vars
EFI_GUID gEfiFirmwareVolumeProtocolGuid = EFI_FIRMWARE_VOLUME_PROTOCOL_GUID;
EFI_GUID gDsdt11Guid                    = EFI_ACPI_DSDT_V_1_1_GUID;
EFI_GUID gDsdt20Guid                    = EFI_ACPI_DSDT_V_2_0_GUID;
EFI_GUID gSetupGuid                     = SETUP_GUID;

ACPI_DB                 gAcpiData = {0, 0, NULL, 0};
FACS_20                 *gFacs, *gxFacs;
RSDT_PTR_20             *gRsdtPtrStr = NULL;
UINTN                   gAcpiTblPages = 0;
ACPI_AML_UPD_INFO       *gAuiGlob = NULL;
UINT16                  gAcpiIaBootArch=0xFFFF;
EFI_EVENT               gEvtReadyToBoot;
EFI_GUID                gAcpi11TAbleGuid = EFI_ACPI_11_TABLE_GUID;
EFI_GUID                gAcpi20TAbleGuid = EFI_ACPI_20_TABLE_GUID;
UINT8                   gForceAcpi1 = 0, gPublishedOnReadyToBoot = 0;
EFI_FPDT_STRUCTURE          *FpdtVar = NULL;
//Here goes Interrupt Source Override MADT entry parameter table
//Generated From Information on IRQ_XX_OVERRIDE_ENABLE SDL tokens
ISO_PARAMETER_TABLE IsoTbl[]=
{
//UINT8 PicIrq; UINT8   Flags;                              UINT16  ApicInt
#if IRQ_00_OVERRIDE_ENABLE == 1
    {   0x00,       (IRQ_00_TRIGGER_MODE<<2)|IRQ_00_POLARITY,   IRQ_00_APIC_INT },
#else
    {   0xFF,       0,                                          0               },
#endif
#if IRQ_01_OVERRIDE_ENABLE == 1
    {   0x01,       (IRQ_01_TRIGGER_MODE<<2)|IRQ_01_POLARITY,   IRQ_01_APIC_INT },
#else
    {   0xFF,       0,                                          1               },
#endif
//just dummy entry instead of IRQ2 to keep array consistent
    {   0xFF,       0,                                          2               },
#if IRQ_03_OVERRIDE_ENABLE == 1
    {   0x03,       (IRQ_03_TRIGGER_MODE<<2)|IRQ_03_POLARITY,   IRQ_03_APIC_INT },
#else
    {   0xFF,       0,                                          3               },
#endif
#if IRQ_04_OVERRIDE_ENABLE == 1
    {   0x04,       (IRQ_04_TRIGGER_MODE<<2)|IRQ_04_POLARITY,   IRQ_04_APIC_INT },
#else
    {   0xFF,       0,                                          4               },
#endif
#if IRQ_05_OVERRIDE_ENABLE == 1
    {   0x05,       (IRQ_05_TRIGGER_MODE<<2)|IRQ_05_POLARITY,   IRQ_05_APIC_INT },
#else
    {   0xFF,       0,                                          5               },
#endif
#if IRQ_06_OVERRIDE_ENABLE == 1
    {   0x06,       (IRQ_06_TRIGGER_MODE<<2)|IRQ_06_POLARITY,   IRQ_06_APIC_INT },
#else
    {   0xFF,       0,                                          6               },
#endif
#if IRQ_07_OVERRIDE_ENABLE == 1
    {   0x07,       (IRQ_07_TRIGGER_MODE<<2)|IRQ_07_POLARITY,   IRQ_07_APIC_INT },
#else
    {   0xFF,       0,                                          7               },
#endif
    
#if IRQ_08_OVERRIDE_ENABLE == 1
    {   0x08,       (IRQ_08_TRIGGER_MODE<<2)|IRQ_08_POLARITY,   IRQ_08_APIC_INT },
#else
    {   0xFF,       0,                                          8               },
#endif
#if IRQ_09_OVERRIDE_ENABLE == 1
    {   0x09,       (IRQ_09_TRIGGER_MODE<<2)|IRQ_09_POLARITY,   IRQ_09_APIC_INT },
#else
    {   0xFF,       0,                                          9               },
#endif
#if IRQ_10_OVERRIDE_ENABLE == 1
    {   0x0A,       (IRQ_10_TRIGGER_MODE<<2)|IRQ_10_POLARITY,   IRQ_10_APIC_INT },
#else
    {   0xFF,       0,                                          10              },
#endif
#if IRQ_11_OVERRIDE_ENABLE == 1
    {   0x0B,       (IRQ_11_TRIGGER_MODE<<2)|IRQ_11_POLARITY,   IRQ_11_APIC_INT },
#else
    {   0xFF,       0,                                          11              },
#endif
#if IRQ_12_OVERRIDE_ENABLE == 1
    {   0x0C,       (IRQ_12_TRIGGER_MODE<<2)|IRQ_12_POLARITY,   IRQ_12_APIC_INT },
#else
    {   0xFF,       0,                                          12              },
#endif
#if IRQ_13_OVERRIDE_ENABLE == 1
    {   0x0D,       (IRQ_13_TRIGGER_MODE<<2)|IRQ_13_POLARITY,   IRQ_13_APIC_INT },
#else
    {   0xFF,       0,                                          13              },
#endif
#if IRQ_14_OVERRIDE_ENABLE == 1
    {   0x0E,       (IRQ_14_TRIGGER_MODE<<2)|IRQ_14_POLARITY,   IRQ_14_APIC_INT },
#else
    {   0xFF,       0,                                         14               },
#endif
#if IRQ_15_OVERRIDE_ENABLE == 1
    {   0x0F,       (IRQ_15_TRIGGER_MODE<<2)|IRQ_15_POLARITY,   IRQ_15_APIC_INT },
#else
    {   0xFF,       0,                                          15              },
#endif
};

static UINTN    IsoCnt=sizeof(IsoTbl)/sizeof(ISO_PARAMETER_TABLE);

EFI_STATUS MpsTableBuilderInit(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
);

UINT8    ACPI_OEM_ID[6]     = ACPI_OEM_ID_MAK;     //"A M I";      //add 0 at the end.OemId 6 bytes
UINT8    ACPI_OEM_TBL_ID[8] = ACPI_OEM_TBL_ID_MAK; //"ALASKA"; //add 0 at the end.OemTableId  8 bytes

#if defined(OemActivation_SUPPORT) && (OemActivation_SUPPORT == 1)
#define EFI_OA3_MSDM_VARIABLE   L"OA3MSDMvariable"

typedef struct _EFI_OA3_MSDM_STRUCTURE {
    EFI_PHYSICAL_ADDRESS  XsdtAddress;
    EFI_PHYSICAL_ADDRESS  MsdmAddress;
    EFI_PHYSICAL_ADDRESS  ProductKeyAddress;
} EFI_OA3_MSDM_STRUCTURE;

BOOLEAN gOA3Variable = FALSE;
EFI_OA3_MSDM_STRUCTURE gMsdmVariable;
#endif
extern EFI_GUID gAmiGlobalVariableGuid;
typedef EFI_STATUS (ACPI_OEM_FUNCTION)(IN OUT ACPI_HDR *AcpiHdr);
extern ACPI_OEM_FUNCTION OEM_LIST EndOfOemList;
ACPI_OEM_FUNCTION* AcpiOemPartsList[] = {OEM_LIST NULL};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   OemAcpiSetPlatformId
//
//  Description:
//  This function provides platform specific OEM_ID and OEM_TABLE_ID to
//  overwrite default ACPI Table header.
//
//  Input:
// 	IN OUT ACPI_HDR *AcpiHdr    - ACPI TABLE header
//
//  Output:
//  EFI_STATUS  EFI_SUCCESS if Values overwtitten.
//              EFI_INSUPPORTED if no need to change values - use default.
//
//  NOTE: UINT8 *AcpiOemId[6]; UINT8 *AcpiOemTblId[8] avoid buffer overrun!
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS OemAcpiSetPlatformId(IN OUT ACPI_HDR *AcpiHdr){
    EFI_STATUS       Status=EFI_UNSUPPORTED;
    UINTN i;

    for (i=0; AcpiOemPartsList[i]; i++) Status = AcpiOemPartsList[i](AcpiHdr);

    return Status;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MemCopy
//
// Description: This routine copies data from source to destination.
//
// Input:
//    fpSrc    - Pointer to the source.
//    fpDest   - Pointer to the destination.
//    wSize    - Number of bytes to copy.
//
// Output: None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
MemCopy (
    UINT8*  fpSrc,
    UINT8*  fpDest,
    UINT32  dSize)
{
    UINT32  dCount;
    
    //
    // Check for pointer validity
    //
    if ((fpSrc) && (fpDest))
    {
        for (dCount = 0; dCount < dSize; dCount++)
        {
            fpDest[dCount] = fpSrc[dCount];
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   PrepareHdr20
//
//  Description:
//  This function creates ACPI table v 2.0+ header with specified signature
//
//  Input:
//  IN UINT32 TblSig - ACPI table signature
//  IN OUT ACPI_HDR* HdrPtr - Pointer to memory, where the header should be placed
//
//  Output:
//  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   PrepareHdr1
//
//  Description:
//  This function creates ACPI V1 table header with specified signature
//
//  Input:
//  IN UINT32 TblSig - ACPI table signature
//  IN OUT ACPI_HDR* HdrPtr - Pointer to memory, where the header should be placed
//
//  Output:
//  VOID
//
//  Notes:
//  Depends on type of memory provided, ACPI table header can be in 32 or 64 bit
//  format
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

void PrepareHdr1(UINT32 TblSig, ACPI_HDR* HdrPtr)
{
    UINTN i;
    EFI_STATUS  Status;
    
    if (HdrPtr==NULL) return;
    
    HdrPtr->Signature = TblSig;
    HdrPtr->Revision = ACPI_REV1;
    
    //Dont' touch Creator ID and Creator Revision;
    if (TblSig != DSDT_SIG)
    {
        if (TblSig == RSDT_SIG)
            HdrPtr->CreatorId = CREATOR_ID_MS;
        else
            HdrPtr->CreatorId = CREATOR_ID_AMI;
            
        HdrPtr->CreatorRev = CREATOR_REV_MS;
        HdrPtr->OemRev = ACPI_OEM_REV;
    }

    //Get the platform specific OEM_IDs
    Status=OemAcpiSetPlatformId(HdrPtr);
    
    //if platform does not support OEM_ID overwrite
    if (EFI_ERROR(Status))
    {
        for (i=0; i<6; i++) HdrPtr->OemId[i]=ACPI_OEM_ID[i];
        
        for (i=0; i<8; i++) HdrPtr->OemTblId[i]=ACPI_OEM_TBL_ID[i];
    }

}//PrepareHdr1

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   GetCpuNanoFreq
//
//  Description: This function will be called to locate CPU Protocol
//							and get Cpu frequency.
//
//  Input:  IN EFI_EVENT Event - signalled event
//          IN VOID *Context - calling context
//
//  Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetCpuNanoFreq (
            IN EFI_EVENT    Event,
            IN VOID         *Context
)
{
	UINT64                    CurrentTicker, TimerPeriod;
	EFI_STATUS                  Status;
    EFI_CPU_ARCH_PROTOCOL       *Cpu;

	Status = pBS->LocateProtocol (
              &gEfiCpuArchProtocolGuid,
              NULL,
              &Cpu
              );
	if (EFI_ERROR (Status)) {
		TRACE((-1,"Acpi BuildFPDT: No Cpu Protocol was found.\n"));
		return Status;
	}

	Status = Cpu->GetTimerValue (Cpu, 0, &CurrentTicker, &TimerPeriod);
	if (!EFI_ERROR (Status)) {
		FpdtVar->NanoFreq = TimerPeriod;
		TRACE((-1,"Acpi BuildFPDT:  Cpu Protocol was found.\n"));
		pBS->CloseEvent(Event);
		}
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   BuildFPDT 
//
//  Description: This function allocates memory for and fills FPDT struscure. It also
//               creates Variable with the addres od S3 and Normal Boot performance srtuctures
//               to be filled later. 
//  Input:
//  IN UINTN TablVer - Version of FACP table
//  IN OUT **ACPI_HDR TablPtr - Pointer to memory, where the FACP table will resides.
//                              Filled by this procedure
//  Output:
//  EFI_STATUS:
//  EFI_OUT_OF_RESOURCES - Memory for the table could not be allocated
//  EFI_SUCCESS - Table was successfully build
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS BuildFPDT (IN UINTN TablVer, OUT ACPI_HDR **TablPtr)

{
    UINT8                       *TempPtr;
    PERF_TAB_HEADER             *S3PerfRecHdr, *BasBootPerfRecHdr;
    BASIC_S3_RESUME_PERF_REC    *S3PerfRec;
    BASIC_BOOT_PERF_REC         *BasBootPerfRec;
    UINTN                       RecordsSize;
    FPDT_50                     *FPDT;
    EFI_STATUS                  Status;
    EFI_FPDT_STRUCTURE          *OldFpdtVarAddress=NULL;
    UINTN                       VarSize = sizeof (UINT32);
    EFI_EVENT   EvtCpu;
    VOID         *RegCpu =  NULL;
//-----------------------------

    if (TablVer<1 || TablVer>4) return EFI_INVALID_PARAMETER;
    RecordsSize = (sizeof(EFI_FPDT_STRUCTURE) + sizeof(PERF_TAB_HEADER)*2 + 
                        sizeof(BASIC_S3_RESUME_PERF_REC)+
                        sizeof(BASIC_BOOT_PERF_REC));
    Status = pBS->AllocatePool(EfiRuntimeServicesData, RecordsSize, &TempPtr);
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
#ifdef EFIx64
    if (((UINT64)TempPtr) & 0xffffffff00000000)
    {
        EFI_PHYSICAL_ADDRESS Memory = 0x00000000ffffffff;
        Status = pBS->FreePool(TempPtr);
        ASSERT_EFI_ERROR(Status);
        Status = pBS->AllocatePages(AllocateMaxAddress, EfiRuntimeServicesData, 1, &Memory);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
        
        TempPtr = (UINT8*)Memory;
    }
#endif
    pBS->SetMem(TempPtr, RecordsSize, 0);

    *TablPtr = MallocZ (sizeof(FPDT_50));

    if ((*TablPtr)==NULL)
    {
        ASSERT(*TablPtr);
        return EFI_OUT_OF_RESOURCES;
    }
    FpdtVar = (EFI_FPDT_STRUCTURE*) TempPtr;
    TempPtr += sizeof(EFI_FPDT_STRUCTURE);
    S3PerfRecHdr = (PERF_TAB_HEADER*) TempPtr;
    TempPtr += sizeof(PERF_TAB_HEADER);
    S3PerfRec = (BASIC_S3_RESUME_PERF_REC*)TempPtr;
    TempPtr += sizeof(BASIC_S3_RESUME_PERF_REC);
    BasBootPerfRecHdr = (PERF_TAB_HEADER*) TempPtr;
    TempPtr += sizeof(PERF_TAB_HEADER);
    BasBootPerfRec = (BASIC_BOOT_PERF_REC*)TempPtr;

    S3PerfRecHdr->Signature = 0x54503353;//   `S3PT'
    S3PerfRecHdr->Length = sizeof(PERF_TAB_HEADER) + sizeof(BASIC_S3_RESUME_PERF_REC);

    S3PerfRec->Header.PerfRecType = 0;
    S3PerfRec->Header.RecLength = sizeof(BASIC_S3_RESUME_PERF_REC);
    S3PerfRec->Header.Revision = 1;

    BasBootPerfRecHdr->Signature = 0x54504246;//  `FBPT'
    BasBootPerfRecHdr->Length = sizeof(PERF_TAB_HEADER) + sizeof(BASIC_BOOT_PERF_REC);

    BasBootPerfRec->Header.PerfRecType = 2;
    BasBootPerfRec->Header.RecLength = sizeof(BASIC_BOOT_PERF_REC);
    BasBootPerfRec->Header.Revision = 2;

    FPDT = (FPDT_50*)*TablPtr;
    PrepareHdr20(FPDT_SIG, (ACPI_HDR*)FPDT, TablVer);
    FPDT->Header.Length = sizeof(FPDT_50);
    FPDT->Header.Revision = 1;
    FPDT->BasS3Rec.PerfRecType = 1;
    FPDT->BasS3Rec.Length = sizeof(FPDT_PERF_RECORD);
    FPDT->BasS3Rec.Revision = 1;
    FPDT->BasS3Rec.Pointer = (EFI_PHYSICAL_ADDRESS)((UINTN)S3PerfRecHdr);

    FPDT->BasBootRec.PerfRecType = 0;
    FPDT->BasBootRec.Length = sizeof(FPDT_PERF_RECORD);
    FPDT->BasBootRec.Revision = 1;
    FPDT->BasBootRec.Pointer = (EFI_PHYSICAL_ADDRESS)((UINTN)BasBootPerfRecHdr);
    
    FPDT->Header.Checksum = ChsumTbl((UINT8*)FPDT, FPDT->Header.Length);
    //---------------------------------------------------------------------------------------------------------------------
    Status = GetCpuNanoFreq (NULL, NULL);  // Get Cpu Frequency

    if (EFI_ERROR(Status)){

        Status = RegisterProtocolCallback ( &gEfiCpuArchProtocolGuid,
        									GetCpuNanoFreq,
                                            NULL,
                                            &EvtCpu,
                                            &RegCpu );
    }

  
    FpdtVar->S3Pointer = FPDT->BasS3Rec.Pointer;
    FpdtVar->BasBootPointer = FPDT->BasBootRec.Pointer;
    Status = pRS->GetVariable(
            EFI_FPDT_VARIABLE, &gAmiGlobalVariableGuid,
		    NULL, &VarSize, &OldFpdtVarAddress
            ); 
    if (EFI_ERROR(Status) || (FpdtVar != OldFpdtVarAddress)) 
    {
        Status = pRS->SetVariable(
            EFI_FPDT_VARIABLE,
            &gAmiGlobalVariableGuid,
            EFI_VARIABLE_NON_VOLATILE |
            EFI_VARIABLE_BOOTSERVICE_ACCESS |
            EFI_VARIABLE_RUNTIME_ACCESS,
            sizeof(UINT32),
            &FpdtVar
        );
        ASSERT_EFI_ERROR(Status);
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   BuildFacpiAll (NUINT TablVer, VOID *TablPtr)
//
//  Description: This function allocates memory for and fills FACP table v 1.1+ with
//               predefined values from SDL tokens
//  Input:
//  IN UINTN TablVer - Version of FACP table
//  IN OUT **ACPI_HDR TablPtr - Pointer to memory, where the FACP table will resides.
//                              Filled by this procedure
//  Output:
//  EFI_STATUS:
//  EFI_OUT_OF_RESOURCES - Memory for the table could not be allocated
//  EFI_SUCCESS - Table was successfully build
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>


EFI_STATUS BuildFacpiAll (IN UINTN TablVer, OUT ACPI_HDR **TablPtr)

{
    FACP_50     *facp;
    ACPI_HDR    *FACP_Hdr;
    UINT32       SizeOfFacp = sizeof(FACP_20);
    SETUP_DATA  *SetupData = NULL; //CSP20140424_23
    UINTN        SetupSize = sizeof(SETUP_DATA); //CSP20140424_23
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
        
    facp = (PFACP_50) *TablPtr;
    
    facp->Reserved1     = ACPI_INT_MODEL;
    
    facp->PM_PPROF      = ACPI_PM_PROFILE;
    
    facp->FLAGS = 0;
    
#if HW_REDUCED_ACPI == 0
    facp->SCI_INT       = ACPI_SCI_INT;
    facp->SMI_CMD       = SW_SMI_IO_ADDRESS;
    facp->ACPI_ENABLE_CMD   = SW_SMI_ACPI_ENABLE;

    facp->ACPI_DISABLE_CMD  = SW_SMI_ACPI_DISABLE;

    facp->S4BIOS_REQ    = SW_SMI_S4BIOS; 
    
    facp->PSTATE_CNT    = SW_SMI_PSTATE_CNT;
    
    if (PM1A_EVT_BLK_ADDRESS > 0xffffffff)
        facp->PM1a_EVT_BLK  = 0;
    else
        facp->PM1a_EVT_BLK  = PM1A_EVT_BLK_ADDRESS;
        
    if (PM1B_EVT_BLK_ADDRESS > 0xffffffff)
        facp->PM1b_EVT_BLK  = 0;
    else
        facp->PM1b_EVT_BLK  = PM1B_EVT_BLK_ADDRESS;
        
    if (PM1A_CNT_BLK_ADDRESS > 0xffffffff) 
        facp->PM1a_CNT_BLK  = 0;
    else
        facp->PM1a_CNT_BLK  = PM1A_CNT_BLK_ADDRESS;
        
    if (PM1B_CNT_BLK_ADDRESS > 0xffffffff)
        facp->PM1b_CNT_BLK  = 0;
    else
        facp->PM1b_CNT_BLK  = PM1B_CNT_BLK_ADDRESS;
        
    if (PM2_CNT_BLK_ADDRESS > 0xffffffff)
        facp->PM2_CNT_BLK   = 0;
    else
        facp->PM2_CNT_BLK   = PM2_CNT_BLK_ADDRESS;
        
    if (PM_TMR_BLK_ADDRESS > 0xffffffff)
        facp->PM_TMR_BLK    = 0;
    else
        facp->PM_TMR_BLK    = PM_TMR_BLK_ADDRESS;
        
    if (GPE0_BLK_ADDRESS > 0xffffffff)
        facp->GPE0_BLK  = 0;
    else
        facp->GPE0_BLK  = GPE0_BLK_ADDRESS;
        
    if (GPE1_BLK_ADDRESS > 0xffffffff) 
        facp->GPE1_BLK  = 0;
    else
        facp->GPE1_BLK  = GPE1_BLK_ADDRESS;
        
    facp->GPE0_BLK_LEN          = GPE0_BLK_LENGTH; 
    facp->GPE1_BLK_LEN          = GPE1_BLK_LENGTH; 
    facp->GPE1_BASE             = GPE1_BASE_OFFSET; 
    facp->PM1_EVT_LEN   = PM1_EVT_LENGTH;
    facp->PM1_CNT_LEN   = PM1_CNT_LENGTH;
    facp->PM2_CNT_LEN   = PM2_CNT_LENGTH;
    facp->PM_TM_LEN     = PM_TMR_LENGTH;
    
    facp->CST_CNT   = SW_SMI_CST_CNT;
    facp->P_LVL2_LAT    = P_LVL2_LAT_VAL;
    facp->P_LVL3_LAT    = P_LVL3_LAT_VAL;
    facp->FLUSH_SIZE    = FLUSH_SIZE_VAL;
    facp->FLUSH_STRIDE  = FLUSH_STRIDE_VAL;
    facp->DUTY_OFFSET   = DUTY_OFFSET_VAL;
    facp->DUTY_WIDTH    = DUTY_WIDTH_VAL; 
    facp->DAY_ALRM      = ACPI_ALARM_DAY_CMOS;
    facp->MON_ALRM      = ACPI_ALARM_MONTH_CMOS;
    facp->CENTURY       = ACPI_CENTURY_CMOS;
    facp->IAPC_BOOT_ARCH = ACPI_IA_BOOT_ARCH;
    
    //CSP20140424_23 >>
    GetEfiVariable(L"Setup",&gSetupGuid,NULL,&SetupSize,&SetupData);
    if ((SetupData->PciExpNative == 0) || (SetupData->NativeAspmEnable == 0))  
      facp->IAPC_BOOT_ARCH |= 0x10;
    //CSP20140424_23 <<
    
    //--------Filling Flags for V.1----------------------
    
    if (FACP_FLAG_WBINVD_FLUSH) facp->FLAGS |= 1<<1;
    
    if (FACP_FLAG_PROC_C1)      facp->FLAGS |= 1<<2;
    
    if (FACP_FLAG_P_LVL2_UP)    facp->FLAGS |= 1<<3;
    
    if (FACP_FLAG_RTC_S4)       facp->FLAGS |= 1<<7;
    
    if (FACP_FLAG_TMR_VAL_EXT)  facp->FLAGS |= 1<<8;
    
    if (FACP_FLAG_HEADLESS)         facp->FLAGS |= 1<<12;
    
    if (FACP_FLAG_CPU_SW_SLP)       facp->FLAGS |= 1<<13;
#if ACPI_BUILD_TABLES_3_0    
    
    if (FACP_FLAG_S4_RTC_STS_VALID)             facp->FLAGS |= 1<<16;
    
    if (FACP_FLAG_REMOTE_POWER_ON_CAPABLE)  	facp->FLAGS |= 1<<17;

    if (FACP_FLAG_PCI_EXP_WAK)      			facp->FLAGS |= 1<<14;
    
#endif //#if ACPI_BUILD_TABLES_3_0 
    
#endif //#if HW_REDUCED_ACPI == 0
    
    if (FACP_FLAG_WBINVD)       facp->FLAGS |= 1;
    
    if (FACP_FLAG_PWR_BUTTON)   facp->FLAGS |= 1<<4;
    
    if (FACP_FLAG_SLP_BUTTON)   facp->FLAGS |= 1<<5;
    
    if (FACP_FLAG_FIX_RTC)      facp->FLAGS |= 1<<6;
    
    if (FACP_FLAG_DCK_CAP)      facp->FLAGS |= 1<<9;
    
    
    //--------Filling Flags for V.2 and GAS compat structure for v.1----------------------
    
    if (FACP_FLAG_RESET_REG_SUP)    facp->FLAGS |= 1<<10;
    
    if (FACP_FLAG_SEALED_CASE)      facp->FLAGS |= 1<<11;
    

    
#if ACPI_BUILD_TABLES_3_0
    
    if (FACP_FLAG_USE_PLATFORM_CLOCK)                   facp->FLAGS |= 1<<15;
    
    if (TablVer > 2)
    {
        if (FACP_FLAG_FORCE_APIC_CLUSTER_MODEL)             facp->FLAGS |= 1<<18;
        
        if (FACP_FLAG_FORCE_APIC_PHYSICAL_DESTINATION_MODE) facp->FLAGS |= 1<<19;
        
    }
    
#endif
    
    // RESET_REG GAS_20 structure and value
    facp->RESET_REG.AddrSpcID   = ACPI_RESET_REG_TYPE;
    facp->RESET_REG.RegBitWidth = ACPI_RESET_REG_BITWIDTH;
    facp->RESET_REG.RegBitOffs  = ACPI_RESET_REG_BITOFFSET;
    facp->RESET_REG.Address     = ACPI_RESET_REG_ADDRESS;
    facp->RESET_VAL             = ACPI_RESET_REG_VALUE;
    
    if (ACPI_RESET_REG_ADDRESS)
    {
        // Set FACP flag
        facp->FLAGS |= 1<<10;
    }
    
    if (TablVer == 1)
    {
        facp->Header.Length     = 0x84;
        facp->Header.Checksum = 0;
        facp->Header.Checksum = ChsumTbl((UINT8*)facp, facp->Header.Length);
        return EFI_SUCCESS;
    }
    
    //--------This is all for V.1-----------------------
#if HW_REDUCED_ACPI == 0    
    // PM1a_EVT_BLK GAS_20 structure
    facp->X_PM1a_EVT_BLK.AddrSpcID  = PM1A_EVT_BLK_TYPE;
    facp->X_PM1a_EVT_BLK.RegBitWidth= PM1A_EVT_BLK_BITWIDTH;
    facp->X_PM1a_EVT_BLK.RegBitOffs = PM1A_EVT_BLK_BITOFFSET;
    facp->X_PM1a_EVT_BLK.AccessSize  = 2;
    facp->X_PM1a_EVT_BLK.Address    = PM1A_EVT_BLK_ADDRESS;
    
    // PM1a_CNT_BLK GAS_20 structure
    facp->X_PM1a_CNT_BLK.AddrSpcID  = PM1A_CNT_BLK_TYPE;
    facp->X_PM1a_CNT_BLK.RegBitWidth= PM1A_CNT_BLK_BITWIDTH;
    facp->X_PM1a_CNT_BLK.RegBitOffs = PM1A_CNT_BLK_BITOFFSET;
    facp->X_PM1a_CNT_BLK.AccessSize  = 2;
    facp->X_PM1a_CNT_BLK.Address    = PM1A_CNT_BLK_ADDRESS;
    
    // PM1b_EVT_BLK GAS_20 structure
    facp->X_PM1b_EVT_BLK.AddrSpcID  = PM1B_EVT_BLK_TYPE;
    facp->X_PM1b_EVT_BLK.RegBitWidth= PM1B_EVT_BLK_BITWIDTH;
    facp->X_PM1b_EVT_BLK.RegBitOffs = PM1B_EVT_BLK_BITOFFSET;
    facp->X_PM1b_EVT_BLK.AccessSize  = 2;
    facp->X_PM1b_EVT_BLK.Address    = PM1B_EVT_BLK_ADDRESS;
    
    // PM1b_CNT_BLK GAS_20 structure
    facp->X_PM1b_CNT_BLK.AddrSpcID  = PM1B_CNT_BLK_TYPE;
    facp->X_PM1b_CNT_BLK.RegBitWidth= PM1B_CNT_BLK_BITWIDTH;
    facp->X_PM1b_CNT_BLK.RegBitOffs = PM1B_CNT_BLK_BITOFFSET;
    facp->X_PM1b_CNT_BLK.AccessSize    = 2;
    facp->X_PM1b_CNT_BLK.Address    = PM1B_CNT_BLK_ADDRESS;
    
    // PM1b_CNT_BLK GAS_20 structure
    facp->X_PM2_CNT_BLK.AddrSpcID   = PM2_CNT_BLK_TYPE;
    facp->X_PM2_CNT_BLK.RegBitWidth = PM2_CNT_BLK_BITWIDTH;
    facp->X_PM2_CNT_BLK.RegBitOffs  = PM2_CNT_BLK_BITOFFSET;
    facp->X_PM2_CNT_BLK.AccessSize    = 1;
    facp->X_PM2_CNT_BLK.Address     = PM2_CNT_BLK_ADDRESS;
    
    facp->X_PM_TMR_BLK.AddrSpcID    = PM_TMR_BLK_TYPE;
    facp->X_PM_TMR_BLK.RegBitWidth  = PM_TMR_BLK_BITWIDTH;
    facp->X_PM_TMR_BLK.RegBitOffs   = PM_TMR_BLK_BITOFFSET;
    facp->X_PM_TMR_BLK.AccessSize    = 3;
    facp->X_PM_TMR_BLK.Address      = PM_TMR_BLK_ADDRESS;
    
    facp->X_GPE0_BLK.AddrSpcID      = GPE0_BLK_TYPE;
    facp->X_GPE0_BLK.RegBitWidth    = GPE0_BLK_BITWIDTH;
    facp->X_GPE0_BLK.RegBitOffs     = GPE0_BLK_BITOFFSET;
    facp->X_GPE0_BLK.AccessSize    = 1;
    facp->X_GPE0_BLK.Address        = GPE0_BLK_ADDRESS;
    
    facp->X_GPE1_BLK.AddrSpcID      = GPE1_BLK_TYPE;
    facp->X_GPE1_BLK.RegBitWidth    = GPE1_BLK_BITWIDTH;
    facp->X_GPE1_BLK.RegBitOffs     = GPE1_BLK_BITOFFSET;
    facp->X_GPE1_BLK.AccessSize    = 1;
    facp->X_GPE1_BLK.Address        = GPE1_BLK_ADDRESS;
#endif //#if HW_REDUCED_ACPI == 0   
    if (ACPI_BUILD_TABLES_5_0 == 1) 
    {
#if HW_REDUCED_ACPI      

        facp->FLAGS |= 1<<20;

        facp->SLEEP_CONTROL_REG.AddrSpcID   = SLEEP_CONTROL_REG_TYPE;
        facp->SLEEP_CONTROL_REG.RegBitWidth = SLEEP_CONTROL_REG_BITWIDTH;
        facp->SLEEP_CONTROL_REG.RegBitOffs  = SLEEP_CONTROL_REG_BITOFFSET;
        facp->SLEEP_CONTROL_REG.AccessSize  = SLEEP_CONTROL_REG_ACCESS_SIZE;
        facp->SLEEP_CONTROL_REG.Address     = SLEEP_CONTROL_REG_ADDRESS;

        facp->SLEEP_STATUS_REG.AddrSpcID    = SLEEP_STATUS_REG_TYPE;
        facp->SLEEP_STATUS_REG.RegBitWidth  = SLEEP_STATUS_REG_BITWIDTH;
        facp->SLEEP_STATUS_REG.RegBitOffs   = SLEEP_STATUS_REG_BITOFFSET;
        facp->SLEEP_STATUS_REG.AccessSize   = SLEEP_STATUS_REG_ACCESS_SIZE;
        facp->SLEEP_STATUS_REG.Address      = SLEEP_STATUS_REG_ADDRESS;
#endif
    
        if (LOW_POWER_S0_IDLE_CAPABLE)       facp->FLAGS |= 1<<21;
        facp->Header.Revision = 5; // ACPI 5.0 revision
    }
    facp->Header.Length     = SizeOfFacp;
    facp->Header.Checksum = 0;
    facp->Header.Checksum = ChsumTbl((UINT8*)facp, facp->Header.Length);
    
    return EFI_SUCCESS;
    
}// end of BuildFacpiAll


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   BuildIoapicIosapic
//
//  Description:
//  This function will probe IndReg and DataReg of discovered IOxAPIC base address
//  and allocate memory for and builds Ioapic/Iosapic entry for MADT table
//
//  Input:
//  IN UINT64 BaseAddr - Base Addrerss where IOxApic presence detected
//  IN OUT UINT64 *VecBase - pointer to the System Vector Base variable that
//                 needs to be updated with IOAPIC.MAXREDIR# by this function
//
//
//  Output:
//  MADT_ENTRY_HEADER * - pointer to the Ioapic/Iosapi entry header for the MADT table
//                        if NULL - not enough memory
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#pragma optimize( "", off )
MADT_ENTRY_HEADER *BuildIoapicIosapic(IN UINT32 BaseAddr, OUT UINT32 *VecBase)
{
    UINT32          Tmp;
    UINT32  volatile        *DatPtr;
    UINT8   volatile        *IdxPtr;
    UINT32          IoapicId;
    UINT32          IoapicVer;
    IOAPIC_H20       *IoapicPtr;
    IOSAPIC_H20      *IosapicPtr;
    
//==============================
    //Read IOAPIC.ID reg and figure out
    //1.How many INTINs it has
    //2.What ID did BIOS gave to it
    IdxPtr=(UINT8*)(BaseAddr + IOA_IND_REG_OFFS);
    DatPtr=(UINT32*)(BaseAddr + IOA_DAT_REG_OFFS);
    
    *IdxPtr = IOA_ID_IND;                   // bits 27..24=IOAPIC_ID
    
    Tmp = *DatPtr;
    
    IoapicId = ((Tmp&0x0f000000)>>24);
    
    *IdxPtr = IOA_VER_IND;                  // IOAPIC_VER register
    
    Tmp=*DatPtr;
    
    IoapicVer=(Tmp&0xff);
    
    if (IoapicVer < IO_APIC_VERSION_PARAMETER)  // This is Ioapic
    {
        IoapicPtr = MallocZ(sizeof(IOAPIC_H20));
        ASSERT (IoapicPtr);
        
        if (!IoapicPtr) return NULL;
        
        IoapicPtr->Header.Type = (UINT8) AT_IOAPIC;     // 1 - For IOAPIC Structure
        IoapicPtr->Header.Length = (UINT8) sizeof (IOAPIC_H20);
        IoapicPtr->IoapicAddress = (UINT32) BaseAddr;
        IoapicPtr->IoapicId = (UINT8) IoapicId;
        IoapicPtr->SysVectBase = (UINT32) *VecBase;
        //Adjust VecBase with current Max Redirection value
        (*VecBase)+=((UINT32)((Tmp&0x00ff0000)>>16))+1;
        return (MADT_ENTRY_HEADER*) IoapicPtr;
    }
    
    else  // This is Iosapic
    {
        IosapicPtr = MallocZ(sizeof(IOSAPIC_H20));
        ASSERT (IosapicPtr);
        
        if (!IosapicPtr) return NULL;
        
        IosapicPtr->Header.Type = (UINT8) AT_IOSAPIC;     // 6 - For IOSAPIC Structure
        IosapicPtr->Header.Length = (UINT8) sizeof (IOSAPIC_H20);
        IosapicPtr->IosapicAddress = (UINT64) BaseAddr;
        IosapicPtr->IoapicId = (UINT8) IoapicId;
        IosapicPtr->SysVectBase = (UINT32) *VecBase;
        //Adjust VecBase with current Max Redirection value
        (*VecBase)+=((UINT32)((Tmp&0x00ff0000)>>16))+1;
        return (MADT_ENTRY_HEADER*) IosapicPtr;
    }
    
}// End of BuildIoapicIosapic
#pragma optimize( "", on )

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   BuildIoapicManualy
//
//  Description: Allocates memory for and builds LAPIC entry for MADT table filled with
//               Dummy information
//
//  Output:
//  MADT_ENTRY_HEADER * - pointer to the LAPIC entry header for the MADT table
//                        if NULL - not enough memory
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

MADT_ENTRY_HEADER *BuildIoapicManualy()
{
    IOAPIC_H20       *IoapicPtr;
    
    IoapicPtr = MallocZ(sizeof(IOAPIC_H20));
    ASSERT (IoapicPtr);
    
    if (!IoapicPtr) return NULL;
    
    IoapicPtr->Header.Type = (UINT8) AT_IOAPIC;     // 1 - For IOAPIC Structure
    IoapicPtr->Header.Length = (UINT8) sizeof (IOAPIC_H20);
    IoapicPtr->IoapicAddress = (UINT32) APCB;
    IoapicPtr->IoapicId = (UINT8) 0;
    IoapicPtr->SysVectBase = (UINT32) 0;
    return (MADT_ENTRY_HEADER*) IoapicPtr;
    
    
}// end of BuildIoapicManualy

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AddIsoFromTbl
//
//  Description: Allocates memory for and builds ISO entry for MADT table, based
//               on INTERRUPT SOURCE OVERRIDE info from predifined table
//
//  Input: UINTN IsoTblNumb - Number of entry in ISO table for which an entry should
//                            be build.
//
//  Output:
//  MADT_ENTRY_HEADER * - pointer to the ISO entry header for the MADT table
//                        if NULL - not enough memory
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

MADT_ENTRY_HEADER *BuildIsoFromTbl (UINTN IsoTblNumb)
{
    ISO_H20     *IsoPtr;
    
    IsoPtr = MallocZ(sizeof(ISO_H20));
    ASSERT (IsoPtr);
    
    if (!IsoPtr) return NULL;
    
    IsoPtr->Header.Type = AT_ISO; // 2 - For ISO structure
    IsoPtr->Header.Length = sizeof(ISO_H20);
    IsoPtr->Bus = 0;
    IsoPtr->Source = (UINT8) IsoTbl[IsoTblNumb].PicIrq;
    IsoPtr->GlobalSysVect = (UINT32) IsoTbl[IsoTblNumb].ApicInt;
    IsoPtr->Flags = (UINT16) IsoTbl[IsoTblNumb].Flags;
    return (MADT_ENTRY_HEADER*) IsoPtr;
}//end of BuildIsoFromTbl

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   BuildMadtAll
//
//  Description:  Creates MADT entries, stores them as T_ITEM_LIST, calculates
//                space needed, allocates memory and builds MADT table from entries
//
//  Input:
//  IN UINTN TablVer - ACPI Spec. Version
//  OUT ACPI_HDR **TablPtr - Pointer, to the MADT table, filled by this function
//
//  Output: EFI_STATUS
//  EFI_OUT_OF_RESOURCES - not enough memory
//  EFI_UNSUPPORTED - CPU info hob not found
//  EFI_INVALID_PARAMETER - invalid ACPI version
//  EFI_SUCCESS - MADT table were successfully build
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS BuildMadtAll(IN UINTN TablVer, OUT ACPI_HDR **TablPtr)
{
    UINTN                       LastItem;
    EFI_STATUS                  Status;
    MADT_ENTRIES                MadtTblEntries = {0, 0, NULL};
    MADT_ENTRY_HEADER           *HdrPtr;
    UINT32                      IoapicVbase = 0, AllStrLength = 0, IoapicAddr = 0;
    UINT8                       *DestinPtr;
    UINTN                       i, j;
    UINTN                       IoApicCnt=0;

    VOID *CpuEntries;
    UINT8 *CpuEntry;
    UINT32 NumCpuEntries;
    UINT32 CpuEntriesSize;
    
//--------------------------------
    if (TablVer<1 || TablVer>4) return EFI_INVALID_PARAMETER;

    Status = CreateCpuMadtEntries((UINT32)TablVer, &CpuEntries, &NumCpuEntries, &CpuEntriesSize);

    //Add CPU MadtTblEntries
    if (!EFI_ERROR(Status)) {
        CpuEntry = (UINT8*)CpuEntries;
        while(NumCpuEntries--){
            UINT8 Length = ((MADT_ENTRY_HEADER*)CpuEntry)->Length;
            HdrPtr = (MADT_ENTRY_HEADER*)Malloc(Length);
            MemCpy(HdrPtr, CpuEntry, Length);
            AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);// First entry
            CpuEntry += Length;
        }
        pBS->FreePool(CpuEntries);
    }

    //Set Marker at last item before IOAPIC/IOsAPIC entries
    LastItem = MadtTblEntries.MadtEntCount;
    
//The folloving code may enable IOAPIC Devices found on PCI BUS
//This IOAPICs might use their ABARs to clame MMIO in FEC0_0000 space.
//So we need to run this code prior  if we don't want to miss any IOAPICs.
//--------------------------------------------------------------------
#if PCI_BUS_APIC_AUTODETECT == 1
    {
        EFI_HANDLE *pHandleBuffer;
        UINTN   NumberOfHandles;
        UINT8   PciData[4];
        EFI_GUID PciIoProtocolGuid = EFI_PCI_IO_PROTOCOL_GUID;
        EFI_PCI_IO_PROTOCOL *pPciIoProtocol;
        
        Status = pBS->LocateHandleBuffer(ByProtocol, &PciIoProtocolGuid,
                                         NULL, &NumberOfHandles, &pHandleBuffer);
                                         
        //The protocol might not be available when function runs first time.
        //we will rerun it on READY_TO_BOOT event again.
        if (!EFI_ERROR(Status))
        {
        
            for (i = 0; i < NumberOfHandles; i++)
            {
                Status = pBS->HandleProtocol(pHandleBuffer[i], &PciIoProtocolGuid,
                                             (VOID**)&pPciIoProtocol);
                                             
                if (EFI_ERROR(Status))
                    continue;
                    
                //read class code information at 0x8 offset in PCI header
                Status = pPciIoProtocol->Pci.Read(pPciIoProtocol, EfiPciIoWidthUint32,
                                                  0x8, 1, &PciData[0]);
                ASSERT_EFI_ERROR(Status);
                
                if (EFI_ERROR(Status)) //problem
                    continue;
                    
                //if IO APIC device
                if ((PciData[3] == 0x8) && (PciData[2] == 0) && (PciData[1] >= 0x10))
                {
                    UINT64  Attr=0, OldAttr=0;
                    //----------------------
                    //1. make sure it is Enabled and Decoding it's resources
                    Status=pPciIoProtocol->Attributes(pPciIoProtocol,EfiPciIoAttributeOperationGet, Attr, &OldAttr);
                    ASSERT_EFI_ERROR(Status);
                    
                    if (EFI_ERROR(Status)) continue;
                    
                    Status=pPciIoProtocol->Attributes(pPciIoProtocol,EfiPciIoAttributeOperationSupported, 0, &Attr);
                    ASSERT_EFI_ERROR(Status);
                    
                    if (EFI_ERROR(Status)) continue;
                    
                    Status=pPciIoProtocol->Attributes(pPciIoProtocol,EfiPciIoAttributeOperationSet, Attr&(EFI_PCI_DEVICE_ENABLE), NULL);
                    ASSERT_EFI_ERROR(Status);
                    
                    if (EFI_ERROR(Status)) continue;
                    
                    
                    //2. collect info
                    Status = pPciIoProtocol->Pci.Read(pPciIoProtocol, EfiPciIoWidthUint32,
                                                      0x10, 1, (VOID*)&IoapicAddr);
                    //problem or mapped to default address range
                    ASSERT_EFI_ERROR(Status);
                    
                    if ( ! (EFI_ERROR(Status) || (IoapicAddr == 0)) )
                    {
                        HdrPtr = BuildIoapicIosapic(IoapicAddr, &IoapicVbase);
                        
                        if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
                        
                        if (MadtTblEntries.MadtEntCount == LastItem)   // Adding entries in groving IOapicId/IOsapicId value order
                        {
                            Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);// First entry
                        }
                        
                        else
                        {
                            for (j = (LastItem); j < MadtTblEntries.MadtEntCount; j++)  // No need to handle LSAPIC entry in different way
                            {
                                // IOapicId and IOsapicId filds are on the same place in bouth structures
                                if (((IOAPIC_H32*)HdrPtr)->IoapicId < ((IOAPIC_H32*)MadtTblEntries.MadtEntries[j])->IoapicId) break;
                            }   // found entry with bigger IOapicId/IOsapicId
                            
                            if (j == MadtTblEntries.MadtEntCount)
                            {
                                Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
                            }
                            
                            else
                            {
                                Status = InsertItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr, j);
                            }
                            
                            ASSERT_EFI_ERROR(Status);
                            
                            if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
                            
                            IoApicCnt++;
                        }
                    }//if( ! (EFI_ERROR(Status) || (IoapicAddr == 0)) )
                    
#if PCI_BUS_APIC_LEAVE_ENABLE != 0
                    //Restore attributes of the device
                    Status=pPciIoProtocol->Attributes(pPciIoProtocol,EfiPciIoAttributeOperationSet, OldAttr, NULL);
                    ASSERT_EFI_ERROR(Status);
                    
                    if (EFI_ERROR(Status)) continue;
                    
#endif
                }//if((PciData[3] == 0x8) && (PciData[2] == 0) && (PciData[1] >= 0x10))
            }//for(i = 0; i < NumberOfHandles; i++)
            
            pBS->FreePool(pHandleBuffer);
        }//if(!EFI_ERROR(Status))
    }
#endif //PCI_BUS_APIC_AUTODETECT == 1
    
//--------------------------------------------------------------------
//---Creating IOAPIC or/and IOSAPIC Entries
#if (APCB != 0)
    IoapicAddr = APCB;
#else
    IoapicAddr = IOA_BASE_BOT;
#endif
    
#if FEC00000_APIC_AUTODETECT == 1
    
    //trying to check if something alive present at 0xFEC00000..0xFED00000
    for ( ; IoapicAddr < IOA_BASE_TOP; IoapicAddr += 0x1000)
    { //If so read IOAPIC.ID reg and figure out
        //1.How many INTINs it has
        //2.What ID BIOS give to it
        if (*((UINT8 *)(IoapicAddr + IOA_IND_REG_OFFS)) != 0xFF)
        {
            HdrPtr = BuildIoapicIosapic(IoapicAddr, &IoapicVbase);
            
            if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
            
            if (MadtTblEntries.MadtEntCount == LastItem)  // Adding entries in groving IOapicId/IOsapicId value order
                Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);// First entry
            else
            {
                for (j = (LastItem); j < MadtTblEntries.MadtEntCount; j++)
                { // No need to handle LSAPIC entry in different way
                    if (((IOAPIC_H32*)HdrPtr)->IoapicId < ((IOAPIC_H32*)MadtTblEntries.MadtEntries[j])->IoapicId) break; // IOapicId and IOsapicId filds are on the same place in bouth structures
                }   // found entry with bigger IOapicId/IOsapicId
                
                if (j == MadtTblEntries.MadtEntCount)
                {
                    Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
                }
                
                else
                {
                    Status = InsertItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr, j);
                }
            }
            
            ASSERT_EFI_ERROR(Status);
            
            if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
            
            IoApicCnt++;
        }
    }
    
#endif //FEC00000_APIC_AUTODETECT == 1
    
//---------------------------------------------------------------------
/*
#if AmiBoardInfo_SUPPORT != 0
#if USE_BOARD_INFO_APIC_DATA == 1
    {
        AMI_APIC_INFO   *BrdApicInfo;
        BOOLEAN         Present;
        
        //-----------------
        for (i=0; i<(gAmiBoardInfoProtocol->ApicInfoLength/sizeof(AMI_APIC_INFO)); i++)
        {
        
            BrdApicInfo=&gAmiBoardInfoProtocol->ApicInfoTable[i];
            
            //This is a special case it could be a duplicate entries in MadtTblEntries[]
            //if other options of IOAPIC/IOsAPIC detection was on.
            //So check if IOAPIC/IOsAPIC entries with the same properties already present in DB.
            for (j=(LastItem),Present=FALSE; j < MadtTblEntries.MadtEntCount; j++)
            {
                HdrPtr=MadtTblEntries.MadtEntries[j];
                
                if (HdrPtr->Type==AT_IOAPIC)
                {
                    IOAPIC_H20 *apic=(IOAPIC_H20*)HdrPtr;
                    
                    //----------------------
                    if (BrdApicInfo->ApicAddress.ADDRESS == apic->IoapicAddress)
                    {
                        Present = TRUE;
                        break;
                    }
                }
                
                else  //Nothing else should not be here except IOAPIC or IOSAPIC entries
                {
                    IOSAPIC_H20 *sapic=(IOSAPIC_H20*)HdrPtr;
                    
                    //----------------------
                    if ((UINT64)BrdApicInfo->ApicAddress.ADDRESS == sapic->IosapicAddress)
                    {
                        Present = TRUE;
                        break;
                    }
                }
            }
            
            //Entry with this address already present in MadtTblEntries array
            if (Present) continue;
            
            //Looks like it is a new IOAPIC/IOSAPIC entry!
            //Check if something alive at this address
            if (*((UINT8 *)BrdApicInfo->ApicAddress.ADDRESS) != 0xFF)
            {
                IoapicAddr=BrdApicInfo->ApicAddress.ADDRESS;
                HdrPtr = BuildIoapicIosapic(IoapicAddr, &IoapicVbase);
                
                if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
                
                if (MadtTblEntries.MadtEntCount == LastItem)  // Adding entries in groving IOapicId/IOsapicId value order
                    Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);// First entry
                else
                {
                    for (j = (LastItem); j < MadtTblEntries.MadtEntCount; j++)
                    { // No need to handle LSAPIC entry in different way
                        if (((IOAPIC_H32*)HdrPtr)->IoapicId < ((IOAPIC_H32*)MadtTblEntries.MadtEntries[j])->IoapicId) break; // IOapicId and IOsapicId filds are on the same place in bouth structures
                    }   // found entry with bigger IOapicId/IOsapicId
                    
                    if (j == MadtTblEntries.MadtEntCount)
                    {
                        Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
                    }
                    
                    else
                    {
                        Status = InsertItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr, j);
                    }
                }
                
                ASSERT_EFI_ERROR(Status);
                
                if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
                
                IoApicCnt++;
            }
        } // if(...i<(gAmiBoardInfoProtocol->ApicInfoLength/sizeof(AMI_APIC_INFO))
    }
#endif //USE_BOARD_INFO_APIC_DATA ==1
#endif //AmiBoardInfo_SUPPORT != 0
*/
//-------------------------------------------------------------------------------------------
    if (!IoApicCnt)
    {
        HdrPtr = BuildIoapicManualy();
        
        if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
        
        Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    }
    
    
//-------Build ISO Structure----------------------------
    for (i=0; i<IsoCnt; i++)
    {
        if (IsoTbl[i].PicIrq == 0xFF) continue; //no override for this entry
        
        HdrPtr = BuildIsoFromTbl(i);
        
        if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
        
        Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return Status;
    }
    
#if (NMIs_QUANTITY > 0)
    HdrPtr = MallocZ(sizeof(NMI_H20));
    ASSERT (HdrPtr);
    
    if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
    
    ((NMI_H20*) HdrPtr)->Header.Type = (UINT8) AT_NMI;       //Type 3 - indicating NMIs Entry
    ((NMI_H20*) HdrPtr)->Header.Length = (UINT8) sizeof(NMI_H20);
    ((NMI_H20*) HdrPtr)->Flags = (UINT16)((NMI_0_TRIGGER_MODE<<2) | NMI_0_POLARITY);
    ((NMI_H20*) HdrPtr)->GlobalSysVect = (UINT32) NMI_GLOBAL_SYS_INT_0;
    Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return Status;
    
#if (NMIs_QUANTITY > 1)
    HdrPtr = MallocZ(sizeof(NMIH_20));
    ASSERT (HdrPtr);
    
    if (!HdrPtr) return EFI_OUT_OF_RESOURCES;
    
    ((NMI_H20*) HdrPtr)->Header.Type = (UINT8) AT_NMI;       //Type 3 - indicating NMIs Entry
    ((NMI_H20*) HdrPtr)->Header.Length = (UINT8) sizeof(NMI_H20);
    ((NMI_H20*) HdrPtr)->Flags = (UINT16)((NMI_1_TRIGGER_MODE<<2) | NMI_1_POLARITY);
    ((NMI_H20*) HdrPtr)->GlobalSysVect = (UINT32) NMI_GLOBAL_SYS_INT_1;
    Status = AppendItemLst ((T_ITEM_LIST*)&MadtTblEntries, (VOID*) HdrPtr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return Status;
#endif
    
//---Porting Hook 1 ------ If NMIs_QUANTITY > 2 - Add more structures
#endif

    //---Tis ia all for Ver 1 - building a table----------------------
//----Here starts entries for V 2 and 3 which are empty for now----------------------------------
//
//          Here entries for LAPIC Address overrride structure and Platforme interrupt source structure
//

    for (i = 0; i < MadtTblEntries.MadtEntCount; i++)
    {
    
        AllStrLength += MadtTblEntries.MadtEntries[i]->Length;
    }
    
    *TablPtr = MallocZ(sizeof(APIC_20H) + (UINTN) AllStrLength);
    ASSERT (*TablPtr);
    
    if (!(*TablPtr)) return EFI_OUT_OF_RESOURCES;
    
    if (TablVer == 1) PrepareHdr1 (APIC_SIG, (PACPI_HDR) *TablPtr);
    else PrepareHdr20 (APIC_SIG, (PACPI_HDR) *TablPtr, TablVer);
    
    (*TablPtr)->Length = (sizeof(APIC_20H) + AllStrLength);
    ((APIC_20H*)*TablPtr)->LAPIC_Address    = LOCAL_APIC_BASE;
    ((APIC_20H*)*TablPtr)->Flags    = ACPI_APIC_FLAGS;
    DestinPtr = ((UINT8*) *TablPtr + sizeof(APIC_20H));
    
    for (i = 0; i < MadtTblEntries.MadtEntCount; i++)
    {
    
        MemCopy ((UINT8*)MadtTblEntries.MadtEntries[i], DestinPtr, (UINT32) MadtTblEntries.MadtEntries[i]->Length);
        DestinPtr += MadtTblEntries.MadtEntries[i]->Length;
        pBS->FreePool((VOID*)MadtTblEntries.MadtEntries[i]);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return Status;
    }
    
    (*TablPtr)->Checksum = 0;
    (*TablPtr)->Checksum = ChsumTbl((UINT8*)*TablPtr, (*TablPtr)->Length);
    
    return EFI_SUCCESS;
}// end of BuildMadtAll-----------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   BuildFacs
//
//  Description:  Allocates ACPI NVS memory and builds FACS table from values,
//                defined by SDL tokens
//
//  Input: none
//
//
//  Output: EFI_STATUS
//  EFI_OUT_OF_RESOURCES - not enough memory
//  EFI_SUCCESS - FACS table were successfully build
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS BuildFacs ()
{
    EFI_STATUS      Status;
    UINTN           Size;
    
    
    if (gForceAcpi1) Size = sizeof(FACS_20);
    else Size = sizeof(FACS_20)*2;
    Status = pBS->AllocatePool(EfiACPIMemoryNVS, Size+64, &gFacs);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    

#ifdef EFIx64
    if (((UINT64)gFacs) & 0xffffffff00000000)
    {
        EFI_PHYSICAL_ADDRESS Memory = 0x00000000ffffffff;
        Status = pBS->FreePool(gFacs);
        ASSERT_EFI_ERROR(Status);
        Status = pBS->AllocatePages(AllocateMaxAddress, EfiACPIMemoryNVS, 1, &Memory);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
        
        (UINTN) gFacs = Memory;
    }
#endif

    (UINTN) gFacs += 64;
    (UINTN) gFacs &= (~0x3F);
    pBS->SetMem(gFacs, Size, 0);
    gFacs->Signature=(UINT32)FACS_SIG;
    gFacs->Length=sizeof(FACS_20);
    gFacs->Flags=FACS_FLAG_S4BIOS;

    if (gForceAcpi1) return EFI_SUCCESS;
    
    (UINTN) gxFacs = (UINTN) gFacs + sizeof(FACS_20);
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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   LocateACPITableProtocol
//
//  Description:
//  This function locates the first instance of a protocol that contains the ACPI table
//  storage file.
//
//  Input:
//  IN EFI_GUID *Protocol - The protocol to find.
//  OUT VOID **Instance - Return pointer to the first instance of the protocol
//
//  Output:
//  EFI_SUCCESS           The function completed successfully.
//  EFI_NOT_FOUND         The protocol could not be located.
//  EFI_OUT_OF_RESOURCES  There are not enough resources to find the protocol.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS LocateACPITableProtocol(IN EFI_GUID *Protocol, OUT VOID **Instance, IN UINT8 Version)
{
    EFI_STATUS              Status;
    EFI_HANDLE              *HandleBuffer;
    UINTN                   NumberOfHandles;
    EFI_FV_FILETYPE         FileType;
    UINT32                  FvStatus=0;
    EFI_FV_FILE_ATTRIBUTES  Attributes;
    UINTN                   Size;
    UINTN                   i;
//--------------------------------------------
    //Locate protocol.
    Status = pBS->LocateHandleBuffer(ByProtocol,Protocol,NULL,&NumberOfHandles,&HandleBuffer);
    
    if (EFI_ERROR (Status)) return Status;
    
    //Look for FV with ACPI storage file
    for (i = 0; i < NumberOfHandles; i++)
    {
        // Get the protocol on this handle
        Status = pBS->HandleProtocol(HandleBuffer[i],Protocol,Instance);
        
        if (EFI_ERROR(Status))return Status;
        
        if (Version == 1)
        {
            Status = ((EFI_FIRMWARE_VOLUME_PROTOCOL*)(*Instance))->ReadFile(*Instance,
                     &gDsdt11Guid,NULL,&Size,&FileType,&Attributes,&FvStatus);
                     
            if (Status == EFI_SUCCESS) break;
        }
        
        else
        {
            Status = ((EFI_FIRMWARE_VOLUME_PROTOCOL*)(*Instance))->ReadFile(*Instance,
                     &gDsdt20Guid,NULL,&Size,&FileType,&Attributes,&FvStatus);
                     
            if (Status == EFI_SUCCESS) break;
        }
    }
    
    pBS->FreePool (HandleBuffer);
    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   GetDsdtFv
//
//  Description:
//  This function finds DSDT table in firmvare volume
//
//  Input:
//  OUT ACPI_HDR **Dsdt1 - pointer to memory where DSDT v1.1 table will be stored
//  OUT ACPI_HDR **Dsdt2 - pointer to memory where DSDT v2.0+ table will be stored
//
//  Output:
//  EFI_SUCCESS - Function executed successfully
//  EFI_ABORTED - ACPI storage file not found
//  EFI_NOT_FOUND - DSDT table not found in firmware volume
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetDsdtFv(OUT ACPI_HDR **Dsdt1, OUT ACPI_HDR **Dsdt2)
{
	EFI_STATUS	Status;
	
    //In current AmiBoardInfo implementation separate instance of
    //DSDT for ACPI version 1.1b  DOES OT SUPPORTED!
	Status=AmiSdlInitBoardInfo();
	if(EFI_ERROR(Status)){
		TRACE((-1,"AcpiCore: No AmiBoardInfo Found: Status=%r\n", Status));
	}

    *Dsdt1=NULL;
    *Dsdt2 = Malloc(((ACPI_HDR*)gAmiBoardInfo2Protocol->BrdAcpiInfo)->Length);
    ASSERT(*Dsdt2);
    if (((UINT64)*Dsdt2) & 0xffffffff00000000)
    {
    	EFI_PHYSICAL_ADDRESS Memory = 0x00000000ffffffff;
        
        Status = pBS->FreePool(*Dsdt2);
        ASSERT_EFI_ERROR(Status);
        
        Status = pBS->AllocatePages(AllocateMaxAddress, EfiBootServicesData, 
        							EFI_SIZE_TO_PAGES(((ACPI_HDR*)gAmiBoardInfo2Protocol->BrdAcpiInfo)->Length), &Memory);
        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
        
        *Dsdt2 = (ACPI_HDR *)((UINTN)Memory);
    }
    
    if (*Dsdt2==NULL) return EFI_OUT_OF_RESOURCES;
    
    pBS->CopyMem(*Dsdt2, gAmiBoardInfo2Protocol->BrdAcpiInfo,
                 ((ACPI_HDR*)gAmiBoardInfo2Protocol->BrdAcpiInfo)->Length);

    
    
    if ((*Dsdt1 == NULL) && (*Dsdt2 == NULL))
    {
        TRACE((-1,"Acpi: No DSDT was FOUND: Status=EFI_NOT_FOUND\n"));
        return EFI_NOT_FOUND;
    }
    
    PrepareHdr1(DSDT_SIG, (ACPI_HDR*)(*Dsdt1));
    PrepareHdr20(DSDT_SIG, (ACPI_HDR*)(*Dsdt2),2);
    
    return EFI_SUCCESS;
    
}// end of GetDsdtFv -----------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateFacp
//
// Description: This function Updates FACP with the new values for DSDT and Facs
//              pointers
//
// Input:       FACS_20* gFacs1 - Pointer to gFacs Table for V 1
//              FACS_20* gFacs2 - Pointer to gFacs Table for V 2 or 3
//
// Output:      EFI_SUCCESS - Function executed successfully
//              EFI_ABORTED - Error
//
//
//              Modifies  gAcpiData
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS UpdateFacp () //(FACS_20 *gFacs1, FACS_20 *gFacs2)
{
    ACPI_HDR    *Facp1 = NULL, *Facp2 = NULL, *Dsdt1 = NULL, *Dsdt2 = NULL;
    UINTN       i;
    EFI_STATUS  Status = EFI_SUCCESS;
    
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG)
        {
            if (gAcpiData.AcpiEntries[i]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)
                Facp1 = gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find FACP for V 1.1
            else
                Facp2 = gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find FACP for V 2+
        }
        
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG)
        {
            if (gAcpiData.AcpiEntries[i]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)
                Dsdt1 = gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find DSDT for V 1.1
            else
                Dsdt2 = gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find DSDT for V 2+
        }
    }
    
    if (Dsdt1 == NULL) Dsdt1 = Dsdt2;  // The same DSDT for V1.1
    
    if (Dsdt2 == NULL) Dsdt2 = Dsdt1;  // The same DSDT for V2
    
    if ((Facp1 == NULL) || (Dsdt1 == NULL)) Status = EFI_ABORTED;
    if ((Facp2 == NULL) && (!gForceAcpi1)) Status = EFI_ABORTED;
    
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR (Status)) return Status;
    
//--- Updating FACP for V1.1 and 2+---------------------------
    ((FACP32*) Facp1)->FIRMWARE_CTRL = (UINT32) gFacs;
    ((FACP32*) Facp1)->DSDT = (UINT32) Dsdt1;
    

    Facp1->Checksum = 0;
    Facp1->Checksum = ChsumTbl((UINT8*)Facp1, Facp1->Length);
    if (gForceAcpi1) return Status;
    if (ACPI_BUILD_TABLES_4_0 == 1) ((FACP_20*) Facp2)->FIRMWARE_CTRL = (UINT32) gxFacs;
    else ((FACP_20*) Facp2)->FIRMWARE_CTRL = (UINT32) gFacs;
    
    ((FACP_20*) Facp2)->DSDT = (UINT32) Dsdt1;
    
    if (ACPI_BUILD_TABLES_4_0 != 1)((FACP_20*) Facp2)->X_FIRMWARE_CTRL = (UINT64) ((UINTN) gxFacs);
    
    ((FACP_20*) Facp2)->X_DSDT = (UINT64) ((UINTN)Dsdt2);
    Facp2->Checksum = 0;
    Facp2->Checksum = ChsumTbl((UINT8*)Facp2, Facp2->Length);
    return Status;
}// end of UpdateFacp

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindAcpiTblByHandle
//
// Description: Finds ACPI table by Handle and returns its entry number in
//              gAcpiData structure
//
// Input:       UINTN *Handle - Handle (pointer to ACPI table header)
//
//
// Output:      UINTN - Entry number in gAcpiData structure
//                      ACPI_TABLE_NOT_FOUND if not found
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>
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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetBtTable
//
// Description: Founds ACPI Table with defined Signature (Sig) and Version in gAcpiData structure.
//
//
// Input:       OUT ACPI_TBL_ITEM **TblPtr - pointer to the ACPI entrie in gAcpiData,
//                                           modifided by this function
//              IN UINT32 Sig - Signature of a table to search for
//              IN EFI_ACPI_TABLE_VERSION - Version of a table to be found
//
// Output:      EFI_SUCCESS - table with corresponding signature was found
//              EFI_NOT_FOUND - otherwise
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>
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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DispatchSdtNotify
//
// Description: Checks if Notify function(s) present dispatches it.
//
//
// Input:       EFI_ACPI_SDT_HEADER *TableHeaderPtr - pointer to the ACPI Table header,
//                                           modifided by this function
// Output:      EFI_STATAUS  - From the function dispatched.
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DispatchSdtNotify(ACPI_TBL_ITEM* TableItemPtr){
    EFI_STATUS  Status=EFI_SUCCESS;
    UINTN   i;
//--------
    for(i=0;i<gAcpiData.NotifyFnCount; i++){
        Status=gAcpiData.AcpiNotifyFn[i]((EFI_ACPI_SDT_HEADER*)TableItemPtr->BtHeaderPtr,TableItemPtr->AcpiTblVer,(UINTN)TableItemPtr);        
        
        ASSERT_EFI_ERROR(Status);        
    }    

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FillAcpiStr
//
// Description: Copys table from position pointed by FromPtr to a position pointed
//              by ToPtr (which is in EfiACPIReclaimMemory) and fills RSDT and XSDT
//              pointers with ToPtr value
//
//
// Input:       RSDT32 *RsdtPtr - pionter to RSDT
//              XSDT_20 *XsdtPtr - pionter to XSDT
//              VOID *FromPtr - pointer to a table which should be copyed
//              VOID *ToPtr - pointer to EfiACPIReclaimMemory where table should be placed
//
// Output:      UINT8* - Pointer to the next avaiable space in allocated EfiACPIReclaimMemory
//                       right after copyed table (alligned already)
//              If NUUL - Invalid parameters.
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>

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
    (UINTN) NextPtr &= (~0x7);
    
    return NextPtr;
    
}//end of FillAcpiStr

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PublishTbl
//
// Description: Creates or rewrites RSDT_PTR structure and copys tables, stored
//              in gAcpiData structure in allocated EfiACPIReclaimMemory.
//
//
// Input:       IN UINTN RsdtBuild - if 1 - Build RSDT and copy tables of Ver 1.1
//              IN UINTN XsdtBuild - if 1 - Build XSDT and copy tables of Ver 2+
//
// Output:      EFI_OUT_OF_RESOURCES - not enough memory
//              EFI_ABORTED - invalid parameters
//              EFI_SUCCESS - RSDT_PTR structure was successfully build
//
//-----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS PublishTbl (IN UINTN RsdtBuild, IN UINTN XsdtBuild)
{
    ACPI_HDR    *Facp = NULL, *FacpX = NULL, *Dsdt = NULL, *DsdtX = NULL;
    RSDT32      *Rsdt = NULL;
    XSDT_20     *Xsdt = NULL;
    UINTN       i, j, RsdtCnt = 0, XsdtCnt = 0, SpaceNeeded = 0, RsdtSize = 0, XsdtSize = 0, Skip;
    EFI_STATUS  Status;
    UINT8       *Ptr = NULL, *Dummy = NULL, OneDsdt = 0;
    EFI_PHYSICAL_ADDRESS Memory = 0x00000000ffffffff;
    
    if (((RsdtBuild == 0) && (XsdtBuild == 0)) || (RsdtBuild > 1 ) || (XsdtBuild > 1)) return EFI_ABORTED;
#if defined(OemActivation_SUPPORT) && (OemActivation_SUPPORT == 1)
    if (!gOA3Variable)
    {
        UINTN SizeOfMsdm = sizeof(EFI_OA3_MSDM_STRUCTURE);
        //Looking for Var which signals that MSDM table may be updated on runtime
        Status = pRS->GetVariable(
            EFI_OA3_MSDM_VARIABLE,
            &gAmiGlobalVariableGuid,
            NULL,
            &SizeOfMsdm,
            &gMsdmVariable
        );
        if (Status == EFI_SUCCESS) 
        {
            gOA3Variable = TRUE;
            XsdtCnt++; // Reserving Entry for MSDM (OEM Activation)
        }
    }
    else XsdtCnt++;
#endif
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if (gAcpiData.AcpiEntries[i]->AcpiTblVer < EFI_ACPI_TABLE_VERSION_1_0B) continue;
        
        if (gAcpiData.AcpiEntries[i]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)
        {
            RsdtCnt ++;
            
            if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG)
                Facp = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
                
            if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG)
                Dsdt = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
        }
        
        if (gAcpiData.AcpiEntries[i]->AcpiTblVer > EFI_ACPI_TABLE_VERSION_1_0B)
        {
            XsdtCnt ++;
            RsdtCnt ++;
            
            if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG)
            {
                FacpX = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
                //if (Facp != NULL) RsdtCnt --;
                //continue;
            }
            
            if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG)
            {
                DsdtX = gAcpiData.AcpiEntries[i]->BtHeaderPtr;
            }
            
            for (j = 0; j < gAcpiData.AcpiEntCount; j++)
            {
                if (j == i) continue;
                
                if ((gAcpiData.AcpiEntries[j]->BtHeaderPtr->Signature == gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature)
                        && (gAcpiData.AcpiEntries[j]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)) RsdtCnt --;
            }
        }
    }
    
    if ((Dsdt == NULL) || (DsdtX == NULL)) OneDsdt = 1;
    
    if (Dsdt == NULL) Dsdt = DsdtX;
    
    if (DsdtX == NULL) DsdtX = Dsdt;
    
    RsdtSize = RsdtBuild * (sizeof(ACPI_HDR) + (RsdtCnt-1) * sizeof(UINT32) + 7); //DSDT does not goes in RSDT (RsdtCnt-1)
    
    XsdtSize = XsdtBuild * (sizeof(ACPI_HDR) + (XsdtCnt-1) * sizeof(UINT64) + 7); //DSDT does not goes in XSDT (XsdtCnt-1)
    SpaceNeeded = sizeof(RSDT_PTR_20) + RsdtSize + XsdtSize + gAcpiData.AcpiLength + (gAcpiData.AcpiEntCount + 1) * 8;
    
    if (gRsdtPtrStr != NULL) 
    {
        Status = pBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN) gRsdtPtrStr, gAcpiTblPages);
        ASSERT_EFI_ERROR(Status);
    }
    /*
        Status = pBS->AllocatePool(EfiACPIReclaimMemory, SpaceNeeded, (VOID**)&gRsdtPtrStr);
        ASSERT_EFI_ERROR(Status);
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    */
    gAcpiTblPages = EFI_SIZE_TO_PAGES(SpaceNeeded);
#if defined(OemActivation_SUPPORT) && (OemActivation_SUPPORT == 1)
    if (gOA3Variable)
        Status = pBS->AllocatePages(AllocateMaxAddress, EfiACPIMemoryNVS, gAcpiTblPages , &Memory);
    else
#endif
    Status = pBS->AllocatePages(AllocateMaxAddress, EfiACPIReclaimMemory, gAcpiTblPages , &Memory);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    
    gRsdtPtrStr = (RSDT_PTR_20*)Memory;
    
    pBS->SetMem(gRsdtPtrStr, SpaceNeeded, 0);
    
    if (RsdtBuild)
    {
        Dummy = (UINT8*) ((UINT8*)gRsdtPtrStr + sizeof(RSDT_PTR_20) + 7);
        (UINTN) Dummy &= (~0x7);
        Rsdt = (RSDT32*) Dummy;
        Ptr = (UINT8*)gRsdtPtrStr + sizeof(RSDT_PTR_20) + RsdtSize + XsdtSize + 7;
        (UINTN) Ptr &= (~0x7);
        Dummy = FillAcpiStr (Rsdt, NULL, (VOID*) Facp, (VOID*) Ptr);// The first table in RSDT must be Facp
        Facp = (ACPI_HDR*) Ptr;
        Ptr = Dummy;
        pBS->CopyMem((VOID*)Ptr, (VOID*)Dsdt, Dsdt->Length);
        
        if (Dsdt == DsdtX) Dsdt = DsdtX = (ACPI_HDR*) Ptr;
        else Dsdt = (ACPI_HDR*) Ptr;
        
        Ptr += (Dsdt->Length + 7);
        (UINTN) Ptr &= (~0x7);
        ((FACP32*)Facp)->FIRMWARE_CTRL = (UINT32) gFacs;
        ((FACP32*)Facp)->DSDT = (UINT32) Dsdt;
        Facp->Checksum = 0;
        Facp->Checksum = ChsumTbl((UINT8*)Facp, Facp->Length);
    }
    
    if ((XsdtBuild) && (!gForceAcpi1))
    {
        Dummy = (UINT8*) ((UINT8*)gRsdtPtrStr + sizeof(RSDT_PTR_20) + RsdtSize + 7);
        (UINTN) Dummy &= (~0x7);
        Xsdt = (XSDT_20*) Dummy;
        
        if (Ptr == NULL)
        {
            Ptr = (UINT8*) ((UINT8*)gRsdtPtrStr + sizeof(RSDT_PTR_20) + RsdtSize + XsdtSize + 7);
            //else Ptr += (((ACPI_HDR*)Ptr)->Length + 7);
            (UINTN) Ptr &= (~0x7);
        }
        
        Dummy = FillAcpiStr (NULL, Xsdt, (VOID*) FacpX,( VOID*) Ptr);
        FacpX = (ACPI_HDR*) Ptr;
        Ptr = Dummy;
        
        if ((!OneDsdt) || (!RsdtBuild))
        {
            pBS->CopyMem((VOID*)Ptr, (VOID*)DsdtX, DsdtX->Length);
            DsdtX = (ACPI_HDR*) Ptr;
            Ptr += (DsdtX->Length + 7);
            (UINTN) Ptr &= (~0x7);
        }
        
        if (ACPI_BUILD_TABLES_4_0 == 1) ((FACP_20*)FacpX)->FIRMWARE_CTRL = (UINT32) gxFacs;
        else ((FACP_20*)FacpX)->FIRMWARE_CTRL = (UINT32) gFacs;
        
        ((FACP_20*)FacpX)->DSDT = (UINT32) Dsdt;
        
        if (ACPI_BUILD_TABLES_4_0 != 1)((FACP_20*)FacpX)->X_FIRMWARE_CTRL = (UINT64) ((UINTN)gxFacs);
        
        ((FACP_20*)FacpX)->X_DSDT = (UINT64) ((UINTN)DsdtX);
        FacpX->Checksum = 0;
        FacpX->Checksum = ChsumTbl((UINT8*)FacpX, FacpX->Length);
    }
    
    for (i = 0; i < gAcpiData.AcpiEntCount; i++)
    {
        if ((gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG) ||
                (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG) ||
                (gAcpiData.AcpiEntries[i]->AcpiTblVer < EFI_ACPI_TABLE_VERSION_1_0B)) continue;
                
        if (gAcpiData.AcpiEntries[i]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)
        {
            Dummy = FillAcpiStr (Rsdt, NULL, (VOID*) gAcpiData.AcpiEntries[i]->BtHeaderPtr,( VOID*) Ptr);
            
            if (Dummy != NULL) Ptr = Dummy;
        }
        
        else
        {
            Skip = 0;
            
            for (j = 0; j < gAcpiData.AcpiEntCount; j++)
            {
                if (j == i) continue;
                
                if ((gAcpiData.AcpiEntries[j]->BtHeaderPtr->Signature == gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature)
                        && (gAcpiData.AcpiEntries[j]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)) Skip = 1;
            }
            
            if (Skip) Dummy = FillAcpiStr (NULL, Xsdt, (VOID*) gAcpiData.AcpiEntries[i]->BtHeaderPtr,( VOID*) Ptr);
            else Dummy = FillAcpiStr (Rsdt, Xsdt, (VOID*) gAcpiData.AcpiEntries[i]->BtHeaderPtr,( VOID*) Ptr);
            
            if (Dummy != NULL) Ptr = Dummy;
        }
    }
    
    if (Ptr > ((UINT8*)gRsdtPtrStr + SpaceNeeded))  Status = EFI_OUT_OF_RESOURCES;
    
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return Status;
    
    gRsdtPtrStr->Signature = RSDP_SIG;
    
    if ((!XsdtBuild) || (gForceAcpi1)) gRsdtPtrStr->Revision = 0; //Reserved in ver 1
    else  gRsdtPtrStr->Revision = ACPI_REV2; // 2 for ver 2 and 3
    
    gRsdtPtrStr->Length = sizeof(RSDT_PTR_20);//this is the length of entire structure
    
    for (i=0; i<6; i++) gRsdtPtrStr->OEMID[i]=ACPI_OEM_ID[i];
    
    gRsdtPtrStr->RsdtAddr = (UINT32) Rsdt;
    gRsdtPtrStr->XsdtAddr = (UINT64) ((UINTN)Xsdt);
    gRsdtPtrStr->Checksum = ChsumTbl((UINT8*)gRsdtPtrStr, 20);
    gRsdtPtrStr->XtdChecksum = ChsumTbl((UINT8*)gRsdtPtrStr, sizeof (RSDT_PTR_20));
    
    if (RsdtBuild)
    {
        PrepareHdr1 (RSDT_SIG, (ACPI_HDR*) Rsdt);
        Rsdt->Header.Length = (UINT32) (RsdtSize - 7);
        Rsdt->Header.Checksum = 0;
        Rsdt->Header.Checksum = ChsumTbl((UINT8*)Rsdt, Rsdt->Header.Length);
        Status = pBS->InstallConfigurationTable(&gAcpi11TAbleGuid, (VOID*) gRsdtPtrStr);
        TRACE((-1,"Installing ACPI 1.1: %r, %X\n",Status,gRsdtPtrStr));////-----------------------------------------
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return Status;
    }
    
    if (XsdtBuild)
    {
        if (ACPI_BUILD_TABLES_3_0 == 1) PrepareHdr20 (XSDT_SIG, (ACPI_HDR*) Xsdt, 3);
        else PrepareHdr20 (XSDT_SIG, (ACPI_HDR*) Xsdt, 2);
#if defined(OemActivation_SUPPORT) && (OemActivation_SUPPORT == 1)
        if (gOA3Variable)
            Xsdt->Header.Length = (UINT32) (XsdtSize - sizeof(UINT64) - 7);//one reserved position for MSDM
        else
#endif        
        Xsdt->Header.Length = (UINT32) (XsdtSize - 7);
        Xsdt->Header.Checksum = 0;
        Xsdt->Header.Checksum = ChsumTbl((UINT8*)Xsdt, Xsdt->Header.Length);
//    if (RsdtBuild) Status = pBS->InstallConfigurationTable(&gAcpi11TAbleGuid, NULL); // Delete V1.1 entry - Only one
        Status = pBS->InstallConfigurationTable(&gAcpi20TAbleGuid, (VOID*) gRsdtPtrStr);
    }
    
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return Status;
#if defined(OemActivation_SUPPORT) && (OemActivation_SUPPORT == 1)
    if ((Xsdt != NULL) && gOA3Variable)
    {
        gMsdmVariable.XsdtAddress = (EFI_PHYSICAL_ADDRESS) Xsdt;
        Status = pRS->SetVariable(
            EFI_OA3_MSDM_VARIABLE,
            &gAmiGlobalVariableGuid,
            EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
            sizeof(EFI_OA3_MSDM_STRUCTURE),
            &gMsdmVariable
        );
        ASSERT_EFI_ERROR(Status);
    }
#endif
    return EFI_SUCCESS;
    
}// end of PublishTbl


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   UpdateAml
//
//  Description:
//  This function updates AML objects with values provided in ACPI_AML_UPD_INFO
//  structure
//
//  Input:
//  IN ACPI_AML_UPD_INFO *AmlUpdInfo - pointer to ACPI_AML_UPD_INFO structure
//
//  Output:
//  EFI_SUCCESS - AML objects updated successfully
//  EFI_ERROR - some error occured during update process
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS UpdateAml(ACPI_AML_UPD_INFO  *AmlUpdInfo)
{
    UINTN           i;
    ACPI_HDR    *Dsdt;
    EFI_STATUS      Status = 0;
    UINT8           *SxPointer = NULL;
    
//-------------------------

    for (i = 0; i < 2; i++)
    {
        if (i) Dsdt = (ACPI_HDR*)AmlUpdInfo->Dsdt1Addr;
        else Dsdt = (ACPI_HDR*)AmlUpdInfo->Dsdt2Addr;
        
        if (Dsdt)
        {
        	if (AmlUpdInfo->SS3 == 0)
        	{
        		SxPointer=FindAslObjectName((UINT8*)Dsdt+sizeof(ACPI_HDR),"_S3",Dsdt->Length-sizeof(ACPI_HDR)-1);
            	if (SxPointer!=NULL) SxPointer[0]='X';
        	}
        	if (AmlUpdInfo->SS1 == 0)
        	{
        		SxPointer=FindAslObjectName((UINT8*)Dsdt+sizeof(ACPI_HDR),"_S1",Dsdt->Length-sizeof(ACPI_HDR)-1);
        	    if (SxPointer!=NULL) SxPointer[0]='X';
        	}
        	if (AmlUpdInfo->SS4 == 0)
        	{
        		SxPointer=FindAslObjectName((UINT8*)Dsdt+sizeof(ACPI_HDR),"_S4",Dsdt->Length-sizeof(ACPI_HDR)-1);
        	    if (SxPointer!=NULL) SxPointer[0]='X';
        	}
            //Update TOPM Object
            Status = UpdateAslNameObject(Dsdt, "TOPM", AmlUpdInfo->TopOfMemory);
            ASSERT_EFI_ERROR (Status)
            
            if (EFI_ERROR(Status)) return Status;
            
            //Update ROMS Object
            //Status = UpdateNameObject(Dsdt, AML_NAME_ROMS, AmlUpdInfo->RomStart);
            //ASSERT_EFI_ERROR(Status)
            //if(EFI_ERROR(Status)) return Status;
//TODO: IOST and SSx Objects update
            //Update IOST Object
            Status = UpdateAslNameObject(Dsdt, "IOST", (UINT64)AmlUpdInfo->SioDevStatusVar.DEV_STATUS);
            ASSERT_EFI_ERROR(Status)
            
            if (EFI_ERROR(Status)) return Status;

            Status = UpdateAslNameObject(Dsdt, "SS4", (UINT64)AmlUpdInfo->SS4);
            ASSERT_EFI_ERROR(Status)
            
            if (EFI_ERROR(Status)) return Status;
            
            Status = UpdateAslNameObject(Dsdt, "SS3", (UINT64)AmlUpdInfo->SS3);
            ASSERT_EFI_ERROR(Status)
            
            if (EFI_ERROR(Status)) return Status;
            
            Status = UpdateAslNameObject(Dsdt, "SS2", (UINT64)AmlUpdInfo->SS2);
            ASSERT_EFI_ERROR(Status)
            
            if (EFI_ERROR(Status)) return Status;
            
            Status = UpdateAslNameObject(Dsdt, "SS1", (UINT64)AmlUpdInfo->SS1);
            ASSERT_EFI_ERROR(Status)
            
            if (EFI_ERROR(Status)) return Status;

            Dsdt->Checksum = 0;
            Dsdt->Checksum = ChsumTbl((UINT8*)Dsdt, Dsdt->Length);

            gxFacs->HardwareSignature = Dsdt->Checksum;

            gFacs->HardwareSignature = Dsdt->Checksum;
        }
    }
    
    return Status;
    
    
}//end of UpdateAml

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   LockLegacyRes
//
//  Description:
//  This function Hides Legacy Resources from OS by destroing _PRS method
//  in each Legacy Device ASL Object in DSDT
//
//  Input:
//  IN ACPI_AML_UPD_INFO *AmlUpdInfo - pointer to ACPI_AML_UPD_INFO structure
//
//  Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID LockLegacyRes (ACPI_AML_UPD_INFO   *AmlUpdInfo)

{

    UINTN               HandleCnt, j, i;
    EFI_HANDLE          *HandleBuffer = NULL;
    SIO_DEV2            *SpIoDev;
    ASL_OBJ_INFO        AslObj={0};
    ACPI_HDR            *Dsdt;
    EFI_STATUS          Status;
    EFI_SIO_DATA        *EfiSioData;

    Status = pBS->LocateHandleBuffer(ByProtocol,&gEfiSioProtocolGuid, NULL, &HandleCnt, &HandleBuffer);
    //Locate all handles for SIO
    
    if (HandleBuffer == NULL) return;
    
    for (j = 0; j < HandleCnt; j++)
    {
        Status = pBS->HandleProtocol(HandleBuffer[j], &gEfiSioProtocolGuid, &EfiSioData);
        if (EFI_ERROR(Status)) continue;
        SpIoDev = EfiSioData->Owner;
        if (SpIoDev->DeviceInfo->AslName[0])
//        TRACE ((-1,"Found SIO Protocol. Name: %s\n",(UINT32)SpIoDev->DeviceInfo->AslName));
            // If this device has ASL Name and is present in DSDT
        {
            for (i = 0; i < 2; i++)
            {
                if (i) Dsdt = (ACPI_HDR*)AmlUpdInfo->Dsdt1Addr;
                else Dsdt = (ACPI_HDR*)AmlUpdInfo->Dsdt2Addr;
                
                if (Dsdt)
                {
//                    TRACE ((-1,"Looking DSDT for Name: %s\n", SpIoDev->DeviceInfo->AslName));
                    Status = GetAslObj((UINT8*)Dsdt+sizeof(ACPI_HDR),
                                       Dsdt->Length-sizeof(ACPI_HDR)-1, &SpIoDev->DeviceInfo->AslName[0],
                                       otDevice, &AslObj);
                    // Get Asl object associated with this Legacy device
                    ASSERT_EFI_ERROR(Status);
                    
//                    TRACE ((-1,"Going to hide object Data statr= %x, Length= %x\n",AslObj.DataStart, AslObj.Length));
                    if (!EFI_ERROR(Status)) HideAslMethodFromOs (&AslObj, "_PRS");
                    
                    // Lock this Device by destroing _PRS method of this object
                }
            }
        }
    }
    
    for (i = 0; i < 2; i++)
    {
        if (i) Dsdt = (ACPI_HDR*)AmlUpdInfo->Dsdt1Addr;
        else Dsdt = (ACPI_HDR*)AmlUpdInfo->Dsdt2Addr;
        
        if (Dsdt)
        {
            Dsdt->Checksum = 0;
            Dsdt->Checksum = ChsumTbl((UINT8*)Dsdt, Dsdt->Length);
        }
    }
    
    pBS->FreePool(HandleBuffer);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   CollectAmlUpdInfo
//
//  Description:
//  This function will be called when ReadyToBoot event will be signaled and
//  will update IO devices status and then update AML binary. It allso publish all
//  ACPI tables.
//
//  Input:
//  IN EFI_EVENT Event - signalled event
//  IN VOID *Context - calling context
//
//  Output:
//  VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CollectAmlUpdInfo(EFI_EVENT Event, VOID *Context)
{
    ACPI_AML_UPD_INFO   *Aui=Context;
    FACP_20     *xFacp = NULL;
//    ACPI_HDR    *Dsdt1 = NULL, *Dsdt2 = NULL;
//    FACP32      *Facp = NULL;
    FACP_20     *Facp = NULL;
    UINTN               i;
    EFI_STATUS          Status = EFI_SUCCESS;
    SETUP_DATA          *SetupData=NULL;
    UINTN SioDevStatusVarSize = sizeof(SIO_DEV_STATUS);
    EFI_GUID SioDevStatusVarGuid = SIO_DEV_STATUS_VAR_GUID;
//-------------------------------------------
    //Init Setup Sleep States with Default Values;
    TRACE((-1,"IN Collect AML Info: %x\n",0));
    
    //we need to recreate MADT table instances on READY_TO_BOOT event.
    for (i=0; i<gAcpiData.AcpiEntCount; i++)
    {
        ACPI_TBL_ITEM   *AcpiTable = gAcpiData.AcpiEntries[i];
        //-------------------------------------------
        
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == FACP_SIG)
        {
            if (gAcpiData.AcpiEntries[i]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)
                Facp = (FACP_20*) gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find FACP for V 1.1
            else
                xFacp = (FACP_20*) gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find FACP for V 2+
        }
        
        if (gAcpiData.AcpiEntries[i]->BtHeaderPtr->Signature == DSDT_SIG)
        {
            if (gAcpiData.AcpiEntries[i]->AcpiTblVer == EFI_ACPI_TABLE_VERSION_1_0B)
                Aui->Dsdt1Addr = (UINT64) gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find DSDT for V 1.1
            else
                Aui->Dsdt2Addr = (UINT64) gAcpiData.AcpiEntries[i]->BtHeaderPtr; // Find DSDT for V 2+
        }
        
        if (AcpiTable->BtHeaderPtr->Signature == APIC_SIG )
        {
#if ACPI_APIC_TBL == 1
			UINTN           AcpiVer;
            switch (AcpiTable->AcpiTblVer)
            {
                case EFI_ACPI_TABLE_VERSION_1_0B: AcpiVer=1; break;
                case EFI_ACPI_TABLE_VERSION_2_0:  AcpiVer=2; break;
                case EFI_ACPI_TABLE_VERSION_3_0:  AcpiVer=3; break;
                case EFI_ACPI_TABLE_VERSION_4_0:  AcpiVer=4; break;
                default: AcpiVer=0;
            }
#endif            
            gAcpiData.AcpiLength -= AcpiTable->BtHeaderPtr->Length;
            
            pBS->FreePool(AcpiTable->BtHeaderPtr);
            AcpiTable->BtHeaderPtr=NULL;
            
#if ACPI_APIC_TBL == 1
            Status = BuildMadtAll (AcpiVer, &AcpiTable->BtHeaderPtr);
            ASSERT_EFI_ERROR(Status);
            
            if (EFI_ERROR(Status)) return;
            
            gAcpiData.AcpiLength += AcpiTable->BtHeaderPtr->Length;
#endif
        }
    }
    
    
    ASSERT_EFI_ERROR(Status);
    TRACE((-1,"IN Collect AML Info after PT: %x\n",Status));
    
    Aui->SS1=DEFAULT_SS1;
    Aui->SS2=DEFAULT_SS2;
    Aui->SS3=DEFAULT_SS3;
    Aui->SS4=DEFAULT_SS4;
    
    Status = GetEfiVariable(L"Setup",&gSetupGuid,NULL,&i,&SetupData);
    
    if (!EFI_ERROR(Status))
    {
        //Such Variable exists so use customer choices
        if (!SetupData->AcpiAuto)
        {
            Aui->SS4=SetupData->AcpiHibernate;
            
            switch (SetupData->AcpiSleepState)
            {
                case 0:
                    Aui->SS3=0;
                    Aui->SS1=0;
                    break;
                case 1:
                    Aui->SS1=1;
                    Aui->SS3=0;
                    break;
                case 2:
                    Aui->SS3=1;
                    Aui->SS1=1;
                    break;
            }
        }
    }
    
//TODO//TODO//TODO//TODO//TODO
//  Check Setup options (if any) to enable/disable some onboard devices
//TODO//TODO//TODO//TODO//TODO

    Status = pRS->GetVariable(SIO_DEV_STATUS_VAR_NAME, &SioDevStatusVarGuid, NULL,
                              &SioDevStatusVarSize, &Aui->SioDevStatusVar.DEV_STATUS);
                              
    if (EFI_ERROR(Status)) Aui->SioDevStatusVar.DEV_STATUS = 0;
    gAcpiIaBootArch &= (~((UINT16)(IA_LEGACY | IA_8042)));// clear first 2 bits of gAcpiIaBootArch,
    // while preserving others. This 2 bits will be updated later.
    
    //Dynamically update IA_PC_BOOT_ARCHITECTURE flag based on SIO_DEV_STATUS_VAR
    if (
        (Aui->SioDevStatusVar.SerialA == 1 ) ||
        (Aui->SioDevStatusVar.SerialB == 1 ) ||
        (Aui->SioDevStatusVar.Lpt == 1 ) ||
        (Aui->SioDevStatusVar.Fdd == 1 ) ||
        (Aui->SioDevStatusVar.Game1 == 1 ) ||
        (Aui->SioDevStatusVar.Game2 == 1 )
    )
        gAcpiIaBootArch |= IA_LEGACY;
        
    if (
        (Aui->SioDevStatusVar.Key60_64 == 1) ||
        (Aui->SioDevStatusVar.Ps2Mouse == 1) ||
        (Aui->SioDevStatusVar.Ec62_66 == 1)
    )
        gAcpiIaBootArch |= IA_8042;
        
        
    if ( xFacp != NULL)
    {
        xFacp->IAPC_BOOT_ARCH &= gAcpiIaBootArch;
        xFacp->Header.Checksum = 0;
        xFacp->Header.Checksum = ChsumTbl((UINT8*)&xFacp->Header, xFacp->Header.Length);
    }
    
    if ((UINT8*) xFacp != (UINT8*) Facp)
    {
        //ACPI v1.0b don't have IAPC_BOOT_ARCH flag field!
        //Legacy free extension has but.. 1.1b Legacy free extension has. REV==2 and higher
        if (Facp->Header.Revision > ACPI_REV1)
        {
            Facp->IAPC_BOOT_ARCH &= gAcpiIaBootArch;
            Facp->Header.Checksum = 0;
            Facp->Header.Checksum = ChsumTbl((UINT8*)&Facp->Header, Facp->Header.Length);
        }
    }
    
    Status=UpdateAml(Aui);
    ASSERT_EFI_ERROR(Status);
    
    if (SetupData->AcpiLockLegacyRes) LockLegacyRes (Aui); // LockLegacyDev
    
    if (!gForceAcpi1) Status = PublishTbl (1, 1);
    else Status = PublishTbl (1, 0);
    ASSERT_EFI_ERROR(Status);
    gPublishedOnReadyToBoot = 1;
//  pBS->CloseEvent()
//  pBS->FreePool(Aui);
}// end of CollectAmlUpdInfo


//#pragma warning ( pop )
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ACPI SUPPORT PPROTOCOL function Implementation
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AcpiSupportGetAcpiTable
//
//  Description:
//  This function returns ACPI table
//
//  Input:
//  IN EFI_ACPI_SUPPORT_PROTOCOL *This - pointer to EFI_ACPI_SUPPORT_PROTOCOL instance
//  IN INTN Index - Index of ACPI table to return
//  OUT VOID **Table - Pointer where to place found table
//  OUT EFI_ACPI_TABLE_VERSION Version - requested ACPI table version
//  OUT UINTN *Handle - requested ACPI table handle
//
//  Output:
//  EFI_SUCCESS - Function executed successfully
//  EFI_OUT_OF_RESOURCES - not enough memory to allocate table
//  EFI_INVALID_PARAMETER - invalid EFI_ACPI_SUPPORT_PROTOCOL pointer
//  EFI_NOT_FOUND - requested ACPI table not found
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AcpiSupportSetAcpiTable
//
//  Description:
//  This function allows to add, remove of modify ACPI table
//
//  Input:
//  IN EFI_ACPI_SUPPORT_PROTOCOL *This - pointer to EFI_ACPI_SUPPORT_PROTOCOL instance
//  IN OPTIONAL VOID *Table - Pointer to update data. If NULL, corresponded table
//                            should be removed
//  IN BOOLEAN Checksum - if TRUE, function will recalculate checksum before adding table
//  IN EFI_ACPI_TABLE_VERSION Version - requested ACPI table version
//  IN OUT UINTN *Handle - requested ACPI table handle
//
//  Output:
//  EFI_SUCCESS - Function executed successfully
//  EFI_OUT_OF_RESOURCES - not enough memory to perform operation
//  EFI_INVALID_PARAMETER - invalid EFI_ACPI_SUPPORT_PROTOCOL pointer or ACPI table
//                          content
//  EFI_ABORTED - provided ACPI table already present
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

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


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AcpiSupportPublishTables
//
//  Description: Causes one or more versions of the ACPI tables to be published in
//               the EFI system configuration tables.
//
//  Input:
//  IN EFI_ACPI_SUPPORT_PROTOCOL *This - pointer to EFI_ACPI_SUPPORT_PROTOCOL instance
//  IN EFI_ACPI_TABLE_VERSION Version - ACPI table version
//
//  Output:
//  EFI_SUCCESS - Function executed successfully
//  EFI_ABORTED - invalid EFI_ACPI_SUPPORT_PROTOCOL pointer or
//                an error occurred and the function could not complete successfully.
//  EFI_UNSUPPORTED - passed ACPI table version invalid
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

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
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AcpiInstallAcpiTable
//
//  Description:  Installs an ACPI table into the RSDT/XSDT.
//
//  Input:
//  IN EFI_ACPI_TABLE_PROTOCOL *This - A pointer to a EFI_ACPI_TABLE_PROTOCOL.
//  IN VOID *AcpiTableBuffer - A pointer to a buffer containing the ACPI table to be installed.
//  IN UINTN AcpiTableBufferSize - Specifies the size, in bytes, of the AcpiTableBuffer buffer.
//  OUT UINTN *TableKey - Returns a key to refer to the ACPI table.
//
//  Output:
//  EFI_SUCCESS - Function executed successfully
//  EFI_INVALID_PARAMETER - Either AcpiTableBuffer is NULL, TableKey is NULL, or
//  AcpiTableBufferSize and the size field embedded in the ACPI
//  table pointed to by AcpiTableBuffer are not in sync
//  EFI_OUT_OF_RESOURCES - Insufficient resources exist to complete the request.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
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
    
    if (!gForceAcpi1) Status = PublishTbl (1, 1);
    else PublishTbl (1, 0);
    return Status;
}// end of AcpiInstallAcpiTable


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AcpiUninstallAcpiTable
//
//  Description:  Function allows a caller to remove an ACPI table.
//
//  Input:
//  IN EFI_ACPI_TABLE_PROTOCOL *This - A pointer to a EFI_ACPI_TABLE_PROTOCOL.
//  IN UINTN *TableKey - Specifies the table to uninstall. The key was returned from
//  InstallAcpiTable().
//
//  Output:
//  EFI_SUCCESS - Function executed successfully
//  EFI_NOT_FOUND - TableKey does not refer to a valid key for a table entry.
//  EFI_OUT_OF_RESOURCES - Insufficient resources exist to complete the request.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure: GetAcpiTable2
//
//  Description:
//  Returns a requested ACPI table.
//  The GetAcpiTable() function returns a pointer to a buffer containing the ACPI table associated
//  with the Index that was input. The following structures are not considered elements in the list of
//  ACPI tables:
//  - Root System Description Pointer (RSD_PTR)
//  - Root System Description Table (RSDT)
//  - Extended System Description Table (XSDT)
//  Version is updated with a bit map containing all the versions of ACPI of which the table is a
//  member.
//  
//  Input:
//  Index       The zero-based index of the table to retrieve.
//  Table       Pointer for returning the table buffer.
//  Version     On return, updated with the ACPI versions to which this table belongs. Type
//              EFI_ACPI_TABLE_VERSION is defined in "Related Definitions" in the
//              EFI_ACPI_SDT_PROTOCOL.    
//  TableKey    On return, points to the table key for the specified ACPI system definition table. This
//              is identical to the table key used in the EFI_ACPI_TABLE_PROTOCOL.  
//  Output:
//  EFI_SUCCESS       The function completed successfully.
//  EFI_NOT_FOUND     The requested index is too large and a table was not found.                                  
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetAcpiTable2 (
  IN  UINTN                               Index,
  OUT EFI_ACPI_SDT_HEADER                 **Table,
  OUT EFI_ACPI_TABLE_VERSION              *Version,
  OUT UINTN                               *TableKey)
{

    return AcpiSupportGetAcpiTable (&gAcpiData.AcpiSupportProtocol, Index,
                                Table, Version,TableKey);

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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AcpiNewCoreEntry
//
//  Description:
//  This function is ACPI driver entry point
//
//  Input:
//  IN EFI_HANDLE ImageHandle - Image handle
//  IN EFI_SYSTEM_TABLE *SystemTable - pointer to system table
//
//  Output:
//  EFI_SUCCESS - Function executed successfully, ACPI_SUPPORT_PROTOCOL installed
//  EFI_ABORTED - Dsdt table not found or table publishing failed
//  EFI_ALREADY_STARTED - driver already started
//  EFI_OUT_OF_RESOURCES - not enough memory to perform operation
//
//  Notes:
//  This function also creates ReadyToBoot event to update AML objects before booting
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS AcpiNewCoreEntry (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS      Status = EFI_SUCCESS;
    ACPI_HDR        *Dsdt1Ptr = NULL, *Dsdt2Ptr = NULL;
    VOID            *DummyPtr;
    static EFI_GUID Acpisupguid = EFI_ACPI_SUPPORT_GUID;
//	    static EFI_GUID AcpiTableProtocolGuid = EFI_ACPI_TABLE_PROTOCOL_GUID;
    UINTN           AcpiVer;
    EFI_ACPI_TABLE_VERSION  EfiAcpiVer;
    ACPI_TBL_ITEM   *AcpiTable = NULL;
#if FORCE_TO_ACPI1_SETUP_ENABLE
    SETUP_DATA      *SetupData = NULL;
    UINTN           SetupSize = 0;
#endif
//------------------------
    InitAmiLib(ImageHandle,SystemTable);
    PROGRESS_CODE(DXE_ACPI_INIT);
    TRACE((-1,"IN ACPI Start: %x\n", Status));
    //it must be ony one instance of this protocol
    Status = pBS->LocateProtocol(&Acpisupguid,NULL,&DummyPtr);
    
    if (!EFI_ERROR(Status)) return EFI_ALREADY_STARTED;

/*
#if AmiBoardInfo_SUPPORT == 1
    Status = pBS->LocateProtocol(&gAmiBoardInfoGuid, NULL,(VOID**)&gAmiBoardInfoProtocol);
    
    if (EFI_ERROR(Status))
    {
        gAmiBoardInfoProtocol = NULL;
        TRACE((-1, "ACPI: Can't find AMI Board Info Protocol %r EXITING!",Status));
        return Status;
    }
    
#endif
*/
 //Will use AmiBoardInfo2 Protocol For DSDT.
    Status=AmiSdlInitBoardInfo();
    if (EFI_ERROR(Status))
    {
        TRACE((-1, "ACPI: Can't find AMI Board Info Protocol %r EXITING!",Status));
        ASSERT_EFI_ERROR(Status);
        return Status;
    }


    AcpiVer = 2;
    EfiAcpiVer = EFI_ACPI_TABLE_VERSION_2_0;
    
    if (ACPI_BUILD_TABLES_3_0 == 1)
    {
        AcpiVer = 3;
        EfiAcpiVer = EFI_ACPI_TABLE_VERSION_3_0;
    }
    
    if (ACPI_BUILD_TABLES_4_0 == 1)
    {
        AcpiVer = 4;
        EfiAcpiVer = EFI_ACPI_TABLE_VERSION_4_0;
    }
#if FORCE_TO_ACPI1_SETUP_ENABLE
    Status = GetEfiVariable(L"Setup",&gSetupGuid,NULL,&SetupSize,&SetupData);
    ASSERT_EFI_ERROR(Status);
    if (!EFI_ERROR (Status)) 
        if (SetupData->ForceToAcpi1 ==1) 
        {
            AcpiVer = gForceAcpi1 = 1;
            EfiAcpiVer = EFI_ACPI_TABLE_VERSION_1_0B;
        }
#endif
    Status = BuildFacs ();
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR (Status)) return EFI_OUT_OF_RESOURCES;
    
    AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
    ASSERT(AcpiTable);
    
    if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
    
    Status = BuildFacpiAll (1, &AcpiTable->BtHeaderPtr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR (Status)) return EFI_OUT_OF_RESOURCES;
    
    AcpiTable->AcpiTblVer = EFI_ACPI_TABLE_VERSION_1_0B;
    Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    
    gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
    if (!gForceAcpi1)
    {
        AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
        ASSERT(AcpiTable);
    
        if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
    
        Status = BuildFacpiAll (AcpiVer, &AcpiTable->BtHeaderPtr);
        ASSERT_EFI_ERROR(Status);
    
        if (EFI_ERROR (Status)) return EFI_OUT_OF_RESOURCES;
    
        AcpiTable->AcpiTblVer = EfiAcpiVer;
        Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
        ASSERT_EFI_ERROR(Status);
    
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    
        gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
    }
    TRACE((-1,"IN ACPI 1: %x\n", Status));
    
    Status = GetDsdtFv(&Dsdt1Ptr, &Dsdt2Ptr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_ABORTED;
    
    if ((Dsdt2Ptr != NULL) && (Dsdt1Ptr != NULL))
    {
        TRACE((-1,"DSDT1 addres 0x%X; -> %r \n", Dsdt1Ptr, Status));
        TRACE((-1,"DSDT2 addres 0x%X; -> %r \n", Dsdt2Ptr, Status));
        
        Dsdt1Ptr->Checksum = 0;
        Dsdt2Ptr->Checksum = 0;
        Dsdt1Ptr->Checksum = ChsumTbl((UINT8*)Dsdt1Ptr, Dsdt1Ptr->Length);
        AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
        ASSERT(AcpiTable);
        
        if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
        
        AcpiTable->BtHeaderPtr = Dsdt1Ptr;
        AcpiTable->AcpiTblVer = EFI_ACPI_TABLE_VERSION_1_0B;
        Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
        
        gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
        
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
    
    if (Dsdt1Ptr == NULL)
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
    
    if (Dsdt2Ptr == NULL)
    {
        TRACE((-1,"DSDT11 addres 0x%lX; -> %r \n", Dsdt1Ptr, Status));
        Dsdt1Ptr->Checksum = 0;
        Dsdt1Ptr->Checksum = ChsumTbl((UINT8*)Dsdt1Ptr, Dsdt1Ptr->Length);
        AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
        ASSERT(AcpiTable);
        
        if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
        
        AcpiTable->BtHeaderPtr = Dsdt1Ptr;
        AcpiTable->AcpiTblVer = EFI_ACPI_TABLE_VERSION_1_0B;
        Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
        ASSERT_EFI_ERROR(Status);
        
        if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
        
        gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
    }
    
    Status = UpdateFacp ();
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_ABORTED;

#if ACPI_APIC_TBL == 1        
    AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
    ASSERT(AcpiTable);
    
    if (!AcpiTable) return EFI_OUT_OF_RESOURCES;
    
    Status = BuildMadtAll (AcpiVer, &AcpiTable->BtHeaderPtr);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_ABORTED;
    
    AcpiTable->AcpiTblVer = EfiAcpiVer;
    Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
    ASSERT_EFI_ERROR(Status);
    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
    
    gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
#endif    

#if MPS_TABLE_SUPPORT == 1
    Status = MpsTableBuilderInit(ImageHandle, SystemTable);
    ASSERT_EFI_ERROR(Status);
#endif
//------ Performance Measurment ------------------------------------

    AcpiTable = MallocZ (sizeof (ACPI_TBL_ITEM));
    ASSERT(AcpiTable);
    
    if (AcpiTable)
    {
        Status = BuildFPDT (AcpiVer, &AcpiTable->BtHeaderPtr);

        if (!EFI_ERROR(Status)) 
        {
            AcpiTable->AcpiTblVer = EfiAcpiVer;
            Status = AppendItemLst ((T_ITEM_LIST*)&gAcpiData, (VOID*) AcpiTable);
            ASSERT_EFI_ERROR(Status);
            gAcpiData.AcpiLength += gAcpiData.AcpiEntries[gAcpiData.AcpiEntCount-1]->BtHeaderPtr->Length;
        }
    }
//------ Performance Measurment End --------------------------------
    gAuiGlob = MallocZ(sizeof(ACPI_AML_UPD_INFO));
    ASSERT(gAuiGlob);
    
    
    
    //free temporary space used during this routine execution ??????
    
    // ???? Publish all tbls 1,1 ??????
    Status = CreateReadyToBootEvent(
                 TPL_CALLBACK, CollectAmlUpdInfo, gAuiGlob, &gEvtReadyToBoot
             );
    ASSERT_EFI_ERROR(Status);
    
    
    
    if (EFI_ERROR(Status))return EFI_ABORTED;
    
    
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
    //TRACE((-1,"Before PublishTbl: %x\n", Status));
    //Status = PublishTbl (1, 1);
    //TRACE((-1,"After PublishTbl: %x\n", Status));
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
