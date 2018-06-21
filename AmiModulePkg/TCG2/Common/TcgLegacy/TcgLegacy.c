//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TcgNext/Common/TcgLegacy/TcgLegacy.c 1     10/08/13 12:04p Fredericko $
//
// $Revision: 1 $
//
// $Date: 10/08/13 12:04p $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/TcgNext/Common/TcgLegacy/TcgLegacy.c $
//**********************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  TcgLegacy.c
//
// Description:
// Contains functions that handle setting up the handoff between EFI and Legacy
//
//<AMI_FHDR_END>
//*************************************************************************
#include <EFI.h>
#include <TcgLegacy.h>
#include "token.h"
#include <AmiTcg\TCGMisc.h>
#include <Protocol\LegacyRegion.h>
#include <AmiTcg\TcgCommon.h>
#include "protocol\TcgTcmService.h"
#include "protocol\TcgService.h"
#include "protocol\TpmDevice.h"
#include <Protocol\FirmwareVolume2.h>
#include <Protocol\LegacyBios.h>
#include <Protocol\LegacyRegion2.h>
#include <AmiTcg\TrEEProtocol.h>
#include<Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Guid/EventLegacyBios.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include<Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>


EFI_GUID  gEfiAmiTcgWakeEventDataHobGuid = EFI_TCG_WAKE_EVENT_DATA_HOB_GUID;
EFI_GUID  gEfiAmiHobListGuid     = TCG_EFI_HOB_LIST_GUID;
EFI_GUID  gEfiAmiTcgLogHobGuid   = EFI_TCG_LOG_HOB_GUID;
EFI_GUID  gEfiIntTcgProtocolGuid = EFI_TCG_PROTOCOL_GUID;



#define  GUID_VARIABLE_DECLARATION( Variable, Guid ) extern EFI_GUID Variable
EFI_GUID gEfiAmiTHobListGuid = TCG_EFI_HOB_LIST_GUID;

#include "Amitcg\TcgPc.h"

void* AllocateRTMemory (
    UINTN size );

EFI_GUID  TCGLEGX16_FILE_GUID
 = {0x142204e2, 0xc7b1, 0x4af9, 0xa7, 0x29, 0x92, 0x37, 0x58, 0xd9, 0x6d, 0x3};

EFI_GUID  TPM32_FILE_GUID = 
   {0xaa31bc6, 0x3379, 0x41e8, 0x82, 0x5a, 0x53, 0xf8, 0x2c, 0xc0, 0xf2, 0x54};

EFI_GUID  MPTPM_FILE_GUID
  ={0x7d113aa9, 0x6280, 0x48c6, 0xba, 0xce, 0xdf, 0xe7, 0x66, 0x8e, 0x83, 0x7};

EFI_GUID  TCMLEGX16_FILE_GUID
 = {0x1E753E16, 0xDCEF, 0x47d0, 0x9A, 0x38, 0x7A, 0xDE, 0xCD, 0xB9, 0x83, 0xED};

EFI_GUID  TCM32_FILE_GUID = 
   {0xB74E676E,0x3B2E, 0x483f, 0x94, 0x58, 0xC3, 0x78, 0xFE, 0x0A, 0xC6, 0x9F};


EFI_GUID  gEfiLegacyRegionProtocol = EFI_LEGACY_REGION_PROTOCOL_GUID;

#define GET_HOB_TYPE( Hob )     ((Hob).Header->HobType)
#define GET_HOB_LENGTH( Hob )   ((Hob).Header->HobLength)
#define GET_NEXT_HOB( Hob )     ((Hob).Raw + GET_HOB_LENGTH( Hob ))
#define END_OF_HOB_LIST( Hob )  (GET_HOB_TYPE( Hob ) == \
                                 EFI_HOB_TYPE_END_OF_HOB_LIST)

static TPM32HEADER      * installedTpm32 = 0;
static EFI_TCG_PROTOCOL * gTcgProtocol   = 0;
extern TCG_ACPI_TABLE   mTcgAcpiTableTemplate;
UINT8                   Tpm20Support = 0;


