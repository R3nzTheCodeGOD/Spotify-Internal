#ifndef MEMORY_H
#define MEMORY_H
#pragma once

#include <Windows.h>
#include <vector>

namespace memory
{
	inline size_t get_mod_size(void* module)
	{
		auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
		auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(module) + dos_header->e_lfanew);
		return nt_header->OptionalHeader.SizeOfImage;
	}

	inline uint8_t* find_pattern(void* module, size_t end, uint8_t* pattern, size_t pattern_size)
	{
		auto start = reinterpret_cast<uint8_t*>(module);
		for (auto i = 0ul; i < end - pattern_size; ++i) {
			bool found = true;
			for (auto j = 0ul; j < pattern_size; ++j) {
				if (start[i + j] != pattern[j] && pattern[j] != 0xCC) {
					found = false;
					break;
				}
			}
			if (found) {
				return &start[i];
			}
		}
		return nullptr;
	}

	inline uint8_t* pattern_scan(void* module, size_t size_of_image, const char* signature)
	{
		static auto pattern_to_byte = [](const char* pattern)
		{
			auto bytes = std::vector < uint8_t >{ };
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);
			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back(0xCC);
				} else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		if (!size_of_image)
			size_of_image = get_mod_size(module);

		auto pattern_bytes = pattern_to_byte(signature);
		auto s = pattern_bytes.size();
		auto d = pattern_bytes.data();
		return find_pattern(module, size_of_image, d, s);
	}
}

#endif