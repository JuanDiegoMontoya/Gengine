#include "../../stdafx.h"
#include "abo.h"

ABO::ABO(GLuint num) : numCounters_(num)
{
	glGenBuffers(1, &rendererID_);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, rendererID_);

#if PERSISTENCE
	GLbitfield storageFlags =
		GL_DYNAMIC_STORAGE_BIT |
		GL_MAP_READ_BIT |
		GL_MAP_WRITE_BIT |
		GL_CLIENT_STORAGE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_COHERENT_BIT;
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER, numCounters_ * sizeof(GLuint), NULL,
		storageFlags);
	rbuffer = (GLuint*)glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		0, num * sizeof(GLuint),
		GL_MAP_READ_BIT |
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_COHERENT_BIT);
#else
	glBufferStorage(GL_ATOMIC_COUNTER_BUFFER,
		numCounters_ * sizeof(GLuint), nullptr,
		GL_DYNAMIC_STORAGE_BIT |
		GL_MAP_READ_BIT |
		GL_MAP_WRITE_BIT |
		GL_CLIENT_STORAGE_BIT);
#endif
}

ABO::~ABO()
{
#if PERSISTENCE
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
#endif
	glDeleteBuffers(1, &rendererID_);
}

void ABO::Bind(GLuint index)
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, rendererID_);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, rendererID_);
}

void ABO::Unbind()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void ABO::Reset()
{
#if PERSISTENCE
	std::memset(rbuffer, 0, numCounters_ * sizeof(GLuint));
#else
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, rendererID_);
	GLuint* buffer;
	buffer = (GLuint*)glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		0,
		numCounters_ * sizeof(GLuint),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	std::memset(buffer, 0, numCounters_ * sizeof(GLuint));
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
#endif
}

void ABO::Set(GLuint index, GLuint value)
{
#if PERSISTENCE
	rbuffer[index] = value;
#else
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, rendererID_);
	glBufferSubData(
		GL_ATOMIC_COUNTER_BUFFER,
		index * sizeof(GLuint),
		sizeof(GLuint),
		&value);
#endif
}

GLuint ABO::Get(GLuint index)
{
#if PERSISTENCE
	return rbuffer[index];
#else
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, rendererID_);
	//GLuint val;
	//glGetBufferSubData(
	//	GL_ATOMIC_COUNTER_BUFFER,
	//	index * sizeof(GLuint),
	//	sizeof(GLuint),
	//	&val);
	GLuint* ptr = (GLuint*)glMapBufferRange(
		GL_ATOMIC_COUNTER_BUFFER,
		index * sizeof(GLuint),
		sizeof(GLuint),
		GL_MAP_READ_BIT);
	GLuint val = *ptr;
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	return val;
#endif
}
