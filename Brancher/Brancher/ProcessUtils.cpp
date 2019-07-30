#include "stdafx.h"
#include "ProcessUtils.h"

bool isValueUnset = true;

DWORD dwProcessId = NULL;
HANDLE hProcess = NULL;

BYTE backup = NULL;

// Set trap flag to make EXCEPTION_SINGLE_STEP.
int SetSingleStepContext(PCONTEXT context) {
    context->EFlags |= (1 << 8);
    return 0;
}

// Set software breakpoint with 0xCC.
int SetBreakPoint(LPVOID lpAddress) {
    DWORD flOldProtect;
    VirtualProtect(lpAddress, 1, PAGE_EXECUTE_READWRITE, &flOldProtect);

	// backup byte
    backup = *(BYTE *)lpAddress;
	// write 0xCC, int 3
    *(BYTE *)lpAddress = 0xCC;

    return 0;
}

// Recover current byte which replaced with 0xCC.
int BackupBreakPoint(LPVOID lpAddress) {
    *(BYTE *)lpAddress = backup;
    return 0;
}

// Set software breakpoint on entry point.
int SetBreakPointOnEntryPoint() {
    MODULEENTRY32W entry;
    entry.dwSize = sizeof(entry);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	// get first module
    Module32FirstW(hSnapshot, &entry);
    CloseHandle(hSnapshot);

	// parse entrypoint
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)entry.modBaseAddr;
    PIMAGE_NT_HEADERS pNtHdr = ImageNtHeader(pDosHdr);

    PIMAGE_OPTIONAL_HEADER pOptHdr = &pNtHdr->OptionalHeader;
    LPVOID lpEntryPoint = entry.modBaseAddr + pOptHdr->AddressOfEntryPoint;

	// set breakpoint on entrypoint
    SetBreakPoint(lpEntryPoint);

    return 0;
}

// Get start and end address of text section.
int GetTextSectionAddress(CDWORD *StartOfTextSection, CDWORD *EndOfTextSection) {
    MODULEENTRY32W entry;
    entry.dwSize = sizeof(entry);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	// get first module
    Module32FirstW(hSnapshot, &entry);
    CloseHandle(hSnapshot);

	// parse entrypoint
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)entry.modBaseAddr;
    PIMAGE_NT_HEADERS pNtHdr = ImageNtHeader(pDosHdr);
    PIMAGE_SECTION_HEADER pSectionHdr = (PIMAGE_SECTION_HEADER)(pNtHdr + 1);

    CDWORD EntryPoint = pNtHdr->OptionalHeader.AddressOfEntryPoint;

	// find section which include the entry point
    DWORD NumberOfSections = pNtHdr->FileHeader.NumberOfSections;
    for (DWORD i = 0; i < NumberOfSections; ++i) {
        CDWORD start = pSectionHdr->VirtualAddress;
        CDWORD end = start + pSectionHdr->SizeOfRawData;

        if (EntryPoint >= start && EntryPoint <= end) {
            *StartOfTextSection = (CDWORD)entry.modBaseAddr + start;
            *EndOfTextSection = (CDWORD)entry.modBaseAddr + end;

            break;
        }

        ++pSectionHdr;
    }

    return 0;
}

// Get module name by address.
int GetModuleNameByAddr(CDWORD dwAddress, WCHAR *wName, SIZE_T size) {
	// if symbol uninitialized
    if (isValueUnset) {
        isValueUnset = false;

        dwProcessId = GetCurrentProcessId();
        hProcess = GetCurrentProcess();
        SymInitialize(hProcess, NULL, TRUE);
    }

    wName[0] = '\0';
    int ret = 1;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
    MODULEENTRY32W entry;
    entry.dwSize = sizeof(entry);

	// iteration module
    if (Module32FirstW(hSnapshot, &entry)) {
        do {
            MODULEINFO modinfo;
            GetModuleInformation(hProcess, entry.hModule, &modinfo, sizeof(modinfo));

            CDWORD start = (CDWORD)modinfo.lpBaseOfDll;
            CDWORD end = start + modinfo.SizeOfImage;

			// find module which include the given address
            if (dwAddress >= start && dwAddress <= end) {
				// copy module name
                StringCbCopyW(wName, size, entry.szModule);
                
                char name[MAX_FILE_PATH];
                StringCbPrintfA(name, MAX_FILE_PATH, "%ws", wName);

				// load module symbol
                if (SymLoadModule(hProcess, NULL, name, 0, (CDWORD)modinfo.lpBaseOfDll, 0) || !GetLastError()) {
                    ret = 0;
                }
                break;
            }
            
        } while (Module32NextW(hSnapshot, &entry));
        CloseHandle(hSnapshot);
    }

    return ret;
}


// Get symbol name by address.
int GetSymbolName(CDWORD called, WCHAR *name, SIZE_T size) {
    name[0] = '\0';
    int ret = 1;

    IMAGEHLP_SYMBOL *pSymbol = (IMAGEHLP_SYMBOL *)new BYTE[sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME];

    memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL) + MAX_SYM_NAME);
    pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    pSymbol->MaxNameLength = MAX_SYM_NAME;

	// get symbol from address
    CDWORD dwDisplacement;
    if (SymGetSymFromAddr(hProcess, called, &dwDisplacement, pSymbol)) {
        StringCbPrintfW(name, size, L"%S", pSymbol->Name);
        ret = 0;
    }

    return ret;
}