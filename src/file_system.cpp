#include "file_system.hpp"
#include <filesystem>
#include "config.hpp"

namespace RexVM {

    const char FILE_SEPARATOR = std::filesystem::path::preferred_separator;

    cstring buildRtPath(const cstring &javaHome) {
        return javaHome + FILE_SEPARATOR + "lib" + FILE_SEPARATOR + "rt.jar";
    }

    cstring buildCharsetsPath(const cstring &javaHome) {
        return javaHome + FILE_SEPARATOR + "lib" + FILE_SEPARATOR + "charsets.jar";
    }

    cstring initJavaHome(const char *envJavaHome) {
        if (envJavaHome == nullptr) {
            return {};
        }

        std::filesystem::path javaHomePath(envJavaHome);
        if (!std::filesystem::is_directory(javaHomePath)) {
            return {};
        }
        auto javaHome = cstring(envJavaHome);
        if (javaHome.back() == FILE_SEPARATOR || javaHome.back() == '\\') {
            javaHome.pop_back();
        }

        auto rtPath = javaHome + FILE_SEPARATOR + "lib" + FILE_SEPARATOR + "rt.jar";
        if (std::filesystem::exists(rtPath)) {
            return javaHome;
        }

        auto jreHome = javaHome + FILE_SEPARATOR + "jre";
        rtPath = jreHome + FILE_SEPARATOR + "lib" + FILE_SEPARATOR + "rt.jar";
        if (std::filesystem::exists(rtPath)) {
            return jreHome;
        }

        return {};
    }

}