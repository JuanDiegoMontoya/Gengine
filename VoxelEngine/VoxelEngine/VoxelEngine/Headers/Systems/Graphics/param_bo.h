#pragma once

// parameter buffer object
// designed for use with glMultiDrawArraysIndirectCount
class Param_BO
{
public:
	Param_BO();
	~Param_BO();

	void Reset();

	void Bind() const;
	void Unbind() const;

	GLuint GetID() const { return id_; }

private:
	GLuint id_;
};