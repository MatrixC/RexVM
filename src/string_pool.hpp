#ifndef STRING_POOL_HPP
#define STRING_POOL_HPP

#include <memory>
#include <mutex>
#include <vector>
#include "basic.hpp"
#include "utils/spin_lock.hpp"
#include "composite_string.hpp"

namespace RexVM {

    struct VM;
    struct InstanceClass;
    struct InstanceOop;
    struct ClassLoader;
    struct ConstantUTF8Info;
    struct Frame;
    struct VMThread;
    struct StringTable;

    struct VMStringHelper {
        static constexpr auto tableSize = STRING_POOL_SIZE;

        static u2 getKeyIndex(const ccstr str, const size_t size) {
            const auto hashCode = std::hash<cview>{}(cview(str, size));
            return CAST_U2(hashCode % tableSize);
        }

        static cstring getJavaString(const InstanceOop *oop);
        static bool equalJavaString(const InstanceOop *oop, const cchar_16 *rawPtr, size_t arrayLength);
        static InstanceOop *createJavaString(VMThread *thread, ccstr str, size_t size);

        template<typename TStr>
        static InstanceOop *createJavaString(VMThread *thread, const TStr &str) {
            return createJavaString(thread, str.c_str(), str.size());
        }
    };

    struct StringTable {
        using Value = InstanceOop *;
        
        struct Node {
            Value value;
            Node *next{nullptr};

            explicit Node(Value value) : value(value) {
            }
        };

        size_t tableSize;
        std::vector<Node *> table;
        
        explicit StringTable(const size_t size) : tableSize(size), table(size, nullptr) {
        }
        ~StringTable();

        bool find(ccstr str, size_t size, Value &ret) const;
        void insert(Value value);
        void erase(Value value);
        void clear();
    };


    struct StringPool {
        VM &vm;
        ClassLoader &classLoader;
        std::unique_ptr<StringTable> stringTable;
        SpinLock lock;

        explicit StringPool(VM &vm, ClassLoader &classLoader);
        ~StringPool();

        InstanceOop *getInternString(VMThread *thread, ccstr str, size_t size);
        InstanceOop *getInternString(VMThread *thread, ccstr str);
        InstanceOop *getInternString(VMThread *thread, cview str);

        void gcStringOop(InstanceOop *oop);
        void clear() const;

    };


}

#endif
