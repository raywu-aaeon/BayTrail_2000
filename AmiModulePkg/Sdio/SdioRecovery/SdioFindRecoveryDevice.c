//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
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
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioFindRecoveryDevice.c 1     7/18/12 4:49a Rajeshms $
//
// $Revision: 1 $
//
// $Date: 7/18/12 4:49a $
//
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SdioDriver/SdioFindRecoveryDevice.c $
// 
// 1     7/18/12 4:49a Rajeshms
// [TAG]  		EIP93345
// [Category]  	New Feature
// [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
// devices
// [Files]  		SdioRecovery.cif
// SdioRecovery.sdl
// SdioRecovery.mak
// SdioRecovery.c
// SdioFindRecoveryDevice.c
// SdioRecovery.h
// 
// 1     7/18/12 4:30a Rajeshms
// [TAG]  		EIP93345 
// [Category]  	New Feature
// [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
// devices
// [Files]  		Board\EM\SdioRecovery\SdioRecovery.cif
// Board\EM\SdioRecovery\SdioRecovery.sdl
// Board\EM\SdioRecovery\SdioRecovery.mak
// Board\EM\SdioRecovery\SdioRecovery.c
// Board\EM\SdioRecovery\SdioFindRecoveryDevice.c
// Board\EM\SdioRecovery\SdioRecovery.h
// 
//*************************************************************************
//<AMI_FHDR_START>
//
//  Name:           SdioFindRecoveryDevice.c
//
//  Description:    This file enumerates PCI bus to find 
//                  the presence of SD/MMC device using the RootBridgeList
//                  given. If found, it detects the presence of card, if
//                  found it intializes the card and fills neccessary info.
//
//<AMI_FHDR_END>
//*************************************************************************

//---------------------------------------------------------------------------
#include "SdioRecovery.h"
#include "SdioRecoveryElink.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
PCI_DEVICE_INFO *gPciDevInfo[MAX_SDIO_RECOVERY_DEVICE]={NULL};
PCI_PROGRAMMED_BRIDGE_INFO *gPciProgrammedBridgeInfo[MAXIMUM_PCI_BRIDGE]={NULL};
PCI_BRIDGE_INFO *gBridgeInfo[MAXIMUM_PCI_BRIDGE]={NULL};

UINT8   gFreeBusNumber = FIRST_SECONDARY_BUS_NUMBER;
UINT32  *gRecoveryTransferAddress = NULL;
UINT8   gBridgeBusCount=0;

extern EFI_PEI_SERVICES     **gPeiServices;
extern EFI_PEI_PCI_CFG_PPI  *gPciCfg;

