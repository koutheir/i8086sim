
#include "i8086sim.h"


#define MaxRegDumpLen	1024


K8086VM::K8086VM() :
	m_ProgramSize(), m_ProgramData(), m_CurCode(), m_Memory()
{
	m_Dump = (char *)malloc(MaxRegDumpLen);
	memset(m_Dump, 0, MaxRegDumpLen);

	m_Memory = (byte *)calloc(0x100000, 1);
}

K8086VM::~K8086VM()
{
	if (m_ProgramData != NULL) free(m_ProgramData);
	if (m_Memory != NULL) free(m_Memory);
	if (m_Dump != NULL) free(m_Dump);
}

void K8086VM::ResetRegisters()
{
	A.X = B.X = D.X = SI = DI = BP = DS = SS = CS = ES = IP = 0;
	SP = 0xFFFE;
	CF = OF = AF = PF = ZF = SF = TF = DF = 0;
	IF = 1;
}

bool K8086VM::LoadFile(char *FileName)
{
	printf("Loading file: %s ...\n", FileName);

	int fd = _open(FileName, _O_RDONLY|_O_BINARY);
	if (fd == -1) {LogErr("Cannot open file."); return false;}

	struct _stat st;
	int err = _fstat(fd, &st);
	if (err != -1) {
		if (st.st_size <= 0x100000) {
			m_ProgramSize = st.st_size;
			m_ProgramData = (byte *)malloc(st.st_size);
			if (!m_ProgramData) {
				LogErr("Not enough memory.");
				err = -1;
			} else {
				err = _read(fd, m_ProgramData, st.st_size);
				if (err == -1)
					LogErr("Cannot read file data.");
			}
		} else {
			errno = EFBIG;
			LogErr("Program too big.");
			err = -1;
		}
	} else
		LogErr("Cannot get file informations.");
	_close(fd);

	if (err == -1) {
		m_ProgramSize = 0;
		m_ProgramData = NULL;
	} else {
		m_ProgramSize = err;
		puts("File loaded.");
	}
	return (err != -1);
}

bool K8086VM::GetDataAt(int SegmentBase, int Offset, byte BytesCount, short *Data)
{
	uint PhysicalAddress = 16*SegmentBase + Offset;
	if ((PhysicalAddress+BytesCount) >= 0x100000) {
		errno = EFAULT;
		LogErr("Physical address out of addressable memory.");
		return false;
	}
	if (BytesCount == 1) {
		char c = *(char *)(m_Memory + PhysicalAddress);
		*Data = (short)c;
	} else if (BytesCount == 2)
		*Data = *(short *)(m_Memory + PhysicalAddress);
	else
		return false;

	if (BytesCount == 2)
		printf("[%04X:%04X] = %04Xh.\n", SegmentBase, Offset, (int)(*Data));
	else
		printf("[%04X:%04X] = %02Xh.\n", SegmentBase, Offset, (int)(*Data));
	return true;
}

bool K8086VM::SetDataAt(int SegmentBase, int Offset, byte BytesCount, short NewData)
{
	uint PhysicalAddress = 16*SegmentBase + Offset;
	if ((PhysicalAddress+BytesCount) >= 0x100000) {
		errno = EFAULT;
		LogErr("Physical address out of addressable memory.");
		return false;
	}
	int nOldData;
	if (BytesCount == 1) {
		char c = (char)NewData;
		nOldData = (int)*(char *)(m_Memory + PhysicalAddress);
		*(char *)(m_Memory + PhysicalAddress) = c;
	} else if (BytesCount == 2) {
		nOldData = (int)*(short *)(m_Memory + PhysicalAddress);
		*(short *)(m_Memory + PhysicalAddress) = (short)NewData;
	} else
		return false;

	if (BytesCount == 2)
		printf("[%04X:%04X] = %04Xh;\tOld value = %04Xh.\n", SegmentBase, Offset, (int)NewData, nOldData);
	else
		printf("[%04X:%04X] = %02Xh;\tOld value = %02Xh.\n", SegmentBase, Offset, (int)NewData, nOldData);
	return true;
}

/*
	16-Bit (w = 1)	8-Bit (w = 0)	Segment
	000 AX			000 AL			00 ES
	001 CX			001 CL			01 CS
	010 DX			010 DL			10 SS
	011 BX			011 BL			11 DS
	100 SP			100 AH
	101 BP			101 CH
	110 SI			110 DH
	111 DI			111 BH
*/

