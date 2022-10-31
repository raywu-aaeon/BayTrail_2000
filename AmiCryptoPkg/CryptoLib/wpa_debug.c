//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//#ifndef PEI_BUILD

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: wpa_debug.c -  Standard C Run-time Library Interface Wrapper
//
// Description:     
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

#include "includes.h"
#include "common.h"
//#include "os.h"

UINT8 crypto_trace_level = 0; // default - no traces

/**
 * wpa_set_trace_level - sets the debug message print mode
 * @level: print level: 0 - no messages, 1 - minimal, 2 - full
 */
void wpa_set_trace_level(int level)
{
    crypto_trace_level = level;
}
#ifndef CONFIG_NO_STDOUT_DEBUG

#ifdef PEI_BUILD
#include "pei.h"
extern EFI_PEI_SERVICES  **ppPS;
extern VOID PeiTrace(UINTN Level, CONST EFI_PEI_SERVICES **gPeiServices, CHAR8 *sFormat,...);
void wpa_printf(int level, const char *fmt, ...)
{
	CHAR8  Buffer[256];
	va_list	ArgList;
    if(!crypto_trace_level) return;
	ArgList = va_start(ArgList,fmt);
	PrepareStatusCodeString( Buffer, sizeof(Buffer), fmt, ArgList );
	(*ppPS)->ReportStatusCode (
		(EFI_PEI_SERVICES**)ppPS, EFI_DEBUG_CODE, 
		EFI_SOFTWARE_UNSPECIFIED, 0, NULL,
		(EFI_STATUS_CODE_DATA *)Buffer
	);
	if(level != MSG_MISC) PeiTrace(-1, ppPS, "\n");
	va_end(ArgList);
}
#else
VOID Trace(UINTN Level, CHAR8 *sFormat,...);
/**
 * wpa_printf - conditional Trace
 * @level: priority level (MSG_*) of the message
 * @fmt: Trace format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
VOID PrintDebugMessageVaList(UINTN Level, CHAR8 *sFormat, va_list ArgList);
void wpa_printf(int level, const char *fmt, ...)
{
	va_list	ArgList;
    if(!crypto_trace_level) return;
    ArgList = va_start(ArgList,fmt);
	PrintDebugMessageVaList(-1, fmt, ArgList);
    if(level != MSG_MISC) Trace(-1, "\n");
	va_end(ArgList);
}
#endif

void wpa_hexdump(int level, const char *title, const UINT8 *buf,
			 size_t len)
{
	size_t i;
    size_t max_print_len;
    
    switch(crypto_trace_level) {
        case 0: return;
        case 1: max_print_len = 16; break;
        case 2: max_print_len = len; break;
        default: max_print_len = len;
    }

	wpa_printf(MSG_MISC, "%s - hexdump(len=%x):", title, (unsigned long) len);
	if (buf == NULL) {
		wpa_printf(MSG_MISC," [NULL]");
	} else {
		for (i = 0; i < len && i < max_print_len; i++) {
				wpa_printf(MSG_MISC, " %02x", buf[i]);
        }
        if(len>max_print_len)
            wpa_printf(MSG_MISC,"...>>>");
	}
	wpa_printf(MSG_MISC,"\n");
}

/**
 * wpa_debug_printf_timestamp - Print timestamp for debug output
 *
 * This function prints a timestamp in seconds_from_1970.microsoconds
 * format if debug output has been configured to include timestamps in debug
 * messages.
 */
void wpa_debug_print_timestamp(void)
{
	struct os_time tv;

    if(!crypto_trace_level) return;
	os_get_time(&tv);
	wpa_printf(MSG_INFO, "Current Time: %x sec\n", (long) tv.sec);
}

/*
 * For the standard ASCII character set, control characters are those between 
 * ASCII codes 0x00 (NUL) and 0x1f (US), plus 0x7f (DEL). 
 * Therefore, printable characters are all but these, although specific compiler 
 * implementations for certain platforms may define additional control characters 
 * in the extended character set (above 0x7f).
*/
int isprint(UINT8 ch)
{
  return (ch > 0x1f) && (ch < 0x7f)?1:0;
}
/**
 * wpa_hexdump_ascii - conditional hex dump
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown.
 */
void wpa_hexdump_ascii(int level, const char *title, const UINT8 *buf,
			       size_t len)
{
	size_t i, llen;
	const UINT8 *pos = buf;
	const size_t line_len = 32;

    size_t max_print_len;
    
    switch(crypto_trace_level) {
        case 0: return;
        case 1: max_print_len = 32; break;
        case 2: max_print_len = len; break;
        default: max_print_len = len;
    }

//	wpa_debug_print_timestamp();
	if (buf == NULL) {
		wpa_printf(MSG_MISC,"%s - hexdump_ascii(len=%x): [NULL]\n", title, (unsigned long) len);
		return;
	}
	wpa_printf(MSG_MISC, "%s - hexdump_ascii(len=%x):\n", title, (unsigned long) len);
	while (len) {
		llen = len > line_len ? line_len : len;
		wpa_printf(MSG_MISC, "\tHEX  :");
		for (i = 0; i < llen; i++)
			wpa_printf(MSG_MISC," %02x", pos[i]);
//		for (i = llen; i < line_len; i++)
//			wpa_printf(MSG_MISC,"===");
		wpa_printf(MSG_MISC, "\n\tASCII: ");
		for (i = 0; i < llen; i++) {
			if (isprint(pos[i]))
				wpa_printf(MSG_MISC, "%c", pos[i]);
			else
				wpa_printf(MSG_MISC,"_");
		}
//		for (i = llen; i < line_len; i++)
//			wpa_printf(MSG_MISC,"=");
		wpa_printf(MSG_MISC,"\n");
		pos += llen;
		len -= llen;
	}
}
#endif //#ifndef CONFIG_NO_STDOUT_DEBUG
//#endif //#ifndef PEI_BUILD
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
