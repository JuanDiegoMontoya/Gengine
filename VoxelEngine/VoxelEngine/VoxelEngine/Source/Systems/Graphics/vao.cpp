#include "../../stdafx.h"

#include "vao.h"
//#include "Renderer.h"

VAO::VAO()
{
	glGenVertexArrays(1, &rendererID_);
	glBindVertexArray(rendererID_);
}

VAO::~VAO()
{
	glDeleteVertexArrays(1, &rendererID_);
}

void VAO::AddBuffer(const VBO & vb, const VBOlayout & layout)
{
	Bind();
	vb.Bind();
	const auto& elements = layout.GetElements();
	GLuint offset = 0;
	for (size_t i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.count, element.type, element.normalized, 
			layout.GetStride(), (const void*)offset);
		offset += element.count * VBOElement::TypeSize(element.type);
	}
}

void VAO::Bind() const
{
	glBindVertexArray(rendererID_);
}

void VAO::Unbind() const
{
	glBindVertexArray(0);
}
