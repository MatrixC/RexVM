#ifndef NATIVE_CORE_JAVA_IO_FILE_OUTPUT_STREAM_HPP
#define NATIVE_CORE_JAVA_IO_FILE_OUTPUT_STREAM_HPP
#include <unistd.h>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"

namespace RexVM::Native::Core {

    //void writeBytes(byte b[], int off, int len, boolean append)
    void writeBytes(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(1));
        const auto off = frame.getLocalI4(2);
        const auto len = frame.getLocalI4(3);
        //const auto append = frame.getLocalBoolean(4);

        const auto fd = CAST_INSTANCE_OOP(self->getFieldValue("fd", "Ljava/io/FileDescriptor;").refVal);
        const auto fdId = fd->getFieldValue("fd", "I").i4Val;

        const auto strPtr = reinterpret_cast<const char*>(b->data.get());
        write(fdId, strPtr + off, len);
    }

}

#endif