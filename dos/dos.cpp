#include "dos/dos.h"

#include "core/file.h"
#include "core/log.h"
#include "core/scope_exit.h"
#include "fmt/format.h"
#include <cstdio>
#include <optional>
#include <string>

// MSVC only has __PRETTY_FUNCTION__ in intellisense,
// TODO(rushfan): Find a better home for this macro.
#if !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

namespace door86::dos {

using namespace wwiv::core;

}

