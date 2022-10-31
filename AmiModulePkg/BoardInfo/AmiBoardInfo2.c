//**********************************************************************
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
// $Header: /Alaska/BIN/Core/Modules/BoardInfo/AmiBoardInfo.c 4     3/01/10 5:06p Yakovlevs $
//
// $Revision: 4 $
//
// $Date: 3/01/10 5:06p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	<This File Name>
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Token.h>
#include <AmiDxeLib.h>
#include <Protocol/AmiBoardInfo2.h>
#include <Library/AmiSdlLib.h>
#include <Setup.h> //EIP148558

//----------------------------------------------------------------------------
//Global GUIDs Definitions goes here
EFI_GUID    gAmiBoardInfo2Guid=AMI_BOARD_INFO2_PROTOCOL_GUID;

/*
//----------------------------------------------------------------------------
//Variable definitions goes here
//PCI Buses Xlate Table
extern UINTN  BusNumXlatTbl;
extern UINTN  BusNumXlatTblEnd;
//----------------------------------------------------------------------------
//Legacy IRQ routing table delivered from oempir.inc and PCIBoard.ASM
extern UINTN IRQ_Table;
extern UINTN IRQ_Table_end;
//----------------------------------------------------------------------------
//IOAPIC IRQ routing table delivered from mppciirq.inc and PCIBoard.ASM
#if AMI_BOARD_VER_COMBINED >= 100
extern MP_IRI_Table;
extern UINTN MP_IRI_Table_END;
#else
extern UINTN  MP_IRI_Table;
extern MP_IRI_Table_END;
#endif
//----------------------------------------------------------------------------
//IO/APIC(s) Info Table
extern UINTN MP_IAI_Table;
extern UINTN MP_IAI_Table_END;
//----------------------------------------------------------------------------
//Hot pluf stuff
//For PIC we need it it has SLOT # Info APIC does not.
//And for verification: HP_SlotP_Count has to much HP_SlotA_Count
extern HP_SlotP_Start;
extern HP_SlotP_End;
//----------------------------------------------------------------------------
//For APIC used in MP 1.4 Tables generation
extern HP_SlotA_Start;
extern HP_SlotA_End;
*/

extern UINT8 AmlCode[];

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:		gAmiBoardInfo2 Protocol
//
// Description:	Ami Board Info 2 Protocol Instance.
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
AMI_BOARD_INFO2_PROTOCOL gAmiBoard2Protocol = {
	//UINTN					BoardInfoVersion
	5,						//APTIO 5 version
	//BOOLEAN                 DataValid;      //Signifies if Board Specific IRQ data was updated.
    FALSE,
    //UINT8                   Reserved[7];
    {0,0,0,0,0,0,0},
    //AMI_SDL_TBL_HEADER      *PciBrdData;
	NULL,
    //AMI_SDL_TBL_HEADER      *SioBrdData;
	NULL,
    //AMI_APIC_INFO           *ApicBrdData;
	NULL,
    //VOID                    *BrdAcpiInfo;
    NULL,
	//UINTN                   BrdAcpiLength;
    0,
    //VOID					*BrdAcpiIrqInfo; //Optional
    NULL,
    //VOID					*BrdSetupInfo;
    NULL,
    //VOID                    *BoardOemInfo;
    NULL,
    //PCI_IRQ_PIC_ROUTE       *PicRoutTable;
    NULL,
    //UINTN                   PicRoutLength;
    0,
    //PCI_IRQ_APIC_ROUTE      *ApicRoutTable;
    NULL,
    //UINTN                   ApicRoutLength;
    0
};

UINT8 *gSdlDataStart=NULL;
UINTN  gSdlDataLen=0;
UINT8 *gDsdtDataStart=NULL;
UINTN  gDsdtDataLen=0;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:		gAmiBoardProtocolHandle
//
// Description:	Ami Board Info Protocol Handle.
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
EFI_HANDLE gAmiBoardInfo2Handle=NULL;

//7B9A0A12-42F8-4d4c-82B6-32F0CA1953F4
#define AMI_BOARD_INFO_OUT_FILE_GUID \
	{ 0x7B9A0A12, 0x42F8, 0x4d4c, { 0x82, 0xB6, 0x32, 0xF0, 0xCA, 0x19, 0x53, 0xF4 } }
// {9BEC7109-6D7A-413A-8E4B-019CED0503E1}
#define AMI_BOARD_INFO_OUT_SECTION_GUID \
	{ 0x9bec7109, 0x6d7a, 0x413a, { 0x8e, 0x4b, 0x1, 0x9c, 0xed, 0x5, 0x3, 0xe1 } }
	
