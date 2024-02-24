#include <stdarg.h>
#include <stdio.h>

#include <wdm.h>

void dbg_printf(const char* format, ... ) {
    char message[512];
    va_list va_args;
    va_start(va_args, format);
    const ULONG n = _vsnprintf(message, sizeof(message) - sizeof(CHAR), format, va_args);
    message[n] = '\0';
    vDbgPrintExWithPrefix("[sov] ", DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, message, va_args);
    va_end(va_args);
}

