#include "ProcessUtils.h"

int AppendSubProcess(DWORD dwParentProcessId, DWORD dwProcessId, ProcessInfo& info) {
	if (info.dwProcessId == dwParentProcessId) {
		ProcessInfo *sub = new ProcessInfo();
		sub->dwProcessId = dwProcessId;

		info.SubProcess.push_back(sub);

		return 0;
	}
	else {
		for (auto iter = info.SubProcess.begin(); iter != info.SubProcess.end(); ++iter) {
			int result = AppendSubProcess(dwParentProcessId, dwProcessId, **iter);
			if (result == 0) {
				return 0;
			}
		}

		return 1;
	}
}

int DeleteProcess(DWORD dwProcessId, ProcessInfo& info) {
	for (auto iter = info.SubProcess.begin(); iter != info.SubProcess.end(); ++iter) {
		if ((*iter)->dwProcessId == dwProcessId) {
			info.SubProcess.erase(iter);
			return 0;
		}
		else {
			int result = DeleteProcess(dwProcessId, **iter);
			if (result == 0) {
				return 0;
			}
		}
	}

	return 1;
}

ProcessInfo* FindProcess(DWORD dwProcessId, ProcessInfo& info) {
	if (info.dwProcessId == dwProcessId) {
		return &info;
	}
	else {
		for (auto iter = info.SubProcess.begin(); iter != info.SubProcess.end(); ++iter) {
			ProcessInfo* result = FindProcess(dwProcessId, **iter);
			if (result != NULL) {
				return result;
			}
		}

		return NULL;
	}
}

int ContinueProcess(DEBUG_EVENT& dbgEvent) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dbgEvent.dwProcessId);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dbgEvent.dwThreadId);

	BYTE read = 0;
	BYTE call = 0xE8;
	SIZE_T written = 0;

	EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;
	ReadProcessMemory(hProcess, (LPCVOID)((DWORD64)record.ExceptionAddress + 1), &read, 1, &written);

	if (read == 0x15) {
		call = 0xFF;
	}

	WriteProcessMemory(hProcess, record.ExceptionAddress, &call, 1, &written);

	CONTEXT context;
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &context);

#ifdef _WIN64
	context.Rip -= 1;
#else
	context.Eip -= 1;
#endif

	SetThreadContext(hThread, &context);

	CloseHandle(hProcess);
	CloseHandle(hThread);
	return 0;
}

int LogBranch(HANDLE hSymbol, DEBUG_EVENT& dbgEvent, std::vector<LibraryInfo *> libs) {
	EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dbgEvent.dwProcessId);

	BYTE memory[8];
	SIZE_T written;

	ReadProcessMemory(hProcess, (LPCVOID)((DWORD64)record.ExceptionAddress + 1), memory, 5, &written);
	DWORD64 called = 0;

	if (memory[0] == 0x15) {
		called = (*(DWORD *)(memory + 1));
#ifdef _WIN64
		DWORD64 dwDisplacement;

		ReadProcessMemory(hProcess, (LPCVOID)((DWORD64)record.ExceptionAddress + called + 6), memory, 8, &written);
		called = *(DWORD64 *)(memory);
#else
		DWORD dwDisplacement;

		ReadProcessMemory(hProcess, (LPCVOID)called, memory, 4, &written);
		called = *(DWORD *)memory;
#endif

		WCHAR *wLibName = L"None";

		for (auto iter = libs.begin(); iter != libs.end(); ++iter) {
			if ((*iter)->dwFileSize == 0) {
				HMODULE hModule = GetRemoteModuleHandle(dbgEvent.dwProcessId, (*iter)->wFileName);
				if (hModule) {
					MODULEINFO modinfo;
					GetModuleInformation(hProcess, hModule, &modinfo, sizeof(modinfo));

					(*iter)->dwFileSize = modinfo.SizeOfImage;
				}
			}

			DWORD64 lpBaseOfDll = (DWORD64)((*iter)->lpBaseOfDll);
			DWORD64 lpEndOfDll = lpBaseOfDll + (*iter)->dwFileSize;

			if (called >= lpBaseOfDll && called <= lpEndOfDll) {
				wLibName = (*iter)->wFileName;
				break;
			}
		}

		IMAGEHLP_SYMBOL *pSymbol;
		pSymbol = (IMAGEHLP_SYMBOL *)new BYTE[sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME];

		memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME);
		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
		pSymbol->MaxNameLength = MAX_SYM_NAME;

		if (SymGetSymFromAddr(hSymbol, called, &dwDisplacement, pSymbol)) {
			printf("%p -> %ls.%s\n", record.ExceptionAddress, wLibName, pSymbol->Name);
		}
		else {
			printf("%p -> %ls.%p\n", record.ExceptionAddress, wLibName, called);
		}
	}
	else {
		called = (DWORD64)record.ExceptionAddress + (*(DWORD *)memory) + 5;
		printf("%p -> %p\n", record.ExceptionAddress, called);
	}

	CloseHandle(hProcess);
	return 0;
}