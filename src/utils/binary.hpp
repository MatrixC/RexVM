#ifndef BINARY_HPP
#define BINARY_HPP

#include <bit>
#include <fstream>
#include <memory>
#include "format.hpp"
#include "../config.hpp"

namespace RexVM {

    inline u2 swap16(const u2 x) noexcept {
        if constexpr (std::endian::native == std::endian::little) {
            return ((x & 0x00FF) << 8) |
                   ((x & 0xFF00) >> 8);
        } else {
            return x;
        }
    }

    inline u4 swap32(u4 x) noexcept {
        if constexpr (std::endian::native == std::endian::little) {
            return ((x & 0xFF000000) >> 24) |
                   ((x & 0x00FF0000) >> 8) |
                   ((x & 0x0000FF00) << 8) |
                   ((x & 0x000000FF) << 24);
        } else {
            return x;
        }
    }

    inline u8 swap64(u8 x) noexcept {
        if constexpr (std::endian::native == std::endian::little) {
            return ((x & 0xFF00000000000000) >> 56) |
                   ((x & 0x00FF000000000000) >> 40) |
                   ((x & 0x0000FF0000000000) >> 24) |
                   ((x & 0x000000FF00000000) >> 8) |
                   ((x & 0x00000000FF000000) << 8) |
                   ((x & 0x0000000000FF0000) << 24) |
                   ((x & 0x000000000000FF00) << 40) |
                   ((x & 0x00000000000000FF) << 56);
        } else {
            return x;
        }
    }


    template<typename E>
    inline E byteSwap(E e) {
//        if constexpr (std::endian::native == std::endian::little) {
//            return std::byteswap(e);
//        } else {
//            return e;
//        }
        constexpr auto size = sizeof(E);
        if constexpr (size == 1) {
            return e;
        } else if constexpr (size == 2) {
            return swap16(e);
        } else if constexpr (size == 4) {
            return swap32(e);
        } else {
            return swap64(e);
        }
    }

    template<typename T>
    concept StreamByteType = std::is_same_v<T, u1> || std::is_same_v<T, u2> ||
                             std::is_same_v<T, u4> || std::is_same_v<T, u8> ||
                             std::is_same_v<T, i1> || std::is_same_v<T, i2> ||
                             std::is_same_v<T, i4> || std::is_same_v<T, i8>;

    template<StreamByteType T>
    T read(std::istream &is) {
        constexpr auto size = sizeof(T);
        T val;
        is.read(reinterpret_cast<char *>(&val), size);
        return byteSwap(val);
    }

    template<StreamByteType T>
    void read(T &val, std::istream &is) {
        val = read<T>(is);
    }

    template<StreamByteType T>
    T peek(std::istream &is) {
        constexpr auto size = sizeof(T);
        if constexpr (size == 1) {
            return is.peek();
        }
        const auto &fpos = is.tellg();
        const auto val = read<T>(is);
        is.seekg(fpos);
        return val;
    }

    template<std::size_t ...Is>
    consteval void loop(std::index_sequence<Is...>, auto &&f) {
        (f.template operator()<Is>(), ...);
    }

    template<typename T>
    void printHex(const T &t) {
        constexpr auto size = sizeof(T);
        loop(
                std::make_index_sequence<size>(),
                [&]<std::size_t n> {
                    constexpr auto rn = size - n - 1;
                    const u1 v = (t >> (rn * 8)) & 0xFF;
                    cprint("{:02X}", v & 0xFF);
                    if constexpr (n != size - 1) {
                        cprint(" ");
                    }
                }
        );
    }

    template<StreamByteType T, StreamByteType ...Args>
    void printStreamBytes(const T &firstArg, const Args &...args) {
        printHex(firstArg);
        if constexpr (sizeof...(args) > 0) {
            cprint(" ");
            printStreamBytes(args...);
        }
    }

    inline std::unique_ptr<u1[]> readBuffer(std::istream &is, const std::size_t size) {
        auto buffer = std::make_unique<u1[]>(size);
        is.read(reinterpret_cast<char *>(buffer.get()), size);
        return buffer;
    }

    template<StreamByteType T>
    T read(const u1 *bytes) {
        constexpr auto size = sizeof(T);
        T val;
        memcpy(&val, bytes, size);
        return byteSwap(val);
    }

}

#endif