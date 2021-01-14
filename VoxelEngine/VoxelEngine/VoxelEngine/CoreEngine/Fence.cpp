#include "EnginePCH.h"
#include "Fence.h"
#include "GraphicsIncludes.h"
#include <Utilities/Timer.h>

namespace GFX
{
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
    glGetQueryObjectui64v(id, GL_QUERY_RESULT, &elapsed);
    glDeleteQueries(1, &id);
    return elapsed;
  }


  TimerQuery::TimerQuery()
  {
    glGenQueries(2, queries);
    glQueryCounter(queries[0], GL_TIMESTAMP);
  }

  TimerQuery::~TimerQuery()
  {
    glDeleteQueries(2, queries);
  }

  uint64_t TimerQuery::Elapsed()
  {
    int complete = 0;
    glQueryCounter(queries[1], GL_TIMESTAMP);
    while (!complete) glGetQueryObjectiv(queries[1], GL_QUERY_RESULT_AVAILABLE, &complete);
    uint64_t startTime, endTime;
    glGetQueryObjectui64v(queries[0], GL_QUERY_RESULT, &startTime);
    glGetQueryObjectui64v(queries[1], GL_QUERY_RESULT, &endTime);
    std::swap(queries[0], queries[1]);
    return endTime - startTime;
  }
}