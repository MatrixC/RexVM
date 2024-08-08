#ifndef BASIC_MACRO_HPP
#define BASIC_MACRO_HPP

#define CAST_U1(x) static_cast<u1>(x)
#define CAST_U2(x) static_cast<u2>(x)
#define CAST_U4(x) static_cast<u4>(x)
#define CAST_U8(x) static_cast<u8>(x)
#define CAST_I1(x) static_cast<i1>(x)
#define CAST_I2(x) static_cast<i2>(x)
#define CAST_I4(x) static_cast<i4>(x)
#define CAST_I8(x) static_cast<i8>(x)
#define CAST_F4(x) static_cast<f4>(x)
#define CAST_F8(x) static_cast<f8>(x)
#define CAST_REF(x) static_cast<ref>(x)
#define CAST_U1_PTR(x) reinterpret_cast<u1 *>(x)
#define CAST_U2_PTR(x) reinterpret_cast<u2 *>(x)
#define CAST_U4_PTR(x) reinterpret_cast<u4 *>(x)
#define CAST_U8_PTR(x) reinterpret_cast<u8 *>(x)
#define CAST_I1_PTR(x) reinterpret_cast<i1 *>(x)
#define CAST_I2_PTR(x) reinterpret_cast<i2 *>(x)
#define CAST_I4_PTR(x) reinterpret_cast<i4 *>(x)
#define CAST_I8_PTR(x) reinterpret_cast<i8 *>(x)
#define CAST_F4_PTR(x) reinterpret_cast<f4 *>(x)
#define CAST_F8_PTR(x) reinterpret_cast<f8 *>(x)
#define CAST_REF_PTR(x) reinterpret_cast<ref *>(x)
#define CAST_VOID_PTR(x) reinterpret_cast<voidPtr>(x)
#define CAST_SIZE_T(x) static_cast<size_t>(x)
#define CAST_CHAR(x) static_cast<char>(x)
#define CAST_CHAR_16_T(x) static_cast<char16_t>(x)
#define CAST_CCHAR_16(x) static_cast<cchar_16>(x)

#define CAST_CONSTANT_TAG_ENUM(x) static_cast<ConstantTagEnum>(x)
#define CAST_CONSTANT_INTEGER_INFO(x) static_cast<ConstantIntegerInfo *>(x)
#define CAST_CONSTANT_LONG_INFO(x) static_cast<ConstantLongInfo *>(x)
#define CAST_CONSTANT_FLOAT_INFO(x) static_cast<ConstantFloatInfo *>(x)
#define CAST_CONSTANT_DOUBLE_INFO(x) static_cast<ConstantDoubleInfo *>(x)
#define CAST_CONSTANT_STRING_INFO(x) static_cast<ConstantStringInfo *>(x)
#define CAST_CONSTANT_UTF_8_INFO(x) static_cast<ConstantUTF8Info *>(x)
#define CAST_CONSTANT_CLASS_INFO(x) static_cast<ConstantClassInfo *>(x)
#define CAST_CONSTANT_1_INDEX_INFO(x) static_cast<Constant1IndexInfo *>(x)
#define CAST_CONSTANT_CLASS_NAME_TYPE_INDEX_INFO(x) static_cast<ConstantClassNameTypeIndexInfo *>(x)
#define CAST_CONSTANT_INVOKE_DYNAMIC_INFO(x) static_cast<ConstantInvokeDynamicInfo *>(x)
#define CAST_CONSTANT_NAME_AND_TYPE_INFO(x) static_cast<ConstantNameAndTypeInfo *>(x)
#define CAST_CONSTANT_METHOD_HANDLE_INFO(x) static_cast<ConstantMethodHandleInfo *>(x)
#define CAST_CONSTANT_METHOD_TYPE_INFO(x) static_cast<ConstantMethodTypeInfo *>(x)

#define CAST_BYTE_STREAM_ATTRIBUTE(x) static_cast<ByteStreamAttribute *>(x)
#define CAST_CODE_ATTRIBUTE(x) static_cast<CodeAttribute *>(x)
#define CAST_SIGNATURE_ATTRIBUTE(x) static_cast<SignatureAttribute *>(x)
#define CAST_SOURCE_FILE_ATTRIBUTE(x) static_cast<SourceFileAttribute *>(x)
#define CAST_BOOT_STRAP_METHODS_ATTRIBUTE(x) static_cast<BootstrapMethodsAttribute *>(x)
#define CAST_ENCLOSING_METHOD_ATTRIBUTE(x) static_cast<EnclosingMethodAttribute *>(x)
#define CAST_CONSTANT_VALUE_ATTRIBUTE(x) static_cast<ConstantValueAttribute *>(x)
#define CAST_LINE_NUMBER_ATTRIBUTE(x) static_cast<LineNumberTableAttribute *>(x)
#define CAST_INNER_CLASSES_ATTRIBUTE(x) static_cast<InnerClassesAttribute *>(x)
#define CAST_EXCEPTIONS_ATTRIBUTE(x) static_cast<ExceptionsAttribute *>(x)

#define CAST_INSTANCE_CLASS(x) static_cast<InstanceClass *>(x)
#define CAST_PRIMITIVE_CLASS(x) static_cast<PrimitiveClass *>(x)
#define CAST_ARRAY_CLASS(x) static_cast<ArrayClass *>(x)
#define CAST_OBJ_ARRAY_CLASS(x) static_cast<ObjArrayClass *>(x)
#define CAST_TYPE_ARRAY_CLASS(x) static_cast<TypeArrayClass *>(x)

#define CAST_INSTANCE_OOP(x) static_cast<InstanceOop *>(x)
#define CAST_ARRAY_OOP(x) static_cast<ArrayOop *>(x)
#define CAST_OBJ_ARRAY_OOP(x) static_cast<ObjArrayOop *>(x)
#define CAST_TYPE_ARRAY_OOP(x) static_cast<TypeArrayOop *>(x)
#define CAST_BYTE_TYPE_ARRAY_OOP(x) static_cast<ByteTypeArrayOop *>(x)
#define CAST_CHAR_TYPE_ARRAY_OOP(x) static_cast<CharTypeArrayOop *>(x)
#define CAST_SHORT_TYPE_ARRAY_OOP(x) static_cast<ShortTypeArrayOop *>(x)
#define CAST_INT_TYPE_ARRAY_OOP(x) static_cast<IntTypeArrayOop *>(x)
#define CAST_FLOAT_TYPE_ARRAY_OOP(x) static_cast<FloatTypeArrayOop *>(x)
#define CAST_LONG_TYPE_ARRAY_OOP(x) static_cast<LongTypeArrayOop *>(x)
#define CAST_DOUBLE_TYPE_ARRAY_OOP(x) static_cast<DoubleTypeArrayOop *>(x)
#define CAST_MIRROR_OOP(x) static_cast<MirrorOop *>(x)
#define CAST_VM_THREAD_OOP(x) static_cast<VMThread *>(x)

#define GET_MIRROR_CLASS(x) CAST_MIRROR_OOP(x)->mirrorClass
#define GET_MIRROR_INSTANCE_CLASS(x) CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(x)->mirrorClass)

#define ASSERT_IF_NULL_THROW_NPE(x) \
  if ((x) == nullptr) { \
    throwNullPointException(frame); \
    return; \
  }

#define ASSERT_IF_ZERO_THROW_DIV_ZERO(x) \
  if ((x) == static_cast<decltype((x))>(0)) { \
    throwArithmeticExceptionDivByZero(frame); \
    return; \
  }

#endif
