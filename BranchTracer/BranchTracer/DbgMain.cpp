#include "DbgMain.h"

int DebugProcess(WCHAR* filename) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	CreateProcessW(filename, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | DEBUG_PROCESS, NULL, NULL, &si, &pi);

	if (!SymInitialize(pi.hProcess, NULL, FALSE)) {
		printf("[*] Symbol init fail..\n");
		return 1;
	}

	DEBUG_EVENT dbgEvent;
	bool dbgContinue = true;

	ProcessInfo proc;
	proc.dwProcessId = pi.dwProcessId;

	std::vector<LibraryInfo *> libs;

	while (dbgContinue) {
		if (FALSE == WaitForDebugEvent(&dbgEvent, 100)) {
			continue;
		}

		switch (dbgEvent.dwDebugEventCode) {
		case CREATE_PROCESS_DEBUG_EVENT:
			if (proc.dwProcessId != dbgEvent.dwProcessId) {
				DWORD dwParentProcessId = GetParentProcessId(dbgEvent.dwProcessId);
				AppendSubProcess(dwParentProcessId, dbgEvent.dwProcessId, proc);
			}
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			if (proc.dwProcessId == dbgEvent.dwProcessId) {
				dbgContinue = false;
			}
			else {
				DeleteProcess(dbgEvent.dwProcessId, proc);
			}
			break;

		case LOAD_DLL_DEBUG_EVENT:
		{
			LOAD_DLL_DEBUG_INFO& info = dbgEvent.u.LoadDll;
			LibraryInfo *lib = new LibraryInfo();
			
			lib->lpBaseOfDll = info.lpBaseOfDll;
			GetFileNameByHandle(info.hFile, lib->wFileName);

			char name[MAX_FILE_PATH];
			sprintf(name, "%ls", lib->wFileName);

			if (!SymLoadModule(pi.hProcess, NULL, name, 0, (DWORD64)info.lpBaseOfDll, 0)) {
				if (GetLastError()) {
					printf("[*] Symbol load err : %s\n", name);
				}
			}

			libs.push_back(lib);
			break;
		}

		case UNLOAD_DLL_DEBUG_EVENT:
		{
			UNLOAD_DLL_DEBUG_INFO& info = dbgEvent.u.UnloadDll;

			auto lib = std::find_if(
				libs.begin(), 
				libs.end(), 
				[info](LibraryInfo *lib)->bool { return lib->lpBaseOfDll == info.lpBaseOfDll; }
			);

			if (lib != libs.end()) {
				libs.erase(lib);
			}

			break;
		}

		case EXCEPTION_DEBUG_EVENT:
		{
			EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;

			if (record.ExceptionCode == EXCEPTION_BREAKPOINT) {
				ProcessInfo* info = FindProcess(dbgEvent.dwProcessId, proc);
				if (info->isInitBreakpoint) {
					LivePatcher(dbgEvent.dwProcessId);

					info->isInitBreakpoint = false;
				}
				else {
					LogBranch(pi.hProcess, dbgEvent, libs);
					ContinueProcess(dbgEvent);
				}
			}
			else {
				printf("[*] Unexpected Exception Occured..\n");

				DWORD64 called = (DWORD64)record.ExceptionAddress;
					
				WCHAR *wLibName = filename;
				for (auto iter = libs.begin(); iter != libs.end(); ++iter) {
					DWORD64 lpBaseOfDll = (DWORD64)((*iter)->lpBaseOfDll);
					DWORD64 lpEndOfDll = lpBaseOfDll + (*iter)->dwFileSize;

					if (called >= lpBaseOfDll && called <= lpEndOfDll) {
						wLibName = (*iter)->wFileName;
						break;
					}
				}

				printf("[*] Exception Code : %p : %ls.%p\n", record.ExceptionCode, wLibName, called);

				dbgContinue = false;
			}
			break;
		}
		default:
			break;
		}

		DWORD status = DBG_CONTINUE;
		ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, status);
	}

	for (auto iter = libs.begin(); iter != libs.end(); ++iter) {
		delete *iter;
	}

	SymCleanup(pi.hProcess);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}