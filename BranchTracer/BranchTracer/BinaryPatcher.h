#pragma once
#include "stdafx.h"
#include "utils.h"
#include "DataTypes.h"

#include <stdio.h>

BaseFileInfo* BinaryPatcher(char *filename);
int RevertBinary(char *filename, BaseFileInfo* bf);