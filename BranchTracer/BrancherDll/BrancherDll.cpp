// BrancherDll.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "BrancherDll.h"
#include "ProcessUtils.h"

#ifdef _WIN64
typedef DWORD64 CDWORD;
#else
typedef DWORD CDWORD;
#endif

DWORD dwCurrentPid = NULL;
HANDLE hCurrentProcess = NULL;
CDWORD lpLastBranch = NULL;
int cnt = 0;

long WINAPI BranchHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	PEXCEPTION_RECORD record = ExceptionInfo->ExceptionRecord;
	PCONTEXT context = ExceptionInfo->ContextRecord;

	if (lpLastBranch == (CDWORD)record->ExceptionAddress) {
		++cnt;
		SetBreakPoint((LPVOID)((BYTE *)record->ExceptionAddress + 6));
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	if (record->ExceptionCode == EXCEPTION_BREAKPOINT) {
		BackupBreakPoint(record->ExceptionAddress);
		SetSingleStepContext(context);

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	if (record->ExceptionCode == EXCEPTION_SINGLE_STEP) {
		BYTE *opc = (BYTE *)record->ExceptionAddress;

		if (opc[0] == 0xFF && opc[1] == 0x15) {
			lpLastBranch = (CDWORD)record->ExceptionAddress;

			if (cnt != 0) {
				printf("+%d times repeat\n", cnt);
				cnt = 0;
			}

			CDWORD called = *(DWORD *)&opc[2];

#ifdef _WIN64
			called = *(DWORD64 *)((BYTE *)record->ExceptionAddress + called + 6);
#else
			called = *(DWORD *)called;
#endif
			if (!dwCurrentPid) {
				dwCurrentPid = GetCurrentProcessId();
				hCurrentProcess = GetCurrentProcess();
				SymInitialize(hCurrentProcess, NULL, FALSE);
			}

			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwCurrentPid);
			MODULEENTRY32W entry;
			entry.dwSize = sizeof(entry);

			WCHAR *wModuleName = L"None";

			Module32FirstW(hSnapshot, &entry);
			do {
				MODULEINFO modinfo;
				GetModuleInformation(hCurrentProcess, entry.hModule, &modinfo, sizeof(modinfo));

				CDWORD dwStartAddress = (CDWORD)modinfo.lpBaseOfDll;
				CDWORD dwEndAddress = dwStartAddress + modinfo.SizeOfImage;

				if (called >= dwStartAddress && called <= dwEndAddress) {
					wModuleName = entry.szModule;

					char name[MAX_FILE_PATH];
					sprintf(name, "%ls", entry.szModule);

					SymLoadModule(hCurrentProcess, NULL, name, 0, (CDWORD)entry.modBaseAddr, 0);
					break;
				}
			} while (Module32NextW(hSnapshot, &entry));
			CloseHandle(hSnapshot);

			IMAGEHLP_SYMBOL *pSymbol;
			pSymbol = (IMAGEHLP_SYMBOL *)new BYTE[sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME];

			memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME);
			pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
			pSymbol->MaxNameLength = MAX_SYM_NAME;

			CDWORD dwDisplacement;
			if (SymGetSymFromAddr(hCurrentProcess, called, &dwDisplacement, pSymbol)) {
				printf("+%p,%p,%ls,%s\n", record->ExceptionAddress, called, wModuleName, pSymbol->Name);
			}
			else {
				printf("+%p,%p,%ls,\n", record->ExceptionAddress, called, wModuleName);
			}

			SetBreakPoint((LPVOID)((BYTE *)record->ExceptionAddress + 6));
		}
		else {
			SetSingleStepContext(context);
		}
		
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}