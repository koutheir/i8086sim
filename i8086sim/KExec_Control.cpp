
#include "i8086sim.h"


bool K8086VM::Execute_CALL_RET(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code;
	byte DispSize=0;
	byte *EA;

	switch (II.m_BaseCode[0]) {
	case 0xE8:	//CALL: Direct within Segment
		VM->SP -= 2;
		VM->SetDataAt(VM->SS, VM->SP, 2, VM->IP + II.m_Size);

		VM->IP += (*(short *)(VM->m_CurCode+1) + II.m_Size);
		break;

	case 0x9A:	//CALL: Direct Intersegment
		VM->SP -= 2;
		VM->SetDataAt(VM->SS, VM->SP, 2, VM->CS);
		VM->SP -= 2;
		VM->SetDataAt(VM->SS, VM->SP, 2, VM->IP + II.m_Size);

		VM->IP = *(short *)(VM->m_CurCode+1);
		VM->CS = *(short *)(VM->m_CurCode+3);
		break;

	case 0xFF:
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, true);

		if (II.m_BaseCode[1] == 0x10) {
			//CALL: Indirect within Segment
			VM->SP -= 2;
			VM->SetDataAt(VM->SS, VM->SP, 2, VM->IP + II.m_Size);

			VM->IP = *(ushort *)EA;
		} else if (II.m_BaseCode[1] == 0x18) {
			//CALL: Indirect Intersegment
			VM->SP -= 2;
			VM->SetDataAt(VM->SS, VM->SP, 2, VM->CS);
			VM->SP -= 2;
			VM->SetDataAt(VM->SS, VM->SP, 2, VM->IP + II.m_Size);

			VM->IP = *(ushort *)EA;
			VM->CS = *(ushort *)(EA+2);
		}
		break;

	case 0xC3:	//RET: Within Segment
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)&VM->IP);
		VM->SP += 2;
		break;

	case 0xC2:	//RET: Within Segment Adding Immediate to SP
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)&VM->IP);
		VM->SP += (2 + *(short *)(VM->m_CurCode+1));
		break;

	case 0xCB:	//Intersegment
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)&VM->IP);
		VM->SP += 2;
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)&VM->CS);
		VM->SP += 2;
		break;

	case 0xCA:	//Intersegment Adding Immediate to SP
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)&VM->IP);
		VM->SP += 2;
		VM->GetDataAt(VM->SS, VM->SP, 2, (short *)&VM->CS);
		VM->SP += (2 + *(short *)(VM->m_CurCode+1));
		break;
	}

	return true;
}

bool K8086VM::Execute_JMP_JZ_JNZ_JL_JLE_JG_JGE_JB_JBE_JA_JAE_JO_JNO_JS_JNS_JPO_JPE_JCXZ(K8086VM *VM, KInstructionID &II)
{
	char mod, mem_reg_code;
	byte DispSize=0;
	byte *EA;

	switch (II.m_BaseCode[0]) {
	case 0xFF:
		mod = (VM->m_CurCode[1] & 0xC0) >> 6;
		mem_reg_code = VM->m_CurCode[1] & 0x7;
		EA = VM->GetEffectiveAddress(mod, mem_reg_code, VM->m_CurCode+2, DispSize, true);

		if (II.m_BaseCode[1] == 0x20)
			VM->IP = *(ushort *)EA;
		else if (II.m_BaseCode[1] == 0x28) {
			VM->IP = *(ushort *)EA;
			VM->CS = *(ushort *)(EA+2);
		}
		puts("Jump is token");
		break;

	case 0xE9:	VM->IP += (*(ushort *)(VM->m_CurCode+1) + II.m_Size);	break;
	case 0xEB:	VM->IP += (*(char *)(VM->m_CurCode+1) + II.m_Size);	break;
	case 0xEA:
		VM->IP = *(ushort *)(VM->m_CurCode+1);
		VM->CS = *(ushort *)(VM->m_CurCode+3);
		puts("Jump is token");
		break;

	case 0xE3:
		VM->IP += II.m_Size;
		if (VM->C.X == 0) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x74:
		VM->IP += II.m_Size;
		if (VM->ZF == 1) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x75:
		VM->IP += II.m_Size;
		if (VM->ZF == 0) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x7C:
		VM->IP += II.m_Size;
		if (VM->SF != VM->OF) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x7E:
		VM->IP += II.m_Size;
		if ((VM->SF != VM->OF) || (VM->ZF == 1)) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x7F:
		VM->IP += II.m_Size;
		if ((VM->SF == VM->OF) && (VM->ZF == 0)) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x7D:
		VM->IP += II.m_Size;
		if (VM->SF == VM->OF) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x72:
		VM->IP += II.m_Size;
		if (VM->CF == 1) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x76:
		VM->IP += II.m_Size;
		if ((VM->CF == 1) || (VM->ZF == 1)) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x77:
		VM->IP += II.m_Size;
		if ((VM->CF == 0) && (VM->ZF == 0)) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x73:
		VM->IP += II.m_Size;
		if (VM->CF == 0) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x70:
		VM->IP += II.m_Size;
		if (VM->OF == 1) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x71:
		VM->IP += II.m_Size;
		if (VM->OF == 0) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x78:
		VM->IP += II.m_Size;
		if (VM->SF == 1) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x79:
		VM->IP += II.m_Size;
		if (VM->SF == 0) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x7A:
		VM->IP += II.m_Size;
		if (VM->PF == 1) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;

	case 0x7B:
		VM->IP += II.m_Size;
		if (VM->PF == 0) {
			puts("Jump is token");
			VM->IP += *(char *)(VM->m_CurCode+1);
		}
		break;
	}

	return true;
}

