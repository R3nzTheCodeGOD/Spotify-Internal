#include <Windows.h>
#include <Psapi.h>

bool DataCompare(BYTE* pData, BYTE* bSig, char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bSig) {
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

BYTE* FindPattern(BYTE* dwAddress, DWORD dwSize, BYTE* pbSig, char* szMask)
{
	DWORD length = strlen(szMask);

	for (DWORD i = NULL; i < dwSize - length; i++) {
		__try
		{
			if (DataCompare(dwAddress + i, pbSig, szMask))
				return dwAddress + i;
		} 
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return nullptr;
		}
	}

	return 0;
}

int __stdcall main() noexcept
{
	const auto hModule = GetModuleHandle(NULL);
	MODULEINFO mInfo = { 0 };
	DWORD oldProtect;

	if (GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO))) {
		const auto skipPod = FindPattern((BYTE*)hModule, mInfo.SizeOfImage, (BYTE*)"\x83\xC4\x08\x84\xC0\x0F\x84\xE5\x03\x00\x00", (char*)"xxxxxxxxxxx");

		if (skipPod) {
			VirtualProtect((char*)skipPod + 5, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset((char*)skipPod + 5, 0x90, 1);
			VirtualProtect((char*)skipPod + 5, 1, oldProtect, &oldProtect);

			VirtualProtect((char*)skipPod + 6, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
			memset((char*)skipPod + 6, 0xE9, 1);
			VirtualProtect((char*)skipPod + 6, 1, oldProtect, &oldProtect);
		}
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		if (!wcsstr(GetCommandLine(), L"--type="))
			CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(main), nullptr, 0, nullptr);
	}

	return TRUE;
}