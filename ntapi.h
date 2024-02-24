#pragma once
#include <ntdef.h>
#include <wdm.h>

// The linker will figure this out automatically, since it's in ntoskrnl.exe
// (which we're already linking against).
NTSYSCALLAPI NTSTATUS NTAPI ZwGetNextThread(
    HANDLE ProcessHandle,
    HANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    ULONG HandleAttributes,
    ULONG Flags,
    PHANDLE NewThreadHandle
);

