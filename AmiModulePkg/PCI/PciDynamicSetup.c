//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**           5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093      **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/BIN/Core/Modules/PciBus/PciBusSetup.c 2     9/25/08 11:54a Yakovlevs $
//
// $Revision: 2 $
//
// $Date: 9/25/08 11:54a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/BIN/Core/Modules/PciBus/PciBusSetup.c $
// 
// 2     9/25/08 11:54a Yakovlevs
// Added PciBus Driver version display and Screen update routines.
// 
// 1     3/12/07 12:07p Yakovlevs
// 
// 
//**********************************************************************
//
//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:		PciBusSetup.c
//
// Description:	Pci Bus Setup Routines
//
// NOTE: Use this hook to dynamically change PCI BUS Driver Setup screen 
// Strings.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Token.h>
#include <PciDynamicSetupStrDefs.h>
#include <AmiDxeLib.h>
#include <PciBus.h>
#include <PciHostBridge.h>
#include <PciSetup.h>
#include "PciDynamicSetupPrivate.h"


//Protocols Used
#include <Protocol/DevicePath.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>

//Library used
#include <Library/HiiLib.h>
#include <Library/AmiSdlLib.h>
#include <Library/AmiPciBusLib.h>
#include <Library/AmiHiiUpdateLib.h>

//-------------------------------------------------------------------------
// Global Variable Definitions
//-------------------------------------------------------------------------
PCI_DEV_FORM_DATA 	gPciDevForm={0};

PCI_IFR_INFO		gPciIfrInfo;

EFI_HII_HANDLE 		gPciHiiHandle = NULL;

VOID SortData(PCI_IFR_INFO* IfrData);

EFI_STATUS PciBrowserCallback(
    CONST EFI_HII_CONFIG_ACCESS_PROTOCOL*, EFI_BROWSER_ACTION,EFI_QUESTION_ID,UINT8,EFI_IFR_TYPE_VALUE*, EFI_BROWSER_ACTION_REQUEST*
    );

EFI_HII_CONFIG_ACCESS_PROTOCOL CallBack = { NULL, NULL, PciBrowserCallback };

EFI_GUID PciFormsetGuid = PCI_FORM_SET_GUID;

CALLBACK_INFO SetupCallBack[] =
{
    // Last field in every structure will be filled by the Setup
    { &PciFormsetGuid, &CallBack, PCI_FORM_SET_CLASS, 0, 0},
};

EFI_GUID gEfiIfrTianoGuid = EFI_IFR_TIANO_GUID;

AMI_BOARD_INIT_PROTOCOL		*gPciInitProtocol = NULL;

PCI_DEV_SETUP_GOTO_DATA		*gCurrentPciDevData=NULL;




EFI_STRING_ID GetClassStrId(PCI_DEV_INFO *Dev){
	static EFI_STRING_ID	pciclasslist[]={
			STRING_TOKEN(STR_PCI_CLAS_00_STR),
			STRING_TOKEN(STR_PCI_CLAS_01_STR),
			STRING_TOKEN(STR_PCI_CLAS_02_STR),
			STRING_TOKEN(STR_PCI_CLAS_03_STR),
			STRING_TOKEN(STR_PCI_CLAS_04_STR),
			STRING_TOKEN(STR_PCI_CLAS_05_STR),
			STRING_TOKEN(STR_PCI_CLAS_06_STR),
			STRING_TOKEN(STR_PCI_CLAS_07_STR),
			STRING_TOKEN(STR_PCI_CLAS_08_STR),
			STRING_TOKEN(STR_PCI_CLAS_09_STR),
			STRING_TOKEN(STR_PCI_CLAS_0A_STR),
			STRING_TOKEN(STR_PCI_CLAS_0B_STR),
			STRING_TOKEN(STR_PCI_CLAS_0C_STR),
			STRING_TOKEN(STR_PCI_CLAS_0D_STR),
			STRING_TOKEN(STR_PCI_CLAS_0E_STR),
			STRING_TOKEN(STR_PCI_CLAS_0F_STR),
			STRING_TOKEN(STR_PCI_CLAS_10_STR),
			STRING_TOKEN(STR_PCI_CLAS_11_STR)
	};
	UINTN	idx;
	EFI_STRING_ID	strid;
//------------------
	if(Dev==NULL){
		strid=STR_NOT_PRESENT_STR;
	} else {
		idx=Dev->Class.BaseClassCode;
		strid=pciclasslist[idx];
	}
	
	return strid;
}

//-------------------------------------------------------------------------
//
// This function generates SIO LD device title string and Help string
//
EFI_STATUS UpdateGotoStrings(PCI_DEV_SETUP_GOTO_DATA *PciDevGotoData){
	EFI_STRING_ID		strid;
	EFI_STRING 			s, locationstr, statusstr, classstr;
//--------------------------------
    // Create goto title string
    // "On Board  Device    [classxxxx]" 
    // "Slot #XX" Empty     [Brg [bdf]] 
    // "Slot #XX  Populated [classxxxx]"
	
#if defined(PCI_SETUP_SHOW_NOT_FOUND_ONBOARD_DEVICES) && (PCI_SETUP_SHOW_NOT_FOUND_ONBOARD_DEVICES==1)
#else	
	if(PciDevGotoData->PciSdlInfo->PciDevFlags.Bits.OnBoard && PciDevGotoData->PciDevInfo==NULL)
		return EFI_SUCCESS;
#endif		
	
    s = MallocZ(160);
    if(PciDevGotoData->PciSdlInfo->PciDevFlags.Bits.OnBoard){
        // "On Board  Device    [classxxxx]" 
    	locationstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_ONBOARD_STR),NULL);
    	statusstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_DEVICE_STR),NULL);
   		strid=GetClassStrId(PciDevGotoData->PciDevInfo);
    	classstr=HiiGetString(gPciHiiHandle,strid,NULL);
    	Swprintf(s, L"%s  %s [%s]", locationstr, statusstr, classstr);
    } else {
    	locationstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_SLOT_STR),NULL);
    	if(PciDevGotoData->PciDevInfo==NULL){
            // "Slot #XX Empty     [Brg [bdf]]" 
    		statusstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_EMPTY_STR),NULL);
    		if(PciDevGotoData->ParentBrgInfo==NULL){
    			classstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_NOT_PRESENT_STR),NULL);
    			Swprintf(s, L"%s #%2d %s [FROM BRG (%s)]", 
    				locationstr, PciDevGotoData->PciSdlInfo->Slot, statusstr, classstr);
    		} else {
    			Swprintf(s, L"%s #%2d %s [FROM BRG (B%X|D%X|F%X)]", 
    				locationstr, PciDevGotoData->PciSdlInfo->Slot, statusstr, 
    				PciDevGotoData->ParentBrgInfo->Address.Addr.Bus,
    				PciDevGotoData->ParentBrgInfo->Address.Addr.Device,
    				PciDevGotoData->ParentBrgInfo->Address.Addr.Function	);
    		}
    	} else {
    	    // "Slot #XX  Populated [classxxxx]"
    		statusstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_POPULATED_STR),NULL);
        	strid=GetClassStrId(PciDevGotoData->PciDevInfo);
        	classstr=HiiGetString(gPciHiiHandle,STRING_TOKEN(strid),NULL);
        	Swprintf(s, L"%s #%2d %s [%s]", locationstr, PciDevGotoData->PciSdlInfo->Slot, statusstr, classstr);
    	}
    }
    
    PciDevGotoData->GotoStringId=HiiSetString(gPciHiiHandle, 0, s, NULL );
    PciDevGotoData->GotoHelpStringId=STRING_TOKEN(STR_PCI_GOTO_HELP);
    PciDevGotoData->ShowItem=TRUE;
    
    pBS->FreePool(s);

    return EFI_SUCCESS;
}

