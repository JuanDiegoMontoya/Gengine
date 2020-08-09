#include "stdafx.h"

#include <Pipeline.h>
#include "Renderer.h"
#include <camera.h>
#include <Frustum.h>

#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "chunk.h"
#include "block.h"
#include "shader.h"
#include <Vertices.h>
#include <sstream>
#include "settings.h"
#include "misc_utils.h"
#include "ChunkStorage.h"
#include "ChunkMesh.h"


Chunk::Chunk()
{
	mesh.SetParent(this);
}


Chunk::~Chunk()
{
}


Chunk::Chunk(const Chunk& other)
{
	*this = other;
}


// copy assignment operator for serialization
Chunk& Chunk::operator=(const Chunk& rhs)
{
	mesh.SetParent(this);
	this->SetPos(rhs.pos_);
	// TODO: storage should be set equal here, but idc about this function rn
	this->storage = rhs.storage;
	return *this;
}


void Chunk::Update()
{
	// in the future, make this function perform other tick update actions,
	// such as updating N random blocks (like in Minecraft)
}

