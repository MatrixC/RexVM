#ifndef NATIVE_CORE_JAVA_LANG_THROWABLE_HPP
#define NATIVE_CORE_JAVA_LANG_THROWABLE_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    InstanceOop *createStackTraceElement(
        Frame &frame, 
        const cstring &declaringClass, 
        const cstring &methodName,
        const cstring &fileName,
        i4 lineNumber
    ) {
        const auto classLoader = frame.getCurrentClassLoader();
        const auto &stringPool = frame.vm.stringPool;
        const auto stackTraceElementClass = classLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_STACK_TRACE_ELEMENT);
        const auto stackTraceElementOop = frame.vm.oopManager->newInstance(stackTraceElementClass);
        stackTraceElementOop->setFieldValue("declaringClass", "Ljava/lang/String;", Slot(stringPool->getInternString(declaringClass)));
        stackTraceElementOop->setFieldValue("methodName", "Ljava/lang/String;", Slot(stringPool->getInternString(methodName)));
        stackTraceElementOop->setFieldValue("fileName", "Ljava/lang/String;", Slot(stringPool->getInternString(fileName)));
        stackTraceElementOop->setFieldValue("lineNumber", "I", Slot(lineNumber));
        return stackTraceElementOop;
    }

    void fillInStackTrace(Frame &frame) {
        const auto self = frame.getThisInstance();
        const auto classLoader = frame.getCurrentClassLoader();
        const auto throwableClass = classLoader->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_THROWABLE);
        const auto stackTraceElementArrayClass = classLoader->getObjectArrayClass(JAVA_LANG_STACK_TRACE_ELEMENT_NAME);
        std::vector<InstanceOop *> stackTraceElements;
        auto notCheck = false; //用于少进行一些 isSubClassOf 检测 提升性能 跳过Exception的栈后就不用再check了
        //frame is native fillInStackTrace
        //frame->previous is public synchronized Throwable fillInStackTrace()
        //so skip them
        for (Frame *currentFrame = (&frame)->previous->previous; currentFrame != nullptr; currentFrame = currentFrame->previous) {
            const auto &method = currentFrame->method;
            const auto &klass = method.klass;
            if (!notCheck && (&klass == throwableClass || klass.isSubClassOf(throwableClass))) {
                //跳过Exception体系中的栈 减少干扰项
                continue;
            }
            notCheck = true;
            const auto className = klass.name;
            const auto methodName = method.name;
            const auto sourceFileName = method.klass.sourceFile;
            const auto lineNumber = method.getLineNumber(currentFrame->pc());
            stackTraceElements.emplace_back(createStackTraceElement(frame, className, methodName, sourceFileName, CAST_I4(lineNumber)));
        }

        const auto arrayOop = frame.vm.oopManager->newObjArrayOop(stackTraceElementArrayClass, stackTraceElements.size());
        std::copy(stackTraceElements.begin(), stackTraceElements.end(), arrayOop->data.get());
        self->setFieldValue("stackTrace", "[Ljava/lang/StackTraceElement;", Slot(nullptr));
        self->setFieldValue("backtrace", "Ljava/lang/Object;", Slot(arrayOop));
        frame.returnRef(self);
    }

    //native StackTraceElement getStackTraceElement(int index);
    void getStackTraceElement(Frame &frame) {
        const auto self = frame.getThisInstance();
        const auto index = frame.getLocalI4(1);
        const auto stackTraceElements = CAST_OBJ_ARRAY_OOP(self->getFieldValue("backtrace", "Ljava/lang/Object;").refVal);
        if (stackTraceElements == nullptr) {
            frame.returnRef(nullptr);
            return;
        }
        frame.returnRef(stackTraceElements->data[index]);
    }

    //native int getStackTraceDepth();
    void getStackTraceDepth(Frame &frame) {
        const auto self = frame.getThisInstance();
        const auto stackTraceElements = CAST_OBJ_ARRAY_OOP(self->getFieldValue("backtrace", "Ljava/lang/Object;").refVal);
        if (stackTraceElements == nullptr) {
            frame.returnI4(0);
            return;
        }
        frame.returnI4(CAST_I4(stackTraceElements->dataLength));
    }
}

#endif