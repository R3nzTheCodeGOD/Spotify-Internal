#pragma once
#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <iostream>

class Spotify
{
public:
	Spotify();
	~Spotify();
	static Spotify* instance();
	uint8_t* m_fn_is_skippable = nullptr;
	uint8_t* m_fn_require_focus = nullptr;
	uint8_t* m_fn_now_playing = nullptr;
	uint8_t* m_jne_ad_missing_id = nullptr;
	uint8_t* m_mov_skip_stuck_seconds = nullptr;

	bool valid_ptrs();
private:

};

#endif
