#include "../../stdafx.h"

#include "vbo.h"

VBO::VBO(const void * data, unsigned int size, GLenum drawmode)
{
	glGenBuffers(1, &vboID_);
	glBindBuffer(GL_ARRAY_BUFFER, vboID_);
	glBufferData(GL_ARRAY_BUFFER, size, data, drawmode);
}

VBO::~VBO()
{
	glDeleteBuffers(1, &vboID_);
}

void VBO::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, vboID_);
}

void VBO::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
