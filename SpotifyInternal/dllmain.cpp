#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <Psapi.h>
#include <uniHook.hpp>
#include <string>

#include "utils.h"
#include "Spotify.h"

void __stdcall main()
{
	version_t v;

	utils::get_version(&v);

	if (v.dwMajor < 1 || v.dwMinor < 1)
	{
		MessageBoxA(nullptr, "Need a higher version", "problem", 0);
		return;
	}
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		if (!wcsstr(GetCommandLine(), L"--type="))
			CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( main ), nullptr, 0, nullptr);
	}

	return TRUE;
}

