#include "GraphicsSystem.h"
#include "Scene.h"
#include <Graphics/GraphicsIncludes.h>

void GraphicsSystem::Update(Scene& scene, float dt)
{
  printf("Updated graphics system\n");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
