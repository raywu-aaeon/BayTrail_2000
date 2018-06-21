//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//*************************************************************************
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*****************************************************************************
// Revision History
// ----------------
// $Log: $
// 
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        SbPpi.h
//
// Description: This header file contains the PPI definition for the SB.
//
//<AMI_FHDR_END>
//*************************************************************************

#ifndef FILE_PEI_SB_PPI_H_
#define FILE_PEI_SB_PPI_H_

// {95E8152B-1B98-4f11-8A77-DB26583EBC42}
#define AMI_PEI_SBINIT_POLICY_PPI_GUID  \
 {0x95e8152b, 0x1b98, 0x4f11, 0x8a, 0x77, 0xdb, 0x26, 0x58, 0x3e, 0xbc, 0x42}

// {38965BB5-8097-40f5-B742-8CC14A649B64}
#define AMI_PEI_SB_CUSTOM_PPI_GUID  \
 {0x38965bb5, 0x8097, 0x40f5, 0xb7, 0x42, 0x8c, 0xc1, 0x4a, 0x64, 0x9b, 0x64}

typedef struct _AMI_PEI_SBINIT_POLICY_PPI   AMI_PEI_SBINIT_POLICY_PPI;

struct _AMI_PEI_SBINIT_POLICY_PPI {
    UINTN                       unFlag;
};

typedef struct _AMI_SB_PCI_DEVICES_TABLE_STRUCT {
    UINT64                      PciAddr;
    UINT8                       PciSidReg;
} AMI_SB_PCI_DEVICES_TABLE_STRUCT;

typedef struct _AMI_SB_PCI_SSID_TABLE_STRUCT AMI_SB_PCI_SSID_TABLE_STRUCT;

struct _AMI_SB_PCI_SSID_TABLE_STRUCT {
    UINT64                      PciAddr;
    UINT32                      Sid;
};

//CSP20130930>>
//(P052813A+)>>
typedef union _AMI_GPIO_STRUCT
{
 UINT32 Dword;
 struct
 {
	 UINT32 Gpi:1;                //0
	 UINT32 Gpo:1;                //1
	 UINT32 Gpod4H :1;            //2
	 UINT32 Gpod4L :1;            //3
	 UINT32 Func:3;               //4,5,6
	 UINT32 IntCap:1;             //7
	 UINT32 IntType:3;            //8,9,10
	 UINT32 Pull:3;               //11,12,13
	 UINT32 Offset:3;             //14,15,16
	 UINT32 TPE:1;                //17
	 UINT32 TNE:1;                //18
	 UINT32 TS:1;                 //19
	 UINT32 WE:1;                 //20
	 UINT32 DirectIrqEn:1;        //21
	 UINT32 RESERVED:10;          //22~31
  } Fileds;
} AMI_GPIO_STRUCT;
//(P052813A+)<<
//CSP20130930<<

typedef struct _AMI_GPIO_INIT_TABLE_STRUCT  AMI_GPIO_INIT_TABLE_STRUCT;

struct _AMI_GPIO_INIT_TABLE_STRUCT {
    UINT16                      GpioNo;
    AMI_GPIO_STRUCT             GpioCfg;
};

typedef struct _AMI_GPIO_INIT_PPI           AMI_GPIO_INIT_PPI;

typedef struct _AMI_GPIO_INIT_PPI {
    UINT32                      GpioBaseAddr;
    AMI_GPIO_INIT_TABLE_STRUCT  *GpioTable;
    BOOLEAN                     InitDefaultGpioSetting;
} AMI_GPIO_INIT_PPI;

typedef struct _AMI_PEI_SB_CUSTOM_PPI       AMI_PEI_SB_CUSTOM_PPI;

struct _AMI_PEI_SB_CUSTOM_PPI {
    AMI_GPIO_INIT_PPI            *GpioInit;
    AMI_SB_PCI_SSID_TABLE_STRUCT  *SsidTable;
};


extern EFI_GUID gAmiPeiSbCustomPpiGuid;
extern EFI_GUID gAmiPeiSbInitPolicyGuid;

#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
