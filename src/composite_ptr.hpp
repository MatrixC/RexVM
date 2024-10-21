#ifndef COMPOSITE_PTR_HPP
#define COMPOSITE_PTR_HPP
#include <cstring>
#include "exception.hpp"

#define COMPOSITE_COMPRESS

namespace RexVM {

    template<typename T>
    concept PointerConcept = std::is_pointer_v<T>;

    using CompositeContainer = std::uint64_t;
    using CompositeData = std::uint16_t;

    constexpr CompositeContainer COM_PTR_LENGTH = 48;
    constexpr CompositeContainer COM_SIZE_LENGTH = 16;
    constexpr CompositeContainer COM_PTR_MASK =  0x0000ffffffffffff;
    constexpr CompositeContainer COM_DATA_SHIFT_MASK = 0xffff;
    constexpr CompositeContainer COM_DATA_MASK = 0xffff000000000000;

    template<PointerConcept P, typename T>
    struct Composite {

#ifdef COMPOSITE_COMPRESS
        CompositeContainer composite{0};
#else
        P ptr{};
        T val{};
#endif

        explicit Composite() = default;

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
            if (static_cast<CompositeContainer>(data) > COM_DATA_SHIFT_MASK) [[unlikely]] {
                panic("Composite not support");
            } 
            composite = (std::bit_cast<CompositeContainer>(p) & COM_PTR_MASK) | (static_cast<CompositeContainer>(data) << COM_PTR_LENGTH);
#else
            ptr = p;
            val = data;
#endif
        }

        inline void reset() {
            composite = 0;
        }

        inline void setPtr(P p) {
#ifdef COMPOSITE_COMPRESS
            composite &= ~COM_PTR_MASK;
            composite |= (std::bit_cast<CompositeContainer>(p) & COM_PTR_MASK);
#else
            ptr = p;
#endif
        }

        inline void setData(T data) {
#ifdef COMPOSITE_COMPRESS
            composite &= ~COM_DATA_MASK;
            composite |= (static_cast<CompositeContainer>(data) << COM_PTR_LENGTH);
#else
            val = data;
#endif
        }

    };



    template<typename Element>
    struct CompositeArray {
        using ArrayPtr = Element *;
        Composite<ArrayPtr, CompositeData> data;

        CompositeArray() = default;

        CompositeArray(ArrayPtr ptr, CompositeData size) : data(ptr, size) {
        }

        void copy(ArrayPtr src, CompositeData size) {
            const auto p = new Element[size];
            std::memcpy(p, src, sizeof(Element) * size);
            data.reset(p, size);
        }

        ~CompositeArray() {
            delete[] data.getPtr();
        }

    };

}

#endif