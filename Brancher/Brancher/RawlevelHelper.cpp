#include "stdafx.h"
#include "RawlevelHelper.h"

enum RnM { RegisterAx, RegisterCx, RegisterDx, RegisterBx, RegisterSp, RegisterBp, RegisterSi, RegisterDi };

CDWORD GetBranchingAddress(BYTE *opc, PCONTEXT context, LPVOID *next) {
	BYTE Mod = opc[1] >> 0x6; // high 2bits
	BYTE Reg = (opc[1] >> 0x3) & 0x7; // mid 3bits
	BYTE RnM = opc[1] & 0x7; // low 3bits

	CDWORD called = NULL;

	switch (RnM) {
	case RegisterAx: 
		called = context->RegisterAx; 
		break;
	case RegisterCx:
		called = context->RegisterCx;
		break;
	case RegisterDx:
		called = context->RegisterDx;
		break;
	case RegisterBx:
		called = context->RegisterBx;
		break;
	case RegisterSp:
		if (Mod != 0x3) { // binary 11
			SIBParseResult SibResult;
			SIBParser(opc, context, &SibResult);

			opc = SibResult.opc;
			called = SibResult.called;
		}
		else {
			called = context->RegisterSp;
		}
		break;
	case RegisterBp:
		if (Mod == 0x00) {
#ifdef _WIN64
			called = context->RegisterIp + *(long *)&opc[2] + 6;
#else
			called = *(long *)&opc[2];
#endif
			opc += 4;
		}
		else {
			called = context->RegisterBp;
		}
		break;
	case RegisterSi:
		called = context->RegisterSi;
		break;
	case RegisterDi:
		called = context->RegisterDi;
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
		CDWORD ret = *(CDWORD *)context->RegisterSp;
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
	case RegisterAx: called = context->RegisterAx; break;
	case RegisterCx: called = context->RegisterCx; break;
	case RegisterDx: called = context->RegisterDx; break;
	case RegisterBx: called = context->RegisterBx; break;
	case RegisterSp: break; //None
	case RegisterBp: called = context->RegisterBp; break;
	case RegisterSi: called = context->RegisterSi; break;
	case RegisterDi: called = context->RegisterDi; break;
	}

	called = called * (1 << Scale);

	switch (Base) {
	case RegisterAx: called += (long)context->RegisterAx; break;
	case RegisterCx: called += (long)context->RegisterCx; break;
	case RegisterDx: called += (long)context->RegisterDx; break;
	case RegisterBx: called += (long)context->RegisterBx; break;
	case RegisterSp: called += (long)context->RegisterSp; break;
	case RegisterBp: {
		BYTE Mod = opc[1] >> 6;
		switch (Mod) {
		case 0: //binary 00
			called += *(long *)&opc[3];
			opc += 4;
			break;
		case 1: //binary 01
			called += (char)opc[3] + context->RegisterBp;
			++opc;
			break;
		case 2: //binary 10
			called += *(long *)&opc[3] + context->RegisterBp;
			opc += 4;
			break;
		}
		break;
	}
	case RegisterSi: called += (long)context->RegisterSi; break;
	case RegisterDi: called += (long)context->RegisterDi; break;
	}

	result->opc = opc + 1;
	result->called = called;

	return called;
}