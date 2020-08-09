#include "stdafx.h"
#include "transform.h"

Transform::Transform(const glm::vec3& pos, const glm::vec3& scale)
{
	translation_ = pos;
	rotation_ = glm::mat4(1.f);
	scale_ = scale;
	model_ = glm::mat4(1.0f);
	dirty_ = true;
	SetType(ComponentType::cTransform);
}

Transform::Transform(const Transform& other)
{
	*this = other;
	SetType(ComponentType::cTransform);
}

Component* Transform::Clone() const
{
	Transform* tran = new Transform(*this);
	tran->SetType(ComponentType::cTransform);
	return tran;
}

void Transform::SetTranslation(const glm::vec3& translation)
{
	translation_ = translation;
	dirty_ = true;
}

void Transform::SetRotation(const glm::mat4& rotation)
{
	rotation_ = rotation;
	dirty_ = true;
}

void Transform::SetScale(const glm::vec3& scale)
{
	scale_ = scale;
	dirty_ = true;
}

// computes the model matrix if necessary, then returns it
const glm::mat4& Transform::GetModel() const
{
	if (dirty_)
	{
		model_ = glm::mat4(1.0f);
		model_ = glm::translate(model_, translation_);
		model_ *= rotation_;
		model_ = glm::scale(model_, scale_);

		dirty_ = false;
	}

	return model_;
}