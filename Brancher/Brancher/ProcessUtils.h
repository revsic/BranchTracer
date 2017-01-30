#include "stdafx.h"

#include <TlHelp32.h>
#include <Psapi.h>
#include <DbgHelp.h>

int SetSingleStepContext(PCONTEXT context);

int SetBreakPoint(LPVOID lpAddress);
int BackupBreakPoint(LPVOID lpAddress);

int SetBreakPointOnEntryPoint();
int GetTextSectionAddress(CDWORD *StartOfTextSection, CDWORD *EndOfTextSection);

int GetModuleNameByAddr(CDWORD dwAddress, WCHAR *name);
int GetSymolName(CDWORD called, WCHAR *name);