// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
// Runs the main Comms thread
DWORD WINAPI CommsThread(LPVOID lpvThreadParam)
{
	//while (true)
	//{
	//	Comms.RunComms();
	//}
	KeyboardHook.InitKeyboardHook();
	while (true) {
		PeekMessage(NULL, NULL, 0, 0, 0);
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: {
		//AllocConsole();
		//FILE* pCout;
		//freopen_s(&pCout, "conout$", "w", stdout);
		////first hook this
		////auto&& msgHook = FindIATFunction("On0cmKeyDown", GetModuleHandle(L"vmwarewui.dll"));
		////oGetMsgProc = (getmessageproc_fn)DetourIATPtr("On0cmKeyDown", (void*)_Dummy::Instance().hk_getmessageproc);
		//system("PAUSE");
		//fclose(pCout);
		//FreeConsole();
		CreateThread(nullptr, 0, CommsThread, nullptr, 0, nullptr); \
		while (true)
		{
			Comms.RunComms();
		}
		

	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:

        break;
    }
    return TRUE;
}