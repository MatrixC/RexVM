#ifndef NATIVE_CORE_JAVA_IO_FILE_INPUT_STREAM_HPP
#define NATIVE_CORE_JAVA_IO_FILE_INPUT_STREAM_HPP
#include <fcntl.h>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../exception_helper.hpp"

namespace RexVM::Native::Core {

    bool inputCheckClosed(Frame &frame, InstanceOop *self) {
        const auto closed = self->getFieldValue("closed" "Z").i4Val;
        if (closed != 0) {
            throwIOException(frame, "Stream Closed");
            return false;
        }
        return true;
    }

    i4 getFd(InstanceOop *self) {
        const auto fdOop = CAST_INSTANCE_OOP(self->getFieldValue("fd" "Ljava/io/FileDescriptor;").refVal);
        return fdOop->getFieldValue("fd" "I").i4Val;
    }

    enum class FileOpenTypeEnum {
        READ,
        WRITE,
    };

    //native void open0(String name) throws FileNotFoundException;
    void openCommon(Frame &frame, FileOpenTypeEnum type, bool append) {
        const auto self = CAST_INSTANCE_OOP(frame.getThisInstance());
        const auto pathStr = VMStringHelper::getJavaString(CAST_INSTANCE_OOP(frame.getLocalRef(1)));
        std::filesystem::path filePath(pathStr);
        if (!std::filesystem::exists(filePath)) {
            throwFileNotFoundException(frame, pathStr);
            return;
        }
        if (std::filesystem::is_directory(filePath)) {
            throwFileNotFoundException(frame, pathStr + " : is directory");
            return;
        }

        const auto fd = 
            type == FileOpenTypeEnum::READ ? 
                open(pathStr.c_str(), O_RDONLY) :
                open(pathStr.c_str(), append ? (O_WRONLY | O_CREAT | O_APPEND) : (O_WRONLY | O_CREAT | O_TRUNC), 0644);
        if (fd == -1) {
            throwFileNotFoundException(frame, pathStr + " open failed");
            return;
        }

        const auto fdOop = CAST_INSTANCE_OOP(self->getFieldValue("fd" "Ljava/io/FileDescriptor;").refVal);
        fdOop->setFieldValue("fd" "I", Slot(CAST_I4(fd)));
    }

    void readOpen(Frame &frame) {
        openCommon(frame, FileOpenTypeEnum::READ, false);
    }

    //native void open0(String name, boolean append)
    void writeOpen(Frame &frame) {
        const auto append = frame.getLocalI4(2) != 0;
        openCommon(frame, FileOpenTypeEnum::WRITE, append);
    }

    //native int readBytes(byte b[], int off, int len) throws IOException;
    void readBytes(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(1));
        const auto off = frame.getLocalI4(2);
        const auto len = frame.getLocalI4(3);
        //const auto append = frame.getLocalBoolean(4);

        if (!inputCheckClosed(frame, self)) {
            return;
        }

        const auto fd = getFd(self);

        auto bytePtr = reinterpret_cast<char*>(b->data.get());
        bytePtr += off;

        const auto ret = ::read(fd, bytePtr, len);
        if (ret == -1) {
            throwIOException(frame, "read error");
            return;
        } else if (ret == 0) {
            frame.returnI4(-1);
            return;
        } else {
            frame.returnI4(CAST_I4(ret));
            return;
        }
    }

    //native int read0() throws IOException;
    void read0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        if (!inputCheckClosed(frame, self)) {
            return;
        }
        const auto fd = getFd(self);

        unsigned char buff;
        const auto ret = ::read(fd, &buff, 1);
        if (ret == -1) {
            throwIOException(frame, "read error");
            return;
        } else if (ret == 0) {
            frame.returnI4(-1);
            return;
        } else {
            frame.returnI4(CAST_I4(buff));
            return;
        }
    }

    //native long skip0(long n) throws IOException;
    void skip0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto n = frame.getLocalI8(1);
        if (!inputCheckClosed(frame, self)) {
            return;
        }

        const auto fd = getFd(self);
        const auto currentPosition = lseek(fd, 0, SEEK_CUR);
        if (currentPosition == -1) {
            throwIOException(frame, "Error getting current position");
            return;
        }
    
        auto newPosition = lseek(fd, static_cast<off_t>(n), SEEK_CUR);
        if (newPosition == -1) {
            throwIOException(frame, "Error skipping bytes");
            return;
        }

        frame.returnI8(newPosition - currentPosition);
    }

    //native int available0() throws IOException;
    void available0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        if (!inputCheckClosed(frame, self)) {
            return;
        }

        const auto fd = getFd(self);
        const auto currentPosition = lseek(fd, 0, SEEK_CUR);
        if (currentPosition == -1) {
            throwIOException(frame, "Error getting current position");
            return;
        }
    
        const auto endPosition = lseek(fd, 0, SEEK_END);
        if (endPosition == -1) {
            throwIOException(frame, "Error getting file end position");
            return;
        }
    
        if (lseek(fd, currentPosition, SEEK_SET) == -1) {
            throwIOException(frame, "Error restoring file position");
            return;
        }

        frame.returnI4(CAST_I4(endPosition - currentPosition));
    }

    //native void close0() throws IOException;
    void close0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto fdId = getFd(self);
        close(fdId);
    }

    //void writeBytes(byte b[], int off, int len, boolean append)
    void writeBytes(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(1));
        const auto off = frame.getLocalI4(2);
        const auto len = frame.getLocalI4(3);
        //const auto append = frame.getLocalBoolean(4);

        const auto fdId = getFd(self);
        const auto bytePtr = reinterpret_cast<const char*>(b->data.get());
        if (write(fdId, bytePtr + off, len) == -1) {
            throwRuntimeException(frame, "write error");
            return;
        }
    }

    //native void write(int b, boolean append) throws IOException;
    void write0(Frame &frame) {
        const auto self = CAST_INSTANCE_OOP(frame.getThis());
        const auto b = frame.getLocalI4(1);
        //const auto append = frame.getLocalBoolean(2);

        const auto fd = getFd(self);
        if (write(fd, &b, sizeof(b)) == -1) {
            throwRuntimeException(frame, "write error");
            return;
        }
    }



}

#endif
