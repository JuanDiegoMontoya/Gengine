#pragma once

class UBO
{
public:
	UBO(const void* data, GLuint size);
	~UBO();

	void Bind(GLuint index) const;
	void Unbind() const;

	GLuint GetID() { return rendererID_; }

	static constexpr int ReservedSlot = 0;
private:
	GLuint rendererID_;
};