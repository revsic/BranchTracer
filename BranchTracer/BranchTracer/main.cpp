#include "stdafx.h"
#include "BinaryPatcher.h"
#include "DbgMain.h"
#include "DataTypes.h"

#include <stdio.h>

int main() {
#ifdef _WIN64
	char *target = "C:\\dbg\\testapp64_2.exe";
	WCHAR *wtarget = L"C:\\dbg\\testapp64_2.exe";
#else
	char *target = "C:\\dbg\\testapp32_2.exe";
	WCHAR *wtarget = L"C:\\dbg\\testapp32_2.exe";
#endif

	BaseFileInfo* bf = BinaryPatcher(target);
	DebugProcess(wtarget, false);
	RevertBinary(target, bf);
}