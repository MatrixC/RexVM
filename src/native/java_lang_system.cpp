#include "java_lang_system.hpp"
#include "../class.hpp"
#include "../class_loader.hpp"
#include "../oop.hpp"
#include "../vm.hpp"
#include "../constant_pool.hpp"
#include <chrono>
#include <algorithm>

namespace RexVM::Native {

    void setIn0(Frame &frame) {
        frame.klass.setFieldValue("in", "Ljava/io/InputStream;", Slot(frame.getLocalRef(0)));
    }

    void setOut0(Frame &frame) {
        frame.klass.setFieldValue("out", "Ljava/io/PrintStream;", Slot(frame.getLocalRef(0)));
    }

    void setErr0(Frame &frame) {
        frame.klass.setFieldValue("err", "Ljava/io/PrintStream;", Slot(frame.getLocalRef(0)));
    }

    void currentTimeMillis(Frame &frame) {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        frame.returnI8(static_cast<i8>(ms));
    }

    void initProperties(Frame &frame) {

        const auto props = static_cast<InstanceOop *>(frame.getLocalRef(0));
        const auto propsClass = dynamic_cast<const InstanceClass *>(props->klass);
        const auto setPropertyMethod = propsClass->getMethod("setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;", false);
        const auto &stringPool = frame.vm.stringPool;

        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("file.encoding")), Slot(stringPool->getInternString("UTF-8")) });
        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("sun.stdout.encoding")), Slot(stringPool->getInternString("UTF-8")) });
        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("sun.stderr.encoding")), Slot(stringPool->getInternString("UTF-8")) });


        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("file.separator")), Slot(stringPool->getInternString("/")) });
        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("path.separator")), Slot(stringPool->getInternString(":")) });
        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("line.separator")), Slot(stringPool->getInternString("\n")) });
        frame.runMethod(*setPropertyMethod, std::vector<Slot>{ Slot(props), Slot(stringPool->getInternString("java.home")), Slot(stringPool->getInternString(".")) });

        frame.returnRef(props);
    }

    //"runFrame error, method java/security/AccessController:doPrivileged nativeMethodHandler is nullptr"

    void doPrivileged(Frame &frame) {
        const auto action = frame.getThisInstance();
        const auto runMethod = action->getInstanceClass()->getMethod("run", "()Ljava/lang/Object;", false);
        const auto result = frame.runMethodGetReturn(*runMethod, { Slot(action) }).refVal;
        if (!frame.markThrow) {
            frame.returnRef(result);
        }
    }

    void arraycopy(Frame &frame) {
        const auto src = static_cast<ArrayOop *>(frame.getLocalRef(0));
        const auto srcPos = frame.getLocalI4(1);
        const auto dest = static_cast<ArrayOop *>(frame.getLocalRef(2));
        const auto destPos = frame.getLocalI4(3);
        const auto length = frame.getLocalI4(4);

        const auto arrayType = src->klass->type;
        if (arrayType == ClassTypeEnum::ObjArrayClass) {
            const auto relSrc = dynamic_cast<ObjArrayOop *>(src);
            const auto relDest = dynamic_cast<ObjArrayOop *>(dest);
            std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
        } else {
            const auto basicTypeArray = dynamic_cast<TypeArrayOop *>(src);
            const auto basicTypeArrayClass = dynamic_cast<const TypeArrayClass *>(basicTypeArray->klass);
            const auto basicType = basicTypeArrayClass->elementType;
            switch (basicType) {
                case BasicType::T_BYTE:
                case BasicType::T_BOOLEAN: {
                    const auto relSrc = dynamic_cast<ByteTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<ByteTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_CHAR: {
                    const auto relSrc = dynamic_cast<CharTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<CharTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_SHORT: {
                    const auto relSrc = dynamic_cast<ShortTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<ShortTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_INT: {
                    const auto relSrc = dynamic_cast<IntTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<IntTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_LONG: {
                    const auto relSrc = dynamic_cast<LongTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<LongTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_FLOAT: {
                    const auto relSrc = dynamic_cast<FloatTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<FloatTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                case BasicType::T_DOUBLE: {
                    const auto relSrc = dynamic_cast<DoubleTypeArrayOop *>(src);
                    const auto relDest = dynamic_cast<DoubleTypeArrayOop *>(dest);
                    std::copy(relSrc->data.get() + srcPos, relSrc->data.get() + srcPos + length, relDest->data.get() + destPos);
                    break;
                }

                default:
                    panic("error basic type");
            }
        }
    }

    void mapLibraryName(Frame &frame) {
        const auto libName = frame.getLocalRef(0);
        frame.returnRef(libName);
    }




}