//++EIP 123587	
//7E374E25-8E01-4FEE-87F2-390C23C606CD
//The GUID ABOVE was matching to the Intel's RC ACPI_TABLE_STORAGE GUID and requested to be replaced.
//#define AMI_ASL_DSDT_OUT_FILE_GUID \
//	{ 0x7E374E25, 0x8E01, 0x4FEE, { 0x87, 0xF2, 0x39, 0x0C, 0x23, 0xC6, 0x06, 0xCD } }
//This new GUID BELOW replacement it MUST be replaced in ACPI.DsdtAsl.inf 
//[Defines]
//  FILE_GUID = C118F50D-391D-45f4-B3D3-11BC931AA56D
//!!!!!ATTENTION!!!!!!  If you reading this check FILE_GUID in ACPI.DsdtAsl.inf !!!!
#define AMI_ASL_DSDT_OUT_FILE_GUID \
    { 0xC118F50D, 0x391D, 0x45f4, { 0xB3, 0xD3, 0x11, 0xBC, 0x93, 0x1A, 0xA5, 0x6D } }

//EIP148558 >>
#define AMI_ASL_DSDT_W7_OUT_FILE_GUID \
    { 0x1b852ce9, 0x6bcb, 0x4c9d, { 0xbe, 0x8a, 0xba, 0x34, 0xfa, 0x8f, 0xbd, 0x77 } }
//EIP148558 <<
//--EIP 123587	


//TODO: Move these into .dec file
EFI_GUID gAmiBoardInfoFileGuid = AMI_BOARD_INFO_OUT_FILE_GUID;
EFI_GUID gAmiBoardInfoSectionGuid = AMI_BOARD_INFO_OUT_SECTION_GUID;
EFI_GUID gAmiAslDsdtFileGuid = AMI_ASL_DSDT_OUT_FILE_GUID;
EFI_GUID gAmiAslDsdtW7FileGuid = AMI_ASL_DSDT_W7_OUT_FILE_GUID; //EIP148558



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   ParseAmiSdlData
//
//  Description:
//  This Parses SDL Data Table Buffer and returnd pointer to the data table 
// 	with given "Signature"
//
//  Input:
//  IN UINT8 	*DataPtr 	Pointer to the SDL Data Buffer. 
//	IN UINTN  	DataLen	 	Total Data Buffer Length in bytes.
//	IN UINT64  	Signature	Total Data Buffer Length in bytes.
//	OUT VOID 	**TblPtr	Pointer to the data table with "Signature". 
//  Output:
//  EFI_SUCCESS - Function executed successfully,
//  EFI_ALREADY_STARTED - driver already started
//  EFI_OUT_OF_RESOURCES - not enough memory to perform operation
//
//  Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ParseAmiSdlData(UINT8 *DataPtr, UINTN DataLen, UINT64 Signature, OUT VOID **TblPtr){
    UINT8   *start=DataPtr;
    UINT64  *sig;
    UINTN   i;
//-----------------
    if(start==NULL || TblPtr==NULL || DataLen==0 ) return EFI_INVALID_PARAMETER;

    for(i=0; i<DataLen-sizeof(UINT64); i++){
        sig=(UINT64*)(&start[i]);
        if(*sig == Signature) {
            *TblPtr=sig;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   AmiBoardEntryPoint
//
//  Description:
//  This function is AMI Board Info driver entry point
//
//  Input:
//  IN EFI_HANDLE ImageHandle - Image handle
//  IN EFI_SYSTEM_TABLE *SystemTable - pointer to system table
//
//  Output:
//  EFI_SUCCESS - Function executed successfully,
//  EFI_ALREADY_STARTED - driver already started
//  EFI_OUT_OF_RESOURCES - not enough memory to perform operation
//
//  Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiBoardEntryPoint(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS                  Status;
    AMI_BOARD_INFO2_PROTOCOL    *amibrd=NULL;
    VOID                        *DataPtr=NULL;
    //EIP148558 >>
    EFI_GUID                    SetupGuid = SETUP_GUID;
    UINTN                       Size = sizeof(SETUP_DATA);
    SETUP_DATA                  SetupData;
    //EIP148558 <<
//--------------------------------

	InitAmiLib(ImageHandle,SystemTable);

	//First try to check if we are running on MultiPlatfortm Enabled BIOS
    Status=pBS->LocateProtocol(&gAmiBoardInfo2Guid, NULL, &amibrd);
    if(!EFI_ERROR(Status)){
        TRACE((-1,"AmiBrdInfo: Multi-Platform BrdInfo present Status=%r Exiting",Status));
        return Status;
    }

//DEBUG
//EFI_DEADLOOP();
//DEBUG

    amibrd=&gAmiBoard2Protocol;

    Status=AmiSdlReadFfsSdlData(&gSdlDataStart, &gSdlDataLen, &gAmiBoardInfoFileGuid, &gAmiBoardInfoSectionGuid);
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status))return Status;

