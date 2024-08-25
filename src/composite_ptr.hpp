#ifndef COMPOSITE_PTR_HPP
#define COMPOSITE_PTR_HPP
#include <type_traits>
#include "config.hpp"
#include "exception.hpp"

#define COMPOSITE_COMPRESS

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

#ifdef COMPOSITE_COMPRESS
        u8 composite{0};
#else
        P ptr{};
        T val{};
#endif

        explicit Composite() {
        }

        explicit Composite(P ptr, T data) {
            reset(ptr, data);
        }

        inline P getPtr() const {
#ifdef COMPOSITE_COMPRESS
            return std::bit_cast<P>(composite & COM_PTR_MASK);
#else
            return ptr;
#endif
        }

        inline T getData() const {
#ifdef COMPOSITE_COMPRESS
            return static_cast<T>((composite >> COM_PTR_LENGTH) & COM_DATA_SHIFT_MASK);
#else
            return val;
#endif
        }

        inline void reset(P p, T data) {
#ifdef COMPOSITE_COMPRESS
            if (CAST_U8(data) > COM_DATA_SHIFT_MASK) [[unlikely]] {
                panic("Composite not support");
            } 
            composite = (std::bit_cast<u8>(p) & COM_PTR_MASK) | (CAST_U8(data) << COM_PTR_LENGTH);
#else
            ptr = p;
            val = data;
#endif
        }

        inline void setPtr(P p) {
#ifdef COMPOSITE_COMPRESS
            composite &= ~COM_PTR_MASK;
            composite |= (std::bit_cast<u8>(p) & COM_PTR_MASK);
#else
            ptr = p;
#endif
        }

        inline void setData(T data) {
#ifdef COMPOSITE_COMPRESS
            composite &= ~COM_DATA_MASK;
            composite |= (CAST_U8(data) << COM_PTR_LENGTH);
#else
            val = data;
#endif
        }

    };

}

#endif