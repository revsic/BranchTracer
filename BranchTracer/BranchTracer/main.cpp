#include "stdafx.h"
#include "BinaryPatcher.h"
#include "DbgMain.h"
#include "DataTypes.h"

#include <stdio.h>

int main() {
	char *target = "C:\\dbg\\iexplore.exe";
	WCHAR *wtarget = L"C:\\dbg\\iexplore.exe";

	BaseFileInfo* bf = BinaryPatcher(target);
	DebugProcess(wtarget);
}