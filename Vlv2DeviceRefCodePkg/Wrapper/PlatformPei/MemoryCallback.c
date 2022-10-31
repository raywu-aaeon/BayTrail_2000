#include "Platform.h"
#include <Ppi/Cache.h>

STATIC
EFI_STATUS
GetMemorySize(
    IN  CONST EFI_PEI_SERVICES    **PeiServices,
    OUT UINT64              *LowMemoryLength,
    OUT UINT64              *HighMemoryLength
)
{
    EFI_STATUS              Status;
    EFI_PEI_HOB_POINTERS    Hob;

    *HighMemoryLength = 0;
    *LowMemoryLength = 0x100000;
    //
    // Get the HOB list for processing
    //
    Status = (*PeiServices)->GetHobList(PeiServices, &Hob.Raw);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Collect memory ranges
    //
    while(!END_OF_HOB_LIST(Hob)) {
        if(Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
            if(Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
                //
                // Need memory above 1MB to be collected here
                //
                if(Hob.ResourceDescriptor->PhysicalStart >= 0x100000 &&
                        Hob.ResourceDescriptor->PhysicalStart < (EFI_PHYSICAL_ADDRESS) 0x100000000) {
                    *LowMemoryLength += (UINT64)(Hob.ResourceDescriptor->ResourceLength);
                } else if(Hob.ResourceDescriptor->PhysicalStart >= (EFI_PHYSICAL_ADDRESS) 0x100000000) {
                    *HighMemoryLength += (UINT64)(Hob.ResourceDescriptor->ResourceLength);
                }
            }
        }
        Hob.Raw = GET_NEXT_HOB(Hob);
    }

    return EFI_SUCCESS;
}

/**
  This function will be called when MRC is done.

  @param  PeiServices General purpose services available to every PEIM.

  @param  NotifyDescriptor Information about the notify event..

  @param  Ppi The notify context.

  @retval EFI_SUCCESS If the function completed successfully.
**/
EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyCallback(
    IN EFI_PEI_SERVICES           **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
    IN VOID                       *Ppi
)
{
    UINT8  Index;

    MTRR_SETTINGS                         MtrrSetting;
    UINT64                                MemoryLength;
    UINT64                                MemOverflow;
    UINT64                                MemoryLengthUc;
    UINT64                                MaxMemoryLength;
    UINT64                                LowMemoryLength;
    UINT64                                HighMemoryLength;
    UINT64                                MsrData;

    //
    //Output Message for MFG
    //
    DEBUG((EFI_D_ERROR, "MRC INIT DONE\n"));
    //
    // Variable initialization
    //
    LowMemoryLength = 0;
    HighMemoryLength = 0;
    MemoryLengthUc = 0;

    //
    // Determine memory usage
    //
    GetMemorySize(
        PeiServices,
        &LowMemoryLength,
        &HighMemoryLength
    );

    MaxMemoryLength = LowMemoryLength;
    // Round up to nearest 256MB
    MemOverflow = (LowMemoryLength & 0x0fffffff);
    if(MemOverflow != 0) {
        MaxMemoryLength = LowMemoryLength + (0x10000000 - MemOverflow);
    }

    ZeroMem(&MtrrSetting, sizeof(MTRR_SETTINGS));
    for(Index = 0; Index < 2; Index++) {
        MtrrSetting.Fixed.Mtrr[Index]=0x0606060606060606;
    }
    for(Index = 3; Index < 11; Index++) {
        MtrrSetting.Fixed.Mtrr[Index]=0x0505050505050505;
    }

//
    // Cache the flash area to improve the boot performance in PEI phase
    //
    MtrrSetting.Variables.Mtrr[0].Base = (FixedPcdGet32(PcdFlashAreaBaseAddress) | CacheWriteProtected);
    // (EIP127918+)>>
    MtrrSetting.Variables.Mtrr[0].Mask = (((~(UINT64)(FixedPcdGet32(PcdFlashAreaSize) - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED);
    // (EIP127918+)<<
    
    Index = 1;
    MemOverflow =0;
    while(MaxMemoryLength > MemOverflow) {
        MtrrSetting.Variables.Mtrr[Index].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;
        MemoryLength = MaxMemoryLength - MemOverflow;
        MemoryLength = GetPowerOfTwo64(MemoryLength);
        MtrrSetting.Variables.Mtrr[Index].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
        DEBUG((EFI_D_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));

        MemOverflow += MemoryLength;
        Index++;
    }

//(CSP20130221E+)>>
    MemoryLength = LowMemoryLength;

    while(MaxMemoryLength != MemoryLength) {
        MemoryLengthUc = GetPowerOfTwo64(MaxMemoryLength - MemoryLength);

        MtrrSetting.Variables.Mtrr[Index].Base = ((MaxMemoryLength - MemoryLengthUc) & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheUncacheable;
        MtrrSetting.Variables.Mtrr[Index].Mask= ((~(MemoryLengthUc   - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
        DEBUG((EFI_D_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));
        MaxMemoryLength -= MemoryLengthUc;
        Index++;
    }

    MemOverflow =0x100000000;
    while(HighMemoryLength > 0) {
        MtrrSetting.Variables.Mtrr[Index].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;
        MemoryLength = HighMemoryLength;
        MemoryLength = GetPowerOfTwo64(MemoryLength);

        if(MemoryLength > 0x100000000) {
            MemoryLength = 0x100000000;	//Cap at Max 4G
        }

        MtrrSetting.Variables.Mtrr[Index].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
        DEBUG((EFI_D_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));

        MemOverflow += MemoryLength;
        HighMemoryLength -= MemoryLength;
        Index++;
    }
//(CSP20130221E+)<<

// (EIP127918+)>>
    //
    // set FE/E bits for IA32_MTRR_DEF_TYPE
    //
    MtrrSetting.MtrrDefType |=  3 <<10;
// (EIP127918+)<<

    //
    // Disable No-Eviction Mode
    //
    MsrData = AsmReadMsr64(0x2E0);
    MsrData &= ~BIT1;
    AsmWriteMsr64(0x2E0, MsrData);

    MsrData = AsmReadMsr64(0x2E0);
    MsrData &= ~BIT0;
    AsmWriteMsr64(0x2E0, MsrData);

    AsmInvd();

    MtrrSetAllMtrrs(&MtrrSetting);

    return EFI_SUCCESS;
}
