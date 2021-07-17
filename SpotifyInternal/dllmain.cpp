#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <Psapi.h>
#include <uniHook.hpp>
#include <string>

#include "utils.hpp"
#include "Spotify.hpp"

void __stdcall load_api(LPVOID* destination, LPCSTR api_name);

#define API_EXPORT_ORIG(N) \
	static LPVOID _##N = NULL; \
	char S_##N[] = "" # N; \
	extern "C" __declspec(dllexport) __declspec(naked) void N ## () \
	{ __asm pushad __asm push offset S_##N __asm push offset _##N __asm call load_api __asm popad __asm jmp [_##N] } \

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

void __stdcall load_api(LPVOID* destination, LPCSTR api_name)
{
	if (*destination) return;
	auto* h_mod = LoadLibrary(L"_chrome_elf.dll");
	if (!h_mod) return;
	*destination = GetProcAddress(h_mod, api_name);
}

char last_playing_uri[200] = { 0 };
bool g_skip_track = false;

__declspec(naked) void is_skippable_hook()
{
	__asm {
		mov eax, 1
		ret
	}
}

_declspec(naked) void can_focus_hook()
{
	__asm {
		xor eax, eax
		ret
	}
}

void __stdcall skip_track()
{
	int cnt = 0;
	while (g_skip_track && cnt++ < 2) {
		Sleep(250);
		if (g_skip_track) {
			keybd_event(VK_MEDIA_NEXT_TRACK, 0x0, KEYEVENTF_EXTENDEDKEY, 0);
			keybd_event(VK_MEDIA_NEXT_TRACK, 0x0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			break;
		}
	}
}

int __fastcall now_playing_hook(char* p_this, void* edx, uintptr_t track)
{
	auto current_track_uri = *reinterpret_cast<char**>(track + 0x184);
	if (current_track_uri && strncmp(current_track_uri, last_playing_uri, 200)) {
		strncpy_s(last_playing_uri, current_track_uri, 200);
		if (!strncmp(current_track_uri, "spotify:ad:", 11) || !strncmp(current_track_uri, "spotify:interruption:", 21)) {
			g_skip_track = true;
			CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(skip_track), nullptr, 0, nullptr);
		}
		else {
			g_skip_track = false;
		}
	}
	return reinterpret_cast<decltype(now_playing_hook)*>(Spotify::instance()->m_fn_now_playing)(p_this, edx, track);
}

void patch_ad_missing_id()
{
	DWORD p;
	VirtualProtect(Spotify::instance()->m_jne_ad_missing_id, 6, PAGE_EXECUTE_READWRITE, &p);
	memset(Spotify::instance()->m_jne_ad_missing_id, 0x90, 6);
	VirtualProtect(Spotify::instance()->m_jne_ad_missing_id, 6, p, &p);
}

void patch_skip_stuck_seconds()
{
	DWORD p;
	VirtualProtect(Spotify::instance()->m_mov_skip_stuck_seconds, 6, PAGE_EXECUTE_READWRITE, &p);
	*reinterpret_cast<DWORD*>(Spotify::instance()->m_mov_skip_stuck_seconds) = 0;
	VirtualProtect(Spotify::instance()->m_mov_skip_stuck_seconds, 6, p, &p);
}

void __stdcall main()
{
	version_t v;
	utils::get_version(&v);
	if (v.dwMajor < 1 || v.dwMinor < 1) {
		MessageBoxA(nullptr, "Need a higher version", "R3nzError", 0);
		return;
	}
	if (!Spotify::instance()->valid_ptrs()) {
		MessageBoxA(nullptr, "Spotify fail", "R3nzError", 0);
		return;
	}

	hk::apply_detour(can_focus_hook, Spotify::instance()->m_fn_require_focus);
	hk::apply_detour(now_playing_hook, Spotify::instance()->m_fn_now_playing);
	hk::apply_detour(is_skippable_hook, Spotify::instance()->m_fn_is_skippable);
	patch_skip_stuck_seconds();
	patch_ad_missing_id();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		if (!wcsstr(GetCommandLine(), L"--type="))
			CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(main), nullptr, 0, nullptr);
	}
	return TRUE;
}