byte *K8086VM::GetRegister(byte RegCode, bool WordData, bool SegmentReg)
{
#define CASE_REG_OP(case_n, regname, regvarname)	\
	case case_n:	\
		printf(regname " was %02Xh.\n", (int)regvarname);	\
		return &regvarname

#define CASE_REG_OP_2(case_n, regname, regvarname)	\
	case case_n:	\
		printf(regname " was %04Xh.\n", (int)regvarname);	\
		return (byte *)&regvarname

	if (!WordData) {
		switch (RegCode) {
		CASE_REG_OP(0, "AL", A.R.L);
		CASE_REG_OP(1, "CL", C.R.L);
		CASE_REG_OP(2, "DL", D.R.L);
		CASE_REG_OP(3, "BL", B.R.L);
		CASE_REG_OP(4, "AH", A.R.H);
		CASE_REG_OP(5, "CH", C.R.H);
		CASE_REG_OP(6, "DH", D.R.H);
		CASE_REG_OP(7, "BH", B.R.H);
		default:
			errno = ENOEXEC;
			LogErr("Unrecognized byte register code.");
			return NULL;
		}
	} else if (!SegmentReg) {
		switch (RegCode) {
		CASE_REG_OP_2(0, "AX", A.X);
		CASE_REG_OP_2(1, "CX", C.X);
		CASE_REG_OP_2(2, "DX", D.X);
		CASE_REG_OP_2(3, "BX", B.X);
		CASE_REG_OP_2(4, "SP", SP);
		CASE_REG_OP_2(5, "BP", BP);
		CASE_REG_OP_2(6, "SI", SI);
		CASE_REG_OP_2(7, "DI", DI);
		default:
			errno = ENOEXEC;
			LogErr("Unrecognized word register code.");
			return NULL;
		}
	} else {
		switch (RegCode) {
		CASE_REG_OP_2(0, "ES", ES);
		CASE_REG_OP_2(1, "CS", CS);
		CASE_REG_OP_2(2, "SS", SS);
		CASE_REG_OP_2(3, "DS", DS);
		default:
			errno = ENOEXEC;
			LogErr("Unrecognized segment register code.");
			return NULL;
		}
	}
}

/*
	if mod = 11 then r/m is treated as a REG field.

	if mod = 00 then DISP=0*, disp-low and disp-high are absent.
	if mod = 01 then DISP = disp-low sign-extended to 16 bits, disp-high is absent.
	if mod = 10 then DISP = disp-high;disp-low

	if r/m = 000 then EA = (BX) + (SI) + DISP
	if r/m = 001 then EA = (BX) + (DI) + DISP
	if r/m = 010 then EA = (BP) + (SI) + DISP
	if r/m = 011 then EA = (BP) + (DI) + DISP
	if r/m = 100 then EA = (SI) + DISP
	if r/m = 101 then EA = (DI) + DISP
	if r/m = 110 then EA = (BP) + DISP*
	if r/m = 111 then EA = (BX) + DISP

	DISP follows 2nd byte of instruction (before data if required).
	* Except if mod = 00 and r/m = 110 then EA = disp-high;disp-low.
*/
byte *K8086VM::GetEffectiveAddress(byte MemMode, byte MemCode, byte *OperandData, byte &DispSize, bool WordData)
{
	short DISP=0;

	switch (MemMode) {
	case 2:
		DISP = *(short *)OperandData;
		DispSize=2;
		break;

	case 1:
		DISP = (short)(*(char *)OperandData);
		DispSize=1;
		break;

	case 0:
		if (MemCode == 6) {
			DISP = *(short *)OperandData;
			DispSize = 2;
		} else {
			DISP = 0;
			DispSize = 0;
		}
		break;

	case 3:
		DispSize = 0;
		return (byte *)this->GetRegister(MemCode, WordData, false);
		break;

	default:
		errno = ENOEXEC;
		LogErr("Invalid memory addressing mode.");
		return 0;
	};
	
	//DS segment register is used for all modes except those with BP register,
	//for these SS segment register is used.
#define DISPLAY_ACCESS_3(segment, offsetvar, index)	{	\
		if (WordData)	\
			printf("[%04X:%04X] was %04Xh.\n", segment, offsetvar+index+DISP, (int)*(ushort *)(m_Memory + EA));	\
		else \
			printf("[%04X:%04X] was %02Xh.\n", segment, offsetvar+index+DISP, (int)*(byte *)(m_Memory + EA));	\
	}

	uint EA=0;
	switch (MemCode) {
	case 0:
		EA = DS * 0x10 + B.X + SI + DISP;
		DISPLAY_ACCESS_3(DS, B.X, SI);
		break;

	case 1:
		EA = DS * 0x10 + B.X + DI + DISP;
		DISPLAY_ACCESS_3(DS, B.X, DI);
		break;

	case 2:
		EA = SS * 0x10 + BP + SI + DISP;
		DISPLAY_ACCESS_3(SS, BP, SI);
		break;

	case 3:
		EA = SS * 0x10 + BP + DI + DISP;
		DISPLAY_ACCESS_3(SS, BP, DI);
		break;

	case 4:
		EA = DS * 0x10 + SI + DISP;
		DISPLAY_ACCESS_3(DS, 0, SI);
		break;

	case 5:
		EA = DS * 0x10 + DI + DISP;
		DISPLAY_ACCESS_3(DS, 0, DI);
		break;

	case 6:
		EA = DISP + ((MemMode == 0) ? (DS * 0x10) : (SS * 0x10 + BP));
		if (MemMode == 0) {
			DISPLAY_ACCESS_3(DS, 0, 0);
		} else {
			DISPLAY_ACCESS_3(SS, BP, 0);
		}
		break;

	case 7:
		EA = DS * 0x10 + B.X + DISP;
		DISPLAY_ACCESS_3(DS, B.X, 0);
		break;

	default:
		errno = ENOEXEC;
		LogErr("Unrecognized memory addressing formula.");
		return 0;
	}

	return (m_Memory + EA);
}

