# BranchTracer

Implementation of Branch Tracer with C++

Branch data refers to data processed in a branching situation such as jmp and call. This data is advantageous for showing its structure regardless of the polymorphism of the binary, and the branch tracer logs such branch data.

- Brancher DLL Main - Pre process :  [dllmain.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/dllmain.cpp)
- Branch Logger - VEH Handler: [Brancher.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/Brancher.cpp)
- Process Utils - Manage Break Points : [ProcessUtils.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/ProcessUtils.cpp)
- Raw level Helper - Asm Parser : [RawlevelHelper.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/RawlevelHelper.cpp)
- Helper Main - DLL Injector : [Main.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Helper/main.cpp)

## Brancher

Brancher is a VEH-based dll-type branch tracer.

VEH is a Vectored Exception Handler, a handler that can handle exceptions that occur throughout the binary. The DLL generates an EXCEPTION_BREAKPOINT, and VEH is called. The handler generates an EXCEPTION_SINGLE_STEP, checks each instruction, and logs if it is a branch.

Despite the single step exception, the tracer is fast enough, because the dll shares memory with the target process, so it is less expensive to access memory, and VEH exception handler is used to speed up single step exception processing.

```cpp
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
    case DLL_PROCESS_ATTACH:
        AddVectoredExceptionHandler(1, BranchHandler);
        SetBreakPointOnEntryPoint();
        break;
    }
}
```

Brancher sets break point at the entry point and registers the VEH handler with first priority.

```cpp
long WINAPI BranchHandler(PEXCEPTION_POINTERS ExceptionInfo) {
	PEXCEPTION_RECORD record = ExceptionInfo->ExceptionRecord;
	PCONTEXT context = ExceptionInfo->ContextRecord;

	if (record->ExceptionCode == EXCEPTION_BREAKPOINT) {
		BackupBreakPoint(record->ExceptionAddress);
	}

	if (record->ExceptionCode == EXCEPTION_BREAKPOINT
		|| record->ExceptionCode == EXCEPTION_SINGLE_STEP)
	{
        ...
    }
}
```

Branch Handler The branch handler handles a single step exception and logs when the instruction is branching.

```cpp
if (opc[0] == 0xFF) {
    BYTE Reg = (opc[1] >> 0x3) & 0x7;
    if (Reg >= 2 && Reg <= 5) {
        // Binary 010(near call) or 011(far call)
        //        100(near jmp)  or 101(far jmp)

        LPVOID next;
        CDWORD called = GetBranchingAddress(opc, context, &next);

        StringCbPrintfW(log,
            MAX_LOG_SIZE,
            L"+%p,%p,%s,%s\r\n",
            record->ExceptionAddress,
            called,
            wModuleName,
            wSymbolName);

        WriteFile(hStdOutput, log, (DWORD)wcslen(log) * sizeof(WCHAR), &written, NULL);
    }
}
```

## Helper

Helper injects the brancher dll into the target process.

By waiting for the created remote thread, it helps the normal operation of the process after the veh registration and break point creation.

```cpp
HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpFunction, lpParam, NULL, NULL);
WaitForSingleObject(hThread, INFINITE);

CloseHandle(hThread);
ResumeThread(pi.hThread);
```
