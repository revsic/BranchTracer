#pragma once
#include "stdafx.h"
#include "WindowsHelp.h"

#include <Psapi.h>
#include <DbgHelp.h>
#include <vector>

class ProcessInfo {
public:
	DWORD dwProcessId;
	bool isInitBreakpoint;
	std::vector<ProcessInfo *> SubProcess;

	ProcessInfo()
		: dwProcessId(0)
		, isInitBreakpoint(true)
	{}

	~ProcessInfo() {
		for (auto iter = SubProcess.begin(); iter != SubProcess.end(); ++iter) {
			delete *iter;
		}
	}
};

class LibraryInfo {
public:
	WCHAR wFileName[MAX_FILE_PATH];
	DWORD64 dwFileSize;
	LPVOID lpBaseOfDll;

	LibraryInfo()
		: dwFileSize(0)
		, lpBaseOfDll(NULL)
	{}
};

int AppendSubProcess(DWORD dwParentProcessId, DWORD dwProcessId, ProcessInfo& info);
int DeleteProcess(DWORD dwProcessId, ProcessInfo& info);
ProcessInfo* FindProcess(DWORD dwProcessId, ProcessInfo& info);

int ContinueProcess(DEBUG_EVENT& dbgEvent);
int LogBranch(HANDLE hSymbol, DEBUG_EVENT& dbgEvent, std::vector<LibraryInfo *> libs);