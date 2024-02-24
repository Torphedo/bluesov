# sov
This is a kernel-mode driver for Windows that stops DLL injections. We use a
list of protected process names to block injections on, because denying access
broadly will crash `wininit.exe` and stop Windows from booting.

Tested on Hyper-V with the latest version Windows 10 as of Feburary 2024. The
driver only uses stable APIs, so there should be no issues on newer versions of
Windows. It won't work on builds older than April 2019, because it relies on
`ZwGetNextThread` being exported from `ntoskrnl.exe`.

## Why?
I wrote this as a side project for the cybersecurity club at my university.

## How?
It uses pre handle creation callbacks registered at boot to revoke
process access rights. `PROCESS_VM_*` & `PROCESS_CREATE_THREAD` rights are
revoked from all handles to processes in the whitelist that are already
initialized.

## How is it installed?
The INF file didn't work last I checked (several months ago). I use System
Informer to install it, setting it to be an `FS Driver` starting at `System
start`. I use WinDbg over a virtual COM port for debugging.

## What does the name mean?
This is a kind of stupid & convoluted chain of word association & inside jokes.
Don't worry about it, it doesn't make sense anyway.

