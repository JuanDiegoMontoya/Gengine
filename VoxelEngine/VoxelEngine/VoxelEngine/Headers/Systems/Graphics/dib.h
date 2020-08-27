#pragma once

// draw indirect buffer object

struct DrawElementsIndirectCommand
{
	GLuint  count;
	GLuint  instanceCount;
	GLuint  firstIndex;
	GLuint  baseVertex;
	GLuint  baseInstance;
	// note: baseInstance is for glMultiDraw*Indirect ONLY
	// for any other purpose it must be zero
};

struct DrawArraysIndirectCommand
{
	GLuint  count;
	GLuint  instanceCount;
	GLuint  first;
	GLuint  baseInstance;
};

class DIB
{
public:
	DIB(void* data, GLsizei size, GLenum drawmode = GL_STATIC_DRAW);
	~DIB();

	void Bind() const;
	void Unbind() const;

	GLuint GetID() { return dibID_; }
private:
	GLuint dibID_;
};