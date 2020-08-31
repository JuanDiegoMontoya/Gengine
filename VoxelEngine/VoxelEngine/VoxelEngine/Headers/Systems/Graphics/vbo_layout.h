#pragma once

struct VBOElement
{
	GLuint type;
	GLuint count;
	GLuint normalized;

	static GLuint TypeSize(GLuint type)
	{
		switch (type)
		{
		case GL_FLOAT:				 return sizeof(GLfloat);
		case GL_UNSIGNED_INT:  return sizeof(GLuint);
		case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
		}

		ASSERT(false);
		return 0;
	}
};

class VBOlayout
{
public:
	VBOlayout() : stride_(0) {}

	inline GLuint GetStride() const { return stride_; }
	inline const std::vector<VBOElement>& GetElements() const { return elements_; }

	template<typename T>
	void Push(unsigned int count)
	{
		ASSERT_MSG(false, "Invalid type specified."); // crash
	}

	// template specializations for Push()
	template<>
	void Push<float>(unsigned int count)
	{
		elements_.push_back({ GL_FLOAT, count, GL_FALSE });
		stride_ += count * VBOElement::TypeSize(GL_FLOAT);
	}

	template<>
	void Push<GLuint>(unsigned int count)
	{
		elements_.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		stride_ += count * VBOElement::TypeSize(GL_UNSIGNED_INT);
	}

	template<>
	void Push<GLubyte>(unsigned int count)
	{
		elements_.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		stride_ += count * VBOElement::TypeSize(GL_UNSIGNED_BYTE);
	}

private:
	std::vector<VBOElement> elements_;
	GLuint stride_;
};