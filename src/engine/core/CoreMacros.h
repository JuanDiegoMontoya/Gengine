#pragma once

#define NOCOPY_NOMOVE(x) \
  x(const x&) = delete; \
  x(x&&) = delete; \
  x& operator=(const x&) = delete; \
  x& operator=(x&&) = delete;