// Brancher.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "Brancher.h"
#include "ProcessUtils.h"
#include "RawlevelHelper.h"

bool isAddrUnset = true;
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
					WCHAR wModuleName[MAX_FILE_PATH];
					if (!GetModuleNameByAddr(called, wModuleName)) {
						WCHAR wSymbolName[MAX_SYM_NAME];
						if (!GetSymolName(called, wSymbolName)) {
							printf("%p -> %ls :: %ls\n", record->ExceptionAddress, wModuleName, wSymbolName);
						}
						else {
							printf("%p -> %ls :: %p\n", record->ExceptionAddress, wModuleName, called);
						}
					}
					else {
						printf("%p -> %p\n", record->ExceptionAddress, wModuleName, called);
					}

					SetBreakPoint(next);
					isBreakPointSet = true;
				}
			}
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
