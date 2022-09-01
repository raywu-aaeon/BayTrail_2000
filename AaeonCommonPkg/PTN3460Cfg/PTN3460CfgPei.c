//---------------------------------------------------------------------------
// Include(s)
//---------------------------------------------------------------------------
#include <AmiPeiLib.h>
#include <AmiCspLib.h>
#include <Setup.h>
#include <token.h>

#include "PTN3460Cfg.h"
// Produced Protocols

// Consumed Protocols
#include <Ppi/Smbus2.h>
#include <Ppi/Stall.h>


//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)
EFI_PEI_SMBUS2_PPI	 *gSmBus2Ppi = NULL;
EFI_PEI_STALL_PPI    *gStallPpi = NULL;
EFI_PEI_SERVICES     **gPeiServices = NULL;

UINT8 DefaultCfgTable[128] = {
	0x00, 0x03, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x08, 0x00, 0x0F, 0x0C,
	0x07, 0xFF, 0x1E, 0x0A, 0x14, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

UINT8 gEdidTable[14][128]  = {
    {// 0_640x480_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   		0x01, 0x19, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26,
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xD7, 0x09, 0x80, 0xA0, 0x20, 0xE0, 0x2D, 0x10, 0x08, 0x60,
   		0x22, 0x00, 0x80, 0xE0, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x36, 0x34, 0x30,
   		0x78, 0x34, 0x38, 0x30, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB2
    },
    {// 1_800x480_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  		0x12, 0x17, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26,
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFE, 0x0C, 0x20, 0x00, 0x31, 0xE0, 0x2D, 0x10, 0x28, 0x80,
   		0x22, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x38, 0x30, 0x30,
   		0x78, 0x34, 0x38, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36
    },
    {// 2_800x600_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
   		0x12, 0x17, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x8B, 0x0F, 0x20, 0x00, 0x31, 0x58, 0x1C, 0x20, 0x28, 0x80,
   		0x14, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x38, 0x30, 0x30,
   		0x78, 0x36, 0x30, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42
    },
    {// 3_1024x600_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
   		0x01, 0x19, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xC8, 0x13, 0x00, 0x40, 0x41, 0x58, 0x1C, 0x20, 0x18, 0x88, 
   		0x14, 0x00, 0x00, 0x2C, 0x21, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x30, 0x32, 
   		0x34, 0x78, 0x36, 0x30, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8D
    },
    {// 4_1024x768_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x04, 0x00, 0x70, 0x03, 0x00, 0x00, 
  		0x15, 0x19, 0x01, 0x03, 0x80, 0x3C, 0x28, 0x00, 0xE2, 0x80, 0x42, 0xAC, 0x51, 0x30, 0xB4, 0x25, 
   		0x10, 0x50, 0x53, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x64, 0x19, 0x00, 0x40, 0x41, 0x00, 0x26, 0x30, 0x18, 0x88, 
   		0x36, 0x00, 0x06, 0x4D, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x30, 0x32, 
   		0x34, 0x78, 0x37, 0x36, 0x38, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A
    },
    {// 5_1280x768_60Hz
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x01, 0x19, 0x01, 0x03, 0x80, 0x3C, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x20, 0x1F, 0x00, 0x80, 0x51, 0x00, 0x1E, 0x30, 0x40, 0x80,
		0x37, 0x00, 0xF4, 0x2C, 0x11, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x32, 0x38,
		0x30, 0x78, 0x37, 0x36, 0x38, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9
    },
	{// 6_1280x800_60Hz
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x12, 0x17, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xBC, 0x1B, 0x00, 0xA0, 0x50, 0x20, 0x17, 0x30, 0x30, 0x50, 
		0x36, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x32, 0x38,
		0x30, 0x78, 0x38, 0x30, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
	},
	{// 7_1280x1024_60Hz
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x01, 0x19, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x2C, 0x2A, 0x00, 0x98, 0x51, 0x00, 0x2A, 0x40, 0x30, 0x70, 
		0x13, 0x00, 0x00, 0x2C, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x32, 0x38, 
		0x30, 0x78, 0x31, 0x30, 0x32, 0x34, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC
    },
	{// 8_1366x768_60Hz
    	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
    	0x03, 0x18, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26,
    	0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x3D, 0x1B, 0x56, 0x78, 0x50, 0x00, 0x0E, 0x30, 0x20, 0x20, 
    	0x24, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x33, 0x36, 
    	0x36, 0x78, 0x37, 0x36, 0x38, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
    	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
    	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46
	},
    {// 9_1440x900_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 
   		0x12, 0x17, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x9A, 0x29, 0xA0, 0xD0, 0x51, 0x84, 0x22, 0x30, 0x50, 0x98, 
   		0x36, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x34, 0x34, 
   		0x30, 0x78, 0x39, 0x30, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA
    },
    {// 10_1600x1200_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 
   		0x12, 0x17, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x48, 0x3F, 0x40, 0x30, 0x62, 0xB0, 0x32, 0x40, 0x40, 0xC0, 
   		0x13, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x36, 0x30, 
   		0x30, 0x78, 0x31, 0x32, 0x30, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCB
    },
    {// 11_1920x1080_60Hz
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x0B, 0x00, 0x70, 0x03, 0x00, 0x00, 
   		0x2C, 0x16, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x02, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2D, 
   		0x36, 0x00, 0x2C, 0xC8, 0x10, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x39, 0x32, 
   		0x30, 0x78, 0x31, 0x30, 0x38, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25
    },
    {// 12_1920x1200_60Hz 20170810_Mark Modify 1920*1200 EDID Table as Reduced Blanking for compatibility issue found in APL5 DP Port
     // 20170810_Drako Correct EDID version and modify Definition blocks to follow same rule as other resolutions.
	 //AaeonCommonPkg_Develop_Shawn Correct PTN3460 EDID table to match 1920x1200 resolution
      0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x12, 0x17, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
      0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x28, 0x3C, 0x80, 0xA0, 0x70, 0xB0, 0x23, 0x40, 0x30, 0x20, 
      0x36, 0x00, 0x07, 0xC8, 0x20, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x39, 0x32, 
      0x30, 0x78, 0x31, 0x32, 0x30, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDA
    },
    {// 13_1280x1024_60Hz-Dummy
   		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x04, 0x2E, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
   		0x01, 0x19, 0x01, 0x03, 0x80, 0x32, 0x28, 0x00, 0x0A, 0x07, 0xF5, 0x9A, 0x57, 0x4E, 0x87, 0x26, 
   		0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
   		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x2C, 0x2A, 0x00, 0x98, 0x51, 0x00, 0x2A, 0x40, 0x30, 0x70, 
   		0x13, 0x00, 0x00, 0x2C, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x31, 0x32, 0x38, 
   		0x30, 0x78, 0x31, 0x30, 0x32, 0x34, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
   		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC
    },
};

//UINT8 gPwmFdValTbl[10] = {0x41, 0x21, 0x1E, 0x13, 0x06, 0x03, 0x01, 0x0, 0x0, 0x0};
UINT8 gPwmFdValTbl[10] = {0x41, 0x21, 0x1E, 0x0D, 0x06, 0x03, 0x01, 0x0, 0x0, 0x0};

// Macro Definition(s)

// Type Definition(s)

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------
// Variable Declaration(s)
SETUP_DATA  gSetupData;

// GUID Definition(s)

// Protocol Definition(s)

// External Declaration(s)

// Function Definition(s)

// Function Prototype(s)
extern PTN3460_CFG_HOOK PTN3460_PANEL1_PRECFG_FUNC EndOfPtn3460Panel1PrecfgFunc;
PTN3460_CFG_HOOK *Ptn3460Panel1PreCfg[] = { PTN3460_PANEL1_PRECFG_FUNC NULL };
extern PTN3460_CFG_HOOK PTN3460_PANEL1_POSTCFG_FUNC EndOfPtn3460Panel1PostcfgFunc;
PTN3460_CFG_HOOK *Ptn3460Panel1PostCfg[] = { PTN3460_PANEL1_POSTCFG_FUNC NULL };
extern PTN3460_CFG_HOOK PTN3460_PANEL2_PRECFG_FUNC EndOfPtn3460Panel2PrecfgFunc;
PTN3460_CFG_HOOK *Ptn3460Panel2PreCfg[] = { PTN3460_PANEL2_PRECFG_FUNC NULL };
extern PTN3460_CFG_HOOK PTN3460_PANEL2_POSTCFG_FUNC EndOfPtn3460Panel2PostcfgFunc;
PTN3460_CFG_HOOK *Ptn3460Panel2PostCfg[] = { PTN3460_PANEL2_POSTCFG_FUNC NULL };

//---------------------------------------------------------------------------
BOOLEAN DirtyCheck(UINT8 Addr, UINT8 CfgTbl[128], UINT8 edidTable[128])
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT8 i = 0;
  UINT8 data = 0;
  EFI_SMBUS_DEVICE_ADDRESS	SlaveAddr;
  UINTN DataLength = 1;

  SlaveAddr.SmbusDeviceAddress = Addr;

  // check EDID
  for ( i = 0 ; i < 128 ; i++ ) {
      Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i, EfiSmbusReadByte, 0x00, &DataLength, &data);
      if ((Status != EFI_SUCCESS) || (data != edidTable[i]))
        return TRUE;
  }
  
  // check cfg table, only read first 32 bytes
  for ( i = 128 ; i < 160 ; i++ ) {
      Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i, EfiSmbusReadByte, 0x00, &DataLength, &data);
      if ((Status != EFI_SUCCESS) || (data != CfgTbl[i]))
        return TRUE;
  }
  return FALSE;
}

