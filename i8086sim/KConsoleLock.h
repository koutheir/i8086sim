
#pragma once

class KConsoleLock
{
public:
	HWND		m_hWnd;

protected:
	HANDLE	m_TermLockerEvt, m_InitLockerEvt, m_Locker;
	DWORD		m_LockerID;
	char		*m_LockerCmd;

public:
	inline KConsoleLock() :
		m_LockerID(), m_TermLockerEvt(NULL), m_InitLockerEvt(NULL), m_Locker(INVALID_HANDLE_VALUE)
	{
		//Create the synchronization events
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		m_InitLockerEvt = CreateEvent(&sa, FALSE, FALSE, NULL);
		m_TermLockerEvt = CreateEvent(&sa, FALSE, FALSE, NULL);

		char *sThisProg, *p;
		size_t nThisProg;
		_get_pgmptr(&sThisProg);
		nThisProg = strlen(sThisProg);
		for (p=sThisProg+nThisProg; *p != '\\'; p--);
		p++;

		m_LockerCmd = new char[nThisProg+64];
		m_LockerCmd[0] = '\"';
		memmove(m_LockerCmd+1, sThisProg, p-sThisProg);
		sprintf(m_LockerCmd+1+(p-sThisProg), "I8086CL.EXE\" %ld %ld %ld", GetCurrentProcessId(), m_InitLockerEvt, m_TermLockerEvt);
	}

	virtual ~KConsoleLock()
	{
		//SetEvent(m_TermLockerEvt);	//Tell the locker to finish execution

		CloseHandle(m_Locker);
		CloseHandle(m_InitLockerEvt);
		CloseHandle(m_TermLockerEvt);

		delete [] m_LockerCmd;
	}

	/*
		This function locks the active console so that the system does not destroy it when
		the current program detaches it.
		The function starts the executable "I8086CL.EXE" to do make the lock.
		The program just attaches itself to the console of the current program
		and waits until it is requested to die, by signaling an event.
		When this function returns true, the active console is still active and is attached to
		the locker program too, so detaching it will not destory it.
	*/
	bool LockActive()
	{
		//Execute the child process
		STARTUPINFO si = {sizeof(STARTUPINFO)};
		PROCESS_INFORMATION pi;
		if (!CreateProcess(NULL, m_LockerCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
			return false;

		//Get some data about the child & wait until it initializes
		CloseHandle(pi.hThread);
		m_Locker = pi.hProcess;
		m_LockerID = pi.dwProcessId;
		return (WaitForSingleObject(m_InitLockerEvt, INFINITE) == WAIT_OBJECT_0);
	}

	/*
		This function unlocks a console locked by "I8086CL.EXE" executable.
		It attaches the console to unlock to this program and instruct the locker program
		to die. The caller program must have detached from its active console before calling.
	*/
	bool UnlockActive()
	{
		if (!AttachConsole(m_LockerID)) return false;
		m_hWnd = GetConsoleWindow();
		SetEvent(m_TermLockerEvt);	//Tell the locker to finish execution
		if (WaitForSingleObject(m_Locker, INFINITE) == WAIT_FAILED)
			return false;

		DWORD nExitCode=0;
		GetExitCodeProcess(m_Locker, &nExitCode);
		CloseHandle(m_Locker);
		m_Locker = INVALID_HANDLE_VALUE;
		return (nExitCode == ERROR_SUCCESS);
	}
};
