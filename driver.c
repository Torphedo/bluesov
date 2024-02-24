// This file contains the driver entry points and callbacks.
#include <string.h>
#include <stdbool.h>

#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>

#include <windef.h>

#include "winapi.h"
#include "ntapi.h"
#include "driver_print.h"
#include "proc_name.h"
#include "crc32.h"

OB_PREOP_CALLBACK_STATUS pre_handle_create(void* RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
    // Leave kernel handles alone. This allows us to open a kernel-mode handle for more info about the target process.
    if (OperationInformation->KernelHandle == 1) {
        return OB_PREOP_SUCCESS;
    }

    PEPROCESS process = NULL;
    void* process_handle = NULL;

    // Get PEPROCESS object from either type of handle request
    if (OperationInformation->ObjectType == *PsProcessType) {
        process = OperationInformation->Object;

        // A process can always have PROCESS_ALL_ACCESS on itself.
        PEPROCESS self = PsGetCurrentProcess();
        if (self == process) {
            return OB_PREOP_SUCCESS;
        }

        KIRQL original_irql = 0;
        if (KeGetCurrentIrql() < APC_LEVEL) {
            KeRaiseIrql(APC_LEVEL, &original_irql);
        }
        NTSTATUS open_result = ObOpenObjectByPointer(process, OBJ_KERNEL_HANDLE, NULL, 0, NULL, KernelMode, &process_handle);
        KeLowerIrql(original_irql);

        if (open_result != STATUS_SUCCESS) {
            dbg_printf("Failed to open process??\n");
            return 1;
        }
    }
    else if (OperationInformation->ObjectType == *PsThreadType) {
        process = IoThreadToProcess(OperationInformation->Object);
    }

    // Skip processes that aren't in our list
    if (!needs_protection(process)) {
        return OB_PREOP_SUCCESS;
    }

    // dbg_printf("process name is %s\n", name_by_eprocess(OperationInformation->Object));


    if (OperationInformation->ObjectType == *PsProcessType) {
        ULONG total_time = 0;
        HANDLE thread_id = NULL;
        NTSTATUS result = STATUS_SUCCESS;
        while (result == STATUS_SUCCESS) {
            result = ZwGetNextThread(process_handle, thread_id, THREAD_ALL_ACCESS, 0, 0, &thread_id);
            if (result == STATUS_SUCCESS && thread_id != 0) {
                ULONG usertime = 0;
                PETHREAD thr_ptr = NULL;
                if (PsLookupThreadByThreadId(thread_id, &thr_ptr) == STATUS_SUCCESS) {
                    total_time += KeQueryRuntimeThread(thr_ptr, &usertime);
                }
                // ObDereferenceObject(thr_ptr);
                total_time += usertime;
            }
        }
        dbg_printf("Total usermode ticks: %lu\n", total_time);

        __debugbreak();
        if (total_time == 0) {
            // Thread with zero usermode time are probably trying to start up
            // their process, so we should leave them alone.
            return OB_PREOP_SUCCESS;
        }
    }

    ACCESS_MASK* desired_access = NULL;
    ACCESS_MASK* original_desired_access = NULL;

    switch (OperationInformation->Operation) {
        case OB_OPERATION_HANDLE_CREATE:
            desired_access = &OperationInformation->Parameters->CreateHandleInformation.DesiredAccess;
            original_desired_access = &OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;
            break;
        case OB_OPERATION_HANDLE_DUPLICATE:
            desired_access = &OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess;
            original_desired_access = &OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess;
            break;
        default:
            // This should never happen...return failure
            return 1;
            break;

    }

    ACCESS_MASK proc_strip_flags = PROCESS_TERMINATE | PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD;
    ACCESS_MASK thread_strip_flags = THREAD_TERMINATE;

    // Strip rights from handles
    if (OperationInformation->ObjectType == *PsProcessType) {
        *desired_access &= ~proc_strip_flags;
    }
    else if (OperationInformation->ObjectType == *PsThreadType) {
        *desired_access &= ~thread_strip_flags;
    }

    UNREFERENCED_PARAMETER(RegistrationContext);

    return OB_PREOP_SUCCESS;
}

void post_handle_create(void* RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {
    UNREFERENCED_PARAMETER(RegistrationContext);
    UNREFERENCED_PARAMETER(OperationInformation);
    return;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registry_path) {
    UNREFERENCED_PARAMETER(driver_obj);
    UNREFERENCED_PARAMETER(registry_path);
    
    UNICODE_STRING altitude = {0};
    RtlInitUnicodeString(&altitude, L"322438");

    OB_OPERATION_REGISTRATION registration = {
        .ObjectType = PsProcessType,
        .Operations = OB_OPERATION_HANDLE_CREATE,
        .PreOperation = &pre_handle_create,
        .PostOperation = &post_handle_create
    };


    OB_CALLBACK_REGISTRATION callbacks = {
        .Version = OB_FLT_REGISTRATION_VERSION,
        .OperationRegistrationCount = 1,
        .Altitude = altitude,
        .RegistrationContext = NULL,
        .OperationRegistration = &registration
    };
    void* registration_handle = NULL;

    // Set up our callbacks.
    NTSTATUS status = ObRegisterCallbacks(&callbacks, &registration_handle);

    dbg_printf("sov.sys is installed and running.\n");

    return status;
}

