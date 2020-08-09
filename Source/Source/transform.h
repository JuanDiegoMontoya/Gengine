#pragma once
#include "component.h"

typedef class Transform : public Component
{
public:
	Transform(const glm::vec3& pos = glm::vec3(0.0f), 
		const glm::vec3& scale = glm::vec3(1.0f));
	Transform(const Transform & other);
	~Transform() {};

	void SetTranslation(const glm::vec3& translation);
	void SetRotation(const glm::mat4& rotation);
	void SetScale(const glm::vec3& scale);

	inline const glm::vec3& GetTranslation() const { return translation_; };
	inline const glm::mat4& GetRotation() const { return rotation_; };
	inline const glm::vec3& GetScale() const { return scale_; };
	const glm::mat4& GetModel() const;
	Component* Clone() const override;

	static const ComponentType ctype = ComponentType::cTransform;

private:
	glm::vec3	translation_;
	glm::mat4 rotation_;
	glm::vec3	scale_;

	mutable glm::mat4	model_;
	mutable bool			dirty_;

}Transform, TransformPtr;