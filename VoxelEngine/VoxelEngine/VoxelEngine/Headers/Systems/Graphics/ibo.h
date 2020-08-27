#pragma once

class IBO
{
public:
	IBO(const GLuint* data, unsigned int count);
	~IBO();

	void Bind() const;
	void Unbind() const;

	inline GLuint GetCount() const { return count_; }

private:
	GLuint rendererID_;
	GLuint count_;
};