#include "execute.hpp"
#include "config.hpp"
#include "utils/format.hpp"
#include "utils/class_utils.hpp"
#include "utils/string_utils.hpp"
#include "vm.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "mirror_oop.hpp"
#include "opcode.hpp"
#include "frame.hpp"
#include "interpreter.hpp"
#include "thread.hpp"
#include "basic_java_class.hpp"
#include "string_pool.hpp"
#include "garbage_collect.hpp"
#include "jit/llvm_jit_manager.hpp"

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
        //const auto throwInstance = frame.throwObject->throwValue;
        const auto throwInstance = frame.throwObject;
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
            frame.pushRef(throwInstance);

            const auto gotoOffset = *handler;
            frame.reader.gotoOffset(gotoOffset);
            frame.cleanThrow();
        } else {
            const auto previousFrame = frame.previous;
            if (previousFrame == nullptr) {
                //TOP Frame
                throwToTopFrame(frame, throwInstance);
                frame.cleanThrow();
                return true;
            } else {
                previousFrame->passException(frame.throwObject);
                return true;
            }
        }

        return false;
    }

    void checkAndPassReturnValue(Frame &frame) {
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

    void checkMethodCompile(const Frame &frame, Method &method) {
        if (const auto methodName = method.getName(); !startWith(methodName, "jicc")) {
            return;
        }
        if (method.compiledMethodHandler == nullptr) {
            const auto jitManager = frame.vm.jitManager.get();
            if (jitManager != nullptr) {
                jitManager->compileMethod(method);
            }
        }
    }

    void executeFrame(Frame &frame, [[maybe_unused]] cview methodName) {
        auto &method = frame.method;
        const auto notNativeMethod = !method.isNative();

        PRINT_EXECUTE_LOG(printExecuteLog, frame)
        checkMethodCompile(frame ,method);

        frame.vm.garbageCollector->checkStopForCollect(frame.thread);

        if (notNativeMethod) [[likely]] {
            if (method.compiledMethodHandler != nullptr) {
                method.compiledMethodHandler(&frame, frame.localVariableTable, frame.localVariableTableType);
                cprintln("jit run: {}", method.getName());
                if (frame.markThrow && handleThrowValue(frame)) {
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
        executeFrame(frame, cformat("{}#{}", method.klass.toView(), method.toView()));
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
            std::copy(method.paramSlotType.cbegin(), method.paramSlotType.cend(), nextFrame.localVariableTableType);
            std::copy(params.cbegin(), params.cend(), nextFrame.localVariableTable);
        }

        monitorExecuteFrame(nextFrame);
    }

    void createFrameAndRunMethodNoPassParams(VMThread &thread, Method &method, Frame *previous, size_t paramSlotSize) {
        Frame nextFrame(thread, method, previous, paramSlotSize);

        monitorExecuteFrame(nextFrame);
    }

}
