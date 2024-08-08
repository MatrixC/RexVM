#ifndef OS_UNIX_LIKE_HPP
#define OS_UNIX_LIKE_HPP

#ifdef __APPLE__
#define OS_NAME "OS X"
#else
#define OS_NAME "Linux"
#endif

#define PATH_SEPARATOR ':'
#define LINE_SEPARATOR "\n"

#endif