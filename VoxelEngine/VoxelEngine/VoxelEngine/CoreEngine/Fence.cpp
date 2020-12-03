#include "Fence.h"
#include "GraphicsIncludes.h"
#include <Utilities/Timer.h>

Fence::Fence()
{
  sync_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

Fence::~Fence()
{
  glDeleteSync(sync_);
}

uint64_t Fence::Sync()
{
  GLuint id;
  glGenQueries(1, &id);
  glBeginQuery(GL_TIME_ELAPSED, id);
  GLenum result = glClientWaitSync(sync_, GL_SYNC_FLUSH_COMMANDS_BIT, -1);
  glEndQuery(GL_TIME_ELAPSED);
  uint64_t elapsed;
  glGetQueryObjectui64v(id, GL_TIME_ELAPSED, &elapsed);
  glDeleteQueries(1, &id);
  return elapsed;
}
