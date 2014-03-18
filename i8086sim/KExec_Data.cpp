
#include "i8086sim.h"



#define CopyValue(t,s,w)	\
{	\
	if (w) {	\
		ushort *_s = (ushort *)(s), *_t = (ushort *)(t);	\
		*_t = *_s;	\
	} else {	\
		byte *_s = (byte *)(s), *_t = (byte *)(t);	\
		*_t = *_s;	\
	}	\
}


bool K8086VM::Execute_MOV(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code, reg_code;
	byte DispSize=0;
	ushort EA;
	byte *p, *t;

	switch (II.m_BaseCode[0]) {
	case 0xB0:	//Immediate to Register:	1011wreg|data|data if w=1
		p = VM->GetRegister(VM->m_CurCode[0] & 0x7, (VM->m_CurCode[0] & 0x8) != 0, false);
		CopyValue(p, VM->m_CurCode+1, VM->m_CurCode[0] & 0x8);
		II.m_Size = 2 + (VM->m_CurCode[0] & 0x8 ? 1 : 0);
		break;

	case 0xC6:	//Immediate to Register/Memory:	1100011w|mod 000 r/m|data|data if w=1
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		p = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);
		CopyValue(p, VM->m_CurCode+2+DispSize, VM->m_CurCode[0] & 0x1);
		II.m_Size = DispSize + 3 + (VM->m_CurCode[0] & 0x1 ? 1 : 0);
		break;

	case 0xA0:	//Memory to Accumulator:	1010000w|addr-low|addr-high
		EA = *(ushort *)(VM->m_CurCode+1);
		p = VM->GetRegister(0 /* Accumulator */, (VM->m_CurCode[0] & 0x1) != 0, false);
		CopyValue(p, VM->m_Memory+VM->DS*16+EA, VM->m_CurCode[0] & 0x1);
		II.m_Size = 3;
		break;

	case 0xA2:	//Accumulator to Memory:	1010001w|addr-low|addr-high
		EA = *(ushort *)(VM->m_CurCode+1);
		p = VM->GetRegister(0 /* Accumulator */, (VM->m_CurCode[0] & 0x1) != 0, false);
		CopyValue(VM->m_Memory+VM->DS*16+EA, p, VM->m_CurCode[0] & 0x1);
		II.m_Size = 3;
		break;

	case 0x88:	//Register/Memory to/from Register:	100010dw|mod reg r/m
		//if d=1 then "to" reg; if d=0 then "from" reg
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;

		if (VM->m_CurCode[0] & 0x2) {
			p = VM->GetRegister(reg_code, (VM->m_CurCode[0] & 0x1) != 0, false);
			t = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);
			CopyValue(p, t, VM->m_CurCode[0] & 0x1);
		} else {
			p = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);
			t = VM->GetRegister(reg_code, (VM->m_CurCode[0] & 0x1) != 0, false);
			CopyValue(p, t, VM->m_CurCode[0] & 0x1);
		}
		II.m_Size = DispSize+2;
		break;

	case 0x8E:	//Register/Memory to Segment Register:	10001110|mod 0 reg r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		p = VM->GetRegister(reg_code, true, true);
		t = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, true);

#pragma warning(disable: 4127)
		CopyValue(p, t, true);
#pragma warning(default: 4127)

		II.m_Size = DispSize+2;
		break;

	case 0x8C:	//Segment Register to Register/Memory:	10001100|mod 0 reg r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		p = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, true);
		t = VM->GetRegister(reg_code, true, true);
#pragma warning(disable: 4127)
		CopyValue(p, t, true);
