//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2010, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**           5555 Oakbrook Pkwy, Norcross, Georgia 30093       **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
#ifndef __HOTKEY_BIN_H__
#define __HOTKEY_BIN_H__

///EIP 115082 : Referance to Style module is removed. Managed from Style hooks
// Hotkey template moved to StyleHoook.h
extern VOID LegacyHandleEscKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID LegacyHandleHelpKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID LegacyHandlePrevValuesKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID LegacyHandleFailsafeKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID LegacyHandleOptimalKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID LegacyHandleSaveExitKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID LegacyHandlePrnScrnKey( VOID *app, VOID *hotkey, VOID *cookie );
extern VOID	PrntScrnKeyNotification(VOID *app, VOID *hotkey, VOID *cookie );//EIP-123311 

#endif //__HOTKEY_BIN_H__
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2010, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**           5555 Oakbrook Pkwy, Norcross, Georgia 30093       **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