const char *K8086VM::GetRegistersDump()
{
	_snprintf_s(m_Dump, MaxRegDumpLen, _TRUNCATE,
		"   AX=%04Xh  BX=%04Xh  CX=%04Xh  DX=%04Xh  SI=%04Xh  DI=%04Xh\n"
		"   SP=%04Xh  BP=%04Xh  DS=%04Xh  SS=%04Xh  CS=%04Xh  ES=%04Xh  IP=%04Xh\n"
		"   Flags: C=%u  Z=%u  S=%u  O=%u  P=%u  A=%u  I=%u  D=%u  T=%u\n",
		(uint)A.X, (uint)B.X, (uint)C.X, (uint)D.X,
		(uint)SI, (uint)DI, (uint)SP, (uint)BP,
		(uint)DS, (uint)SS, (uint)CS, (uint)ES,
		(uint)IP,
		(uint)CF, (uint)ZF, (uint)SF, (uint)OF, (uint)PF, (uint)AF, (uint)IF, (uint)DF, (uint)TF);

	return m_Dump;
}

const char *K8086VM::GetMemoryDump(uint PhysicalAddress)
{
	int count=0, temp;
	uint p=PhysicalAddress;

	for (int i=0; i<8; i++) {
		temp = _snprintf_s(m_Dump+count, MaxRegDumpLen-count, _TRUNCATE, 
			"%6xh: %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X\n",
			p,
			(uint)m_Memory[p  ],	(uint)m_Memory[p+1], 
			(uint)m_Memory[p+2],	(uint)m_Memory[p+3], 
			(uint)m_Memory[p+4],	(uint)m_Memory[p+5], 
			(uint)m_Memory[p+6],	(uint)m_Memory[p+7], 
			(uint)m_Memory[p+8],	(uint)m_Memory[p+9], 
			(uint)m_Memory[p+10], (uint)m_Memory[p+11], 
			(uint)m_Memory[p+12], (uint)m_Memory[p+13], 
			(uint)m_Memory[p+14], (uint)m_Memory[p+15]);
		if (temp == -1) break;
		
		count += temp;
		p += 16;
	}

	return m_Dump;
}

/*
Bit set:	11   10   9    8    7    6    5    4   3    2   1    0
	     [OF] [DF] [IF] [TF] [SF] [ZF] [0] [AF] [0] [PF] [1] [CF]
*/
void K8086VM::SetFlagsFromBitSet(byte FlagsBitSet)
{
	CF = (!(FlagsBitSet & 0x1) ? 0 : 1);
	PF = (!(FlagsBitSet & 0x4) ? 0 : 1);
	AF = (!(FlagsBitSet & 0x10) ? 0 : 1);
	ZF = (!(FlagsBitSet & 0x40) ? 0 : 1);
	SF = (!(FlagsBitSet & 0x80) ? 0 : 1);
}

