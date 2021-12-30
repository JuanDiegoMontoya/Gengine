#include "../../PCH.h"
#include "CubemapReflections.h"
#include <engine/gfx/resource/ShaderManager.h>
#include "../../GAssert.h"
#include <glad/glad.h>

namespace GFX::FX
{
  void CompileReflectionShaders()
  {
    ShaderManager::AddShader("specular_cube_trace",
      {
        { "reflections/specular_cube_trace.cs.glsl", ShaderType::COMPUTE }
      });
    ShaderManager::AddShader("specular_cube_sample",
      {
        { "reflections/specular_cube_sample.cs.glsl", ShaderType::COMPUTE }
      });
    ShaderManager::AddShader("unproject_depth",
      {
        { "reflections/unproject_depth.cs.glsl", ShaderType::COMPUTE }
      });
    ShaderManager::AddShader("specular_composite",
      {
        { "reflections/specular_composite.cs.glsl", ShaderType::COMPUTE }
      });
    ShaderManager::AddShader("atrous_reflection",
      {
        { "reflections/denoise_atrous.cs.glsl", ShaderType::COMPUTE }
      });
  }

  void SampleCubemapReflections(SampleCubemapReflectionsParameters p)
  {
    SamplerState samplerState{};
    samplerState.asBitField.minFilter = Filter::LINEAR;
    samplerState.asBitField.magFilter = Filter::LINEAR;
    samplerState.asBitField.mipmapFilter = Filter::NONE;
    samplerState.asBitField.addressModeU = AddressMode::REPEAT;
    samplerState.asBitField.addressModeV = AddressMode::REPEAT;
    p.common.scratchSampler.SetState(samplerState);

    Extent2D targetDim = p.target.Extent();
    auto shader = ShaderManager::GetShader("specular_cube_sample");
    shader->Bind();

    shader->SetMat4("u_invProj", glm::inverse(p.common.camera.proj));
    shader->SetMat4("u_invView", glm::inverse(p.common.camera.viewInfo.GetViewMatrix()));
    shader->SetVec3("u_viewPos", p.common.camera.viewInfo.position);
    shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });

    BindTextureView(0, p.common.gbDepth, p.common.scratchSampler);
    BindTextureView(1, p.common.gbColor, p.common.scratchSampler);
    BindTextureView(2, p.common.gbNormal, p.common.scratchSampler);
    BindTextureView(3, p.common.gbPBR, p.common.scratchSampler);
    BindTextureView(4, p.env, p.common.scratchSampler);
    BindTextureView(5, p.blueNoise, p.common.scratchSampler);
    BindImage(0, p.target, 0);

    const int local_size = 16;
    const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
    const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

  void TraceCubemapReflections(TraceCubemapReflectionsParameters p)
  {
    SamplerState samplerState{};
    samplerState.asBitField.minFilter = Filter::NEAREST;
    samplerState.asBitField.magFilter = Filter::NEAREST;
    samplerState.asBitField.mipmapFilter = Filter::NONE;
    samplerState.asBitField.addressModeU = AddressMode::REPEAT;
    samplerState.asBitField.addressModeV = AddressMode::REPEAT;
    p.common.scratchSampler.SetState(samplerState);

    samplerState.asBitField.minFilter = Filter::LINEAR;
    samplerState.asBitField.magFilter = Filter::LINEAR;
    p.common.scratchSampler2.SetState(samplerState);

    // read each face of the cube depth, unproject to get the distance to the camera, and write it to an RXX_FLOAT cube texture face
    {
      Extent2D targetDim = p.distanceViews[0]->Extent();
      auto shader = ShaderManager::GetShader("unproject_depth");
      shader->Bind();

      const int local_sizeaaa = 16;
      const int numGroupsXa = (targetDim.width + local_sizeaaa - 1) / local_sizeaaa;
      const int numGroupsYa = (targetDim.height + local_sizeaaa - 1) / local_sizeaaa;
      for (size_t i = 0; i < 6; i++)
      {
        BindTextureView(0, *p.depthViews[i], p.common.scratchSampler2);
        BindImage(0, *p.distanceViews[i], 0);
        shader->SetMat4("u_invProj", glm::inverse(p.cameras[i].proj));
        shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
        glDispatchCompute(numGroupsXa, numGroupsYa, 1);
      }
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    {
      Extent2D targetDim = p.target.Extent();

      auto shader = ShaderManager::GetShader("specular_cube_trace");
      shader->Bind();
      shader->SetMat4("u_invProj", glm::inverse(p.common.camera.proj));
      shader->SetMat4("u_invView", glm::inverse(p.common.camera.viewInfo.GetViewMatrix()));
      shader->SetVec3("u_viewPos", p.common.camera.viewInfo.position);
      shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });

      BindTextureView(0, p.common.gbDepth, p.common.scratchSampler);
      BindTextureView(1, p.common.gbColor, p.common.scratchSampler);
      BindTextureView(2, p.common.gbNormal, p.common.scratchSampler);
      BindTextureView(3, p.common.gbPBR, p.common.scratchSampler);
      BindTextureView(4, p.probeColor, p.common.scratchSampler);
      BindTextureView(5, p.probeDistance, p.common.scratchSampler);
      BindTextureView(6, p.skybox, p.common.scratchSampler2);
      BindTextureView(7, p.blueNoise, p.common.scratchSampler);
      BindImage(0, p.target, 0);

      const int local_size = 16;
      const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
      const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
      glDispatchCompute(numGroupsX, numGroupsY, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
  }

  void DenoiseReflections(DenoiseReflectionsParameters p)
  {
    Extent2D targetDim = p.target.Extent();

    SamplerState samplerState{};
    samplerState.asBitField.magFilter = Filter::NEAREST;
    samplerState.asBitField.minFilter = Filter::NEAREST;
    samplerState.asBitField.mipmapFilter = Filter::NONE;
    samplerState.asBitField.addressModeU = AddressMode::CLAMP_TO_EDGE;
    samplerState.asBitField.addressModeV = AddressMode::CLAMP_TO_EDGE;
    p.common.scratchSampler.SetState(samplerState);

    BindTextureView(1, p.common.gbDepth, p.common.scratchSampler);
    BindTextureView(2, p.common.gbNormal, p.common.scratchSampler);
    BindTextureView(3, p.common.gbPBR, p.common.scratchSampler);

    auto shader = ShaderManager::GetShader("atrous_reflection");
    shader->Bind();
    shader->SetFloat("u_nPhi", p.atrousParams.nPhi);
    shader->SetFloat("u_pPhi", p.atrousParams.pPhi);
    shader->SetFloat("u_stepwidth", p.atrousParams.stepWidth);
    shader->SetMat4("u_invViewProj", glm::inverse(p.common.camera.GetViewProj()));
    shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
    shader->Set1FloatArray("u_kernel[0]", p.atrousParams.kernel);

    const int local_size = 16;
    const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
    const int numGroupsY = (targetDim.height + local_size - 1) / local_size;

    for (uint32_t i = 0; i < p.atrousParams.passes; i++)
    {
      float offsets2[5]{};
      for (int j = 0; j < 5; j++)
      {
        offsets2[j] = p.atrousParams.offsets[j] * glm::pow(2.0f, i);
      }

      // fake separable a-trous wavelet filter
      // technically incorrect, but looks good enough
      shader->Set1FloatArray("u_offsets[0]", offsets2);
      shader->SetBool("u_horizontal", false);
      BindTextureView(0, p.target, p.common.scratchSampler);
      BindImage(0, p.scratchTexture, 0);
      glDispatchCompute(numGroupsX, numGroupsY, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

      shader->SetBool("u_horizontal", true);
      BindTextureView(0, p.scratchTexture, p.common.scratchSampler);
      BindImage(0, p.target, 0);
      glDispatchCompute(numGroupsX, numGroupsY, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
  }

  void CompositeReflections(CompositeReflectionsParameters p)
  {
    Extent2D sourceDim = p.source.Extent();
    Extent2D targetDim = p.target.Extent();

    SamplerState samplerState{};
    samplerState.asBitField.magFilter = Filter::NEAREST;
    samplerState.asBitField.minFilter = Filter::NEAREST;
    samplerState.asBitField.mipmapFilter = Filter::NONE;
    p.common.scratchSampler.SetState(samplerState);

    auto shader = ShaderManager::GetShader("specular_composite");
    shader->Bind();
    shader->SetMat4("u_invProj", glm::inverse(p.common.camera.proj));
    shader->SetMat4("u_invView", glm::inverse(p.common.camera.viewInfo.GetViewMatrix()));
    shader->SetVec3("u_viewPos", p.common.camera.viewInfo.position);
    shader->SetIVec2("u_sourceDim", { sourceDim.width, sourceDim.height });
    shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });

    BindTextureView(0, p.common.gbDepth, p.common.scratchSampler);
    BindTextureView(1, p.common.gbColor, p.common.scratchSampler);
    BindTextureView(2, p.common.gbNormal, p.common.scratchSampler);
    BindTextureView(3, p.common.gbPBR, p.common.scratchSampler);
    BindTextureView(4, p.source, p.common.scratchSampler);
    BindImage(0, p.target, 0);

    const int local_size = 16;
    const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
    const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
}