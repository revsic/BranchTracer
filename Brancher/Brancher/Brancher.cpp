#include "stdafx.h"
#include "Brancher.h"
#include "ProcessUtils.h"
#include "RawlevelHelper.h"

bool isAddrUnset = true;
HANDLE hStdOutput = NULL;
CDWORD StartOfTextSection = NULL;
CDWORD EndOfTextSection = NULL;

// Branch tracer.
long WINAPI BranchHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    PEXCEPTION_RECORD record = ExceptionInfo->ExceptionRecord;
    PCONTEXT context = ExceptionInfo->ContextRecord;

    // if break point trigged
    if (record->ExceptionCode == EXCEPTION_BREAKPOINT) {
        BackupBreakPoint(record->ExceptionAddress);
    }

    // if debugging event trigged
    if (record->ExceptionCode == EXCEPTION_BREAKPOINT
        || record->ExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        // if log file uninitialized
        if (isAddrUnset) {
            isAddrUnset = false;

            hStdOutput = CreateFileW(L"C:\\dbg\\log.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            GetTextSectionAddress(&StartOfTextSection, &EndOfTextSection);
        }

        bool isBreakPointSet = false;
        BYTE *opc = (BYTE *)record->ExceptionAddress;

        // if 0xFF branch operation
        if (opc[0] == 0xFF) {
            BYTE Reg = (opc[1] >> 0x3) & 0x7;
            if (Reg >= 2 && Reg <= 5) { 
                // Binary 010(near call) or 011(far call) 
                //        100(near jmp)  or 101(far jmp)

                // get destination address
                LPVOID next;
                CDWORD called = GetBranchingAddress(opc, context, &next);

                // if address out of text section
                if (called >= EndOfTextSection || called <= StartOfTextSection) {
                    WCHAR log[MAX_LOG_SIZE];
                    WCHAR wModuleName[MAX_FILE_PATH];
                    // get module name where destination address exist
                    if (!GetModuleNameByAddr(called, wModuleName, MAX_FILE_PATH)) {
                        WCHAR wSymbolName[MAX_SYM_NAME];
                        // get symbol name of destination address
                        if (!GetSymbolName(called, wSymbolName, MAX_SYM_NAME)) {
                            // write log
                            StringCbPrintfW(log, 
                                            MAX_LOG_SIZE, 
                                            L"+%p,%p,%s,%s\r\n", 
                                            record->ExceptionAddress, 
                                            called, 
                                            wModuleName, 
                                            wSymbolName);
                        }
                        else {
                            // if symbol doesn't exist
                            StringCbPrintfW(log,
                                            MAX_LOG_SIZE,
                                            L"+%p,%p,%s\r\n",
                                            record->ExceptionAddress,
                                            called,
                                            wModuleName);
                        }
                    }
                    else {
                        // if module name doesn't exist
                        StringCbPrintfW(log, MAX_LOG_SIZE, L"+%p,%p,,\r\n", record->ExceptionAddress, called);
                    }

                    // write log message
                    DWORD written;
                    WriteFile(hStdOutput, log, (DWORD)wcslen(log) * sizeof(WCHAR), &written, NULL);

                    // set bp on return address
                    SetBreakPoint(next);
                    isBreakPointSet = true;
                }
            }
        }
        // if 0xE8 branching operation
        else if (opc[0] == 0xE8) {
            // get destination address
            CDWORD called = context->RegisterIp + *(long *)&opc[1] + 5;

            DWORD written;
            WCHAR log[MAX_LOG_SIZE];
            // write log message
            StringCbPrintfW(log, MAX_LOG_SIZE, L"+%p,%p,,\r\n", record->ExceptionAddress, called);
            WriteFile(hStdOutput, log, (DWORD)wcslen(log) * sizeof(WCHAR), &written, NULL);
        }

        // set trap flag for sigle step exception
        if (!isBreakPointSet) {
            SetSingleStepContext(context);
        }

        // continue process without passing exception
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else {
        // pass exception to process
        return EXCEPTION_CONTINUE_SEARCH;
    }
}
