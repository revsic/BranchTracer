#include "Utils.h"
#include "DataTypes.h"

DWORD GetFileSize(_In_ FILE *fp) {
	DWORD dwCurSet = ftell(fp);
	DWORD dwFileSize = -1;

	fseek(fp, 0, SEEK_END);
	dwFileSize = ftell(fp);
	fseek(fp, dwCurSet, SEEK_SET);

	return dwFileSize;
}


DWORD GetTextSectionInfo(_In_ BYTE *buffer, _Out_  TextSectionInfo *info) {
	PIMAGE_NT_HEADERS64 pNtHdr = ImageNtHeader(buffer);
	PIMAGE_SECTION_HEADER pSectionHdr = (PIMAGE_SECTION_HEADER)(pNtHdr + 1);

	DWORD dwNumberOfSections = pNtHdr->FileHeader.NumberOfSections;

	for (int i = 0; i < dwNumberOfSections; ++i) {
		char *name = (char *)pSectionHdr->Name;

		if (!strcmp(name, ".text")) {
			info->PointerToRawData = pSectionHdr->PointerToRawData;
			info->SizeOfRawData = pSectionHdr->SizeOfRawData;

			break;
		}
	}

	return 0;
}

BaseFileInfo* GetOffsetOfInstructions(_In_ char *filename) {
	FILE *fp = fopen(filename, "rb");
	DWORD dwFileSize = GetFileSize(fp);

	BYTE *buffer = new BYTE[dwFileSize];
	fread(buffer, 1, dwFileSize, fp);

	TextSectionInfo *info = new TextSectionInfo();
	GetTextSectionInfo(buffer, info);

	BYTE *code = new BYTE[info->SizeOfRawData];
	memcpy(code, &buffer[info->PointerToRawData], info->SizeOfRawData);

	_OffsetType dOffset = 0;

	_DecodeType dt = Decode64Bits;
	_DecodedInst dInsts[MAX_INSTRUCTIONS];

	UINT uInstCount = 0;

	distorm_decode(dOffset, code, info->SizeOfRawData, dt, dInsts, MAX_INSTRUCTIONS, &uInstCount);

	DWORD *offsets = new DWORD[uInstCount];

	std::transform(dInsts, dInsts + uInstCount, offsets, [](_DecodedInst& val)->DWORD { return val.offset; });

	DisasmInfo *disasm = new DisasmInfo(uInstCount, offsets);

	delete[] buffer;
	delete[] code;

	fclose(fp);

	return ( new BaseFileInfo(info, disasm) );
}