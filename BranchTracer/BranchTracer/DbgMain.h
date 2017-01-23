#pragma once
#include "stdafx.h"
#include "LivePatcher.h"
#include "WindowsHelp.h"
#include "ProcessUtils.h"

#include <Psapi.h>
#include <DbgHelp.h>
#include <algorithm>

int DebugProcess(WCHAR* filename);