#include "LivePatcher.h"

int LivePatcher(DWORD dwProcessId) {
	SectionInfo *info = GetTextSectionInfo(dwProcessId);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	SIZE_T written;
	BYTE *opcodes = new BYTE[info->SizeOfRawData];
	ReadProcessMemory(hProcess, info->VirtualAddress, opcodes, info->SizeOfRawData, &written);

#ifdef _WIN64
	_DecodeType dt = Decode64Bits;
#else
	_DecodeType dt = Decode32Bits;
#endif

	_OffsetType dOffset = 0;
	_DecodedInst dInsts[MAX_INSTRUCTIONS];

	UINT uInstCount = 0;
	distorm_decode(dOffset, opcodes, info->SizeOfRawData, dt, dInsts, MAX_INSTRUCTIONS, &uInstCount);

	DWORD *offsets = new DWORD[uInstCount];
	std::transform(dInsts, dInsts + uInstCount, offsets, [](_DecodedInst& val)->DWORD { return val.offset; });

	for (int i = 0; i < uInstCount; ++i) {
		DWORD offset = offsets[i];

		if (opcodes[offset] == 0xE8 && opcodes[offset + 1] != 0x15) {
			opcodes[offset] = 0xCC;
		}
		else if (opcodes[offset] == 0xFF && opcodes[offset + 1] == 0x15) {
			opcodes[offset] = 0xCC;
		}
	}

	DWORD dwOldProtect, tmp;
	VirtualProtectEx(hProcess, info->VirtualAddress, info->SizeOfRawData, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	WriteProcessMemory(hProcess, info->VirtualAddress, opcodes, info->SizeOfRawData, &written);
	VirtualProtectEx(hProcess, info->VirtualAddress, info->SizeOfRawData, dwOldProtect, &tmp);
	
	CloseHandle(hProcess);
	delete[] opcodes;
	delete[] offsets;

	return 0;
}