// Return value:
// TRUE: Table from chip, allow partial update
// FALSE: Using default table, need full update
BOOLEAN GetChipCfgTable(UINT8 Addr, UINT8* CfgTbl)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT8 i = 0;
  EFI_SMBUS_DEVICE_ADDRESS	SlaveAddr;
  UINTN DataLength = 1;
  UINT8 VerifyTag[4];

  SlaveAddr.SmbusDeviceAddress = Addr;

  // Miles: Add ready check. PTN3460 needs 100ms to get ready after power up
  for ( i = 0 ; i < DEVICE_CHECK_RETRY; i++ )
  {
      Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0xEC, EfiSmbusReadByte, 0x00, &DataLength, &VerifyTag[0]);
      if (Status != EFI_SUCCESS)
      {
    	  VerifyTag[0] = 0;
    	  if (gStallPpi != NULL)
    		  gStallPpi->Stall(gPeiServices, gStallPpi, 20000);
      } else
      {
    	  break;
      }
  }
  
  // verify flashed tag
  // Miles: Get remain tag if first tag is got(may not valid, check later)
  if (VerifyTag[0] != 0x0)
  {
      for ( i = 1 ; i < 4 ; i++ ) {
          Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i+0xEC, EfiSmbusReadByte, 0x00, &DataLength, &VerifyTag[i]);
          if (Status != EFI_SUCCESS)
    	      VerifyTag[i] = 0;
      }
  }
  
  if ((VerifyTag[0] == 0x12) && (VerifyTag[1] == 0x34) && (VerifyTag[2] == 0x56) && (VerifyTag[3] == 0x78))
  {
	  //Get table from chip
	  for ( i = 0 ; i < 128 ; i++ ) {
	      Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i+0x80, EfiSmbusReadByte, 0x00, &DataLength, &CfgTbl[i]);
	      if (Status != EFI_SUCCESS)
	      {
	    	  //Get default table
	    	  MemCpy(CfgTbl, (UINT8 *)DefaultCfgTable, sizeof(DefaultCfgTable));
	    	  return FALSE;
	      }
	  }
  } else
  {
	  //Get default table
	  MemCpy(CfgTbl, (UINT8 *)DefaultCfgTable, sizeof(DefaultCfgTable));
	  return FALSE;
  }
  return TRUE;
}

