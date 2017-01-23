#pragma once
#include "stdafx.h"

#include <TlHelp32.h>
#include <Psapi.h>

#define MAX_FILE_PATH 512

class SectionInfo {
public:
	DWORD SizeOfRawData;
	LPVOID VirtualAddress;

	SectionInfo(DWORD SizeOfRawData, LPVOID VirtualAddress)
		: SizeOfRawData(SizeOfRawData)
		, VirtualAddress(VirtualAddress)
	{}
};

DWORD GetParentProcessId(DWORD dwProcessId);
int GetFileNameByHandle(HANDLE hFile, WCHAR *filename);
HMODULE GetRemoteModuleHandle(DWORD dwProcessId, WCHAR *lpModuleName);
SectionInfo* GetTextSectionInfo(DWORD dwProcessId);