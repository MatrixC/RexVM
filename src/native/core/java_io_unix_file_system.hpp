#ifndef NATIVE_CORE_JAVA_IO_UNIX_FILE_SYSTEM_HPP
#define NATIVE_CORE_JAVA_IO_UNIX_FILE_SYSTEM_HPP
#include <unistd.h>
#include <filesystem>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../constant_pool.hpp"

namespace RexVM::Native::Core {

    //native int getBooleanAttributes0(File f)
    void getBooleanAttributes0(Frame &frame) {
        const auto fileOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));

        const auto pathOop = fileOop->getFieldValue("path", "Ljava/lang/String;");
        const auto pathStr = StringPool::getJavaString(CAST_INSTANCE_OOP(pathOop.refVal));
        i4 attribute = 0;
        std::filesystem::path filePath(pathStr);
        if (std::filesystem::exists(filePath)) {
            attribute |= 0x01;

            if (std::filesystem::is_regular_file(filePath)) {
                attribute |= 0x02;
            }

            if (std::filesystem::is_directory(filePath)) {
                attribute |= 0x04;
            }

            if (filePath.filename().string().at(0) == '.') {
                attribute |= 0x08;
            }
        }

        frame.returnI4(attribute);
    }

}

#endif