void PreCfgPanel1(AAEON_PTN3460_CFG *CfgPanel)
{
    UINTN i = 0;
    
    for(i = 0; Ptn3460Panel1PreCfg[i] != NULL; i++)
    {
        Ptn3460Panel1PreCfg[i](CfgPanel);
    }
    
	return;
}

void PostCfgPanel1(AAEON_PTN3460_CFG *CfgPanel)
{
    UINTN i = 0;
    
    for(i = 0; Ptn3460Panel1PostCfg[i] != NULL; i++)
    {
        Ptn3460Panel1PostCfg[i](CfgPanel);
    }
    
	return;
}

#if (PTN3460_SUPPORT_NUMBER > 1)
void PreCfgPanel2(AAEON_PTN3460_CFG *CfgPanel)
{
    UINTN i = 0;
    
    for(i = 0; Ptn3460Panel2PreCfg[i] != NULL; i++)
    {
        Ptn3460Panel2PreCfg[i](CfgPanel);
    }
    
	return;
}

void PostCfgPanel2(AAEON_PTN3460_CFG *CfgPanel)
{
    UINTN i = 0;
    
    for(i = 0; Ptn3460Panel2PostCfg[i] != NULL; i++)
    {
        Ptn3460Panel2PostCfg[i](CfgPanel);
    }
    
	return;
}
#endif

