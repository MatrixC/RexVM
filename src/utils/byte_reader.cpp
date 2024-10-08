#include "byte_reader.hpp"

namespace RexVM {

    ByteReader::ByteReader() : ptr(nullptr) {
    }


    void ByteReader::init(u1 *in, size_t length_) {
        begin = in;
        ptr = in;
        length = length_;
    }

    bool ByteReader::eof() const {
        const auto codeEnd = begin + length;
        return ptr >= codeEnd;
    }

    u1 ByteReader::peek() const {
        return *ptr;
    }

    u1 ByteReader::readU1() {
        const auto val = *ptr;
        ptr += 1;
        cycleOffset += 1;
        return val;
    }

    i1 ByteReader::readI1() {
        const auto val = CAST_I1(*ptr);
        ptr += 1;
        cycleOffset += 1;
        return val;
    }

    u2 ByteReader::readU2() {
        const StreamByteType auto val = read<u2>(ptr);
        ptr += 2;
        cycleOffset += 2;
        return val;
    }

    i2 ByteReader::readI2() {
        const StreamByteType auto val = read<i2>(ptr);
        ptr += 2;
        cycleOffset += 2;
        return val;
    }

    i4 ByteReader::readI4() {
        const StreamByteType auto val = read<i4>(ptr);
        ptr += 4;
        cycleOffset += 4;
        return val;
    }

    void ByteReader::resetCurrentOffset() {
        cycleOffset = 0;
    }

    void ByteReader::relativeOffset(i4 offset) {
        ptr += (offset - cycleOffset);
    }

    void ByteReader::gotoOffset(i4 offset) {
        ptr = begin + offset;
    }

    u4 ByteReader::pc() const {
        return CAST_U4(ptr - begin);
    }

}