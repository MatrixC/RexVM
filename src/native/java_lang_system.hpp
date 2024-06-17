#ifndef NATIVE_JAVA_LANG_SYSTEM_HPP
#define NATIVE_JAVA_LANG_SYSTEM_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void setIn0(Frame &frame);

    void setOut0(Frame &frame);

    void setErr0(Frame &frame);

    void currentTimeMillis(Frame &frame);

    void initProperties(Frame &frame);

    void doPrivileged(Frame &frame);

    void arraycopy(Frame &frame);

    void mapLibraryName(Frame &frame);
}

#endif