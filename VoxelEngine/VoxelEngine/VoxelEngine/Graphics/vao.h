#pragma once

#include <Graphics/vbo.h>
#include <Graphics/vbo_layout.h>

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