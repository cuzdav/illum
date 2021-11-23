#pragma once

#if defined(DEBUG)
#include <fmt/core.h>
#define LOG_DEBUG fmt::print
#else
#define LOG_DEBUG(...)
#endif
