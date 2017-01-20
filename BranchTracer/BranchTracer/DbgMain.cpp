#include "DbgMain.h"

int DebugProcess(WCHAR* filename) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	CreateProcessW(filename, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | DEBUG_PROCESS, NULL, NULL, &si, &pi);

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
			printf("[*] Process Created : %d\n\n", dbgEvent.dwProcessId);

			if (proc.dwProcessId != dbgEvent.dwProcessId) {
				DWORD dwParentProcessId = GetParentProcessId(dbgEvent.dwProcessId);
				AppendSubProcess(dwParentProcessId, dbgEvent.dwProcessId, proc);
			}
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			printf("[*] Process Exited : %d\n\n", dbgEvent.dwProcessId);

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

			DWORD dwFileSizeHi = 0;
			DWORD dwFileSizeLo = GetFileSize(info.hFile, &dwFileSizeHi);
			lib->dwFileSize = (dwFileSizeHi << 32) + dwFileSizeLo;

			printf("[*] Load Library : %ls\n", lib->wFileName);
			printf("[*] library addr : %p\n\n", lib->lpBaseOfDll);

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
				printf("[*] Unload Library : %ls\n", (*lib)->wFileName);
				printf("[*] library addr : %p\n\n", (*lib)->lpBaseOfDll);
			}

			break;
		}

		case EXCEPTION_DEBUG_EVENT:
		{
			EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;

			if (record.ExceptionCode == EXCEPTION_BREAKPOINT) {
				printf("[*] Exception occured : pid - %d , tid - %d\n", dbgEvent.dwProcessId, dbgEvent.dwThreadId);

				ProcessInfo* info = FindProcess(dbgEvent.dwProcessId, proc);
				if (info->isInitBreakpoint) {
					printf("[*] initial bp : %p\n\n", record.ExceptionAddress);

					info->isInitBreakpoint = false;
				}
				else {
					printf("[*] branch occured : %p\n", record.ExceptionAddress);

					LogBranch(dbgEvent, libs);
					ContinueProcess(dbgEvent);
				}
			}
			else {
				printf("[*] Unexpected Exception Occured..\n");
				return 1;
			}
			break;
		}
		default:
			break;
		}

		DWORD status = DBG_CONTINUE;
		ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, status);
	}

	return 0;
}