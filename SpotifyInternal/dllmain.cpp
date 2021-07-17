#include <Windows.h>

void __stdcall main()
{
	// ...
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

