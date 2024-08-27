#include "class_annotation_container.hpp"
#include "frame.hpp"
#include "frame_memory_handler.hpp"
#include "oop.hpp"

namespace RexVM {

    ByteTypeArrayOop *AnnotationContainer::createByteTypeArrayOop(Frame &frame) const {
        if (composite.getPtr() != nullptr) {
            return frame.mem.newByteArrayOop(
                CAST_SIZE_T(composite.getData()), 
                composite.getPtr()
            );
        }
        return nullptr;
    }

    
    
}