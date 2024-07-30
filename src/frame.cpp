#include "frame.hpp"
#include "constant_info.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "vm.hpp"
#include "execute.hpp"
#include "print_helper.hpp"
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

    ClassLoader *Frame::getCurrentClassLoader() const {
        //TODO: This klass's class loader
        return vm.bootstrapClassLoader.get();
    }

    //java方法执行过程中用,被invoke系列指令调用,参数都是字节码提前push好的
    //先pop上一个Frame, 移动其sp.因为新Frame的Top是根据上一个Frame的操作数栈sp来计算的
    //所以可以直接将新Frame的Local区对齐到上个Frame push的参数起点,用来减少一次参数写入
    void Frame::runMethodInner(Method &runMethod) {
        const auto slotSize = runMethod.paramSlotSize;
        if (slotSize > 0) {
            operandStackContext.pop(CAST_I4(slotSize));
        }
        createFrameAndRunMethodNoPassParams(thread, runMethod, this, slotSize);
    }

    void Frame::runMethodInner(Method &runMethod, size_t popSlotSize) {
        if (popSlotSize > 0) {
            operandStackContext.pop(CAST_I4(popSlotSize));
        }
        createFrameAndRunMethodNoPassParams(thread, runMethod, this, popSlotSize);
    }

    //手动调用java方法用,创建新Frame,用params向其传递参数
    std::tuple<Slot, SlotTypeEnum> Frame::runMethodManual(Method &runMethod, std::vector<Slot> params) {
        createFrameAndRunMethod(thread, runMethod, std::move(params), this);
        //Method可能有返回值,需要处理返回值的pop
        if (runMethod.returnSlotType == SlotTypeEnum::NONE) {
            //void函数，无返回
            return std::make_tuple(ZERO_SLOT, SlotTypeEnum::NONE);
        } else {
            if (runMethod.returnSlotType == SlotTypeEnum::I8) {
                return std::make_tuple(Slot(popI8()), runMethod.returnSlotType);
            } else if (runMethod.returnSlotType == SlotTypeEnum::F8) {
                return std::make_tuple(Slot(popF8()), runMethod.returnSlotType);
            } else {
                //除了I8和F8,其他类型只需要pop一个值
                return std::make_tuple(pop(), runMethod.returnSlotType);
            }
        }
    }

    std::tuple<Slot, SlotTypeEnum> Frame::runMethodManualTypes(
        Method &runMethod, 
        const std::vector<std::tuple<Slot, SlotTypeEnum>>& paramsWithType
    ) {
        std::vector<Slot> params;
        params.reserve(paramsWithType.size());
        for (const auto &item : paramsWithType) {
            params.emplace_back(std::get<0>(item));
        }
        return runMethodManual(runMethod, params);
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
        operandStackContext.push(Slot(CAST_I8(0)), SlotTypeEnum::I8);
    }
    
    void Frame::pushF8(f8 val) {
        operandStackContext.push(Slot(val), SlotTypeEnum::F8);
        operandStackContext.push(Slot(CAST_F8(0)), SlotTypeEnum::F8);
    }

    void Frame::push(Slot val, SlotTypeEnum type) {
        operandStackContext.push(val, type);
        if (isWideSlotType(type)) {
            operandStackContext.push(Slot(CAST_I8(0)), type);
        }
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

    std::tuple<Slot, SlotTypeEnum> Frame::popWithSlotType() {
        return operandStackContext.popWithSlotType();
    }

    void Frame::pushLocal(size_t index) {
        const auto val = localVariableTable[index];
        const auto type = localVariableTableType[index];
        operandStackContext.push(val, type);
    }

    void Frame::pushLocalWide(size_t index) {
        const auto val1 = localVariableTable[index];
        const auto val1Type = localVariableTableType[index];
        const auto val2 = localVariableTable[index + 1];
        const auto val2Type = localVariableTableType[index + 1];
        operandStackContext.push(val1, val1Type);
        operandStackContext.push(val2, val2Type);
    }

    void Frame::popLocal(size_t index) {
        const auto [val, type] = operandStackContext.popWithSlotType();
        localVariableTable[index] = val;
        localVariableTableType[index] = type;
    }

    void Frame::popLocalWide(size_t index) {
        const auto [val1, val1Type] = operandStackContext.popWithSlotType();
        const auto [val2, val2Type] = operandStackContext.popWithSlotType();
        localVariableTable[index] = val2;
        localVariableTableType[index] = val2Type;
        localVariableTable[index + 1] = val1;
        localVariableTableType[index + 1] = val1Type;
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

    std::tuple<Slot, SlotTypeEnum> Frame::getLocalWithType(size_t index) const {
        return std::make_tuple(localVariableTable[index], localVariableTableType[index]);
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

    void Frame::returnSlot(Slot val, SlotTypeEnum type) {
        markReturn = true;
        existReturnValue = true;
        returnType = type;
        returnValue = val;
    }

    void Frame::returnRef(ref val) {
        returnSlot(Slot(val), SlotTypeEnum::REF);
    }

    void Frame::returnI4(i4 val) {
        returnSlot(Slot(val), SlotTypeEnum::I4);
    }

    void Frame::returnI8(i8 val) {
        returnSlot(Slot(val), SlotTypeEnum::I8);
    }

    void Frame::returnF4(f4 val) {
        returnSlot(Slot(val), SlotTypeEnum::F4);
    }

    void Frame::returnF8(f8 val) {
        returnSlot(Slot(val), SlotTypeEnum::F8);
    }

    void Frame::returnBoolean(bool val) {
        returnSlot(Slot(val ? CAST_I8(1) : CAST_I8(0)), SlotTypeEnum::I4);
    }

    void Frame::throwException(InstanceOop * const val, u4 throwPc) {
        markThrow = true;
        //throwObject = std::make_unique<FrameThrowable>(val, method, throwPc);
        throwObject = val;
    }

    void Frame::throwException(InstanceOop * const val) {
        throwException(val, pc());
    }

    // void Frame::passException(std::unique_ptr<FrameThrowable> lastException) {
    //     markThrow = true;
    //     throwObject = std::move(lastException);
    //     throwObject->addPath(method, pc());
    // }

    void Frame::passException(InstanceOop *lastException) {
        markThrow = true;
        throwObject = lastException;
    }

    void Frame::cleanThrow() {
        markThrow = false;
        //throwObject.reset();
        throwObject = nullptr;
    }


    Slot Frame::getStackOffset(size_t offset) const {
        return operandStackContext.getStackOffset(offset);
    }


    Oop *Frame::getThis() const {
        return getLocalRef(0);
    }

    InstanceOop *Frame::getThisInstance() const {
        return CAST_INSTANCE_OOP(getThis());
    }

    std::vector<Oop *> Frame::getLocalObjects() const {
        std::vector<Oop *> result;
        for (size_t i = 0; i < localVariableTableSize; ++i) {
            const auto value = localVariableTable[i];
            if (localVariableTableType[i] == SlotTypeEnum::REF && value.refVal != nullptr) {
                 result.emplace_back(value.refVal);
            }
        }
        return result;
    }

    void Frame::printCallStack() {
        for (auto f = this; f != nullptr; f = f->previous) {
            const auto nativeMethod = method.isNative();
            cprintln("  {}#{}:{} {}", klass.name, method.name, method.descriptor, nativeMethod ? "[Native]" : "");
        }
    }

    void Frame::printLocalSlot() {
        for (size_t i = 0; i < localVariableTableSize; ++i) {
            const auto val = localVariableTable[i];
            const auto type = localVariableTableType[i];
            const auto slotStr = formatSlot(*this, val, type);
            cprintln("Local[{}]: {}", i, slotStr);
        }
    }

    void Frame::printStackSlot() {
        for (i4 offset = 0; offset <= operandStackContext.sp; ++offset) {
            const auto index = operandStackContext.sp - offset;
            const auto val = operandStackContext.memory[index];
            const auto valType = operandStackContext.memoryType[index];
            const auto slotStr = formatSlot(*this, val, valType);
            cprintln("Stack[{}]: {}", offset, slotStr);
        }
    }

    void Frame::print() {
        printLocalSlot();
        printStackSlot();
    }

}