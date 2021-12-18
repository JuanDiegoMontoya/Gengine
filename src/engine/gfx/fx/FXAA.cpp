#include "FXAA.h"
#include <glad/glad.h>
#include "../ShaderManager.h"

void GFX::FX::ApplyFXAA(const TextureView& source, const TextureView& target,
  float contrastThreshold, float relativeThreshold,
  float pixelBlendStrength, float edgeBlendStrength,
  TextureSampler& scratchSampler)
{
  SamplerState samplerState{};
  samplerState.asBitField.minFilter = Filter::LINEAR;
  samplerState.asBitField.magFilter = Filter::LINEAR;
  samplerState.asBitField.mipmapFilter = Filter::LINEAR;
  samplerState.asBitField.addressModeU = AddressMode::MIRRORED_REPEAT;
  samplerState.asBitField.addressModeV = AddressMode::MIRRORED_REPEAT;
  samplerState.lodBias = 0;
  samplerState.minLod = -1000;
  samplerState.maxLod = 1000;
  scratchSampler.SetState(samplerState);
  Extent2D targetDim = target.Extent();

  auto shader = GFX::ShaderManager::Get()->GetShader("fxaa");
  shader->Bind();
  shader->SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
  shader->SetFloat("u_contrastThreshold", contrastThreshold);
  shader->SetFloat("u_relativeThreshold", relativeThreshold);
  shader->SetFloat("u_pixelBlendStrength", pixelBlendStrength);
  shader->SetFloat("u_edgeBlendStrength", edgeBlendStrength);

  BindTextureView(0, source, scratchSampler);
  BindImage(0, target, 0);

  const int local_size = 16;
  const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
  const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
  glDispatchCompute(numGroupsX, numGroupsY, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}