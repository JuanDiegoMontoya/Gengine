#pragma once

namespace Interface
{
	inline bool activeCursor = false;
	inline bool debug_graphs = true;

	void Init();
	void Update();
	void DrawImGui();
	inline bool IsCursorActive()
	{
		return activeCursor;
	}
}