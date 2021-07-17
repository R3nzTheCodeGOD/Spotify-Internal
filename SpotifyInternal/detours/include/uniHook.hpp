#pragma once
#include <windows.h>
#include <iostream>
#include <map>
#include <detours.h>
#if _WIN64
#pragma comment(lib, "detours.lib")
#else
#pragma comment(lib, "detours.lib")
#endif
using namespace std;
enum hkType
{
	MS_Detour,
	MinHook,
	Simple
};
namespace hk
{
	
	template<typename T = void*, typename D = void*>
	bool apply_detour(D detour, T & address_o)
	{
		const auto p_detour = reinterpret_cast<void*>(detour);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&reinterpret_cast<void*&>(address_o), p_detour);
		DetourTransactionCommit();
		return true;
	}
	template<typename T = void*, typename D = void*>
	bool rem_detour(D detour, T & address_o)
	{
		const auto p_detour = reinterpret_cast<void*>(detour);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&reinterpret_cast<void*&>(address_o), p_detour);
		DetourTransactionCommit();
		return true;
	}
	template<typename T, typename V>
	bool apply_vtable(T v_table, V v_detour, PVOID * original)
	{
		auto ret = false;
		auto const change_this	= reinterpret_cast<uintptr_t*>(v_table);
		*original			= reinterpret_cast<void*>(change_this[0]);
		DWORD p = 0;
		if (VirtualProtect(change_this, 0x100, PAGE_EXECUTE_READWRITE, &p))
		{
			change_this[0] = uintptr_t(v_detour);
			ret = true;
			VirtualProtect(change_this, 0x100, p, &p);
		}
		return ret;
	}

};