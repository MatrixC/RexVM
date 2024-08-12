#ifndef STRING_POOL_HPP
#define STRING_POOL_HPP

#include <unordered_map>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>
#include "config.hpp"

namespace RexVM {

    struct VM;
    struct InstanceClass;
    struct InstanceOop;
    struct ClassLoader;
    struct ConstantUTF8Info;
    struct Frame;
    struct VMThread;

    struct StringTable {
        using Key = const cstring &;
        using Value = InstanceOop *;
        using HashFunction = std::hash<cstring>;

        struct Node {
            InstanceOop *value;
            Node *next{nullptr};

            explicit Node(InstanceOop *value);
        };

        size_t tableSize;
        std::vector<Node *> table;
        
        explicit StringTable(size_t size);
        ~StringTable();

        bool find(Key key, Value &ret);
        void insert(Key key, Value value);
        void erase(Value value);

    private:
        [[nodiscard]] size_t getKeyIndex(Key key) const;

    };

    struct StringPool {
        VM &vm;
        ClassLoader &classLoader;
        std::unique_ptr<StringTable> stringTable;
        std::unordered_map<cstring, InstanceOop *> internMap;
        std::mutex mtx;

        [[nodiscard]] static cstring getJavaString(InstanceOop *oop) ;
        [[nodiscard]] static bool equalJavaString(InstanceOop *oop, const cchar_16 *rawPtr, size_t length);
        void eraseString(InstanceOop *oop);

        InstanceOop *getInternString(const cstring &str);
        InstanceOop *getInternString(VMThread *thread, const cstring &str);
        //InstanceOop *getInternStringOld(const cstring &str);
        void gcStringOop(InstanceOop *oop);

        explicit StringPool(VM &vm, ClassLoader &classLoader);

        ~StringPool();

    private:
        InstanceClass *stringClass;
        [[nodiscard]] InstanceOop *createJavaString(VMThread *thread, const cstring &str) const;
    };


}

#endif
