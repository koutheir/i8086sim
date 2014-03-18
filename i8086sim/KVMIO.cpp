
#include "i8086sim.h"


struct EnhKeyVals {
	ushort ScanCode;
	byte RegChars[2];
	byte ShiftChars[2];
	byte CtrlChars[2];
	byte AltChars[2];
};

struct NormKeyVals {
	byte RegChars[2];
	byte ShiftChars[2];
	byte CtrlChars[2];
	byte AltChars[2];
};


//Table of key values for enhanced keys
const static EnhKeyVals s_EnhancedKeys[] = {
  { 28, {  13,   0 }, {  13,   0 }, {  10,   0 }, {   0, 166 } },
  { 53, {  47,   0 }, {  63,   0 }, {   0, 149 }, {   0, 164 } },
  { 71, { 224,  71 }, { 224,  71 }, { 224, 119 }, {   0, 151 } },
  { 72, { 224,  72 }, { 224,  72 }, { 224, 141 }, {   0, 152 } },
  { 73, { 224,  73 }, { 224,  73 }, { 224, 134 }, {   0, 153 } },
  { 75, { 224,  75 }, { 224,  75 }, { 224, 115 }, {   0, 155 } },
  { 77, { 224,  77 }, { 224,  77 }, { 224, 116 }, {   0, 157 } },
  { 79, { 224,  79 }, { 224,  79 }, { 224, 117 }, {   0, 159 } },
  { 80, { 224,  80 }, { 224,  80 }, { 224, 145 }, {   0, 160 } },
  { 81, { 224,  81 }, { 224,  81 }, { 224, 118 }, {   0, 161 } },
  { 82, { 224,  82 }, { 224,  82 }, { 224, 146 }, {   0, 162 } },
  { 83, { 224,  83 }, { 224,  83 }, { 224, 147 }, {   0, 163 } }
};

