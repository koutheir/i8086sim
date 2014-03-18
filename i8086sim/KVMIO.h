
#pragma once


class KVMIO
{
protected:
	KConsoleLock	m_SimConsole, m_VMConsole;
	bool				m_SimConsoleAttached;

public:
	inline KVMIO() : m_SimConsoleAttached(true) {}
	virtual ~KVMIO() {}

	bool CreateConsole();
	bool SwitchToConsole(bool ToVirtualMachine);

	const byte *GetExtendedKeyCode(KEY_EVENT_RECORD *pKE);
	char ReadChar(bool WithEcho=false);
	char ReadCharFromBuffer();
	bool IsCharBufferEmpty();
	void WriteChar(char ch, size_t count=1);
	void WriteString(char *str, size_t count);
	size_t ReadString(char *str, size_t max);
	void SetCursorPosition(byte x, byte y);
	void GetCursorPosition(byte *x, byte *y);
};
