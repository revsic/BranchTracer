#pragma once
#include "stdafx.h"

#include <TlHelp32.h>
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

DWORD GetParentProcessId(DWORD dwProcessId);

ProcessInfo* FindProcess(DWORD dwProcessId, ProcessInfo& info);
int AppendSubProcess(DWORD dwParentProcessId, DWORD dwProcessId, ProcessInfo& info);
int DeleteProcess(DWORD dwProcessId, ProcessInfo& info);

int ContinueProcess(DEBUG_EVENT& dbgEvent);
int LogBranch(DEBUG_EVENT& dbgEvent);