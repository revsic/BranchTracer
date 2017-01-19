#pragma once
#include "stdafx.h"

#include <vector>

class TextSectionInfo {
public:
	DWORD PointerToRawData;
	DWORD SizeOfRawData;

	TextSectionInfo()
		: PointerToRawData(0)
		, SizeOfRawData(0)
	{}
};

class DisasmInfo {
public:
	DWORD SizeOfOffsets;
	DWORD *Offsets;

	DisasmInfo(DWORD SizeOfOffsets, DWORD *Offsets)
		: SizeOfOffsets(SizeOfOffsets)
		, Offsets(Offsets)
	{}

	~DisasmInfo() {
		delete[] Offsets;
	}
};

class BaseFileInfo {
public:
	TextSectionInfo* process;
	DisasmInfo* disasm;

	BaseFileInfo(TextSectionInfo* process, DisasmInfo* disasm)
		: process(process)
		, disasm(disasm)
	{}

	~BaseFileInfo() {
		delete process;
		delete disasm;
	}
};