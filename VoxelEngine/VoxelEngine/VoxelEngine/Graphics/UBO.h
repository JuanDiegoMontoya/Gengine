#pragma once

class UBO
{
public:
	UBO(const void* data, GLuint size);
	~UBO();

	void Bind(GLuint location) const;
	void Unbind() const;

	GLuint GetID() { return rendererID_; }
private:
	GLuint rendererID_;
};