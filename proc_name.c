#include <ntifs.h>
#include <wdm.h>
#include <stdbool.h>

#include "driver_print.h"
#include "proc_name.h"
#include "crc32.h"

NTSTATUS (*ZwQueryInformationProcess)(
    HANDLE           ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
);

typedef PCHAR (*GET_PROCESS_IMAGE_NAME) (PEPROCESS Process);
GET_PROCESS_IMAGE_NAME gGetProcessImageFileName;

// List of processes/services to protect
static const char* targets[] = {
    "notepad.exe",
    "PDUWP.exe"
};

bool needs_protection(PEPROCESS process) {
    bool result = false;
    const uint32_t name_count = sizeof(targets) / sizeof(*targets);

    const char* cur_name = name_by_eprocess(process);
    const uint32_t cur_hash = crc32str(cur_name);

    for (uint32_t i = 0; i < name_count; i++) {
        const char* target_name = targets[i];
        const uint32_t target_hash = crc32str(target_name);

        result |= (target_hash == cur_hash);
    }
    return result;
}

const char* name_by_eprocess(PEPROCESS proc_obj) {
    UNICODE_STRING sPsGetProcessImageFileName = RTL_CONSTANT_STRING(L"PsGetProcessImageFileName");
    gGetProcessImageFileName = (GET_PROCESS_IMAGE_NAME)
    MmGetSystemRoutineAddress( &sPsGetProcessImageFileName );
    if (gGetProcessImageFileName != NULL) {
        return gGetProcessImageFileName(proc_obj);
    }
    return NULL;
}

MAXPATH_UNICODE_STRING make_maxpath_unicode() {
    MAXPATH_UNICODE_STRING output = {0};
    output.Buffer = (PWSTR)&output.maxpath_buf;
    output.MaximumLength = sizeof(WCHAR) * sizeof(MAX_PATH);

    return output;
}

MAXPATH_UNICODE_STRING get_name_by_handle(PEPROCESS process) {
    // Buffer is sizeof(UNICODE_STRING) + (sizeof(WCHAR) * MAX_PATH).
    MAXPATH_UNICODE_STRING proc_name_buf = make_maxpath_unicode();

    UNICODE_STRING* proc_name = (UNICODE_STRING*)&proc_name_buf;

    void* process_handle = NULL;
    KIRQL original_irql = 0;
    if (KeGetCurrentIrql() < 1) {
        KeRaiseIrql(APC_LEVEL, &original_irql);
    }
    ObOpenObjectByPointer(process, OBJ_KERNEL_HANDLE, NULL, 0, NULL, KernelMode, &process_handle);

    KeLowerIrql(original_irql);

    NTSTATUS read_name = ZwQueryInformationProcess(process_handle, ProcessImageFileName, &proc_name_buf, sizeof(proc_name_buf), NULL);
    if (read_name == STATUS_INFO_LENGTH_MISMATCH) {
        dbg_printf("Not enough space to get process name.\n");
        return proc_name_buf;
    }
    ZwClose(process_handle);

    dbg_printf("Process name of handle target is %ws\n", proc_name->Buffer);

    return proc_name_buf;
}

