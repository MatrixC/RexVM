#ifndef ENUMS_HPP
#define ENUMS_HPP

namespace RexVM {

    enum class BasicType : u1 {
        T_BOOLEAN = 4,
        T_CHAR = 5,
        T_FLOAT = 6,
        T_DOUBLE = 7,
        T_BYTE = 8,
        T_SHORT = 9,
        T_INT = 10,
        T_LONG = 11,
        T_OBJECT = 14,
        T_ARRAY = 13,
        T_VOID = 12,
        T_ADDRESS = 16,
        T_NARROWOOP = 251,
        T_METADATA = 252,
        T_NARROWKLASS = 253,
        T_CONFLICT = 254,
        T_ILLEGAL = 255,
    };

    enum class AccessFlagEnum : u2 {
        ACC_PUBLIC = 0x0001, // class, field, method
        ACC_PRIVATE = 0x0002, // class, field, method
        ACC_PROTECTED = 0x0004, // class, field, method
        ACC_STATIC = 0x0008, // field, method
        ACC_FINAL = 0x0010, // class, field, method, parameter
        ACC_SUPER = 0x0020, // class
        ACC_SYNCHRONIZED = 0x0020, // method
        ACC_OPEN = 0x0020, // module
        ACC_TRANSITIVE = 0x0020, // module requires
        ACC_VOLATILE = 0x0040, // field
        ACC_BRIDGE = 0x0040, // method
        ACC_STATIC_PHASE = 0x0040, // module requires
        ACC_VARARGS = 0x0080, // method
        ACC_TRANSIENT = 0x0080, // field
        ACC_NATIVE = 0x0100, // method
        ACC_INTERFACE = 0x0200, // class
        ACC_ABSTRACT = 0x0400, // class, method
        ACC_STRICT = 0x0800, // method
        ACC_SYNTHETIC = 0x1000, // class, field, method, parameter, module *
        ACC_ANNOTATION = 0x2000, // class
        ACC_ENUM = 0x4000, // class(?) field inner
        ACC_MANDATED = 0x8000, // parameter, module, module *
        ACC_MODULE = 0x8000, // class
    };

    enum class ClassInitStatusEnum : u1 {
        LOADED,
        INIT,
        INITED,
    };

    enum class ClassTypeEnum : u1 {
        PRIMITIVE_CLASS,
        INSTANCE_CLASS,
        TYPE_ARRAY_CLASS,
        OBJ_ARRAY_CLASS,
    };

    //标记在InstanceClass上, 用于在new指令上提升效率
    enum class SpecialClassEnum : u1 {
        NONE,
        THREAD_CLASS,
        CLASS_LOADER_CLASS,
        MEMBER_NAME_CLASS,
    };

    enum class ClassMemberTypeEnum {
        FIELD,
        METHOD,
    };

    enum class OopTypeEnum : u1 {
        INSTANCE_OOP = 0,
        TYPE_ARRAY_OOP = 1,
        OBJ_ARRAY_OOP = 2,
    };

    enum class MirrorObjectTypeEnum : u2 {
        CLASS = 0,
        METHOD = 1,
        CONSTRUCTOR = 2,
        FIELD = 3,
        MEMBER_NAME = 4,
        CONSTANT_POOL = 5,
    };

    enum class ThreadStatusEnum : u2 {
        NEW = 0x0000,
        RUNNABLE = 0x0004,
        BLOCKED = 0x0400,
        WAITING = 0x0010,
        TIMED_WAITING = 0x0020,
        TERMINATED = 0x0002,
    };

    enum class NameDescriptorIdentifierType : u1 {
        CLASS,
        CLASS_MEMBER,
    };

}

#endif