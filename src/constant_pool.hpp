//
// Created by ch on 1/13/24.
//

#ifndef CONSTANT_POOL_HPP
#define CONSTANT_POOL_HPP

#include <unordered_map>
#include <memory>
#include <mutex>
#include "config.hpp"

namespace RexVM {

    struct VM;
    struct InstanceOop;
    struct ClassLoader;
    struct ConstantUTF8Info;

    struct StringPool {
        VM &vm;
        ClassLoader &classLoader;
        std::unordered_map<cstring, InstanceOop *> internMap;

        std::mutex mtx;

        InstanceOop *getInternString(const cstring &str);

        explicit StringPool(VM &vm, ClassLoader &classLoader);

        ~StringPool();
    };

}

#endif
