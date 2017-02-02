// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>
#include <strsafe.h>

#define MAX_FILE_PATH 512
#define MAX_LOG_SIZE 256

#ifdef _WIN64
typedef DWORD64 CDWORD;

#define RegisterAx Rax
#define RegisterCx Rcx
#define RegisterDx Rdx
#define RegisterBx Rbx
#define RegisterSp Rsp
#define RegisterBp Rbp
#define RegisterSi Rsi
#define RegisterDi Rdi
#define RegisterIp Rip

#else
typedef DWORD CDWORD;

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

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
