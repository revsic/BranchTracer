#include "stdafx.h"
#include "ProcessUtils.h"

BYTE backup = NULL;

int SetSingleStepContext(PCONTEXT context) {
	context->EFlags |= (1 << 8);
	return 0;
}

int SetBreakPoint(LPVOID lpAddress) {
	DWORD flOldProtect;
	VirtualProtect(lpAddress, 1, PAGE_EXECUTE_READWRITE, &flOldProtect);

	backup = *(BYTE *)lpAddress;
	*(BYTE *)lpAddress = 0xCC;

	return 0;
}

int BackupBreakPoint(LPVOID lpAddress) {
	*(BYTE *)lpAddress = backup;
	return 0;
}

int SetBreakPointOnEntryPoint() {
	MODULEENTRY32W entry;
	entry.dwSize = sizeof(entry);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	Module32FirstW(hSnapshot, &entry);
	CloseHandle(hSnapshot);

	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)entry.modBaseAddr;
	PIMAGE_NT_HEADERS pNtHdr = ImageNtHeader(pDosHdr);

	PIMAGE_OPTIONAL_HEADER pOptHdr = &pNtHdr->OptionalHeader;
	LPVOID lpEntryPoint = entry.modBaseAddr + pOptHdr->AddressOfEntryPoint;

	SetBreakPoint(lpEntryPoint);

	return 0;
}