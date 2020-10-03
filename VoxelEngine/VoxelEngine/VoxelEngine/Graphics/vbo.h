#pragma once

// vertex buffer object
class VBO
{
public:
	VBO(const void* data, unsigned int size, GLenum drawmode = GL_STATIC_DRAW);
	~VBO();

	void Bind() const;
	void Unbind() const;

	GLuint GetID() { return vboID_; }
private:
	GLuint vboID_;
};