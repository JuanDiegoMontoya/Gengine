#include "../../PCH.h"
#include "Bloom.h"
#include <glad/glad.h>
#include <engine/gfx/resource/ShaderManager.h>
#include "../../GAssert.h"

namespace GFX::FX
{
  void CompileBloomShaders()
  {
    ShaderManager::AddShader("bloom/downsampleLowPass",
      { { "bloom/downsampleLowPass.cs.glsl", GFX::ShaderType::COMPUTE } });

    ShaderManager::AddShader("bloom/downsample",
      { { "bloom/downsample.cs.glsl", GFX::ShaderType::COMPUTE } });

    ShaderManager::AddShader("bloom/upsample",
      { { "bloom/upsample.cs.glsl", GFX::ShaderType::COMPUTE } });
  }

  void ApplyBloom(const TextureView& target, uint32_t passes, float strength, float width,
    const TextureView& scratchTexture, TextureSampler& scratchSampler)
  {
    ASSERT(target.Extent().width >> passes > 0 && target.Extent().height >> passes > 0);

    SamplerState samplerState{};
    samplerState.asBitField.minFilter = Filter::LINEAR;
    samplerState.asBitField.magFilter = Filter::LINEAR;
    samplerState.asBitField.mipmapFilter = Filter::NEAREST;
    samplerState.asBitField.addressModeU = AddressMode::MIRRORED_REPEAT;
    samplerState.asBitField.addressModeV = AddressMode::MIRRORED_REPEAT;
    samplerState.lodBias = 0;
    samplerState.minLod = -1000;
    samplerState.maxLod = 1000;
    scratchSampler.SetState(samplerState);

    auto downsampleLowPass = ShaderManager::GetShader("bloom/downsampleLowPass");
    auto downsample = ShaderManager::GetShader("bloom/downsample");

    const int local_size = 16;
    for (uint32_t i = 0; i < passes; i++)
    {
      Extent2D sourceDim{};
      Extent2D targetDim = target.Extent() >> i + 1;
      float sourceLod{};

      Shader* shader{ nullptr };
      const TextureView* sourceView{ nullptr };

      // first pass, use downsampling with low-pass filter
      if (i == 0)
      {
        shader = &downsampleLowPass.value();
        shader->Bind();

        sourceLod = 0;
        sourceView = &target;
        sourceDim = { target.Extent().width, target.Extent().height };
      }
      else
      {
        shader = &downsample.value();
        shader->Bind();

        sourceLod = i - 1;
        sourceView = &scratchTexture;
        sourceDim = target.Extent() >> i;
      }

      BindTextureView(0, *sourceView, scratchSampler);
      BindImage(0, scratchTexture, i);

      shader->SetIVec2("u_sourceDim", { sourceDim.width, sourceDim.height });
      shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
      shader->SetFloat("u_sourceLod", sourceLod);

      const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
      const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
      glDispatchCompute(numGroupsX, numGroupsY, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    auto us = ShaderManager::GetShader("bloom/upsample");
    us->Bind();
    for (int32_t i = passes - 1; i >= 0; i--)
    {
      Extent2D sourceDim = target.Extent() >> i + 1;
      Extent2D targetDim{};
      const TextureView* targetView{ nullptr };
      float realStrength = 1.0f;
      float targetMip{};

      // final pass
      if (i == 0)
      {
        realStrength = strength;
        targetMip = 0;
        targetView = &target;
        targetDim = target.Extent();
      }
      else
      {
        targetMip = i - 1;
        targetView = &scratchTexture;
        targetDim = target.Extent() >> i;
      }

      BindTextureView(0, scratchTexture, scratchSampler);
      BindTextureView(1, *targetView, scratchSampler);
      BindImage(0, *targetView, targetMip);

      us->SetIVec2("u_sourceDim", { sourceDim.width, sourceDim.height });
      us->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
      us->SetFloat("u_width", width);
      us->SetFloat("u_strength", realStrength);
      us->SetFloat("u_sourceLod", i);
      us->SetFloat("u_targetLod", targetMip);

      const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
      const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
      glDispatchCompute(numGroupsX, numGroupsY, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
  }
}