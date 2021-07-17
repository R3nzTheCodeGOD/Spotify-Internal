#include "utils.hpp"
#pragma comment(lib, "Version.lib")

typedef struct {
	WORD             wLength;
	WORD             wValueLength;
	WORD             wType;
	WCHAR            szKey[16];
	WORD             Padding1;
	VS_FIXEDFILEINFO Value;
	WORD             Padding2;
	WORD             Children;
} VS_VERSIONINFO;

bool utils::get_version(version_t* v)
{
	WCHAR module_file_path[MAX_PATH];
	bool ok = false;
	DWORD ver_handle = 0;
	GetModuleFileName(GetModuleHandle(nullptr), module_file_path, MAX_PATH);
	auto ver_size = GetFileVersionInfoSize(module_file_path, &ver_handle);
	if (ver_size != 0)
	{
		void* ver_buffer = nullptr;
		UINT size = 0;
		auto* ver_data = new char[ver_size];
		if (GetFileVersionInfoW(module_file_path, 0, ver_size, ver_data) && VerQueryValueA(ver_data, "\\", &ver_buffer, &size) && size)
		{
			auto* ver_info = reinterpret_cast<VS_VERSIONINFO*>(ver_data);
			if (ver_info->Value.dwSignature == 0xfeef04bd)
			{
				v->dwMajor = ver_info->Value.dwFileVersionMS >> 16 & 0xffff;
				v->dwMinor = ver_info->Value.dwFileVersionMS & 0xffff;
				v->dwBuild = ver_info->Value.dwFileVersionLS >> 16 & 0xffff;
				v->dwRevision = ver_info->Value.dwFileVersionLS & 0xffff;
				ok = true;
			}
		}
		delete[] reinterpret_cast<char*>(ver_data);
	}
	return ok;
}