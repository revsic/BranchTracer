#include "stdafx.h"
#include "BinaryPatcher.h"
#include "DbgMain.h"
#include "DataTypes.h"

#include <stdio.h>

int main() {
	char *target = "C:\\dbg\\debuggee2.exe";
	WCHAR *wtarget = L"C:\\dbg\\debuggee2.exe";

	BaseFileInfo* bf = BinaryPatcher(target);
	DebugProcess(wtarget);
}