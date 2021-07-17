#include "Spotify.hpp"
#include "memory.hpp"
#include "utils.hpp"

Spotify* l = nullptr;

Spotify::Spotify()
{
	auto main = GetModuleHandle(nullptr);
	auto get_function_start = [=](uint8_t* ptr) -> uint8_t* {
		for (size_t i = 0; i < 0x1000; ++i) {
			if (*reinterpret_cast<uint32_t*>(ptr - i) == 0x6AEC8B55) return ptr - i;
		}
		return nullptr;
	};
	m_jne_ad_missing_id = memory::pattern_scan(main, 0, "0F 85 ?? ?? ?? ?? A1 ?? ?? ?? ?? F3 0F 7E 05 ?? ?? ?? ?? 89 45 E0 A0");
	m_mov_skip_stuck_seconds = memory::pattern_scan(main, 0, "B9 E8 03 00 00 F7 E9 83");
	m_fn_is_skippable = memory::pattern_scan(main, 0, "83 C4 10 BB ? ? ? ? 83 78 14 10");
	m_fn_now_playing = memory::pattern_scan(main, 0, "8B C1 89 85 ? ? ? ? 8B 55 08 C7 85");
	m_fn_require_focus = memory::pattern_scan(main, 0, "72 65 71 75 69 72 65 5F 66 6F 63 75 73 00 00 00 0D");
	if (m_fn_is_skippable)
		m_fn_is_skippable = get_function_start(m_fn_is_skippable);
	if (m_fn_now_playing)
		m_fn_now_playing = get_function_start(m_fn_now_playing);
	if (m_mov_skip_stuck_seconds)
		m_mov_skip_stuck_seconds += 1;
	if (m_fn_require_focus) {
		auto pattern = new uint8_t[5];
		pattern[0] = 0x68;
		*reinterpret_cast<uint8_t**>(&pattern[1]) = m_fn_require_focus;
		m_fn_require_focus = memory::find_pattern(main, memory::get_mod_size(main), pattern, 5);
	}
	if (m_fn_require_focus)
		m_fn_require_focus = get_function_start(m_fn_require_focus);
}

bool Spotify::valid_ptrs()
{
	return (m_jne_ad_missing_id && m_mov_skip_stuck_seconds && m_fn_is_skippable && m_fn_now_playing && m_fn_require_focus);
}

Spotify::~Spotify() = default;

Spotify* Spotify::instance()
{
	if (l == nullptr)
		l = new Spotify();
	return l;
}