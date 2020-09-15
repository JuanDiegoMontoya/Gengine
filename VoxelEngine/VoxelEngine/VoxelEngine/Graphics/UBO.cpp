#include <Graphics/GraphicsIncludes.h>
#include <Graphics/UBO.h>

UBO::UBO(const void* data, GLuint size)
{
	glCreateBuffers(1, &rendererID_);
	glNamedBufferStorage(rendererID_, size, data, GL_DYNAMIC_STORAGE_BIT);
}

UBO::~UBO()
{
	glDeleteBuffers(1, &rendererID_);
}

void UBO::Bind(GLuint index) const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, index, rendererID_);
	glBindBuffer(GL_UNIFORM_BUFFER, rendererID_);
}

void UBO::Unbind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
