#pragma once

namespace Vertices
{
	static const GLfloat cube_vertices_light[] =
	{
		// f
		-.5f,-.5f,.5f, // lbf
		 .5f,-.5f,.5f, // rbf
		-.5f, .5f,.5f, // ltf
		 .5f, .5f,.5f, // rtf

		// n
		-.5f,-.5f,-.5f, // lbn
		 .5f,-.5f,-.5f, // rbn
		-.5f, .5f,-.5f, // ltn
		 .5f, .5f,-.5f, // rtn

		// l
		-.5f,-.5f,-.5f, // lbn
		-.5f,-.5f, .5f, // lbf
		-.5f, .5f,-.5f, // ltn
		-.5f, .5f, .5f, // ltf

		// r
		 .5f,-.5f,-.5f, // rbn
		 .5f,-.5f, .5f, // rbf
		 .5f, .5f,-.5f, // rtn
		 .5f, .5f, .5f, // rtf

		// t
		-.5f, .5f,-.5f, // ltn
		-.5f, .5f, .5f, // ltf
		 .5f, .5f,-.5f, // rtn
		 .5f, .5f, .5f, // rtf

		// b
		-.5f,-.5f,-.5f, // lbn
		-.5f,-.5f, .5f, // lbf
		 .5f,-.5f,-.5f, // rbn
		 .5f,-.5f, .5f, // rbf
	};

	static const float cube[] =
	{
		// Back face (-Z)
		-0.5f, -0.5f, -0.5f, // Bottom-left
		 0.5f,  0.5f, -0.5f, // top-right
		 0.5f, -0.5f, -0.5f, // bottom-right         
		 0.5f,  0.5f, -0.5f, // top-right
		-0.5f, -0.5f, -0.5f, // bottom-left
		-0.5f,  0.5f, -0.5f, // top-left

		// Front face (+Z)
		-0.5f, -0.5f,  0.5f, // bottom-left
		 0.5f, -0.5f,  0.5f, // bottom-right
		 0.5f,  0.5f,  0.5f, // top-right
		 0.5f,  0.5f,  0.5f, // top-right
		-0.5f,  0.5f,  0.5f, // top-left
		-0.5f, -0.5f,  0.5f, // bottom-left

		// Left face (-X)
		-0.5f,  0.5f,  0.5f, // top-right
		-0.5f,  0.5f, -0.5f, // top-left
		-0.5f, -0.5f, -0.5f, // bottom-left
		-0.5f, -0.5f, -0.5f, // bottom-left
		-0.5f, -0.5f,  0.5f, // bottom-right
		-0.5f,  0.5f,  0.5f, // top-right

		// Right face (+X)
		 0.5f,  0.5f,  0.5f, // top-left
		 0.5f, -0.5f, -0.5f, // bottom-right
		 0.5f,  0.5f, -0.5f, // top-right         
		 0.5f, -0.5f, -0.5f, // bottom-right
		 0.5f,  0.5f,  0.5f, // top-left
		 0.5f, -0.5f,  0.5f, // bottom-left     

		// Bottom face (-Y)
		-0.5f, -0.5f, -0.5f, // top-right
		 0.5f, -0.5f, -0.5f, // top-left
		 0.5f, -0.5f,  0.5f, // bottom-left
		 0.5f, -0.5f,  0.5f, // bottom-left
		-0.5f, -0.5f,  0.5f, // bottom-right
		-0.5f, -0.5f, -0.5f, // top-right

		// Top face (+Y)
		-0.5f,  0.5f, -0.5f, // top-left
		 0.5f,  0.5f,  0.5f, // bottom-right
		 0.5f,  0.5f, -0.5f, // top-right     
		 0.5f,  0.5f,  0.5f, // bottom-right
		-0.5f,  0.5f, -0.5f, // top-left
		-0.5f,  0.5f,  0.5f  // bottom-left    
	};

