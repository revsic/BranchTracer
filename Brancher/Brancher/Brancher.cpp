// Brancher.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "Brancher.h"
#include "ProcessUtils.h"
#include "RawlevelHelper.h"

bool isAddrUnset = true;
HANDLE hStdOutput = NULL;
CDWORD StartOfTextSection = NULL;
CDWORD EndOfTextSection = NULL;

long WINAPI BranchHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	PEXCEPTION_RECORD record = ExceptionInfo->ExceptionRecord;
	PCONTEXT context = ExceptionInfo->ContextRecord;

	if (record->ExceptionCode == EXCEPTION_BREAKPOINT) {
		BackupBreakPoint(record->ExceptionAddress);
	}

	if (record->ExceptionCode == EXCEPTION_BREAKPOINT
		|| record->ExceptionCode == EXCEPTION_SINGLE_STEP)
	{
		if (isAddrUnset) {
			isAddrUnset = false;

			hStdOutput = CreateFileW(L"C:\\dbg\\log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetTextSectionAddress(&StartOfTextSection, &EndOfTextSection);
		}

		bool isBreakPointSet = false;
		BYTE *opc = (BYTE *)record->ExceptionAddress;

		if (opc[0] == 0xFF) {
			BYTE Reg = (opc[1] >> 0x3) & 0x7;
			if (Reg >= 2 && Reg <= 5) { 
				// Binary 010(near call) or 011(far call) 
				//        100(near jmp)  or 101(far jmp)

				LPVOID next;
				CDWORD called = GetBranchingAddress(opc, context, &next);

				if (called >= EndOfTextSection || called <= StartOfTextSection) {
					WCHAR log[MAX_LOG_SIZE];
					WCHAR wModuleName[MAX_FILE_PATH];
					if (!GetModuleNameByAddr(called, wModuleName, MAX_FILE_PATH)) {
						WCHAR wSymbolName[MAX_SYM_NAME];
						if (!GetSymolName(called, wSymbolName, MAX_SYM_NAME)) {
							StringCbPrintfW(log, 
											MAX_LOG_SIZE, 
											L"+%p,%p,%s,%s\r\n", 
											record->ExceptionAddress, 
											called, 
											wModuleName, 
											wSymbolName);
						}
						else {
							StringCbPrintfW(log,
											MAX_LOG_SIZE,
											L"+%p,%p,%s\r\n",
											record->ExceptionAddress,
											called,
											wModuleName);
						}
					}
					else {
						StringCbPrintfW(log, MAX_LOG_SIZE, L"+%p,%p,,\r\n", record->ExceptionAddress, called);
					}

					DWORD written;
					WriteFile(hStdOutput, log, (DWORD)wcslen(log) * sizeof(WCHAR), &written, NULL);

					SetBreakPoint(next);
					isBreakPointSet = true;
				}
			}
		}
		else if (opc[0] == 0xE8) {
			CDWORD called = context->RegisterIp + *(long *)&opc[1] + 5;

			DWORD written;
			WCHAR log[MAX_LOG_SIZE];
			StringCbPrintfW(log, MAX_LOG_SIZE, L"+%p,%p,,\r\n", record->ExceptionAddress, called);
			WriteFile(hStdOutput, log, (DWORD)wcslen(log) * sizeof(WCHAR), &written, NULL);
		}

		if (!isBreakPointSet) {
			SetSingleStepContext(context);
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else {
		return EXCEPTION_CONTINUE_SEARCH;
	}
}
