
#include "i8086sim.h"


/*
Bit set:	11   10   9    8    7    6    5    4   3    2   1    0
	     [OF] [DF] [IF] [TF] [SF] [ZF] [0] [AF] [0] [PF] [1] [CF]
*/


bool K8086VM::Execute_ADD_ADC_INC(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code, reg_code;
	byte DispSize=0;
	byte *REG, *EA, c;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	switch (II.m_BaseCode[0]) {
	case 0x0:	//Register/Memory with Register to Either:	000000dw|mod reg r/m
	case 0x10:	//Add CF to the result
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;
		c = ((II.m_BaseCode[0] == 0x10) ? VM->CF : 0);

		REG = VM->GetRegister(reg_code, (VM->m_CurCode[0] & 0x1) != 0, false);
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		//if d=1 then "to" reg; if d=0 then "from" reg
		if (VM->m_CurCode[0] & 0x2) {
			//REG = REG + REG/MEM
			if (VM->m_CurCode[0] & 0x1) {
				//*(ushort *)REG += (*(ushort *)EA + (ushort)c);
				*(ushort *)REG += (ushort)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[EA]
					mov dx,[edx]
					mov eax,[REG]
					add [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {
				//*REG += (*EA + c);
				*REG += c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[EA]
					mov dl,[edx]
					mov eax,[REG]
					add [eax],dl
					pushf
					pop word ptr [i8086Flags]
				}
			}
		} else {
			//REG/MEM = REG + REG/MEM
			if (VM->m_CurCode[0] & 0x1) {
				//*(ushort *)EA += (*(ushort *)REG + (ushort)c);
				*(ushort *)EA += (ushort)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[REG]
					mov dx,[edx]
					mov eax,[EA]
					add [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {
				//*EA += (*REG + c);
				*EA += c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[REG]
					mov dl,[edx]
					mov eax,[EA]
					add [eax],dl
					pushf
					pop word ptr [i8086Flags]
				}
			}
		}
		II.m_Size = DispSize+2;
		break;

	case 0x80:	//Immediate to Register/Memory:	100000sw|mod 0 c 0 r/m|data|data if sw=01
		//The bit "c" indicates if we should add the CF to the result
		c = ((VM->m_CurCode[1] & 0x10) ? VM->CF : 0);
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		//If sw=01 then 16 bits of immediate data form the operand.
		//If sw=11 then an immediate data byte is sign extended to form the 16-bit operand.
		if (VM->m_CurCode[0] & 0x1) {
			if (VM->m_CurCode[0] & 0x2) {	//sw=11
				//*(short *)EA += (imm + c);
				short imm = (short)(*(char *)(VM->m_CurCode+2+DispSize));
				*(short *)EA += (short)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov dx,[imm]
					mov eax,[EA]
					add [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {	//sw=01
				//*(ushort *)EA += (*(ushort *)(VM->m_CurCode+2+DispSize) + (ushort)c);
				ushort imm = *(ushort *)(VM->m_CurCode+2+DispSize);
				*(ushort *)EA += (ushort)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov dx,[imm]
					mov eax,[EA]
					add [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			}
		} else {
			//*EA += (*(VM->m_CurCode+2+DispSize) + c);
			byte imm = *(VM->m_CurCode+2+DispSize);
			*EA += c;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dl,[imm]
				mov eax,[EA]
				add [eax],dl
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = DispSize + 3 + (((VM->m_CurCode[0] & 0x3) == 0x1) ? 1 : 0);
		break;

	case 0x4:	//Immediate to Accumulator:	0000010w|data|data if w=1
	case 0x14:	//Add CF to the result
		c = ((II.m_BaseCode[0] == 0x14) ? VM->CF : 0);

		if (VM->m_CurCode[0] & 0x1) {
			//VM->A.X += (*(ushort *)(VM->m_CurCode+1) + (ushort)c);
			ushort imm = *(ushort *)(VM->m_CurCode+1);
			VM->A.X += (ushort)c;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dx,[imm]
				mov eax,VM
				lea eax,[eax]VM.A.X
				add [eax],dx
				pushf
				pop word ptr [i8086Flags]
			}
		} else {
			//VM->A.R.L += (*(VM->m_CurCode+1) + c);
			byte imm = *(VM->m_CurCode+1);
			VM->A.R.L += c;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dl,[imm]
				mov eax,VM
				lea eax,[eax]VM.A.R.L
				add [eax],dl
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;

	case 0xFE:	//Register/Memory:	1111111w|mod 000 r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		if (VM->m_CurCode[0] & 0x1) {
			//(*(ushort *)EA) ++;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov eax,EA
				inc word ptr [eax]
				pushf
				pop word ptr [i8086Flags]
			}
		} else {
			//(*EA) ++;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov eax,EA
				inc byte ptr [eax]
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = DispSize + 2;
		break;

	case 0x40:	//Register:	01000reg
		reg_code = VM->m_CurCode[0] & 0x7;
		VM->GetRegister(reg_code, true, false);
		__asm {
			push word ptr [i8086Flags]
			popf
			inc word ptr [eax]
			pushf
			pop word ptr [i8086Flags]
		}
		II.m_Size = 1;
		break;
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_SUB_SSB_DEC(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code, reg_code;
	byte DispSize=0;
	byte *REG, *EA, c;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	switch (II.m_BaseCode[0]) {
	case 0x28:	//Register/Memory and Register to Either
	case 0x18:	//Substract CF from result
		c = ((II.m_BaseCode[0] == 0x18) ? VM->CF : 0);
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;

		REG = VM->GetRegister(reg_code, (VM->m_CurCode[0] & 0x1) != 0, false);
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		//if d=1 then "to" reg; if d=0 then "from" reg
		if (VM->m_CurCode[0] & 0x2) {
			//REG = REG - REG/MEM
			if (VM->m_CurCode[0] & 0x1) {
				//*(ushort *)REG -= (*(ushort *)EA + (ushort)c);
				*(ushort *)REG -= (ushort)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[EA]
					mov dx,[edx]
					mov eax,[REG]
					sub [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {
				//*REG -= (*EA + c);
				*REG -= c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[EA]
					mov dl,[edx]
					mov eax,[REG]
					sub [eax],dl
					pushf
					pop word ptr [i8086Flags]
				}
			}
		} else {
			//REG/MEM = REG/MEM - REG
			if (VM->m_CurCode[0] & 0x1) {
				//*(ushort *)EA -= (*(ushort *)REG + (ushort)c);
				*(ushort *)EA -= (ushort)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[REG]
					mov dx,[edx]
					mov eax,[EA]
					sub [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {
				//*EA -= (*REG + c);
				*EA -= c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[REG]
					mov dl,[edx]
					mov eax,[EA]
					sub [eax],dl
					pushf
					pop word ptr [i8086Flags]
				}
			}
		}
		II.m_Size = DispSize+2;
		break;

	case 0x80:	//Immediate from Register/Memory:	100000sw|mod 0 c 1 r/m|data|data if sw=01
		//The bit "c" indicates if we should substract the CF from the result
		c = ((VM->m_CurCode[1] & 0x10) ? VM->CF : 0);
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		//If sw=01 then 16 bits of immediate data form the operand.
		//If sw=11 then an immediate data byte is sign extended to form the 16-bit operand.
		if (VM->m_CurCode[0] & 0x1) {
			if (VM->m_CurCode[0] & 0x2) {	//sw=11
				//*(short *)EA -= (imm + c);
				short imm = (short)(*(char *)(VM->m_CurCode+2+DispSize));
				*(short *)EA -= (short)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov dx,[imm]
					mov eax,[EA]
					sub [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {	//sw=01
				//*(ushort *)EA -= (*(ushort *)(VM->m_CurCode+2+DispSize) + (ushort)c);
				ushort imm = *(ushort *)(VM->m_CurCode+2+DispSize);
				*(ushort *)EA -= (ushort)c;
				__asm {
					push word ptr [i8086Flags]
					popf
					mov dx,[imm]
					mov eax,[EA]
					sub [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			}
		} else {
			//*EA -= (*(VM->m_CurCode+2+DispSize) + c);
			byte imm = *(VM->m_CurCode+2+DispSize);
			*EA -= c;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dl,[imm]
				mov eax,[EA]
				sub [eax],dl
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = DispSize + 3 + (((VM->m_CurCode[0] & 0x3) == 0x1) ? 1 : 0);
		break;

	case 0x2C:	//Immediate from Accumulator:		0010110w|data|data if w=1
	case 0x0E:	//Substract CF from result
		c = ((II.m_BaseCode[0] == 0x0E) ? VM->CF : 0);

		if (VM->m_CurCode[0] & 0x1) {
			//VM->A.X -= (*(ushort *)(VM->m_CurCode+1) + (ushort)c);
			ushort imm = *(ushort *)(VM->m_CurCode+1);
			VM->A.X -= (ushort)c;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dx,[imm]
				mov eax,VM
				lea eax,[eax]VM.A.X
				sub [eax],dx
				pushf
				pop word ptr [i8086Flags]
			}
		} else {
			//VM->A.R.L -= (*(VM->m_CurCode+1) + c);
			byte imm = *(VM->m_CurCode+1);
			VM->A.R.L += c;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dl,[imm]
				mov eax,VM
				lea eax,[eax]VM.A.R.L
				sub [eax],dl
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;

	case 0xFE:	//Register/Memory:	1111111w|mod 001 r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		if (VM->m_CurCode[0] & 0x1) {
			//(*(ushort *)EA) --;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov eax,EA
				dec word ptr [eax]
				pushf
				pop word ptr [i8086Flags]
			}
		} else {
			//(*EA) --;
			__asm {
				push word ptr [i8086Flags]
				popf
				mov eax,EA
				dec byte ptr [eax]
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = DispSize + 2;
		break;

	case 0x48:	//Register:	01001reg
		reg_code = VM->m_CurCode[0] & 0x7;
		VM->GetRegister(reg_code, true, false);
		__asm {
			push word ptr [i8086Flags]
			popf
			dec word ptr [eax]
			pushf
			pop word ptr [i8086Flags]
		}
		II.m_Size = 1;
		break;
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_NEG(K8086VM *VM, KInstructionID &II)
{
	//1111011w|mod 011 r/m
	byte DispSize=0;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	char mod = (VM->m_CurCode[1] & 0xC0) >> 6;
	char mem_reg_code = VM->m_CurCode[1] & 0x7;
	byte *EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

	if (VM->m_CurCode[0] & 0x1) {
		__asm {
			push word ptr [i8086Flags]
			popf
			mov eax,EA
			neg word ptr [eax]
			pushf
			pop word ptr [i8086Flags]
		}
	} else {
		__asm {
			push word ptr [i8086Flags]
			popf
			mov eax,EA
			neg byte ptr [eax]
			pushf
			pop word ptr [i8086Flags]
		}
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	II.m_Size = DispSize + 2;
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_CMP(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code, reg_code;
	byte DispSize=0;
	byte *REG, *EA;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	switch (II.m_BaseCode[0]) {
	case 0x38:	//Register/Memory and Register:	001110dw|mod reg r/m
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		reg_code = (VM->m_CurCode[1] & 0x38) >> 3;

		REG = VM->GetRegister(reg_code, (VM->m_CurCode[0] & 0x1) != 0, false);
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		//if d=1 then "to" reg; if d=0 then "from" reg
		if (VM->m_CurCode[0] & 0x2) {
			//REG = REG - REG/MEM
			if (VM->m_CurCode[0] & 0x1) {
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[EA]
					mov dx,[edx]
					mov eax,[REG]
					cmp [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[EA]
					mov dl,[edx]
					mov eax,[REG]
					cmp [eax],dl
					pushf
					pop word ptr [i8086Flags]
				}
			}
		} else {
			//REG/MEM = REG/MEM - REG
			if (VM->m_CurCode[0] & 0x1) {
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[REG]
					mov dx,[edx]
					mov eax,[EA]
					cmp [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {
				__asm {
					push word ptr [i8086Flags]
					popf
					mov edx,[REG]
					mov dl,[edx]
					mov eax,[EA]
					cmp [eax],dl
					pushf
					pop word ptr [i8086Flags]
				}
			}
		}
		II.m_Size = DispSize+2;
		break;

	case 0x80:	//Immediate with Register/Memory:	100000sw|mod 111 r/m|data|data if sw=01
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;

		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

		//If sw=01 then 16 bits of immediate data form the operand.
		//If sw=11 then an immediate data byte is sign extended to form the 16-bit operand.
		if (VM->m_CurCode[0] & 0x1) {
			if (VM->m_CurCode[0] & 0x2) {	//sw=11
				short imm = (short)(*(char *)(VM->m_CurCode+2+DispSize));
				__asm {
					push word ptr [i8086Flags]
					popf
					mov dx,[imm]
					mov eax,[EA]
					cmp [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			} else {	//sw=01
				ushort imm = *(ushort *)(VM->m_CurCode+2+DispSize);
				__asm {
					push word ptr [i8086Flags]
					popf
					mov dx,[imm]
					mov eax,[EA]
					cmp [eax],dx
					pushf
					pop word ptr [i8086Flags]
				}
			}
		} else {
			byte imm = *(VM->m_CurCode+2+DispSize);
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dl,[imm]
				mov eax,[EA]
				cmp [eax],dl
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = DispSize + 3 + (((VM->m_CurCode[0] & 0x3) == 0x1) ? 1 : 0);
		break;

	case 0x3C:	//Immediate with Accumulator:	0011110w|data|data if w=1
		if (VM->m_CurCode[0] & 0x1) {
			ushort imm = *(ushort *)(VM->m_CurCode+1);
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dx,[imm]
				mov eax,VM
				lea eax,[eax]VM.A.X
				cmp [eax],dx
				pushf
				pop word ptr [i8086Flags]
			}
		} else {
			byte imm = *(VM->m_CurCode+1);
			__asm {
				push word ptr [i8086Flags]
				popf
				mov dl,[imm]
				mov eax,VM
				lea eax,[eax]VM.A.R.L
				cmp [eax],dl
				pushf
				pop word ptr [i8086Flags]
			}
		}
		II.m_Size = 2 + ((VM->m_CurCode[0] & 0x1) ? 1 : 0);
		break;
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_AAA_DAA_AAS_DAS_AAM_AAD_CBW_CWD(K8086VM *VM, KInstructionID &II)
{
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

#define ADJUST_ASM_CODE(adjust_instruction)	\
	__asm push word ptr [i8086Flags]	\
	__asm popf						\
	__asm mov edx,[VM]			\
	__asm lea edx,[edx]VM.A.X	\
	__asm mov ax,[edx]			\
	__asm adjust_instruction	\
	__asm mov [edx],ax			\
	__asm pushf						\
	__asm pop word ptr [i8086Flags]

	switch (II.m_BaseCode[0]) {
	case 0x37:	//ASCII Adjust for Add
		ADJUST_ASM_CODE(aaa);
		break;

	case 0x27:	//Decimal Adjust for Add
		ADJUST_ASM_CODE(daa);
		break;

	case 0x3F:	//ASCII Adjust for Subtract
		ADJUST_ASM_CODE(aas);
		break;

	case 0x2F:	//Decimal Adjust for Subtract
		ADJUST_ASM_CODE(das);
		break;

	case 0xD4:	//ASCII Adjust for Multiply
		ADJUST_ASM_CODE(aam);
		break;

	case 0xD5:	//ASCII Adjust for Divide
		ADJUST_ASM_CODE(aad);
		break;

	case 0x98:	//Convert Byte to Word
		ADJUST_ASM_CODE(cbw);
		break;

	case 0x99:	//Convert Word to Double Word
		__asm {
			push word ptr [i8086Flags]
			popf
			mov ecx,[VM]
			lea ecx,[ecx]VM.A.X

			mov ax,[ecx]
			cwd
			mov [ecx],ax

			pushf
			pop word ptr [i8086Flags]

			mov ecx,[VM]
			lea ecx,[ecx]VM.D.X
			mov [ecx],dx
		}
		break;
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	VM->IP += II.m_Size;
	return true;
}


bool K8086VM::Execute_MUL_IMUL_DIV_IDIV(K8086VM *VM, KInstructionID &II)
{
	//1111011w|mod 100 r/m
	byte DispSize=0;
	ushort i8086Flags=VM->GetFlagsIntoBitSet();

	char mod = (VM->m_CurCode[1] & 0xC0) >> 6;
	char mem_reg_code = VM->m_CurCode[1] & 0x7;
	byte *EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, VM->m_CurCode[0] & 0x1);

#define MULDIV_ASM_CODE(instruction_mnemonic)	\
	if (VM->m_CurCode[0] & 0x1) {	\
		__asm push word ptr [i8086Flags]				\
		__asm popf							\
		__asm mov ecx,[VM]				\
		__asm lea ecx,[ecx]VM.A.X		\
		__asm mov ax,[ecx]				\
		__asm mov ecx,[VM]				\
		__asm lea ecx,[ecx]VM.D.X		\
		__asm mov dx,[ecx]				\
		__asm mov ecx,[EA]				\
		__asm instruction_mnemonic word ptr [ecx]	\
		__asm mov ecx,[VM]				\
		__asm lea ecx,[ecx]VM.A.X		\
		__asm mov [ecx],ax				\
		__asm mov ecx,[VM]				\
		__asm lea ecx,[ecx]VM.D.X		\
		__asm mov [ecx],dx				\
		__asm pushf							\
		__asm pop word ptr [i8086Flags]				\
	} else {									\
		__asm push word ptr [i8086Flags]				\
		__asm popf							\
		__asm mov ecx,[VM]				\
		__asm lea ecx,[ecx]VM.A.X		\
		__asm mov ax,[ecx]				\
		__asm mov ecx,[EA]				\
		__asm instruction_mnemonic byte ptr [ecx]	\
		__asm mov ecx,[VM]				\
		__asm lea ecx,[ecx]VM.A.X		\
		__asm mov [ecx],ax				\
		__asm pushf							\
		__asm pop word ptr [i8086Flags]				\
	}

	if (VM->m_CurCode[1] & 0x10) {	//Division
		if (VM->m_CurCode[1] & 0x8) {	//Signed
			MULDIV_ASM_CODE(idiv);
		} else {	//Unsigned
			MULDIV_ASM_CODE(div);
		}
	} else {	//Multiplication
		if (VM->m_CurCode[1] & 0x8) {	//Signed
			MULDIV_ASM_CODE(imul);
		} else {	//Unsigned
			MULDIV_ASM_CODE(mul);
		}
	}

	VM->SetFlagsFromBitSet(i8086Flags);
	II.m_Size = DispSize+2;
	VM->IP += II.m_Size;
	return true;
}