#define _CR( Record, TYPE,\
       Field )((TYPE*) ((CHAR8*) (Record) - (CHAR8*) &(((TYPE*) 0)->Field)))

#define TCG_DXE_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TCG_DXE_PRIVATE_DATA, TcgServiceProtocol )

#define TCM_DXE_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TCM_DXE_PRIVATE_DATA, TcgServiceProtocol )

typedef struct _TCG_DXE_PRIVATE_DATA
{
    EFI_TCG_PROTOCOL        TcgServiceProtocol;
    EFI_TPM_DEVICE_PROTOCOL *TpmDevice;
} TCG_DXE_PRIVATE_DATA;

typedef struct _TCM_DXE_PRIVATE_DATA
{
    EFI_TCM_PROTOCOL        TcgServiceProtocol;
    EFI_TPM_DEVICE_PROTOCOL *TpmDevice;
} TCM_DXE_PRIVATE_DATA;

EFI_TREE_PROTOCOL          *TrEEProtocolInstance = NULL;
#define                     MAX_LOG_AREA_SIZE (32 * 1024) // Make this an SDL token


#define SEG_ALIGNMENT    0x10

VOID DummyFunction(IN EFI_EVENT Event, IN VOID *Context) {}


VOID* GetHob(
    IN UINT16 Type,
    IN VOID   *HobStart  )
{
    EFI_PEI_HOB_POINTERS Hob;

    Hob.Raw = HobStart;

    //
    // Return input if not found
    //
    if ( HobStart == NULL )
    {
        return HobStart;
    }

    //
    // Parse the HOB list, stop if end of list or matching type found.
    //
    while ( !END_OF_HOB_LIST( Hob ))
    {
        if ( Hob.Header->HobType == Type )
        {
            break;
        }

        Hob.Raw = GET_NEXT_HOB( Hob );
    }

    //
    // Return input if not found
    //
    if ( END_OF_HOB_LIST( Hob ))
    {
        return HobStart;
    }

    return (VOID*)(Hob.Raw);
}


EFI_STATUS TcgGetNextGuidHob(
    IN OUT VOID          **HobStart,
    IN EFI_GUID          * Guid,
    OUT VOID             **Buffer,
    OUT UINTN            *BufferSize OPTIONAL )
{
    EFI_STATUS           Status;
    EFI_PEI_HOB_POINTERS GuidHob;

    if ( Buffer == NULL )
    {
        return EFI_INVALID_PARAMETER;
    }

    for ( Status = EFI_NOT_FOUND; EFI_ERROR( Status );)
    {
        GuidHob.Raw = *HobStart;

        if ( END_OF_HOB_LIST( GuidHob ))
        {
            return EFI_NOT_FOUND;
        }

        GuidHob.Raw = GetHob( EFI_HOB_TYPE_GUID_EXTENSION, *HobStart );

        if ( GuidHob.Header->HobType == EFI_HOB_TYPE_GUID_EXTENSION )
        {
            if ( (CompareMem( Guid, &GuidHob.Guid->Name, sizeof(EFI_GUID) ))==0)
            {
                Status  = EFI_SUCCESS;
                *Buffer = (VOID*)((UINT8*)(&GuidHob.Guid->Name) 
                          + sizeof (EFI_GUID));

                if ( BufferSize != NULL )
                {
                    *BufferSize = GuidHob.Header->HobLength
                                  - sizeof (EFI_HOB_GUID_TYPE);
                }
            }
        }

        *HobStart = GET_NEXT_HOB( GuidHob );
    }

    return Status;
}


