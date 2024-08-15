#include "frame_memory_handler.hpp"
#include "basic_type.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "oop.hpp"
#include "thread.hpp"
#include "memory.hpp"
#include "vm.hpp"
#include "string_pool.hpp"
#include "class_loader.hpp"

namespace RexVM {

    FrameMemoryHandler::FrameMemoryHandler(const Frame &frame) :
        vmThread(frame.thread), oopManager(*frame.vm.oopManager), stringPool(*frame.vm.stringPool), classLoader(*frame.getCurrentClassLoader()) {
    }

    InstanceOop *FrameMemoryHandler::newInstance(InstanceClass * klass) {
        return oopManager.newInstance(&vmThread, klass);
    }

    MirOop *FrameMemoryHandler::newMirror(InstanceClass * klass, voidPtr mirror, MirrorObjectTypeEnum type) {
        return oopManager.newMirror(&vmThread, klass, mirror, type);
    }

    ObjArrayOop *FrameMemoryHandler::newObjArrayOop(ObjArrayClass * klass, size_t length) {
        return oopManager.newObjArrayOop(&vmThread, klass, length);
    }

    ObjArrayOop *FrameMemoryHandler::newObjectObjArrayOop(size_t length) {
        return oopManager.newObjectObjArrayOop(&vmThread, length);
    }

    ObjArrayOop *FrameMemoryHandler::newClassObjArrayOop(size_t length) {
        return oopManager.newClassObjArrayOop(&vmThread, length);
    }

    ObjArrayOop *FrameMemoryHandler::newStringObjArrayOop(size_t length) {
        return oopManager.newStringObjArrayOop(&vmThread, length);
    }

    TypeArrayOop *FrameMemoryHandler::newTypeArrayOop(BasicType type, size_t length) {
        return oopManager.newTypeArrayOop(&vmThread, type, length);
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(size_t length) {
        return oopManager.newByteArrayOop(&vmThread, length);
    }

    ByteTypeArrayOop *FrameMemoryHandler::newByteArrayOop(size_t length, const u1 *initBuffer) {
        return oopManager.newByteArrayOop(&vmThread, length, initBuffer);
    }

    CharTypeArrayOop *FrameMemoryHandler::newCharArrayOop(size_t length) {
        return oopManager.newCharArrayOop(&vmThread, length);
    }

    InstanceOop *FrameMemoryHandler::newBooleanOop(i4 value) {
        return oopManager.newBooleanOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newByteOop(i4 value) {
        return oopManager.newByteOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newCharOop(i4 value) {
        return oopManager.newCharOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newShortOop(i4 value) {
        return oopManager.newShortOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newIntegerOop(i4 value) {
        return oopManager.newIntegerOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newFloatOop(f4 value) {
        return oopManager.newFloatOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newLongOop(i8 value) {
        return oopManager.newLongOop(&vmThread, value);
    }

    InstanceOop *FrameMemoryHandler::newDoubleOop(f8 value) {
        return oopManager.newDoubleOop(&vmThread, value);
    }

    VMThread *FrameMemoryHandler::newVMThread(InstanceClass * klass) {
        return oopManager.newVMThread(&vmThread, klass);
    }

    InstanceOop *FrameMemoryHandler::getInternString(const cstring &str) {
        return stringPool.getInternString(&vmThread, str);
    }

    Class *FrameMemoryHandler::getClass(const cstring &name) {
        return classLoader.getClass(name);
    }

    InstanceClass *FrameMemoryHandler::getInstanceClass(const cstring &name) {
        return classLoader.getInstanceClass(name);
    }

    ArrayClass *FrameMemoryHandler::getArrayClass(const cstring &name) {
        return classLoader.getArrayClass(name);
    }

    TypeArrayClass *FrameMemoryHandler::getTypeArrayClass(BasicType type) {
        return classLoader.getTypeArrayClass(type);
    }

    ObjArrayClass *FrameMemoryHandler::getObjectArrayClass(const cstring &name) {
        return classLoader.getObjectArrayClass(name);
    }

    InstanceClass *FrameMemoryHandler::loadInstanceClass(u1 *ptr, size_t length, bool notAnonymous) {
        return classLoader.loadInstanceClass(ptr, length, notAnonymous);
    }

    InstanceClass *FrameMemoryHandler::getBasicJavaClass(BasicJavaClassEnum classEnum) const {
        return classLoader.getBasicJavaClass(classEnum);
    }


}