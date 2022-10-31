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

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
//**********************************************************************

/** @file CpuDxeBoard.c
    Installs CPU Architectural Protocol and initializes the
    processor interrupt vector table. The CPU Architectural
    Protocol enables/disables/get state of interrupts, set
    memory range cache type, and installs/uninstalls
    interrupt handlers.

**/
#include <Efi.h>
#include <Dxe.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>

#include <Setup.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Cpu.h>
#include <Library/CpuCspLib.h>
#include <Protocol/MpService.h>
#include <Protocol/LegacyBios.h>
#include <Library/CpuPolicy.h>
#include <Library/CpuConfigLib.h>

typedef struct {
    UINT32  ApicId;
    UINT32  Msr;
    UINT64  UcodeVer;
} CPU_UCODE_VERSION;

UINT32 * GetPtrToPrivateAmiCpuInfo2Entry(
    IN UINT32 Package,
    IN UINT32 Core,
    IN UINT32 Thread
);

EFI_STATUS GetCpuInfo(
    IN AMI_CPU_INFO_PROTOCOL    *This,
    IN UINTN                    Cpu,
    OUT AMI_CPU_INFO            **Info
);

static CACHE_DESCRIPTOR_INFO gZeroCacheDesc = {0, 0, 0, 0, 0};

static PKG_CACHE_DESCR *gPkgCacheDesc; //Array of Package Cache Descriptions.

static VOID   *gAcpiData;
static UINT32 gAcpiDataNumEntries;
static EFI_GUID gAmiSetupGuid = SETUP_GUID;

UINTN                     gNumOfCpus;  
UINTN	                  gMaximumNumberOfCPUs;
UINTN	                  gNumberOfEnabledCPUs;
UINTN 	                  gRendezvousIntNumber;
UINTN	                  gRendezvousProcLength;

UINT32                    gNumOfCpuCores;
UINT32                    gNumOfThreads;
UINT32                    gMaxRatioFromBrandStr;

EFI_EVENT 	gAmiMpEvent;
VOID		*gAmiMpEventRegistration = 0;

EFI_MP_SERVICES_PROTOCOL   *gEfiMpServicesProtocol;
EFI_PROCESSOR_INFORMATION  *gEfiMpProcContext;

EFI_GUID gAmiCpuInfoProtocolGuid    = AMI_CPU_INFO_PROTOCOL_GUID;
EFI_GUID gAmiCpuInfo2ProtocolGuid   = AMI_CPU_INFO_2_PROTOCOL_GUID;
AMI_CPU_INFO            *gAmiCpuInfo;

SETUP_CPU_FEATURES  gSetupCpuFeatures;
AMI_CPU_INFO_PROTOCOL gAmiCpuInfoProtocol = {GetCpuInfo};

PRIVATE_AMI_CPU_INFO_2_PROTOCOL *gPrivateAmiCpuInfo2;

UINTN                   *IntTempBuffer;
UINTN                   CSMStart = FALSE;

#ifdef EFI_DEBUG

VOID CheckCpuMsr (
    VOID
);

