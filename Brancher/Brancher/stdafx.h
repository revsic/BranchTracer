#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <strsafe.h>

#define MAX_FILE_PATH 512
#define MAX_LOG_SIZE 256

// if x64 environment
#ifdef _WIN64
typedef DWORD64 CDWORD;

// declare register with rax-family
#define RegisterAx Rax
#define RegisterCx Rcx
#define RegisterDx Rdx
#define RegisterBx Rbx
#define RegisterSp Rsp
#define RegisterBp Rbp
#define RegisterSi Rsi
#define RegisterDi Rdi
#define RegisterIp Rip

// if x86 environment
#else
typedef DWORD CDWORD;

// declare register with eax-familty
#define RegisterAx Eax
#define RegisterCx Ecx
#define RegisterDx Edx
#define RegisterBx Ebx
#define RegisterSp Esp
#define RegisterBp Ebp
#define RegisterSi Esi
#define RegisterDi Edi
#define RegisterIp Eip

#endif
