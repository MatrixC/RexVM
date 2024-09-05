#include "class_attribute_container.hpp"
#include "frame.hpp"
#include "frame_memory_handler.hpp"
#include "oop.hpp"

namespace RexVM {

    ByteTypeArrayOop *AnnotationContainer::createByteTypeArrayOop(Frame &frame) const {
        if (composite.data.getPtr() != nullptr) {
            return frame.mem.newByteArrayOop(
                CAST_SIZE_T(composite.data.getData()), 
                composite.data.getPtr()
            );
        }
        return nullptr;
    }


    
}