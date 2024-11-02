#ifndef REX_FILE_SYSTEM_HPP
#define REX_FILE_SYSTEM_HPP
#include "basic.hpp"


namespace RexVM {

    extern const char FILE_SEPARATOR;

    cstring buildRtPath(cview javaHome);

    cstring buildCharsetsPath(cview javaHome);

    cstring initJavaHome(const char *envJavaHome);

    cstring getJavaHome();

    cstring getTmpDir();
}

#endif