void TcgLogEventProxy(
    TCG_PCR_EVENT* data )
{
    UINT32 n;

    gTcgProtocol->LogEvent( gTcgProtocol, data, &n, 0x01 );
}
void TreeLogEventProxy(
    TCG_PCR_EVENT* data )
{
    return; //unsupported   
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   linkTPMDriver
//
// Description: Establishes link used to synchronize change to the
//              LOG when done through the INT1A interface while DXE is still in
//              control.
//
//
// Input:       IN     EFI_PEI_SERVICES  **PeiServices,
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
void linkTPMDriver(
    IN OUT TPM32HEADER  * tpm32,
    IN EFI_TCG_PROTOCOL *tcgProtocol )
{
    TCG_LOG_HOB                         *TcgLog;
    TCG_EFI_BOOT_SERVICE_CAPABILITY     Cap;
    EFI_PHYSICAL_ADDRESS                EventLogLoc;
    EFI_PHYSICAL_ADDRESS                LastEv;
    BOOLEAN                             IsTrancated;
    EFI_STATUS                          Status;

    gTcgProtocol = tcgProtocol;

    tpm32->lastEventShadow   = 0;
    tpm32->ptrOnTPMFailue    = 0;

    if(Tpm20Support == 0)
    {
        tpm32->efi_log_event_ptr =  (UINTN) (void*)(UINTN)TcgLogEventProxy;

        gTcgProtocol->StatusCheck( gTcgProtocol, &Cap, NULL, &EventLogLoc, &LastEv );
        TcgLog = (TCG_LOG_HOB*)(UINTN)EventLogLoc;
        TcgLog--;

        DEBUG((-1, "\n\n linkTPMDriver: TCGLOG( %x )\n", TcgLog));


        tpm32->log.memptr      = (UINT32)( UINTN ) EventLogLoc;
        tpm32->log.dwSize      = TcgLog->TableMaxSize;
        tpm32->lastEventShadow = 0;

    }else{
        if(TrEEProtocolInstance ==NULL) return;

        tpm32->efi_log_event_ptr =  (UINTN) (void*)(UINTN)TreeLogEventProxy;
        
        Status = TrEEProtocolInstance->GetEventLog(TrEEProtocolInstance, TREE_BOOT_HASH_ALG_SHA1,\
                                           &EventLogLoc, &LastEv, &IsTrancated);
        tpm32->lastEventShadow   = 0;
        tpm32->ptrOnTPMFailue    = 0;
        
        tpm32->log.memptr      = (UINT32)( UINTN ) EventLogLoc;
        tpm32->log.dwSize      = MAX_LOG_AREA_SIZE;
        tpm32->lastEventShadow = 0;
        
    }
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   UnlinkTPM32fromEFI
//
// Description: Breaks the link that exist between TPM32 code and DXE TCG
//              driver. The established link was used to synchronize change to the
//              LOG when done through the INT1A interface while DXE is still in
//              control. Need to break this link one DXE driver is stoped or EFI
//              boots OS ( including calling INT19)
//
//
// Input:       IN     EFI_EVENT  Event,
//              IN       VOID     *Context
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS EFIAPI UnlinkTPM32fromEFI(
    IN EFI_EVENT Event,
    IN VOID      *Context )
{
    TCG_LOG_HOB                     *TcgLog;
    TPM32HEADER                     * tpm32 = (TPM32HEADER*)Context;
    EFI_PHYSICAL_ADDRESS            logStart,         logLast;
    TCG_EFI_BOOT_SERVICE_CAPABILITY TcgCapability;
    BOOLEAN                         IsTrancated;
    EFI_STATUS                      Status;
    UINT32                          FreeLogSize;
    SHA1_PCR_EVENT                   *PtrtoLastEvent;

    if ( installedTpm32 == 0 )
    {
      return EFI_SUCCESS; 
    }
    
    if(Tpm20Support == 0)
    {
        //ASSERT( installedTpm32 == tpm32 );
        DEBUG((-1, "UnlinkTPM32fromEFI: TPM32( %x )\n", tpm32));
        installedTpm32 = 0;

        tpm32->lastEventShadow   = 0;
        tpm32->efi_log_event_ptr = 0;
        Status                   = gTcgProtocol->StatusCheck( gTcgProtocol,
                                                          &TcgCapability,
                                                          NULL,
                                                          &logStart,
                                                          &logLast );

        if ( EFI_ERROR( Status ))
        {
            tpm32->last    = 0;
            tpm32->freelog = 0;
            return Status;
        }
        TcgLog = (TCG_LOG_HOB*)(UINTN)logStart;
        TcgLog--;

        tpm32->last       = (UINT32)( UINTN ) ( logLast - logStart );
        tpm32->TPMAcDeact = TcgCapability.TPMDeactivatedFlag;
        tpm32->freelog    = TcgLog->TableSize;
        tpm32->nextevent  = TcgLog->EventNum + 1;

        tpm32->Tpm20Enabled = 0;
    }else{

        ASSERT( installedTpm32 == tpm32 );
        DEBUG((-1, "UnlinkTPM32fromEFI: TPM32( %x )\n", tpm32));
        installedTpm32 = 0;

        if(TrEEProtocolInstance ==NULL) return EFI_SUCCESS;
       
        Status = TrEEProtocolInstance->GetEventLog(TrEEProtocolInstance, TREE_BOOT_HASH_ALG_SHA1,\
                                           &logStart, &logLast, &IsTrancated);
        tpm32->lastEventShadow   = 0;
        tpm32->ptrOnTPMFailue    = 0;
       
        tpm32->lastEventShadow   = 0;
        tpm32->efi_log_event_ptr = 0;
        
        tpm32->last       = (UINT32)( UINTN ) ( logLast );
        tpm32->TPMAcDeact = 0;
        FreeLogSize = (UINT32)(logLast - logStart);
        FreeLogSize += (sizeof(SHA1_PCR_EVENT)- 1);
        PtrtoLastEvent = (SHA1_PCR_EVENT *)(logLast);
        FreeLogSize += PtrtoLastEvent->EventSize;
        tpm32->freelog    = FreeLogSize;
        tpm32->nextevent  = 11;  //count events in event logs
        tpm32->Tpm20Enabled = 1;

    }

    return EFI_SUCCESS;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   TcmGetRawImage
//
// Description: Loads binary from RAW section of main firwmare volume
//
//
// Input:       IN     EFI_GUID   *NameGuid,
//              IN OUT VOID   **Buffer,
//              IN OUT UINTN  *Size
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS TcmGetRawImage(
    IN EFI_GUID  *NameGuid,
    IN OUT VOID  **Buffer,
    IN OUT UINTN *Size  )
{
    EFI_STATUS                   Status;
    EFI_HANDLE                   *HandleBuffer = 0;
    UINTN                        HandleCount   = 0;
    UINTN                        i;
    EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
    UINT32                       AuthenticationStatus;
    EFI_FV_FILETYPE               FileType;
    EFI_FV_FILE_ATTRIBUTES        FileAttributes;

    
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiFirmwareVolume2ProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
        );

    if ( EFI_ERROR( Status ) || HandleCount == 0 )
    {
        return EFI_NOT_FOUND;
    }

    //
    // Find desired image in all Fvs
    //
    for ( i = 0; i < HandleCount; i++ )
    {
        Status = gBS->HandleProtocol(
            HandleBuffer[i],
            &gEfiFirmwareVolume2ProtocolGuid,
            &Fv
            );

        if ( EFI_ERROR( Status ))
        {
            gBS->FreePool( HandleBuffer );
            return EFI_LOAD_ERROR;
        }

        DEBUG((-1, "Tcm Before ReadSection\n"));
        
        //
        // Try a raw file
        //
        *Buffer = NULL;
        *Size   = 0;
        Status = Fv->ReadFile (
                            Fv,
                            NameGuid,
                            Buffer,
                            Size,
                            &FileType,
                            &FileAttributes,
                            &AuthenticationStatus
                            );
        
        DEBUG((-1, "Tcm ReadSection Status %r \n",  Status));
        if ( !EFI_ERROR( Status ))
        {
            DEBUG((-1, "Buffer Sign  %x \n", *( (UINT16 *) *Buffer)));
            break;
        }
    }
    gBS->FreePool( HandleBuffer );

    if ( i >= HandleCount )
    {
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetRawImage
//
// Description: Loads binary from RAW section of main firwmare volume
//
//
// Input:       IN     EFI_GUID   *NameGuid,
//              IN OUT VOID   **Buffer,
//              IN OUT UINTN  *Size
//
// Output:      EFI STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS GetRawImage(
    IN EFI_GUID  *NameGuid,
    IN OUT VOID  **Buffer,
    IN OUT UINTN *Size  )
{
    EFI_STATUS                   Status;
    EFI_HANDLE                   *HandleBuffer = 0;
    UINTN                        HandleCount   = 0;
    UINTN                        i;
    EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
    UINT32                       AuthenticationStatus;

    
    if(AutoSupportType()){
    	return (TcmGetRawImage(NameGuid,Buffer, Size));
    }
    
    Status = gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiFirmwareVolume2ProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
        );

    if ( EFI_ERROR( Status ) || HandleCount == 0 )
    {
        return EFI_NOT_FOUND;
    }

    //
    // Find desired image in all Fvs
    //
    for ( i = 0; i < HandleCount; i++ )
    {
        Status = gBS->HandleProtocol(
            HandleBuffer[i],
            &gEfiFirmwareVolume2ProtocolGuid,
            &Fv
            );

        if ( EFI_ERROR( Status ))
        {
            gBS->FreePool( HandleBuffer );
            return EFI_LOAD_ERROR;
        }

        //
        // Try a raw file
        //
        *Buffer = NULL;
        *Size   = 0;
        Status  = Fv->ReadSection(
            Fv,
            NameGuid,
            EFI_SECTION_RAW,
            0,
            Buffer,
            Size,
            &AuthenticationStatus
            );

        
        if ( !EFI_ERROR( Status ))
        {
            break;
        }
    }
    gBS->FreePool( HandleBuffer );

    if ( i >= HandleCount )
    {
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   AllocateRTMemory
//
// Description: Allocates memory used by TCG Legacy code during run-time
//
//
// Input:       IN  UINTN size
//
// Output:      VOID*
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
void* AllocateRTMemory(
    IN UINTN size )
{
    EFI_STATUS Status;
    void       * addr = 0;

    Status = gBS->AllocatePool( EfiRuntimeServicesCode, size, &addr );
    return addr;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   FindCompatibility16Table
//
// Description: This function identifies the validity of CSM16 binary by
//                searching "$EFI" and verifying table checksum and returs the
//                location of $EFI table
//
// Input:
//
// Output:      EFI_COMPATIBILITY16_TABLE*
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_COMPATIBILITY16_TABLE* FindCompatibility16Table( )
{
    UINT8    *p = NULL, *p1 = NULL;
    UINT32   *data32;
    UINT32   count32;
    UINT8    chksum, count8, tablelength;

    //
    // Find CSM16 entry point; "$EFI" is in E0000..FFFFF, 16 bytes aligned.
    //
    data32 = (UINT32*)(UINTN)0xE0000;

    for ( count32 = 0; count32 < 0x8000; count32 += 4 )
    {
        if ( data32[count32] != 0x24454649 )
        {
            continue;
        }
        p = (UINT8*)((UINTN)data32 + (count32 << 2));    // Pointer to "$EFI"

        //
        // Verify the checksum
        //
        tablelength = ((EFI_COMPATIBILITY16_TABLE*)p)->TableLength;

        for ( chksum = 0, count8 = 0, p1 = p; count8 < tablelength; count8++ )
        {
            chksum = chksum + *p1++;
        }

        if ( chksum == 0 )
        {
            break;
        }
    }

    if ( count32 == 0x8000 )
    {
        return 0;
    }

    return ((EFI_COMPATIBILITY16_TABLE*)p);
}



VOID*  TcgLegacyLocateHob(
    UINTN                   NoTableEntries,
    EFI_CONFIGURATION_TABLE *ConfigTable,
    EFI_GUID                *HOB_guid )
{
    VOID *HobStart;
    VOID *PtrHob;
    EFI_GUID       localHobListGuid = TCG_EFI_HOB_LIST_GUID;

    while ( NoTableEntries > 0 )
    {
        NoTableEntries-=1;

        if ((!CompareMem(
                 &ConfigTable[NoTableEntries].VendorGuid,
                 &localHobListGuid, sizeof(EFI_GUID)
                 )))
        {
            HobStart = ConfigTable[NoTableEntries].VendorTable;
            if(HobStart == NULL)continue;
            
            if ( !EFI_ERROR(
                     TcgGetNextGuidHob( &HobStart, HOB_guid, &PtrHob, NULL )
                     ))
            {
                return PtrHob;
            }
        }
    }
    return NULL;
}


EFI_STATUS CreateLegacyBootEvent(
    IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY   NotifyFunction,
    IN VOID *pNotifyContext, OUT EFI_EVENT *pEvent
)
{
    static EFI_GUID guidLegacyBoot = EFI_EVENT_LEGACY_BOOT_GUID;
    return gBS->CreateEventEx(
        EVT_NOTIFY_SIGNAL, NotifyTpl,
        (NotifyFunction) ? NotifyFunction : DummyFunction,
        pNotifyContext, &guidLegacyBoot,
        pEvent
    );
}


//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetTCGLegacyInterface
//
// Description: Loads legacy BIOS binary extensions (TcgLegX16,
//                TPM32 and MPTPM ) and initializes the TCG Legacy support.
//
// Input:       IN EFI_HANDLE        ImageHandle,
//              IN EFI_SYSTEM_TABLE *SystemTable
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS InitTCGLegacyInterface(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS                 Status;
    TCG_LEGX16_HEADER          *legX16header;
    void                       * legX16dest;
    UINTN                      legX16size;
    EFI_LEGACY_BIOS_PROTOCOL   *legBios;
    void                       * rawPtr;
    static TpmBinaryStruct     tpmBinary;
    UINT16                     segLegX16;
    TPM32HEADER                * tpm32hdr;
    UINTN                      tpm32size;
    TPMBIOSDriverHeader        * mptpmHdr;
#if TCG_LEGACY == 0
    UINTN                      mpTpmSize;
#endif
    EFI_TCG_PROTOCOL           *tcgProtocol;
    EFI_EVENT                  event;
    EFI_LEGACY_REGION2_PROTOCOL * iRegion;
    EFI_GUID                   gMpDriverHobGuid = EFI_TCG_MPDriver_HOB_GUID;
    FAR32LOCALS                *LegFill         =    NULL;
    UINT32					Granularity;
    void                       *Table           = NULL;
    UINTN                      TableSize        = 0;
    UINTN                      Location         = 0;
    UINTN                      Alignment        = 0;
    EFI_GUID                   gEfiTrEEProtocolGuid = EFI_TREE_PROTOCOL_GUID;
    
    if ( installedTpm32 != 0 )
    {
        ASSERT( installedTpm32 == 0 );
        return EFI_UNSUPPORTED;
    }
    
    //
    // Access EFI TPM driver
    //
    Status = gBS->LocateProtocol( &gEfiIntTcgProtocolGuid, NULL,
                                  &tcgProtocol );

    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1, "TCG Service is not found: error=%x\n",  Status));
        tcgProtocol = 0;
        Status = gBS->LocateProtocol( &gEfiTrEEProtocolGuid, NULL, &TrEEProtocolInstance );
        if(EFI_ERROR(Status))return Status;
        Tpm20Support = 1;
    }


    //
    // Access legacyBios internals
    //
    Status = gBS->LocateProtocol(
            &gEfiLegacyBiosProtocolGuid,
            NULL,
            &legBios
            );

    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1, "Failed Locate Legacybiosprotocol=%r\n",  Status));
        return Status;
    }