//
// 	When we will have SDL DATA Stored as an AMI SDL Image resource... following code will bw used.
// 	Read resource section from this driver image.
//  Status=ReadImageResource(ImageHandle, &gAmiBoardInfoSectionGuid, &gSdlDataStart, &gSdlDataLen);
//    ASSERT_EFI_ERROR(Status);
//    if(EFI_ERROR(Status)) return Status;
//

    //Init Ami Platform Info Protocol Instance...

    //1. First get PCI Data pointer AMI_PCI_DATA_SIG $PCIDATA
    Status=ParseAmiSdlData(gSdlDataStart,gSdlDataLen,AMI_PCI_DATA_SIG,&DataPtr);
    //It must be there! if it is not - ASSERT HERE.
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)) return Status;
    amibrd->PciBrdData=(AMI_SDL_PCI_DATA*)DataPtr;


    //2. Then Get Dsdt It mus be there
    //Take care if ACPI_SUPPORT IS OFF we will not have a DSDT table in ROM
    //EIP148558 >>
    Status = pRS->GetVariable( L"Setup", &SetupGuid, NULL, &Size, &SetupData );
    if (SetupData.OsSelect == OS_WINDOWS7) { //EIP149462 //CSP20140122
      Status=AmiSdlReadFfsSdlData(&gDsdtDataStart, &gDsdtDataLen, &gAmiAslDsdtW7FileGuid, NULL);
    } else {
   	//EIP148558 <<
      Status=AmiSdlReadFfsSdlData(&gDsdtDataStart, &gDsdtDataLen, &gAmiAslDsdtFileGuid, NULL);
    } //EIP148558
    //ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)){
        TRACE((-1,"\n======================================================================================\n"));
        TRACE((-1,"AmiBrdInfo: !!!WARNING!!! Can't find DSDT Table in BIOS FV - %r.\n            !!!WARNING!!! Check your project ACPI settings...\n",Status));
        TRACE((-1,"======================================================================================\n\n"));
	} else {
    	amibrd->BrdAcpiInfo=gDsdtDataStart;
	    amibrd->BrdAcpiLength=gDsdtDataLen;
	}

    //3. Than get SIO Data Pointer...AMI_SIO_DATA_SIG $SIODATA
    DataPtr=NULL;
    Status=ParseAmiSdlData(gSdlDataStart,gSdlDataLen,AMI_SIO_DATA_SIG,&DataPtr);

    //Not A BIG deal if we can't find SIO Device. System Might be a LEGACY FREE.
    //not need to assert here //ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)){
        TRACE((-1,"\n======================================================================================\n"));
        TRACE((-1,"AmiBrdInfo: !!!WARNING!!! Can't find SIO Data Table in BIOS FV - %r.\n            !!!WARNING!!! Check your project SIO settings...\n",Status));
        TRACE((-1,"======================================================================================\n\n"));
	} else 
		amibrd->SioBrdData=(AMI_SDL_SIO_DATA*)DataPtr;

    //3. Get IoApic Data Pointer...AMI_APIC_DATA_SIG  $APIDATA
    DataPtr=NULL;
    Status=ParseAmiSdlData(gSdlDataStart,gSdlDataLen,AMI_APIC_DATA_SIG,&DataPtr);
    //Not A BIG deal if we can't find SIO Device. System Might be a LEGACY FREE.
    //not need to assert here //ASSERT_EFI_ERROR(Status);
        if(EFI_ERROR(Status)){
        TRACE((-1,"\n======================================================================================\n"));
        TRACE((-1,"AmiBrdInfo: !!!WARNING!!! Can't find IOAPIC Data Table in BIOS FV - %r.\n            !!!WARNING!!! Check your project APIC and ROUTER settings...\n",Status));
        TRACE((-1,"======================================================================================\n\n"));
	} else 
		amibrd->ApicBrdData=(AMI_SDL_APIC_DATA*)DataPtr;


    //Now install AMI BOARD INFO 2 Protocol...
    Status=pBS->InstallMultipleProtocolInterfaces(
        &gAmiBoardInfo2Handle,
        &gAmiBoardInfo2Guid, &gAmiBoard2Protocol, NULL
    );

	return Status;
}

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
