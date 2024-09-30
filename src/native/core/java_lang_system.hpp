#ifndef NATIVE_CORE_JAVA_LANG_SYSTEM_HPP
#define NATIVE_CORE_JAVA_LANG_SYSTEM_HPP

#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../string_pool.hpp"
#include "../../file_system.hpp"
#include "../../utils/time.hpp"
#include "../../os_platform.hpp"
#include <thread>
#include <chrono>
#include <filesystem>


namespace RexVM::Native::Core {

    void currentTimeMillis(Frame &frame) {
        frame.returnI8(getCurrentTimeMillis());
    }

    //java/lang/System#nanoTime:()J
    void nanoTime(Frame &frame) {
        const auto duration = frame.vm.startTime.time_since_epoch();
        const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        frame.returnI8(nanos);
    }

    //static native int identityHashCode(Object x);
    void identityHashCode(Frame &frame) {
        frame.returnI4(CAST_I4(std::bit_cast<u8>(frame.getLocalRef(0))));
    }

    void arraycopy(Frame &frame) {
        const auto src = CAST_ARRAY_OOP(frame.getLocalRef(0));
        const auto srcPos = frame.getLocalI4(1);
        const auto dest = CAST_ARRAY_OOP(frame.getLocalRef(2));
        const auto destPos = frame.getLocalI4(3);
        const auto length = frame.getLocalI4(4);
        const auto srcEndPos = srcPos + length;

        const auto arrayType = src->getClass()->type;
        const auto destArrayType = dest->getClass()->type;
        if (arrayType != ClassTypeEnum::OBJ_ARRAY_CLASS && arrayType != ClassTypeEnum::TYPE_ARRAY_CLASS) {
            panic("obj is not array type");
        }
        if (arrayType != destArrayType) {
            panic("array type is different");
        }
        if (arrayType == ClassTypeEnum::OBJ_ARRAY_CLASS) {
            const auto relSrc = CAST_OBJ_ARRAY_OOP(src);
            const auto relDest = CAST_OBJ_ARRAY_OOP(dest);
            std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
        } else {
            const auto basicTypeArray = CAST_TYPE_ARRAY_OOP(src);
            const auto basicTypeArrayClass = CAST_TYPE_ARRAY_CLASS(basicTypeArray->getClass());
            const auto basicType = basicTypeArrayClass->elementType;
            if (basicType != CAST_TYPE_ARRAY_CLASS(dest->getClass())->elementType) {
                panic("type array is different");
            }
            switch (basicType) {
                case BasicType::T_BYTE:
                case BasicType::T_BOOLEAN: {
                    const auto relSrc = CAST_BYTE_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_BYTE_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_CHAR: {
                    const auto relSrc = CAST_CHAR_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_CHAR_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_SHORT: {
                    const auto relSrc = CAST_SHORT_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_SHORT_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_INT: {
                    const auto relSrc = CAST_INT_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_INT_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_LONG: {
                    const auto relSrc = CAST_LONG_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_LONG_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_FLOAT: {
                    const auto relSrc = CAST_FLOAT_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_FLOAT_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_DOUBLE: {
                    const auto relSrc = CAST_DOUBLE_TYPE_ARRAY_OOP(src);
                    const auto relDest = CAST_DOUBLE_TYPE_ARRAY_OOP(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcEndPos, relDest->data.get() + destPos);
                    break;
                }

                default:
                    panic("error basic type");
            }
        }
    }

    void setIn0(Frame &frame) {
        frame.klass.setFieldValue("in" "Ljava/io/InputStream;", Slot(frame.getLocalRef(0)));
    }

    void setOut0(Frame &frame) {
        const auto defaultPrintStream = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto rexPrintStreamClass = frame.mem.getInstanceClass("RexPrintStream");
        if (rexPrintStreamClass != nullptr) {
            rexPrintStreamClass->clinit(frame);
            const auto getMethod = rexPrintStreamClass->getMethod("get" "(Ljava/io/PrintStream;)Ljava/io/PrintStream;", true);
            const auto [retVal, retType] = frame.runMethodManual(*getMethod, { Slot(defaultPrintStream) });
            frame.klass.setFieldValue("out" "Ljava/io/PrintStream;", retVal);
        } else {
            frame.klass.setFieldValue("out" "Ljava/io/PrintStream;", Slot(frame.getLocalRef(0)));
        }
    }

    void setErr0(Frame &frame) {
        frame.klass.setFieldValue("err" "Ljava/io/PrintStream;", Slot(frame.getLocalRef(0)));
    }

    void mapLibraryName(Frame &frame) {
        const auto libName = frame.getLocalRef(0);
        frame.returnRef(libName);
    }

    void initProperties(Frame &frame) {
        const auto props = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto propsClass = CAST_INSTANCE_CLASS(props->getClass());
        const auto setPropertyMethod = propsClass->getMethod("setProperty" "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;", false);
        const auto utf8ConstString = frame.mem.getInternString("UTF-8");

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("java.vm.name")), Slot(frame.mem.getInternString("RexVM")) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("java.specification.version")), Slot(frame.mem.getInternString("1.8")) });

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("file.encoding")), Slot(utf8ConstString) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("sun.stdout.encoding")), Slot(utf8ConstString) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("sun.stderr.encoding")), Slot(utf8ConstString) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("sun.jnu.encoding")), Slot(utf8ConstString) });
        
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("file.separator")), Slot(frame.mem.getInternString(cstring{FILE_SEPARATOR})) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("path.separator")), Slot(frame.mem.getInternString(cstring{PATH_SEPARATOR})) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("line.separator")), Slot(frame.mem.getInternString(cstring{LINE_SEPARATOR})) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("java.home")), Slot(frame.mem.getInternString(frame.vm.javaHome)) });

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("java.class.path")), Slot(frame.mem.getInternString(frame.vm.javaClassPath)) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("user.dir")), Slot(frame.mem.getInternString(std::filesystem::current_path().string())) });

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("os.arch")), Slot(frame.mem.getInternString(OS_ARCH_IMPL)) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("os.name")), Slot(frame.mem.getInternString(OS_NAME)) });

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(frame.mem.getInternString("sun.reflect.noCaches")), Slot(frame.mem.getInternString("true")) });

        frame.returnRef(props);
    }

    void getStackAccessControlContext(Frame &frame) {
        frame.returnRef(nullptr);
    }

    void doPrivileged(Frame &frame) {
        const auto action = frame.getThisInstance();
        const auto runMethod = action->getInstanceClass()->getMethod("run" "()Ljava/lang/Object;", false);
        const auto [result , _] = frame.runMethodManual(*runMethod, { Slot(action) });
        if (!frame.markThrow) {
            frame.returnRef(result.refVal);
        }
    }
}

#endif