#pragma once

// Process handle permission flags
#define PROCESS_TERMINATE                  (1 << 0)  
#define PROCESS_CREATE_THREAD              (1 << 1)  // 0x2
#define PROCESS_SET_SESSIONID              (1 << 2)  // 0x4
#define PROCESS_VM_OPERATION               (1 << 3)  // 0x8
#define PROCESS_VM_READ                    (1 << 4)  // 0x10
#define PROCESS_VM_WRITE                   (1 << 5)  // 0x20
// #define PROCESS_DUP_HANDLE                 (1 << 6)  
#define PROCESS_CREATE_PROCESS             (1 << 7)  
#define PROCESS_SET_QUOTA                  (1 << 8)  
#define PROCESS_SET_INFORMATION            (1 << 9)   // 0x200
#define PROCESS_QUERY_INFORMATION          (1 << 10)  // 0x400
#define PROCESS_SUSPEND_RESUME             (1 << 11)  
#define PROCESS_QUERY_LIMITED_INFORMATION  (1 << 12)  // 0x1000
#define PROCESS_SET_LIMITED_INFORMATION    (1 << 13)  // 0x2000

int test = PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION;

#if (NTDDI_VERSION >= NTDDI_VISTA)
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFFF)
#else
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)
#endif

// #define MAX_PATH (260)

