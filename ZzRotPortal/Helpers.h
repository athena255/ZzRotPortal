#pragma once
#define Helpers _Helpers::Instance()
class _Helpers
{
public:

	static _Helpers& Instance() {
		static _Helpers _Handle;
		return _Handle;
	}
	void __fastcall WritetoFile(const wchar_t* szFile, const char* DataBuffer, DWORD dwBytesToWrite);

	void _Open(int PID, HANDLE& hProc);
	int GetProcecssID(const wchar_t* szProc);


};
