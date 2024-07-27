#ifndef REX_FILE_SYSTEM_HPP
#define REX_FILE_SYSTEM_HPP
#include "config.hpp"

#define FILE_SEPARATOR '/'
#define PATH_SEPARATOR ':'
#define LINE_SEPARATOR "\n"

namespace RexVM {

    cstring buildRtPath(const cstring &javaHome);

    cstring buildCharsetsPath(const cstring &javaHome);

    cstring initJavaHome(const char *envJavaHome);

}

#endif
