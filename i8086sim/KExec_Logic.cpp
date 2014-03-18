
#include "i8086sim.h"


/*
Bit set:	11   10   9    8    7    6    5    4   3    2   1    0
	     [OF] [DF] [IF] [TF] [SF] [ZF] [0] [AF] [0] [PF] [1] [CF]
*/

bool K8086VM::Execute_NOT_SHL_SAL_SHR_SAR_ROL_ROR_RCL_RCR(K8086VM *VM, KInstructionID &II)
{
	//NOT:		1111011w|mod 010 r/m
	//Others:	110100vw|mod 000 r/m

	byte DispSize=0;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	char mod = (VM->m_CurCode[1] & 0xC0) >> 6;
	char mem_reg_code = VM->m_CurCode[1] & 0x7;

	byte *EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

#define LOGIC_ASM_CODE_LEN(mnemonic, len_op)	{	\
		__asm push word ptr [i8086Flags]				\
		__asm popf											\
		__asm mov eax,[EA]								\
		__asm mov cl,[count]								\
		__asm mnemonic len_op ptr [eax],cl			\
		__asm pushf											\
		__asm pop word ptr [i8086Flags]				\
	}

#define NOT_ASM_CODE_LEN(len_op)	{					\
		__asm push word ptr [i8086Flags]				\
		__asm popf											\
		__asm mov eax,[EA]								\
		__asm not len_op ptr [eax]						\
		__asm pushf											\
		__asm pop word ptr [i8086Flags]				\
	}

#define LOGIC_ASM_CODE(mnemonic)			{	\
	if (VM->m_CurCode[0] & 0x1)				\
		LOGIC_ASM_CODE_LEN(mnemonic, word)	\
	else												\
		LOGIC_ASM_CODE_LEN(mnemonic, byte)	\
	}

