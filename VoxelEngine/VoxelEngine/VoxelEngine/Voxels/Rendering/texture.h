#pragma once

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	void Bind(GLuint slot = 0) const;
	void Unbind() const;

	inline GLint GetWidth() const { return width_; }
	inline GLint GetHeight() const { return height_; }
	inline GLint GetID() const { return rendererID_; }

	inline void SetType(std::string ty) { type_ = ty; }
	inline const std::string& GetType() const { return type_; }

private:
	GLuint rendererID_;
	std::string filepath_;
	std::string type_;
	GLubyte* localbuffer_;
	GLint width_, height_, BPP_;
	static const char* texture_dir_;
};