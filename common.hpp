#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>

#define __THROW_EXCEPTION_WITH_LOCATION(x)				\
  {									\
    std::stringstream stm;						\
    stm << x << " at " << __FILE__ << ":" << __LINE__ << " (" << __PRETTY_FUNCTION__ << ")"; \
    throw std::runtime_error(stm.str());				\
  }									\

#define BILLION  1000000000L
#define BILLION_F 1000000000.

template <typename handle>
struct close_deleter;

template <>
struct close_deleter<FILE> {
    void operator() (FILE* f) {
	::fclose(f);
    }
};

typedef std::unique_ptr<FILE, close_deleter<FILE>> unique_file;
