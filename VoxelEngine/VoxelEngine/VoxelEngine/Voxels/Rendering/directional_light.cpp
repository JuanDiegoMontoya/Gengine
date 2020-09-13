#include <Components/Camera.h>
#include <Rendering/Renderer.h>
#include <Refactor/settings.h>
#include <Rendering/directional_light.h>
#include <MathIncludes.h>

DirLight::DirLight()
{
	initCascadedShadowMapFBO();
}

void DirLight::Update(const glm::vec3& pos, const glm::vec3& dir)
{
	tPos_ = pos;
	tDir_ = dir;

	view_ = glm::lookAt(tPos_, -glm::normalize(Renderer::GetPipeline()->GetCamera(0)->GetPos()), glm::vec3(0.0, 1.0, 0.0));
	//view_ = glm::lookAt(-tPos_, glm::vec3(0), glm::vec3(0.0, 1.0, 0.0));

	// equally spaced cascade ends (may change in future)
	float persNear = Renderer::GetPipeline()->GetCamera(0)->GetNear();
	float persFar = Renderer::GetPipeline()->GetCamera(0)->GetFar();
	cascadeEnds_[0] = persNear;
	cascadeEnds_[1] = 25.f;
	cascadeEnds_[2] = 70.f;
	cascadeEnds_[3] = persFar;
	//calcOrthoProjs();
	//calcPersProjs();
}

void DirLight::initCascadedShadowMapFBO()
{
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO_);
	// create depth texture
	glGenTextures(shadowCascades_, &depthMapTexes_[0]);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	//float borderColor[] = { 0, 0, 0, 1 };
	for (unsigned i = 0; i < shadowCascades_; i++)
	{
		glBindTexture(GL_TEXTURE_2D, depthMapTexes_[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowSize_.x, shadowSize_.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, shadowSize_.x, shadowSize_.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexes_[0], 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
		"Shadow cascade framebuffer incomplete.");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DirLight::bindForWriting(unsigned index)
{
	ASSERT(index < shadowCascades_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthMapFBO_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexes_[index], 0);
}

void DirLight::bindForReading()
{
	//switch (shadowCascades_) // fall-through intended
	//{
	//case 3:
	//	glActiveTexture(2);
	//	glBindTexture(GL_TEXTURE_2D, depthMapTexes_[2]);
	//case 2:
	//	glActiveTexture(1);
	//	glBindTexture(GL_TEXTURE_2D, depthMapTexes_[1]);
	//case 1:
	//	glActiveTexture(0);
	//	glBindTexture(GL_TEXTURE_2D, depthMapTexes_[0]);
	//}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMapTexes_[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMapTexes_[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMapTexes_[2]);
}

void DirLight::calcOrthoProjs(const glm::mat4& vView)
{
	// Get the inverse of the view transform
	glm::mat4 Cam = Renderer::GetPipeline()->GetCamera(0)->GetView();
	//glm::mat4 Cam = glm::lookAt(tPos_, Render::GetCamera()->GetDir(), glm::vec3(0, 1, 0));
	glm::mat4 CamInv = glm::inverse(Cam);

	// Get the light space tranform
	//glm::mat4 LightM = view_;
	glm::mat4 LightM = vView;// *glm::translate(glm::mat4(1), tPos_);

	float ar = (float)Settings::Graphics.screenX / (float)Settings::Graphics.screenY;
	float fov = Renderer::GetPipeline()->GetCamera(0)->GetFov(); // degrees
	float tanHalfHFOV = glm::tan(glm::radians(fov / 2.0f)) / ar;
	float tanHalfVFOV = glm::tan(glm::radians((fov * ar) / 2.0f)) / ar;

	for (unsigned i = 0; i < shadowCascades_; i++)
	{
		float xn = cascadeEnds_[i] * tanHalfHFOV;
		float xf = cascadeEnds_[i + 1] * tanHalfHFOV;
		float yn = cascadeEnds_[i] * tanHalfVFOV;
		float yf = cascadeEnds_[i + 1] * tanHalfVFOV;

		glm::vec4 frustumCorners[8] =
		{
			// near face
			glm::vec4(xn, yn, -cascadeEnds_[i], 1.0),
			glm::vec4(-xn, yn, -cascadeEnds_[i], 1.0),
			glm::vec4(xn, -yn, -cascadeEnds_[i], 1.0),
			glm::vec4(-xn, -yn, -cascadeEnds_[i], 1.0),

			// far face
			glm::vec4(xf, yf, -cascadeEnds_[i + 1], 1.0),
			glm::vec4(-xf, yf, -cascadeEnds_[i + 1], 1.0),
			glm::vec4(xf, -yf, -cascadeEnds_[i + 1], 1.0),
			glm::vec4(-xf, -yf, -cascadeEnds_[i + 1], 1.0)
		};

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		for (unsigned j = 0; j < 8; j++)
		{
			// Transform the frustum coordinate from view to world space
			glm::vec4 vW = CamInv * frustumCorners[j];

			// Transform the frustum coordinate from world to light space
			frustumCorners[j] = LightM * vW;

			minX = glm::min(minX, frustumCorners[j].x);
			maxX = glm::max(maxX, frustumCorners[j].x);
			minY = glm::min(minY, frustumCorners[j].y);
			maxY = glm::max(maxY, frustumCorners[j].y);
			minZ = glm::min(minZ, frustumCorners[j].z);
			maxZ = glm::max(maxZ, frustumCorners[j].z);

			if (j == 7)
			{
				modeldFrusCorns[i][0] = glm::inverse(LightM) * glm::vec4(maxX, maxY, minZ, 1.0f);
				modeldFrusCorns[i][1] = glm::inverse(LightM) * glm::vec4(minX, maxY, minZ, 1.0f);
				modeldFrusCorns[i][2] = glm::inverse(LightM) * glm::vec4(maxX, minY, minZ, 1.0f);
				modeldFrusCorns[i][3] = glm::inverse(LightM) * glm::vec4(minX, minY, minZ, 1.0f);
				modeldFrusCorns[i][4] = glm::inverse(LightM) * glm::vec4(maxX, maxY, maxZ, 1.0f);
				modeldFrusCorns[i][5] = glm::inverse(LightM) * glm::vec4(minX, maxY, maxZ, 1.0f);
				modeldFrusCorns[i][6] = glm::inverse(LightM) * glm::vec4(maxX, minY, maxZ, 1.0f);
				modeldFrusCorns[i][7] = glm::inverse(LightM) * glm::vec4(minX, minY, maxZ, 1.0f);
			}
		}
	}
}

void DirLight::calcPersProjs()
{
	/*
	float fov_ = 80.f;
	float near_ = .1f;
	float far_ = 500.f;
	*/
	for (unsigned i = 0; i < shadowCascades_; i++)
	{
		shadowOrthoProjMtxs_[i] = glm::perspective(glm::radians(80.f), 1920.f / 1080.f, cascadeEnds_[i], cascadeEnds_[i + 1]) * Renderer::GetPipeline()->GetCamera(0)->GetView();
	}
}
