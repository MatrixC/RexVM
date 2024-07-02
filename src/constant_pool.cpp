#include "constant_pool.hpp"
#include "class_loader.hpp"
#include "oop.hpp"
#include "utils/string_utils.hpp"
#include "utils/format.hpp"
#include "vm.hpp"
#include "key_slot_id.hpp"
#include "memory.hpp"

namespace RexVM {

    StringPool::StringPool(VM &vm, ClassLoader &classLoader)
            : vm(vm), classLoader(classLoader),
            stringTable(std::make_unique<StringTable>(128)),
            stringClass(classLoader.getInstanceClass(JAVA_LANG_STRING_NAME)) {
    }

    StringPool::~StringPool() = default;

    cstring StringPool::getJavaString(InstanceOop *oop) {
        const auto charArray = static_cast<CharTypeArrayOop *>(oop->getFieldValue(stringClassValueFieldSlotId).refVal);
        const auto char16Ptr = charArray->data.get();
        return utf16ToUtf8(char16Ptr, charArray->dataLength);
    }

    InstanceOop *StringPool::getInternString(const RexVM::cstring &str) {
        std::lock_guard<std::mutex> lock(mtx);
        InstanceOop *result = nullptr;
        if (stringTable->find(str, result)) {
            return result;
        }
        result = createJavaString(str);
        stringTable->insert(str, result);
        return result;
    }

    InstanceOop *StringPool::getInternStringOld(const cstring &str) {
        std::lock_guard<std::mutex> lock(mtx);
        if (const auto iter = internMap.find(str); iter != internMap.end()) {
            return iter->second;
        }
        auto stringPtr = createJavaString(str);
        internMap.emplace(str, stringPtr);
        return stringPtr;
    }

    void StringPool::gcStringOop(InstanceOop *oop) {
        std::lock_guard<std::mutex> lock(mtx);
        stringTable->erase(oop);
    }

    InstanceOop *StringPool::createJavaString(const cstring &str) const {
        const auto utf16Length = str.length();  //same as utf8
        const auto utf16Ptr = utf8ToUtf16(str.c_str(), utf16Length);

        const auto charArrayOop = vm.oopManager->newCharArrayOop(utf16Length);
        if (utf16Length > 0) [[likely]] {
            std::memcpy(charArrayOop->data.get(), utf16Ptr.get(), sizeof(cchar_16) * utf16Length);
        }
        auto result = vm.oopManager->newInstance(stringClass);
        result->setFieldValue(stringClassValueFieldSlotId, Slot(charArrayOop));
        return result;
    }

    bool StringPool::equalJavaString(InstanceOop *oop, const cchar_16 *rawPtr, size_t arrayLength) {
        const auto charArray = static_cast<CharTypeArrayOop *>(oop->getFieldValue(stringClassValueFieldSlotId).refVal);
        const auto charArrayPtr = charArray->data.get();
        const auto charArrayLength = charArray->dataLength;
        return arrayLength == charArrayLength
            && (arrayLength == 0 //empty String
                or std::memcmp(rawPtr, charArrayPtr, sizeof(cchar_16) * charArrayLength) == 0
                );
    }

    StringTable::StringTable(RexVM::size_t size)
            : tableSize(size), table(size, nullptr) {
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

    StringTable::Node::Node(RexVM::InstanceOop *value) : value(value) {
    }

    size_t StringTable::getKeyIndex(Key key) const {
        const auto hashCode = HashFunction{}(key);
        return hashCode % tableSize;
    }

    bool StringTable::find(Key key, Value &ret) {
        const auto utf16Length = key.length(); //same as utf8
        const auto utf16Ptr = utf8ToUtf16(key.c_str(), utf16Length);

        const auto index = getKeyIndex(key);
        auto current = table[index];
        while (current != nullptr) {
            const auto oopVal = current->value;
            if (StringPool::equalJavaString(oopVal, utf16Ptr.get(), utf16Length)) {
                ret = oopVal;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    void StringTable::insert(Key key, Value value) {
        const auto index = getKeyIndex(key);
        auto newNode = new Node(value);

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
        const auto key = StringPool::getJavaString(value);
        const auto index = getKeyIndex(key);
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
                return;
            }
            prev = current;
            current = current->next;
        }
    }
}