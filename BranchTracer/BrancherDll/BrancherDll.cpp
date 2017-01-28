// BrancherDll.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "BrancherDll.h"
#include "ProcessUtils.h"

DWORD dwCurrentPid = NULL;
HANDLE hCurrentProcess = NULL;

long WINAPI BranchHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	PEXCEPTION_RECORD record = ExceptionInfo->ExceptionRecord;
	PCONTEXT context = ExceptionInfo->ContextRecord;

	if (record->ExceptionCode == EXCEPTION_BREAKPOINT) {
		BackupBreakPoint(record->ExceptionAddress);
		SetSingleStepContext(context);

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	if (record->ExceptionCode == EXCEPTION_SINGLE_STEP) {
		BYTE *opc = (BYTE *)record->ExceptionAddress;

		if (opc[0] == 0xFF && opc[1] == 0x15) {
#ifdef _WIN64
			DWORD64 called = *(DWORD *)&opc[2];
			called = *(DWORD64 *)((BYTE *)record->ExceptionAddress + called + 6);
#else
			DWORD called = *(DWORD *)&opc[2];
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

				DWORD64 dwStartAddress = (DWORD64)modinfo.lpBaseOfDll;
				DWORD64 dwEndAddress = dwStartAddress + modinfo.SizeOfImage;

				if (called >= dwStartAddress && called <= dwEndAddress) {
					wModuleName = entry.szModule;

					char name[MAX_FILE_PATH];
					sprintf(name, "%ls", entry.szModule);

					SymLoadModule(hCurrentProcess, NULL, name, 0, (DWORD64)entry.modBaseAddr, 0);
					break;
				}
			} while (Module32NextW(hSnapshot, &entry));
			CloseHandle(hSnapshot);

			IMAGEHLP_SYMBOL *pSymbol;
			pSymbol = (IMAGEHLP_SYMBOL *)new BYTE[sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME];

			memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME);
			pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
			pSymbol->MaxNameLength = MAX_SYM_NAME;

#ifdef _WIN64
			DWORD64 dwDisplacement;
#else
			DWORD dwDisplacement;
#endif

			if (SymGetSymFromAddr(hCurrentProcess, called, &dwDisplacement, pSymbol)) {
#ifdef _WIN64
				printf("%p,%p,%ls,%s\n", record->ExceptionAddress, called, wModuleName, pSymbol->Name);
#else
				printf("00000000%p,00000000%p,%ls,%s\n", record->ExceptionAddress, called, wModuleName, pSymbol->Name);
#endif
			}
			else {
#ifdef _WIN64
				printf("%p,%p,%ls,\n", record->ExceptionAddress, called, wModuleName);
#else
				printf("00000000%p,00000000%p,%ls,\n", record->ExceptionAddress, called, wModuleName);
#endif
			}

			SetBreakPoint((LPVOID)((DWORD64)record->ExceptionAddress + 6));
		}
		else {
			SetSingleStepContext(context);
		}
		
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}