void PTN3460_Init(AAEON_PTN3460_CFG *CfgPanel)
{
    EFI_STATUS Status = EFI_SUCCESS;
    int i = 0;
    UINT8 data = 0;
    UINT16 value = 0;
    UINT8 *EdidTblPtr = NULL;
    UINT8 EdidIdx = 0;
    BOOLEAN Dirty = FALSE;
    BOOLEAN UpdateOnly = FALSE;
	EFI_SMBUS_DEVICE_ADDRESS	SlaveAddr;
	UINTN DataLength = 1;
	UINT8 BLK_Mode;
	UINT8 MagicCode[]={0x01, 0x78, 0x45, 0x56};
    
    UINT8 configTable[128] = {0x00}, OrigTable[256] = {0x00};

    //if LVDS not enabled, then return
    if (CfgPanel->Enabled == 0)
        return;
    
	if (CfgPanel->PanelType < 7)
	{
		EdidIdx = CfgPanel->PanelType;
	} else
	{
		EdidIdx = CfgPanel->PanelType - 7;
	}
	EdidTblPtr = &gEdidTable[CfgPanel->PanelType][0];

	SlaveAddr.SmbusDeviceAddress = CfgPanel->SlaveAddr;

    // verify PTN3460 Cfgtable
    UpdateOnly = GetChipCfgTable(CfgPanel->SlaveAddr, &configTable[0]);
#if FORCE_TO_WRITE_FULLTABLE
    UpdateOnly = FALSE;
#endif
    
	// Backlight Mode , 0x8d
	Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0x8D, EfiSmbusReadByte, 0x00, &DataLength, &BLK_Mode);
	BLK_Mode = ( CfgPanel->BacklightMode == 1 ) ? ( BLK_Mode|BIT04):( BLK_Mode&~(BIT04));
	Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0x8D, EfiSmbusWriteByte, 0x00, &DataLength, &BLK_Mode);

    if (UpdateOnly == TRUE)
    {
    	// Allow partial update
    	// Get Full config table
		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0x85, EfiSmbusWriteByte, 0x00, &DataLength, &EdidIdx); //set EDID window
        for ( i = 0 ; i < 128 ; i++ )
        {
    		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i, EfiSmbusReadByte, 0x00, &DataLength, &OrigTable[i]);
            if (Status != EFI_SUCCESS)
            	break;
        }
    	MemCpy(&OrigTable[128], &configTable[0], sizeof(configTable)); 
    }

   	configTable[0x01] &= 0xC7;  //clear bit3/4/5
   	configTable[0x01] |= 0x03;  //11: valid clock output on both buses

   	switch (CfgPanel->PanelMode)
   	{
   		case 1: //Dual LVDS bus mode, bit3=1
   			configTable[0x01] |= BIT3;
   			break;
   		default: case 0:  //Single LVDS bus mode, bit3=0
   			configTable[0x01] &= ~BIT3;
   			break;
   	}   

   	switch (CfgPanel->ColorDepth)
   	{
   		case 0: //18-bit and others, bit3=0,bit4=0,bit5=1
   			configTable[0x01] |= 0x20;
   			break;
   		case 2:	//36-bit, bit3=1,bit4=0,bit5=1
   			configTable[0x01] |= 0x28;
   			break;
   		case 3: //48-bit, bit3=1,bit4=0,bit5=0
   			configTable[0x01] |= 0x08;
   			break;
   		default: case 1:  //24-bit, bit3=0,bit4=0,bit5=0
   			// do nothing
   			break;
   	}

   	// Cofigurate EDID 0x85
   	configTable[0x05] = EdidIdx & 0xFF;
  	// Emulate EDID 0x84
 	configTable[0x04] =  (EdidIdx << 1) | 0x01;
   	// Backlight, 0x90-0x91
   	value = ( CfgPanel->BacklightType == 1 ) ? 
   			( CfgPanel->BacklightLv == 10 ? 4095 : 400 * CfgPanel->BacklightLv): 
   			( CfgPanel->BacklightLv == 0 ? 4095 : 400 * (10 - CfgPanel->BacklightLv));
   	data = (value & 0xFF00) >> 8;
   	configTable[0x10] = data;
   	data = (value & 0xFF);
   	configTable[0x11] = data;

    // Configure backlight mode 0x8D bit4
   	configTable[0x0D] &= ~(BIT4);
   	configTable[0x0D] |= (CfgPanel->BacklightMode == 1) ? BIT4 : 0;

   	configTable[0x0E] = 0x0F;  //CFGx pin do not override cfgtable

   	// PWM frequency
    configTable[0x0F] = DefaultCfgTable[0x0F];
	if (CfgPanel->BacklightPwmFreq < PwmFreq_Invalid)
	{
   	    configTable[0x12] = gPwmFdValTbl[CfgPanel->BacklightPwmFreq];
	} else
	{
   	    configTable[0x12] = DefaultCfgTable[0x12];
	}
   	
    if (UpdateOnly == FALSE)
    {
    	// Dirty check: Check if CfgTbl modified
    	
		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0x85, EfiSmbusWriteByte, 0x00, &DataLength, &configTable[0x05]); //set EDID window
    	Dirty = DirtyCheck(CfgPanel->SlaveAddr, &configTable[0], EdidTblPtr);
    	if ( !Dirty )
    	{
    		//flash for SRAM data is the same as we wish, do nothing.
    		return;
    	}
    	// Init 0x80 - 0xFF
    	for ( i = 0 ; i < 128 ; i++ ) {
    		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0x80 + i, EfiSmbusWriteByte, 0x00, &DataLength, &configTable[i]);
    	}

    	// update EDID
    	for ( i = 0 ; i < 128 ; i++ ) {
    		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i, EfiSmbusWriteByte, 0x00, &DataLength, &EdidTblPtr[i]);
    	}
    } else
    {
    	// EDID part
        for ( i = 0 ; i < 128 ; i++ )
        {
        	if (OrigTable[i] != EdidTblPtr[i])
        	{
        		Dirty = TRUE;
        		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, i, EfiSmbusWriteByte, 0x00, &DataLength, &EdidTblPtr[i]);
        	}
        }
    	
    	// Cfg table part
        for ( i = 0 ; i < 128 ; i++ )
        {
        	if (OrigTable[0x80+i] != configTable[i])
        	{
        		Dirty = TRUE;
        		Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0x80+i, EfiSmbusWriteByte, 0x00, &DataLength, &configTable[i]);
        	}
        }
    	if ( !Dirty )
    	{
    		//flash for SRAM data is the same as we wish, do nothing.
    		return;
    	}
    }