#define NOT_ASM_CODE()			{	\
	if (VM->m_CurCode[0] & 0x1)	\
		NOT_ASM_CODE_LEN(word)		\
	else									\
		NOT_ASM_CODE_LEN(byte)		\
	}

	if (II.m_BaseCode[0] == 0xF6)	//NOT
		NOT_ASM_CODE()
	else {
		byte count = (!(VM->m_CurCode[0] & 0x2) ? 1 : VM->C.R.L);

		switch (VM->m_CurCode[1] & 0x38) {
		case 0x20:	LOGIC_ASM_CODE(shl);	break;
		case 0x28:	LOGIC_ASM_CODE(shr);	break;
		case 0x38:	LOGIC_ASM_CODE(sar);	break;
		case 0x00:	LOGIC_ASM_CODE(rol);	break;
		case 0x08:	LOGIC_ASM_CODE(ror);	break;
		case 0x10:	LOGIC_ASM_CODE(rcl);	break;
		case 0x18:	LOGIC_ASM_CODE(rcr);	break;
		}
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	II.m_Size = DispSize+2;
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_TEST_AND_OR_XOR(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code, reg_code;
	byte DispSize=0;
	byte *REG, *EA;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	switch (II.m_BaseCode[0]) {
	case 0x84:
	case 0x20:
	case 0x08:
	case 0x30:
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		REG = VM->GetRegister(reg_code, (VM->m_CurCode[0] & 0x1) != 0, false);
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

	//if d=1 then "to" reg; if d=0 then "from" reg
#define LOGIC_OP_ASM_CODE(mnemonic)	\
	if (VM->m_CurCode[0] & 0x2) {	\
		if (VM->m_CurCode[0] & 0x1) {	\
			__asm push word ptr [i8086Flags]	\
			__asm popf	\
			__asm mov edx,[EA]	\
			__asm mov dx,[edx]	\
			__asm mov eax,[REG]	\
			__asm mnemonic [eax],dx	\
			__asm pushf	\
			__asm pop word ptr [i8086Flags]	\
		} else {	\
			__asm push word ptr [i8086Flags]	\
			__asm popf	\
			__asm mov edx,[EA]	\
			__asm mov dl,[edx]	\
			__asm mov eax,[REG]	\
			__asm mnemonic [eax],dl	\
			__asm pushf	\
			__asm pop word ptr [i8086Flags]	\
		}	\
	} else {	\
		if (VM->m_CurCode[0] & 0x1) {	\
			__asm push word ptr [i8086Flags]	\
			__asm popf	\
			__asm mov edx,[REG]	\
			__asm mov dx,[edx]	\
			__asm mov eax,[EA]	\
			__asm mnemonic [eax],dx	\
			__asm pushf	\
			__asm pop word ptr [i8086Flags]	\
		} else {	\
			__asm push word ptr [i8086Flags]	\
			__asm popf	\
			__asm mov edx,[REG]	\
			__asm mov dl,[edx]	\
			__asm mov eax,[EA]	\
			__asm mnemonic [eax],dl	\
			__asm pushf	\
			__asm pop word ptr [i8086Flags]	\
		}	\
	}

		switch (II.m_BaseCode[0]) {
		case 0x84:	LOGIC_OP_ASM_CODE(test);	break;
		case 0x20:	LOGIC_OP_ASM_CODE(and);		break;
		case 0x08:	LOGIC_OP_ASM_CODE(or);		break;
		case 0x30:	LOGIC_OP_ASM_CODE(xor);		break;
		}

		II.m_Size = DispSize+2;
		break;

	case 0xF6:
	case 0x80:
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

#define LOGIC_OP_ASM_CODE_2(mnemonic)	\
	if (VM->m_CurCode[0] & 0x1) {	\
		ushort imm = *(ushort *)(VM->m_CurCode+2+DispSize);	\
		__asm push word ptr [i8086Flags]	\
		__asm popf	\
		__asm mov dx,[imm]	\
		__asm mov eax,[EA]	\
		__asm mnemonic [eax],dx	\
		__asm pushf	\
		__asm pop word ptr [i8086Flags]	\
	} else {	\
		byte imm = *(VM->m_CurCode+2+DispSize);	\
		__asm push word ptr [i8086Flags]	\
		__asm popf	\
		__asm mov dl,[imm]	\
		__asm mov eax,[EA]	\
		__asm mnemonic [eax],dl	\
		__asm pushf	\
		__asm pop word ptr [i8086Flags]	\
	}

		if (II.m_BaseCode[0] == 0xF6) {
			LOGIC_OP_ASM_CODE_2(test)
		} else {
			switch (II.m_BaseCode[1]) {
			case 0x20:	LOGIC_OP_ASM_CODE_2(and);	break;
			case 0x08:	LOGIC_OP_ASM_CODE_2(or);	break;
			case 0x30:	LOGIC_OP_ASM_CODE_2(xor);	break;
			}
		}
		II.m_Size = DispSize + 3 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;

	case 0xA8:
#define LOGIC_OP_ASM_CODE_3(mnemonic)	\
	if (VM->m_CurCode[0] & 0x1) {	\
		ushort imm = *(ushort *)(VM->m_CurCode+1);	\
		__asm push word ptr [i8086Flags]	\
		__asm popf	\
		__asm mov dx,[imm]	\
		__asm mov eax,VM	\
		__asm lea eax,[eax]VM.A.X	\
		__asm mnemonic [eax],dx	\
		__asm pushf	\
		__asm pop word ptr [i8086Flags]	\
	} else {	\
		byte imm = *(VM->m_CurCode+1);	\
		__asm push word ptr [i8086Flags]	\
		__asm popf	\
		__asm mov dl,[imm]	\
		__asm mov eax,VM	\
		__asm lea eax,[eax]VM.A.R.L	\
		__asm mnemonic [eax],dl	\
		__asm pushf	\
		__asm pop word ptr [i8086Flags]	\
	}

		LOGIC_OP_ASM_CODE_3(test);
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;

	case 0x24:
		LOGIC_OP_ASM_CODE_3(and);
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;

	case 0x0C:
		LOGIC_OP_ASM_CODE_3(or);
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;

	case 0x34:
		LOGIC_OP_ASM_CODE_3(xor);
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	VM->IP += II.m_Size;
	return true;
}