VOID CreatePciGotoItems(PCI_IFR_INFO *PciIfrInfo)
{
    UINTN 			i;
//    EFI_STATUS 		Status;
    EFI_QUESTION_ID GotoId;
//----------------------------------------------

    for (i=0; i<PciIfrInfo->PciDevCount; i++) {
    	PCI_DEV_SETUP_GOTO_DATA	*pcigoto;
    //------------------------------
    	pcigoto=PciIfrInfo->PciDevSetupData[i];
    	if(pcigoto->ShowItem==FALSE) continue;
    	//Generate ID based on 0x6000 + AMI SDL Index i structure 
    	GotoId=PCI_GOTO_ID_BASE+(UINT16)i;
        
        // Add goto control
        HiiCreateGotoOpCode (
            PciIfrInfo->StartOpCodeHandle,
            PCI_DEV_FORM_ID,
            pcigoto->GotoStringId,
            pcigoto->GotoHelpStringId,
            EFI_IFR_FLAG_CALLBACK,
            GotoId
        );
    }

    HiiUpdateForm (
      gPciHiiHandle,
      &PciFormsetGuid,
      PCI_MAIN_FORM_ID,
      PciIfrInfo->StartOpCodeHandle,
      PciIfrInfo->EndOpCodeHandle
    );
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:  GetVarStoreID
//
// Description:
//  Returns associated with passed SioLd Volatile or Non Volatile varstoreid.
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_VARSTORE_ID GetVarStoreID(UINTN PciIdx, UINT8 VarType){
	PCI_VAR_ID	varid;
//-----------------------	
	varid.IdField.Always1=1;
	varid.IdField.DataType=VarType;
	varid.IdField.PciDevIndex=(UINT8)PciIdx;
	return (EFI_VARSTORE_ID)(varid.VAR_ID);
}

VOID PciSetupCreatePciLatencyOptions(VOID *OpcodeHandle){
	static EFI_STRING_ID	pcilatstr[]={
		STRING_TOKEN(STR_PCI_32), 	STRING_TOKEN(STR_PCI_64),
		STRING_TOKEN(STR_PCI_96),	STRING_TOKEN(STR_PCI_128), 
		STRING_TOKEN(STR_PCI_160),	STRING_TOKEN(STR_PCI_192),
		STRING_TOKEN(STR_PCI_224),	STRING_TOKEN(STR_PCI_248)
	};
	UINTN			i;
    UINT8			val;

//--------------------------------
	for (i=0, val=32; i<(sizeof(pcilatstr)/sizeof(EFI_STRING_ID)); i++, val+=32){
					HiiCreateOneOfOptionOpCode (
					OpcodeHandle,
					pcilatstr[i],
					EFI_IFR_FLAG_RESET_REQUIRED,
		            EFI_IFR_NUMERIC_SIZE_1,
		            val );
	}
}


VOID PciSetupCreatePcie1MaxRrOptions(VOID *OpcodeHandle){
	static EFI_STRING_ID	mrrstr[]={
		STRING_TOKEN(STR_MP128), 	STRING_TOKEN(STR_MP256),
		STRING_TOKEN(STR_MP512),	STRING_TOKEN(STR_MP512), 
		STRING_TOKEN(STR_MP1024),	STRING_TOKEN(STR_MP2048),
		STRING_TOKEN(STR_MP4096)
	};
	UINTN			i;
//--------------------------------
    //First Create AUTO Option
	//*DefaultOption=OpcodeHandle;
    HiiCreateOneOfOptionOpCode (
			OpcodeHandle,
			STRING_TOKEN(STR_AUTO),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            PCI_SETUP_AUTO_VALUE );
    
    
	for (i=0; i<(sizeof(mrrstr)/sizeof(EFI_STRING_ID)); i++){
			HiiCreateOneOfOptionOpCode (
					OpcodeHandle,
					mrrstr[i],
					EFI_IFR_FLAG_RESET_REQUIRED,
		            EFI_IFR_NUMERIC_SIZE_1,
		            (UINT8)i);
	}
}

VOID PciSetupCreatePaddingOptions(VOID *OpcodeHandle, UINT8 OpType){
	static EFI_STRING_ID	busstr[]={
		STRING_TOKEN(STR_DISABLED),	
		STRING_TOKEN(STR_1), 	STRING_TOKEN(STR_2),
		STRING_TOKEN(STR_3),	STRING_TOKEN(STR_4), 
		STRING_TOKEN(STR_5),	STRING_TOKEN(STR_6),
		STRING_TOKEN(STR_7),	STRING_TOKEN(STR_8),
		STRING_TOKEN(STR_9),	STRING_TOKEN(STR_10)
	};
	
	static EFI_STRING_ID	iostr[]={
		STRING_TOKEN(STR_DISABLED),	
		STRING_TOKEN(STR_4K), 	STRING_TOKEN(STR_8K),
		STRING_TOKEN(STR_16K),	STRING_TOKEN(STR_32K) 
	};
	
	static EFI_STRING_ID	memstr[]={
		STRING_TOKEN(STR_DISABLED),	
		STRING_TOKEN(STR_1M), 	STRING_TOKEN(STR_2M),
		STRING_TOKEN(STR_4M),	STRING_TOKEN(STR_8M), 
		STRING_TOKEN(STR_16M),	STRING_TOKEN(STR_32M),
		STRING_TOKEN(STR_64M),	STRING_TOKEN(STR_128M),
		STRING_TOKEN(STR_256M),	STRING_TOKEN(STR_512M),
		STRING_TOKEN(STR_1G),	STRING_TOKEN(STR_2G),
		STRING_TOKEN(STR_4G),	STRING_TOKEN(STR_8G)
//		STRING_TOKEN(STR_16G),	STRING_TOKEN(STR_32G),
//		STRING_TOKEN(STR_64G),	STRING_TOKEN(STR_128G),
//		STRING_TOKEN(STR_256G),	STRING_TOKEN(STR_512G),
	};
	
	EFI_STRING_ID	*strid;
	UINTN			cnt,i;
	UINT64			base,val;
//--------------------------------
	//Based on Option padding type;
	switch(OpType){
		case optBus:
			strid=&busstr[0];
			cnt=sizeof(busstr)/sizeof(EFI_STRING_ID);
			base=0;
		break;
		case optIo:
			strid=&iostr[0];
			cnt=sizeof(iostr)/sizeof(EFI_STRING_ID);
			base=0x800; 
		break;
		case optMmio:
			strid=&memstr[0];
			cnt=(sizeof(memstr)/sizeof(EFI_STRING_ID))-4;
			base=0x80000;
		break;
		
		case optMmio64:
			strid=&memstr[0];
			cnt=(sizeof(memstr)/sizeof(EFI_STRING_ID));
			base=0x80000;
		break;
	}
	
	for (i=0; i<cnt; i++){
			if(OpType==optBus) val=i;
			else {
				if(i==0)val=i;
				else val=Shl64(base,(UINT8)i); 
			}
			
			HiiCreateOneOfOptionOpCode (
					OpcodeHandle,
					strid[i],
					EFI_IFR_FLAG_RESET_REQUIRED,
		            EFI_IFR_NUMERIC_SIZE_8,
		            val);
	}
}

VOID UpdateDeviceInfoString(PCI_DEV_SETUP_GOTO_DATA	*GotoData, 
		BOOLEAN Pcie1, BOOLEAN Pcie2, BOOLEAN HotPlug, BOOLEAN Ari)
{
	UINT16	format[100]={0};
	UINT16	string[100]={0};
	EFI_STRING s=&string[0];
	EFI_STRING f=&format[0];
//-------------------------	
	//1. Empty Slot here with disabled bridge... 
	if(GotoData->PciDevInfo==NULL && GotoData->ParentBrgInfo==NULL){
		s=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_DEV_INFO_EMPTY_STR1),NULL); 
		HiiSetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DEV_SUBTITLE1),s, NULL);

		s=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_DEV_INFO_EMPTY_STR2),NULL); 
		HiiSetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DEV_SUBTITLE2),s, NULL);
		return; 
	}
	
			
	if(GotoData->PciDevInfo!=NULL || GotoData->ParentBrgInfo!=NULL){
		EFI_STRING	onx=L"X";
		EFI_STRING	off=L" ";
		EFI_STRING	g1, g2, ari, hp;
	//----------------------------
		f=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_DEV_INFO_STR1),NULL);
		
		if(GotoData->PciDevInfo==NULL){
			PCI_BRG_EXT	 *brg=&((PCI_BRG_INFO*)(GotoData->ParentBrgInfo))->Bridge; 
		//----------------------
		       	Swprintf(s, f, brg->Res[rtBus].Base, 
       			GotoData->PciSdlInfo->Device,GotoData->PciSdlInfo->Function,
       			0xFFFF, 0xFFFF);
		} else {
	       	Swprintf(s, f, GotoData->PciDevInfo->Address.Addr.Bus, 
	       			GotoData->PciDevInfo->Address.Addr.Device,GotoData->PciDevInfo->Address.Addr.Function,
	       			GotoData->PciDevInfo->DevVenId.VenId, GotoData->PciDevInfo->DevVenId.DevId);
			
		}
		HiiSetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DEV_SUBTITLE1),string, NULL);

		if(Pcie1){
			g1=onx;
			if(Pcie2)g2=onx;
			else g2=off;
		}
		else{
			g1=off;
			g2=off;
		}
		
		if(HotPlug)hp=onx;
		else hp=off;
		
		if(Ari)ari=onx;
		else ari=off;
		
		//Feature String
		f=HiiGetString(gPciHiiHandle,STRING_TOKEN(STR_DEV_INFO_STR2),NULL);		
       	Swprintf(s, f, g1,g2,ari,hp); 
	
		HiiSetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DEV_SUBTITLE2),s, NULL);
	}	
	
	return; 
	
}

