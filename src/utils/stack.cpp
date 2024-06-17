#include "stack.hpp"
#include "../oop.hpp"

namespace RexVM {

    StackContext::StackContext(Slot *memory, const i4 pos) :
        memory(memory), sp(pos) {
    }

    StackContext::StackContext(i4 size, i4 pos) :
        size(size), 
        sp(pos),
        useSelfMemory(true), 
        selfMemory(std::make_unique<SlotWithType[]>(size)) {
    }

    void StackContext::push(Slot val, SlotTypeEnum slotType) {
        ++sp;
        if (useSelfMemory) {
            selfMemory[sp] = { val, slotType };
        } else {
            memory[sp] = val;
        }
    }

    Slot StackContext::pop() {
        if (useSelfMemory) {
            return selfMemory[sp--].slot;
        } else {
            return memory[sp--];
        }
    }

    void StackContext::reset() {
        sp = -1;
    }

     Slot StackContext::getStackOffset(size_t offset) const {
        if (useSelfMemory) {
            return selfMemory[sp - offset].slot;
        } else {
            return memory[sp - offset];
        }
     }

     std::vector<Oop *> StackContext::getObjects() const {
        std::vector<Oop *> result;
        if (useSelfMemory) {
            for (auto i = 0; i <= sp; ++i) {
                const auto &val = selfMemory[i];
                if (val.type == SlotTypeEnum::REF && val.slot.refVal != nullptr) {
                    result.push_back(static_cast<Oop *>(val.slot.refVal));
                }
            }
        }

        return result;
     }

}