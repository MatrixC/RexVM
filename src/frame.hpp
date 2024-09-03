#ifndef FRAME_HPP
#define FRAME_HPP
#include <memory>
#include <vector>
#include <tuple>
#include "config.hpp"
#include "basic_type.hpp"
#include "utils/stack.hpp"
#include "utils/byte_reader.hpp"
#include "frame_memory_handler.hpp"

namespace RexVM {

    struct VM;
    struct VMThread;
    struct ClassLoader;
    struct ConstantInfo;
    struct InstanceClass;
    struct ObjArrayClass;
    struct Method;
    class Oop;
    struct InstanceOop;
    struct FrameMemoryHandler;
    
    struct FrameThrowable {
        InstanceOop *throwValue{nullptr};
        std::vector<std::tuple<Method&, u4>> throwPath;

        explicit FrameThrowable(InstanceOop *throwValue, Method &method, u4 throwPc);
        void addPath(Method &method, u4 throwPc);
        ~FrameThrowable();
    };

    struct Frame {
        Frame *previous{nullptr};
        size_t methodParamSlotSize{}; //仅在MethodHandle.invoke的时候需要使用 因为method.paramSlotSize是不准确的
        size_t localVariableTableSize;
        Slot *localVariableTable;
        SlotTypeEnum *localVariableTableType;
        StackContext operandStackContext;
        u1 currentByteCode{};
        u2 level{};

        VM &vm;
        VMThread &thread;
        Method &method;
        InstanceClass &klass;
        FrameMemoryHandler mem;
        ByteReader reader{};

        std::vector<std::unique_ptr<ConstantInfo>> &constantPool;
        ClassLoader &classLoader;

        bool markReturn{false};
        bool existReturnValue{false};
        Slot returnValue{};
        SlotTypeEnum returnType{};

        bool markThrow{false};
        //std::unique_ptr<FrameThrowable> throwObject;
        InstanceOop *throwObject{nullptr};

        explicit Frame(VMThread &thread, Method &method, Frame *previousFrame, size_t fixMethodParamSlotSize = 0);
        ~Frame();

        [[nodiscard]] ClassLoader *getCurrentClassLoader() const;

        void runMethodInner(Method &runMethod);
        void runMethodInner(Method &runMethod, size_t popSlotSize);
        std::tuple<Slot,SlotTypeEnum> runMethodManual(Method &runMethod_, std::vector<Slot> params);
        std::tuple<Slot, SlotTypeEnum> runMethodManualTypes(Method &runMethod, const std::vector<std::tuple<Slot, SlotTypeEnum>>& paramsWithType);
        void cleanOperandStack();
        
        [[nodiscard]] u4 pc() const;
        [[nodiscard]] u4 nextPc() const;

        //OperandStack Method
        void pushRef(ref ref);
        void pushI4(i4 val);
        void pushF4(f4 val);
        void pushI8(i8 val);
        void pushF8(f8 val);
        void push(Slot val, SlotTypeEnum type);

        ref popRef();
        i4 popI4();
        f4 popF4();
        i8 popI8();
        f8 popF8();
        Slot pop();
        std::tuple<Slot, SlotTypeEnum> popWithSlotType();

        void pushLocal(size_t index);
        void pushLocalWide(size_t index);
        void popLocal(size_t index);
        void popLocalWide(size_t index);

        //LocalVariableTable Method
        [[nodiscard]] ref getLocalRef(size_t index) const;
        [[nodiscard]] i4 getLocalI4(size_t index) const;
        [[nodiscard]] i8 getLocalI8(size_t index) const;
        [[nodiscard]] f4 getLocalF4(size_t index) const;
        [[nodiscard]] f8 getLocalF8(size_t index) const;
        [[nodiscard]] bool getLocalBoolean(size_t index) const;
        [[nodiscard]] Slot getLocal(size_t index) const;
        [[nodiscard]] std::tuple<Slot, SlotTypeEnum> getLocalWithType(size_t index) const;

        void setLocalRef(size_t index, ref val) const;
        void setLocalI4(size_t index, i4 val) const;
        void setLocalI8(size_t index, i8 val) const;
        void setLocalF4(size_t index, f4 val) const;
        void setLocalF8(size_t index, f8 val) const;
        void setLocalBoolean(size_t index, bool val) const;
        void setLocal(size_t index, Slot val, SlotTypeEnum type) const;

        void returnVoid();
        void returnSlot(Slot val, SlotTypeEnum type);
        void returnRef(ref val);
        void returnI4(i4 val);
        void returnI8(i8 val);
        void returnF4(f4 val);
        void returnF8(f8 val);
        void returnBoolean(bool val);

        void throwException(InstanceOop * val, u4 pc);
        void throwException(InstanceOop * val);
        //void passException(std::unique_ptr<FrameThrowable> lastException);
        void passException(InstanceOop *lastException);
        void cleanThrow();

        [[nodiscard]] Slot getStackOffset(size_t offset) const;
        
        [[nodiscard]] ref getThis() const;
        [[nodiscard]] InstanceOop *getThisInstance() const;
        void getLocalObjects(std::vector<ref> &result) const;
  
        void printCallStack();
        void printLocalSlot();
        void printStackSlot();
        void printCollectRoots();
        void printReturn();
        void printStr(ref oop);
        void print();

    };

}

#endif
