#ifndef NATIVE_CORE_JAVA_IO_FILE_INPUT_STREAM_HPP
#define NATIVE_CORE_JAVA_IO_FILE_INPUT_STREAM_HPP
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../exception_helper.hpp"

namespace RexVM::Native::Core {

    //native void open0(String name) throws FileNotFoundException;
    void open0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThisInstance());
        const auto pathStr = StringPool::getJavaString(CAST_INSTANCE_OOP(frame.getLocalRef(1)));

        const auto fd = open(pathStr.c_str(), O_RDONLY);
        if (fd == -1) {
            throwFileNotFoundException(frame, pathStr);
            return;
        }

        struct stat statbuf;
        if (fstat(fd, &statbuf) == -1) {
            close(fd);
            throwFileNotFoundException(frame, pathStr + " : stat error");
            return;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            close(fd);
            throwFileNotFoundException(frame, pathStr + " : is dir");
            return;
        } 

        const auto fdOop = CAST_INSTANCE_OOP(self->getFieldValue("fd", "Ljava/io/FileDescriptor;").refVal);
        fdOop->setFieldValue("fd", "I", Slot(CAST_I4(fd)));
    }

    //native int readBytes(byte b[], int off, int len) throws IOException;
    void readBytes(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(1));
        const auto off = frame.getLocalI4(2);
        const auto len = frame.getLocalI4(3);
        //const auto append = frame.getLocalBoolean(4);

        const auto fdOop = CAST_INSTANCE_OOP(self->getFieldValue("fd", "Ljava/io/FileDescriptor;").refVal);
        const auto fd = fdOop->getFieldValue("fd", "I").i4Val;

        auto bytePtr = reinterpret_cast<char*>(b->data.get());
        bytePtr += off;

        const auto ret = ::read(fd, bytePtr, len);
        frame.returnI4(ret);
    }

    //native void close0() throws IOException;
    void close0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto fdOop = CAST_INSTANCE_OOP(self->getFieldValue("fd", "Ljava/io/FileDescriptor;").refVal);
        const auto fd = fdOop->getFieldValue("fd", "I").i4Val;
        ::close(fd);
    }



}

#endif