bool K8086VM::Execute_LOOP_LOOPE_LOOPNE(K8086VM *VM, KInstructionID &II)
{
	VM->C.X --;

	switch (II.m_BaseCode[0]) {
	case 0xE2:	if (VM->C.X != 0) VM->IP += *(char *)(VM->m_CurCode+1);	break;
	case 0xE1:	if ((VM->C.X != 0) && (VM->ZF == 1)) VM->IP += *(char *)(VM->m_CurCode+1);	break;
	case 0xE0:	if ((VM->C.X != 0) && (VM->ZF == 0)) VM->IP += *(char *)(VM->m_CurCode+1);	break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_CLC_CMC_STC_CLD_STD_CLI_STI_HLT(K8086VM *VM, KInstructionID &II)
{
	switch (II.m_BaseCode[0]) {
	case 0xF8:	VM->CF = 0;	break;
	case 0xF5:	VM->CF = !VM->CF;	break;
	case 0xF9:	VM->CF = 1;	break;
	case 0xFC:	VM->DF = 0;	break;
	case 0xFD:	VM->DF = 1;	break;
	case 0xFA:	VM->IF = 0;	break;
	case 0xFB:	VM->IF = 1;	break;
	case 0xF4:	VM->m_Terminated = true;	break;
	}

	VM->IP += II.m_Size;
	return true;
}

bool K8086VM::Execute_INT(K8086VM *VM, KInstructionID &II)
{
	switch (II.m_BaseCode[0]) {
	case 0xCD:
		printf("INT %02Xh / AH = %02Xh\n", (int)VM->m_CurCode[1], (int)VM->A.R.H);

		switch (VM->m_CurCode[1]) {
		case 0x10:	//int 10h
			switch (VM->A.R.H) {
			case 0x2:
				//Set cursor position
				if (VM->B.R.H != 0)
					throw K8086VMException(EINVAL, "INT 10h/AH=2h: Only supported if BH is zero.");
				VM->SwitchToConsole(true);
				VM->SetCursorPosition(VM->D.R.L, VM->D.R.H);
				VM->SwitchToConsole(false);
				break;

			case 0x3:
				//Get cursor position and size
				if (VM->B.R.H != 0)
					throw K8086VMException(EINVAL, "INT 10h/AH=3h: Only supported if BH is zero.");
				VM->SwitchToConsole(true);
				VM->GetCursorPosition(&VM->D.R.L, &VM->D.R.H);
				VM->SwitchToConsole(false);
				VM->C.X = 0x0607;
				break;

			case 0xA:
				//Write character at cursor position
				if (VM->B.R.H != 0)
					throw K8086VMException(EINVAL, "INT 10h/AH=0Ah: Only supported if BH is zero.");
				VM->SwitchToConsole(true);
				VM->WriteChar(VM->A.R.L, VM->C.X);
				VM->SwitchToConsole(false);
				break;

			case 0xE:
				VM->SwitchToConsole(true);
				VM->WriteChar(VM->A.R.L);
				VM->SwitchToConsole(false);
				break;

			case 0x10:
				if (VM->A.R.L == 0x03) {	//Toggle intensity/blinking
					if (VM->B.R.L != 0)
						throw K8086VMException(EINVAL, "INT 10h/AX=1003h: Only supported if BX is zero.");
				} else
					throw K8086VMException(ENOEXEC, "Unrecognized interruption.");
				break;

			default:
				throw K8086VMException(ENOEXEC, "Unrecognized interruption.");
			}
			break;

		case 0x20:	//int 20h
			//Return control to the operating system (stop program)
			VM->m_Terminated = true;
			break;

		case 0x21:	//int 21h
			switch (VM->A.R.H) {
			case 0x1:
				//Read character from standard input, with echo, result is stored in AL
				VM->SwitchToConsole(true);
				VM->A.R.L = (byte)VM->ReadChar(true);
				VM->SwitchToConsole(false);
				break;

			case 0x2:
				//Write character in DL to standard output, then AL = DL
				VM->SwitchToConsole(true);
				VM->WriteChar(VM->D.R.L);
				VM->SwitchToConsole(false);

				VM->A.R.L = VM->D.R.L;
				break;

			case 0x6:
				//Direct console input or output
				if (VM->D.R.L == 0xFF) {	//Input
					char ch = VM->ReadCharFromBuffer();
					VM->ZF = (ch == 0xFF);
					VM->A.R.L = ((ch == 0xFF) ? 0 : ch);
				} else {	//Output
					VM->SwitchToConsole(true);
					VM->WriteChar(VM->D.R.L);
					VM->SwitchToConsole(false);

					VM->A.R.L = VM->D.R.L;
				}
				break;

			case 0x7:
				//Character input without echo to AL
				VM->SwitchToConsole(true);
				VM->A.R.L = (byte)VM->ReadChar();
				VM->SwitchToConsole(false);
				break;

			case 0x9:
				//Output of a string at DS:DX. String must be terminated by '$'
				{
					char *p = (char *)(VM->m_Memory + VM->DS * 16 + VM->D.X);
					size_t i;
					for (i=0; (p[i] != '$') && (i<65536); i++);
					if (i == 65536)
						throw K8086VMException(EINVAL, "INT 21h/AH=9h: Invalid string passed in DS:DX.");

					p[i] = '\0';
					VM->SwitchToConsole(true);
					VM->WriteString(p, strlen(p));
					VM->SwitchToConsole(false);
					p[i] = '$';
					VM->A.R.L = '$';
				}
				break;

			case 0xA:
				//Input of a string to DS:DX, first byte is buffer size, second byte is
				//number of characters actually read. this function does not add '$' in
				//the end of string. To print using INT 21h/AH=9 you must set dollar
				//character at the end of it and start printing from address DS:DX + 2
				{
					byte *p = VM->m_Memory + VM->DS * 16 + VM->D.X;
					VM->SwitchToConsole(true);
					p[1] = (byte)VM->ReadString((char *)(p+2), p[0]);
					VM->A.R.L = '\r';
					VM->SwitchToConsole(false);
				}
				break;

			case 0xB:
				//Get input status
				VM->A.R.L = (VM->IsCharBufferEmpty() ? 0x00 : 0xFF);
				break;

			case 0x4C:
				//Return control to the operating system (stop program)
				VM->m_Terminated = true;
				break;

			default:
				throw K8086VMException(ENOEXEC, "Unrecognized interruption.");
			}
			break;

		default:
			throw K8086VMException(ENOEXEC, "Unrecognized interruption.");
		}
		break;

	case 0xCC:
		throw K8086VMException(ENOEXEC, "Unrecognized interruption.");

	default:
		throw K8086VMException(ENOEXEC, "Unrecognized interruption.");
	}

	VM->IP += II.m_Size;
	return true;
}
