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
#include <thread>
#include <chrono>


namespace RexVM::Native::Core {

    void currentTimeMillis(Frame &frame) {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        frame.returnI8(CAST_I8(ms));
    }

    //java/lang/System#nanoTime:()J
    void nanoTime(Frame &frame) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto duration = now.time_since_epoch();
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

        const auto arrayType = src->klass->type;
        const auto destArrayType = dest->klass->type;
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
            const auto basicTypeArrayClass = CAST_TYPE_ARRAY_CLASS(basicTypeArray->klass);
            const auto basicType = basicTypeArrayClass->elementType;
            if (basicType != CAST_TYPE_ARRAY_CLASS(dest->klass)->elementType) {
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
        frame.klass.setFieldValue("in", "Ljava/io/InputStream;", Slot(frame.getLocalRef(0)));
    }

    void setOut0(Frame &frame) {
        frame.klass.setFieldValue("out", "Ljava/io/PrintStream;", Slot(frame.getLocalRef(0)));
    }

    void setErr0(Frame &frame) {
        frame.klass.setFieldValue("err", "Ljava/io/PrintStream;", Slot(frame.getLocalRef(0)));
    }

    void mapLibraryName(Frame &frame) {
        const auto libName = frame.getLocalRef(0);
        frame.returnRef(libName);
    }

    void initProperties(Frame &frame) {
        const auto props = CAST_INSTANCE_OOP(frame.getLocalRef(0));
        const auto propsClass = dynamic_cast<const InstanceClass *>(props->klass);
        const auto setPropertyMethod = propsClass->getMethod("setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;", false);
        const auto &stringPool = frame.vm.stringPool;
        const auto utf8ConstString = stringPool->getInternString("UTF-8");

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("java.vm.name")), Slot(stringPool->getInternString("RexVM")) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("java.specification.version")), Slot(stringPool->getInternString("1.8")) });

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("file.encoding")), Slot(utf8ConstString) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("sun.stdout.encoding")), Slot(utf8ConstString) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("sun.stderr.encoding")), Slot(utf8ConstString) });

        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("file.separator")), Slot(stringPool->getInternString(cstring{FILE_SEPARATOR})) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("path.separator")), Slot(stringPool->getInternString(cstring{PATH_SEPARATOR})) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("line.separator")), Slot(stringPool->getInternString(cstring{LINE_SEPARATOR})) });
        frame.runMethodManual(*setPropertyMethod, { Slot(props), Slot(stringPool->getInternString("java.home")), Slot(stringPool->getInternString(frame.vm.javaHome)) });

        frame.returnRef(props);
    }

    void getStackAccessControlContext(Frame &frame) {
        //TODO
        frame.returnRef(nullptr);
    }

    void doPrivileged(Frame &frame) {
        const auto action = frame.getThisInstance();
        const auto runMethod = action->getInstanceClass()->getMethod("run", "()Ljava/lang/Object;", false);
        const auto [result , _] = frame.runMethodManual(*runMethod, { Slot(action) });
        if (!frame.markThrow) {
            frame.returnRef(result.refVal);
        }
    }
}

#endif