#include "config.hpp"
#include "utils/binary.hpp"
#include "utils/class_utils.hpp"
#include "class_loader.hpp"
#include "class.hpp"
#include "class_member.hpp"
#include "oop.hpp"
#include "runtime.hpp"
#include "constant_info.hpp"
#include "constant_pool.hpp"
#include "vm.hpp"
#include "opcode.hpp"
#include "frame.hpp"
#include "memory.hpp"
#include "interpreter.hpp"

namespace RexVM {

    Thread::Thread(VM &vm, Executor &executor) : 
        vm(vm), 
        executor(executor) 
    {
        const auto &oopManager = vm.oopManager;
        vmThread = oopManager->newThreadOop(this);
    }

    Thread::~Thread() {
    }

    std::vector<Oop *> Thread::getThreadGCRoots() const {
        std::vector<Oop *> result;
        for (auto cur = currentFrame; cur != nullptr; cur = cur->previous) {
            const auto localObjects = cur->getLocalObjects();
            const auto stackObjects = cur->operandStackContext.getObjects();
            if (localObjects.size() > 0) {
                result.insert(result.end(), localObjects.begin(), localObjects.end());
            }
            if (stackObjects.size() > 0) {
                result.insert(result.end(), stackObjects.begin(), stackObjects.end());
            }
        }
        return result;
    }

    ThreadOop * Thread::getThreadMirror() const {
        return vmThread;
    }

    Executor::Executor(VM &vm) : vm(vm) {
    }

    void Executor::createFrameAndRunMethod(Thread &thread, Method &method_, std::vector<Slot> params, Frame *previous) {
        Frame nextFrame(thread.vm, thread, thread.executor, method_, previous);
        const auto slotSize = method_.paramSlotSize;
        if (slotSize != params.size()) {
            panic("error params length " + method_.name);
        }
        for (auto i = 0; i < params.size(); ++i) {
            const auto slotType = method_.getParamSlotType(i);
            nextFrame.setLocal(i, params.at(i), slotType);
        }
        const auto backupFrame = thread.currentFrame;
        thread.currentFrame = &nextFrame;
        const auto methodName = method_.klass.name + "#" + method_.name;
        executeFrame(nextFrame, methodName);
        thread.currentFrame = backupFrame;
    }

    void Executor::runStaticMethodOnNewThread(Method &method_, std::vector<Slot> params) {
        auto thread = std::make_unique<Thread>(vm, *this);
        createFrameAndRunMethod(*thread, method_, params, nullptr);
    }

    void passReturnValue(Frame &frame) {
        const auto previous = frame.previous;
        if (previous == nullptr) {
            panic("previous is nullptr");
        }
        const auto returnValue = frame.returnValue;
        switch (frame.returnType) {
            case SlotTypeEnum::I4:
            case SlotTypeEnum::F4:
            case SlotTypeEnum::REF:
                previous->push(returnValue);
                break;
                            
            case SlotTypeEnum::I8:
                previous->pushI8(returnValue.i8Val);
                break;

            case SlotTypeEnum::F8:
                previous->pushF8(returnValue.f8Val);
                break;

            default:
                panic("unknown slot Type");
                break;
        }
    }

    //return mark current frame return(throw to previous frame)
    bool handleThrowValue(Frame &frame) {
        const auto throwInstance = static_cast<const InstanceOop *>(frame.throwValue);
        auto &method = frame.method;
        const auto handler =
            method.findExceptionHandler(
                dynamic_cast<const InstanceClass *>(throwInstance->klass),
                frame.pc()
            );

        if (handler) {
            //catch, push the exception to stack(must)
            //TODO frame.cleanStack();
            frame.cleanOperandStack();
            frame.pushRef(frame.throwValue);

            const auto gotoOffset = *handler;
            frame.reader.gotoOffset(gotoOffset);
            frame.markThrow = false;
            frame.throwPc = 0;
            frame.throwValue = nullptr;

            //println("{}method {}#{} handle exception", cstring(frame.level * 2, ' '), frame.klass->name, frame.method.name);
        } else {
            const auto previousFrame = frame.previous;
            if (previousFrame == nullptr) {
                const auto message = static_cast<Oop *>(throwInstance->getFieldValue("detailMessage", "Ljava/lang/String;").refVal);
                if (message != nullptr) {
                    const auto messageStr = getStringNativeValue(message);
                    println("exception message {}", messageStr);
                }
                
                println("throw exception top frame");
                std::exit(0);
            } else {
                previousFrame->throwException(frame.throwValue);
                return true;
            }
        }

        return false;
    }

    void Executor::executeFrame(Frame &frame, cstring methodName) {
        auto &method = frame.method;
        const auto nativeMethod = method.isNative();

        if (method.name == "loadLibrary" && frame.klass.name == "java/lang/System") {
            return;
        }
        //println("{}{}#{}:{} {}", cstring(frame.level * 2, ' '), frame.klass->name, method.name, method.descriptor, nativeMethod ? "[Native]" : "");

        if (!nativeMethod) [[likely]] {
            const auto &byteReader = frame.reader;
            while (!byteReader.eof()) {
                frame.currentByteCode = frame.reader.readU1();
                const auto opCode __attribute__((unused)) = static_cast<OpCodeEnum>(frame.currentByteCode);
                OpCodeHandlers[frame.currentByteCode](frame);
                if (frame.markReturn) {
                    if (frame.existReturnValue) {
                        passReturnValue(frame);
                    }
                    return;
                }
                if (frame.markThrow) {
                    if (handleThrowValue(frame)) {
                        return;
                    }
                }
                frame.reader.resetCurrentOffset();
            }
        } else {
            const auto nativeMethodHandler = method.nativeMethodHandler;
            if (nativeMethodHandler == nullptr) {
                panic("runFrame error, method " + method.klass.name + "#" + method.name + ":" + method.descriptor + " nativeMethodHandler is nullptr");
            }
            nativeMethodHandler(frame);
            if (frame.markReturn) {
                if (frame.existReturnValue) {
                    passReturnValue(frame);
                }
                return;
            }
            if (frame.markThrow) {
                if (handleThrowValue(frame)) {
                    return;
                }
            }
        }

        //TODO pop Frame
    }


}