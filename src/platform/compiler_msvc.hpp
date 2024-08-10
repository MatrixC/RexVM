#ifndef COMPILE_MSVC_HPP
#define COMPILE_MSVC_HPP
#pragma warning(disable: 4996)

#define ATTR_UNUSED
#define NOMINMAX

#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

#endif