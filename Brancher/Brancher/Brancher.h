#include "stdafx.h"

#include <TlHelp32.h>
#include <Psapi.h>
#include <DbgHelp.h>

#define MAX_FILE_PATH 512

long WINAPI BranchHandler(PEXCEPTION_POINTERS ExceptionInfo);