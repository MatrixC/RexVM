#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>

namespace RexVM {

    struct Oop;

    using u1 = std::uint8_t;
    using u2 = std::uint16_t;
    using u4 = std::uint32_t;
    using u8 = std::uint64_t;
    using i1 = std::int8_t;
    using i2 = std::int16_t;
    using i4 = std::int32_t;
    using i8 = std::int64_t;
    using f4 = float;
    using f8 = double;
    //using ref = void *;
    using ref = Oop *;
    using size_t = std::size_t;
    using cchar = char8_t;
    using cchar_16 = char16_t;
    using cstring = std::string;
    using cview = std::string_view;
    using ustring = std::u16string;

    union Slot {
        i4 i4Val{0};
        i8 i8Val;
        f4 f4Val;
        f8 f8Val;
        ref refVal;

        Slot() {
        }

        explicit Slot(i4 val) : i4Val(val) {
        }

        explicit Slot(i8 val) : i8Val(val) {
        }

        explicit Slot(f4 val) : f4Val(val) {
        }

        explicit Slot(f8 val) : f8Val(val) {
        }

        explicit Slot(ref val) : refVal(val) {
        }
    };

    enum class SlotTypeEnum : u1 {
        UNKNOWN,
        I4,
        F4,
        I8,
        F8,
        REF,
    };

    inline bool isWideSlotType(SlotTypeEnum slotType) {
        return slotType == SlotTypeEnum::I8 || slotType == SlotTypeEnum::F8;
    }

    inline bool isWideSlotType(cchar first) {
        return first == 'J' || first == 'D';
    }

    inline SlotTypeEnum getSlotTypeByDescriptorFirstChar(cchar first) {
        switch (first) {
            case 'L':
            case '[':
                return SlotTypeEnum::REF;
            case 'F':
                return SlotTypeEnum::F4;
            case 'J':
                return SlotTypeEnum::I8;
            case 'D':
                return SlotTypeEnum::F8;
            default:
                return SlotTypeEnum::I4;
        }
    }

    //1MB byte memory
    constexpr std::size_t STACK_MEMORY_BYTE_SIZE = 1024 * 1024;
    constexpr std::size_t SLOT_BYTE_SIZE = sizeof(Slot);
    constexpr std::size_t STACK_SLOT_SIZE = STACK_MEMORY_BYTE_SIZE / SLOT_BYTE_SIZE;

    constexpr size_t THREAD_STACK_SLOT_SIZE = 16384;
    //128KB
    constexpr size_t THREAD_STACK_MEMORY_KB = THREAD_STACK_SLOT_SIZE * sizeof(Slot) / 1024;

    const cstring EMPTY_STRING{};

    struct Frame;
    using MethodHandler = void (*)(Frame &frame);
    using NativeMethodHandler = MethodHandler;
}


#endif