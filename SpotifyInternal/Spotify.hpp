#pragma once

#include <iostream>

class Spotify
{
public:
	Spotify() noexcept;
	~Spotify();
	static Spotify* instance() noexcept;
	uint8_t* m_fn_is_skippable = nullptr;
	uint8_t* m_fn_require_focus = nullptr;
	uint8_t* m_fn_now_playing = nullptr;
	uint8_t* m_jne_ad_missing_id = nullptr;
	uint8_t* m_mov_skip_stuck_seconds = nullptr;

	bool valid_ptrs() noexcept;
private:

};
