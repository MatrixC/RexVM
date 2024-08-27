#ifndef CLASS_ANNOTATION_CONTAINER_HPP
#define CLASS_ANNOTATION_CONTAINER_HPP
#include "config.hpp"
#include <memory>
#include "composite_ptr.hpp"
#include "attribute_info.hpp"

namespace RexVM {

    struct ByteTypeArrayOop;
    struct Frame;

    struct AnnotationContainer {
        Composite<u1 *, u2> composite;

        explicit AnnotationContainer(ByteStreamAttribute *stream) {
            if (stream != nullptr) {
                composite.reset(stream->bytes.release(), CAST_U2(stream->attributeLength));
            }
        }

        ~AnnotationContainer() {
            delete[] composite.getPtr();
        }

        ByteTypeArrayOop *createByteTypeArrayOop(Frame &frame) const;
    };

    struct BasicAnnotationContainer {
        explicit BasicAnnotationContainer() = delete;
        explicit BasicAnnotationContainer(
            ByteStreamAttribute *annotation,
            ByteStreamAttribute *typeAnnotation
        ) : runtimeVisibleAnnotation(annotation), runtimeVisibleTypeAnnotation(typeAnnotation) {

        }

        AnnotationContainer runtimeVisibleAnnotation;
        AnnotationContainer runtimeVisibleTypeAnnotation;

    };

    struct MethodAnnotationContainer {
        explicit MethodAnnotationContainer() = delete;
        explicit MethodAnnotationContainer(
            ByteStreamAttribute *parameterAnnotation,
            ByteStreamAttribute *defaultAnnotation
        ) : runtimeVisibleParameterAnnotation(parameterAnnotation), annotationDefault(defaultAnnotation) {
            
        }

        AnnotationContainer runtimeVisibleParameterAnnotation;
        AnnotationContainer annotationDefault;

    };

    struct ClassAttributeContainer {

    };

}

#endif