EFI_STATUS UpdateHpForm(PCI_DEV_SETUP_GOTO_DATA	*GotoData)
{
    EFI_VARSTORE_ID		vsid;
//----------------------------	
    if (gPciDevForm.PciHpStartOpCodeHandle != NULL)
    {
        HiiFreeOpCodeHandle (gPciDevForm.PciHpStartOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.PciHpEndOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.BusPaddOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.IoPaddOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.Mmio32OpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.Mmio32PfOpCodeHandle);
//        HiiFreeOpCodeHandle (gPciDevForm.Mmio64OpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.Mmio64PfOpCodeHandle);
    }
    
    gPciDevForm.PciHpStartOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.PciHpEndOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.BusPaddOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.IoPaddOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.Mmio32OpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.Mmio32PfOpCodeHandle = HiiAllocateOpCodeHandle ();
//    gPciDevForm.Mmio64OpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.Mmio64PfOpCodeHandle = HiiAllocateOpCodeHandle ();
    
    // Create Hii Extended Label OpCode as the start and end opcode For PCI DevForm
    gPciDevForm.PciHpStartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.PciHpStartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.PciHpStartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.PciHpEndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.PciHpEndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.PciHpEndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.PciHpStartLabel->Number = PCI_HP_LABEL_START;
    gPciDevForm.PciHpEndLabel->Number = PCI_HP_LABEL_END;

    //Get Varstor id for this item...
    vsid=GetVarStoreID(GotoData->PciDevIdx,SDATA_TYPE_HOTPL);
    
    //Now create HP Padding subtitle...
    //1. PCIe Device Settings subtitle
    HiiCreateSubTitleOpCode(gPciDevForm.PciHpStartOpCodeHandle,
    		STRING_TOKEN(STR_PCI_HP_SUB),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
    
    //2a Now Bus Padding OneOf Options...
    PciSetupCreatePaddingOptions(gPciDevForm.BusPaddOpCodeHandle,optBus);
	//2b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.PciHpStartOpCodeHandle,			//*OpCodeHandle
		PCIHP_BUS_PADD_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_HP_SETUP_DATA, BusPadd),//Offset
        STRING_TOKEN(STR_BUS_PADD_PROMPT),			//Prompt
        STRING_TOKEN(STR_BUS_PADD_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_8,						//OneOfFlags
        gPciDevForm.BusPaddOpCodeHandle,			//OptionsOpCodeHandle
        NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);
    
    //3a Now IO Padding OneOf Options...
    PciSetupCreatePaddingOptions(gPciDevForm.IoPaddOpCodeHandle,optIo);
	//3b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.PciHpStartOpCodeHandle,			//*OpCodeHandle
		PCIHP_IO_PADD_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_HP_SETUP_DATA, IoPadd),//Offset
        STRING_TOKEN(STR_IO_PADD_PROMPT),			//Prompt
        STRING_TOKEN(STR_IO_PADD_HELP),				//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_8,						//OneOfFlags
        gPciDevForm.IoPaddOpCodeHandle,			//OptionsOpCodeHandle
        NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);

    //4a Now MMIO32 Padding OneOf Options...
    PciSetupCreatePaddingOptions(gPciDevForm.Mmio32OpCodeHandle,optMmio);
	//4b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.PciHpStartOpCodeHandle,			//*OpCodeHandle
		PCIHP_MMIO32_PADD_QID,						//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_HP_SETUP_DATA, Mmio32Padd),//Offset
        STRING_TOKEN(STR_MMIO32_PADD_PROMPT),		//Prompt
        STRING_TOKEN(STR_MMIO32_PADD_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_8,						//OneOfFlags
        gPciDevForm.Mmio32OpCodeHandle,				//OptionsOpCodeHandle
        NULL//defopt             					//*DefaultsOpCodeHandle  OPTIONAL
	);
	
    //5a Now MMIO32PF Padding OneOf Options...
    PciSetupCreatePaddingOptions(gPciDevForm.Mmio32PfOpCodeHandle,optMmio);
	//5b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.PciHpStartOpCodeHandle,			//*OpCodeHandle
		PCIHP_MMIO32PF_PADD_QID,					//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_HP_SETUP_DATA, Mmio32PfPadd),//Offset
        STRING_TOKEN(STR_MMIO32PF_PADD_PROMPT),		//Prompt
        STRING_TOKEN(STR_MMIO32PF_PADD_HELP),		//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_8,						//OneOfFlags
        gPciDevForm.Mmio32PfOpCodeHandle,				//OptionsOpCodeHandle
        NULL//defopt             					//*DefaultsOpCodeHandle  OPTIONAL
	);
	
	//Now we should create a suppress if option for 64bit resources...
	//Create Suppress if opcode based on Global Settings PCI_COMMON_SETUP_DATA.Above4gDecode
    AmiHiiCreateSuppresGrayIdVal(gPciDevForm.PciHpStartOpCodeHandle, PCI_CLOBAL_4G_QID, 0,TRUE);	
	//Create Suppress if opcode based on Local Settings PCI_DEV_SETUP_DATA.Disable4gDecode
	AmiHiiCreateSuppresGrayIdVal(gPciDevForm.PciHpStartOpCodeHandle, PCI_LOCAL_4G_QID, 1,TRUE);	