EFI_PEI_NOTIFY_DESCRIPTOR gNotifyList = {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiEndOfPeiSignalPpiGuid,
    NotifyOnRecoveryCapsuleLoaded
};
//---------------------------------------------------------------------------

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: EnumerateSdDevices
//
// Description:
//  This function calls the enumerates function to find SD/MMC device and 
//  if found intializes the card and fills neccessary info.
//
// Input:
//  IN OUT SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev
//
// Output:  
//  Status
//  *Sd_BlkIoDev -> fills neccessary info of the SD/MMC devices found.
//
// Modified:
//  gFreeBusNumber
//
// Referrals:
//  gFreeBusNumber, gPciDevInfo, gPeiServices, gPciCfg
//
// Notes:
//
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS
EnumerateSdDevices(
    IN OUT SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev
)
{

    EFI_STATUS  Status;
    UINT8       Index;
    UINT8       SubBusNoForRootBridge=MAX_SUBORDINATE_NUMBER;
    UINT8       SecondaryBusNo;
    UINT8       Port=0;
    UINT8       DeviceCount = 0;
    PCI_BRIDGE_INFO BridgeInfo;
    ROOT_BRIDGE RootBridgeList[] = {
                                    SD_ROOT_BRIDGE_LIST
                                    {0xff,0xff,0xff}
                                    };

    for( Index=0; RootBridgeList[Index].Func != 0xff ; Index++) {

        if( (RootBridgeList[Index].Dev == 0) && 
                (RootBridgeList[Index].Func == 0) ) {

            //
            // Enumerate Bus Zero.
            //
            Status = EnumerateBus(0);
            if ( EFI_ERROR( Status )) {
                return Status;
            }

        } else {
            //
            // Program the Secondary & subordinate Bus no. for root Bridge.
            //
            gPciCfg->Write (gPeiServices, \
                            gPciCfg, \
                            EfiPeiPciCfgWidthUint8, \
                            PEI_PCI_CFG_ADDRESS (RootBridgeList[Index].Bus,
                            RootBridgeList[Index].Dev, 
                            RootBridgeList[Index].Func, PCI_SBUS),\
                            &gFreeBusNumber);

            gPciCfg->Write (gPeiServices, \
                            gPciCfg, \
                            EfiPeiPciCfgWidthUint8, \
                            PEI_PCI_CFG_ADDRESS (RootBridgeList[Index].Bus,
                            RootBridgeList[Index].Dev,
                            RootBridgeList[Index].Func, PCI_SUBUS),\
                            &SubBusNoForRootBridge);


            BridgeInfo.PrimaryBusNumber = RootBridgeList[Index].Bus;
            BridgeInfo.Device = RootBridgeList[Index].Dev;
            BridgeInfo.Function = RootBridgeList[Index].Func;
            BridgeInfo.SecBusNumber = gFreeBusNumber;
            BridgeInfo.SubBusNumber = SubBusNoForRootBridge;

            //
            // Store the Bridge info for programing the Bridge's
            // above this Bridge.
            //
            Status = (**gPeiServices).AllocatePool( gPeiServices,
                                                 sizeof(PCI_BRIDGE_INFO),
                                                 &gBridgeInfo[gBridgeBusCount]);
            if ( EFI_ERROR( Status )) {
                return EFI_OUT_OF_RESOURCES;
            }
                    
            *gBridgeInfo[gBridgeBusCount] = BridgeInfo;

            SecondaryBusNo = gFreeBusNumber;
            gFreeBusNumber++;
            gBridgeBusCount++;
        
            //
            // Program Subordinate for the bridge above this Bridge.
            //
            ProgramSubordinateForBridgeAbove(RootBridgeList[Index].Bus,SecondaryBusNo);
            
            //
            // Enumerate the device's under this RootBrdige.
            //
            Status = EnumerateBus(SecondaryBusNo);
            if ( EFI_ERROR( Status )) {
                return Status;
            }

            //
            // Program Root Bridge based on the bridge or device programmed
            // under this Bridge.
            //
            Status = CheckforProgrammedBridgeorDevice(
                                            RootBridgeList[Index].Bus,
                                            RootBridgeList[Index].Dev,
                                            RootBridgeList[Index].Func,
                                            SecondaryBusNo);
            if ( EFI_ERROR( Status )) {
                return Status;
            }

        }
    }

  	Status = (**gPeiServices).NotifyPpi (gPeiServices, &gNotifyList);
    ASSERT_PEI_ERROR (gPeiServices, Status);

    //
    // Check for presence of card in the controller and intialize it.
    //
    for(Index=0; Index < MAX_SDIO_RECOVERY_DEVICE ;Index++) {
        if( gPciDevInfo[Index] ) {

            Status = (**gPeiServices).AllocatePool( gPeiServices, sizeof(SDIO_RECOVERY_DEVICE_INFO), &(Sd_BlkIoDev->DeviceInfo[DeviceCount]));

            if ( EFI_ERROR( Status )) {
                return EFI_OUT_OF_RESOURCES;
            }

            Sd_BlkIoDev->DeviceInfo[DeviceCount]->SdioDeviceInfo.SdioBaseAddress = gPciDevInfo[Index]->BaseAddress;

            //
            // Check for presence of SD/MMC card, if found initialise the card.
            //                
            Status = CheckDevicePresence_Controller(&Sd_BlkIoDev->DeviceInfo[DeviceCount]->SdioDeviceInfo, Port);
            if ( EFI_ERROR( Status )) {
                Sd_BlkIoDev->DeviceInfo[DeviceCount]->MediaInfo.MediaPresent = FALSE;
                DeviceCount++;
                continue;
            }

            Status = ConfigureMassDevice_Controller(&Sd_BlkIoDev->DeviceInfo[DeviceCount]->SdioDeviceInfo, Port);
            if ( EFI_ERROR( Status )) {
                //
                // Some cards fail when card is intialised, it again needs to be intialised.
                //
                Status = ConfigureMassDevice_Controller(&Sd_BlkIoDev->DeviceInfo[DeviceCount]->SdioDeviceInfo, Port);
                if ( !EFI_ERROR( Status )) {
                    goto GetDeviceInformation;
                }
                DeviceCount++;
                continue;
            }

GetDeviceInformation:

            Status = GetDeviceInformation(&Sd_BlkIoDev->DeviceInfo[DeviceCount]->SdioDeviceInfo, Port);
            if ( EFI_ERROR( Status )) {
                DeviceCount++;
                continue;
            }

            Sd_BlkIoDev->DeviceInfo[DeviceCount]->MediaInfo.MediaPresent = TRUE;
            Sd_BlkIoDev->DeviceInfo[DeviceCount]->MediaInfo.DeviceType = MaxDeviceType;
            Sd_BlkIoDev->DeviceInfo[DeviceCount]->MediaInfo.LastBlock = Sd_BlkIoDev->DeviceInfo[DeviceCount]->SdioDeviceInfo.dMaxLBA;
            Sd_BlkIoDev->DeviceInfo[DeviceCount]->MediaInfo.BlockSize = SD_BLOCK_SIZE;

            DeviceCount++;
        }
    }

    Sd_BlkIoDev->DeviceCount = DeviceCount;

    if( gPciDevInfo[0] ) {
        Status = (**gPeiServices).AllocatePool( gPeiServices,
                                               SIZE_FOR_DMA_TRANFER,
                                               &gRecoveryTransferAddress );
        if ( EFI_ERROR( Status )) {
            return EFI_OUT_OF_RESOURCES;
        }
        
    }

    return Status;

}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: EnumerateBus
//
// Description:
//  Enumerates the Bus number given and finds the SD/MMC Device.
//  If Bridge is found it enumerates that bridge also and search for 
//  SD/MMC Device.
//
// Input:
//  IN UINT8 Bus - Bus number to be enumerated.
//
// Output:
//  EFI_STATUS
//      EFI_OUT_OF_RESOURCES - Resource is not enough for storing informations.
//      EFI_SUCCESS - Bus has been enumerated successfully.
//
// Modified:
//  gPciDevInfo, gBridgeInfo, gFreeBusNumber
//
// Referrals:
//  gPciDevInfo, gBridgeInfo, gPeiServices, gPciCfg, gFreeBusNumber,
//  gBridgeBusCount, DeviceCount
//
// Notes:
//
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS
EnumerateBus(
    IN UINT8 Bus
)
{
    UINT8   Device;
    UINT8   Function;
    UINT16  VendorId;
    UINT8   HeaderType;
    UINT8   RevisionId[4];
    BOOLEAN MultiFunc=FALSE;
    UINT32  BarAddress;
    UINT8   CmdReg;
    UINT8   Index;
    PCI_DEVICE_INFO DeviceInfo;
    PCI_BRIDGE_INFO BridgeInfo;
    UINT8   SecondaryBusNo;    
    EFI_STATUS Status;
    static UINT8  DeviceCount=0;
    static UINT32 IoDeviceAddress = PCI_SD_MMC_IO_MAP_BASE_ADDRESS;
    static UINT32 MMIODeviceAddress = PCI_SD_MMC_MEM_MAP_BASE_ADDRESS;
    UINT32 MaskAddress = BAR_ADDRESS_MASK;
    UINT32  DeviceDecodeRange;    

    for( Device = 0;  Device <= PCI_MAX_DEVICE; Device++ ) {

        for( Function = 0;  Function <= PCI_MAX_FUNC; Function++ ) {

            gPciCfg->Read(gPeiServices, \
                          gPciCfg, \
                          EfiPeiPciCfgWidthUint16,\
                          PEI_PCI_CFG_ADDRESS(Bus,Device,
                          Function,PCI_VID),\
                          &VendorId);

            //
            // Check for valid Device/Bridge
            //
            if( (Function == 0) && VendorId == INVALID_VENDOR_ID) {
                break;
            } else if ( VendorId == INVALID_VENDOR_ID ) {
                continue;
            }

            gPciCfg->Read(gPeiServices, \
                          gPciCfg, \
                          EfiPeiPciCfgWidthUint8,\
                          PEI_PCI_CFG_ADDRESS(Bus,Device,
                          Function,PCI_HDR),\
                          &HeaderType);
            //
            // Check whether it is a multifunction Device.
            //
            if( Function == 0) {
                if(HeaderType & HDR_TYPE_MULTIFUNC) {
                    MultiFunc = TRUE;
                }
            }

            HeaderType &= 3;

            switch(HeaderType) {
                case HDR_TYPE_DEVICE :
                    //
                    // It is a device.
                    //
                    gPciCfg->Read(gPeiServices, \
                                  gPciCfg, \
                                  EfiPeiPciCfgWidthUint32,\
                                  PEI_PCI_CFG_ADDRESS(Bus,Device,
                                  Function,PCI_REV_ID_OFFSET),\
                                  &RevisionId);

                    //
                    // Check for the presence of SD/MMC Device.
                    //
                    if ((RevisionId[3] == PCI_CL_SYSTEM_PERIPHERALS) &&
                            (RevisionId[2] == PCI_CL_SYSTEM_PERIPHERALS_SCL_SD)) {
                        
                        gPciCfg->Write (gPeiServices, \
                                        gPciCfg, \
                                        EfiPeiPciCfgWidthUint32, \
                                        PEI_PCI_CFG_ADDRESS (Bus,
                                        Device, Function, PCI_BAR0),\
                                        &MaskAddress);
                    
                        gPciCfg->Read(gPeiServices, \
                                      gPciCfg, \
                                      EfiPeiPciCfgWidthUint32,\
                                      PEI_PCI_CFG_ADDRESS(Bus,Device,
                                      Function,PCI_BAR0),\
                                      &DeviceDecodeRange);

                        DeviceInfo.BusNumber = Bus;
                        DeviceInfo.Device = Device;
                        DeviceInfo.Function = Function;
                        
                        //
                        // Check whether BAR0 is implemented as I/O or memory
                        // Sizing the BAR0 to get I/O or memory needed for 
                        // accessing the UART registers.
                        //
                        if( DeviceDecodeRange & 1 ) {
                            //
                            // It's IO mapped. Assign IO address & enable IO
                            // in command register. Make next IO address
                            // to be 16-byte aligned.
                            //
                            for(Index=0; Index < MAX_SDIO_RECOVERY_DEVICE ;Index++) {
                                if( gPciDevInfo[Index] ) {
                                    if ( gPciDevInfo[Index]->BusNumber == Bus ) {
                                        break;
                                    }
                                } else {
                                    //
                                    // Change IO address, if another device is 
                                    // not found in same bus. eg, address assigned
                                    // is 0xF000, then next device address would be
                                    // 0xE000;
                                    //
                                    if(Index == 0) break;
                                    IoDeviceAddress -= 0x1000;
                                    IoDeviceAddress &= 0xF000;  
                                    break;
                                }
                            }
                            BarAddress = IoDeviceAddress;
                            CmdReg = CMD_ENABLE_IO;
                            DeviceInfo.IsMmioDevice = FALSE;
                            DeviceDecodeRange &= MASK_IO_DECODE_RANGE;
                            DeviceDecodeRange = ~DeviceDecodeRange + 1;
                            IoDeviceAddress += DeviceDecodeRange; 
                            while((IoDeviceAddress & 0xf) != 0) {
                                IoDeviceAddress++;
                            }
                        } else {
                            //
                            // It's Memory mapped. Assign memory address
                            // & enable memory in cmd register. Make next
                            // memory address to be 16-byte aligned.
                            //
                            for(Index=0; Index < MAX_SDIO_RECOVERY_DEVICE ;Index++) {
                                //
                                // Check for any presence of device already in same bus.
                                // if not found change the baseaddress to next set.
                                // eg. if addr is 0xD0B00000, next device address would 
                                // be 0xD0C0000 if that device is not in same bus compared
                                // to previously found device.
                                //
                                if( gPciDevInfo[Index] ) {
                                    if ( gPciDevInfo[Index]->BusNumber == Bus ) {
                                        break;
                                    }
                                } else {
                                    if(Index == 0) break;
                                    MMIODeviceAddress += 0x100000;
                                    MMIODeviceAddress &= 0xFFFF0000;  
                                    break;
                                }
                            }

                            BarAddress = MMIODeviceAddress;
                            CmdReg = CMD_ENABLE_MEM ;
                            DeviceInfo.IsMmioDevice = TRUE;
                            DeviceDecodeRange &= MASK_MEM_DECODE_RANGE;
                            DeviceDecodeRange = ~DeviceDecodeRange + 1;
                            MMIODeviceAddress += DeviceDecodeRange;
                            while((MMIODeviceAddress & 0xf) != 0) {
                                MMIODeviceAddress++;
                            }
                        }

                        DeviceInfo.BaseAddress = BarAddress;
                        //
                        // Program BAR addr and cmd register for the device.
                        //
                        gPciCfg->Write (gPeiServices, \
                                       gPciCfg, \
                                       EfiPeiPciCfgWidthUint32, \
                                       PEI_PCI_CFG_ADDRESS (Bus,
                                       Device, Function, PCI_BAR0),\
                                       &BarAddress);

                        gPciCfg->Write (gPeiServices, \
                                       gPciCfg, \
                                       EfiPeiPciCfgWidthUint8, \
                                       PEI_PCI_CFG_ADDRESS (Bus,
                                       Device, Function, PCI_CMD),\
                                       &CmdReg);
                        //
                        // Store the PCI SD/MMC device's info for
                        // programming Bridge above this device.
                        //
                        Status = (**gPeiServices).AllocatePool( gPeiServices,
                                                  sizeof(PCI_DEVICE_INFO),
                                                  &gPciDevInfo[DeviceCount]);

                        if ( EFI_ERROR( Status )) {
                            return EFI_OUT_OF_RESOURCES;
                        }

                        *gPciDevInfo[DeviceCount] = DeviceInfo;
                        DeviceCount++;
                    }
                    break;

                case HDR_TYPE_P2P_BRG:
                    //
                    // It is a Bridge.
                    //

                    //
                    // Don't enumerate Bridge's if enumeration is for Bus 0.
                    //
                    //if(!Bus) {
                    //    break;
                    //}

                    if( gBridgeBusCount > MAXIMUM_PCI_BRIDGE ) {
                        return EFI_OUT_OF_RESOURCES;
                    }

                    //
                    // Program Prim.,Sec. & subordinate Bus no. for Bridge.
                    //
                    gPciCfg->Write (gPeiServices, \
                                   gPciCfg, \
                                   EfiPeiPciCfgWidthUint8, \
                                   PEI_PCI_CFG_ADDRESS (Bus,
                                   Device, Function, PCI_PBUS),\
                                   &Bus);
                    
                    gPciCfg->Write (gPeiServices, \
                                   gPciCfg, \
                                   EfiPeiPciCfgWidthUint8, \
                                   PEI_PCI_CFG_ADDRESS (Bus,
                                   Device, Function, PCI_SBUS),\
                                   &gFreeBusNumber);

                    gPciCfg->Write (gPeiServices, \
                                   gPciCfg, \
                                   EfiPeiPciCfgWidthUint8, \
                                   PEI_PCI_CFG_ADDRESS (Bus,
                                   Device, Function, PCI_SUBUS),\
                                   &gFreeBusNumber);

                    BridgeInfo.PrimaryBusNumber = Bus;
                    BridgeInfo.Device = Device;
                    BridgeInfo.Function = Function;
                    BridgeInfo.SecBusNumber = gFreeBusNumber;
                    BridgeInfo.SubBusNumber = gFreeBusNumber;

                    //
                    // Store the Bridge info for programing the Bridge's
                    // above this Bridge.
                    //
                    Status = (**gPeiServices).AllocatePool( gPeiServices,
                                                 sizeof(PCI_BRIDGE_INFO),
                                                 &gBridgeInfo[gBridgeBusCount]);
                    if ( EFI_ERROR( Status )) {
                        return EFI_OUT_OF_RESOURCES;
                    }
                    
                    *gBridgeInfo[gBridgeBusCount] = BridgeInfo;
                    SecondaryBusNo = gFreeBusNumber;

                    gBridgeBusCount++;
                    gFreeBusNumber++;

                    //
                    // Program Subordinate of the bridge which is above this Bridge.
                    //

                    ProgramSubordinateForBridgeAbove(Bus,SecondaryBusNo);
                    
                    //
                    // Enumerate the Device's/Bridge's under this Bridge.
                    //
                    Status = EnumerateBus(SecondaryBusNo);
                    if ( EFI_ERROR( Status )) {
                        return Status;
                    }

                    //
                    // Check for any Device/Bridge programmed under this 
                    // bridge and Program this brdige based on that.
                    // 
                    Status = CheckforProgrammedBridgeorDevice(Bus,
                                                              Device,
                                                              Function,
                                                              SecondaryBusNo);
                    if ( EFI_ERROR( Status )) {
                        return Status;
                    }

                    break;

                default:
                    break;

            } // Switch
        
            if((Function == 0) && !MultiFunc) {
                break;
            } else if(MultiFunc) {
                MultiFunc=FALSE;
            }
        } // Function loop

    } // Device Loop

    return EFI_SUCCESS;

}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: CheckforProgrammedBridgeorDevice
//
// Description:
//  This checks whether any bridge or device is programmed under this bridge.
//  If found it checks for what has been programmed like cmd reg, IO or MMIO
//  address and programs this bridge accordingly.
//
// Input:
//  IN UINT8 Bus        - Bus no. of the Bridge
//  IN UINT8 Device     - Device no. of the Bridge
//  IN UINT8 Function   - Function no. of the Bridge   
//  IN UINT8 SecondaryBusNo - Secondary Bus No. Programmed for the Bridge
//
// Output:
//  EFI_STATUS
//      EFI_OUT_OF_RESOURCES - Resource isn't enough for storing informations.
//      EFI_SUCCESS - Bridge has been Programmed successfully if device or
//                    bridge is programmed under this bridge.
//
// Modified:
//
// Referrals:
//  gPciDevInfo, gPciProgrammedBridgeInfo, gBridgeInfo, gPciCfg, gPeiServices
//
// Notes:
//
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS
CheckforProgrammedBridgeorDevice (
    IN UINT8 Bus,
    IN UINT8 Device,
    IN UINT8 Function,
    IN UINT8 SecondaryBusNo
)
{
    UINT8   Index;
    UINT8   Index1;
    UINT8   Subordinate=0;
    EFI_STATUS Status;
    UINT16  Address;
    
    //
    // Check the stored info and Program the Bridge, if any
    // Device/Bridge is programmed behind this.
    //
    for(Index=0; Index < MAX_SDIO_RECOVERY_DEVICE ;Index++) {
        if( gPciDevInfo[Index] ) {  
            if( gPciDevInfo[Index]->BusNumber == SecondaryBusNo ) {
                //
                // Check whether Devices found behind this bridge
                // If found program the bridge.
                //
                if(gPciDevInfo[Index]->IsMmioDevice ) {
                    Address = gPciDevInfo[Index]->BaseAddress >> 16;
                } else {
                    Address = gPciDevInfo[Index]->BaseAddress >> 8;
                } 
                Status = ProgramPciBridge(Bus,Device,Function,Address,gPciDevInfo[Index]->IsMmioDevice);
                if ( EFI_ERROR( Status )) {
                    return Status;
                }
                break;
            }
        }
    }

    //
    // Program the Bridge if any Bridge behind this programmed.
    //
    for(Index=0; Index<MAXIMUM_PCI_BRIDGE ;Index++) {
        if( gPciProgrammedBridgeInfo[Index] ) {
            if(SecondaryBusNo == gPciProgrammedBridgeInfo[Index]->PrimaryBusNumber) {
                Status = ProgramPciBridge(Bus,Device,Function,gPciProgrammedBridgeInfo[Index]->MemIOBaseLimit,
                                                        gPciProgrammedBridgeInfo[Index]->IsMMIO);
                if ( EFI_ERROR( Status )) {
                    return Status;
                }
            }
        }
    }   
    
    //
    // Program the Sub-ordinate no. for thr bridge based on the bridge's programmed behind this.
    //
        for(Index=0; Index<MAXIMUM_PCI_BRIDGE ;Index++) {
            if( gBridgeInfo[Index] ) {
                if(SecondaryBusNo == gBridgeInfo[Index]->PrimaryBusNumber) {
                    if(!Subordinate) {
                        Subordinate = gBridgeInfo[Index]->SubBusNumber;
                    } else {
                        if ( gBridgeInfo[Index]->SubBusNumber > Subordinate) {
                            Subordinate = gBridgeInfo[Index]->SubBusNumber;
                        }
                    }
                }
            }
        }

    if(Subordinate != 0) {
        for(Index1=0; Index1 < MAXIMUM_PCI_BRIDGE ;Index1++) {
            if( gBridgeInfo[Index1] ) {
                if(SecondaryBusNo == gBridgeInfo[Index1]->SecBusNumber) {
                    gBridgeInfo[Index1]->SubBusNumber = Subordinate;
                }
            }
        }

     } else {
        //
        // If no bridge is programmed behind this bridge, then assign Sec. no as Sub no.
        //
        for(Index1=0; Index1 < MAXIMUM_PCI_BRIDGE ;Index1++) {
            if( gBridgeInfo[Index1] ) {
                if((gBridgeInfo[Index1]->PrimaryBusNumber == Bus) && (gBridgeInfo[Index1]->Device == Device)
                    && (gBridgeInfo[Index1]->Function == Function)) {
                    Subordinate = gBridgeInfo[Index1]->SecBusNumber;
                }
            }
        }
    }

    gPciCfg->Write (gPeiServices, \
                        gPciCfg, \
                        EfiPeiPciCfgWidthUint8, \
                        PEI_PCI_CFG_ADDRESS (Bus,
                        Device, Function, PCI_SUBUS),\
                        &Subordinate);

    return EFI_SUCCESS;

}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: ProgramPciBridge
//
// Description:
//  Programs the PCI Bridge with the given input.
// 
// Input:
//  IN UINT8 Bus        - Bus no. of the Bridge
//  IN UINT8 Device     - Device no. of the Bridge
//  IN UINT8 Function   - Function no. of the Bridge   
//  IN BOOLEAN IsMmio   - Bridge/Device programmed behind this bridge is
//                        MMIO/IO
// Output:
//  EFI_STATUS
//      EFI_OUT_OF_RESOURCES - Resource is not enough for storing informations.
//      EFI_SUCCESS - Bridge has been Programmed successfully.
//
// Modified:
//  gPciProgrammedBridgeInfo, BridgeCount
//
// Referrals:
//  gPciCfg, gPeiServices, gPciProgrammedBridgeInfo
//
// Notes:
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS
ProgramPciBridge (
    IN UINT8 Bus,
    IN UINT8 Device,
    IN UINT8 Function,
    IN UINT16 Address,
    IN BOOLEAN IsMmio
)
{
    PCI_PROGRAMMED_BRIDGE_INFO ProgrammedBridgeInfo;   
    UINT8           CmdReg;
    EFI_STATUS      Status;
    static UINT8    BridgeCount=0;

    if( BridgeCount > MAXIMUM_PCI_BRIDGE ) {
        return EFI_OUT_OF_RESOURCES;
    }

    gPciCfg->Read(gPeiServices, \
                  gPciCfg, \
                  EfiPeiPciCfgWidthUint8,\
                  PEI_PCI_CFG_ADDRESS(Bus,Device,
                  Function,PCI_CMD),\
                  &CmdReg);

    //
    // Program the Bridge, cmd reg, Base and Limit registers...
    //
    if( IsMmio ) {
        CmdReg |= CMD_ENABLE_MEM;
        gPciCfg->Write (gPeiServices, \
                        gPciCfg, \
                        EfiPeiPciCfgWidthUint16, \
                        PEI_PCI_CFG_ADDRESS (Bus,
                        Device, Function, PCI_MEMBASE),\
                        &Address);

        gPciCfg->Write (gPeiServices, \
                        gPciCfg, \
                        EfiPeiPciCfgWidthUint16, \
                        PEI_PCI_CFG_ADDRESS (Bus,
                        Device, Function, PCI_MEMLIMIT),\
                        &Address);

    } else {
        CmdReg |= CMD_ENABLE_IO;
        gPciCfg->Write (gPeiServices, \
                        gPciCfg, \
                        EfiPeiPciCfgWidthUint8, \
                        PEI_PCI_CFG_ADDRESS (Bus,
                        Device, Function, PCI_IOBASE),\
                        &Address);
        gPciCfg->Write (gPeiServices, \
                        gPciCfg, \
                        EfiPeiPciCfgWidthUint8, \
                        PEI_PCI_CFG_ADDRESS (Bus,
                        Device, Function, PCI_IOLIMIT),\
                        &Address);
    }

    gPciCfg->Write (gPeiServices, \
                    gPciCfg, \
                    EfiPeiPciCfgWidthUint8, \
                    PEI_PCI_CFG_ADDRESS (Bus,
                    Device, Function, PCI_CMD),\
                    &CmdReg);

    //
    // Store the Programmed Bridge info.
    //
    ProgrammedBridgeInfo.PrimaryBusNumber = Bus;
    ProgrammedBridgeInfo.Device = Device;
    ProgrammedBridgeInfo.Function = Function;
    ProgrammedBridgeInfo.MemIOBaseLimit = Address;

    if ( IsMmio ) {
        ProgrammedBridgeInfo.IsMMIO = TRUE;
    } else {
        ProgrammedBridgeInfo.IsMMIO = FALSE;
    }

    Status = (**gPeiServices).AllocatePool( gPeiServices,
                                     sizeof(PCI_PROGRAMMED_BRIDGE_INFO),
                                     &gPciProgrammedBridgeInfo[BridgeCount] );
    if ( EFI_ERROR( Status )) {
        return EFI_OUT_OF_RESOURCES;
    }

    *gPciProgrammedBridgeInfo[BridgeCount] = ProgrammedBridgeInfo;
    BridgeCount++;

    return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: ProgramSubordinateForBridgeAbove
//
// Description:
//  Programs the Subordinate number of Bridge which has the secondary Bus no.
//  as PrimaryBusNo value.
// 
// Input:
//  IN UINT8   PrimaryBusNo,
//  IN UINT8   SubordinateBusNo
// Output:
//  VOID
//
// Modified:
//  Subordinate number of Bridge if found.
//
// Referrals:
//  gPciCfg, gPeiServices, gBridgeInfo
//
// Notes:
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>
VOID
ProgramSubordinateForBridgeAbove(
    IN UINT8   PrimaryBusNo,
    IN UINT8   SubordinateBusNo
)
{
    UINT8   Index;    

    for(Index=0; Index<MAXIMUM_PCI_BRIDGE ;Index++) {
        if( gBridgeInfo[Index] ) {
            if(PrimaryBusNo == gBridgeInfo[Index]->SecBusNumber) {
                gPciCfg->Write (gPeiServices, \
                            gPciCfg, \
                            EfiPeiPciCfgWidthUint8, \
                            PEI_PCI_CFG_ADDRESS (gBridgeInfo[Index]->PrimaryBusNumber,
                            gBridgeInfo[Index]->Device, gBridgeInfo[Index]->Function, PCI_SUBUS),\
                            &SubordinateBusNo);
            }
        }
    }
}

// <AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: NotifyOnRecoveryCapsuleLoaded
//
// Description:
//  It is a nofication function after capsule is loaded to reprogram the PCI
//  enumeration.
// 
// Input:
//  IN EFI_PEI_SERVICES          **PeiServices,
//  IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
//  IN VOID                      *InvokePpi
//
// Output:
//  EFI_SUCCESS
//
// Modified:
//
// Referrals:
//  gPciCfg, gPeiServices, gPciProgrammedBridgeInfo
//
// Notes:
//--------------------------------------------------------------------------- 
// <AMI_PHDR_END>
EFI_STATUS NotifyOnRecoveryCapsuleLoaded (
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *InvokePpi )
{
    UINT8       Index;
    UINT8       Index1;
    UINT8       BusNumber = 0;
    UINT16      BaseAddrLimit = 0;
    UINT8       CmdReg;
    UINT8       Bus;
    UINT8       Device;
    UINT8       Function;

    //
    // Program the bridge's Prim./Sec./Subordinate bus number to Zero.
    // and reprogram the membase/memlinit/Iobase/IoLimit , Command reg.
    //        
    for(Index=0; Index<MAXIMUM_PCI_BRIDGE ;Index++) {
        if( gBridgeInfo[Index] ) {
            gPciCfg->Write (gPeiServices, \
                            gPciCfg, \
                            EfiPeiPciCfgWidthUint8, \
                            PEI_PCI_CFG_ADDRESS (gBridgeInfo[Index]->PrimaryBusNumber,
                            gBridgeInfo[Index]->Device, gBridgeInfo[Index]->Function, PCI_PBUS),\
                            &BusNumber);

            gPciCfg->Write (gPeiServices, \
                            gPciCfg, \
                            EfiPeiPciCfgWidthUint8, \
                            PEI_PCI_CFG_ADDRESS (gBridgeInfo[Index]->PrimaryBusNumber,
                            gBridgeInfo[Index]->Device, gBridgeInfo[Index]->Function, PCI_SBUS),\
                            &BusNumber);

            gPciCfg->Write (gPeiServices, \
                            gPciCfg, \
                            EfiPeiPciCfgWidthUint8, \
                            PEI_PCI_CFG_ADDRESS (gBridgeInfo[Index]->PrimaryBusNumber,
                            gBridgeInfo[Index]->Device, gBridgeInfo[Index]->Function, PCI_SUBUS),\
                            &BusNumber);

             for(Index1=0; Index1<MAXIMUM_PCI_BRIDGE ;Index1++) {
                if( gPciProgrammedBridgeInfo[Index1] ) {
                    if( (gPciProgrammedBridgeInfo[Index1]->PrimaryBusNumber ==  gBridgeInfo[Index]->PrimaryBusNumber) &&
                        (gPciProgrammedBridgeInfo[Index1]->Device ==  gBridgeInfo[Index]->Device) &&
                        (gPciProgrammedBridgeInfo[Index1]->Function ==  gBridgeInfo[Index]->Function) ) {
                        

                        Bus      = gBridgeInfo[Index]->PrimaryBusNumber;
                        Device   = gBridgeInfo[Index]->Device;
                        Function = gBridgeInfo[Index]->Function;

                        gPciCfg->Read(gPeiServices, \
                                      gPciCfg, \
                                      EfiPeiPciCfgWidthUint8,\
                                      PEI_PCI_CFG_ADDRESS(Bus,Device,
                                      Function,PCI_CMD),\
                                      &CmdReg);

                        
                        if( gPciProgrammedBridgeInfo[Index1]->IsMMIO ) {  

                            CmdReg &= MASK_MEM_BUS_MASTER;
                            gPciCfg->Write (gPeiServices, \
                                            gPciCfg, \
                                            EfiPeiPciCfgWidthUint16, \
                                            PEI_PCI_CFG_ADDRESS (Bus,
                                            Device, Function, PCI_MEMBASE),\
                                            &BaseAddrLimit);

                            gPciCfg->Write (gPeiServices, \
                                            gPciCfg, \
                                            EfiPeiPciCfgWidthUint16, \
                                            PEI_PCI_CFG_ADDRESS (Bus,
                                            Device, Function, PCI_MEMLIMIT),\
                                            &BaseAddrLimit);
                        } else {                            

                            CmdReg &= MASK_IO_BUS_MASTER;
                            gPciCfg->Write (gPeiServices, \
                                            gPciCfg, \
                                            EfiPeiPciCfgWidthUint8, \
                                            PEI_PCI_CFG_ADDRESS (Bus,
                                            Device, Function, PCI_IOBASE),\
                                            &BaseAddrLimit);
                            gPciCfg->Write (gPeiServices, \
                                            gPciCfg, \
                                            EfiPeiPciCfgWidthUint8, \
                                            PEI_PCI_CFG_ADDRESS (Bus,
                                            Device, Function, PCI_IOLIMIT),\
                                            &BaseAddrLimit);
                        }

                        gPciCfg->Write (gPeiServices, \
                                        gPciCfg, \
                                        EfiPeiPciCfgWidthUint8, \
                                        PEI_PCI_CFG_ADDRESS (Bus,
                                        Device, Function, PCI_CMD),\
                                        &CmdReg);
                    }
                } else {
                    break;
                }
            }
        }
    }

    return EFI_SUCCESS;   
    
}        


//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
