#include <Pipeline.h>
#include <Rendering/Renderer.h>
#include <Components/Camera.h>
#include <Rendering/Frustum.h>

#include <Graphics/vbo.h>
#include <Graphics/vao.h>
#include "ibo.h"
#include <Chunks/Chunk.h>
#include <block.h>
#include <Graphics/shader.h>
#include <Graphics/Vertices.h>
#include <sstream>
#include <Refactor/settings.h>
#include <Graphics/misc_utils.h>
#include <Chunks/ChunkStorage.h>
#include <Chunks/ChunkMesh.h>


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