#pragma warning(default: 4127)

		II.m_Size = DispSize+2;
		break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_PUSH(K8086VM *VM, KInstructionID &II)
{
	char mod, reg_code, mem_reg_code;
	byte DispSize=0;

	switch (II.m_BaseCode[0]) {
	case 0x50:	//Register: 01010reg
		reg_code = (VM->m_CurCode[0] & 0x7);
		VM->SP -= 2;
		VM->SetDataAt(VM->SS, VM->SP, 2, *(short *)VM->GetRegister(reg_code, true, false));
		II.m_Size = 1;
		break;

	case 0x06:	//Segment Register: 000reg110
		reg_code = (VM->m_CurCode[0] & 0x38) >> 3;
		VM->SP -= 2;
		VM->SetDataAt(VM->SS, VM->SP, 2, *(short *)VM->GetRegister(reg_code, true, true));
		II.m_Size = 1;
		break;

	case 0xFF:	//Register/Memory:	11111111|mod 110 r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		VM->SP -= 2;
		VM->SetDataAt(VM->SS, VM->SP, 2, *(short *)VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, true));
		II.m_Size = 2+DispSize;
		break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_POP(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code, reg_code;
	byte DispSize=0;

	switch (II.m_BaseCode[0]) {
	case 0x8F:	//Register/Memory:	10001111|mod 000 r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, true));
		VM->SP += 2;
		II.m_Size = 2+DispSize;
		break;

	case 0x58:	//Register: 01011reg
		reg_code = (VM->m_CurCode[0] & 0x7);

		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)VM->GetRegister(reg_code, true, false));
		VM->SP += 2;
		II.m_Size = 1;
		break;

	case 0x07:	//Segment Register: 000reg111
		reg_code = (VM->m_CurCode[0] & 0x38) >> 3;
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)VM->GetRegister(reg_code, true, true));
		VM->SP += 2;
		II.m_Size = 1;
		break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_XCHG(K8086VM *VM, KInstructionID &II)
{
	char mod, reg_code, mem_reg_code;
	byte DispSize=0;
	byte *EA;

	switch (II.m_BaseCode[0]) {
	case 0x86:	//Register/Memory with Register:	1000011w|mod reg r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		if (VM->m_CurCode[0] & 0x1) {
			ushort *REG = (ushort *)VM->GetRegister(reg_code, VM->m_CurCode[0] & 0x1, false);

			ushort t = *REG;
			*REG = *(ushort *)EA;
			*(ushort *)EA = t;
		} else {
			byte *REG = (byte *)VM->GetRegister(reg_code, VM->m_CurCode[0] & 0x1, false);

			byte t = *REG;
			*REG = *(byte *)EA;
			*(byte *)EA = t;
		}

		II.m_Size = 2+DispSize;
		break;

	case 0x90:	//Register with Accumulator: 10010reg
		reg_code = VM->m_CurCode[0] & 0x7;
		ushort *REG = (ushort *)VM->GetRegister(reg_code, true, false);
		ushort *ACC = (ushort *)VM->GetRegister(0 /* Accumulator */, true, false);

		ushort t = *REG;
		*REG = *ACC;
		*ACC = t;

		II.m_Size = 1;
		break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_XLAT(K8086VM *VM, KInstructionID &II)
{
	VM->A.R.L = *(VM->m_Memory + VM->DS * 0x10 + VM->B.X + (byte)VM->A.R.L);
	II.m_Size = 1;
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_LEA_LDS_LES(K8086VM *VM, KInstructionID &II)
{
	char mod = (VM->m_CurCode[1] & 0xC0) >> 6;
	char reg_code = (VM->m_CurCode[1] & 0x38) >> 3;
	char mem_reg_code = VM->m_CurCode[1] & 0x7;
	byte DispSize=0;
	byte *EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);
	unsigned short *REG = (unsigned short *)VM->GetRegister(reg_code, true, false);

	switch (VM->m_CurCode[0]) {
	case 0x8D:	//LEA:	10001101|mod reg r/m
		*REG = (ushort)((EA - VM->m_Memory) - VM->DS * 16);	//Get the effective address
		break;

	case 0xC5:	//LDS:	11000101|mod reg r/m
		*REG = *(unsigned short *)EA;
		VM->DS = *(unsigned short *)(EA+2);
		break;

	case 0xC4:	//LES:	11000101|mod reg r/m
		*REG = *(unsigned short *)EA;
		VM->ES = *(unsigned short *)(EA+2);
		break;
	}

	II.m_Size = 2+DispSize;
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_LAHF_SAHF(K8086VM *VM, KInstructionID &II)
{
	//AH bit:   7    6    5    4   3    2   1    0
	//	        [SF] [ZF] [0] [AF] [0] [PF] [1] [CF]

	switch (VM->m_CurCode[0]) {
	case 0x9F:	//LAHF:	10011111
		VM->A.R.H = (byte)VM->GetFlagsIntoBitSet();
		break;

	case 0x9E:	//SAHF:	10011110
		VM->SetFlagsFromBitSet(VM->A.R.H);
		break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_PUSHF_POPF(K8086VM *VM, KInstructionID &II)
{
	ushort *STACK;
	switch (VM->m_CurCode[0]) {
	case 0x9C:	//PUSHF
		VM->SP -= 2;
		STACK = (ushort *)(VM->m_Memory + VM->SS * 0x10 + VM->SP);

		*STACK = VM->CF | 0x2;
		*STACK |= (VM->PF << 2);
		*STACK |= (VM->AF << 4);
		*STACK |= (VM->ZF << 6);
		*STACK |= (VM->SF << 7);
		break;

	case 0x9D:	//POPF
		STACK = (ushort *)(VM->m_Memory + VM->SS * 0x10 + VM->SP);
		VM->SP += 2;

		VM->CF = (*STACK) & 0x1;
		VM->PF = (*STACK) & 0x4;
		VM->AF = (*STACK) & 0x10;
		VM->ZF = (*STACK) & 0x40;
		VM->SF = (*STACK) & 0x80;
		break;
	}

	VM->IP += II.m_Size;
	return true;
}


#undef CopyValue
