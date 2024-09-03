#ifndef CLASS_ATTRIBUTE_CONTAINER_HPP
#define CLASS_ATTRIBUTE_CONTAINER_HPP
#include "config.hpp"
#include <memory>
#include "composite_string.hpp"
#include "attribute_info.hpp"
#include "basic_type.hpp"

namespace RexVM {

    struct ByteTypeArrayOop;
    struct Frame;

    struct AnnotationContainer {
        CompositeArray<u1> composite;

        explicit AnnotationContainer(ByteStreamAttribute *stream) {
            if (stream != nullptr) {
                composite.data.reset(stream->bytes.release(), CAST_U2(stream->attributeLength));
            }
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

    struct NameDescriptorIdentifier {

        rstring id;
        cview nameView;
        cview descriptorView;

        cview getName() const {
            return nameView;
        }

        cview getDescritpor() const {
            return descriptorView;
        }

        explicit NameDescriptorIdentifier(cview name, cview descriptor) :
            id(rstring(name.data(), name.size(), descriptor.data(), descriptor.size())),
            nameView(cview(id.c_str(), name.size())),
            descriptorView(cview(id.c_str() + name.size(), descriptor.size())) {
            //Method
        }

        explicit NameDescriptorIdentifier(cview name, ClassTypeEnum classTypeEnum) {
            //Class
            setName(name, classTypeEnum);
        }

        void setName(cview name, ClassTypeEnum classTypeEnum) {
            //Class
            if (classTypeEnum == ClassTypeEnum::PRIMITIVE_CLASS) {
                //Primitive
                const auto primitiveDesc = getDescriptorByPrimitiveClassName(name);
                id = rstring(name.data(), name.size(), &primitiveDesc, 1);
                nameView = cview(id.c_str(), name.size());
                descriptorView = cview(id.c_str() + name.size(), 1);
            } else if (classTypeEnum == ClassTypeEnum::INSTANCE_CLASS) {
                //InstanceClass
                cstring descriptor{};
                const auto descriptorSize = name.size() + 2;
                descriptor.reserve(descriptorSize);
                descriptor += 'L';
                descriptor.append(name);
                descriptor += ';';
                id = rstring(descriptor);
                nameView = cview(id.c_str() + 1, name.size());
                descriptorView = cview(id.c_str(), descriptorSize);
            } else {
                //Array
                id = rstring(name.data(), name.size());
                nameView = cview(id.c_str(), name.size());
                descriptorView = nameView;
            }
        }

        static bool compare(NameDescriptorIdentifier *id1, NameDescriptorIdentifier *id2) {
            return (id1->id) < (id2->id);
        }
    };

}

#endif