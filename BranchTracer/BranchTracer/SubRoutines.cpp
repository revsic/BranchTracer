#include "SubRoutines.h"

DWORD GetParentProcessId(DWORD dwProcessId) {
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);

	DWORD dwParentProcessId = 0;

	if (Process32FirstW(hSnapshot, &entry)) {
		do {
			if (entry.th32ProcessID == dwProcessId) {
				dwParentProcessId = entry.th32ParentProcessID;
				break;
			}
		} while (Process32NextW(hSnapshot, &entry));
	}

	CloseHandle(hSnapshot);
	return dwParentProcessId;
}

int GetFileNameByHandle(HANDLE hFile, WCHAR *filename) {
	WCHAR wFileName[MAX_FILE_PATH];
	HANDLE hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 1, NULL);
	
	void *pMem = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 1);

	GetMappedFileNameW(GetCurrentProcess(), pMem, wFileName, MAX_FILE_PATH);
			
	WCHAR *ptr = wcsrchr(wFileName, '\\');
	if (ptr) {
		wcscpy(filename, ptr + 1);
	}
	else {
		wcscpy(filename, wFileName);
	}

	UnmapViewOfFile(pMem);
	CloseHandle(hMap);
	return 0;
}

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

	BYTE call = 0xE8;
	SIZE_T written = 0;

	EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;
	WriteProcessMemory(hProcess, record.ExceptionAddress, &call, 1, &written);

	CONTEXT context;
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &context);

	context.Rip -= 1;
	SetThreadContext(hThread, &context);

	CloseHandle(hProcess);
	CloseHandle(hThread);

	return 0;
}

int LogBranch(DEBUG_EVENT& dbgEvent, std::vector<LibraryInfo *> libs) {
	EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dbgEvent.dwProcessId);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dbgEvent.dwThreadId);

	CONTEXT context;
	context.ContextFlags = CONTEXT_ALL;
	GetThreadContext(hThread, &context);

	BYTE memory[4];
	SIZE_T written;
	ReadProcessMemory(hProcess, (LPCVOID)context.Rip, memory, 4, &written);

	DWORD64 called = context.Rip + (*(DWORD *)memory) + 4;
	printf("[*] branched to : %p\n", called);

	for (auto iter = libs.begin(); iter != libs.end(); ++iter) {
		DWORD64 lpBaseOfDll = (DWORD64)((*iter)->lpBaseOfDll);
		DWORD64 lpEndOfDll = lpBaseOfDll + (*iter)->dwFileSize;

		if (called >= lpBaseOfDll && called <= lpEndOfDll) {
			printf("[*] address in %ls\n", (*iter)->wFileName);
		}

		break;
	}

	printf("\n");

	CloseHandle(hProcess);
	CloseHandle(hThread);
	return 0;
}