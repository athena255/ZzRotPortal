#include "pch.h"
#include "Helpers.h"
// szFile: name of the file to write to
// DataBuffer: contents to write to the file
// dwBytesToWrite: number of bytes to write to the file
void __fastcall _Helpers::WritetoFile(const wchar_t* szFile, const char* DataBuffer, DWORD dwBytesToWrite)
{

	HANDLE hFile;
	DWORD dwBytesWritten = 0;
	BOOL bError = FALSE;

	hFile = CreateFile(szFile, FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DEBUG_PRINT(("CreateFile %d", GetLastError()));
	}

	bError = WriteFile(hFile, DataBuffer, dwBytesToWrite, &dwBytesWritten, NULL);
	if (bError == FALSE)
	{
		DEBUG_PRINT(("WriteFile %d", GetLastError()));
	}

	if (dwBytesWritten != dwBytesToWrite)
	{
		DEBUG_PRINT(("dwBytesWritten != dwBytesToWrite"));
	}
	CloseHandle(hFile);
}

void _Helpers::_Open(int PID, HANDLE& hProc)
{
	hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (!hProc)
	{
		printf("OpenProcess failed: 0x%x\n", GetLastError());
		system("PAUSE");
	}
}


int  _Helpers::GetProcecssID(const wchar_t* szProc)
{
	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	// 0 bc want to enumerate all processes
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed: 0x%x\n", GetLastError());
		system("PAUSE");
		return 0;
	}

	DWORD PID = 0;
	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet)
	{
		// if the process name and
		if (!wcscmp(szProc, PE32.szExeFile))
		{
			PID = PE32.th32ProcessID;
			break;
		}

		bRet = Process32Next(hSnap, &PE32);
	}
	CloseHandle(hSnap);
	return PID;
}
