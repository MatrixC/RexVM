#ifndef NATIVE_CORE_JAVA_IO_MISC_HPP
#define NATIVE_CORE_JAVA_IO_MISC_HPP
#include <unistd.h>
#include <filesystem>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../string_pool.hpp"

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

    void initNative(Frame &frame) {
    }

    //native static boolean hasStaticInitializer(Class<?> cl);
    void hasStaticInitializer(Frame &frame) {
        const auto cl = frame.getLocalRef(0);
        if (cl == nullptr) {
            throwNullPointException(frame);
            return;
        }
        const auto mirrorClass = GET_MIRROR_CLASS(cl);
        if (mirrorClass->type != ClassTypeEnum::INSTANCE_CLASS) {
            frame.returnBoolean(false);
            return;
        }
        const auto instanceMirrorClass = CAST_INSTANCE_CLASS(mirrorClass);
        frame.returnBoolean(instanceMirrorClass->getMethodSelf("<clinit>", "()V", true) != nullptr);
    }



}

#endif