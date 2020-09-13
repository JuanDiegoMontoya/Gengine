#pragma once
#include "block.h"

// renders stuff directly to the screen
class HUD
{
public:
	void Update();

	BlockType GetSelected() { return selected_; }
	void SetSelected(BlockType s) { selected_ = s; }
private:
	BlockType selected_ = BlockType::bError;
};