
#include "i8086sim.h"


#define DOS_EXE_LOAD_BASE	0x710

struct DOS_EXE
{
	/*
		NOTES:
		All multi-byte values are stored LSB first.
		
		One block is 512 bytes. One paragraph is 16 bytes.

		After the header, there follow the relocation items, which are used to span
		multiple segments

		The raw binary code starts on a paragraph boundary within the executable file, it
		is calculated: ExeCodeStart = ExeData + Header.SizeInParagraphs * 16;

		The offset of the byte just after the EXE data is:
		ExtraDataStart = Header.BlocksInFile * 512;
		if (Header.SizeOfLastPage) ExtraDataStart -= (512 - Header.SizeOfLastPage);

		All segments are relative to the start of the executable in memory, and this
		value must be added to every segment if relocation is done manually.

		To get the position of the relocation within the file, you calculate the
		physical address from the segment:offset pair, which is done by multiplying the
		segment by 16 and adding the offset and then adding the offset of the binary start.
		RelocationPosition = RelocSegment * 16 + RelocOffset + BinaryStartOffset;
	*/

	struct ParagraphsBounds {
		/*
			Number of paragraphs of additional memory that the program will need.
			This is the equivalent of the BSS size in a Unix program. The program can't be
			loaded if there isn't at least this much memory available to it.
		*/
		ushort	m_Minimum;

		/*
			Maximum number of paragraphs of additional memory. Normally, the OS reserves
			all the remaining conventional memory for your program, but you can limit it
			with this field.
		*/
		ushort	m_Maximum;
	};

	struct Header {
		ushort	m_Signature;			//"MZ" or 0x5A4D

		/*
			The number of bytes in the last block of the program that are actually used.
			If this value is zero, that means the entire last block is used, meaning that
			the effective value is 512.
		*/
		ushort	m_SizeOfLastPage;

		/*
			Number of blocks in the file that are part of the EXE file.
			If m_SizeOfLastPage is non-zero, only that much of the last block is used.
		*/
		ushort	m_PagesCount;

		//Number of relocation entries stored after the header. May be zero.
		ushort	m_RelocationEntriesCount;

		/*
			Number of paragraphs in the header. The program's data begins just after the header,
			and this field can be used to calculate the appropriate file offset.
		*/
		ushort	m_SizeInParagraphs;	//Header size in paragraphs

		ParagraphsBounds m_AdditionalParagraphs;
		ushort	m_InitialSS;	//Relative to start of executable
		ushort	m_InitialSP;
		ushort	m_Checksum;
		ushort	m_EntryPointIP;
		ushort	m_EntryPointCS;	//Relative to start of executable
		ushort	m_RelocationTableOffset;
		ushort	m_OverlayNumber;	//0x0 = main program
	};

	struct RelocationEntry {
		ushort	m_Offset;	//Within segment
		ushort	m_Segment;
	};
};

bool K8086VM_EXE::LoadFile(char *FileName)
{
	if (_stricmp(FileName + strlen(FileName) - 4, ".exe") != 0) {
		errno = EINVAL;
		LogErr("MS-DOS EXE executable must have .EXE file extension to be executed.");
		return false;
	}

	if (!K8086VM::LoadFile(FileName))
		return false;

	DOS_EXE::Header *pHdr = (DOS_EXE::Header *)m_ProgramData;
	if (pHdr->m_Signature != 0x5A4D) {
		free(m_ProgramData);
		m_ProgramSize = 0;
		errno = EBADF;
		LogErr("EXE file is not a normal MSDOS executable. File may be corrupt.");
		return false;
	}
	return true;
}

bool K8086VM_EXE::Execute(KExecFlags Flags)
{
	if (!m_ProgramSize) {LogErr("No program is loaded."); errno = ENOENT; return false;}
	if (!m_Memory) {LogErr("Not enough memory."); errno = ENOMEM; return false;}

	DOS_EXE::Header *pHdr = (DOS_EXE::Header *)m_ProgramData;
	DOS_EXE::RelocationEntry *pRelocationTable = (DOS_EXE::RelocationEntry *)(m_ProgramData + pHdr->m_RelocationTableOffset);
	byte *pCodeBase = m_ProgramData + (pHdr->m_SizeInParagraphs * 16);

	for (ushort i=0; i<pHdr->m_RelocationEntriesCount; i++)
		*(ushort *)(pCodeBase + (pRelocationTable[i].m_Segment*16 + pRelocationTable[i].m_Offset)) += DOS_EXE_LOAD_BASE;

	puts("Program relocations fixed.");

	this->ResetRegisters();

	C.X = (ushort)((pHdr->m_PagesCount - 1) * 512 + (!pHdr->m_SizeOfLastPage ? 512 : pHdr->m_SizeOfLastPage) - pHdr->m_SizeInParagraphs * 16);
	CS = pHdr->m_EntryPointCS + DOS_EXE_LOAD_BASE;
	IP = pHdr->m_EntryPointIP;
	SP = pHdr->m_InitialSP;
	SS = pHdr->m_InitialSS + DOS_EXE_LOAD_BASE;

	puts("Registers initialized.");

	memmove(m_Memory + DOS_EXE_LOAD_BASE*16, pCodeBase, C.X);
	printf("Program loaded into memory at [%04X:%04X].\n", CS, IP);

	return this->K8086VM::Execute(Flags);
}
