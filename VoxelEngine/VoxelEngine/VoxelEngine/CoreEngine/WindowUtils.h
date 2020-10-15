#pragma once

struct Settings
{
	Settings(bool pMaximize = false, bool pFullscreen = false, bool pVsync = false, int pMSAA = 0) :
		maximize(pMaximize), fullscreen(pFullscreen), vsync(pVsync), MSAA(pMSAA) {}

	bool vsync;
	int MSAA;
	bool maximize;
	bool fullscreen;
};
struct Layout
{
	Layout(int pW = 0, int pH = 0, int pL = 0, int pT = 0) : width(pW), height(pH), left(pL), top(pT) {}
	int width;
	int height;

	int left;
	int top;
};
