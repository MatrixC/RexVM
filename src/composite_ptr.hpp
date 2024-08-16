#ifndef COMPOSITE_PTR_HPP
#define COMPOSITE_PTR_HPP
#include <type_traits>
#include "config.hpp"
#include "exception.hpp"

namespace RexVM {

    template<typename T>
    concept PointerConcept = std::is_pointer_v<T>;

    constexpr u8 COM_PTR_LENGTH = 48;
    constexpr u8 COM_SIZE_LENGTH = 16;
    constexpr u8 COM_PTR_MASK =  0x0000ffffffffffff;
    constexpr u8 COM_DATA_SHIFT_MASK = 0xffff;
    constexpr u8 COM_DATA_MASK = 0xffff000000000000;

    template<PointerConcept P, typename T>
    struct Composite {

        u8 composite;

        explicit Composite() : composite(CAST_U8(0)) {
        }

        explicit Composite(P ptr, T data) {
            reset(ptr, data);
        }

        inline P getPtr() const {
            return std::bit_cast<P>(composite & COM_PTR_MASK);
        }

        inline T getData() const {
            return static_cast<T>((composite >> COM_PTR_LENGTH) & COM_DATA_SHIFT_MASK);
        }

        inline void reset(P ptr, T data) {
            if (data > COM_DATA_SHIFT_MASK) [[unlikely]] {
                panic("Composite not support");
            } 
            composite = (std::bit_cast<u8>(ptr) & COM_PTR_MASK) | (CAST_U8(data) << COM_PTR_LENGTH);
        }

        inline void setPtr(P ptr) {
            composite &= ~COM_PTR_MASK;
            composite |= (std::bit_cast<u8>(ptr) & COM_PTR_MASK);
        }

        inline void setData(T data) {
            composite &= ~ COM_DATA_MASK;
            composite |= (CAST_U8(data) << COM_PTR_LENGTH);
        }

    };

}

#endif