#ifndef NATIVE_CORE_DEFINE_CLASS_HPP
#define NATIVE_CORE_DEFINE_CLASS_HPP
#include <algorithm>
#include <miniz.h>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../oop.hpp"
#include "../../string_pool.hpp"
#include "../../exception_helper.hpp"
#include "../../memory.hpp"

namespace RexVM::Native::Misc {

    mz_zip_archive *zipGetArchive(const i8 jzFile) {
        return std::bit_cast<mz_zip_archive *>(jzFile);
    }

    mz_zip_archive_file_stat *zipGetFieStat(const i8 jzentry) {
        return std::bit_cast<mz_zip_archive_file_stat *>(jzentry);
    }

    i8 zipToArchive(mz_zip_archive *archive) {
        return std::bit_cast<i8>(archive);
    }

    i8 zipToFileStat(mz_zip_archive_file_stat *fileStat) {
        return std::bit_cast<i8>(fileStat);
    }

    void zipInitIDs(Frame &frame) {
    }

    //private static native long getEntry(long jzfile, byte[] name, boolean addSlash);
    void zipGetEntry(Frame &frame) {
        const auto jzFile = frame.getLocalI8(0);
        const auto archive = zipGetArchive(jzFile);

        const auto nameBytes = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(2));
        ASSERT_IF_NULL_THROW_NPE(nameBytes);
        //传进来的参数没有以\0结尾 miniz中用了strlen 会有问题 自己处理下
        std::vector<char> nameBytesVec(nameBytes->getDataLength() + 1, '\0');
        const auto nameBytesPtr = nameBytes->data.get();
        std::copy(nameBytesPtr, nameBytesPtr + nameBytes->getDataLength(), nameBytesVec.data());
        const auto nameBytesFixPtr = nameBytesVec.data();

        const auto fileIndex = mz_zip_reader_locate_file(archive, nameBytesFixPtr, nullptr, 0);
        if (fileIndex == -1) {
            //throwIOException(frame, "locate zip file failed");
            frame.returnI8(CAST_I8(0));
            return;
        }

        auto fileStat = new mz_zip_archive_file_stat;
        std::memset(fileStat, 0, sizeof(mz_zip_archive_file_stat));
        if (!mz_zip_reader_file_stat(archive, fileIndex, fileStat)) {
            //throwIOException(frame, "get zip file stat failed");
            frame.returnI8(CAST_I8(0));
            return;
        }

