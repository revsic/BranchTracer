#pragma once

#include "stdafx.h"
#include "lib\distorm.h"

#include "DataTypes.h"

#include <DbgHelp.h>
#include <algorithm>

#define MAX_INSTRUCTIONS 4096

BaseFileInfo* GetOffsetOfInstructions(_In_ char * filename);