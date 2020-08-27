#pragma once

#define PERSISTENCE false

// atomic counter buffer object
class ABO
{
public:
	ABO(GLuint num);
	~ABO();

	void Bind(GLuint index);
	void Unbind();
	void Reset(); // set all counters to 0
	void Set(GLuint index, GLuint value);

	GLuint Get(GLuint index);
	GLuint GetID() { return rendererID_; }
	GLuint GetSize() { return numCounters_; }
private:
	GLuint rendererID_;
	GLuint numCounters_;
#if PERSISTENCE
	GLuint* rbuffer; // persistently mapped
#endif
};