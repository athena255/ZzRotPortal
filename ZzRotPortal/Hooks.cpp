#include "pch.h"
#include "Hooks.h"

void** FindIATFunction(const char* function, HMODULE module = 0)
{
	if (!module)
		module = GetModuleHandle(0);

	PIMAGE_DOS_HEADER img_dos_headers = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS img_nt_headers = (PIMAGE_NT_HEADERS)((BYTE*)img_dos_headers + img_dos_headers->e_lfanew);

	PIMAGE_IMPORT_DESCRIPTOR img_import_desc =
		(PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)img_dos_headers
			+ img_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	if (img_dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
		std::cout<< (("ERROR: e_magic is no valid DOS signature\ %d", GetLastError()));

	for (IMAGE_IMPORT_DESCRIPTOR* iid = img_import_desc; iid->Name != 0; iid++)
	{
		for (int func_idx = 0; *(func_idx + (void**)(iid->FirstThunk + (size_t)module)) != nullptr; func_idx++)
		{
			char* mod_func_name = (char*)(*(func_idx + (size_t*)(iid->OriginalFirstThunk + (size_t)module)) + (size_t)module + 2);

			const intptr_t nmod_func_name = (intptr_t)mod_func_name;

			if (nmod_func_name >= 0)
			{
				if (!::strcmp(function, mod_func_name))
					return func_idx + (void**)(iid->FirstThunk + (size_t)module);
			}
		}
	}

	return 0;
}


uintptr_t DetourIATPtr(const char* function, void* newfunction, HMODULE module = 0)
{
	auto&& func_ptr = FindIATFunction(function, module);

	if (*func_ptr == newfunction || *func_ptr == nullptr) return 0;

	DWORD old_rights, new_rights = PAGE_READWRITE;

	VirtualProtect(func_ptr, sizeof(uintptr_t), new_rights, &old_rights);

	uintptr_t ret = (uintptr_t)* func_ptr;

	*func_ptr = newfunction;

	VirtualProtect(func_ptr, sizeof(uintptr_t), old_rights, &new_rights);

	return ret;
}

LRESULT __thiscall _Dummy::hk_getmessageproc(uintptr_t t, UINT code, LPARAM lParam, intptr_t param)
{

	/*return oGetMsgProc(t, code, lParam, param);*/
	return NULL;
}