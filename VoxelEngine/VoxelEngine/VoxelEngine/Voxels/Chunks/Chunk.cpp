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


Chunk::Chunk(const Chunk& other) : mesh(this)
{
	*this = other;
}


// copy assignment operator for serialization
Chunk& Chunk::operator=(const Chunk& rhs)
{
	this->pos_ = rhs.pos_;
	this->storage = rhs.storage;
	return *this;
}