/*    
    //6a Now MMIO64 Padding OneOf Options...
    PciSetupCreatePaddingOptions(gPciDevForm.Mmio64OpCodeHandle,optMmio64);
	//6b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.PciHpStartOpCodeHandle,			//*OpCodeHandle
		PCIHP_MMIO64_PADD_QID,						//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_HP_SETUP_DATA, Mmio64Padd),//Offset
        STRING_TOKEN(STR_MMIO64_PADD_PROMPT),		//Prompt
        STRING_TOKEN(STR_MMIO64_PADD_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_8,						//OneOfFlags
        gPciDevForm.Mmio64OpCodeHandle,				//OptionsOpCodeHandle
        NULL//defopt             					//*DefaultsOpCodeHandle  OPTIONAL
	);
*/	
	
    //7a Now MMIO64PF Padding OneOf Options...
    PciSetupCreatePaddingOptions(gPciDevForm.Mmio64PfOpCodeHandle,optMmio64);
	//7b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.PciHpStartOpCodeHandle,			//*OpCodeHandle
		PCIHP_MMIO64PF_PADD_QID,					//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_HP_SETUP_DATA, Mmio64PfPadd),//Offset
        STRING_TOKEN(STR_MMIO64PF_PADD_PROMPT),		//Prompt
        STRING_TOKEN(STR_MMIO64PF_PADD_HELP),		//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_8,						//OneOfFlags
        gPciDevForm.Mmio64PfOpCodeHandle,				//OptionsOpCodeHandle
        NULL//defopt             					//*DefaultsOpCodeHandle  OPTIONAL
	);

    //8. PCI_HOTPLUG_WARNING subtitle
    HiiCreateSubTitleOpCode(gPciDevForm.PciHpStartOpCodeHandle,
    		STRING_TOKEN(STR_EMPTY),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
	
    HiiCreateSubTitleOpCode(gPciDevForm.PciHpStartOpCodeHandle,
    		STRING_TOKEN(STR_PCI_HOTPLUG_WARNING),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
	
    AmiHiiTerminateScope(gPciDevForm.PciHpStartOpCodeHandle);
    AmiHiiTerminateScope(gPciDevForm.PciHpStartOpCodeHandle);
	
    //We are done!!!!! Update the form...
    HiiUpdateForm (
      gPciHiiHandle,
      &PciFormsetGuid,
      PCI_HOTPLUG_FORM_ID,
      gPciDevForm.PciHpStartOpCodeHandle,
      gPciDevForm.PciHpEndOpCodeHandle
    );
    return EFI_SUCCESS;
}


EFI_STATUS UpdatePcie2Form(PCI_DEV_SETUP_GOTO_DATA	*GotoData)
{
    EFI_VARSTORE_ID		vsid;
//    VOID				*defopt=NULL;
//----------------------------	
    if (gPciDevForm.Pcie2StartOpCodeHandle != NULL)
    {
        HiiFreeOpCodeHandle (gPciDevForm.Pcie2StartOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.Pcie2EndOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.ComplTimeoutOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.LnkSpeedOpCodeHandle);
    }
    
    gPciDevForm.Pcie2StartOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.Pcie2EndOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.ComplTimeoutOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.LnkSpeedOpCodeHandle = HiiAllocateOpCodeHandle ();
    
    // Create Hii Extended Label OpCode as the start and end opcode For PCI DevForm
    gPciDevForm.Pcie2StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.Pcie2StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.Pcie2StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.Pcie2EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.Pcie2EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.Pcie2EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.Pcie2StartLabel->Number = PCIE2_LABEL_START;
    gPciDevForm.Pcie2EndLabel->Number = PCIE2_LABEL_END;

    //Get Varstor id for this item...
    vsid=GetVarStoreID(GotoData->PciDevIdx,SDATA_TYPE_PCIE2);
    
    //Now create PCIE GEN 2 controls...
    //1. PCIe Device Settings subtitle
    HiiCreateSubTitleOpCode(gPciDevForm.Pcie2StartOpCodeHandle,
    		STRING_TOKEN(STR_PCIE2_DEVICE),
    		STRING_TOKEN(STR_EMPTY),
            0,0);

    //2. Now goes Completion Timeout selection...
    //2a. It has 4 OneOf options... 
    //defopt=gPciDevForm.ComplTimeoutOpCodeHandle; //Default is "STR_DEFAULT"
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.ComplTimeoutOpCodeHandle,
			STRING_TOKEN(STR_DEFAULT),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0xFF );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.ComplTimeoutOpCodeHandle,
			STRING_TOKEN(STR_SHORT),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x55 );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.ComplTimeoutOpCodeHandle,
			STRING_TOKEN(STR_LONG),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0xAA );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.ComplTimeoutOpCodeHandle,
			STRING_TOKEN(STR_DISABLED),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x0 );
	//2b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,			//*OpCodeHandle
		PCIE2_COMPL_TIMEOUT_QID,					//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, ComplTimeOut),//Offset
        STRING_TOKEN(STR_DEV_COMPL_TIMEOUT_PROMPT),	//Prompt
        STRING_TOKEN(STR_DEV_COMPL_TIMEOUT_HELP),	//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.ComplTimeoutOpCodeHandle,			//OptionsOpCodeHandle
        NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);

    //3. ARI Forwarding  Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_ARI_FWD_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, AriFwd),
        STRING_TOKEN(STR_DEV_ARI_PROMPT),
        STRING_TOKEN(STR_DEV_ARI_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
    //4. Atomic Operations ... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_AOP_REQ_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, AtomOpReq),
        STRING_TOKEN(STR_DEV_AOP_REQ_PROMPT),
        STRING_TOKEN(STR_DEV_AOP_REQ_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );

	//5. Atomic Operation Egress... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_AOP_EGRES_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, AtomOpEgressBlk),
        STRING_TOKEN(STR_DEV_AOP_EGRESS_BLK_PROMPT),
        STRING_TOKEN(STR_DEV_AOP_EGRESS_BLK_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );

	//6. IDO Req... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_IDO_REQ_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, IDOReq),
        STRING_TOKEN(STR_DEV_IDO_REQ_PROMPT),
        STRING_TOKEN(STR_DEV_IDO_REQ_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
	//7. IDO Completion... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_IDO_COMPL_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, IDOCompl),
        STRING_TOKEN(STR_DEV_IDO_COMPL_PROMPT),
        STRING_TOKEN(STR_DEV_IDO_COMPL_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
	//8. LTR Reporting... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_LTR_REP_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, LtrReport),
        STRING_TOKEN(STR_DEV_LTR_REPORT_PROMPT),
        STRING_TOKEN(STR_DEV_LTR_REPORT_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
	//9. Egress 2 Egress TLP Prefix Block... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_E2E_TLP_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, E2ETlpPrBlk),
        STRING_TOKEN(STR_DEV_E2E_TLP_BLK_PROMPT),
        STRING_TOKEN(STR_DEV_E2E_TLP_BLK_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );

	
	//10. Now goes separator and Subtitle with Link Control options.
    //Separator...
	HiiCreateSubTitleOpCode(gPciDevForm.Pcie2StartOpCodeHandle,
    		STRING_TOKEN(STR_EMPTY),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
	//10a
    HiiCreateSubTitleOpCode(gPciDevForm.Pcie2StartOpCodeHandle,
    		STRING_TOKEN(STR_PCIE2_LINK),
    		STRING_TOKEN(STR_EMPTY),
            0,0);

    //11. Now goes Link Speed Selection... selection...
    //11a. It has 3 OneOf options... 
    //defopt=gPciDevForm.ComplTimeoutOpCodeHandle; //Default is "STR_AUTO"
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkSpeedOpCodeHandle,
			STRING_TOKEN(STR_AUTO),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            PCI_SETUP_AUTO_VALUE );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkSpeedOpCodeHandle,
			STRING_TOKEN(STR_2_5G),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x1 );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkSpeedOpCodeHandle,
			STRING_TOKEN(STR_5_0G),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x2 );
	//11b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,			//*OpCodeHandle
		PCIE2_LNK_SPEED_QID,						//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, LnkSpeed),//Offset
        STRING_TOKEN(STR_LNK_SPEED_PROMPT),			//Prompt
        STRING_TOKEN(STR_LNK_SPEED_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.LnkSpeedOpCodeHandle,			//OptionsOpCodeHandle
        NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);

	//12. Compliance SOS... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_COMPL_SOS_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, ComplSos),
        STRING_TOKEN(STR_LNK_COMPL_SOS_PROMPT),
        STRING_TOKEN(STR_LNK_COMPL_SOS_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
   
	//13. HW Auto Width Disable... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_HW_AW_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, HwAutoWidth),
        STRING_TOKEN(STR_LNK_HW_AUTO_WIDTH_PROMPT),
        STRING_TOKEN(STR_LNK_HW_AUTO_WIDTH_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );

	//14. HW Auto Width Disable... Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie2StartOpCodeHandle,
		PCIE2_HW_AS_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE2_SETUP_DATA, HwAutoSpeed),
        STRING_TOKEN(STR_LNK_HW_AUTO_SPEED_PROMPT),
        STRING_TOKEN(STR_LNK_HW_AUTO_SPEED_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
    
    //We are done!!!!! Update the form...
    HiiUpdateForm (
      gPciHiiHandle,
      &PciFormsetGuid,
      PCIE2_FORM_ID,
      gPciDevForm.Pcie2StartOpCodeHandle,
      gPciDevForm.Pcie2EndOpCodeHandle
    );
    
    return EFI_SUCCESS;
}

EFI_STATUS UpdatePcie1Form(PCI_DEV_SETUP_GOTO_DATA	*GotoData)
{
    EFI_VARSTORE_ID		vsid;
//    VOID				*defopt=NULL;
//----------------------------	
    if (gPciDevForm.Pcie1StartOpCodeHandle != NULL)
    {
        HiiFreeOpCodeHandle (gPciDevForm.Pcie1StartOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.Pcie1EndOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.MaxPayLoadOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.MaxReadReqOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.AspmModeOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.LnkTrainRetryOpCodeHandle);
    }
    
    gPciDevForm.Pcie1StartOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.Pcie1EndOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.MaxPayLoadOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.MaxReadReqOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.AspmModeOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.LnkTrainRetryOpCodeHandle = HiiAllocateOpCodeHandle ();
    
    // Create Hii Extended Label OpCode as the start and end opcode For PCI DevForm
    gPciDevForm.Pcie1StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.Pcie1StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.Pcie1StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.Pcie1EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.Pcie1EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.Pcie1EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.Pcie1StartLabel->Number = PCIE1_LABEL_START;
    gPciDevForm.Pcie1EndLabel->Number = PCIE1_LABEL_END;

    //Get Varstor id for this item...
    vsid=GetVarStoreID(GotoData->PciDevIdx,SDATA_TYPE_PCIE1);
    
    //Now create PCIE GEN 1 controls...
    //1. PCIe Device Settings subtitle
    HiiCreateSubTitleOpCode(gPciDevForm.Pcie1StartOpCodeHandle,
    		STRING_TOKEN(STR_PCIE_DEVICE),
    		STRING_TOKEN(STR_EMPTY),
            0,0);

    //2. Relaxed Ordering Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,
		PCIE1_RELAXED_ORD_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, RelaxedOrdering),
        STRING_TOKEN(STR_PCIE_RELAXEDORDERING_PROMPT),
        STRING_TOKEN(STR_PCIE_RELAXEDORDERING_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
 
    //3. Extended Tag Field Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,
		PCIE1_EXT_TAG_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, ExtTagField),
        STRING_TOKEN(STR_PCIE_EXTTAGFLD_PROMPT),
        STRING_TOKEN(STR_PCIE_EXTTAGFLD_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
    //4. No Snoop Check Box...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,
		PCIE1_NO_SNOOP_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, NoSnoop),
        STRING_TOKEN(STR_PCIE_NOSNOOP_PROMPT),
        STRING_TOKEN(STR_PCIE_NOSNOOP_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        1, // Enable - the default settings.
        NULL
    );
	
	//5. Now create MPL OneOf Items and control
	PciSetupCreatePcie1MaxRrOptions(gPciDevForm.MaxPayLoadOpCodeHandle);
	//Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,			//*OpCodeHandle
		PCI_PCIE1_MPL_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, MaxPayload),//Offset
        STRING_TOKEN(STR_PCIE_MAXPAYLOAD_PROMPT),	//Prompt
        STRING_TOKEN(STR_PCIE_MAXPAYLOAD_HELP),		//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.MaxPayLoadOpCodeHandle,			//OptionsOpCodeHandle
		NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);
	
	//6. Now create MRR OneOf Items and control
	PciSetupCreatePcie1MaxRrOptions(gPciDevForm.MaxReadReqOpCodeHandle);
	//Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,			//*OpCodeHandle
		PCI_PCIE1_MRR_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, MaxReadRequest),//Offset
        STRING_TOKEN(STR_PCIE_MAXREADREQUEST_PROMPT),//Prompt
        STRING_TOKEN(STR_PCIE_MAXREADREQUEST_HELP),	//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.MaxReadReqOpCodeHandle,			//OptionsOpCodeHandle
		NULL             							//*DefaultsOpCodeHandle  OPTIONAL
	);

	//7. Now goes separator and Subtitle with Link Control options.
    //Separator...
	HiiCreateSubTitleOpCode(gPciDevForm.Pcie1StartOpCodeHandle,
    		STRING_TOKEN(STR_EMPTY),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
	//
    HiiCreateSubTitleOpCode(gPciDevForm.Pcie1StartOpCodeHandle,
    		STRING_TOKEN(STR_PCIE_LINK),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
    //8. Now goes ASPM Mode selection...
    //8a. It has 3 one of options and
    //defopt=gPciDevForm.AspmModeOpCodeHandle; //Default is disabled.
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.AspmModeOpCodeHandle,
			STRING_TOKEN(STR_DISABLED),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x0 );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.AspmModeOpCodeHandle,
			STRING_TOKEN(STR_AUTO),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            PCI_SETUP_AUTO_VALUE ); 
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.AspmModeOpCodeHandle,
			STRING_TOKEN(STR_FORCE_L0),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x1 );
	//8b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,			//*OpCodeHandle
		PCI_PCIE1_ASPM_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, AspmMode),//Offset
        STRING_TOKEN(STR_PCIE_ASPM_PROMPT),			//Prompt
        STRING_TOKEN(STR_PCIE_ASPM_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.AspmModeOpCodeHandle,			//OptionsOpCodeHandle
		NULL             							//*DefaultsOpCodeHandle  OPTIONAL
	);
    
    //9. Now Extended Sync CheckBox...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,
		PCIE1_EXT_SYNC_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, ExtendedSynch),
        STRING_TOKEN(STR_PCIE_EXTD_SYNCH_PROMPT),
        STRING_TOKEN(STR_PCIE_EXTD_SYNCH_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
    //10. Now Clock PM CheckBox...
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,
		PCIE1_CLOCK_PM_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, ClockPm),
        STRING_TOKEN(STR_LNK_CLOCK_PM_PROMPT),
        STRING_TOKEN(STR_LNK_CLOCK_PM_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );
	
    //11. Now goes Link Training Retry Count selection...
    //11a. It has 4 one of options and
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkTrainRetryOpCodeHandle,
			STRING_TOKEN(STR_DISABLED),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x0 );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkTrainRetryOpCodeHandle,
			STRING_TOKEN(STR_2),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x2 );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkTrainRetryOpCodeHandle,
			STRING_TOKEN(STR_3),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x3 );
    HiiCreateOneOfOptionOpCode (
    		gPciDevForm.LnkTrainRetryOpCodeHandle,
			STRING_TOKEN(STR_5),
			EFI_IFR_FLAG_RESET_REQUIRED,
            EFI_IFR_NUMERIC_SIZE_1,
            0x5 );
	//11b. Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,			//*OpCodeHandle
		PCIE1_TR_RETRY_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, LnkTrRetry),//Offset
        STRING_TOKEN(STR_LNK_TR_RETRY_PROMPT),		//Prompt
        STRING_TOKEN(STR_LNK_TR_RETRY_HELP),		//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.LnkTrainRetryOpCodeHandle,		//OptionsOpCodeHandle
		NULL             							//*DefaultsOpCodeHandle  OPTIONAL
	);

	//12 Now create NUMERIC Link Training Timeout option
	HiiCreateNumericOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,			//*OpCodeHandle
		PCIE1_TR_TIMEOUT_QID,						//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, LnkTrTimeout),//Offset
        STRING_TOKEN(STR_LNK_TR_TIMEOUT_PROMPT),		//Prompt
        STRING_TOKEN(STR_LNK_TR_TIMEOUT_HELP),		//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_2,            			//NumericFlags,
        10,							           		//Minimum,
        10000,           							//Maximum,
        10,											//Step,
	  	NULL             							//*DefaultsOpCodeHandle  OPTIONAL
	  );
	
	//13. Last option Disable Empty Links
	HiiCreateCheckBoxOpCode (
		gPciDevForm.Pcie1StartOpCodeHandle,
		PCIE1_EMPTY_LNK_QID,
        vsid,
        EFI_FIELD_OFFSET(PCIE1_SETUP_DATA, LnkDisable),
        STRING_TOKEN(STR_LNK_UNPOPULATED_PROMPT),
        STRING_TOKEN(STR_LNK_UNPOPULATED_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0, // Disable - the default settings.
        NULL
    );

	
    //We are done!!!!! Update the form...
    HiiUpdateForm (
      gPciHiiHandle,
      &PciFormsetGuid,
      PCIE1_FORM_ID,
      gPciDevForm.Pcie1StartOpCodeHandle,
      gPciDevForm.Pcie1EndOpCodeHandle
    );

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:  UpdatePciDevForm
//
// Description:
//  Updates Logical Device Configuration Form with data applicable for 
//  selected Logical Device device
//
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS UpdatePciDevForm(
    IN EFI_QUESTION_ID KeyValue
)
{
    UINTN  i=(UINTN)(KeyValue & (~PCI_GOTO_ID_BASE));
    PCI_DEV_SETUP_GOTO_DATA	*gotodata;
    EFI_STRING			s=NULL;
    EFI_VARSTORE_ID		vsid;
    BOOLEAN				pcie1, pcie2, hp, ari; 
//    VOID				*defopt=NULL;
//--------------------------------------------
//EFI_DEADLOOP();    
    
	//Check if it is PCIe1; PCie2 or Hp Form Update -
	//we have updated it already, just exit...
   	if(KeyValue>=PCIE1_GOTO_ID){
   		if(gCurrentPciDevData!=NULL){
			switch(KeyValue){
				case PCIE1_GOTO_ID:
					UpdatePcie1Form(gCurrentPciDevData);
					break;
				case PCIE2_GOTO_ID:
					UpdatePcie2Form(gCurrentPciDevData);
					break;
				case PCI_HP_GOTO_ID:
					UpdateHpForm(gCurrentPciDevData);
					break;
			}
   		}
   		return EFI_SUCCESS;
   	}
    //Using KeyValue - locate Goto Form Data...
    gotodata=gPciIfrInfo.PciDevSetupData[i];   
    gCurrentPciDevData=gotodata;

    // Create new OpCode Handle
    if (gPciDevForm.DevStartOpCodeHandle != NULL)
    {
        HiiFreeOpCodeHandle (gPciDevForm.DevStartOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.DevEndOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.PciLatencyOpCodeHandle);
        HiiFreeOpCodeHandle (gPciDevForm.PciXLatencyOpCodeHandle);
    }
    
    //Update Caption String...
    s = MallocZ(160); 
    if(s==NULL) return EFI_OUT_OF_RESOURCES;
    
    s=HiiGetString(gPciHiiHandle, gotodata->GotoStringId, NULL);
    HiiSetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DEV_SUBTITLE),s, NULL);
    
	//Now check if device or slot supports PCI EXPRESS and HP features and what kind.  
	pcie1=FALSE;
	pcie2=FALSE;
	hp=FALSE;
	ari=FALSE;
	if(gotodata->PciDevInfo!=NULL){
		//if Slot is Populated or OnBoard Device we should have this Dilled in...
		// Get PciE Caps from device itself....
//		pcie1=(BOOLEAN)((gotodata->PciDevInfo)->PciExpress);
//		pcie2=(BOOLEAN)((gotodata->PciDevInfo)->PciExpress->Pcie2);
		if ((gotodata->PciDevInfo)->PciExpress)  pcie1 = TRUE ; else  pcie1 = FALSE;
		if ((gotodata->PciDevInfo)->PciExpress->Pcie2) pcie2 = TRUE ; else  pcie2 = FALSE  ;
		if(pcie2){
//			ari=(BOOLEAN)((gotodata->PciDevInfo)->PciExpress->AriData); //Siva
			ari  = ((gotodata->PciDevInfo)->PciExpress->AriData) ? TRUE : FALSE;
		}
	} else {
		//the upstream bridge must be a PCIe Device as well.
		if(gotodata->ParentBrgInfo!=NULL){
//			pcie1=(BOOLEAN)((gotodata->ParentBrgInfo)->PciExpress);
//			pcie2=(BOOLEAN)((gotodata->ParentBrgInfo)->PciExpress->Pcie2);
			if ((gotodata->ParentBrgInfo)->PciExpress!=NULL)  pcie1 = TRUE ; 
			else  pcie1 = FALSE;
			if ((gotodata->ParentBrgInfo)->PciExpress->Pcie2!=NULL) pcie2 = TRUE ; 
			else  pcie2 = FALSE  ;
			if(pcie2){
//				ari=(BOOLEAN)((gotodata->ParentBrgInfo)->PciExpress->AriData);
				if ((gotodata->ParentBrgInfo)->PciExpress->AriData!=NULL) ari = TRUE;  
				else ari = FALSE;
			}
		} else {
			//Last chance here we are judging about PCIe capability of the slot by it's SDL data.
//			pcie1=((gotodata->PciSdlInfo)->PciDevFlags.PciExpress); //Siva
			if ((gotodata->PciSdlInfo)->PciDevFlags.Bits.PciExpress) pcie1 = TRUE ; 
			else pcie1 = FALSE;  //Siva
			pcie2=pcie1;
			if ((gotodata->PciSdlInfo)->PciDevFlags.Bits.Ari) ari = TRUE ; 
			else ari = FALSE;  //Siva
		}
	}
	
	
	//Now check if device or slot supports HP features and what kind.  
	//Check if it Slot with Enabled bridge, bridge has to have HotPlug flag set..
	if((gotodata->PciSdlInfo)->PciDevFlags.Bits.OnBoard){ 
		//for on board devices - it must be a bridge to configure HP settings.
		hp=((gotodata->PciSdlInfo)->PciDevFlags.Bits.HotPlug);
	} else {
		//Check if it Slot with Enabled bridge, bridge has to have HotPlug flag set..
		if(gotodata->ParentBrgInfo!=NULL){
			hp=(BOOLEAN)((gotodata->ParentBrgInfo)->AmiSdlPciDevData->PciDevFlags.Bits.HotPlug);
		}
	}	

    //Update Device Info String #2    
	UpdateDeviceInfoString(gotodata, pcie1, pcie2, hp, ari);
    
    gPciDevForm.DevStartOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.DevEndOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.PciLatencyOpCodeHandle = HiiAllocateOpCodeHandle ();
    gPciDevForm.PciXLatencyOpCodeHandle = HiiAllocateOpCodeHandle ();
    
    // Create Hii Extended Label OpCode as the start and end opcode For PCI DevForm
    gPciDevForm.DevStartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.DevStartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.DevStartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.DevEndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		gPciDevForm.DevEndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    gPciDevForm.DevEndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    gPciDevForm.DevStartLabel->Number = PCI_DEV_LABEL_START;
    gPciDevForm.DevEndLabel->Number = PCI_DEV_LABEL_END;

    //Form Varstore ID for PCIDevice Data Storage
    vsid=GetVarStoreID(gotodata->PciDevIdx,SDATA_TYPE_DEVICE);
	PCI_TRACE((TRACE_PCI,"PciSetup: PCI Dev varstoreid=0x%X.\n.",vsid));
    
    //1. Create PCI Latency timer OneOf items
	PciSetupCreatePciLatencyOptions(gPciDevForm.PciLatencyOpCodeHandle);
	//Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.DevStartOpCodeHandle,			//*OpCodeHandle
		PCI_PCI_LAT_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, PciLatency),//Offset
        STRING_TOKEN(STR_PCI_LATENCY_PROMPT),		//Prompt
        STRING_TOKEN(STR_PCI_LATENCY_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.PciLatencyOpCodeHandle,			//OptionsOpCodeHandle
		NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);

    //2. Create PCI-X Latency timer OneOf items
	PciSetupCreatePciLatencyOptions(gPciDevForm.PciXLatencyOpCodeHandle);
	//Now create OneOf OpCode...
	HiiCreateOneOfOpCode (
		gPciDevForm.DevStartOpCodeHandle,			//*OpCodeHandle
		PCI_PCIX_LAT_QID,							//QuestionId
        vsid,										//VarStore ID
        EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, PciXLatency),//Offset
        STRING_TOKEN(STR_PCIX_LATENCY_PROMPT),		//Prompt
        STRING_TOKEN(STR_PCI_LATENCY_HELP),			//Help 
        EFI_IFR_FLAG_RESET_REQUIRED,				//QuestionFlags
        EFI_IFR_NUMERIC_SIZE_1,						//OneOfFlags
        gPciDevForm.PciXLatencyOpCodeHandle,		//OptionsOpCodeHandle
		NULL//defopt             							//*DefaultsOpCodeHandle  OPTIONAL
	);

	//3. VGA Pallete Snoop 
	HiiCreateCheckBoxOpCode (
		gPciDevForm.DevStartOpCodeHandle,
		PCI_PAL_SNOOP_QID,
        vsid,
        EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, VgaPallete),
        STRING_TOKEN(STR_PCI_VGASNOOP_PROMPT),
        STRING_TOKEN(STR_PCI_VGASNOOP_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0,
        NULL
    );

	//3. PERR#
	HiiCreateCheckBoxOpCode (
		gPciDevForm.DevStartOpCodeHandle,
		PCI_PCI_PERR_QID,
        vsid,
        EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, PerrEnable),
        STRING_TOKEN(STR_PCI_PERR_PROMPT),
        STRING_TOKEN(STR_PCI_PERR_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0,
        NULL
    );

	//4. SERR#
	HiiCreateCheckBoxOpCode (
		gPciDevForm.DevStartOpCodeHandle,
		PCI_PCI_SERR_QID,
        vsid,
        EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, SerrEnable),
        STRING_TOKEN(STR_PCI_SERR_PROMPT),
        STRING_TOKEN(STR_PCI_SERR_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0,
        NULL
    );

	//SEPARATOR
    HiiCreateSubTitleOpCode(gPciDevForm.DevStartOpCodeHandle,
    		STRING_TOKEN(STR_EMPTY),
    		STRING_TOKEN(STR_EMPTY),
            0,0);

    //Create Suppress if opcode based on Global Settings PCI_COMMON_SETUP_DATA.Above4gDecode
    AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_CLOBAL_4G_QID, 0,TRUE);	
    
    //Create Local 4g Decoding Check Box.
	HiiCreateCheckBoxOpCode (
		gPciDevForm.DevStartOpCodeHandle,
		PCI_LOCAL_4G_QID,
        vsid,
        EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, Decode4gDisable),
        STRING_TOKEN(STR_PCI_4G_LOCAL_PROMPT),
        STRING_TOKEN(STR_PCI_4G_LOCAL_HELP),
        EFI_IFR_FLAG_RESET_REQUIRED,
        0,
        NULL
    );
	AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);

	if(hp){
		//Create Suppress if opcode based on Global Settings PCI_COMMON_SETUP_DATA.Above4gDecode
		AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_CLOBAL_HP_QID, 0,TRUE);	
    
		//Create 
		HiiCreateCheckBoxOpCode (
			gPciDevForm.DevStartOpCodeHandle,
			PCI_LOCAL_HP_QID,
			vsid,
			EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, HpDisable),
			STRING_TOKEN(STR_HOTPLUG_ENABLE_PROMPT),
			STRING_TOKEN(STR_HOTPLUG_ENABLE_HELP),
			EFI_IFR_FLAG_RESET_REQUIRED,
			1,
			NULL
		);
		AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);
	}

	if(pcie1){
		HiiCreateCheckBoxOpCode (
			gPciDevForm.DevStartOpCodeHandle,
			PCI_LOCAL_PCIE1_QID,
			vsid,
			EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, Pcie1Disable),
			STRING_TOKEN(STR_PCIE1_ENABLE_PROMPT),
			STRING_TOKEN(STR_PCIE1_ENABLE_HELP),
			EFI_IFR_FLAG_RESET_REQUIRED,
			1,
			NULL
		);
		
		if(pcie2){
			AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_LOCAL_PCIE1_QID, 1,FALSE);	
			HiiCreateCheckBoxOpCode (
				gPciDevForm.DevStartOpCodeHandle,
				PCI_LOCAL_PCIE2_QID,
				vsid,
				EFI_FIELD_OFFSET(PCI_DEVICE_SETUP_DATA, Pcie2Disable),
				STRING_TOKEN(STR_PCIE2_ENABLE_PROMPT),
				STRING_TOKEN(STR_PCIE2_ENABLE_HELP),
				EFI_IFR_FLAG_RESET_REQUIRED,
				1,
				NULL
			);
			AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);
		}
	}
	
	//SEPARATOR
    HiiCreateSubTitleOpCode(gPciDevForm.DevStartOpCodeHandle,
    		STRING_TOKEN(STR_EMPTY),
    		STRING_TOKEN(STR_EMPTY),
            0,0);
    //based on discovered features create or not to create certain controls
    if(pcie1){
        // Add Local PCIe disable controls
    	
		AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_LOCAL_PCIE1_QID, 1,FALSE);	

        HiiCreateGotoOpCode (
       		gPciDevForm.DevStartOpCodeHandle,
       		PCIE1_FORM_ID,
       		STRING_TOKEN(STR_PCI_PCIE1_FORM),
       		STRING_TOKEN(STR_PCI_PCIE1_FORM_HELP),
            EFI_IFR_FLAG_CALLBACK,
            PCIE1_GOTO_ID
        );
        
		if(pcie2){
			AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_LOCAL_PCIE2_QID, 1,FALSE);	

			HiiCreateGotoOpCode (
	       		gPciDevForm.DevStartOpCodeHandle,
	       		PCIE2_FORM_ID,
	       		STRING_TOKEN(STR_PCI_PCIE2_FORM),
	       		STRING_TOKEN(STR_PCI_PCIE2_FORM_HELP),
	            EFI_IFR_FLAG_CALLBACK,
	            PCIE2_GOTO_ID
	        );
			AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);
		}
		AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);
	}
	
	if(hp){
		//Create Suppress if opcode based on Global Settings PCI_COMMON_SETUP_DATA.Above4gDecode
		AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_CLOBAL_HP_QID, 0,TRUE);	
		AmiHiiCreateSuppresGrayIdVal(gPciDevForm.DevStartOpCodeHandle, PCI_LOCAL_HP_QID, 1,FALSE);	
		//if we have both - Slots and Bridge setup Page we prefer to Configure the bridge
		//display instructions where to go and what to do...
		if(gotodata->ParentBrgInfo->AmiSdlPciDevData->PciDevFlags.Bits.HasSetup){
		    HiiCreateSubTitleOpCode(gPciDevForm.DevStartOpCodeHandle,
		    		STRING_TOKEN(STR_USE_BRG_HP_CONFIG1),
		    		STRING_TOKEN(STR_EMPTY),
		            0,0);
		    HiiCreateSubTitleOpCode(gPciDevForm.DevStartOpCodeHandle,
		    		STRING_TOKEN(STR_USE_BRG_HP_CONFIG2),
		    		STRING_TOKEN(STR_EMPTY),
		            0,0);
		} else {
			//Create Goto item to enter HP padding Page for the slot...
			HiiCreateGotoOpCode (
				gPciDevForm.DevStartOpCodeHandle,
				PCI_HOTPLUG_FORM_ID,
				STRING_TOKEN(STR_PCI_HOTPLUG_FORM),
				STRING_TOKEN(STR_PCI_HOTPLUG_FORM_HELP),
				EFI_IFR_FLAG_CALLBACK,
				PCI_HP_GOTO_ID
			);
	
		}
		//terminate conditional option
		AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);
		AmiHiiTerminateScope(gPciDevForm.DevStartOpCodeHandle);
	}
    //We are done!!
    HiiUpdateForm (
      gPciHiiHandle,
      &PciFormsetGuid,
      PCI_DEV_FORM_ID,
      gPciDevForm.DevStartOpCodeHandle,
      gPciDevForm.DevEndOpCodeHandle
    );

    if(s!=NULL)pBS->FreePool(s);
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:  InitIfrData
//
// Description:
//  Prepares the IFR labels within the SIO page for the controls insertion. It
//  also gets the list of AmiSio handles.
//
// Notes:
//  This call does not free the memory allocated earlier for SIO_IFR_INFO;
//  caller is responsible for freeing any memory before executing this function
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InitPciIfrData(PCI_IFR_INFO *PciIfrInfo){
    EFI_STATUS  				Status=EFI_SUCCESS;
    UINTN						PciDevHandleCount, i, j;
    EFI_HANDLE					*PciDevHandles=NULL;
	EFI_PCI_IO_PROTOCOL			*PciIo;
	PCI_DEV_INFO				*dev;
//-------------------------
    // Create new OpCode Handle
    PciIfrInfo->StartOpCodeHandle = HiiAllocateOpCodeHandle ();
    PciIfrInfo->EndOpCodeHandle = HiiAllocateOpCodeHandle ();

    // Create Hii Extend Label OpCode as the start opcode
    PciIfrInfo->StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
            PciIfrInfo->StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    PciIfrInfo->StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    PciIfrInfo->EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (
    		PciIfrInfo->EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
    PciIfrInfo->EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;

    PciIfrInfo->StartLabel->Number = PCI_GOTO_LABEL_START;
    PciIfrInfo->EndLabel->Number = PCI_GOTO_LABEL_END;

    // Update PCI Dev information
    Status = pBS->LocateHandleBuffer (ByProtocol,
        &gEfiPciIoProtocolGuid, NULL, &PciDevHandleCount, &PciDevHandles);

//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG
//EFI_DEADLOOP();
//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG
    
    //Now check devices with Handles this ones are Enabled and Active
	for(i=0; i<PciIfrInfo->PciDevCount; i++){
    	AMI_SDL_PCI_DEV_INFO	*sdlpcidev;
	//----------------------------------
		sdlpcidev=(PciIfrInfo->PciDevSetupData[i])->PciSdlInfo;

		for (j=0; j<PciDevHandleCount; j++){
			Status=pBS->HandleProtocol(PciDevHandles[j], &gEfiPciIoProtocolGuid, (VOID**)&PciIo); //Siva
			ASSERT_EFI_ERROR(Status);
    	
			dev=(PCI_DEV_INFO*)PciIo;
			//Here check if current PciIo instance belongs to the Func0 of the device on the slot...
			//On board devices could have setup for different functions though...
			if(!sdlpcidev->PciDevFlags.Bits.OnBoard && !IsFunc0(dev)) continue;
			
			//Check if we running with AMI deriver and can type cast data like that
			if(dev->Signature != AMI_PCI_SIG) return EFI_NOT_FOUND;
			if(dev->SdlDevCount==0) continue;
			if(dev->SdlDevCount==1){
				if(sdlpcidev==dev->AmiSdlPciDevData){
					(PciIfrInfo->PciDevSetupData[i])->PciDevInfo = dev;
				}
			} else {//Here dev->SdlDevCount>1
				//here if we have more than 1 SDL object matching Device
				UINTN					k;
				UINTN					idx;
				AMI_SDL_PCI_DEV_INFO	*tmpsdl;
			//--------------	
				for(k=0; k<dev->SdlDevCount; k++){
					idx=dev->SdlIdxArray[k];
					Status=AmiSdlFindIndexRecord(idx,&tmpsdl);
					if(sdlpcidev==tmpsdl){
						(PciIfrInfo->PciDevSetupData[i])->PciDevInfo = dev;
						break;
					}
				}
			}
			
    		if(sdlpcidev->ParentIndex==dev->SdlDevIndex){
    			(PciIfrInfo->PciDevSetupData[i])->ParentBrgInfo=dev;
    		}
    	}

		UpdateGotoStrings(PciIfrInfo->PciDevSetupData[i]);
	}
		
    if(PciDevHandles!=NULL) pBS->FreePool(PciDevHandles);
    
    SortData(&gPciIfrInfo);

	return Status;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:    PciBrowserCallback
// 
// Description:  This function is called by Setup browser to perform flash update
//               
// Input:   
//  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This - pointer to the instance of 
//                                                  ConfigAccess protocol
//  IN EFI_BROWSER_ACTION Action - action, that setup browser is performing
//  IN EFI_QUESTION_ID KeyValue - value of currently processed setup control
//  IN UINT8 Type - value type of setup control data
//  IN EFI_IFR_TYPE_VALUE *Value - pointer to setup control data
//  OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest - pointer where to store requested action
//
// Output:
//  EFI_SUCCESS - flash updated successfully
//  EFI_UNSUPPORTED - browser action is not supported
// 
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PciBrowserCallback(
    IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
    IN EFI_BROWSER_ACTION Action,
    IN EFI_QUESTION_ID KeyValue,
    IN UINT8 Type,
    IN EFI_IFR_TYPE_VALUE *Value,
    OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest
)
{
    EFI_STATUS Status=EFI_SUCCESS;
//---------------------------
	PCI_TRACE((TRACE_PCI,"PciSetup: BrowserCallback(Action %x, KeyValue %x, Type %x).\n", Action, KeyValue, Type));

    if (ActionRequest) 
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;

    if(Action != EFI_BROWSER_ACTION_CHANGING)
        return EFI_UNSUPPORTED;

    Status = UpdatePciDevForm(KeyValue);
    ASSERT_EFI_ERROR(Status);

    return Status;
}


VOID InitPciIfr(IN EFI_EVENT Event, IN VOID *Context)
{
	EFI_STATUS			Status;
//------------------------------------
    PCI_TRACE((TRACE_PCI, "SioSetup: InitSioIfr(Event 0x%X, Context @0x%X) = ",Event, Context));

    Status=InitPciIfrData (&gPciIfrInfo);

    PCI_TRACE((TRACE_PCI, "%r.\n",Status));
    
    if(EFI_ERROR(Status)) return;
    
    CreatePciGotoItems(&gPciIfrInfo);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	InitDriverVersionString
//
// Description:
//  This function Updates Driver version string with actual number.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID InitDriverVersionString()
{
    CHAR16 	s[80];
	CHAR16	d[40];
	EFI_STRING	pd=&d[0];
//-----------------------------	
	pd=HiiGetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DRIVER_VER_PROMPT), NULL);
    Swprintf(s, L"%s%02X.%02d.%02d", pd, PCI_BUS_MAJOR_VER, PCI_BUS_MINOR_VER, PCI_BUS_REVISION);
    HiiSetString(gPciHiiHandle, STRING_TOKEN(STR_PCI_DRIVER_VER), s, NULL);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SortData
//
// Description:
//  This will sort having setup devices items in Following order:
//  1. First go slots in Ascending order than on board devices...
//
// Input:       IN EFI_HANDLE ImageHandle - ImageHandle of the loaded driver
//              IN EFI_SYSTEM_TABLE SystemTable - Pointer to the System Table
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SortData(PCI_IFR_INFO* IfrData){
	T_ITEM_LIST	slt={0,0,NULL};
	T_ITEM_LIST	obd={0,0,NULL};
	EFI_STATUS	Status;
	UINTN		i;
	PCI_DEV_SETUP_GOTO_DATA	*gt;
	AMI_SDL_PCI_DEV_INFO	*sd;
	PCI_DEV_SETUP_GOTO_DATA	*tmpgt;
	AMI_SDL_PCI_DEV_INFO	*tmpsd;
	BOOLEAN					ap;
//------------------------------------	
//	EFI_DEADLOOP();
	while(IfrData->PciDevCount){
		gt=IfrData->PciDevSetupData[0];
		sd=gt->PciSdlInfo;
		
		//Remove item from unsorted array;
		DeleteItemLst((T_ITEM_LIST*)&IfrData->PciDevInitCnt, 0, FALSE);
		
		if(sd->PciDevFlags.Bits.OnBoard){
			//Add OnBoard device to the separate list, 
	        for(i=0, ap=TRUE; i<obd.ItemCount; i++){
	        	tmpgt=(PCI_DEV_SETUP_GOTO_DATA*)obd.Items[i];
	            //this is for hidden On Board Devices...
	            if(gt->PciDevInfo == NULL) break;
#if (PCI_SETUP_SORT_ONBOARD == 1)  //Sort By PCI Address...
            	if((tmpgt->PciDevInfo != NULL) && (tmpgt->PciDevInfo->Address.ADDR > gt->PciDevInfo->Address.ADDR) ){
	            	Status=InsertItemLst(&obd, gt, i);
	            	ap=FALSE;
	            	break;
            	}
#else 
#if (PCI_SETUP_SORT_ONBOARD == 2)  //Sort By Base Class Code...
               	if((tmpgt->PciDevInfo != NULL) && (tmpgt->PciDevInfo->Class.BaseClassCode > gt->PciDevInfo->Class.BaseClassCode)){
   	            	Status=InsertItemLst(&obd, gt, i);
   	            	ap=FALSE;
   	            	break;
               	}
#else	//Don't Sort 0 or 3...
            	break;
#endif
#endif
	        }//for
	        // if we apPEND Flag is set Slot # == the biggest one yet.
	        if(ap) Status = AppendItemLst(&obd, gt);
		} else {
		//-----------------	
	        for(i=0, ap=TRUE; i<slt.ItemCount; i++){
	        	tmpgt=(PCI_DEV_SETUP_GOTO_DATA*)slt.Items[i];
	            tmpsd=tmpgt->PciSdlInfo;
	            if(tmpsd->Slot > sd->Slot){
	                Status=InsertItemLst(&slt, gt, i);
	                ap=FALSE;
	                break;
	            }
	        }//for
	        // if we apPEND Flag is set Slot # == the biggest one yet.
	        if(ap) Status = AppendItemLst(&slt, gt);
		}
		ASSERT_EFI_ERROR(Status);
		if(EFI_ERROR(Status)) return;
	} //keep going until unsorted list empty.
	
	//now fill same list we just emptied IfrData->PciDevSetupData[] with sorted data.
	//With Sorted Slot Data....
#if	(PCI_SETUP_SHOW_SLOTS_FIRST == 1)	
	for(i=0;i<slt.ItemCount; i++) {
		Status=AppendItemLst((T_ITEM_LIST*)&IfrData->PciDevInitCnt, slt.Items[i]);
		ASSERT_EFI_ERROR(Status);
	}
#endif
	//with On Board Devices Data...
	for(i=0;i<obd.ItemCount; i++) {
		Status=AppendItemLst((T_ITEM_LIST*)&IfrData->PciDevInitCnt, obd.Items[i]);
		ASSERT_EFI_ERROR(Status);
	}
	
#if	(PCI_SETUP_SHOW_SLOTS_FIRST == 0)	
	for(i=0;i<slt.ItemCount; i++) {
		Status=AppendItemLst((T_ITEM_LIST*)&IfrData->PciDevInitCnt, slt.Items[i]);
		ASSERT_EFI_ERROR(Status);
	}
#endif	
	ClearItemLst(&obd,FALSE);
	ClearItemLst(&slt,FALSE);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PciSetupEntryPoint
//
// Description:
//  This is the standard EFI driver entry point called for CSM Policy module
//  initlaization
//
// Input:       IN EFI_HANDLE ImageHandle - ImageHandle of the loaded driver
//              IN EFI_SYSTEM_TABLE SystemTable - Pointer to the System Table
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI PciSetupEntryPoint (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_STATUS Status;
	UINTN			i;
//---------------------	
	
#if (PCI_SETUP_USE_APTIO_4_STYLE == 1)
	//Don't do anything if customer opted APTIO 4.x setup style...
	return EFI_SUCCESS;
#endif	
	
	InitAmiLib(ImageHandle, SystemTable);
    AmiSdlInitBoardInfo();
    pBS->SetMem(&gPciIfrInfo, sizeof(PCI_IFR_INFO), 0);
    
//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG
//EFI_DEADLOOP();
//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG//DEBUG
    
    //check if PCI Devices has setup Pages... If no pages - No Setup Resources will be loaded.
    for(i=0; i<gSdlPciData->PciDevCount; i++){
    	AMI_SDL_PCI_DEV_INFO	*sdlpcidev;
    //---------------------------------------------------------------	
    	sdlpcidev=&gSdlPciData->PciDevices[i];
    	if(sdlpcidev->PciDevFlags.Bits.HasSetup){
        	PCI_DEV_SETUP_GOTO_DATA	*gotoitem;
        //--------------------------------
            gotoitem=MallocZ(sizeof(PCI_DEV_SETUP_GOTO_DATA));
        	if(gotoitem==NULL) return EFI_OUT_OF_RESOURCES;
        	gotoitem->PciSdlInfo=sdlpcidev;   
        	gotoitem->PciDevIdx=i;
         		
         	Status=AppendItemLst((T_ITEM_LIST*)&gPciIfrInfo.PciDevInitCnt, gotoitem);
       		ASSERT_EFI_ERROR(Status);
      		if(EFI_ERROR(Status)) return Status;
     	}
	}
    //SortData(&gPciIfrInfo);
    //Tell how much devices with setup we have found...
	PCI_TRACE((TRACE_PCI,"PciSetup: Found %d PCI Devices With Setup Pages... %r.\n",gPciIfrInfo.PciDevCount, Status));

	//If no PCI devices with Setup .... just exit..
    if(gPciIfrInfo.PciDevCount==0) return EFI_SUCCESS;
    
    //Load setup page and update strings
    LoadResources(ImageHandle, sizeof(SetupCallBack) / sizeof(CALLBACK_INFO), SetupCallBack, NULL);
    gPciHiiHandle = SetupCallBack[0].HiiHandle;

    InitDriverVersionString();
    
    // Register callback on TSE event to update IFR data
    {
        VOID *SetupRegistration;
        static EFI_GUID SetupEnterGuid = AMITSE_SETUP_ENTER_GUID;
        static EFI_EVENT SetupEnterEvent;

        RegisterProtocolCallback(
            &SetupEnterGuid, InitPciIfr,
            NULL, &SetupEnterEvent, &SetupRegistration
        );
    }
    return Status;
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**           5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093      **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
