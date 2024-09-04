#include "file_system.hpp"
#include <filesystem>
#include "config.hpp"

namespace RexVM {

    const char FILE_SEPARATOR = std::filesystem::path::preferred_separator;

    cstring buildRtPath(cview javaHome) {
        return cformat("{}{}lib{}rt.jar", javaHome, FILE_SEPARATOR, FILE_SEPARATOR);
        //return javaHome + FILE_SEPARATOR + "lib" + FILE_SEPARATOR + "rt.jar";
    }

    cstring buildCharsetsPath(cview javaHome) {
        return cformat("{}{}lib{}charsets.jar", javaHome, FILE_SEPARATOR, FILE_SEPARATOR);
        //return javaHome + FILE_SEPARATOR + "lib" + FILE_SEPARATOR + "charsets.jar";
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

    cstring getJavaHome() {
        cstring javaHome{};
        javaHome = initJavaHome(std::getenv("JAVA_HOME"));
        if (javaHome.empty()) {
            javaHome = initJavaHome(std::getenv("JAVA8_HOME"));
        }
        return javaHome;
    }

}