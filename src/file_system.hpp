#ifndef REX_FILE_SYSTEM_HPP
#define REX_FILE_SYSTEM_HPP
#include "config.hpp"

#define PATH_SEPARATOR ':'
#define LINE_SEPARATOR "\n"

namespace RexVM {

    extern const char FILE_SEPARATOR;

    cstring buildRtPath(const cstring &javaHome);

    cstring buildCharsetsPath(const cstring &javaHome);

    cstring initJavaHome(const char *envJavaHome);

}

#endif
