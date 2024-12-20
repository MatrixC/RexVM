#ifndef BYTE_READER_HPP
#define BYTE_READER_HPP
#include "../basic.hpp"

namespace RexVM {

    struct ByteReader {
        u1 *begin{nullptr};
        u1 *ptr{nullptr};
        size_t length{0};
        i4 cycleOffset{0};
        u1 *codeEnd{nullptr};

        explicit ByteReader();
        
        void init(u1 *in, size_t length_);

        [[nodiscard]] bool eof() const;

        [[nodiscard]] u1 peek() const;
        void skip(u2 n);
        u1 readU1();
        i1 readI1();
        u2 readU2();
        i2 readI2();
        i4 readI4();
        void resetCurrentOffset();
        void relativeOffset(i4 offset);
        void gotoOffset(i4 offset);

    };
}

#endif