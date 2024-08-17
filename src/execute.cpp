#include "execute.hpp"
#include "config.hpp"
#include "utils/format.hpp"
#include "utils/class_utils.hpp"
#include "utils/string_utils.hpp"
#include "vm.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "opcode.hpp"
#include "frame.hpp"
#include "interpreter.hpp"
#include "thread.hpp"
#include "basic_java_class.hpp"
#include "string_pool.hpp"
#include "garbage_collect.hpp"

namespace RexVM {

    bool printExecuteLog{false};

    //Print Exception stack on stdout
    void throwToTopFrame(Frame &frame, InstanceOop *throwInstance) {
        const auto printStackTraceMethod = 
            throwInstance->getInstanceClass()->getMethod("printStackTrace", "()V", false);
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

    void executeFrame(Frame &frame, [[maybe_unused]] const cstring& methodName) {
        auto &method = frame.method;
        const auto notNativeMethod = !method.isNative();

        PRINT_EXECUTE_LOG(printExecuteLog, frame)

        frame.vm.garbageCollector->checkStopForCollect(frame);

        if (notNativeMethod) [[likely]] {
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
                panic("executeFrame error, method " + method.klass.name + "#" + method.name + ":" + method.descriptor + " nativeMethodHandler is nullptr");
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
        if (method.returnSlotType != SlotTypeEnum::NONE) {
            panic("Method stack error: " + method.name);
        }
    }

    void monitorExecuteFrame(Frame &frame, const cstring &methodName) {
        const auto &method = frame.method;
        EXCLUDE_EXECUTE_METHODS(method)
        auto lock = false;
        ref monitorHandler = nullptr;
        if (method.isSynchronized()) [[unlikely]] {
            monitorHandler = method.isStatic() ? method.klass.getMirror(&frame) : frame.getThis();
            monitorHandler->lock();
            lock = true;
        }
        executeFrame(frame, methodName);
        if (lock) {
            monitorHandler->unlock();
        }
    }

    void createFrameAndRunMethod(VMThread &thread, Method &method_, std::vector<Slot> params, Frame *previous, bool nativeCall) {
        Frame nextFrame(thread.vm, thread, method_, previous);
        nextFrame.nativeCall = nativeCall;
        const auto slotSize = method_.paramSlotSize;
        nextFrame.methodParamSlotSize = slotSize;
        if (slotSize != params.size()) {
             panic("createFrameAndRunMethod error: params length " + method_.name);
        }
        FOR_FROM_ZERO(params.size()) {
            const auto slotType = method_.getParamSlotType(i);
            nextFrame.setLocal(i, params[i], slotType);
        }
        const auto backupFrame = thread.currentFrame;
        thread.currentFrame = &nextFrame;
        const auto methodName = method_.klass.name + "#" + method_.name;
        monitorExecuteFrame(nextFrame, methodName);
        thread.currentFrame = backupFrame;
    }

    void createFrameAndRunMethodNoPassParams(VMThread &thread, Method &method_, Frame *previous, size_t paramSlotSize, bool nativeCall) {
        Frame nextFrame(thread.vm, thread, method_, previous, paramSlotSize);
        nextFrame.nativeCall = nativeCall;
        nextFrame.methodParamSlotSize = paramSlotSize;
        const auto backupFrame = thread.currentFrame;
        thread.currentFrame = &nextFrame;
        const auto methodName = method_.klass.name + "#" + method_.name;
        monitorExecuteFrame(nextFrame, methodName);
        thread.currentFrame = backupFrame;
    }

}