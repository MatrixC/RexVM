#include "stack.hpp"
#include "../oop.hpp"

namespace RexVM {

    StackContext::StackContext(Slot *memory, SlotTypeEnum *memoryType, const i4 pos) :
        sp(pos), maxSp(sp), memory(memory), memoryType(memoryType) {
    }

    void StackContext::push(const Slot val, const SlotTypeEnum slotType) {
        ++sp;
        memory[sp] = val;
        memoryType[sp] = slotType;
        if (sp > maxSp) {
            maxSp = sp;
        }
    }

    void StackContext::push(std::tuple<Slot, SlotTypeEnum> valWithType) {
        const auto [val, type] = valWithType;
        push(val, type);
    }

    Slot StackContext::pop() {
        return memory[sp--];
    }

    Slot StackContext::top() const {
        return memory[sp];
    }

    std::tuple<Slot, SlotTypeEnum> StackContext::popWithSlotType() {
       const auto val = std::make_tuple(memory[sp], memoryType[sp]);
       sp--;
       return val;
    }

    std::tuple<Slot, SlotTypeEnum> StackContext::topWithSlotType() const {
        return std::make_tuple(memory[sp], memoryType[sp]);
    }

    void StackContext::pop(i4 size) {
        sp -= size;
    }

    void StackContext::reset() {
        sp = -1;
    }

     Slot StackContext::getStackOffset(const size_t offset) const {
         return memory[sp - offset];
     }

    void StackContext::getStackObjects(std::vector<ref> &result) const {
        for (auto i = 0; i <= sp; ++i) {
            const auto &val = memory[i];
            const auto valType = memoryType[i];
            if (valType == SlotTypeEnum::REF && val.refVal != nullptr) {
                result.emplace_back(val.refVal);
            }
        }
    }

    [[nodiscard]] Slot *StackContext::getCurrentSlotPtr() const {
        return memory + sp;
    }

    [[nodiscard]] SlotTypeEnum *StackContext::getCurrentSlotTypePtr() const {
        return memoryType + sp;
    }

    void StackContext::dup() {
        push(topWithSlotType());
    }

    void StackContext::dup_x1() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        push(val1);
        push(val2);
        push(val1);
    }

    void StackContext::dup_x2() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        const auto val3 = popWithSlotType();
        push(val1);
        push(val3);
        push(val2);
        push(val1);
    }

    void StackContext::dup2() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        push(val2);
        push(val1);
        push(val2);
        push(val1);
    }

    void StackContext::dup2_x1() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        const auto val3 = popWithSlotType();
        push(val2);
        push(val1);
        push(val3);
        push(val2);
        push(val1);
    }

    void StackContext::dup2_x2() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        const auto val3 = popWithSlotType();
        const auto val4 = popWithSlotType();
        push(val2);
        push(val1);
        push(val4);
        push(val3);
        push(val2);
        push(val1);
    }

    void StackContext::swapTop() {
        const auto val1 = popWithSlotType();
        const auto val2 = popWithSlotType();
        push(val1);
        push(val2);
    }

}