#include "stdafx.h"

struct SIBParseResult {
	BYTE *opc;
	CDWORD called;
};

CDWORD GetBranchingAddress(BYTE *opc, PCONTEXT context, LPVOID *next);
CDWORD SIBParser(BYTE* opc, PCONTEXT context, SIBParseResult *result);