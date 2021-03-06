# BranchTracer

Implementation of Branch Tracer with C++

Branch data refers to the metadata from the branching operation such as jmp and call. This data is advantageous for showing the structure of binary regardless of the machine code polymorphism. Branch tracer writes log about the branch data.

- Brancher DLL Main - Preprocessor :  [dllmain.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/dllmain.cpp)
- Branch Logger - VEH Handler: [Brancher.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/Brancher.cpp)
- Process Utils - Manage Break Points : [ProcessUtils.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/ProcessUtils.cpp)
- Raw level Helper - Asm Parser : [RawlevelHelper.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Brancher/RawlevelHelper.cpp)
- Helper Main - DLL Injector : [Main.cpp](https://github.com/revsic/BranchTracer/blob/master/Brancher/Helper/main.cpp)

## Brancher

Brancher is a VEH-based dll-type windows debugger.

VEH is a Vectored Exception Handler that can handle exceptions occured throughout the process. When dll is injected, dllmain sets a software breakpoint at the entry point on the target process and add *Brancher* as the first priority vectored exception handler. When instruction pointer executes the entry point, EXCEPTION_BREAKPOINT occurs and system calls *Brancher*. It parses opcode and writes log if it is a branching instruction. The handler sets trap flag before continuing process then EXCEPTION_SINGLE_STEP occur at the next instruction. And *Brancher* can be called recursively, step the instruction and write logs when branching instruction is found. Sometimes *Brancher* writes software breakpoint on return address of API call and run without trap flag, in order to reduce unnecessary logging.

## Helper

Helper injects the Brancher dll into the target process.

By waiting for the created remote thread, it helps the normal operation of the process after the veh registration and break point creation.

```cpp
HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpFunction, lpParam, NULL, NULL);
WaitForSingleObject(hThread, INFINITE);

CloseHandle(hThread);
ResumeThread(pi.hThread);
```

## Log Example

Branch data generated by notepad

```
+00007FF730C9B842,00007FFF4E004CC0,msvcrt.dll,memset
+00007FF730C84279,00007FFF4D83E420,KERNEL32.DLL,GetCommandLineW
+00007FF730C84289,00007FF730C81130,,
+00007FF730C8117C,00007FFF4F6CCDE0,ntdll.dll,EtwEventRegister
+00007FF730C811A9,00007FFF4F6F3720,ntdll.dll,EtwEventSetInformation
+00007FF730C811B9,00007FF730C9AEC0,,
+00007FF730C84298,00007FFF4DC0DE40,combase.dll,CoCreateGuid
+00007FF730C84331,00007FFF4D83EEB0,KERNEL32.DLL,HeapSetInformation
+00007FF730C8433E,00007FFF4DBBF1A0,combase.dll,CoInitializeEx
+00007FF730C84354,00007FF730C9A8B0,,
+00007FF730C9A8DE,00007FFF4DC5C850,combase.dll,RoInitialize
+00007FF730C9A903,00007FFF4DBC9260,combase.dll,WindowsCreateStringReference
+00007FF730C9A94E,00007FFF4DBC3FC0,combase.dll,RoGetActivationFactory
+00007FF730C9A971,00007FFF4F70FC10,ntdll.dll,RtlRetrieveNtUserPfn
+00007FF730C9AAD7,00007FFF4F70FC10,ntdll.dll,RtlRetrieveNtUserPfn
+00007FF730C9AAE6,00007FF730C9AEC0,,
```
