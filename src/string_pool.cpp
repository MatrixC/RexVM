#include "string_pool.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "utils/string_utils.hpp"
#include "vm.hpp"
#include "key_slot_id.hpp"
#include "memory.hpp"
#include "thread.hpp"

namespace RexVM {

    bool VMStringHelper::equalJavaString(const InstanceOop *oop, const cchar_16 *rawPtr, const size_t arrayLength) {
        const auto charArray = CAST_CHAR_TYPE_ARRAY_OOP(oop->getFieldValue(stringClassValueFieldSlotId).refVal);
        const auto charArrayPtr = charArray->data.get();
        const auto charArrayLength = charArray->getDataLength();
        return arrayLength == charArrayLength
            && (arrayLength == 0 //empty String
                || std::memcmp(rawPtr, charArrayPtr, sizeof(cchar_16) * charArrayLength) == 0
                );
    }

    InstanceOop *VMStringHelper::createJavaString(VMThread *thread, const ccstr str, const size_t size) {
        const auto utf16Vec = utf8ToUtf16Vec(str, size);
        const auto vmHash = VMStringHelper::getKeyIndex(str, size);
        const auto utf16Ptr = utf16Vec.data();
        const auto utf16Length = utf16Vec.size();
        const auto &vm = thread->vm;

        const auto charArrayOop = vm.oopManager->newCharArrayOop(thread, utf16Length);
        if (utf16Length > 0) [[likely]] {
            std::memcpy(charArrayOop->data.get(), utf16Ptr, sizeof(cchar_16) * utf16Length);
        }

        auto result = vm.oopManager->newStringOop(thread, charArrayOop);
        result->setStringHash(vmHash);
        return result;
    }

    cstring VMStringHelper::getJavaString(const InstanceOop *oop) {
        const auto charArray = CAST_CHAR_TYPE_ARRAY_OOP(oop->getFieldValue(stringClassValueFieldSlotId).refVal);
        if (charArray->getDataLength() == 0) {
            return {};
        }
        const auto char16Ptr = charArray->data.get();
        return utf16ToUtf8(char16Ptr, charArray->getDataLength());
    }

    StringTable::~StringTable() {
        for (auto &head: table) {
            while (head != nullptr) {
                auto temp = head;
                head = head->next;
                delete temp;
            }
        }
    }

    bool StringTable::find(const ccstr str, const size_t size, Value &ret) const {
        //TODO 对于size为0的是不是还可以优化
        const auto index = VMStringHelper::getKeyIndex(str, size);
        auto current = table[index];
        if (current == nullptr) {
            return false;
        }

        const auto utf16Vec = utf8ToUtf16Vec(str, size);
        const auto utf16Ptr = utf16Vec.data();
        const auto utf16Length = utf16Vec.size();
        while (current != nullptr) {
            const auto oopVal = current->value;
            if (VMStringHelper::equalJavaString(oopVal, utf16Ptr, utf16Length)) {
                ret = oopVal;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    void StringTable::insert(const Value value) {
        const auto [hasHash, index] = value->getStringHash();
        const auto newNode = new Node(value);

        if (table[index] == nullptr) {
            table[index] = newNode;
        } else {
            auto current = table[index];
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = newNode;
        }
    }

    void StringTable::erase(Value value) {
        const auto [hasHash, index] = value->getStringHash(); 
        if (!hasHash) {
            return;
        }
        auto current = table[index];
        Node *prev = nullptr;

        while (current != nullptr) {
            if (current->value == value) {
                if (prev == nullptr) {
                    table[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    void StringTable::clear() {
        for (size_t i = 0; i < tableSize; ++i) {
            auto current = table[i];
            while (current != nullptr) {
                const auto next = current->next;
                delete current;
                current = next;
            }
        }
        table.clear();
    }


    StringPool::StringPool(VM &vm, ClassLoader &classLoader)
            : vm(vm), classLoader(classLoader),
            stringTable(std::make_unique<StringTable>(STRING_POOL_SIZE)) {
    }

    StringPool::~StringPool() = default;


    InstanceOop *StringPool::getInternString(VMThread *thread, const ccstr str, const size_t size) {
        if (str == nullptr) {
            panic("point nullptr");
        }

        std::lock_guard guard(lock);
        InstanceOop *result = nullptr;

        if (stringTable->find(str, size, result)) {
            return result;
        }

        result = VMStringHelper::createJavaString(thread, str, size);
        stringTable->insert(result);

        return result;
    }

    InstanceOop *StringPool::getInternString(VMThread *thread, const ccstr str) {
        if (str == nullptr) {
            panic("point nullptr");
        }

        return getInternString(thread, str, strlen(str));
    }

    InstanceOop *StringPool::getInternString(VMThread *thread, const cview str) {
        if (str.data() == nullptr) {
            if (str.empty()) {
                //cview可能是CompositeString 产生的
                //空的CompositeString 指针部分为nullptr 会因为后续函数异常
                return getInternString(thread, "", 0);
            } else {
                panic("point nullptr");
            }
        }
        return getInternString(thread, str.data(), str.size());
    }

    void StringPool::gcStringOop(InstanceOop *oop) {
        std::lock_guard guard(lock);
        stringTable->erase(oop);
    }

    void StringPool::clear() const {
        stringTable->clear();
    }

}