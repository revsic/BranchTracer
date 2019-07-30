#include "stdafx.h"

// Result of SIB parser
struct SIBParseResult {
	BYTE *opc;
	CDWORD called;
};

// Get destination address of branching operation.
CDWORD GetBranchingAddress(BYTE *opc, PCONTEXT context, LPVOID *next);
// Parse SIB from given context.
CDWORD SIBParser(BYTE* opc, PCONTEXT context, SIBParseResult *result);
