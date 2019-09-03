#pragma once

void** FindIATFunction(const char* function, HMODULE module );

uintptr_t DetourIATPtr(const char* function, void* newfunction, HMODULE module);

using getmessageproc_fn = LRESULT(__stdcall*)(uintptr_t t, UINT code, LPARAM lParam, intptr_t param);
//getmessageproc_fn oGetMsgProc;



class _Dummy {
public:
	
	static _Dummy& Instance() {
		static _Dummy _Handle;
		return _Handle;
	}
	LRESULT __thiscall hk_getmessageproc(uintptr_t t, UINT code, LPARAM lParam, intptr_t param);
};
