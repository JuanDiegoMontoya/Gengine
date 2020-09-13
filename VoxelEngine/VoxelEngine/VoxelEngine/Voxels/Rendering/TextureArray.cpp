#include "stdafx.h"
#include "TextureArray.h"
#include <stb_image.h>
#include <filesystem>

#pragma optimize("", off)
TextureArray::TextureArray(const std::vector<std::string>& textures)
{
	const GLsizei layerCount = textures.size();
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipCount_, GL_RGBA8, dim, dim, layerCount);

	stbi_set_flip_vertically_on_load(true);

	int i = 0;
	for (auto texture : textures)
	{
		std::string tex = texPath + texture;
		bool hasTex = std::filesystem::exists(texPath + texture);

		// TODO: gen mipmaps (increment mip level each mip iteration)
		if (hasTex == false)
		{
			std::cout << "Failed to load texture " << texture << ", using fallback.\n";
			tex = texPath + "error.png";
		}

		int width, height, n;
		auto pixels = (unsigned char*)stbi_load(tex.c_str(), &width, &height, &n, 4);
		ASSERT(pixels != nullptr);
		ASSERT(width == dim && height == dim);

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,           // mip level 0
			0, 0, i,     // image start layer
			dim, dim, 1, // x, y, z size (z = 1 b/c it's just a single layer)
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixels);

		stbi_image_free(pixels);

		i++;
	}

	// sets the anisotropic filtering texture paramter to the highest supported by the system
	// TODO: make this parameter user-selectable
	GLfloat a;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &a);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY, a);

	// TODO: play with this parameter for optimal looks, maybe make it user-selectable
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// use OpenGL to generate mipmaps for us
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
#pragma optimize("", on)


TextureArray::~TextureArray()
{
	glDeleteTextures(1, &id_);
}


void TextureArray::Bind(GLuint slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
}


void TextureArray::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
