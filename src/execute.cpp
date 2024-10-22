#include "execute.hpp"
#include "config.hpp"
#include "utils/format.hpp"
#include "vm.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "mirror_oop.hpp"
#include "opcode.hpp"
#include "frame.hpp"
#include "interpreter.hpp"
#include "thread.hpp"
#include "basic_java_class.hpp"
#include "cfg.hpp"
#include "string_pool.hpp"
#include "garbage_collect.hpp"
#ifdef LLVM_JIT
#include "jit/llvm_jit_manager.hpp"
#endif

namespace RexVM {

    bool printExecuteLog{false};

    //Print Exception stack on stdout
    void throwToTopFrame(Frame &frame, InstanceOop *throwInstance) {
        const auto printStackTraceMethod = 
            throwInstance->getInstanceClass()->getMethod("printStackTrace" "()V", false);
        frame.runMethodManual(*printStackTraceMethod, {Slot(throwInstance)});
    }

    //return mark current frame return(throw to previous frame)
    bool handleThrowValue(Frame &frame) {
        // const auto throwInstance = CAST_INSTANCE_OOP(frame.popRef());
        const auto throwInstance = frame.throwValue;
        const auto throwInstanceClass = CAST_INSTANCE_CLASS(throwInstance->getClass());
        auto &method = frame.method;
        const auto handler =
            method.findExceptionHandler(
                throwInstanceClass,
                frame.pc()
            );

        if (handler) {
            //clean [operand] stack and push the catched exception to stack(must)
            frame.cleanOperandStack();
            frame.cleanThrow();

            frame.pushRef(throwInstance);
            const auto gotoOffset = *handler;
            frame.reader.gotoOffset(gotoOffset);
        } else {
            const auto previousFrame = frame.previous;
            if (previousFrame == nullptr) {
                //TOP Frame
                throwToTopFrame(frame, throwInstance);
                frame.cleanThrow();
                return true;
            }
            previousFrame->throwException(throwInstance);
            return true;
        }

        return false;
    }

    void handleThrowValueJIT(Frame &frame) {
        const auto throwInstance = frame.throwValue;
        const auto previousFrame = frame.previous;
        if (previousFrame == nullptr) {
            throwToTopFrame(frame, throwInstance);
            frame.cleanThrow();
            return;
        }
        previousFrame->throwException(throwInstance);
    }

    void checkAndPassReturnValue(const Frame &frame) {
        if (!frame.existReturnValue) {
            return;
        } 

        const auto previous = frame.previous;
        if (previous == nullptr) {
            panic("checkAndPassReturnValue error: previous is nullptr");
            return;
        }
        const auto returnValue = frame.returnValue;
        switch (frame.returnType) {
            case SlotTypeEnum::I4:
                previous->pushI4(returnValue.i4Val);
                break;

            case SlotTypeEnum::F4:
                previous->pushF4(returnValue.f4Val);
                break;

            case SlotTypeEnum::REF:
                previous->pushRef(returnValue.refVal);
                break;
                            
            case SlotTypeEnum::I8:
                previous->pushI8(returnValue.i8Val);
                break;

            case SlotTypeEnum::F8:
                previous->pushF8(returnValue.f8Val);
                break;

            default:
                panic("checkAndPassReturnValue error: unknown slot Type");
                break;
        }
    }