#if TCG_LEGACY == 0
    if(AutoSupportType()){
        TCGLEGX16_FILE_GUID = TCMLEGX16_FILE_GUID;
        TPM32_FILE_GUID = TCM32_FILE_GUID;
    }
#else
	TCGLEGX16_FILE_GUID = TCMLEGX16_FILE_GUID;
        TPM32_FILE_GUID = TCM32_FILE_GUID;
#endif


    //
    // Load LEGX16 into legacy region
    //
    rawPtr = 0;
    Status = GetRawImage( &TCGLEGX16_FILE_GUID, &rawPtr, &legX16size    );

    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1, "Failed to load LEGX16: error=%x\n",  Status));
        return Status;
    }
    legX16header = (TCG_LEGX16_HEADER* )rawPtr;
    
    Status = legBios->GetLegacyRegion( legBios, legX16size + SEG_ALIGNMENT, 0,
            SEG_ALIGNMENT, &legX16dest);
            
    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1, "GetLegacy Region Status =%r\n",  Status));        
        return Status;
    }
    
    DEBUG((-1, "GetLegacy Region Status =%r\n",  Status));        
        
    segLegX16 = (UINT16)((UINTN)legX16dest >> 4 );

    //
    // Load TPM32 into runtime-code memory
    //
    Status = GetRawImage( &TPM32_FILE_GUID, &rawPtr, &tpm32size    );

    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1, "Failed to load TPM32: error=%x\n",  Status));
        return Status;
    }
    tpm32hdr = AllocateRTMemory( tpm32size );
    gBS->CopyMem( tpm32hdr, rawPtr, tpm32size );

    //
    // Load MPTPM into runtime-code memory
    //
