#pragma once
#include <wdm.h>

#include <windef.h>

typedef struct {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
    WCHAR maxpath_buf[MAX_PATH];
}MAXPATH_UNICODE_STRING;

// Get the shortened ASCII name of a process directly from its EPROCESS object
// (using stable APIs). Limited to only 15 ASCII characters (a char[16] buffer
// with null terminator).
const char* name_by_eprocess(PEPROCESS proc_obj);

// Get the full name by opening a handle to the process. If opening from a
// handle creation callback, make sure to allow kernel-mode handles through to
// avoid an infinite loop resulting in a kernel crash. The returned structure
// is just a custom UNICODE_STRING so that the entire string can be returned on
// the stack.

// This function is WIP and probably doesn't work right now.
MAXPATH_UNICODE_STRING get_name_by_handle(PEPROCESS process);

bool needs_protection(PEPROCESS process);

