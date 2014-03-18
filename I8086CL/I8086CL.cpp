
#include <windows.h>


BOOL APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	int argc;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (argc < 4) return ERROR_INVALID_PARAMETER;

	DWORD nErr=ERROR_SUCCESS, nParentID = (DWORD)_wtol(argv[1]);
	HANDLE hList[2] = {
		OpenProcess(SYNCHRONIZE, FALSE, nParentID),
		(HANDLE)_wtol(argv[3])
	};
	if (!hList[0]) return GetLastError();

	if (!AttachConsole(nParentID)) {	//Attach to the parent console
		nErr = GetLastError();
		goto Done;
	}

	if (!SetEvent((HANDLE)_wtol(argv[2]))) {		//Allow the parent to continue execution
		nErr = GetLastError();
		goto Done;
	}

	if (WaitForMultipleObjects(2, hList, FALSE, INFINITE) == WAIT_FAILED)
		nErr = GetLastError();

Done:
	CloseHandle(hList[0]);
	return nErr;
}
