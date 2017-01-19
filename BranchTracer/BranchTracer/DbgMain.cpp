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

		case EXCEPTION_DEBUG_EVENT:
		{
			EXCEPTION_RECORD& record = dbgEvent.u.Exception.ExceptionRecord;

			if (record.ExceptionCode == EXCEPTION_BREAKPOINT) {
				printf("[*] Exception occured : pid - %d , tid - %d\n", dbgEvent.dwProcessId, dbgEvent.dwThreadId);

				ProcessInfo* info = FindProcess(dbgEvent.dwProcessId, proc);
				if (info->isInitBreakpoint) {
					printf("[*] init .. : %p\n\n", record.ExceptionAddress);

					info->isInitBreakpoint = false;
				}
				else {
					printf("[*] branch occured : %p\n\n", record.ExceptionAddress);

					LogBranch(dbgEvent);
					ContinueProcess(dbgEvent);
				}
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