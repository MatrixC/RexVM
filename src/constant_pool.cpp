#include "constant_pool.hpp"
#include "class.hpp"
#include "constantInfo.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "utils/string_utils.hpp"
#include "vm.hpp"
#include "memory.hpp"
#include <unicode/unistr.h>

namespace RexVM {

    StringPool::StringPool(VM &vm, ClassLoader &classLoader) : vm(vm), classLoader(classLoader) {
    }

    StringPool::~StringPool() = default;

    InstanceOop *StringPool::getInternString(const cstring &str) {
        //TODO lock
        
        if (const auto iter = internMap.find(str); iter != internMap.end()) {
            return iter->second;
        }

        const auto &oopManager = vm.oopManager;

        const auto utf16Str = icu::UnicodeString::fromUTF8(str);
        const auto utf16BPtr = utf16Str.getBuffer();

        const auto charArrayOop = oopManager->newCharArrayOop(utf16Str.length());
        for (size_t i = 0; i < utf16Str.length(); ++i) {
            charArrayOop->data[i] = utf16BPtr[i];
        }

        auto stringClass = classLoader.getInstanceClass("java/lang/String");
        auto ptr = oopManager->newInstance(stringClass);
        ptr->setFieldValue("value", "[C", Slot(charArrayOop));

        //charArrayOop->setNotGC();
        //ptr->setNotGC();
        internMap.emplace(str, ptr);
        return ptr;
    }


}