	static const float skybox[] =
	{
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// positions, normals, texture coords
	static const float cube_norm_tex[] =
	{
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
	};

	static const glm::vec3 cube_normals_divisor2[] =
	{
		{ 0.0f,  0.0f, -1.0f },

		{ 0.0f,  0.0f,  1.0f },

		{-1.0f,  0.0f,  0.0f },

		{ 1.0f,  0.0f,  0.0f },

		{ 0.0f, -1.0f,  0.0f },

		{ 0.0f,  1.0f,  0.0f }
	};

	// positions, texture coords
	static const float cube_tex[] =
	{
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left

		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left

		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right

		// Right face
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     

		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right

		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
	};

	// positions
	static const float pyramid_square[] =
	{
		// Triangle 1
		0.0, 1.0, 0.0,
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,

		//Triangle 2
		0.0, 1.0, 0.0,
		1.0, -1.0, 1.0,
		1.0, -1.0, -1.0,

		//Triangle 3
		0.0, 1.0, 0.0,
		1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,

		//Triangle 4
		0.0, 1.0, 0.0,
		-1.0, -1.0, -1.0,
		-1.0, -1.0, 1.0,
	};

	static const float square_vertices[] =
	{
		 0.0f,  0.5f,
		 0.5f, -0.5f,
		-0.5f, -0.5f
	};

	static const float square_vertices_3d[] =
	{
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
	};

	// f n l r t b
	static const GLubyte cube_indices_light_ccw[] =
	{
		0, 2, 1, 2, 3, 1, // f
		0, 1, 2, 1, 3, 2, // n
		0, 2, 1, 1, 2, 3, // l
		0, 1, 2, 1, 3, 2, // r
		0, 2, 1, 2, 3, 1, // t
		0, 1, 2, 1, 3, 2  // b
	};

	static const GLubyte cube_indices_light_cw[] =
	{
		//1, 2, 0, 1, 3, 2, // f
		//2, 1, 0, 2, 3, 1, // n
		//1, 2, 0, 3, 2, 1, // l
		//2, 1, 0, 2, 3, 1, // r
		//0, 1, 3, 3, 1, 2, // t
		//2, 1, 0, 2, 3, 1  // b
		0, 1, 3, 3, 1, 2, // t
		0, 1, 3, 3, 1, 2, // t
		0, 1, 3, 3, 1, 2, // t
		0, 1, 3, 3, 1, 2, // t
		0, 1, 3, 3, 1, 2, // t
		0, 1, 3, 3, 1, 2, // t
	};

	static const GLubyte cube_indices_light_cw_anisotropic[] =
	{
		//0, 1, 3, 3, 2, 0, // f
		//0, 3, 1, 0, 2, 3, // n
		//0, 1, 3, 3, 2, 0, // l
		//0, 3, 1, 0, 2, 3, // r
		//0, 1, 2, 2, 3, 0, // t
		//1, 0, 3, 3, 0, 2  // b
		0, 1, 2, 2, 3, 0, // t
		0, 1, 2, 2, 3, 0, // t
		0, 1, 2, 2, 3, 0, // t
		0, 1, 2, 2, 3, 0, // t
		0, 1, 2, 2, 3, 0, // t
		0, 1, 2, 2, 3, 0, // t
	};

	// f n l r t b
	static const GLfloat cube_light[] =
	{
		// f
		 .5f,-.5f,.5f, // rbf
		 .5f, .5f,.5f, // rtf
		-.5f, .5f,.5f, // ltf
		-.5f,-.5f,.5f, // lbf

		// n
		-.5f,-.5f,-.5f, // lbn
		-.5f, .5f,-.5f, // ltn
		 .5f, .5f,-.5f, // rtn
		 .5f,-.5f,-.5f, // rbn

		// l
		-.5f,-.5f, .5f, // lbf
		-.5f, .5f, .5f, // ltf
		-.5f, .5f,-.5f, // ltn
		-.5f,-.5f,-.5f, // lbn

		// r
		 .5f,-.5f,-.5f, // rbn
		 .5f, .5f,-.5f, // rtn
		 .5f, .5f, .5f, // rtf
		 .5f,-.5f, .5f, // rbf

		// t
		-.5f, .5f,-.5f, // ltn
		-.5f, .5f, .5f, // ltf
		 .5f, .5f, .5f, // rtf
		 .5f, .5f,-.5f, // rtn

		// b
		-.5f,-.5f, .5f, // lbf
		-.5f,-.5f,-.5f, // lbn
		 .5f,-.5f,-.5f, // rbn
		 .5f,-.5f, .5f, // rbf
	};
}