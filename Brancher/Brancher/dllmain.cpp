#include "stdafx.h"
#include "Brancher.h"
#include "ProcessUtils.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // add veh to run branch handler
        AddVectoredExceptionHandler(1, BranchHandler);
        // set bp on entry point
        SetBreakPointOnEntryPoint();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