        frame.returnI8(zipToFileStat(fileStat));
    }

    //private static native void freeEntry(long jzfile, long jzentry);
    void zipFreeEntry(Frame &frame) {
        const auto jzentry = frame.getLocalI8(2);
        const auto fileStat = zipGetFieStat(jzentry);
        delete fileStat;
    }

    //private static native long getNextEntry(long jzfile, int i);
    void zipGetNextEntry(Frame &frame) {
        const auto jzFile = frame.getLocalI8(0);
        const auto archive = zipGetArchive(jzFile);
        const auto fileIndex = frame.getLocalI4(2);

        auto fileStat = new mz_zip_archive_file_stat;
        if (!mz_zip_reader_file_stat(archive, fileIndex, fileStat)) {
            //throwIOException(frame, "get zip file stat failed");
            frame.returnI8(CAST_I8(0));
            return;
        }

        frame.returnI8(zipToFileStat(fileStat));
    }

    //private static native int getTotal(long jzfile);
    void zipGetTotal(Frame &frame) {
        const auto jzFile = frame.getLocalI8(0);
        const auto archive = zipGetArchive(jzFile); 
        frame.returnI4(CAST_I4(archive->m_total_files));
    }

    //private static native boolean startsWithLOC(long jzfile);
    void zipStartsWithLOC(Frame &frame) {
        frame.returnBoolean(true);
    }

    //private static native int getManifestNum(long jzfile);
    void zipGetManifestNum(Frame &frame) {
        frame.returnI4(1);
    }

    //private static native long getEntryCrc(long jzentry);
    void zipGetEntryCrc(Frame &frame) {
        const auto jzentry = frame.getLocalI8(0);
        const auto fileStat = zipGetFieStat(jzentry);
        frame.returnI8(CAST_I8(fileStat->m_crc32));
    }

    //private static native long getEntryTime(long jzentry);
    void zipGetEntryTime(Frame &frame) {
        const auto jzentry = frame.getLocalI8(0);
        const auto fileStat = zipGetFieStat(jzentry);
        frame.returnI8(CAST_I8(fileStat->m_time));
    }

    //private static native long getEntryCSize(long jzentry);
    void zipGetEntryCSize(Frame &frame) {
        const auto jzentry = frame.getLocalI8(0);
        const auto fileStat = zipGetFieStat(jzentry);
        //frame.returnI8(CAST_I8(fileStat->m_comp_size));
        //因为使用了miniz的解压 所以返回到jvm不需要再进行解压 当做STORE格式
        frame.returnI8(CAST_I8(fileStat->m_uncomp_size));
    }

    //private static native long getEntrySize(long jzentry);
    void zipGetEntrySize(Frame &frame) {
        const auto jzentry = frame.getLocalI8(0);
        const auto fileStat = zipGetFieStat(jzentry);
        frame.returnI8(CAST_I8(fileStat->m_uncomp_size));
    }

    //private static native int getEntryMethod(long jzentry);
    void zipGetEntryMethod(Frame &frame) {
        //const auto jzentry = frame.getLocalI8(0);
        //const auto fileStat = zipGetFieStat(jzentry);
        //frame.returnI4(CAST_I4(fileStat->m_method));
        //openjdk里的解包和解压是做了区分处理 miniz已经解压 所以直接返回STORE类型即可
        frame.returnI4(CAST_I4(0));
    }

    //private static native int getEntryFlag(long jzentry);
    void zipGetEntryFlag(Frame &frame) {
        const auto jzentry = frame.getLocalI8(0);
        const auto fileStat = zipGetFieStat(jzentry);
        frame.returnI4(CAST_I4(fileStat->m_bit_flag));
    }

    //private static native byte[] getCommentBytes(long jzfile);
    void zipGetCommentBytes(Frame &frame) {
        //const auto jzentry = frame.getLocalI8(0);
        //const auto fileStat = zipGetFieStat(jzentry);
        frame.returnRef(nullptr);
    }

    //private static native byte[] getEntryBytes(long jzentry, int type);
    void zipGetEntryBytes(Frame &frame) {
        const auto jzentry = frame.getLocalI8(0);
        const auto fileStat = zipGetFieStat(jzentry);
        const auto type = frame.getLocalI4(2);

        switch (type) {
            case 0: {//JZENTRY_NAME
                const auto fileNameOop = frame.mem.newByteArrayOop(MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE);
                std::copy(fileStat->m_filename, fileStat->m_filename + MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE, fileNameOop->data.get());
                frame.returnRef(fileNameOop);
                return;
            }
            case 1: //JZENTRY_EXT
                frame.returnRef(nullptr);
                return;
            case 2: {//JZENTRY_COMMENT
                frame.returnRef(nullptr);
                return;
            }
        }

        panic("error type");
    }

    //private static native String getZipMessage(long jzfile);
    void zipGetZipMessage(Frame &frame) {
        frame.returnRef(nullptr);
    }

    //private static native byte[] getCentralDirectoryBytes(long jzfile) throws ZipException;
    void zipGetCentralDirectoryBytes(Frame &frame) {
        frame.returnRef(nullptr);
    }

    void jarGetMetaInfEntryNames(Frame &frame) {
        const auto self = frame.getThisInstance();
        const auto jzfile = self->getFieldValue("jzfile" "J").i8Val;
        const auto archive = zipGetArchive(jzfile);
        std::vector<cview> fileNames;

        for (decltype(archive->m_total_files) i = 0; i < archive->m_total_files; ++i) {
            std::array<char, 512> nameBuffer{};
            if (!mz_zip_reader_get_filename(archive, i, nameBuffer.data(), sizeof(nameBuffer))) {
                frame.returnRef(nullptr);
                return;
            }

            if (strncmp(nameBuffer.data(), "META-INF/", 9) == 0) {
                fileNames.emplace_back(nameBuffer.data());
            }
        }

        const auto result = frame.mem.newStringObjArrayOop(fileNames.size());
        for (size_t i = 0; i < fileNames.size(); ++i) {
            result->data[i] = frame.mem.getInternString(fileNames[i]);
        }

        frame.returnRef(result);
    }

    //private static native int read(long jzfile, long jzentry, long pos, byte[] b, int off, int len);
    void zipRead(Frame &frame) {
        const auto jzFile = frame.getLocalI8(0);
        const auto archive = zipGetArchive(jzFile); 
        const auto jzentry = frame.getLocalI8(2);
        const auto pos = CAST_SIZE_T(frame.getLocalI8(4));
        const auto b = CAST_BYTE_TYPE_ARRAY_OOP(frame.getLocalRef(6));
        const auto off = CAST_SIZE_T(frame.getLocalI4(7));
        const auto len = CAST_SIZE_T(frame.getLocalI4(8));
        const auto fileStat = zipGetFieStat(jzentry);

        if (len > b->getDataLength() - off) {
            throwIOException(frame, "len > byte[] size - off");
            return;  
        }

        const auto uncompressedSize = CAST_SIZE_T(fileStat->m_uncomp_size);
        if (pos == uncompressedSize) {
            frame.returnI4(CAST_I4(-1));
            return; 
        }

        if (pos > uncompressedSize) {
            throwIOException(frame, "pos > file size");
            return; 
        }

        std::vector<u1> buffer(uncompressedSize);
        if (!mz_zip_reader_extract_to_mem(archive, fileStat->m_file_index, buffer.data(), uncompressedSize, 0)) {
            throwIOException(frame, "read zip file failed");
            return;
        }

        const auto readLen = std::min(len, uncompressedSize - pos);

        const u1 *srcStartPtr = buffer.data() + pos;
        const u1 *srcEndPtr = srcStartPtr + readLen;
        u1 *destStartPtr = b->data.get() + off;
        std::copy(srcStartPtr, srcEndPtr, destStartPtr);
        
        frame.returnI4(CAST_I4(readLen));
    }

    //private static native long open(String name, int mode, long lastModified, boolean usemmap) throws IOException;
    void zipOpen(Frame &frame) {
        const auto nameOop = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        ASSERT_IF_NULL_THROW_NPE(nameOop);
        const auto pathStr = VMStringHelper::getJavaString(nameOop);

        const auto archive = new mz_zip_archive();
        //cprintln("open {:p}\t{}", (void*)archive, pathStr);
        std::memset(archive, 0, sizeof(mz_zip_archive));
        if (!mz_zip_reader_init_file(archive, pathStr.c_str(), 0)) {
            //throwIOException(frame, "open zip file failed"); 
            frame.returnI8(CAST_I8(0));
            return;
        }
        frame.returnI8(zipToArchive(archive));
    }

    //private static native void close(long jzfile);
    void zipClose(Frame &frame) {
        const auto jzFile = frame.getLocalI8(0);
        const auto archive = zipGetArchive(jzFile);
        //cprintln("close {}", (void*)archive);
        mz_zip_reader_end(archive);
        delete archive;
    }

}


#endif