
#include "i8086sim.h"


//z is used for string primitives for comparison with ZF flag

bool K8086VM::Execute_REP(K8086VM *VM, KInstructionID &II)
{
	short MEM1, MEM2;
	byte nByteCount = (VM->m_CurCode[1] & 0x1 ? 2 : 1);
	byte bContinuationZF = (VM->m_CurCode[0] & 0x1);
	ushort i8086Flags;

	switch (VM->m_CurCode[1] & 0xFE) {
	case 0xA4:	//movs
		for (; VM->C.X != 0; VM->C.X --) {
			VM->GetDataAt(VM->DS, VM->SI, nByteCount, &MEM1);
			VM->SetDataAt(VM->ES, VM->DI, nByteCount, MEM1);
			VM->SI += (!VM->DF ? nByteCount : -nByteCount);
			VM->DI += (!VM->DF ? nByteCount : -nByteCount);
		}
		break;

	case 0xA6:	//cmps
		if (VM->C.X != 0) {
			do {
				VM->GetDataAt(VM->DS, VM->SI, nByteCount, &MEM1);
				VM->GetDataAt(VM->ES, VM->DI, nByteCount, &MEM2);

				i8086Flags = VM->GetFlagsIntoBitSet();
				__asm {
					push word ptr [i8086Flags]
					popf
					mov ax,[MEM1]
					mov dx,[MEM2]
					cmp ax,dx
					pushf
					pop word ptr [i8086Flags]
				}
				VM->SetFlagsFromBitSet(i8086Flags);

				VM->SI += (!VM->DF ? nByteCount : -nByteCount);
				VM->DI += (!VM->DF ? nByteCount : -nByteCount);

				VM->C.X --;
			} while ((VM->ZF == bContinuationZF) && (VM->C.X != 0));
		}
		break;

	case 0xAE:	//scas
		if (VM->C.X != 0) {
			do {
				if (nByteCount == 2)
					MEM1 = VM->A.X;
				else
					MEM1 = (short)VM->A.R.L;
				VM->GetDataAt(VM->ES, VM->DI, nByteCount, &MEM2);

				i8086Flags = VM->GetFlagsIntoBitSet();
				__asm {
					push word ptr [i8086Flags]
					popf
					mov ax,[MEM1]
					mov dx,[MEM2]
					cmp ax,dx
					pushf
					pop word ptr [i8086Flags]
				}
				VM->SetFlagsFromBitSet(i8086Flags);

				VM->DI += (!VM->DF ? nByteCount : -nByteCount);

				VM->C.X --;
			} while ((VM->ZF == bContinuationZF) && (VM->C.X != 0));
		}
		break;

	case 0xAC:	//lods
		for (; VM->C.X != 0; VM->C.X --) {
			VM->GetDataAt(VM->DS, VM->SI, nByteCount, &MEM1);
			if (nByteCount == 2)
				VM->A.X = MEM1;
			else
				VM->A.R.L = (byte)MEM1;
			VM->SI += (!VM->DF ? nByteCount : -nByteCount);
		}
		break;

	case 0xAA:	//stos
		for (; VM->C.X != 0; VM->C.X --) {
			if (nByteCount == 2)
				MEM1 = VM->A.X;
			else
				MEM1 = (short)VM->A.R.L;
			VM->SetDataAt(VM->ES, VM->DI, nByteCount, MEM1);
			VM->DI += (!VM->DF ? nByteCount : -nByteCount);
		}
		break;
	}

	II.m_Size = 2;
	VM->IP += II.m_Size;
	return true;
}
