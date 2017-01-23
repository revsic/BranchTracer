#include "WindowsHelp.h"

int wcscmpi(WCHAR *a, WCHAR *b) {
	while (*a != '\0' && *b != '\0') {
		if (*a != *b && *a != (*b ^ 0x20)) {
			return 1;
		}

		++a; ++b;
	}

	if (*a == '\0' && *b == '\0') {
		return 0;
	}

	return 1;
}

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

HMODULE GetRemoteModuleHandle(DWORD dwProcessId, WCHAR *lpModuleName) {
	HMODULE hModule = NULL;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

	MODULEENTRY32W entry;
	entry.dwSize = sizeof(entry);

	if (Module32FirstW(hSnapshot, &entry)) {
		do {
			if (!wcscmpi(entry.szModule, lpModuleName)) {
				hModule = entry.hModule;
				break;
			}
		} while (Module32NextW(hSnapshot, &entry));
	}

	return hModule;
}

SectionInfo* GetTextSectionInfo(DWORD dwProcessId) {
	MODULEENTRY32W entry;
	entry.dwSize = sizeof(entry);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

	Module32FirstW(hSnapshot, &entry);
	CloseHandle(hSnapshot);

	LPVOID lpModBaseAddr = entry.modBaseAddr;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	SIZE_T written;
	PIMAGE_DOS_HEADER pDosHdr = new IMAGE_DOS_HEADER();
	ReadProcessMemory(hProcess, lpModBaseAddr, pDosHdr, sizeof(IMAGE_DOS_HEADER), &written);

	PIMAGE_NT_HEADERS pNtHdr = new IMAGE_NT_HEADERS();
	DWORD64 pNtHdrOffset = (DWORD64)lpModBaseAddr + pDosHdr->e_lfanew;
	ReadProcessMemory(hProcess, (LPCVOID)pNtHdrOffset, pNtHdr, sizeof(IMAGE_NT_HEADERS), &written);

	LPVOID lpVirtualAddress = NULL;
	DWORD dwSizeOfRawData = 0;

	PIMAGE_SECTION_HEADER pSectionHdr = new IMAGE_SECTION_HEADER();
	DWORD64 dwSectionHdrOffset = pNtHdrOffset + sizeof(IMAGE_NT_HEADERS);
	for (int i = 0; i < pNtHdr->FileHeader.NumberOfSections; ++i) {
		ReadProcessMemory(hProcess, (LPCVOID)dwSectionHdrOffset, pSectionHdr, sizeof(IMAGE_SECTION_HEADER), &written);

		char *name = (char *)pSectionHdr->Name;
		if (!strcmp(name, ".text")) {
			lpVirtualAddress = (LPVOID)((DWORD64)lpModBaseAddr + pSectionHdr->VirtualAddress);
			dwSizeOfRawData = pSectionHdr->SizeOfRawData;
		}

		dwSectionHdrOffset += sizeof(IMAGE_SECTION_HEADER);
	}

	CloseHandle(hProcess);
	delete pDosHdr;
	delete pNtHdr;
	delete pSectionHdr;

	return (new SectionInfo(dwSizeOfRawData, lpVirtualAddress));
}