#pragma once
#include <Rendering/directional_light.h>

class VAO;
class VBO;
class Frustum;

// right, left, bottom, top, far, near
struct BasicFrustum
{
	float r, l, b, t, f, n;
};

class Sun
{
public:
	Sun();

	void Update();
	//void Init();
	void Render();

	// getters
	inline const glm::vec3& GetDir() const { return dir_; }
	inline const glm::vec3& GetPos() const { return pos_; }
	inline const glm::mat4& GetView() { return view_; }
	inline DirLight& GetDirLight() { return dirLight_; }

	// setters
	inline void SetPos(const glm::vec3& pos) { pos_ = pos; }
	inline void SetDir(const glm::vec3& dir) { dir_ = dir; }

	bool orbits = false;
	glm::vec3 orbitPos = glm::vec3(0);

	bool followCam = true;
	float followDist = 250.f;
private:
	DirLight dirLight_;
	glm::vec3 dir_;
	glm::vec3 pos_; // position relative to camera

	// shadow related
	glm::mat4 sunViewProj_;
	glm::mat4 view_;

	VAO* vao_;
	VBO* vbo_;
};