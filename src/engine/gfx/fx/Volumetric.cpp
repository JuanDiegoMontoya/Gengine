#include "../../PCH.h"
#include "Volumetric.h"
#include <glad/glad.h>
#include <engine/gfx/resource/ShaderManager.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utility/Timer.h>

namespace GFX::FX::Volumetric
{
  namespace
  {
    glm::mat4 MakeVolumeProjection(GFX::PerspectiveProjectionInfo projInfo, float nearPlane, float farPlane)
    {
      return glm::perspectiveZO(projInfo.fovyRadians, projInfo.aspectRatio, nearPlane, farPlane);
    }
  }

  void CompileShaders()
  {
    ShaderManager::AddShader("volume_accumulate",
      {
        { "volumetric/accumulate.cs.glsl", ShaderType::COMPUTE }
      });
    ShaderManager::AddShader("volume_march",
      {
        { "volumetric/march.cs.glsl", ShaderType::COMPUTE }
      });
    ShaderManager::AddShader("volume_apply_deferred",
      {
        { "volumetric/applyDeferred.cs.glsl", ShaderType::COMPUTE }
      });
  }

  void ResetVolume(const TextureView& volume)
  {
    ASSERT(volume.CreateInfo().viewType == ImageType::TEX_3D);
    Extent3D dim = volume.Extent();
    GLfloat data[4]{ 0, 0, 0, 0 };
    glClearTexSubImage(volume.GetAPIHandle(), 0,
      0, 0, 0,
      dim.width, dim.height, dim.depth,
      GL_RGBA, GL_FLOAT, data);
  }

  void Accumulate(AccumulateParameters params)
  {
    ASSERT(params.densityVolume.CreateInfo().viewType == ImageType::TEX_3D);
    Extent3D targetDim = params.densityVolume.Extent();
    glm::mat4 proj = MakeVolumeProjection(params.common.camera.projInfo.info, params.common.nearPlane, params.common.farPlane);

    auto shader = ShaderManager::GetShader("volume_accumulate");
    shader->Bind();
    shader->SetIVec3("u_targetDim", { targetDim.width, targetDim.height, targetDim.depth });
    shader->SetMat4("u_invViewProj", glm::inverse(proj * params.common.camera.viewInfo.GetViewMatrix()));
    shader->SetFloat("u_time", ProgramTimer::TimeSeconds());
    shader->SetFloat("u_volNearPlane", params.common.nearPlane);
    shader->SetFloat("u_volFarPlane", params.common.farPlane);
    BindImage(0, params.densityVolume, 0);

    const int local_size = 8;
    Extent3D numGroups = (targetDim + local_size - 1) / local_size;
    glDispatchCompute(numGroups.width, numGroups.height, numGroups.depth);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  void March(MarchParameters params)
  {
    ASSERT(params.targetVolume.CreateInfo().viewType == ImageType::TEX_3D);
    ASSERT(params.sourceVolume.CreateInfo().viewType == ImageType::TEX_3D);
    Extent3D targetDim = params.targetVolume.Extent();
    Extent3D sourceDim = params.sourceVolume.Extent();
    glm::mat4 proj = MakeVolumeProjection(params.common.camera.projInfo.info, params.common.nearPlane, params.common.farPlane);

    auto shader = ShaderManager::GetShader("volume_march");
    shader->Bind();
    shader->SetIVec3("u_targetDim", { targetDim.width, targetDim.height, targetDim.depth });
    shader->SetVec3("u_viewPos", params.common.camera.viewInfo.position);
    shader->SetMat4("u_invViewProj", glm::inverse(proj * params.common.camera.viewInfo.GetViewMatrix()));
    shader->SetFloat("u_volNearPlane", params.common.nearPlane);
    shader->SetFloat("u_volFarPlane", params.common.farPlane);
    BindTextureView(0, params.sourceVolume, params.scratchSampler);
    BindImage(0, params.targetVolume, 0);

    const int local_size = 16;
    Extent2D numGroups = (targetDim + local_size - 1) / local_size;
    glDispatchCompute(numGroups.width, numGroups.height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  void ApplyDeferred(ApplyParameters params)
  {
    ASSERT(params.sourceVolume.CreateInfo().viewType == ImageType::TEX_3D);
    ASSERT(params.targetTexture.Extent() == params.colorTexture.Extent() && params.targetTexture.Extent() == params.depthTexture.Extent());
    Extent3D targetDim = params.targetTexture.Extent();
    glm::mat4 proj = MakeVolumeProjection(params.common.camera.projInfo.info, params.common.nearPlane, params.common.farPlane);

    SamplerState samplerState{};
    samplerState.asBitField.magFilter = Filter::LINEAR;
    samplerState.asBitField.minFilter = Filter::LINEAR;
    samplerState.asBitField.mipmapFilter = Filter::NONE;
    params.scratchSampler.SetState(samplerState);

    auto shader = ShaderManager::GetShader("volume_apply_deferred");
    shader->Bind();
    shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
    shader->SetMat4("u_viewProjVolume", proj * params.common.camera.viewInfo.GetViewMatrix());
    shader->SetMat4("u_invViewProjScene", glm::inverse(params.common.camera.GetViewProj()));
    shader->SetFloat("u_volNearPlane", params.common.nearPlane);
    shader->SetFloat("u_volFarPlane", params.common.farPlane);
    BindTextureView(0, params.colorTexture, params.scratchSampler);
    BindTextureView(1, params.depthTexture, params.scratchSampler);
    BindTextureView(2, params.sourceVolume, params.scratchSampler);
    BindTextureView(3, params.blueNoiseTexture, params.scratchSampler);
    BindImage(0, params.targetTexture, 0);

    const int local_size = 16;
    Extent2D numGroups = (targetDim + local_size - 1) / local_size;
    glDispatchCompute(numGroups.width, numGroups.height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
}