#if UPDATE_FLASH_ONCHANGE
	Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0xE8, EfiSmbusWriteByte, 0x00, &DataLength, &MagicCode[0]);
	Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0xE9, EfiSmbusWriteByte, 0x00, &DataLength, &MagicCode[1]);
	Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0xEA, EfiSmbusWriteByte, 0x00, &DataLength, &MagicCode[2]);
	Status = gSmBus2Ppi->Execute(gSmBus2Ppi, SlaveAddr, 0xEB, EfiSmbusWriteByte, 0x00, &DataLength, &MagicCode[3]);

    // Add 300ms delay 20170810_Mark Modify to 300ms to follow PTN3460 programming guide V1.8
    if (gStallPpi != NULL)
	    gStallPpi->Stall(gPeiServices, gStallPpi, 300000); 

#endif
    return;
}

//----------------------------------------------------------------------------
//
// Procedure:   PTN3460CfgDxeInit
//
// Description: This function is the entry point for CRB DXE driver.
//              This function initializes the CRB in DXE phase.
//
// Input:       ImageHandle - Image handle
//              SystemTable - Pointer to the system table
//
// Output:      EFI_SUCCESS
//
// Notes:       This routine is called very early, prior to SBDXE and NBDXE.
//----------------------------------------------------------------------------
EFI_STATUS
EFIAPI 
PTN3460CfgPeiInit(
	IN EFI_FFS_FILE_HEADER      *FfsHeader,
	IN EFI_PEI_SERVICES         **PeiServices)
{
    EFI_STATUS   Status = EFI_SUCCESS;
    EFI_GUID     SetupGuid = SETUP_GUID;
    UINTN        VariableSize = sizeof(SETUP_DATA);
    AAEON_PTN3460_CFG  PanelCfg;
	EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadOnlyVariable = NULL;
	
	PEI_TRACE((-1, PeiServices, "PTN3460CfgPeiInit Start\n"));
    
    Status = (*PeiServices)->LocatePpi(PeiServices, &gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, &ReadOnlyVariable);
    if (!EFI_ERROR(Status)) {
    	Status = ReadOnlyVariable->GetVariable(ReadOnlyVariable, L"Setup", &SetupGuid, NULL, &VariableSize, &gSetupData);
    }  
    
	Status = (*PeiServices)->LocatePpi( PeiServices, &gEfiPeiSmbus2PpiGuid, 0, NULL, &gSmBus2Ppi );
    if ( EFI_ERROR( Status ) ) {
		PEI_TRACE((-1, PeiServices, "Failed to locate SmBus ppi\n"));
		return Status;
    }

    Status = (*PeiServices)->LocatePpi(PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, &gStallPpi);
    if ( EFI_ERROR( Status ) ) {
		PEI_TRACE((-1, PeiServices, "Failed to locate stall ppi\n"));
		return Status;
    }
    
	gPeiServices = PeiServices;

    PanelCfg.SlaveAddr = PTN3460_SLAVE_ADDRESS1;
    PanelCfg.Enabled = gSetupData.PTN3460En1;
    PanelCfg.PanelType = gSetupData.PTN3460PanelType1;
    PanelCfg.PanelMode = gSetupData.PTN3460PanelMode1;
    PanelCfg.ColorDepth = gSetupData.PTN3460PanelColor1;
    PanelCfg.BacklightMode = gSetupData.PTN3460PanelBacklightMode;
    PanelCfg.BacklightType = gSetupData.PTN3460PanelBacklightType1;
    PanelCfg.BacklightLv = gSetupData.PTN3460PanelBacklight1;
    PanelCfg.BacklightPwmFreq = gSetupData.PTN3460PanelBacklightPwmFreq1;   
    PreCfgPanel1(&PanelCfg);
    PTN3460_Init(&PanelCfg);
    PostCfgPanel1(&PanelCfg);

#if (PTN3460_SUPPORT_NUMBER > 1)
    PanelCfg.SlaveAddr = PTN3460_SLAVE_ADDRESS2;
    PanelCfg.Enabled = gSetupData.PTN3460En2;
    PanelCfg.PanelType = gSetupData.PTN3460PanelType2;
    PanelCfg.PanelMode = gSetupData.PTN3460PanelMode2;
    PanelCfg.ColorDepth = gSetupData.PTN3460PanelColor2;
    PanelCfg.BacklightType = gSetupData.PTN3460PanelBacklightType2;
    PanelCfg.BacklightLv = gSetupData.PTN3460PanelBacklight2;
    PanelCfg.BacklightPwmFreq = gSetupData.PTN3460PanelBacklightPwmFreq2;   
    PreCfgPanel2(&PanelCfg);
    PTN3460_Init(&PanelCfg);
    PostCfgPanel2(&PanelCfg);
#endif
    
	PEI_TRACE((-1, PeiServices, "PTN3460CfgPeiInit End\n"));

    return Status;
}