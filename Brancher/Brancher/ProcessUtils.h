#include "stdafx.h"

#include <TlHelp32.h>
#include <Psapi.h>
#include <DbgHelp.h>

// Set trap flag to make EXCEPTION_SINGLE_STEP.
int SetSingleStepContext(PCONTEXT context);

// Set software breakpoint with 0xCC.
int SetBreakPoint(LPVOID lpAddress);
// Recover current byte which replaced with 0xCC.
int BackupBreakPoint(LPVOID lpAddress);

// Set software breakpoint on entry point.
int SetBreakPointOnEntryPoint();
// Get start and end address of text section.
int GetTextSectionAddress(CDWORD *StartOfTextSection, CDWORD *EndOfTextSection);

// Get module name by address.
int GetModuleNameByAddr(CDWORD dwAddress, WCHAR *wName, SIZE_T size);
// Get symbol name by address.
int GetSymbolName(CDWORD called, WCHAR *name, SIZE_T size);
