#pragma once

#include "stdafx.h"

// TODO: refactor this
namespace Settings
{
	struct GFX
	{
		bool vsync = true;
		int multisamples = 0; // 0 = no msaa
		bool reinit = false; // enabled when reinitializing context

		bool fullscreen = false;

		// actual screen X and Y, use a temp to store settings before they're set
		int screenX = 1920;
		int screenY = 1080;

		// add supported resolutions to this list
		const std::vector<std::pair<int, int>> resolutions =
		{
			{ 3840, 2160 },
			{ 2560, 1440 },
			{ 1920, 1080 },
			{ 1600, 900 },
			{ 1440, 900 },
			{ 1366, 768 }
		};
		const size_t res_amt = resolutions.size();

		static inline bool blockAO = true;
	};

	struct SND
	{
		// from 0 to 10
		float master = 1;		// global multiplier
		float music = 1;		// music
		float sfx = 1;			// various sound effects
		float ambient = 1;  // environment, background, etc.
	};

	static inline typename GFX Graphics;
	static inline typename SND Sound;
}