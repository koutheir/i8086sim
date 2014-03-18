
#include "i8086sim.h"


/*
	The COM files are raw binary executables. A COM program can only have a size of
	less than one segment (64Kb), including code and static data since no fixups for segment
	relocation or anything else is included. In principle, a COM file is just loaded
	at Offset 100h in the segment and then executed.

	WARNING: In COM files, the executable code is directly included, without headers or
	preliminary data. The data inside the file does not allow ensuring that it is executable.
	The only possible (but not too reliable) solution is the file extension.
	This is why the KComLoader will not load the file if it does not have the .COM extension.
*/
bool K8086VM_COM::LoadFile(char *FileName)
{
	if (_stricmp(FileName + strlen(FileName) - 4, ".com") != 0) {
		errno = EINVAL;
		LogErr("MS-DOS COM executable must have .COM file extension to be executed.");
		return false;
	}

	if (!K8086VM::LoadFile(FileName))
		return false;

	if (m_ProgramSize > 65536) {
		free(m_ProgramData);
		m_ProgramSize = 0;
		errno = EFBIG;
		LogErr("Program too big.");
		return false;
	}

	return true;
}

bool K8086VM_COM::Execute(KExecFlags Flags)
{
	if (!m_ProgramSize) {LogErr("No program is loaded."); errno = ENOENT; return false;}
	if (!m_Memory) {LogErr("Not enough memory."); errno = ENOMEM; return false;}

	this->ResetRegisters();
	IP = 0x100;
	C.X = (ushort)m_ProgramSize;
	DS = ES = CS = SS = 0x700;
	puts("Registers initialized.");

	memmove(m_Memory + 16*CS + IP, m_ProgramData, m_ProgramSize);
	printf("Program loaded into memory at [%04X:%04X].\n", CS, IP);

	return this->K8086VM::Execute(Flags);
}
