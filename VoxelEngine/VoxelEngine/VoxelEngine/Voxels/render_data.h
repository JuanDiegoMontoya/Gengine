#pragma once

typedef class RenderData : public Component
{
public:
	RenderData() { SetType(ComponentType::cRenderData); }
	~RenderData() {}
	Component* Clone() const override;

	void UseUntexturedBlockData(); // draw as unreflective, colored block

	inline void SetIsTextured(bool b) { isTextured_ = b; }
	inline void SetTexture(Texture* t) { texture_ = t; }
	inline void SetColor(glm::vec4 c) { color_ = c; }

	inline bool GetIsTextured() const { return isTextured_; }
	inline glm::vec4 GetColor() const { return color_; }
	inline const VAO& GetVao() { return *vao_; }
	inline const VBO& GetVbo() { return *vbo_; }
	inline const IBO& GetIbo() { return *ibo_; }
	inline class Shader* GetShader() { return shader_; } // not const so we can modify it

	static const ComponentType ctype = ComponentType::cRenderData;

private:
	// do NOT delete these upon destruction of this object
	VAO* vao_ = nullptr;
	VBO* vbo_ = nullptr;
	IBO* ibo_ = nullptr;

	Texture* texture_ = nullptr;
	class Shader* shader_ = nullptr;
	bool isTextured_ = false;
	glm::vec4 color_ = glm::vec4(1.f); // if not textured

}RenderData, *RenderDataPtr;