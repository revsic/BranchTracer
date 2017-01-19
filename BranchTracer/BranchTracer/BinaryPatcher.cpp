#include "BinaryPatcher.h"

int BinaryPatcher(char *filename) {
	BaseFileInfo *bf = GetOffsetOfInstructions(filename);

	DisasmInfo *disasm = bf->disasm;
	TextSectionInfo *info = bf->process;

	FILE *fp = fopen(filename, "rb");

	fseek(fp, 0, SEEK_END);
	DWORD dwSizeOfFile = ftell(fp);

	BYTE *exe = new BYTE[dwSizeOfFile];
	fseek(fp, 0, SEEK_SET);
	fread(exe, 1, dwSizeOfFile, fp);
	fclose(fp);

	DWORD entry = info->PointerToRawData;
	for (int i = 0; i < disasm->SizeOfOffsets; ++i) {
		DWORD opcode = entry + disasm->Offsets[i];

		if (exe[opcode] == 0xE8) {
			exe[opcode] = 0xCC;
		}
	}

	fp = fopen(filename, "wb");
	fwrite(exe, 1, dwSizeOfFile, fp);
	fclose(fp);

	return 0;
}