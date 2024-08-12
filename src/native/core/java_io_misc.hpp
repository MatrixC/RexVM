#ifndef NATIVE_CORE_JAVA_IO_MISC_HPP
#define NATIVE_CORE_JAVA_IO_MISC_HPP
#include <filesystem>
#include <limits.h>
#include <stdlib.h>
#include <chrono>
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

    //native String canonicalize0(String path) throws IOException;
    void canonicalize0(Frame &frame) {
        const auto pathOop = frame.getLocalRef(1);
        if (pathOop == nullptr) {
            throwNullPointException(frame);
            return;
        }

        const auto path = StringPool::getJavaString(CAST_INSTANCE_OOP(pathOop));
        char resolvedPath[PATH_MAX];
        if (realpath(path.c_str(), resolvedPath) == nullptr) {
            throwNullPointException(frame);
        }
        frame.returnRef(frame.mem.getInternString(cstring{resolvedPath}));
    }

    //public native long getLastModifiedTime(File f);
    void getLastModifiedTime(Frame &frame) {
        const auto fileOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        ASSERT_IF_NULL_THROW_NPE(fileOop);

        const auto pathOop = fileOop->getFieldValue("path", "Ljava/lang/String;").refVal;
        ASSERT_IF_NULL_THROW_NPE(pathOop);

        const auto path = StringPool::getJavaString(CAST_INSTANCE_OOP(pathOop));
        std::filesystem::path fsPath(path);
    
        if (std::filesystem::exists(fsPath)) {
            const auto lastWriteTime = std::filesystem::last_write_time(fsPath);
            const auto sctp = 
                std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    lastWriteTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
            const auto timestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(sctp).time_since_epoch().count();
            frame.returnI8(CAST_I8(timestamp));
        } else {
            throwFileNotFoundException(frame, path);
            return;
        }
    }
}

#endif