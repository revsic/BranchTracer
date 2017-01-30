#include "stdafx.h"
#include "RawlevelHelper.h"

enum RnM { CustomAx, CustomCx, CustomDx, CustomBx, CustomSp, CustomBp, CustomSi, CustomDi };

CDWORD GetBranchingAddress(BYTE *opc, PCONTEXT context, LPVOID *next) {
	BYTE Mod = opc[1] >> 0x6; // high 2bits
	BYTE Reg = (opc[1] >> 0x3) & 0x7; // mid 3bits
	BYTE RnM = opc[1] & 0x7; // low 3bits

	CDWORD called = NULL;

	switch (RnM) {
	case CustomAx: 
		called = context->CustomAx; 
		break;
	case CustomCx:
		called = context->CustomCx;
		break;
	case CustomDx:
		called = context->CustomDx;
		break;
	case CustomBx:
		called = context->CustomBx;
		break;
	case CustomSp:
		if (Mod != 0x3) { // binary 11
			SIBParseResult SibResult;
			SIBParser(opc, context, &SibResult);

			opc = SibResult.opc;
			called = SibResult.called;
		}
		else {
			called = context->CustomSp;
		}
		break;
	case CustomBp:
		if (Mod == 0x00) {
#ifdef _WIN64
			called = context->CustomIp + *(long *)&opc[2] + 6;
#else
			called = *(long *)&opc[2];
#endif
			opc += 4;
		}
		else {
			called = context->CustomBp;
		}
		break;
	case CustomSi:
		called = context->CustomSi;
		break;
	case CustomDi:
		called = context->CustomDi;
		break;
	}

	if (Mod == 0x1) { //binary 01
		called += (char)opc[2];
		++opc;
	}
	else if (Mod == 0x2) { //binary 10
		called += *(long *)&opc[2];
		opc += 4;
	}
	
	if (Mod != 0x3) { //binary 11
		called = *(CDWORD *)called;
	}

	if (Reg == 2 || Reg == 3) { //binary 010 (near call) , 011 (far call)
		*next = opc + 2;
	}
	else if (Reg == 4 || Reg == 5) { //binary 100 (near jmp) , 101 (far jmp)
		CDWORD ret = *(CDWORD *)context->CustomSp;
		*next = (LPVOID)ret;
	}

	return called;
}

CDWORD SIBParser(BYTE* opc, PCONTEXT context, SIBParseResult *result) {
	BYTE SIB = opc[2];
	BYTE Scale = SIB >> 0x6;
	BYTE Index = (SIB >> 0x3) & 0x7;
	BYTE Base = SIB & 0x7;

	CDWORD called = NULL;
	switch (Index) {
	case CustomAx: called = context->CustomAx; break;
	case CustomCx: called = context->CustomCx; break;
	case CustomDx: called = context->CustomDx; break;
	case CustomBx: called = context->CustomBx; break;
	case CustomSp: break; //None
	case CustomBp: called = context->CustomBp; break;
	case CustomSi: called = context->CustomSi; break;
	case CustomDi: called = context->CustomDi; break;
	}

	called = called * (1 << Index);

	switch (Base) {
	case CustomAx: called += (long)context->CustomAx; break;
	case CustomCx: called += (long)context->CustomCx; break;
	case CustomDx: called += (long)context->CustomDx; break;
	case CustomBx: called += (long)context->CustomBx; break;
	case CustomSp: {
		BYTE Mod = opc[1] >> 6;
		switch (Mod) {
		case 0: //binary 00
			called += *(long *)&opc[3];
			opc += 4;
			break;
		case 1: //binary 01
			called += (char)opc[3] + context->CustomBp;
			++opc;
			break;
		case 2: //binary 10
			called += *(long *)&opc[3] + context->CustomBp;
			opc += 4;
			break;
		}
	}
	case CustomBp: called += (long)context->CustomBp; break;
	case CustomSi: called += (long)context->CustomSi; break;
	case CustomDi: called += (long)context->CustomDi; break;
	}

	result->opc = opc + 1;
	result->called = called;

	return called;
}