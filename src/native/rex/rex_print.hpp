#ifndef REX_PRINT_HPP
#define REX_PRINT_HPP
#include <string>
#include <array>
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../class_member.hpp"
#include "../../memory.hpp"
#include "../../string_pool.hpp"
#include "../../utils/string_utils.hpp"
#include "../../exception_helper.hpp"


namespace RexVM::Native::Rex {

    void print_boolean(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI4(1);
        cprint("{}", val == 0 ? "false" : "true");
    }

    void print_char(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI4(1);
        cprint("{}", CAST_CHAR(val));
    }

    void print_int(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI4(1);
        cprint("{}", val);
    }

    void print_long(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI8(1);
        cprint("{}", val);
    }

    void print_float(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalF4(1);
        //展示float和double 在用科学计数法表示时E的最后会有个加号 跟openjdk不一致
        //暂时没有找到去掉的办法
        cprint("{:E}", val);
    }

    void print_double(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalF8(1);
        cprint("{:E}", val);
    }

    void print_char_arr(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalRef(1);
        ASSERT_IF_NULL_THROW_NPE(val)
        const auto arrObj = CAST_CHAR_TYPE_ARRAY_OOP(val);
        const auto charArr = arrObj->data.get();
        cprint("{}", utf16ToUtf8(charArr, arrObj->getDataLength()));
    }

    void print_String(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalRef(1);
        if (val == nullptr) {
            cprint("null");
            return;
        }
        cprint("{}", VMStringHelper::getJavaString(CAST_INSTANCE_OOP(val)));
    }

    void print_Object(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalRef(1);
        if (val == nullptr) {
            cprint("null");
            return;
        }
        const auto instance = CAST_INSTANCE_OOP(val);
        const auto toStringMethod = 
            instance->getInstanceClass()
                    ->getMethod("toString" "()Ljava/lang/String;", false);

        const auto [retVal, retSlotType] = frame.runMethodManual(*toStringMethod, { Slot(instance) });
        if (frame.markThrow) {
            return;
        }
        const auto stringOop = retVal.refVal;
        if (stringOop == nullptr) {
            cprint("null");
            return;
        }
        cprint("{}", VMStringHelper::getJavaString(CAST_INSTANCE_OOP(stringOop)));
    }

    void println_boolean(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI4(1);
        cprintln("{}", val == 0 ? "false" : "true");
    }

    void println_char(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI4(1);
        cprintln("{}", CAST_CHAR(val));
    }

    void println_int(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI4(1);
        cprintln("{}", val);
    }

    void println_long(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalI8(1);
        cprintln("{}", val);
    }

    void println_float(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalF4(1);
        cprintln("{:E}", val);
    }

    void println_double(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalF8(1);
        cprintln("{:E}", val);
    }

    void println_char_arr(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalRef(1);
        ASSERT_IF_NULL_THROW_NPE(val)
        const auto arrObj = CAST_CHAR_TYPE_ARRAY_OOP(val);
        const auto charArr = arrObj->data.get();
        cprintln("{}", utf16ToUtf8(charArr, arrObj->getDataLength()));
    }

    void println_String(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalRef(1);
        if (val == nullptr) {
            cprintln("null");
            return;
        }
        cprintln("{}", VMStringHelper::getJavaString(CAST_INSTANCE_OOP(val)));
    }

    void println_Object(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        const auto val = frame.getLocalRef(1);
        if (val == nullptr) {
            cprintln("null");
            return;
        }
        const auto instance = CAST_INSTANCE_OOP(val);
        const auto klass = instance->getClass();

        Method *toStringMethod{nullptr};
        if (klass->isArray()) {
            const auto objectClass = frame.mem.getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_OBJECT);
            toStringMethod =
                    objectClass->getMethod("toString" "()Ljava/lang/String;", false);
        } else {
            toStringMethod =
                    instance->getInstanceClass()
                    ->getMethod("toString" "()Ljava/lang/String;", false);
        }

        const auto [retVal, retSlotType] = frame.runMethodManual(*toStringMethod, { Slot(instance) });
        if (frame.markThrow) {
            return;
        }
        const auto stringOop = retVal.refVal;
        if (stringOop == nullptr) {
            cprintln("null");
            return;
        }
        cprintln("{}", VMStringHelper::getJavaString(CAST_INSTANCE_OOP(stringOop)));
    }

    void println_empty(Frame &frame) {
        ASSERT_IF_NULL_THROW_NPE(frame.getThis())
        cprintln("");
    }


}

#endif