#ifndef NATIVE_CORE_DEFINE_CLASS_HPP
#define NATIVE_CORE_DEFINE_CLASS_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include "../../constant_info.hpp"
#include "../../class_loader.hpp"
#include "../../string_pool.hpp"
#include "../../utils/string_utils.hpp"

namespace RexVM::Native::Core {

    void defineClassCommon(Frame &frame, ref bufferOop, i4 off, size_t len, bool useArrayLength, bool notAnonymous) {
        if (bufferOop == nullptr) {
            panic("bufferOop can't be null");
        }

        const auto buffer = CAST_BYTE_TYPE_ARRAY_OOP(bufferOop);
        if (useArrayLength) [[unlikely]] {
            len = buffer->getDataLength();
        }
        const auto bufferPtr = buffer->data.get() + off;

        auto &classLoader = *frame.getCurrentClassLoader();
        const auto classOop = classLoader.loadInstanceClass(bufferPtr, len, notAnonymous);
        frame.returnRef(classOop->getMirrorOop());
    }

    //native Class<?> defineClass0(String name, byte[] b, int off, int len, ProtectionDomain pd);
    void classLoaderDefineClass0(Frame &frame) {
        const auto bufferOop = frame.getLocalRef(2);
        const auto off = frame.getLocalI4(3);
        const auto len = frame.getLocalI4(4);
        defineClassCommon(frame, bufferOop, off, CAST_SIZE_T(len), false, true);
    }

    //native Class<?> defineClass2(String name, java.nio.ByteBuffer b, int off, int len, ProtectionDomain pd, String source);
    void classLoaderDefineClass2(Frame &frame) {
        const auto byteBufferOop = CAST_INSTANCE_OOP(frame.getLocalRef(2));
        if (byteBufferOop == nullptr) {
            panic("byteBufferOop can't be null");
        }
        const auto bufferOop = byteBufferOop->getFieldValue("hb", "[B").refVal;
        const auto off = frame.getLocalI4(3);
        const auto len = frame.getLocalI4(4);
        defineClassCommon(frame, bufferOop, off, CAST_SIZE_T(len), false, true);
    }

    //static native Class<?> defineClass0(ClassLoader loader, String name, byte[] b, int off, int len);
    void proxyDefineClass0(Frame &frame) {
        const auto bufferOop = frame.getLocalRef(2);
        const auto off = frame.getLocalI4(3);
        const auto len = frame.getLocalI4(4);
        defineClassCommon(frame, bufferOop, off, CAST_SIZE_T(len), false, true);
    }

    //native Class<?> defineClass(String name, byte[] b, int off, int len, ClassLoader loader, ProtectionDomain protectionDomain);
    void unsafeDefineClass(Frame &frame) {
        const auto bufferOop = frame.getLocalRef(2);
        const auto off = frame.getLocalI4(3);
        const auto len = frame.getLocalI4(4);
        defineClassCommon(frame, bufferOop, off, CAST_SIZE_T(len), false, true);

    }

    //native Class<?> defineAnonymousClass(Class<?> hostClass, byte[] data, Object[] cpPatches);
    void unsafeDefineAnonymousClass(Frame &frame) {
        //const auto hostClassMirrorOop = CAST_INSTANCE_OOP(frame.getLocalRef(1));
        const auto bufferOop = frame.getLocalRef(2);
        const auto cpPatchesOop = frame.getLocalRef(3);
        (void)cpPatchesOop;
        defineClassCommon(frame, bufferOop, 0, 0, true, false);
    }



}


#endif
