#include <SbDxeInitElink.h>
#include <Library/CrbLib.h>

UINT32 mAzaliaVerbTableDataVlv[] = {
//EIP136267 >>
  //
  // Audio Verb Table - 0x80862804
  //
  // Pin Widget 5 - PORT B
  0x20471C10,
  0x20471D00,
  0x20471E56,
  0x20471F18,

  // Pin Widget 6 - PORT C
  0x20571C20,
  0x20571D00,
  0x20571E56,
  0x20571F18,

  // Pin Widget 7 - PORT D
  0x20671C30,
  0x20671D00,
  0x20671E56,
  0x20671F58
//EIP136267 <<
};

//P20130627_3 >>
UINT32 mAzaliaVerbTableDataALC262[] = {
  //
  //ALC262 Verb Table - 10EC0262
  //
  //Pin Complex (NID 0x11 )
  0x01171CF0,
  0x01171D11,
  0x01171E11,
  0x01171F41,
  //Pin Complex (NID 0x12 )
  0x01271CF0,
  0x01271D11,
  0x01271E11,
  0x01271F41,
  //Pin Complex (NID 0x14 )
  0x01471C10,
  0x01471D40,
  0x01471E01,
  0x01471F01,
  //Pin Complex (NID 0x15 )
  0x01571CF0,
  0x01571D11,
  0x01571E11,
  0x01571F41,
  //Pin Complex (NID 0x16 )
  0x01671CF0,
  0x01671D11,
  0x01671E11,
  0x01671F41,
  //Pin Complex (NID 0x18 )
  0x01871C20,
  0x01871D98,
  0x01871EA1,
  0x01871F01,
  //Pin Complex (NID 0x19 )
  0x01971C21,
  0x01971D98,
  0x01971EA1,
  0x01971F02,
  //Pin Complex (NID 0x1A )
  0x01A71C2F,
  0x01A71D30,
  0x01A71E81,
  0x01A71F01,
  //Pin Complex (NID 0x1B )
  0x01B71C1F,
  0x01B71D40,
  0x01B71E21,
  0x01B71F02,
  //Pin Complex (NID 0x1C )
  0x01C71CF0,
  0x01C71D11,
  0x01C71E11,
  0x01C71F41,
  //Pin Complex (NID 0x1D )
  0x01D71C01,
  0x01D71DC6,
  0x01D71E14,
  0x01D71F40,
  //Pin Complex (NID 0x1E )
  0x01E71CF0,
  0x01E71D11,
  0x01E71E11,
  0x01E71F41,
  //Pin Complex (NID 0x1F )
  0x01F71CF0,
  0x01F71D11,
  0x01F71E11,
  0x01F71F41
};
//P20130627_3 <<

UINT32 UpxAdl01_A01_OemHdaVerbTblSample[] = {
//===================================================================================================
//
//                               Realtek Semiconductor Corp.
//
//===================================================================================================

//Realtek High Definition Audio Configuration - Version : 5.0.3.3
//Realtek HD Audio Codec : ALC888S-VD
//PCI PnP ID : PCI\VEN_8086&DEV_2668&SUBSYS_00000000
//HDA Codec PnP ID : HDAUDIO\FUNC_01&VEN_10EC&DEV_0888&SUBSYS_00000000
//The number of verb command block : 17

//    NID 0x11 : 0x40000000
//    NID 0x12 : 0x411111F0
//    NID 0x14 : 0x02211010
//    NID 0x15 : 0x411111F0
//    NID 0x16 : 0x411111F0
//    NID 0x17 : 0x411111F0
//    NID 0x18 : 0x02A11020
//    NID 0x19 : 0x411111F0
//    NID 0x1A : 0x411111F0
//    NID 0x1B : 0x411111F0
//    NID 0x1C : 0x411111F0
//    NID 0x1D : 0x4025122D
//    NID 0x1E : 0x411111F0
//    NID 0x1F : 0x411111F0


//===== HDA Codec Subsystem ID Verb-table =====
//HDA Codec Subsystem ID  : 0x00000000
0x00172000,
0x00172100,
0x00172200,
0x00172300,


//===== Pin Widget Verb-table =====
//Widget node 0x01 :
0x0017FF00,
0x0017FF00,
0x0017FF00,
0x0017FF00,
//Pin widget 0x11 - S/PDIF-OUT2
0x01171C00,
0x01171D00,
0x01171E00,
0x01171F40,
//Pin widget 0x12 - DMIC
0x01271CF0,
0x01271D11,
0x01271E11,
0x01271F41,
//Pin widget 0x14 - FRONT (Port-D)
0x01471C10,
0x01471D11,
0x01471E21,
0x01471F02,
//Pin widget 0x15 - SURR (Port-A)
0x01571CF0,
0x01571D11,
0x01571E11,
0x01571F41,
//Pin widget 0x16 - CEN/LFE (Port-G)
0x01671CF0,
0x01671D11,
0x01671E11,
0x01671F41,
//Pin widget 0x17 - SIDESURR (Port-H)
0x01771CF0,
0x01771D11,
0x01771E11,
0x01771F41,
//Pin widget 0x18 - MIC1 (Port-B)
0x01871C20,
0x01871D11,
0x01871EA1,
0x01871F02,
//Pin widget 0x19 - MIC2 (Port-F)
0x01971CF0,
0x01971D11,
0x01971E11,
0x01971F41,
//Pin widget 0x1A - LINE1 (Port-C)
0x01A71CF0,
0x01A71D11,
0x01A71E11,
0x01A71F41,
//Pin widget 0x1B - LINE2 (Port-E)
0x01B71CF0,
0x01B71D11,
0x01B71E11,
0x01B71F41,
//Pin widget 0x1C - CD-IN
0x01C71CF0,
0x01C71D11,
0x01C71E11,
0x01C71F41,
//Pin widget 0x1D - BEEP-IN
0x01D71C2D,
0x01D71D12,
0x01D71E25,
0x01D71F40,
//Pin widget 0x1E - S/PDIF-OUT1
0x01E71CF0,
0x01E71D11,
0x01E71E11,
0x01E71F41,
//Pin widget 0x1F - S/PDIF-IN
0x01F71CF0,
0x01F71D11,
0x01F71E11,
0x01F71F41,
//Widget node 0x20 :
0x02050007,
0x020409C8,
0x02050007,
0x020409C8
};

SB_HDA_VERB_TABLE mAzaliaVerbTable[] = {	//EIP176554
  OEM_HDA_VERB_TABLE
  {
    // End of the Verb table
    {0, 0, 0, 0, 0, 0}, 0
  },
};
