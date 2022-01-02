#pragma warning(disable:4326)
#include <Windows.h>
#include <Psapi.h>

void WINAPI load_api(LPVOID* destination, LPCSTR api_name) noexcept
{
	if (auto h_mod{ ::LoadLibrary(L"_chrome_elf.dll") }; h_mod && !*destination)
		*destination = ::GetProcAddress(h_mod, api_name);
}

#define API_EXPORT_ORIG(N) \
	static LPVOID _##N = NULL; \
	char S_##N[] = "" # N; \
	extern "C" __declspec(dllexport) __declspec(naked) void N ## () \
	{ \
		__asm pushad \
		__asm push offset S_##N \
		__asm push offset _##N \
		__asm call load_api \
		__asm popad \
		__asm jmp [_##N] \
	} \

API_EXPORT_ORIG(ClearReportsBetween_ExportThunk)
API_EXPORT_ORIG(CrashForException_ExportThunk)
API_EXPORT_ORIG(DisableHook)
API_EXPORT_ORIG(DrainLog)
API_EXPORT_ORIG(DumpHungProcessWithPtype_ExportThunk)
API_EXPORT_ORIG(DumpProcessWithoutCrash)
API_EXPORT_ORIG(GetApplyHookResult)
API_EXPORT_ORIG(GetBlockedModulesCount)
API_EXPORT_ORIG(GetCrashReports_ExportThunk)
API_EXPORT_ORIG(GetCrashpadDatabasePath_ExportThunk)
API_EXPORT_ORIG(GetHandleVerifier)
API_EXPORT_ORIG(GetInstallDetailsPayload)
API_EXPORT_ORIG(GetUniqueBlockedModulesCount)
API_EXPORT_ORIG(GetUserDataDirectoryThunk)
API_EXPORT_ORIG(InjectDumpForHungInput_ExportThunk)
API_EXPORT_ORIG(IsBrowserProcess)
API_EXPORT_ORIG(IsCrashReportingEnabledImpl)
API_EXPORT_ORIG(IsThirdPartyInitialized)
API_EXPORT_ORIG(RegisterLogNotification)
API_EXPORT_ORIG(RequestSingleCrashUpload_ExportThunk)
API_EXPORT_ORIG(SetCrashKeyValueImpl)
API_EXPORT_ORIG(SetMetricsClientId)
API_EXPORT_ORIG(SetUploadConsent_ExportThunk)
API_EXPORT_ORIG(SignalChromeElf)
API_EXPORT_ORIG(SignalInitializeCrashReporting)

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
	DWORD length{ ::strlen(szMask) };

	for (auto i{ 0ul }; i < dwSize - length; ++i) {
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
	return FindPattern((char*)::GetModuleHandle(nullptr), m.SizeOfImage, pattern, mask);
}

void removeAds(char* fn) noexcept
{
	DWORD oldProtect;

	::VirtualProtect(fn + 5, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	::memset(fn + 5, 0x90, 1);
	::VirtualProtect(fn + 5, 1, oldProtect, &oldProtect);

	::VirtualProtect(fn + 6, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	::memset(fn + 6, 0xE9, 1);
	::VirtualProtect(fn + 6, 1, oldProtect, &oldProtect);
}

void APIENTRY main() noexcept
{
	const auto hModule{ ::GetModuleHandle(nullptr) };
	MODULEINFO mInfo = { 0 };

	if (::GetModuleInformation(::GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO))) {
		auto skipPod = scan("\x6C\xFF\xFF\xFF\x07\x0F\x85\xB7\x02\x00\x00", "xxxxxxxxxxx", mInfo);

		if (skipPod) {
			removeAds(skipPod);
			::MessageBox(nullptr, L"Hook Successful", L"SpotifyInternal", MB_OK | MB_ICONINFORMATION);
		} else {
			skipPod = scan("\x83\xC4\x08\x84\xC0\x0F\x84\xED\x03\x00\x00", "xxxxxxxxxxx", mInfo);
			
			if (skipPod) {
				removeAds(skipPod);
				::MessageBox(nullptr, L"Hook Successful", L"SpotifyInternal", MB_OK | MB_ICONINFORMATION);
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		::DisableThreadLibraryCalls(hModule);
		if (!wcsstr(GetCommandLine(), L"--type="))
			if (const auto hThread{ ::CreateThread(nullptr, 0u, reinterpret_cast<LPTHREAD_START_ROUTINE>(main), nullptr, 0ul, nullptr)}; hThread)
				::CloseHandle(hThread);

	}

	return TRUE;
}