//Table of key values for normal keys. Note that the table is padded so
//that the key scan code serves as an index into the table
const static NormKeyVals s_NormalKeys[] = {
  /* padding */
  { /*  0 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /*  1 */ {  27,   0 }, {  27,   0 }, {  27,   0 }, {   0,   1 } },
  { /*  2 */ {  49,   0 }, {  33,   0 }, {   0,   0 }, {   0, 120 } },
  { /*  3 */ {  50,   0 }, {  64,   0 }, {   0,   3 }, {   0, 121 } },
  { /*  4 */ {  51,   0 }, {  35,   0 }, {   0,   0 }, {   0, 122 } },
  { /*  5 */ {  52,   0 }, {  36,   0 }, {   0,   0 }, {   0, 123 } },
  { /*  6 */ {  53,   0 }, {  37,   0 }, {   0,   0 }, {   0, 124 } },
  { /*  7 */ {  54,   0 }, {  94,   0 }, {  30,   0 }, {   0, 125 } },
  { /*  8 */ {  55,   0 }, {  38,   0 }, {   0,   0 }, {   0, 126 } },
  { /*  9 */ {  56,   0 }, {  42,   0 }, {   0,   0 }, {   0, 127 } },
  { /* 10 */ {  57,   0 }, {  40,   0 }, {   0,   0 }, {   0, 128 } },
  { /* 11 */ {  48,   0 }, {  41,   0 }, {   0,   0 }, {   0, 129 } },
  { /* 12 */ {  45,   0 }, {  95,   0 }, {  31,   0 }, {   0, 130 } },
  { /* 13 */ {  61,   0 }, {  43,   0 }, {   0,   0 }, {   0, 131 } },
  { /* 14 */ {   8,   0 }, {   8,   0 }, { 127,   0 }, {   0,  14 } },
  { /* 15 */ {   9,   0 }, {   0,  15 }, {   0, 148 }, {   0,  15 } },
  { /* 16 */ { 113,   0 }, {  81,   0 }, {  17,   0 }, {   0,  16 } },
  { /* 17 */ { 119,   0 }, {  87,   0 }, {  23,   0 }, {   0,  17 } },
  { /* 18 */ { 101,   0 }, {  69,   0 }, {   5,   0 }, {   0,  18 } },
  { /* 19 */ { 114,   0 }, {  82,   0 }, {  18,   0 }, {   0,  19 } },
  { /* 20 */ { 116,   0 }, {  84,   0 }, {  20,   0 }, {   0,  20 } },
  { /* 21 */ { 121,   0 }, {  89,   0 }, {  25,   0 }, {   0,  21 } },
  { /* 22 */ { 117,   0 }, {  85,   0 }, {  21,   0 }, {   0,  22 } },
  { /* 23 */ { 105,   0 }, {  73,   0 }, {   9,   0 }, {   0,  23 } },
  { /* 24 */ { 111,   0 }, {  79,   0 }, {  15,   0 }, {   0,  24 } },
  { /* 25 */ { 112,   0 }, {  80,   0 }, {  16,   0 }, {   0,  25 } },
  { /* 26 */ {  91,   0 }, { 123,   0 }, {  27,   0 }, {   0,  26 } },
  { /* 27 */ {  93,   0 }, { 125,   0 }, {  29,   0 }, {   0,  27 } },
  { /* 28 */ {  13,   0 }, {  13,   0 }, {  10,   0 }, {   0,  28 } },

  /* padding */
  { /* 29 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 30 */ {  97,   0 }, {  65,   0 }, {   1,   0 }, {   0,  30 } },
  { /* 31 */ { 115,   0 }, {  83,   0 }, {  19,   0 }, {   0,  31 } },
  { /* 32 */ { 100,   0 }, {  68,   0 }, {   4,   0 }, {   0,  32 } },
  { /* 33 */ { 102,   0 }, {  70,   0 }, {   6,   0 }, {   0,  33 } },
  { /* 34 */ { 103,   0 }, {  71,   0 }, {   7,   0 }, {   0,  34 } },
  { /* 35 */ { 104,   0 }, {  72,   0 }, {   8,   0 }, {   0,  35 } },
  { /* 36 */ { 106,   0 }, {  74,   0 }, {  10,   0 }, {   0,  36 } },
  { /* 37 */ { 107,   0 }, {  75,   0 }, {  11,   0 }, {   0,  37 } },
  { /* 38 */ { 108,   0 }, {  76,   0 }, {  12,   0 }, {   0,  38 } },
  { /* 39 */ {  59,   0 }, {  58,   0 }, {   0,   0 }, {   0,  39 } },
  { /* 40 */ {  39,   0 }, {  34,   0 }, {   0,   0 }, {   0,  40 } },
  { /* 41 */ {  96,   0 }, { 126,   0 }, {   0,   0 }, {   0,  41 } },

  /* padding */
  { /* 42 */ {    0,  0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 43 */ {  92,   0 }, { 124,   0 }, {  28,   0 }, {   0,   0 } },
  { /* 44 */ { 122,   0 }, {  90,   0 }, {  26,   0 }, {   0,  44 } },
  { /* 45 */ { 120,   0 }, {  88,   0 }, {  24,   0 }, {   0,  45 } },
  { /* 46 */ {  99,   0 }, {  67,   0 }, {   3,   0 }, {   0,  46 } },
  { /* 47 */ { 118,   0 }, {  86,   0 }, {  22,   0 }, {   0,  47 } },
  { /* 48 */ {  98,   0 }, {  66,   0 }, {   2,   0 }, {   0,  48 } },
  { /* 49 */ { 110,   0 }, {  78,   0 }, {  14,   0 }, {   0,  49 } },
  { /* 50 */ { 109,   0 }, {  77,   0 }, {  13,   0 }, {   0,  50 } },
  { /* 51 */ {  44,   0 }, {  60,   0 }, {   0,   0 }, {   0,  51 } },
  { /* 52 */ {  46,   0 }, {  62,   0 }, {   0,   0 }, {   0,  52 } },
  { /* 53 */ {  47,   0 }, {  63,   0 }, {   0,   0 }, {   0,  53 } },

  /* padding */
  { /* 54 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 55 */ {  42,   0 }, {   0,   0 }, { 114,   0 }, {   0,   0 } },

  /* padding */
  { /* 56 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 57 */ {  32,   0 }, {  32,   0 }, {  32,   0 }, {  32,   0 } },

  /* padding */
  { /* 58 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 59 */ {   0,  59 }, {   0,  84 }, {   0,  94 }, {   0, 104 } },
  { /* 60 */ {   0,  60 }, {   0,  85 }, {   0,  95 }, {   0, 105 } },
  { /* 61 */ {   0,  61 }, {   0,  86 }, {   0,  96 }, {   0, 106 } },
  { /* 62 */ {   0,  62 }, {   0,  87 }, {   0,  97 }, {   0, 107 } },
  { /* 63 */ {   0,  63 }, {   0,  88 }, {   0,  98 }, {   0, 108 } },
  { /* 64 */ {   0,  64 }, {   0,  89 }, {   0,  99 }, {   0, 109 } },
  { /* 65 */ {   0,  65 }, {   0,  90 }, {   0, 100 }, {   0, 110 } },
  { /* 66 */ {   0,  66 }, {   0,  91 }, {   0, 101 }, {   0, 111 } },
  { /* 67 */ {   0,  67 }, {   0,  92 }, {   0, 102 }, {   0, 112 } },
  { /* 68 */ {   0,  68 }, {   0,  93 }, {   0, 103 }, {   0, 113 } },

  /* padding */
  { /* 69 */ {    0,  0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },
  { /* 70 */ {    0,  0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 71 */ {   0,  71 }, {  55,   0 }, {   0, 119 }, {   0,   0 } },
  { /* 72 */ {   0,  72 }, {  56,   0 }, {   0, 141 }, {   0,   0 } },
  { /* 73 */ {   0,  73 }, {  57,   0 }, {   0, 132 }, {   0,   0 } },
  { /* 74 */ {   0,   0 }, {  45,   0 }, {   0,   0 }, {   0,   0 } },
  { /* 75 */ {   0,  75 }, {  52,   0 }, {   0, 115 }, {   0,   0 } },
  { /* 76 */ {   0,   0 }, {  53,   0 }, {   0,   0 }, {   0,   0 } },
  { /* 77 */ {   0,  77 }, {  54,   0 }, {   0, 116 }, {   0,   0 } },
  { /* 78 */ {   0,   0 }, {  43,   0 }, {   0,   0 }, {   0,   0 } },
  { /* 79 */ {   0,  79 }, {  49,   0 }, {   0, 117 }, {   0,   0 } },
  { /* 80 */ {   0,  80 }, {  50,   0 }, {   0, 145 }, {   0,   0 } },
  { /* 81 */ {   0,  81 }, {  51,   0 }, {   0, 118 }, {   0,   0 } },
  { /* 82 */ {   0,  82 }, {  48,   0 }, {   0, 146 }, {   0,   0 } },
  { /* 83 */ {   0,  83 }, {  46,   0 }, {   0, 147 }, {   0,   0 } },

  /* padding */
  { /* 84 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },
  { /* 85 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },
  { /* 86 */ {   0,   0 }, {   0,   0 }, {   0,   0 }, {   0,   0 } },

  { /* 87 */ { 224, 133 }, { 224, 135 }, { 224, 137 }, { 224, 139 } },
  { /* 88 */ { 224, 134 }, { 224, 136 }, { 224, 138 }, { 224, 140 } }
};

static int s_ch_buffer = EOF;


bool KVMIO::CreateConsole()
{
	m_SimConsoleAttached = true;

	//Create a new console for the program I/O
	m_SimConsole.m_hWnd = GetConsoleWindow();
	m_SimConsole.LockActive();
	FreeConsole();

	AllocConsole();

	//Set the console's input/output code page to the thread's one
	char sCodePage[6];
	GetLocaleInfo(GetThreadLocale(), LOCALE_IDEFAULTCODEPAGE, sCodePage, sizeof(sCodePage));
	uint nThreadCodePage = (uint)atol(sCodePage);
	SetConsoleCP(nThreadCodePage);
	SetConsoleOutputCP(nThreadCodePage);

	//Set the console's output buffer size to 80x25 standard size
	HANDLE hConsoleOutput = CreateFile("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	COORD nSize = {80, 25};
	SetConsoleScreenBufferSize(hConsoleOutput, nSize);
	CloseHandle(hConsoleOutput);

	uint nVMTitle = strlen(g_FileName) + 64;
	char *sVMTitle = new char[nVMTitle];
	strcpy(sVMTitle, "I8086SIM - Virtual Machine - ");
	strcat(sVMTitle, g_FileName);
	SetConsoleTitle(sVMTitle);
	delete [] sVMTitle;

	m_VMConsole.m_hWnd = GetConsoleWindow();
	m_VMConsole.LockActive();
	FreeConsole();

	if (!m_SimConsole.UnlockActive()) return false;

	BringWindowToTop(m_SimConsole.m_hWnd);
	SetForegroundWindow(m_SimConsole.m_hWnd);
	return true;
}

bool KVMIO::SwitchToConsole(bool ToVirtualMachine)
{
	if (m_SimConsoleAttached != ToVirtualMachine) return true;

	if (ToVirtualMachine) {	//Switch to the virtual machine console
		m_SimConsole.LockActive();
		FreeConsole();
		if (!m_VMConsole.UnlockActive())
			throw K8086VMException(EINVAL, "Failed to switch to virtual machine screen console.");

		BringWindowToTop(m_VMConsole.m_hWnd);
		SetForegroundWindow(m_VMConsole.m_hWnd);
	} else {	//Switch to the simulator console
		m_VMConsole.LockActive();
		FreeConsole();
		if (!m_SimConsole.UnlockActive())
			throw K8086VMException(EINVAL, "Failed to switch to simulator screen console.");
	}

	m_SimConsoleAttached = !ToVirtualMachine;
	return true;
}

const byte *KVMIO::GetExtendedKeyCode(KEY_EVENT_RECORD *pKE)
{
	DWORD CKS;			//hold dwControlKeyState value
	if ((CKS = pKE->dwControlKeyState) & ENHANCED_KEY) {
		//Find the appropriate entry in EnhancedKeys[]
		for (int i=0; i<_countof(s_EnhancedKeys); i++) {
			if (s_EnhancedKeys[i].ScanCode != pKE->wVirtualScanCode) continue;

			if (CKS & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
				return (const byte *)&(s_EnhancedKeys[i].AltChars);
			else if (CKS & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
				return (const byte *)&(s_EnhancedKeys[i].CtrlChars);
			else if (CKS & SHIFT_PRESSED)
				return (const byte *)&(s_EnhancedKeys[i].ShiftChars);
			else
				return (const byte *)&(s_EnhancedKeys[i].RegChars);
		}
	} else {
		//Regular key or a keyboard event which shouldn't be recognized.
		//Determine which by getting the proper field of the proper
		//entry in NormalKeys[], and examining the extended code.
		const byte *pCP;   //pointer to the extended code

		if (CKS & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED))
			pCP = (const byte *)&(s_NormalKeys[pKE->wVirtualScanCode].AltChars);
		else if (CKS & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
			pCP = (const byte *)&(s_NormalKeys[pKE->wVirtualScanCode].CtrlChars);
		else if (CKS & SHIFT_PRESSED)
			pCP = (const byte *)&(s_NormalKeys[pKE->wVirtualScanCode].ShiftChars);
		else
			pCP = (const byte *)&(s_NormalKeys[pKE->wVirtualScanCode].RegChars);
		
		if (((pCP[0] != 0) && (pCP[0] != 224)) || !pCP[1])
			return NULL;	//A keyboard event which should not be recognized (e.g., shift key was pressed)
		return pCP;
	}
	return NULL;
}

void KVMIO::WriteChar(char ch, size_t count)
{
	if (!count) return;

	HANDLE hConsoleOutput = CreateFile("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleOutput == INVALID_HANDLE_VALUE) return;

	DWORD nWritten;
	for (size_t i=0; i<count; i++)
		WriteFile(hConsoleOutput, &ch, 1, &nWritten, NULL);
	CloseHandle(hConsoleOutput);
}

char KVMIO::ReadChar(bool WithEcho)
{
	char ch=EOF;

	if (s_ch_buffer != EOF) {
		ch = (byte)(s_ch_buffer & 0xFF);
		s_ch_buffer = EOF;
		if (WithEcho) this->WriteChar(ch);
		return ch;
	}

	HANDLE hConsoleInput = CreateFile("CONIN$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleInput == INVALID_HANDLE_VALUE) return ch;

	DWORD nOldState;
	GetConsoleMode(hConsoleInput, &nOldState);
	SetConsoleMode(hConsoleInput, 0);	//Switch to raw mode: no line input, no echo input

	INPUT_RECORD ir;
	DWORD nEventsCount;
	for (;;) {
		if (!ReadConsoleInput(hConsoleInput, &ir, 1, &nEventsCount)) break;
		if (!nEventsCount) break;

		if ((ir.EventType != KEY_EVENT) || !ir.Event.KeyEvent.bKeyDown) continue;

		if (ir.Event.KeyEvent.uChar.AsciiChar != '\0') {
			ch = (byte)ir.Event.KeyEvent.uChar.AsciiChar;
			break;
		}

		const byte *pch = this->GetExtendedKeyCode(&ir.Event.KeyEvent);
		if (pch) {
			ch = pch[0];
			s_ch_buffer = pch[1];
			break;
		}
	}

	SetConsoleMode(hConsoleInput, nOldState);
	CloseHandle(hConsoleInput);

	if (WithEcho && (ch != 0xFF)) this->WriteChar(ch);
	return ch;
}

char KVMIO::ReadCharFromBuffer()
{
	if (s_ch_buffer != EOF) {
		char ch = (char)(byte)(s_ch_buffer & 0xFF);
		s_ch_buffer = EOF;
		return ch;
	} else
		return (char)(byte)(s_ch_buffer & 0xFF);
}

bool KVMIO::IsCharBufferEmpty()
{
	return (s_ch_buffer == EOF);
}

void KVMIO::WriteString(char *str, size_t count)
{
	if (!count) return;

	HANDLE hConsoleOutput = CreateFile("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleOutput == INVALID_HANDLE_VALUE) return;

	DWORD nWritten;
	WriteFile(hConsoleOutput, str, count, &nWritten, NULL);
	CloseHandle(hConsoleOutput);
}

size_t KVMIO::ReadString(char *str, size_t max)
{
	if (!max) return 0;

	HANDLE hConsoleInput = CreateFile("CONIN$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleInput == INVALID_HANDLE_VALUE) return 0;

	HANDLE hConsoleOutput = CreateFile("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleOutput == INVALID_HANDLE_VALUE) return 0;

	char *buffer = (char *)malloc(max);
	if (!buffer) {CloseHandle(hConsoleInput); return 0;}

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);

	DWORD nRead;
	ReadFile(hConsoleInput, buffer, max, &nRead, NULL);
	if (buffer[nRead-1] == '\n') {
		if (nRead <= 2)
			*str = '\r';
		else
			memmove(str, buffer, nRead-1);
		nRead -= 2;
	} else if (buffer[nRead-1] == '\r') {
		if (nRead <= 1)
			*str = '\r';
		else
			memmove(str, buffer, nRead);
		nRead--;
		FlushConsoleInputBuffer(hConsoleInput);
	} else {
		memmove(str, buffer, nRead);
		FlushConsoleInputBuffer(hConsoleInput);
	}

	csbi.dwCursorPosition.X += (ushort)nRead;
	if (csbi.dwCursorPosition.X > csbi.dwSize.X) {
		csbi.dwCursorPosition.X %= csbi.dwSize.X;
		csbi.dwCursorPosition.Y ++;
		if (csbi.dwCursorPosition.Y > csbi.dwSize.Y)
			csbi.dwCursorPosition.Y %= csbi.dwSize.Y;
	}
	SetConsoleCursorPosition(hConsoleOutput, csbi.dwCursorPosition);

	free(buffer);
	CloseHandle(hConsoleInput);
	CloseHandle(hConsoleOutput);
	return nRead;
}

void KVMIO::SetCursorPosition(byte x, byte y)
{
	HANDLE hConsoleOutput = CreateFile("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleOutput == INVALID_HANDLE_VALUE) return;

	COORD pos = {x,y};
	SetConsoleCursorPosition(hConsoleOutput, pos);
	CloseHandle(hConsoleOutput);
}

void KVMIO::GetCursorPosition(byte *x, byte *y)
{
	HANDLE hConsoleOutput = CreateFile("CONOUT$", GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hConsoleOutput == INVALID_HANDLE_VALUE) return;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsoleOutput, &csbi);
	CloseHandle(hConsoleOutput);

	*x = (byte)csbi.dwCursorPosition.X;
	*y = (byte)csbi.dwCursorPosition.Y;
}
