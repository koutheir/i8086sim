
#pragma once


#pragma pack(push, 1)

union K16BitReg {
	ushort	X;			//Full register

	struct K16BitRegElt {
		byte	L;			//Low part
		byte	H;			//High part
	} R;
};

#pragma pack(pop)


enum KExecFlags {
	KEF_None = 0,
	KEF_StepByStep			= 0x1,
};

struct KInstructionID {
	char	*m_Description;
	byte	m_BaseCode[2];
	byte	m_ValidityMask[2];
	byte	m_Size;	//Minimum (or exact) instruction size
	bool	(_cdecl * m_ExecuteProc)(class K8086VM *, KInstructionID &);
};

struct K8086VMException
{
	int	m_no;
	char	*m_desc;

	inline K8086VMException(int err_no, char *err_desc) : m_no(err_no), m_desc(err_desc) {}
	inline void Display() {errno = m_no; LogErr(m_desc);}
};

class K8086VM :
	public KVMIO
{
protected:
	static KInstructionID	InsID[];
	static uint					InsIDCount;

	byte	*m_ProgramData, *m_Memory, *m_CurCode;
	char	*m_Dump;
	uint	m_ProgramSize;
	bool	m_Terminated;

	K16BitReg	A, B, C, D;			//General purpose registers
	ushort		SI, DI, SP, BP;	//Pointer & index registers
	ushort		DS, SS, CS, ES;	//Segment registers
	ushort		IP;					//Instruction pointer
	byte			CF, PF, AF, ZF, SF, TF, IF, DF, OF;	//Flags

public:
	K8086VM();
	virtual ~K8086VM();
	static bool CreateVMLoader(K8086VM **pLdr, char *FileName);
	virtual bool LoadFile(char *FileName);
	virtual bool Execute(KExecFlags Flags=KEF_None);

protected:
	virtual bool ExecuteNext();
	virtual void CallExecuter(KInstructionID &ins, KInstructionID &exec);

	const char *GetRegistersDump();
	const char *GetMemoryDump(uint PhysicalAddress);

	bool GetDataAt(int SegmentBase, int Offset, byte BytesCount, short *Data);
	bool SetDataAt(int SegmentBase, int Offset, byte BytesCount, short NewData);

	byte *GetRegister(byte RegCode, bool WordData, bool SegmentReg);
	byte *GetEffectiveAddress(byte MemMode, byte MemCode, byte *OperandData, byte &DispSize, bool WordData);

	void ResetRegisters();
	void SetFlagsFromBitSet(byte FlagsBitSet);
	void SetFlagsFromBitSet(ushort FlagsBitSet);
	ushort GetFlagsIntoBitSet(byte _CF, byte _PF, byte _AF, byte _ZF, byte _SF, byte _TF, byte _IF, byte _DF, byte _OF);
	ushort GetFlagsIntoBitSet();

#define DEF_EXEC_FUNC(f)	\
	static bool Execute_ ##f (K8086VM *VM, KInstructionID &II)

	DEF_EXEC_FUNC(MOV);
	DEF_EXEC_FUNC(PUSH);
	DEF_EXEC_FUNC(POP);
	DEF_EXEC_FUNC(XCHG);
	DEF_EXEC_FUNC(XLAT);
	DEF_EXEC_FUNC(LEA_LDS_LES);
	DEF_EXEC_FUNC(LAHF_SAHF);
	DEF_EXEC_FUNC(PUSHF_POPF);

	DEF_EXEC_FUNC(ADD_ADC_INC);
	DEF_EXEC_FUNC(SUB_SSB_DEC);
	DEF_EXEC_FUNC(NEG);
	DEF_EXEC_FUNC(CMP);
	DEF_EXEC_FUNC(AAA_DAA_AAS_DAS_AAM_AAD_CBW_CWD);
	DEF_EXEC_FUNC(MUL_IMUL_DIV_IDIV);

	DEF_EXEC_FUNC(NOT_SHL_SAL_SHR_SAR_ROL_ROR_RCL_RCR);
	DEF_EXEC_FUNC(TEST_AND_OR_XOR);

	DEF_EXEC_FUNC(REP);

	DEF_EXEC_FUNC(CALL_RET);
	DEF_EXEC_FUNC(JMP_JZ_JNZ_JL_JLE_JG_JGE_JB_JBE_JA_JAE_JO_JNO_JS_JNS_JPO_JPE_JCXZ);
	DEF_EXEC_FUNC(LOOP_LOOPE_LOOPNE);
	DEF_EXEC_FUNC(CLC_CMC_STC_CLD_STD_CLI_STI_HLT);
	DEF_EXEC_FUNC(INT);
};


class K8086VM_COM :
	public K8086VM
{
public:
	virtual ~K8086VM_COM() {}

	virtual bool LoadFile(char *FileName);
	virtual bool Execute(KExecFlags Flags=KEF_None);
};

class K8086VM_EXE :
	public K8086VM
{
public:
	virtual ~K8086VM_EXE() {}

	virtual bool LoadFile(char *FileName);
	virtual bool Execute(KExecFlags Flags=KEF_None);
};
