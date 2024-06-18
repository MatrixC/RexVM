#include "constant_pool.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "utils/string_utils.hpp"
#include "vm.hpp"
#include "memory.hpp"

namespace RexVM {

    StringPool::StringPool(VM &vm, ClassLoader &classLoader) : vm(vm), classLoader(classLoader) {
    }

    StringPool::~StringPool() = default;

    InstanceOop *StringPool::getInternString(const cstring &str) {
        //TODO lock
        
        if (const auto iter = internMap.find(str); iter != internMap.end()) {
            return iter->second;
        }

        const auto u16Str = stringToUString(str);
        const auto u16Buffer = u16Str.c_str();
        const auto u16Length = u16Str.length();

        const auto &oopManager = vm.oopManager;

        const auto charArrayOop = oopManager->newCharArrayOop(u16Length);
        for (size_t i = 0; i < u16Length; ++i) {
            charArrayOop->data[i] = u16Buffer[i];
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