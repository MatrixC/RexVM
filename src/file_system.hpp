#ifndef REX_FILE_SYSTEM_HPP
#define REX_FILE_SYSTEM_HPP
#include "config.hpp"


namespace RexVM {

    extern const char FILE_SEPARATOR;

    cstring buildRtPath(const cstring &javaHome);

    cstring buildCharsetsPath(const cstring &javaHome);

    cstring initJavaHome(const char *envJavaHome);

    cstring getJavaHome();

}

#endif
