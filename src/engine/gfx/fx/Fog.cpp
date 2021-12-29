#include "../../PCH.h"
#include "Fog.h"
#include <glad/glad.h>
#include "../ShaderManager.h"

namespace GFX::FX
{
  void CompileFogShader()
  {
    ShaderManager::AddShader("fog",
      {
        { "fog.cs.glsl", ShaderType::COMPUTE }
      });
  }

  void ApplyFog(FogParameters p)
  {
    ASSERT(p.sourceColor.Extent() == p.sourceDepth.Extent() && p.sourceColor.Extent() == p.targetColor.Extent());

    SamplerState samplerState{};
    samplerState.lodBias = 0;
    samplerState.minLod = -1000;
    samplerState.maxLod = 1000;
    p.scratchSampler.SetState(samplerState);
    Extent2D targetDim = p.targetColor.Extent();

    Shader shader = *ShaderManager::GetShader("fog");
    shader.Bind();
    shader.SetFloat("u_a", p.a);
    shader.SetFloat("u_b", p.b);
    shader.SetFloat("u_heightOffset", p.heightOffset);
    shader.SetFloat("u_fog2Density", p.fog2Density);
    shader.SetVec3("u_envColor", p.albedo);
    shader.SetFloat("u_beer", p.beer);
    shader.SetFloat("u_powder", p.powder);
    shader.SetIVec2("u_targetDim", { targetDim.width, targetDim.height });
    shader.SetMat4("u_invViewProj", glm::inverse(p.camera.GetViewProj()));

    BindTextureView(0, p.sourceColor, p.scratchSampler);
    BindTextureView(1, p.sourceDepth, p.scratchSampler);
    BindImage(0, p.targetColor, 0);

    const int local_size = 16;
    const int numGroupsX = (targetDim.width + local_size - 1) / local_size;
    const int numGroupsY = (targetDim.height + local_size - 1) / local_size;
    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
}