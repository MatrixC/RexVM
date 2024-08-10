#ifndef NATIVE_CORE_MISC_HPP
#define NATIVE_CORE_MISC_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    static u4 impUpdateBytes(u4 crc, const unsigned char* buf, i4 offset, i4 length) {
        static u4 table[256];
        static bool tableComputed = false;

        if (!tableComputed) {
            for (u4 i = 0; i < 256; i++) {
                u4 c = i;
                for (int j = 0; j < 8; j++) {
                    if (c & 1)
                        c = 0xedb88320L ^ (c >> 1);
                    else
                        c = c >> 1;
                }
                table[i] = c;
            }
            tableComputed = true;
        }

        crc = crc ^ 0xFFFFFFFF;
        for (i4 i = 0; i < length; i++) {
            crc = table[(crc ^ buf[offset + i]) & 0xFF] ^ (crc >> 8);
        }
        return crc ^ 0xFFFFFFFF;
    }

    //native static int updateBytes(int crc, byte[] b, int off, int len);
    void updateBytes(Frame &frame) {
        const auto crc = frame.getLocalI4(0);
        const auto b = frame.getLocalRef(1);
        const auto off = frame.getLocalI4(2);
        const auto len = frame.getLocalI4(3);
        const auto buff = CAST_BYTE_TYPE_ARRAY_OOP(b);
        const auto buffPtr = buff->data.get();

        const auto ret = impUpdateBytes(crc, buffPtr, off, len);
        frame.returnI4(CAST_I4(ret));
    }
}


#endif