void K8086VM::SetFlagsFromBitSet(ushort FlagsBitSet)
{
	TF = (!(FlagsBitSet & 0x100) ? 0 : 1);
	IF = (!(FlagsBitSet & 0x200) ? 0 : 1);
	DF = (!(FlagsBitSet & 0x400) ? 0 : 1);
	OF = (!(FlagsBitSet & 0x800) ? 0 : 1);

	this->SetFlagsFromBitSet((byte)FlagsBitSet);
}

ushort K8086VM::GetFlagsIntoBitSet(byte _CF, byte _PF, byte _AF, byte _ZF, byte _SF, byte _TF, byte _IF, byte _DF, byte _OF)
{
	return (ushort)(0x2 | _CF | (_PF << 2) | (_AF << 4) | (_ZF << 6) | (_SF << 7) | (_TF << 8) | (_IF << 9) | (_DF << 10) | (_OF << 11));
}

ushort K8086VM::GetFlagsIntoBitSet()
{
	return (ushort)(0x2 | CF | (PF << 2) | (AF << 4) | (ZF << 6) | (SF << 7) | (TF << 8) | (IF << 9) | (DF << 10) | (OF << 11));
}

bool K8086VM::Execute(KExecFlags Flags)
{
	if (!m_ProgramSize) {LogErr("No program is loaded."); errno = ENOENT; return false;}
	if (!m_Memory) {LogErr("Not enough memory."); errno = ENOMEM; return false;}

	//Create a new console for the program I/O
	if (!this->CreateConsole()) {
		LogErr("Failed to create a virtual machine screen console.");
		return false;
	}
	puts("Virtual machine screen console created.");

	m_Terminated = false;
	do {
		if (this->ExecuteNext())
			puts(this->GetRegistersDump());

		if (Flags & KEF_StepByStep) {
			BringWindowToTop(m_SimConsole.m_hWnd);
			SetForegroundWindow(m_SimConsole.m_hWnd);

			this->ReadChar();
		}
	} while (!m_Terminated);

	BringWindowToTop(m_SimConsole.m_hWnd);
	SetForegroundWindow(m_SimConsole.m_hWnd);
	puts("Execution done.");
	return true;
}

bool K8086VM::ExecuteNext()
{
	m_CurCode = m_Memory + 16*CS + IP;

	//Search the instruction
	uint i,j;
	for (i=0; i<InsIDCount; i++) {
		if (InsID[i].m_Size > (uint)(0x100000 - (16*CS+IP))) continue;

		if (InsID[i].m_BaseCode[0] != (m_CurCode[0] & InsID[i].m_ValidityMask[0])) continue;
		if (InsID[i].m_Size >= 2)
			if (InsID[i].m_BaseCode[1] != (m_CurCode[1] & InsID[i].m_ValidityMask[1])) continue;

		break;
	}
	if (i == InsIDCount) {
		errno = ENOEXEC;
		LogErr("Unrecognized processor instruction.");
		m_Terminated = true;
		return false;
	} else {
		//Find the executer function
		for (j=i+1; InsID[j].m_Description != NULL; j++);
		
		try {
			this->CallExecuter(InsID[i], InsID[j]);
		} catch(K8086VMException &e) {
			e.Display();
			m_Terminated = true;
			return false;
		}
		return true;
	}
}

void K8086VM::CallExecuter(KInstructionID &ins, KInstructionID &exec)
{
	puts(ins.m_Description);

	__try {
		exec.m_ExecuteProc(this, ins);	//Execute the instruction
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		DWORD nErr = GetExceptionCode();
		switch (nErr)
		{
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			throw K8086VMException(ENOEXEC, "Integer divide by zero exception.");	break;
		case EXCEPTION_INT_OVERFLOW:
			throw K8086VMException(ENOEXEC, "Integer overflow exception.");	break;
		case EXCEPTION_PRIV_INSTRUCTION:
			throw K8086VMException(ENOEXEC, "Privileged instruction exception.");	break;
		default:
			throw K8086VMException(ENOEXEC, "Unknown exception.");
		}
	}
}

bool K8086VM::CreateVMLoader(K8086VM **pLdr, char *FileName)
{
	if (_stricmp(FileName + strlen(FileName) - 4, ".exe") == 0)
		*pLdr = new K8086VM_EXE;
	else if (_stricmp(FileName + strlen(FileName) - 4, ".com") == 0)
		*pLdr = new K8086VM_COM;
	else
		*pLdr = NULL;
	return (*pLdr != NULL);
}
