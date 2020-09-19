#pragma once
#include <Graphics/GraphicsIncludes.h>

class VAO;
class VBO;

/*
	Orthographic parallel-rays light (such as the sun)
	with cascaded shadows. 
*/
class DirLight
{
public:
	DirLight();

	void Update(const glm::vec3& pos, const glm::vec3& dir);

	// getters
	inline const glm::vec3& GetDir() const { return tDir_; }
	inline const glm::vec3& GetPos() const { return tPos_; }
	inline const glm::ivec2& GetShadowSize() const { return shadowSize_; }
	inline const glm::mat4& GetView() { return view_; }
	inline const GLuint GetDepthFBO() const { return depthMapFBO_; }
	inline const glm::uvec3& GetDepthTex() const { return depthMapTexes_; }
	inline const unsigned GetNumCascades() const { return shadowCascades_; }
	inline const glm::vec4& GetCascadeEnds() const { return cascadeEnds_; }
	inline const glm::mat4* GetShadowOrthoProjMtxs() const { return shadowOrthoProjMtxs_; }

	// special
	glm::mat4 GetProjMat(glm::mat4& view, unsigned int index)
	{
		return glm::ortho(
			(view * modeldFrusCorns[index][1]).x, 
			(view * modeldFrusCorns[index][0]).x, 
			(view * modeldFrusCorns[index][2]).y, 
			(view * modeldFrusCorns[index][0]).y, 
			(view * modeldFrusCorns[index][0]).z, 
			(view * modeldFrusCorns[index][4]).z);
	}

	float GetRatio(glm::mat4& view, int index)
	{
		if (index > -1 && index < 4)
		{
			return((-(view*modeldFrusCorns[index][0]).z + (view*modeldFrusCorns[index][4]).z));
		}
		else
		{
			return 1.0f;
		}
	}
	glm::vec3 GetModlCent(unsigned int index)
	{
		glm::vec4 temp = glm::vec4(0.0f);
		for (unsigned int i = 0; i < 8; ++i)
		{
			temp += modeldFrusCorns[index][i];
		}
		glm::vec4 temp2 = temp / 8.0f;
		return glm::vec3(temp2);
	}

	// setters
	inline void SetDir(const glm::vec3& dir) { tDir_ = dir; }
	inline void SetPos(const glm::vec3& pos) { tPos_ = pos; }
	inline void SetShadowSize(const glm::ivec2& s) { shadowSize_ = s; }
	inline void SetNumCascades(unsigned num) { shadowCascades_ = num; }

	void bindForWriting(unsigned index);
	void bindForReading();
	void calcOrthoProjs(const glm::mat4& vView);
	void calcPersProjs();
private:
	// functions
	void initCascadedShadowMapFBO();

	// vars
	glm::vec3 tDir_;
	glm::vec3 tPos_;
	glm::mat4 view_;
	glm::ivec2 shadowSize_ = glm::ivec2(4096, 4096);
	GLuint depthMapFBO_;
	unsigned shadowCascades_ = 3; // 3 = max (uvec3)
	glm::uvec3 depthMapTexes_;
	glm::vec4 cascadeEnds_;
	
	glm::vec4 modeldFrusCorns[3][8];
	glm::mat4 shadowOrthoProjMtxs_[3];
};