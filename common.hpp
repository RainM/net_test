#pragma once

#define BILLION  1000000000L
#define BILLION_F 1000000000.

#include <sstream>
#include <stdexcept>

#define __THROW_EXCEPTION_WITH_LOCATION(x)				\
  {									\
    std::stringstream stm;						\
    stm << x << " at " << __FILE__ << ":" << __LINE__ << " (" << __PRETTY_FUNCTION__ << ")"; \
    throw std::runtime_error(stm.str());				\
  }									\

