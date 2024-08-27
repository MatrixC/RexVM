#ifndef CLASS_ANNOTATION_CONTAINER_HPP
#define CLASS_ANNOTATION_CONTAINER_HPP
#include "config.hpp"
#include <memory>
#include "composite_ptr.hpp"
#include "attribute_info.hpp"

namespace RexVM {

    struct BasicAnnotationContainer {
        explicit BasicAnnotationContainer() = default;
        explicit BasicAnnotationContainer(
            ByteStreamAttribute *annotation,
            ByteStreamAttribute *typeAnnotation
        ) {
            initAnnotation(annotation);
            initTypeAnnotation(typeAnnotation);
        }

        Composite<u1 *, u2> runtimeVisibleAnnotation;
        Composite<u1 *, u2> runtimeVisibleTypeAnnotation;

        size_t getAnnotationLength() const {
            return CAST_SIZE_T(runtimeVisibleAnnotation.getData());
        }

        u1 *getAnnotationPtr() const {
            return runtimeVisibleAnnotation.getPtr();
        }

        size_t getTypeAnnotationLength() const {
            return CAST_SIZE_T(runtimeVisibleTypeAnnotation.getData());
        }

        u1 *getTypeAnnotationPtr() const {
            return runtimeVisibleTypeAnnotation.getPtr();
        }

        void initAnnotation(ByteStreamAttribute *attributeBytes) {
            if (attributeBytes != nullptr) {
                runtimeVisibleAnnotation.reset(attributeBytes->bytes.release(), CAST_U2(attributeBytes->attributeLength));
            }
        }

        void initTypeAnnotation(ByteStreamAttribute *attributeBytes) {
            if (attributeBytes != nullptr) {
                runtimeVisibleTypeAnnotation.reset(attributeBytes->bytes.release(), CAST_U2(attributeBytes->attributeLength));
            }
        }

        ~BasicAnnotationContainer() {
            delete []runtimeVisibleAnnotation.getPtr();
            delete []runtimeVisibleTypeAnnotation.getPtr();
        }

    };

    struct MethodAnnotationContainer {
        explicit MethodAnnotationContainer() = default;
        explicit MethodAnnotationContainer(
            ByteStreamAttribute *parameterAnnotation,
            ByteStreamAttribute *defaultAnnotation
        ) {
            initParameterAnnotation(parameterAnnotation);
            initAnnotationDefault(defaultAnnotation);
        }

        Composite<u1 *, u2> runtimeVisibleParameterAnnotation;
        Composite<u1 *, u2> annotationDefault;

        size_t getParameterAnnotationLength() const {
            return CAST_SIZE_T(runtimeVisibleParameterAnnotation.getData());
        }

        u1 *getParameterAnnotationPtr() const {
            return runtimeVisibleParameterAnnotation.getPtr();
        }

        size_t getAnnotationDefaultLength() const {
            return CAST_SIZE_T(annotationDefault.getData());
        }

        u1 *getAnnotationDefaultPtr() const {
            return annotationDefault.getPtr();
        }

        void initParameterAnnotation(ByteStreamAttribute *attributeBytes) {
            if (attributeBytes != nullptr) {
                runtimeVisibleParameterAnnotation.reset(attributeBytes->bytes.release(), CAST_U2(attributeBytes->attributeLength));
            }
        }

        void initAnnotationDefault(ByteStreamAttribute *attributeBytes) {
            if (attributeBytes != nullptr) {
                annotationDefault.reset(attributeBytes->bytes.release(), CAST_U2(attributeBytes->attributeLength));
            }
        }

        ~MethodAnnotationContainer() {
            delete []runtimeVisibleParameterAnnotation.getPtr();
            delete []annotationDefault.getPtr();
        }


    };

}

#endif