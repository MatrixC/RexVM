#ifndef NATIVE_SUM_MISC_VM_HPP
#define NATIVE_SUM_MISC_VM_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void initialize(Frame &frame);

    void arrayBaseOffset(Frame &frame);

    void arrayIndexScale(Frame &frame);

    void addressSize(Frame &frame);

    void getCallerClass(Frame &frame);

    void objectFieldOffset(Frame &frame);

    void compareAndSwapObject(Frame &frame);

    void compareAndSwapInt(Frame &frame);

    void compareAndSwapLong(Frame &frame);

    void getObjectVolatile(Frame &frame);

    void putObjectVolatile(Frame &frame);

    void getIntVolatile(Frame &frame);

    void putIntVolatile(Frame &frame);

    void getClassAccessFlags(Frame &frame);

    void newInstance0(Frame &frame);

    void allocateMemory(Frame &frame);

    void putLong(Frame &frame);

    void getByte(Frame &frame);

    void freeMemory(Frame &frame);

    void findSignal(Frame &frame);

    void handle0(Frame &frame);


}

#endif
