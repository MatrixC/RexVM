#include "java_io_file_output_stream.hpp"
#include "../oop.hpp"
#include <unistd.h>

namespace RexVM::Native {

    //void writeBytes(byte b[], int off, int len, boolean append)
    void writeBytes(Frame &frame) {
        const auto self = dynamic_cast<InstanceOop *>(frame.getThis());
        const auto b = static_cast<ByteTypeArrayOop *>(frame.getLocalRef(1));
        const auto off = frame.getLocalI4(2);
        const auto len = frame.getLocalI4(3);
        //const auto append = frame.getLocalBoolean(4);

        const auto fd = static_cast<InstanceOop *>(self->getFieldValue("fd", "Ljava/io/FileDescriptor;").refVal);
        const auto fdId = fd->getFieldValue("fd", "I").i4Val;

        const auto strPtr = reinterpret_cast<const char*>(b->data.get());
        write(fdId, strPtr + off, len);
    }
}