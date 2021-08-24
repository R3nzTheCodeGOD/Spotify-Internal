#include <Windows.h>
#include <Psapi.h>

bool __cdecl DataCompare(char* pData, const char* bSig, const char* szMask) noexcept
{
	for (; *szMask; ++szMask, ++pData, ++bSig) {
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

char* __cdecl FindPattern(char* dwAddress, DWORD dwSize, const char* pbSig, const char* szMask) noexcept
{
	DWORD length = strlen(szMask);

	for (DWORD i = NULL; i < dwSize - length; i++) {
		__try {
			if (DataCompare(dwAddress + i, pbSig, szMask))
				return dwAddress + i;
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			return nullptr;
		}
	}

	return 0;
}

char* __cdecl scan(const char* pattern, const char* mask, MODULEINFO m) noexcept
{
	const auto hModule = GetModuleHandle(NULL);
	return FindPattern((char*)hModule, m.SizeOfImage, pattern, mask);
}

void __stdcall userMsg() noexcept
{
	MessageBoxA(nullptr, "Hook Successful", "SpotifyInternal", MB_OK | MB_ICONINFORMATION);
}

void __stdcall removeAds(char* fn) noexcept
{
	DWORD oldProtect;

	VirtualProtect(fn + 5, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset(fn + 5, 0x90, 1);
	VirtualProtect(fn + 5, 1, oldProtect, &oldProtect);

	VirtualProtect(fn + 6, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset(fn + 6, 0xE9, 1);
	VirtualProtect(fn + 6, 1, oldProtect, &oldProtect);
}

int __stdcall main() noexcept
{
	const auto hModule = GetModuleHandle(NULL);
	MODULEINFO mInfo = { 0 };

	if (GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO))) {
		auto skipPod = scan("\x83\xC4\x08\x84\xC0\x0F\x84\xE5\x03\x00\x00", "xxxxxxxxxxx", mInfo);

		if (skipPod) {
			removeAds(skipPod);
			userMsg();
		} else {
			skipPod = scan("\x83\xC4\x08\x84\xC0\x0F\x84\xED\x03\x00\x00", "xxxxxxxxxxx", mInfo);
			
			if (skipPod) {
				removeAds(skipPod);
				userMsg();
			}
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