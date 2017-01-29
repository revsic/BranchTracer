#include "stdafx.h"

#include <TlHelp32.h>
#include <Psapi.h>
#include <DbgHelp.h>

int SetSingleStepContext(PCONTEXT context);

int SetBreakPoint(LPVOID lpAddress);
int BackupBreakPoint(LPVOID lpAddress);

int SetBreakPointOnEntryPoint();