#pragma once

#include "vbo.h"
#include "vbo_layout.h"

class VAO
{
public:
	VAO();
	~VAO();

	void AddBuffer(const VBO& vb, const VBOlayout& layout);

	void Bind() const;
	void Unbind() const;

private:
	GLuint rendererID_;
};