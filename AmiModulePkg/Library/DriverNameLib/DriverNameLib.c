#include <Library/DriverNameLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <IndustryStandard/PeImage.h>

BOOLEAN GetPeImageName(UINT8 *p, CHAR8 *sName)
{
	
	UINTN length;
	CHAR8 *s, *q;
    s=PeCoffLoaderGetPdbPointer(p);
    if (s== NULL) return FALSE;
	length = AsciiStrLen(s);
	if (length<5 || AsciiStrCmp(&s[length-4],".pdb")) return FALSE;
	for(q = &s[length-5]; q>=s && *q!='\\'; q--);
	q++;
	length = s+length-4-q;
	CopyMem(sName,q,length);
	sName[length]=0;
	return TRUE;
}

BOOLEAN GetImageNameByHandle(EFI_HANDLE ImageHandle, CHAR8 *sName, UINTN size)
{
	EFI_LOADED_IMAGE_PROTOCOL *pImage;
	EFI_COMPONENT_NAME_PROTOCOL *pComponentName;
	CHAR16 *wsName;
	EFI_STATUS Status;
 	if (   !EFI_ERROR(Status = gDxeCoreST->BootServices->HandleProtocol(ImageHandle,&gEfiLoadedImageProtocolGuid,(VOID **)&pImage))
		&& GetPeImageName((UINT8*)pImage->ImageBase,sName)
	) return TRUE;

	if (   !EFI_ERROR(gDxeCoreST->BootServices->HandleProtocol(ImageHandle,&gEfiComponentName2ProtocolGuid,(VOID **)&pComponentName))
		&& !EFI_ERROR(pComponentName->GetDriverName(pComponentName,"eng",&wsName))
//Try UEFI 2.0 ComponentName protocol
       ||  !EFI_ERROR(gDxeCoreST->BootServices->HandleProtocol(ImageHandle,&gEfiComponentNameProtocolGuid,(VOID **)&pComponentName))
		&& !EFI_ERROR(pComponentName->GetDriverName(pComponentName,"eng",&wsName))
	)
	{
		AsciiSPrint(sName,size,"%S", wsName);
		return TRUE;
	}
    Status = gDxeCoreST->BootServices->HandleProtocol(ImageHandle,&gEfiLoadedImageProtocolGuid,(VOID **)&pImage);
	if (   !EFI_ERROR(Status)
		&& pImage->FilePath->Type==MEDIA_DEVICE_PATH
		&& pImage->FilePath->SubType==MEDIA_PIWG_FW_FILE_DP
	)
	{
 		AsciiSPrint(sName,size,"[%g]",&((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH*)pImage->FilePath)->FvFileName);
		return TRUE;
	}
	AsciiSPrint(sName,size,"Unknown");
	return FALSE;
}

CHAR8* GetDriverName(EFI_DRIVER_BINDING_PROTOCOL *pDriver)
{
	static CHAR8 sName[0x100];
	if (   !GetImageNameByHandle(pDriver->DriverBindingHandle,sName,0x100)
		&& !GetImageNameByHandle(pDriver->ImageHandle,sName,0x100)
	) AsciiSPrint(sName,0x100,"Unknown");
	return sName;
}
