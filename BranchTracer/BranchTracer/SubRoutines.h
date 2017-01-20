#pragma once
#include "stdafx.h"

#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>

#define MAX_FILE_PATH 512

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

DWORD GetParentProcessId(DWORD dwProcessId);
int GetFileNameByHandle(HANDLE hFile, WCHAR *filename);

ProcessInfo* FindProcess(DWORD dwProcessId, ProcessInfo& info);
int AppendSubProcess(DWORD dwParentProcessId, DWORD dwProcessId, ProcessInfo& info);
int DeleteProcess(DWORD dwProcessId, ProcessInfo& info);

int ContinueProcess(DEBUG_EVENT& dbgEvent);
int LogBranch(DEBUG_EVENT& dbgEvent, std::vector<LibraryInfo *> libs);