#include "frame.hpp"
#include "constant_info.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "vm.hpp"
#include "execute.hpp"
#include <utility>


namespace RexVM {

    FrameThrowable::FrameThrowable(InstanceOop *throwValue, Method &method, u4 throwPc) : throwValue(throwValue) {
        addPath(method, throwPc);
    }

    void FrameThrowable::addPath(Method &method, u4 throwPc) {
        throwPath.emplace_back(method, throwPc);
    }

    FrameThrowable::~FrameThrowable() = default;

    Frame::Frame(VM &vm, VMThread &thread, Method &method, Frame *previousFrame) :
            previous(previousFrame),
            localVariableTableSize(method.maxLocals == 0 ? method.paramSlotSize : method.maxLocals),
            localVariableTable(
                    previous == nullptr ?
                        thread.stackMemory.get() :
                        previous->operandStackContext.getCurrentSlotPtr() + 1), //Previous last operand slot + 1
            localVariableTableType(
                    previous == nullptr ?
                        thread.stackMemoryType.get() :
                        previous->operandStackContext.getCurrentSlotTypePtr() + 1), //Previous last operand slot + 1
            operandStackContext(
                    localVariableTable + localVariableTableSize,
                    localVariableTableType + localVariableTableSize,
                    -1
            ),
            vm(vm),
            thread(thread),
            method(method),
            klass(method.klass),
            constantPool(klass.constantPool),
            classLoader(klass.classLoader) {
        const auto nativeMethod = method.isNative();
        if (!nativeMethod) {
            auto codePtr = method.code.get();
            reader.init(codePtr, method.codeLength);
        }

        if (previous != nullptr) {
            level = previous->level + 1;
        }
    }

    Frame::~Frame() = default;

    void Frame::runMethod(Method &runMethod_) {
        const auto slotSize = runMethod_.paramSlotSize;
        if (slotSize > 0) {
            operandStackContext.pop(static_cast<i4>(slotSize));
        }
        createFrameAndRunMethodNoPassParams(thread, runMethod_, this);
    }

    void Frame::runMethod(Method &runMethod_, std::vector<Slot> params) {
        createFrameAndRunMethod(thread, runMethod_, std::move(params), this);
    }

    void Frame::cleanOperandStack() {
        operandStackContext.reset();
    }

    u4 Frame::pc() const {
        return reader.ptr - method.code.get() - 1;
    }

    u4 Frame::nextPc() const {
        return reader.ptr - method.code.get();
    }

    void Frame::pushRef(ref ref) {
        operandStackContext.push(Slot(ref), SlotTypeEnum::REF);
    }

    void Frame::pushI4(i4 val) {
        operandStackContext.push(Slot(val), SlotTypeEnum::I4);
    }

    void Frame::pushF4(f4 val) {
        operandStackContext.push(Slot(val), SlotTypeEnum::F4);
    }
    
    void Frame::pushI8(i8 val) {
        operandStackContext.push(Slot(val), SlotTypeEnum::I8);
        operandStackContext.push(Slot(static_cast<i8>(0)), SlotTypeEnum::I8);
    }
    
    void Frame::pushF8(f8 val) {
        operandStackContext.push(Slot(val), SlotTypeEnum::F8);
        operandStackContext.push(Slot(static_cast<f8>(0)), SlotTypeEnum::F8);
    }

    void Frame::push(Slot val, SlotTypeEnum type) {
        operandStackContext.push(val, type);
    }

    ref Frame::popRef() {
        return operandStackContext.pop().refVal;
    }
    
    i4 Frame::popI4() {
        return operandStackContext.pop().i4Val;
    }
    
    f4 Frame::popF4() {
        return operandStackContext.pop().f4Val;
    }
    
    i8 Frame::popI8() {
        operandStackContext.pop();
        return operandStackContext.pop().i8Val;
    }

    f8 Frame::popF8() {
        operandStackContext.pop();
        return operandStackContext.pop().f8Val;
    }

    Slot Frame::pop() {
        return operandStackContext.pop();
    }

    void Frame::pushLocal(size_t index) {
        const auto val = localVariableTable[index];
        operandStackContext.push(val, SlotTypeEnum::I4);
    }

    void Frame::pushLocalWide(size_t index) {
        const auto val1 = localVariableTable[index];
        const auto val2 = localVariableTable[index + 1];
        operandStackContext.push(val1, SlotTypeEnum::I4);
        operandStackContext.push(val2, SlotTypeEnum::I4);
    }

