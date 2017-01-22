#pragma once
#include "stdafx.h"
#include "SubRoutines.h"
#include "LivePatcher.h"

#include <Psapi.h>
#include <algorithm>

int DebugProcess(WCHAR* filename, bool isLivePatch);