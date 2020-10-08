#include <Graphics/GlobalRendererInfo.h>
#include <Graphics/GraphicsIncludes.h>
#include <Systems/Camera.h>
#include <Graphics/UBO.h>

void GlobalRendererInfo::SetCamera(Camera* camera)
{
  UBOData uboData;
  activeCamera = camera;

  if (activeCamera)
  {
    uboData.viewProj = activeCamera->GetProj() * activeCamera->GetView();
    uboData.invProj = glm::inverse(activeCamera->GetProj());
    uboData.invView = glm::inverse(activeCamera->GetView());
    ubo = std::make_unique<UBO>(&uboData, sizeof(UBOData));
  }
}