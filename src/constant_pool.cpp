#include "constant_pool.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "utils/string_utils.hpp"
#include "vm.hpp"
#include "key_slot_id.hpp"
#include "memory.hpp"

namespace RexVM {

    StringPool::StringPool(VM &vm, ClassLoader &classLoader)
            : vm(vm), classLoader(classLoader),
              stringClass(classLoader.getInstanceClass(JAVA_LANG_STRING_NAME)) {
    }

    StringPool::~StringPool() = default;

    InstanceOop *StringPool::getInternString(const cstring &str) {
        std::lock_guard<std::mutex> lock(mtx);
        if (const auto iter = internMap.find(str); iter != internMap.end()) {
            return iter->second;
        }

        const auto u16Str = stringToUString(str);
        const auto u16Buffer = u16Str.c_str();
        const auto u16Length = u16Str.length();

        const auto charArrayOop = vm.oopManager->newCharArrayOop(u16Length);
        for (size_t i = 0; i < u16Length; ++i) {
            charArrayOop->data[i] = u16Buffer[i];
        }

        auto stringPtr = vm.oopManager->newInstance(stringClass);
        stringPtr->setFieldValue(stringClassValueFieldSlotId, Slot(charArrayOop));

        internMap.emplace(str, stringPtr);
        return stringPtr;
    }

    cstring StringPool::getJavaString(InstanceOop *oop) {
        const auto charArray = static_cast<CharTypeArrayOop *>(oop->getFieldValue(stringClassValueFieldSlotId).refVal);
        const auto char16Ptr = charArray->data.get();
        return u16charsToString(char16Ptr, charArray->dataLength);
    }


}