#pragma once
#include "stdafx.h"
#include "lib\distorm.h"

#include "WindowsHelp.h"

#include <TlHelp32.h>
#include <algorithm>

#define MAX_INSTRUCTIONS 4096

int LivePatcher(DWORD dwProcessId);