#if TCG_LEGACY == 0
    if(!(AutoSupportType())){
      Status = GetRawImage( &MPTPM_FILE_GUID, &rawPtr, &mpTpmSize );

      if ( EFI_ERROR( Status )){
            DEBUG((-1, "Failed to load MPTPM: error=%x\n",  Status));
            return Status;
        }
        mptpmHdr = AllocateRTMemory( mpTpmSize );
        gBS->CopyMem( mptpmHdr, rawPtr, mpTpmSize );
    }else{
        LegFill = (FAR32LOCALS*)TcgLegacyLocateHob( gST->NumberOfTableEntries,
                                               gST->ConfigurationTable,
                                               &gMpDriverHobGuid );
        mptpmHdr = (TPMBIOSDriverHeader*)(UINTN)( LegFill->Offset - LegFill->Codep );
    }
#else
    LegFill = (FAR32LOCALS*)TcgLegacyLocateHob( gST->NumberOfTableEntries,
                                               gST->ConfigurationTable,
                                               &gMpDriverHobGuid );
    mptpmHdr = (TPMBIOSDriverHeader*)(UINTN)( LegFill->Offset - LegFill->Codep );
#endif



    //
    // Link pointers from TPM32 to MPTPM
    //
    tpm32hdr->MP_HDR         = (UINT32)( UINTN ) mptpmHdr;
    tpm32hdr->scratch.memptr =  (UINT32)( UINTN ) AllocateRTMemory(
        TPM32_SCRATCHMEM_SIZE );
    tpm32hdr->scratch.dwSize = TPM32_SCRATCHMEM_SIZE;
    {
        UINT32 * p
            =  (UINT32*)((UINT8*)legX16header + legX16header->wTpm32entryPtr);
        *p  = (UINT32)( UINTN ) tpm32hdr + tpm32hdr->entryPoint;
    }
    legX16header->wTpm32hdr = (UINT32)( UINTN ) tpm32hdr;
    linkTPMDriver( tpm32hdr, tcgProtocol );
    legX16header->regMOR = ((TCG_MOR_REG) << 8);

    //
    // Copy final LegX16 to dest (F000-E000 area)
    //
    legBios->CopyLegacyRegion( legBios, legX16size, legX16dest, legX16header );
    legX16header = (TCG_LEGX16_HEADER* )legX16dest;


    //
    // Unlock E000-F000: Init will update variable inside LEGX16
    //
    Status = gBS->LocateProtocol( &gEfiLegacyRegion2ProtocolGuid, NULL, &iRegion );

    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1,
               "Failed to locate Legacy Region Protocol to unlock E000: %r\n",
               Status));
        return Status;
    }
    Status = iRegion->UnLock( iRegion, 0xE0000, 0x20000,  &Granularity);

    //
    // Init LEGX16
    //
    {
        EFI_IA32_REGISTER_SET Regs;

        if ( EFI_ERROR( Status ))
        {
            DEBUG((-1, "Failed to unlock Legacy region E000: %r\n",
                   Status));
            return Status;
        }

       gBS->SetMem( &Regs, sizeof (EFI_IA32_REGISTER_SET), 0 );
        legBios->FarCall86(
            legBios,
            segLegX16,
            legX16header->InitCode,
            &Regs,
            NULL,
            0
            );
    }

    //
    // Lock E000-F000
    //
    iRegion->Lock( iRegion, 0xE0000, 0x20000, &Granularity );

    //
    // Fills TpmBinaryStruct : Setup int1Ahook, int19Hook, and osloaderHook
    //
    tpmBinary.wLegX16Seg     = segLegX16;
    tpmBinary.int1a_dispatch = legX16header->int1a_dispatch;
    tpmBinary.int19Ev        = legX16header->int19Ev;
    tpmBinary.intLoaderEv    = legX16header->intLoaderEv;
    tpmBinary.BevBcvEv       = legX16header->BevBcvEv;

    //
    // Hookup for BootimeServices shutdown: need to strip *installedTpm32 off the
    // pointers to BootTime data
    //
    installedTpm32 = tpm32hdr;

    #if defined(EFI_EVENT_SIGNAL_READY_TO_BOOT) && EFI_SPECIFICATION_VERSION <\
    0x20000
    EFI_VERIFY(
        Status = gBS->CreateEvent( EFI_EVENT_SIGNAL_LEGACY_BOOT,
                                   EFI_TPL_CALLBACK, UnlinkTPM32fromEFI,
                                   tpm32hdr, &event ));
    #else
    EFI_VERIFY(
        Status = CreateLegacyBootEvent(
            EFI_TPL_CALLBACK,
            UnlinkTPM32fromEFI,
            tpm32hdr,
            &event
            ));
    #endif
    DEBUG((-1, "\tLEGX16: %x:%x, %x, %x\n",
           tpmBinary.wLegX16Seg, tpmBinary.int1a_dispatch, tpmBinary.int19Ev,
           tpmBinary.intLoaderEv));
    DEBUG((-1, "\tTPM32: header:%x entry:%x log:%x logsize:%x\n",
           (UINT32)( UINTN ) tpm32hdr, (UINT32)( UINTN ) tpm32hdr
           + tpm32hdr->entryPoint));
    DEBUG((-1, "\tMPTPM: %x\n", (UINT32)( UINTN ) mptpmHdr));

    //
    // Returns table pointing to TpmBinaryStruct
    //
    if ( Table != NULL && TableSize != 0 )
    {
        Table     = &tpmBinary;
        TableSize = (UINTN)sizeof(tpmBinary);

        if ( Location )
        {
            Location = 0;
        }

        if ( Alignment )
        {
            Alignment = SEG_ALIGNMENT;
        }
    }
    else {
        //
        // Put them int Leg Segment on our own
        //
        void                      * legPtr = 0;
        EFI_COMPATIBILITY16_TABLE * bfi16  = 0;
		UINT8					  * p;
    	UINT8   				  chksum;
		UINT8					  i=0;

        EFI_VERIFY(
            legBios->GetLegacyRegion( legBios, sizeof(tpmBinary)
                                      + SEG_ALIGNMENT, 0,
                                      SEG_ALIGNMENT, &legPtr ));
        Status = iRegion->UnLock( iRegion, 0xE0000, 0x20000, &Granularity );
        gBS->CopyMem( legPtr, &tpmBinary, sizeof(tpmBinary));
        bfi16 = FindCompatibility16Table( );

        if ( bfi16 != 0 )
        {
            DEBUG((-1, "\tFound BFI at 0x%x\n", bfi16));
            bfi16->TpmSegment =  (UINT16)((UINTN)legPtr >> 4 );
            bfi16->TpmOffset  = 0;

			bfi16->TableChecksum = 0;
   			p = (UINT8*)bfi16;
    		for (chksum=0, i=0; i<bfi16->TableLength; i++) {
        	chksum+=*p++;
    		}
    		bfi16->TableChecksum = ~(--chksum);
        }
				
		
        else {
                 DEBUG((-1, "\t!!!Not Found BFI\n"));
        }
        iRegion->Lock( iRegion, 0xE0000, 0x20000, &Granularity );
    }


    return EFI_SUCCESS;
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
