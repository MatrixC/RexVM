#include "execute.hpp"
#include "config.hpp"
#include "utils/format.hpp"
#include "utils/class_utils.hpp"
#include "vm.hpp"
#include "opcode.hpp"
#include "frame.hpp"
#include "memory.hpp"
#include "interpreter.hpp"
#include "thread.hpp"
#include "basic_java_class.hpp"
#include <mutex>
#include <utility>
#include <sstream>

namespace RexVM {

    //return mark current frame return(throw to previous frame)
    bool handleThrowValue(Frame &frame) {
        const auto throwInstance = frame.exception->throwValue;
        const auto throwInstanceClass = static_cast<const InstanceClass *>(throwInstance->klass);
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
                const auto message = static_cast<Oop *>(throwInstance->getFieldValue("detailMessage", "Ljava/lang/String;").refVal);
                cstring messageStr = "";
                if (message != nullptr) {
                    messageStr = ": " + getStringNativeValue(message);
                }

                cstring outMessage = cformat(
                    "Exception in thread \"{}\" {}{}", 
                    frame.thread.name,
                    throwInstanceClass->name,
                    messageStr
                );

                for (const auto &item : frame.exception->throwPath) {
                    //example. at ExceptionTest.p(ExceptionTest.java:13)
                    const auto className = item->method.klass.name;
                    const auto methodName = item->method.name;
                    const auto lineNumber = item->method.getLineNumber(item->throwPc);
                    const auto sourceFileName = item->method.klass.sourceFile;
                    outMessage += cformat("\n\tat {}.{}({}:{})", className, methodName, sourceFileName, lineNumber);
                }
                cprintln("{}", outMessage);
            } else {
                previousFrame->passException(std::move(frame.exception));
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
                panic("checkAndPassReturnValue error: unknown slot Type");
                break;
        }
    }

    void executeFrame(Frame &frame, const cstring& methodName) {
        auto &method = frame.method;
        const auto notNativeMethod = !method.isNative();

        if (method.name == "loadLibrary" && frame.klass.name == JAVA_LANG_SYSTEM_NAME) {
            return;
        }
        //println("{}{}#{}:{} {}", cstring(frame.level * 2, ' '), frame.klass.name, method.name, method.descriptor, nativeMethod ? "[Native]" : "");

        if (notNativeMethod) [[likely]] {
            const auto &byteReader = frame.reader;
            while (!byteReader.eof()) {
                frame.currentByteCode = frame.reader.readU1();
                const auto opCode __attribute__((unused)) = static_cast<OpCodeEnum>(frame.currentByteCode);
                OpCodeHandlers[frame.currentByteCode](frame);
                if (frame.markReturn) {
                    checkAndPassReturnValue(frame);
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
                panic("executeFrame error, method " + method.klass.name + "#" + method.name + ":" + method.descriptor + " nativeMethodHandler is nullptr");
            }
            nativeMethodHandler(frame);
            if (frame.markReturn) {
                checkAndPassReturnValue(frame);
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

    void createFrameAndRunMethod(Thread &thread, Method &method_, std::vector<Slot> params, Frame *previous) {
        Frame nextFrame(thread.vm, thread, method_, previous);
        const auto slotSize = method_.paramSlotSize;
        if (slotSize != params.size()) {
            panic("error params length " + method_.name);
        }
        for (size_t i = 0; i < params.size(); ++i) {
            const auto slotType = method_.getParamSlotType(i);
            nextFrame.setLocal(i, params.at(i), slotType);
        }
        const auto backupFrame = thread.currentFrame;
        thread.currentFrame = &nextFrame;
        const auto methodName = method_.klass.name + "#" + method_.name;
        executeFrame(nextFrame, methodName);
        thread.currentFrame = backupFrame;
    }

    void runStaticMethodOnThread(VM &vm, Method &method, std::vector<Slot> params, const cstring &name) {
        Thread thread(vm, name);
        thread.status = ThreadStatusEnum::Running;
        vm.addVMThread(&thread);
        createFrameAndRunMethod(thread, method, std::move(params), nullptr);
        thread.status = ThreadStatusEnum::Terminated; 
        vm.removeVMThread(&thread);
        
    }

    void runStaticMethodOnNewThread(VM &vm, Method &method, std::vector<Slot> params) {
        std::lock_guard<std::mutex> lock(vm.threadMtx);
        vm.threadDeque.emplace_back([&vm, &method, params = std::move(params)]() {
            std::stringstream ss;
            ss << std::this_thread::get_id();
            const auto threadName = "Thread-" + ss.str();
            runStaticMethodOnThread(vm, method, params, threadName);
        });
    }



}