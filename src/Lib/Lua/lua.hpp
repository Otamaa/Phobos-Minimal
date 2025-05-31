// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#pragma warning(push)
#pragma warning(disable : 4702)
#pragma warning(disable : 4701)
#pragma warning(disable : 4244)
#pragma warning(disable : 4310)
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#pragma warning(pop)
