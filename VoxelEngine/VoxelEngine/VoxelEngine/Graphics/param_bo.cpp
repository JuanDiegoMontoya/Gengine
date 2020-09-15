#include <Graphics/GraphicsIncludes.h>

Param_BO::Param_BO()
{
	glGenBuffers(1, &id_);
	glBindBuffer(GL_PARAMETER_BUFFER, id_);
	GLsizei val{ 0 };
	//glBufferData(GL_PARAMETER_BUFFER, sizeof(GLsizei), &val, GL_STATIC_READ);
	glBufferStorage(GL_PARAMETER_BUFFER,
		sizeof(GLsizei), &val,
		GL_DYNAMIC_STORAGE_BIT);
}

Param_BO::~Param_BO()
{
	glDeleteBuffers(1, &id_);
}

void Param_BO::Reset()
{
	glBindBuffer(GL_PARAMETER_BUFFER, id_);
	GLsizei val{ 0 };
	glBufferSubData(GL_PARAMETER_BUFFER, 0, sizeof(GLsizei), &val);
}

void Param_BO::Bind() const
{
	glBindBuffer(GL_PARAMETER_BUFFER, id_);
}

void Param_BO::Unbind() const
{
	glBindBuffer(GL_PARAMETER_BUFFER, 0);
}
