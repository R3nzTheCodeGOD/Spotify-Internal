#include <Windows.h>
#include <Psapi.h>

bool DataCompare(char* pData, const char* bSig, const char* szMask) noexcept
{
	for (; *szMask; ++szMask, ++pData, ++bSig) {
		if (*szMask == 'x' && *pData != *bSig)
			return false;
	}
	return (*szMask) == NULL;
}

char* FindPattern(char* dwAddress, DWORD dwSize, const char* pbSig, const char* szMask) noexcept
{
	DWORD length{ strlen(szMask) };

	for (DWORD i{ 0 }; i < dwSize - length; ++i) {
		__try {
			if (DataCompare(dwAddress + i, pbSig, szMask))
				return dwAddress + i;
		} __except (EXCEPTION_EXECUTE_HANDLER) {
			return nullptr;
		}
	}

	return 0;
}

char* scan(const char* pattern, const char* mask, MODULEINFO m) noexcept
{
	const auto hModule{ GetModuleHandleA(nullptr) };
	return FindPattern((char*)hModule, m.SizeOfImage, pattern, mask);
}

void removeAds(char* fn) noexcept
{
	DWORD oldProtect;

	VirtualProtect(fn + 5, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset(fn + 5, 0x90, 1);
	VirtualProtect(fn + 5, 1, oldProtect, &oldProtect);

	VirtualProtect(fn + 6, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	memset(fn + 6, 0xE9, 1);
	VirtualProtect(fn + 6, 1, oldProtect, &oldProtect);
}

void APIENTRY main() noexcept
{
	const auto hModule{ ::GetModuleHandleA(nullptr) };
	MODULEINFO mInfo = { 0 };

	if (::K32GetModuleInformation(::GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO))) {
		auto skipPod = scan("\x6C\xFF\xFF\xFF\x07\x0F\x85\xB7\x02\x00\x00", "xxxxxxxxxxx", mInfo);

		if (skipPod) {
			removeAds(skipPod);
			::MessageBoxA(nullptr, "Hook Successful", "SpotifyInternal", MB_OK | MB_ICONINFORMATION);
		} else {
			skipPod = scan("\x83\xC4\x08\x84\xC0\x0F\x84\xED\x03\x00\x00", "xxxxxxxxxxx", mInfo);
			
			if (skipPod) {
				removeAds(skipPod);
				::MessageBoxA(nullptr, "Hook Successful", "SpotifyInternal", MB_OK | MB_ICONINFORMATION);
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		::DisableThreadLibraryCalls(hModule);
		if (!wcsstr(GetCommandLine(), L"--type="))
			if (const auto hThread{ ::CreateThread(nullptr, 0u, (LPTHREAD_START_ROUTINE)main, nullptr, 0ul, nullptr) }; hThread)
				::CloseHandle(hThread);

	}

	return TRUE;
}