    void Frame::popLocal(size_t index) {
        const auto val = operandStackContext.pop();
        localVariableTable[index] = val;
    }

    void Frame::popLocalWide(size_t index) {
        const auto val1 = operandStackContext.pop();
        const auto val2 = operandStackContext.pop();
        localVariableTable[index] = val2;
        localVariableTable[index + 1] = val1;
    }

    ref Frame::getLocalRef(size_t index) const {
        return localVariableTable[index].refVal;
    }

    i4 Frame::getLocalI4(size_t index) const {
        return localVariableTable[index].i4Val; 
    }

    i8 Frame::getLocalI8(size_t index) const {
        return localVariableTable[index].i8Val;
    }

    f4 Frame::getLocalF4(size_t index) const {
        return localVariableTable[index].f4Val;
    }

    f8 Frame::getLocalF8(size_t index) const {
        return localVariableTable[index].f8Val;
    }

    bool Frame::getLocalBoolean(size_t index) const {
        return localVariableTable[index].i4Val != 0;
    }

    Slot Frame::getLocal(size_t index) const {
        return localVariableTable[index];
    }

    void Frame::setLocalRef(size_t index, ref val) const {
        setLocal(index, Slot(val), SlotTypeEnum::REF);
    }

    void Frame::setLocalI4(size_t index, i4 val) const {
        setLocal(index, Slot(val), SlotTypeEnum::I4);
    }

    void Frame::setLocalI8(size_t index, i8 val) const {
        setLocal(index, Slot(val), SlotTypeEnum::I8);
    }

    void Frame::setLocalF4(size_t index, f4 val) const {
        setLocal(index, Slot(val), SlotTypeEnum::F4);
    }

    void Frame::setLocalF8(size_t index, f8 val) const {
        setLocal(index, Slot(val), SlotTypeEnum::F8);
    }

    void Frame::setLocalBoolean(size_t index, bool val) const {
        setLocal(index, Slot(val), SlotTypeEnum::I4);
    }

    void Frame::setLocal(size_t index, Slot val, SlotTypeEnum type) const {
        localVariableTableType[index] = type;
        localVariableTable[index] = val;
    }

    void Frame::returnVoid() {
        markReturn = true;
        existReturnValue = false;
    }

    void Frame::returnRef(ref val) {
        markReturn = true;
        existReturnValue = true;
        returnType = SlotTypeEnum::REF;
        returnValue = Slot(val);
    }

    void Frame::returnI4(i4 val) {
        markReturn = true;
        existReturnValue = true;
        returnType = SlotTypeEnum::I4;
        returnValue = Slot(val);
    }

    void Frame::returnI8(i8 val) {
        markReturn = true;
        existReturnValue = true;
        returnType = SlotTypeEnum::I8;
        returnValue = Slot(val);
    }

    void Frame::returnF4(f4 val) {
        markReturn = true;
        existReturnValue = true;
        returnType = SlotTypeEnum::F4;
        returnValue = Slot(val);
    }

    void Frame::returnF8(f8 val) {
        markReturn = true;
        existReturnValue = true;
        returnType = SlotTypeEnum::F8;
        returnValue = Slot(val);
    }

    void Frame::returnBoolean(bool val) {
        markReturn = true;
        existReturnValue = true;
        returnType = SlotTypeEnum::I4;
        returnValue = Slot(val ? 1 : 0);
    }

    void Frame::throwException(InstanceOop * const val, u4 throwPc) {
        markThrow = true;
        throwObject = std::make_unique<FrameThrowable>(val, method, throwPc);
    }

    void Frame::passException(std::unique_ptr<FrameThrowable> lastException) {
        markThrow = true;
        throwObject = std::move(lastException);
        throwObject->addPath(method, pc());
    }

    void Frame::cleanThrow() {
        markThrow = false;
        throwObject.reset();
    }


    Slot Frame::getStackOffset(size_t offset) const {
        return operandStackContext.getStackOffset(offset);
    }

    Oop *Frame::getThis() const {
        return getLocalRef(0);
    }

    InstanceOop *Frame::getThisInstance() const {
        return static_cast<InstanceOop *>(getThis());
    }

    std::vector<Oop *> Frame::getLocalObjects() const {
        std::vector<Oop *> result;
        for (size_t i = 0; i < localVariableTableSize; ++i) {
            const auto value = localVariableTable[i];
            if (localVariableTableType[i] == SlotTypeEnum::REF && value.refVal != nullptr) {
                 result.emplace_back(static_cast<Oop *>(value.refVal));
            }
        }
        return result;
    }

}