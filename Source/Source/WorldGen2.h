#pragma once

struct Chunk;

// https://github.com/tModLoader/tModLoader/wiki/Vanilla-World-Generation-Steps
namespace WorldGen2
{
	void Init();
	void GenerateWorld();
	void InitMeshes();
	void InitBuffers();
};