#include "stdafx.h"
#include "DbgMain.h"

#include <stdio.h>

int main() {
#ifdef _WIN64
	WCHAR *wtarget = L"C:\\Windows\\notepad.exe";
#else
	WCHAR *wtarget = L"C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe";
#endif

	DebugProcess(wtarget);
}