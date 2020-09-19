#include <Chunks/Chunk.h>
#include <Graphics/shader.h>
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