CPU_UCODE_VERSION  gCpuUcodeVer[16];
#endif
UINT8 GetCacheSharedThreads(IN UINT8 Level);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: GetCpuInfo
//
// Description: Protocol function to get Cpu Info.
//
// Input:
//      IN AMI_CPU_INFO_PROTOCOL    *This
//      IN UINTN                    Cpu
//      OUT AMI_CPU_INFO            **Info
//
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetCpuInfo(
    IN AMI_CPU_INFO_PROTOCOL    *This,
    IN UINTN                    Cpu,
    OUT AMI_CPU_INFO            **Info
)
{
    if (Cpu >= gNumOfCpus) return EFI_INVALID_PARAMETER;
    *Info = &gAmiCpuInfo[Cpu];
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: FindPtrToPrivCpuInfoPkg
//
// Description: Pointer to internal Package information.
//
// Input:
//  IN UINT32  PkgPtr -- Internal package information.
//
// Output:  UINT32 * -- Internal Core information.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 *FindPtrToPrivCpuInfoPkg(
    IN UINT32 Package
)
{
    UINT32 *p = (UINT32*)(gPrivateAmiCpuInfo2 + 1);
    UINT32 i;

    if (Package >= *p) return (UINT32*)-1;   //Package does not exist.
    p++; //now p = Num cores of package 0.

    //Skip entries for previous packages.
    for (i = 0; i < Package; ++i) {
        UINT32 NumCores = *p++;    //p = now number of threads
        UINT32 j;
        for (j = 0; j < NumCores; ++j) {
            UINT32 NumThreads = *p++;
            p += NumThreads * PRIVATE_INFO_NUM_OF_CPU_DATA;
        }
    }
    return p;
}
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: FindPtrToPrivCpuInfoCore
//
// Description: Pointer to internal Core information.
//
// Input:
//  IN UINT32* PkgPtr -- Internal package information.
//  IN UINT32  Core
//
// Output:  UINT32 * -- Internal Core information.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 *FindPtrToPrivCpuInfoCore(
    IN UINT32 *PkgPtr,
    IN UINT32 Core
)
{
    UINT32 *p = PkgPtr;
    UINT32 NumCores = *p++;
    UINT32 i;
    if (Core >= NumCores) return (UINT32*)-1;   //Core does not exist.

    //Skip previous cores.
    for (i = 0; i < Core; ++i) {
        UINT32 NumThreads = *p++;
        p += NumThreads * PRIVATE_INFO_NUM_OF_CPU_DATA;
    }
    return p;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: FindPtrToPrivCpuInfoThread
//
// Description: Pointer to internal Core information.
//
// Input:
//  IN UINT32* CorePtr -- Internal core information.
//  IN UINT32  Thread
//
// Output:  UINT32 * -- Internal thread information.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 *FindPtrToPrivCpuInfoThread(
    IN UINT32 *CorePtr,
    IN UINT32 Thread
)
{
    UINT32 *p = CorePtr;
    UINT32 NumThreads = *p++;
    if (Thread >= NumThreads) return (UINT32*)-1;   //Thread does not exist.
    p += Thread * PRIVATE_INFO_NUM_OF_CPU_DATA;
    return p;
}
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GetBoardSocketNumber
//
// Description: Get socket number from Apic ID.
//
// Input:
//  IN UINT32 ApicId
//
// Output:  UINT32 - Physical Socket Id
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 GetBoardSocketNumber(IN UINT32 ApicId)
{
    UINT32 RegEax, RegEbx, RegEcx, RegEdx;
    UINT8  MaxThreadsPackage;

    CPULib_CpuID(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);

    MaxThreadsPackage = (UINT8)(RegEbx >> 16);
    
    return ApicId / MaxThreadsPackage;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GetNumPackages
//
// Description: Get the number of packages populated and sockets.
//
// Input:
//	IN AMI_CPU_INFO_2_PROTOCOL  *This
//  OUT UINT32                  *NumPopulatedPackages
//  OUT UINT32                  *NumBoardSockets OPTIONAL
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetNumPackages(
	IN AMI_CPU_INFO_2_PROTOCOL  *This,
    OUT UINT32                  *NumPopulatedPackages,
    OUT UINT32                  *NumBoardSockets OPTIONAL
)
{
    //First entry after protocol functions is number of discovered packages.
    /*UINT32 *p = (UINT32*)(gPrivateAmiCpuInfo2 + 1);

    *NumPopulatedPackages = *p;
    //NUMBER_CPU_SOCKETS must be ported if more than 1 socket.
    ASSERT(*NumPopulatedPackages <= NUMBER_CPU_SOCKETS);
    if (NumBoardSockets) {
        *NumBoardSockets = NUMBER_CPU_SOCKETS;

        //In case of porting error, Board sockets can never be less than Populated packages.
        if (*NumPopulatedPackages > *NumBoardSockets) *NumBoardSockets = *NumPopulatedPackages;
    }*/
    
    if(NumPopulatedPackages == NULL)
        return EFI_INVALID_PARAMETER;
        
    *NumPopulatedPackages = 1;

   if (NumBoardSockets)
        *NumBoardSockets = 1;
    
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GetNumCoresThreads
//
// Description: Get the number of Cores
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL  *This,
//  IN UINT32                   Package
//  OUT UINT32                  *NumEnabledCores
//  OUT UINT32                  *NumEnabledThreads - This value is total for package.
//  OUT UINT32                  *NumDisabledCores OPTIONAL -- Flag must be set in Protocol.
//  OUT UINT32                  *NumEnabledThreads OPTIONAL  -- Flag must be set in Protocol.
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetNumCoresThreads(
	IN AMI_CPU_INFO_2_PROTOCOL  *This,
    IN UINT32                   Package,
    OUT UINT32                  *NumEnabledCores,
    OUT UINT32                  *NumEnabledThreads,
    OUT UINT32                  *NumDisabledCores OPTIONAL,
    OUT UINT32                  *NumDisabledThreads OPTIONAL
)
{
    UINT32 *p = FindPtrToPrivCpuInfoPkg(Package);
    UINT32 TotNumCores;
    UINT32 TotNumThreads = 0;
    UINT32 i;

    if (NumEnabledCores == NULL || NumEnabledThreads == NULL)
        return EFI_INVALID_PARAMETER;
        
    if (p == (UINT32*) -1) return EFI_INVALID_PARAMETER;

    //After package is number of cores.
    TotNumCores = *p++;
    for (i = 0; i < TotNumCores; ++i) {
        UINT32 NumThreads = *p++; //After core is Number of Threads
        TotNumThreads += NumThreads;
        p += NumThreads * PRIVATE_INFO_NUM_OF_CPU_DATA;    //APIC ID and CPU NUM;
    }

    *NumEnabledCores = TotNumCores;
    *NumEnabledThreads = TotNumThreads;

    if (NumDisabledCores) *NumDisabledCores = 0;        //Flag not set in Protocol.
    if (NumDisabledThreads) *NumDisabledThreads = 0;    //Flag not set in Protocol.*/

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GetNumThreads
//
// Description: Get the number of Threads
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL  *This
//  IN UINT32                   Package
//  IN UINT32                   Core
//  OUT UINT32                  *NumEnabledThreads
//  OUT UINT32                  *NumDisabledThreads OPTIONAL  -- Flag must be set if valid
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetNumThreads(
	IN AMI_CPU_INFO_2_PROTOCOL  *This,
    IN UINT32                   Package,
    IN UINT32                   Core,
    OUT UINT32                  *NumEnabledThreads,
    OUT UINT32                  *NumDisabledThreads OPTIONAL
)
{
    UINT32 *p = FindPtrToPrivCpuInfoPkg(Package);
    
    if (NumEnabledThreads == NULL) return EFI_INVALID_PARAMETER;
    
    if (p == (UINT32*) -1) return EFI_INVALID_PARAMETER;

    p = FindPtrToPrivCpuInfoCore(p, Core);
    if (p == (UINT32*) -1) return EFI_INVALID_PARAMETER;

    *NumEnabledThreads = *p;
    if (NumDisabledThreads) *NumDisabledThreads = 0;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetSbsp
//
// Description: Get SBSP
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL  *This
//  OUT UINT32                  *Package
//  OUT UINT32                  *Core
//  OUT UINT32                  *Thread
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetSbsp(
	IN AMI_CPU_INFO_2_PROTOCOL  *This,
    OUT UINT32                  *Package,
    OUT UINT32                  *Core,
    OUT UINT32                  *Thread
)
{
    //Desktop system, it must be package0, core0, thread0
    if (Package == NULL || Core == NULL || Thread == NULL)
        return EFI_INVALID_PARAMETER;
        
    *Package = 0;
    *Core = 0;
    *Thread = 0;
    
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetApicInfo
//
// Description: Get Apic Number and Version
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL  *This
//  IN UINT32                   Package
//  IN UINT32                   Core
//  IN UINT32                   Thread
//  OUT UINT32                  *ApicId
//  OUT UINT32                  *ApicVer OPTIONAL
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetApicInfo(
	IN AMI_CPU_INFO_2_PROTOCOL  *This,
    IN UINT32                   Package,
    IN UINT32                   Core,
    IN UINT32                   Thread,
    OUT UINT32                  *ApicId,
    OUT UINT32                  *ApicVer OPTIONAL
)
{
    UINT32 *p = GetPtrToPrivateAmiCpuInfo2Entry(
        Package, Core, Thread);
    UINT32 CpuNum;
    
    if (ApicId == NULL) return EFI_INVALID_PARAMETER;
    
    if (p == (UINT32*) -1) return EFI_INVALID_PARAMETER;

    //p points to 32-bit APIC ID and 32-bit CPU Num for internal structures.

    *ApicId = *p++;

    if (ApicVer) {
        CpuNum = *p;
        *ApicVer = (UINT8)MemRead32((UINT32*)(UINTN)(LOCAL_APIC_BASE + APIC_VERSION_REGISTER));
    }

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetAcpiInfo
//
// Description: Get Cpu ACPI information.
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL  *This
//  OUT VOID                    **AcpiData  - ACPI Data
//  OUT UINT32                  *NumEntries - Number of Entries in data.
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetAcpiInfo(
	IN AMI_CPU_INFO_2_PROTOCOL  *This,
    OUT VOID                    **AcpiData,
    OUT UINT32                  *NumEntries
)
{   
    if (AcpiData == NULL || NumEntries == NULL)
        return EFI_INVALID_PARAMETER;
        
    *AcpiData = gAcpiData;
    *NumEntries = gAcpiDataNumEntries;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetPackageCacheDescr
//
// Description: Get the package Cache Information
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL      *This
//  IN UINT32                       Package - Socket number. Intenal socket number (continous)
//  OUT AMI_CPU_INFO_2_CACHE_DESCR  **Description - Updates pointer to pointer with pointer to Cache information. 
//  OUT UINT32                      *NumEntries - Number of AMI_CPU_INFO_2_CACHE_DESCR Entries.
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetPackageCacheDescr(
    IN AMI_CPU_INFO_2_PROTOCOL      *This,
    IN UINT32                       Package,
    OUT AMI_CPU_INFO_2_CACHE_DESCR  **Description,
    OUT UINT32                      *NumEntries
)
{
    PKG_CACHE_DESCR *PkgCacheDesc;

    if (Package >= NumberOfCpuSocketsPopulated()) return EFI_INVALID_PARAMETER;
    
    if (Description == NULL || NumEntries == NULL)
        return EFI_INVALID_PARAMETER;

    PkgCacheDesc = &gPkgCacheDesc[Package];
    *Description = &PkgCacheDesc->PkgCacheDesc[0];
    *NumEntries = PkgCacheDesc->NumEntries;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetCoreCacheDescr
//
// Description: Get the Core Cache Information
//
// Input:
//  IN AMI_CPU_INFO_2_PROTOCOL      *This
//  IN UINT32                       Package - Socket number. Internal socket number (continous)
//  IN UINT32                       Core - Core number. Internal core number (continous)
//  OUT AMI_CPU_INFO_2_CACHE_DESCR  **Description - Updates pointer to pointer with pointer to Cache information. 
//  OUT UINT32                      *NumEntries - Number of AMI_CPU_INFO_2_CACHE_DESCR Entries.
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetCoreCacheDescr(
    IN AMI_CPU_INFO_2_PROTOCOL      *This,
    IN UINT32                       Package,
    IN UINT32                       Core,
    OUT AMI_CPU_INFO_2_CACHE_DESCR  **Description,
    OUT UINT32                      *NumEntries
)
{
    PKG_CACHE_DESCR *PkgCacheDesc;    

    if (Package >= NumberOfCpuSocketsPopulated()) return EFI_INVALID_PARAMETER;
    
    if (Description == NULL || NumEntries == NULL)
        return EFI_INVALID_PARAMETER;
    
    PkgCacheDesc = &gPkgCacheDesc[Package];
    
    if (Core >= PkgCacheDesc->NumCores) return EFI_INVALID_PARAMETER;

    *Description = &PkgCacheDesc->CoreCacheDesc[0];
    *NumEntries = PkgCacheDesc->NumEntries;
    return EFI_SUCCESS;
}

PRIVATE_AMI_CPU_INFO_2_PROTOCOL PrivateAmiCpuInfo2Init = {
    {
        3,          //ProtocolVer
        0,          //Flags
        GetNumPackages,
        GetNumCoresThreads,
        GetNumThreads,
        GetSbsp,
        GetApicInfo,
        GetAcpiInfo,
        GetPackageCacheDescr,
        GetCoreCacheDescr
    }
    //Additional information will allocated.
};

PRIVATE_AMI_CPU_INFO_2_PROTOCOL *gPrivateAmiCpuInfo2;

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: FillPrivateAmiCpuInfo2
//
// Description: Fill CPU information in Private Ami Cpu Info structure.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID FillPrivateAmiCpuInfo2()
{
    //See PRIVATE_AMI_CPU_INFO_2_PROTOCOL definition for comments on internal CPU information.

    UINT32 *p = (UINT32*)(gPrivateAmiCpuInfo2 + 1);
    UINT32 NumSockets = 1;
    UINT32 CpuNum = 0;
    UINT32 i;
    UINT32 j;
    UINT32 k;

    *gPrivateAmiCpuInfo2 = PrivateAmiCpuInfo2Init;
    //*gPrivateAmiCpuInfo2 = NULL;
    *p++ = NumSockets;
    for (i = 0; i < NumSockets; ++i) {
        *p++ = gNumOfCpuCores;        
        for (j = 0; j < gNumOfCpuCores; ++j) {            
            *p++ = gNumOfThreads;
            for (k = 0; k < gNumOfThreads; ++k) {                
                *p++ = (UINT32)(gEfiMpProcContext[CpuNum].ProcessorId);
                *p++ = CpuNum;
                ++CpuNum;
                ASSERT(CpuNum <= gNumOfCpus);
            }
        }
    }
}
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: SizeOfPrivateAmiCpuInfo2
//
// Description: Size of Private Ami Cpu Info 2 structure to be allocated.
//
// Input: VOID
//
// Output:  UINT32 -- Size
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 SizeOfPrivateAmiCpuInfo2()
{
    //See PRIVATE_AMI_CPU_INFO_2_PROTOCOL defintion for comments on internal CPU information.

    UINT32 Size = sizeof(AMI_CPU_INFO_2_PROTOCOL);
    UINT32 NumSockets = 1;
    UINT32 CpuNum = 0;
    UINT32 j;
    
    Size += sizeof(UINT32);  //Number of populated sockets entry.    
    Size += sizeof(UINT32); //Number of cores for socket;
    for (j = 0; j < gNumOfCpuCores; ++j) {    
        Size += sizeof(UINT32); //Number of thread per core.
        Size += gNumOfThreads * sizeof(UINT32) * PRIVATE_INFO_NUM_OF_CPU_DATA;    //APIC ID and CPU NUM;
        CpuNum += gNumOfThreads;
        ASSERT(CpuNum <= gNumOfCpus);
    }    
    return Size;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: FillAcpiData
//
// Description: Fill ACPI Data structure
//
// Input:
//  IN ACPI_PROCESSOR_INFO *AcpiProcData
//  IN UINT32 Package
//  IN UINT32 Core
//  IN UINT32 Thread
//  IN BOOLEAN Bsp
//
// Output:  BOOLEAN -- If filled, return TRUE.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN FillAcpiData(
    OUT ACPI_PROCESSOR_INFO *AcpiProcData,
    IN UINT32 Package,
    IN UINT32 Core,
    IN UINT32 Thread,
    IN BOOLEAN Bsp
)
{
    UINT32 ApicId;
    UINT32 CpuNum;

    UINT32  CpuSignature;
    UINT32  FeatureFlagsEdx;
    UINT32  FeatureFlagsEcx;
	UINT64  CpuFeatureInfo;

    static UINT32 ProcId = 1;

    UINT32 *ptr = GetPtrToPrivateAmiCpuInfo2Entry(Package, Core, Thread);
    if (ptr == (UINT32*) -1) return FALSE;
    //ptr points to 32-bit APIC ID and 32-bit CPU Num for internal structures.

    CPULib_CpuID(1,
        &CpuSignature, NULL, 
        &FeatureFlagsEcx, &FeatureFlagsEdx
    );

    CpuFeatureInfo = LShiftU64(FeatureFlagsEcx, 32) + FeatureFlagsEdx;

    ApicId = *ptr++;
    CpuNum = *ptr;

    AcpiProcData->Type = ACPI_PROCESSOR_INFO_TYPE;    //0
    AcpiProcData->Length = sizeof(ACPI_PROCESSOR_INFO);
    AcpiProcData->Enable = 1;
    AcpiProcData->Bsp = Bsp;
    AcpiProcData->Package = Package;
    AcpiProcData->Core = Core;
    AcpiProcData->Thread = Thread;
    AcpiProcData->ApicId = ApicId;      //LAPIC number for processor.
    //AcpiProcData->ApicVer = gCpuInfoHob->Cpuinfo[CpuNum].ApicVer;
    AcpiProcData->ApicVer = (UINT8)MemRead32((UINT32*)(UINTN)(LOCAL_APIC_BASE + APIC_VERSION_REGISTER));
	AcpiProcData->CpuSignature = CpuSignature;
    AcpiProcData->FeatureFlags = (UINT32) CpuFeatureInfo;
    AcpiProcData->ProcId = ProcId;     //ASL processor object ID.
    //AcpiProcData->ProcObjPath = (EFI_PHYSICAL_ADDRESS)(UINTN)&gProcObjPath;  //ASL processor object ID.
    AcpiProcData->LocalApicType = FALSE;            //All processors will either be xAPIC or x2APIC Mode not mixed.
	AcpiProcData->ProduceNmi = 1;
    AcpiProcData->NmiFlags = (LAPIC_1_TRIGGER_MODE<<2) | LAPIC_1_POLARITY;
    AcpiProcData->LintnPin = LAPIC_1_DEST_LINTIN;
	
    ++ProcId;
    return TRUE;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: AddLocalApicCoreEntries
//
// Description: Create Private Ami Cpu Info2 Acpi Data.
//
// Input:
//  IN UINT32 Package - Processor package
//  IN UINT32 Thread - Processor thread (usually either 0 or 1 for HT)
//
// Output:  BOOLEAN - TRUE if any entries added.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN AddLocalApicCoreEntries(
    IN UINT32 Package,
    IN UINT32 Thread,
    IN BOOLEAN Bsp,
    IN UINT32 BspCore
){
    UINT32 NumEnabledCores;
    UINT32 NumEnabledThreads;
    UINT32 Core;
    BOOLEAN ValidEntry;
    AMI_CPU_INFO_2_PROTOCOL *AmiCpu2Info = (AMI_CPU_INFO_2_PROTOCOL*)gPrivateAmiCpuInfo2;
    ACPI_PROCESSOR_INFO *AcpiProcData = (ACPI_PROCESSOR_INFO *)gAcpiData;
    static UINT32 Entry = 0;

    AmiCpu2Info->GetNumCoresThreads(AmiCpu2Info, Package, &NumEnabledCores, &NumEnabledThreads, NULL, NULL);
    NumEnabledThreads = NumEnabledThreads / NumEnabledCores;

    if (Thread >= NumEnabledThreads) return FALSE;    //Different packages could have different numbers of threads;

    ValidEntry = FillAcpiData(
        &AcpiProcData[Entry],
        Package,
        BspCore,
        Thread,
        Bsp && Thread == 0
    );
    if (ValidEntry) ++Entry;

    for (Core = 0; Core < NumEnabledCores; ++Core) {
        if (Core == BspCore) continue;
        ValidEntry = FillAcpiData(
            &AcpiProcData[Entry],
            Package,
            Core,
            Thread,
            FALSE
        );
        if (ValidEntry) ++Entry;
    }

    return TRUE;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: CreateAcpiData
//
// Description: Create Private Ami Cpu Info2 Acpi Data.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CreateAcpiData()
{
	EFI_STATUS  Status;
	UINT32      Package;
	UINT32      Thread;
    UINT32      BspPackage;
    UINT32      BspCore;
    UINT32      BspThread;
	UINT32      MaxPackages;
    BOOLEAN     ProcessedEntries;
    AMI_CPU_INFO_2_PROTOCOL *AmiCpu2Info = (AMI_CPU_INFO_2_PROTOCOL*)gPrivateAmiCpuInfo2;

    gAcpiDataNumEntries = (UINT32)gNumOfCpus;
    
    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(ACPI_PROCESSOR_INFO) * gNumOfCpus, &gAcpiData);
    ASSERT_EFI_ERROR(Status);

    Status = AmiCpu2Info->GetSbsp(
	    AmiCpu2Info,
        &BspPackage,
        &BspCore,
        &BspThread
    );
    ASSERT_EFI_ERROR(Status);

    Status = AmiCpu2Info->GetNumPackages(AmiCpu2Info, &MaxPackages, NULL);
    ASSERT_EFI_ERROR(Status);
    Thread = 0;     //Thread count for a core.

    do {            //Thread
        ProcessedEntries = FALSE;

        //Bsp is always first entry.
        if (AddLocalApicCoreEntries(BspPackage, Thread, TRUE, BspCore))
            ProcessedEntries = TRUE;

        for (Package = 0; Package < MaxPackages; ++Package) {
            if (Package == BspPackage) continue;
            if (AddLocalApicCoreEntries(Package, Thread, FALSE, BspCore))
                ProcessedEntries = TRUE;
        }
        ++Thread;
    } while (ProcessedEntries);     //No more threads
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: CreateCacheData
//
// Description: Get Cache information.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CreateCacheData()
{
    AMI_CPU_INFO_2_PROTOCOL *AmiCpu2Info = (AMI_CPU_INFO_2_PROTOCOL*)gPrivateAmiCpuInfo2;
    UINT32  NumPackages;
    UINT32  Package;
    UINT32  CpuNum;
    EFI_STATUS Status;

    BOOLEAN IsLxSharedByPackage[4];
                                                                                                                                                //Assume between CPUs on the board, similar cache sharing. Only like CPUs can power the board.
    IsLxSharedByPackage[1] = GetCacheSharedThreads(1) > 2 ? TRUE: FALSE;
    IsLxSharedByPackage[2] = GetCacheSharedThreads(2) > 2 ? TRUE: FALSE;
    IsLxSharedByPackage[3] = GetCacheSharedThreads(3) > 2 ? TRUE: FALSE;

    Status = AmiCpu2Info->GetNumPackages(AmiCpu2Info, &NumPackages, NULL);
    ASSERT_EFI_ERROR(Status);

    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(PKG_CACHE_DESCR) * NumPackages, &gPkgCacheDesc);
    ASSERT_EFI_ERROR(Status);

    MemSet(gPkgCacheDesc, sizeof(PKG_CACHE_DESCR) * NumPackages, 0);

    //Assume symmetry between the cores in a package. This never likely to change.
//    for(Package = 0, CpuNum = 0; CpuNum < gNumCpus; ++Package) {
    for(Package = 0, CpuNum = 0; CpuNum < gNumOfCpus; ++Package) {
        AMI_CPU_INFO *AmiCpuInfo    = &gAmiCpuInfo[CpuNum];
        PKG_CACHE_DESCR *PkgDesc    = &gPkgCacheDesc[Package];
        CACHE_DESCRIPTOR_INFO *CacheInfo    = AmiCpuInfo->CacheInfo;
        UINT32 NumCores             = AmiCpuInfo->NumCores;
        UINT32 Entry = 0;

        ASSERT (Package < NumPackages);

        PkgDesc->NumCores = NumCores;

        while(CacheInfo->Desc != 0) {
            AMI_CPU_INFO_2_CACHE_DESCR *PkgCacheDesc = &PkgDesc->PkgCacheDesc[Entry];
            AMI_CPU_INFO_2_CACHE_DESCR *CoreCacheDesc = &PkgDesc->CoreCacheDesc[Entry];

            ASSERT(Entry < 4);
            ASSERT(CacheInfo->Level < 4);

            CoreCacheDesc->LengthDesc = sizeof(AMI_CPU_INFO_2_CACHE_DESCR);
            CoreCacheDesc->Level = CacheInfo->Level;
            CoreCacheDesc->Type = CacheInfo->Type;
            CoreCacheDesc->Size = CacheInfo->Size;
            CoreCacheDesc->Associativity = CacheInfo->Associativity;
            CoreCacheDesc->Shared = IsLxSharedByPackage[CacheInfo->Level] + 1;

            if (CoreCacheDesc->Type == 3) {
                CoreCacheDesc->Type = 2;   //Translate type from AMI CPU INFO 1 to AMI CPU INFO 2.
            }

            MemCpy(PkgCacheDesc, CoreCacheDesc, sizeof(AMI_CPU_INFO_2_CACHE_DESCR));

            PkgCacheDesc->Size *= !IsLxSharedByPackage[CacheInfo->Level] ? NumCores : 1;

            ++Entry;
            ++CacheInfo;
        }
        CpuNum += NumCores * (AmiCpuInfo->NumHts ? 2 : 1)                             ;
        PkgDesc->NumEntries = Entry;
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: CreatePrivateAmiCpuInfo2
//
// Description: Create Private Ami Cpu Info2 structure.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CreatePrivateAmiCpuInfo2()
{
    EFI_STATUS Status;

    
    Status = pBS->AllocatePool(
        EfiBootServicesData,
        SizeOfPrivateAmiCpuInfo2(),
        &gPrivateAmiCpuInfo2
    );
    ASSERT_EFI_ERROR(Status);
    
    FillPrivateAmiCpuInfo2();
    
    CreateAcpiData();
    
    CreateCacheData();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: FillCacheDesc
//
// Description: Update cache information with CPUID 4.
//
// Input:
//  CACHE_DESCRIPTOR_INFO * CacheInfo - Array to be filled of cache info structures.
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID FillCacheDesc(CACHE_DESCRIPTOR_INFO * CacheInfo)
{
    CPUID4_EAX_CACHE_INFO EaxInfo;
    CPUID4_EBX_CACHE_INFO EbxInfo;
    UINT32 RegEcx;
    UINT32 RegEdx;
    UINT32 CacheCount;

    for (CacheCount  = 0; CacheCount < (MAX_NUM_CACHE_DESC - 1); ++CacheCount) {
        RegEcx = CacheCount;
        CPULib_CpuID(4, (UINT32*)&EaxInfo, (UINT32*)&EbxInfo, &RegEcx, &RegEdx);
        if (EaxInfo.CacheType == 0) break; //No more cache.

        CacheInfo[CacheCount].Desc = 0xff;  //Unused.
        CacheInfo[CacheCount].Level = EaxInfo.CacheLevel;
        switch (EaxInfo.CacheType) {
        case 1: CacheInfo[CacheCount].Type = 0; break;
        case 2: CacheInfo[CacheCount].Type = 1; break;
        case 3: CacheInfo[CacheCount].Type = 3; break;
        }

        CacheInfo[CacheCount].Size =
            (EbxInfo.Ways + 1) * (EbxInfo.Partitions + 1) * (EbxInfo.LineSize + 1) * (RegEcx + 1) /
             1024;
        CacheInfo[CacheCount].Associativity = EbxInfo.Ways + 1;
    }
    CacheInfo[CacheCount] = gZeroCacheDesc;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
// Procedure:	GetPlatformCpuGroup
//
// Description:	Get the CPU group for the platform policy.
//
// Input:
//      IN VOID *Handle
//
// Output:
//      UINT32 Cpu Group
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 GetPlatformCpuGroup(IN VOID *Handle) {
//    UINT32 CpuSignature = GetCpuSignature() & 0xfffffff0;
//    UINT32 NumCores = NumSupportedCpuCores();
//    if (CpuSignature == NEHALEM_EX)
	    return 0xff;  // ALL, 1, 2
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	DxeInitPlatformCpuLib
//
// Description:	Initialize DXE Init Platform Cpu Lib.
//
// Input:
//  IN EFI_BOOT_SERVICES       *Bs
//  IN EFI_RUNTIME_SERVICES    *Rs
//
// Output:
//  EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeInitSetupCpuFeatures(
    IN EFI_BOOT_SERVICES       *Bs,
    IN EFI_RUNTIME_SERVICES    *Rs
)
{
    UINT32                  CpuSignature = GetCpuSignature() & 0xfffffff0;

    //Only show as supported if all are supported.
    MemSet(&gSetupCpuFeatures, sizeof(gSetupCpuFeatures), 0);     //Initialize features to 0.
    
    gSetupCpuFeatures.CpuGroup = GetPlatformCpuGroup(NULL);
    if (CpuSignature == SANDY_BRIDGE || CpuSignature == JAKETOWN || CpuSignature == IVY_BRIDGE)
	    gSetupCpuFeatures.IsSandyBridge = TRUE;

    gSetupCpuFeatures.NumCores = NumCpuCores();

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	DxeUpdatePlatformCpuFeatures
//
// Description:	Update DXE Platform Cpu Lib.
//
// Input:
//  IN EFI_BOOT_SERVICES       *Bs
//  IN EFI_RUNTIME_SERVICES    *Rs
//
// Output:
//  EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS DxeUpdatePlatformCpuFeatures(
    IN EFI_BOOT_SERVICES       *Bs,
    IN EFI_RUNTIME_SERVICES    *Rs
)
{	
	CPU_FEATURES CpuFeatures;
    EFI_STATUS    Status = EFI_SUCCESS;
    UINT32	CpuSignature = GetCpuSignature();
	UINT32 	CpuSigNoVer = CpuSignature & 0xfffffff0; 	
    UINT64  MsrData, Ia32MiscEnables;
    UINT32  RegEax, RegEbx;


	CPULib_CpuID(1, &RegEax, &RegEbx,
        &CpuFeatures.FeatureEcx, &CpuFeatures.FeatureEdx);

	CPULib_CpuID(0x80000001, &CpuFeatures.ExtFeatureEax, &CpuFeatures.ExtFeatureEbx,
            &CpuFeatures.ExtFeatureEcx, &CpuFeatures.ExtFeatureEdx);

	//Determine SMRR feature support.
	MsrData = ReadMsr(MSR_IA32_MTRR_CAP); //MSR_IA32_MTRR_CAP
	CpuFeatures.Flags.SmrrSupport = !!((UINT32)MsrData & SMRR_SUPPORT_MASK);

	Ia32MiscEnables = ReadMsr(MSR_IA32_MTRR_CAP);
	if ((Ia32MiscEnables & (1 << 16)))  gSetupCpuFeatures.EISTAvailable = TRUE;
	if (IsSmxSupported(&CpuFeatures))  	gSetupCpuFeatures.SmxAvailable = TRUE;
	if (isXDSupported(&CpuFeatures))	gSetupCpuFeatures.XDBitAvailable = TRUE;
	if (isTurboModeSupported())  gSetupCpuFeatures.TurboModeAvailable = TRUE;
	if (isXETdcTdpLimitSupported())		gSetupCpuFeatures.XETdcTdpLimitAvailable = TRUE;
	if (isXECoreRatioLimitSupported())	gSetupCpuFeatures.XETdcTdpLimitAvailable = TRUE;
	if (isLimitCpuidSupported())	gSetupCpuFeatures.LimitCpuidAvailable = TRUE;
	if (IsVmxSupported(&CpuFeatures))  gSetupCpuFeatures.VTAvailable = TRUE;
	if (NumSupportedThreadsPerCore() > 1)  gSetupCpuFeatures.HTAvailable = TRUE;
	if (NumSupportedCpuCores() >  1)  gSetupCpuFeatures.MultiCoreAvailable = TRUE;
	if (NUMBER_CPU_SOCKETS > 1)  gSetupCpuFeatures.MultiSocketAvailable = TRUE;
	if (NumberOfCpuSocketsPopulated() > 1)  gSetupCpuFeatures.MultiSocketAvailable = TRUE;
	if (IsCxInterruptFilteringSupported())  gSetupCpuFeatures.CxIntrFilterAvailable = TRUE;

	if (IsEnergyPerfBiasSupported() && isTurboModeSupported())
		gSetupCpuFeatures.CpuEngPerfBiasAvailable = TRUE;
    if (CpuSigNoVer == WESTMERE) gSetupCpuFeatures.DataReuseOptAvailable = TRUE;

    if (CpuFeatures.FeatureEcx & (1<<3)) {
        UINT32 RegEax, RegEbx, RegEcx, RegEdx;
        CPULib_CpuID(5, &RegEax, &RegEbx, &RegEcx, &RegEdx);
		if (!!(RegEdx & C3_SUB_STATES_MASK)) gSetupCpuFeatures.C3Available = TRUE;

		if (!!(RegEdx & C6_SUB_STATES_MASK)) gSetupCpuFeatures.C6Available = TRUE;

		if (!!(RegEdx & C7_SUB_STATES_MASK)) gSetupCpuFeatures.C7Available = TRUE;

		if ( gSetupCpuFeatures.C3Available|| gSetupCpuFeatures.C6Available
			|| gSetupCpuFeatures.C7Available)
		gSetupCpuFeatures.CxAvailable = TRUE;
    }

	gSetupCpuFeatures.Skt0Pop = TRUE;
	gSetupCpuFeatures.EISTAvailable = FALSE;

	Status = Rs->SetVariable(
			L"SetupCpuFeatures",
			&gAmiSetupGuid,
			EFI_VARIABLE_BOOTSERVICE_ACCESS,
			sizeof(SETUP_CPU_FEATURES),
			&gSetupCpuFeatures
		);
    if (EFI_ERROR(Status)) return Status;

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: CreatePrivateAmiCpuInfo1
//
// Description: Create Private Ami Cpu Info1 structure.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CreatePrivateAmiCpuInfo1()
{
    EFI_STATUS      Status;
    AMI_CPU_INFO    *AmiCpuInfo;
    UINT64          MicroCodeVersion;
    UINT32          CpuSignature; /*, CpuSigNoVer; */
    UINT32          i;
    UINT32          Bclk, Freq;
    UINT32          RegEAX, RegEBX, RegECX, RegEDX;
    UINT32          MyFeatureEcx, MyFeatureEdx;
    UINT32          MyExtFeatureEdx;
    CHAR8           *BrandString;    
    CHAR8           *BrandString1;    
    //UINT64          TimerPeriod;
    UINT8           *pos1;
    BOOLEAN         Ht0 = IsHt0();   //True, if not hyper-threaded CPU.
    //BOOLEAN         IsSandyBridge;
    //UINT32          MaxFreqBrandStr;

    //Allocate memory for AMI_CPU_INFO. This will be filled by CPU initialization.
    Status = pBS->AllocatePool(
        EfiBootServicesData,
        sizeof(AMI_CPU_INFO) * gNumOfCpus,
        &gAmiCpuInfo
    );
    ASSERT_EFI_ERROR(Status);
    
    //Get Cpu Signature
    CpuSignature = GetCpuSignature();
    
    //Get Cpu Fsb, some part of CPUs are 133.3, others are 83.3 or the rest.
    Bclk = GetCpuFsbFromMsr();
    
    Freq = (UINT32)ReadMsr(MSR_PLATFORM_INFO);  //Get Clock multiplier which is embedded in this value.

//    if (IsSandyBridge) {
    	Freq >>= 8;  //ValleyView [15:8]
//    }

    Freq = (Freq & 0xff) * Bclk/10;    //Frequency = Multiplier * Bclk 

    //Allocate memory for Brand string
    Status = pBS->AllocatePool(EfiBootServicesData, 49, &BrandString);
    ASSERT_EFI_ERROR(Status);
    BrandString1 = BrandString;

    //Get Brand string
    CPULib_CpuID(0x80000002, &RegEAX, &RegEBX, &RegECX, &RegEDX);
    *(UINT32*)BrandString = RegEAX; BrandString +=4;
    *(UINT32*)BrandString = RegEBX; BrandString +=4;
    *(UINT32*)BrandString = RegECX; BrandString +=4;
    *(UINT32*)BrandString = RegEDX; BrandString +=4;

    CPULib_CpuID(0x80000003, &RegEAX, &RegEBX, &RegECX, &RegEDX);
    *(UINT32*)BrandString = RegEAX; BrandString +=4;
    *(UINT32*)BrandString = RegEBX; BrandString +=4;
    *(UINT32*)BrandString = RegECX; BrandString +=4;
    *(UINT32*)BrandString = RegEDX; BrandString +=4;

    CPULib_CpuID(0x80000004, &RegEAX, &RegEBX, &RegECX, &RegEDX);
    *(UINT32*)BrandString = RegEAX; BrandString +=4;
    *(UINT32*)BrandString = RegEBX; BrandString +=4;
    *(UINT32*)BrandString = RegECX; BrandString +=4;
    *(UINT32*)BrandString = RegEDX; BrandString +=4;
    *BrandString = '\0';
    
    BrandString = BrandString1;    
        
    //Using CPUID to get related feature
    CPULib_CpuID(1, &RegEAX, &RegEBX, &MyFeatureEcx, &MyFeatureEdx);
    CPULib_CpuID(0x80000001, &RegEAX, &RegEBX, &RegECX, &MyExtFeatureEdx);
    
    //Get loaded Microcode version, MSR 0x8b [EDX] = Microcode version   
    MicroCodeVersion = ReadMsr(0x8b);
    MicroCodeVersion = *((UINT32*)&MicroCodeVersion + 1); //ignore upper 32-bits.

    //TimerPeriod = CalculateTimerPeriod();   //10^-15 s.
    for(i = 0; i < gNumOfCpus; i++ )
    {    
    
        AmiCpuInfo = &gAmiCpuInfo[i];            
        Status = pBS->AllocatePool(EfiBootServicesData, MAX_NUM_CACHE_DESC * sizeof(CACHE_DESCRIPTOR_INFO), &AmiCpuInfo->CacheInfo);
        ASSERT_EFI_ERROR(Status);
            
        FillCacheDesc(AmiCpuInfo->CacheInfo);   //Get Cache Information.
        //Remove leading spaces. After removing leading spaces, the Brand String can not be
        //freed. However, it should never be freed.
        
        AmiCpuInfo->BrandString = BrandString;
        
        while (*AmiCpuInfo->BrandString == ' ') ++AmiCpuInfo->BrandString;
        
        //Remove extra spaces in middle.
        pos1 = AmiCpuInfo->BrandString;
        
        for(;;) {
            UINT8 *pos2;
            UINT8 *pos3;
            while (*pos1 != ' ' && *pos1 != '\0') ++pos1;   //Find next space.
            if (*pos1 == '\0') break;                       //If found terminator, break.
            if (*++pos1 != ' ') continue;                   //If not second space, continue scanning.
            pos2 = pos1;                                    //Found 2 spaces.
            while(*++pos2 == ' ');                          //Skip spaces.
            pos3 = pos1;
            while(*pos2 != '\0') *pos3++ = *pos2++;         //copy string
            *pos3++ = '\0';                                 //Add terminator.
        }       
        
        AmiCpuInfo->Version      = CpuSignature;
        AmiCpuInfo->X64Supported = (MyExtFeatureEdx >> 29) & 1;
        AmiCpuInfo->Ht0          = Ht0;
        AmiCpuInfo->Features     = Shl64(MyFeatureEcx, 32) + MyFeatureEdx;
        AmiCpuInfo->NumCores     = gNumOfCpuCores;
        AmiCpuInfo->NumHts       = IsHtEnabled() * 2;   //Either 2 or 0.
        AmiCpuInfo->FSBFreq      = Bclk/10;
        AmiCpuInfo->Voltage         = 0;    //Voltage is variable, and no information os available.       
        AmiCpuInfo->MicroCodeVers   = (UINT32)MicroCodeVersion;
        AmiCpuInfo->IntendedFreq = Freq; /* ((UINT32)ReadMsr(0x198) >> 8) * Bclk;*/
        //AmiCpuInfo->ActualFreq      = 1000000000/(UINT32)TimerPeriod;            
    }
    //MaxFreqBrandStr = GetMaxSpeedFromBrandString(AmiCpuInfo->BrandString );
    //gMaxRatioFromBrandStr = MaxFreqBrandStr / Bclk;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   CheckCpuMsr
//
// Description: Check the Microcode Loading Status for All APs.
//
// Input: VOID
//
// Output:  VOID
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
#ifdef EFI_DEBUG
VOID
CheckCpuMsr (
	VOID
)
{

    UINT32		RegEbx;
    UINT32		RegEdx;
    UINT64		MicroCodeVersion;

    //Get loaded Microcode version, MSR 0x8b [EDX] = Microcode version   
    MicroCodeVersion = AsmReadMsr64(0x8b);
    MicroCodeVersion = *((UINT32*)&MicroCodeVersion + 1); //ignore upper 32-bits.
	    
    AsmCpuid (1, NULL, &RegEbx, NULL, &RegEdx);
    RegEbx = RegEbx >>24;
    gCpuUcodeVer[RegEbx>>1].ApicId = RegEbx;
    gCpuUcodeVer[RegEbx>>1].Msr = 0x8b;
    gCpuUcodeVer[RegEbx>>1].UcodeVer = MicroCodeVersion;
}
#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   GlobalDataInitialize
//
// Description: Cpu Dxe Entrypoint.
//
// Input:   VOID
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GlobalDataInitialize(VOID)
{
    EFI_STATUS  	Status;
    EFI_PROCESSOR_INFORMATION *ptr;
    UINT32 			i;
    UINTN  			BufferSize;
    CPU_SETUP_DATA	*VlvCpuPolicyData;
    //    UINT64 MsrData = ReadMsr(0x35);
    //    gNumOfCpuCores = ((UINT32)(MsrData >> 16 & 0xff));

    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(CPU_SETUP_DATA) , &VlvCpuPolicyData);
    ASSERT_EFI_ERROR(Status);

    MemSet(VlvCpuPolicyData, sizeof(CPU_SETUP_DATA), 0);
    
    GetCpuSetupData((VOID*)pBS, VlvCpuPolicyData, FALSE);
    
    gNumOfCpuCores = VlvCpuPolicyData->ActiveProcessorCores;
    TRACE((-1, "gNumOfCpuCores %x\n", VlvCpuPolicyData->ActiveProcessorCores));
    //Locate MP services protocol provided by CPU RC
    Status = pBS->LocateProtocol(&gEfiMpServiceProtocolGuid, NULL, &gEfiMpServicesProtocol);    
    ASSERT_EFI_ERROR(Status);
    
    if (!EFI_ERROR(Status)){
        //Get number of Cpus on system
        gEfiMpServicesProtocol->GetNumberOfProcessors(
                                gEfiMpServicesProtocol,
                                &gNumOfCpus,
                                &gNumberOfEnabledCPUs
                                );
    }else{
        return Status;
    }
    
    DEBUG_CODE_BEGIN ();
#ifdef EFI_DEBUG    
{    
        UINT32 i;
        CheckCpuMsr();
        gEfiMpServicesProtocol->StartupAllAPs (
                                gEfiMpServicesProtocol,
                                (EFI_AP_PROCEDURE) CheckCpuMsr,
                                0,
                                NULL,
                                0,
                                NULL,
                                NULL
                                );
	TRACE((-1, " CPU No.   APIC ID    Msr Addr       Msr Value\n"));
	for (i=0; i <gNumOfCpus; i++){
            TRACE((-1, "%8d%10x%12X%16lx\n", i, gCpuUcodeVer[i].ApicId, gCpuUcodeVer[i].Msr, gCpuUcodeVer[i].UcodeVer));
	}
}	
#endif	
    DEBUG_CODE_END ();
     
    gNumOfThreads = (UINT32)(gNumOfCpus / gNumOfCpuCores);    
    //gNumOfThreads = ((UINT32)(MsrData & 0xffff));
    TRACE((-1, "Cpu MP service cpus = %x, cores %x, threads %x\n", gNumOfCpus, gNumOfCpuCores, gNumOfThreads));
    
    //Get MP processor context of each CPU
    Status = pBS->AllocatePool(
        EfiBootServicesData,
        sizeof(EFI_PROCESSOR_INFORMATION) * gNumOfCpus,
        &gEfiMpProcContext
    );
    
    ptr = gEfiMpProcContext;
    BufferSize = sizeof(EFI_PROCESSOR_INFORMATION);
    for(i = 0; i < gNumOfCpus; i++ , ptr++)
    {
        gEfiMpServicesProtocol->GetProcessorInfo(
                                    gEfiMpServicesProtocol,
                                    i,
                                    ptr
                                ); 
    }
    
    return Status;
}    

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetPtrToPrivateAmiCpuInfo2Entry
//
// Description: Get pointer to APIC/Cpu Num
//
// Input:
//  IN UINT32 Package
//  IN UINT32 Core
//  IN UINT32 Thread
//
// Output:  UINT32 *
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 * GetPtrToPrivateAmiCpuInfo2Entry(
    IN UINT32 Package,
    IN UINT32 Core,
    IN UINT32 Thread
)
{
    UINT32 *p;

    p = FindPtrToPrivCpuInfoPkg(Package);
    if (p == (UINT32*) -1) return (UINT32*)-1;   //Package does not exist.

    p = FindPtrToPrivCpuInfoCore(p, Core);
    if (p == (UINT32*) -1) return (UINT32*)-1;   //Core does not exist.

    p = FindPtrToPrivCpuInfoThread(p, Thread);
    return p;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure: GetCpuPkgCoreThrdByNum
//
// Description: Get CPU Package/Core/Thread by CPU Number. Number sequencial to APIC ID.
//
// Input:
//  IN UINT32 CpuNum
//  OUT UINT32 *Package
//  OUT UINT32 *Core
//  OUT UINT32 *Thread
//
// Output:  BOOLEAN -- If found, return TRUE.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN GetCpuPkgCoreThrdByNum(
    IN UINT32 CpuNum,
    OUT UINT32 *Package,
    OUT UINT32 *Core,
    OUT UINT32 *Thread
)
{
    UINT32 *p = (UINT32*)(gPrivateAmiCpuInfo2 + 1);
    UINT32 NumPkgs = *p++;
    UINT32 Pkg;

    for (Pkg = 0; Pkg < NumPkgs; ++Pkg) {
        UINT32 NumCores = *p++;
        UINT32 Cor;
        for (Cor = 0; Cor < NumCores; ++Cor) {
            UINT32 NumThrds = *p++;
            UINT32 Thrd;
            for (Thrd = 0; Thrd < NumThrds; ++Thrd) {
                ++p;    //Skip ApicId;
                if (*p++ == CpuNum) {
                    *Package = Pkg;
                    *Core = Cor;
                    *Thread = Thrd;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   AmiCpuMpServiceCallback
//
// Description: Cpu Dxe Entrypoint.
//
// Input:
//  IN EFI_EVENT       Event    -- 
//  IN VOID *          Context  -- 
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID AmiCpuMpServiceCallback(IN EFI_EVENT Event, IN VOID *Context)
{
    
    
    EFI_STATUS  Status;
    EFI_HANDLE  Handle=0;
    
    //Init global data for later usage
    Status = GlobalDataInitialize();
       
    //Create AMI private CpuInfo1 and CpuInfo2 for AMI other module usage   
    CreatePrivateAmiCpuInfo1();
    CreatePrivateAmiCpuInfo2();

    Status = pBS->InstallProtocolInterface(
                    &TheImageHandle,
                    &gAmiCpuInfo2ProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    gPrivateAmiCpuInfo2
                );

// Patch for CpuMpDxe destroy offset 0
//    if (CSMStart == TRUE) MemCpy((UINTN*)0x0, IntTempBuffer, 256);
    Status = pBS->InstallProtocolInterface(
                    &TheImageHandle,
                    &gAmiCpuInfoProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &gAmiCpuInfoProtocol
                );
    //EFI_DEADLOOP();
    
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   AmiLegacyBiosCallback
//
// Description: 
//
// Input:
//  IN EFI_EVENT       Event    -- 
//  IN VOID *          Context  -- 
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID AmiLegacyBiosCallback(IN EFI_EVENT Event, IN VOID *Context)
{
    EFI_STATUS  Status;

    pBS->CloseEvent(Event);    
    Status = pBS->AllocatePool(EfiBootServicesData, 256, &IntTempBuffer);
    MemCpy(IntTempBuffer, (UINTN*)0x0, 256);
    CSMStart = TRUE;
}  

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   DxeInitializeCpu
//
// Description: Cpu Dxe Entrypoint.
//
// Input:
//  IN EFI_HANDLE       ImageHandle   -- Handle assigned to this driver.
//  IN EFI_SYSTEM_TABLE *SystemTable  -- Efi System table.
//
// Output:  EFI_STATUS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS DxeInitializeCpu(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
    EFI_STATUS    Status = EFI_SUCCESS;
    EFI_MP_SERVICES_PROTOCOL   *MpService;
    EFI_EVENT 	  gLBCallbackEvent;
    VOID          *gLBCallbackRegistration = 0;
//	VOID			*UpdateSetupHandle;

    InitAmiLib(ImageHandle, SystemTable);
    
    Status = pBS->LocateProtocol (
    &gEfiMpServiceProtocolGuid,
    NULL,
    (VOID**)&MpService
    );
       
    if (Status == EFI_SUCCESS)
    {
        TRACE((-1, "MP service is available   \n"));   
    } else {
        TRACE((-1, "MP service is not available   \n"));
    }        

	// Init CPU setup menu variable
    TRACE((-1, "DxeInitializeCpu - 1   \n"));

    Status = DxeInitSetupCpuFeatures(pBS, pRS /*, &UpdateSetupHandle*/);
    if (EFI_ERROR(Status))
    	TRACE((-1, "DxeInitSetupCpuFeatures Faild!!\n"));
    TRACE((-1, "DxeInitializeCpu - 2   \n"));
	DxeUpdatePlatformCpuFeatures(pBS, pRS /*, &UpdateSetupHandle*/);
	TRACE((-1, "DxeInitializeCpu - 3   \n"));

	Status = RegisterProtocolCallback(
    	&gEfiMpServiceProtocolGuid,
        AmiCpuMpServiceCallback,
        NULL,
        &gAmiMpEvent,
        &gAmiMpEventRegistration
    );

    TRACE((-1, "DxeInitializeCpu - 4   \n"));
    // Patch for CpuMpDxe destroy offset 0
    Status = RegisterProtocolCallback(
                &gEfiLegacyBiosProtocolGuid,
                AmiLegacyBiosCallback,
                NULL,
                &gLBCallbackEvent,
                &gLBCallbackRegistration);

    TRACE((-1, "DxeInitializeCpu  %r \n", Status));
    
    return EFI_SUCCESS;
}

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