    void executeFrame(Frame &frame, [[maybe_unused]] cview methodName) {
        auto &method = frame.method;
        const auto notNativeMethod = !method.isNative();
        method.invokeCounter++;

#ifdef LLVM_JIT
        do {
            break;
            if (notNativeMethod && method.canCompile && method.compiledMethodHandler == nullptr) {
                if (const auto jitManager = frame.vm.jitManager.get(); jitManager != nullptr) {
                    jitManager->compileMethod(method);
                }
            }
        } while (false);
#endif

        // printExecuteLog = true;
        PRINT_EXECUTE_LOG(printExecuteLog, frame)
        frame.vm.garbageCollector->checkStopForCollect(frame.thread);

        if (notNativeMethod) [[likely]] {
            if (method.compiledMethodHandler != nullptr) {
                method.compiledMethodHandler(&frame, frame.localVariableTable, frame.localVariableTableType, &frame.throwValue);
                if (frame.markThrow) {
                    //JIT函数的异常 可以catch的在函数里已经完成 抛出的都是无法catch的
                    handleThrowValueJIT(frame);
                    return;
                }
                if (frame.markReturn) {
                    checkAndPassReturnValue(frame);
                    return;
                }
                frame.reader.resetCurrentOffset();
                return;
            }

            const auto &byteReader = frame.reader;
            while (!byteReader.eof()) {
                frame.pcCode = CAST_U4(byteReader.ptr - byteReader.begin);
                frame.currentByteCode = frame.reader.readU1();
                #ifdef DEBUG
                ATTR_UNUSED const auto pc = frame.pc();
                ATTR_UNUSED const auto opCode = static_cast<OpCodeEnum>(frame.currentByteCode);
                ATTR_UNUSED const auto sourceFile = method.klass.sourceFile;
                ATTR_UNUSED const auto lineNumber = method.getLineNumber(pc);
                #endif

                OpCodeHandlers[frame.currentByteCode](frame);

                if (frame.markThrow && handleThrowValue(frame)) {
                    return;
                }
                if (frame.markReturn) {
                    checkAndPassReturnValue(frame);
                    return;
                }
                frame.reader.resetCurrentOffset();
            }
        } else {
            const auto nativeMethodHandler = method.nativeMethodHandler;
            if (nativeMethodHandler == nullptr) {
                frame.printCallStack();
                panic(cformat("executeFrame error, method {}#{} nativeMethodHandler is nullptr", method.klass.toView(), method.toView()));
            }
            nativeMethodHandler(frame);
            if (frame.markThrow && handleThrowValue(frame)) {
                return;
            }
            if (frame.markReturn) {
                checkAndPassReturnValue(frame);
                return;
            }
        }
        if (method.slotType != SlotTypeEnum::NONE) {
            //not return but return slot type is not none
            panic(cformat("Method stack error: {}", method.toView()));
        }
    }

    //备份/恢复线程的currentFrame 以及 对于处理synchronized标记的方法
    inline void monitorExecuteFrame(Frame &frame) {
        auto &thread = frame.thread;
        //backup frame ptr
        const auto backupFrame = thread.currentFrame;
        thread.currentFrame = &frame;

        const auto &method = frame.method;
        EXCLUDE_EXECUTE_METHODS(method)
        auto lock = false;

        //monitor execute start
        ref monitorHandler = nullptr;
        if (method.isSynchronized()) [[unlikely]] {
            monitorHandler = method.isStatic() ? method.klass.getMirror(&frame) : frame.getThis();
            monitorHandler->lock();
            lock = true;
        }

#ifdef DEBUG
       executeFrame(frame, cformat("{}#{}", method.klass.toView(), method.toView()));
#else
        executeFrame(frame, "");
#endif

        if (lock) {
            monitorHandler->unlock();
        }
        //monitor execute end

        //recovery currentFrame
        thread.currentFrame = backupFrame;
    }

    void createFrameAndRunMethod(VMThread &thread, Method &method, Frame *previous, std::vector<Slot> params) {
        Frame nextFrame(thread, method, previous);
        if (method.paramSlotSize != params.size()) [[unlikely]] {
             panic(cformat("createFrameAndRunMethod error: params length {}", method.toView()));
        }

        if (!params.empty()) {
            std::ranges::copy(std::as_const(method.paramSlotType), nextFrame.localVariableTableType);
            std::ranges::copy(std::as_const(params), nextFrame.localVariableTable);
        }

        monitorExecuteFrame(nextFrame);
    }

    void createFrameAndRunMethodNoPassParams(VMThread &thread, Method &method, Frame *previous, size_t paramSlotSize) {
        Frame nextFrame(thread, method, previous, paramSlotSize);

        monitorExecuteFrame(nextFrame);
    }

}
