#ifndef FRAME_HPP
#define FRAME_HPP
#include <memory>
#include <vector>
#include <tuple>
#include "config.hpp"
#include "basic_type.hpp"
#include "utils/stack.hpp"
#include "utils/byte_reader.hpp"

namespace RexVM {

    struct VM;
    struct VMThread;
    struct ClassLoader;
    struct ConstantInfo;
    struct InstanceClass;
    struct Method;
    struct Oop;
    struct InstanceOop;

    struct FrameThrowable {
        InstanceOop *throwValue{nullptr};
        std::vector<std::tuple<Method&, u4>> throwPath;

        explicit FrameThrowable(InstanceOop *throwValue, Method &method, u4 throwPc);
        void addPath(Method &method, u4 throwPc);
        ~FrameThrowable();
    };

    struct Frame {
        Frame *previous{nullptr};
        size_t localVariableTableSize;
        Slot *localVariableTable;
        SlotTypeEnum *localVariableTableType;
        StackContext operandStackContext;
        u1 currentByteCode{};
        u2 level{0};

        VM &vm;
        VMThread &thread;
        Method &method;
        InstanceClass &klass;
        ByteReader reader{};

        std::vector<std::unique_ptr<ConstantInfo>> &constantPool;
        ClassLoader &classLoader;

        bool markReturn{false};
        bool existReturnValue{false};
        Slot returnValue{};
        SlotTypeEnum returnType{};

        bool markThrow{false};
        std::unique_ptr<FrameThrowable> throwObject;

        explicit Frame(VM &vm, VMThread &thread, Method &method, Frame *previousFrame);
        ~Frame();

        ClassLoader *getCurrentClassLoader() const;

        void runMethod(Method &runMethod_);
        void runMethod(Method &runMethod_, std::vector<Slot> params);
        Slot runMethodGetReturn(Method &runMethod_, std::vector<Slot> params);
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

        void setLocalRef(size_t index, ref val) const;
        void setLocalI4(size_t index, i4 val) const;
        void setLocalI8(size_t index, i8 val) const;
        void setLocalF4(size_t index, f4 val) const;
        void setLocalF8(size_t index, f8 val) const;
        void setLocalBoolean(size_t index, bool val) const;
        void setLocal(size_t index, Slot val, SlotTypeEnum type) const;

        void returnVoid();
        void returnRef(ref val);
        void returnI4(i4 val);
        void returnI8(i8 val);
        void returnF4(f4 val);
        void returnF8(f8 val);
        void returnBoolean(bool val);

        void throwException(InstanceOop * val, u4 pc);
        void passException(std::unique_ptr<FrameThrowable> lastException);
        void cleanThrow();

        [[nodiscard]] Slot getStackOffset(size_t offset) const;

        [[nodiscard]] Oop *getThis() const;
        [[nodiscard]] InstanceOop *getThisInstance() const;
        [[nodiscard]] std::vector<Oop *> getLocalObjects() const;
  
        void printCallStack() const;
    };

}

#endif
