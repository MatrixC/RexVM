#ifndef BASIC_MACRO_HPP
#define BASIC_MACRO_HPP

#define INSTANCE_CAST static_cast

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
#define CAST_CONSTANT_INTEGER_INFO(x) INSTANCE_CAST<ConstantIntegerInfo *>(x)
#define CAST_CONSTANT_LONG_INFO(x) INSTANCE_CAST<ConstantLongInfo *>(x)
#define CAST_CONSTANT_FLOAT_INFO(x) INSTANCE_CAST<ConstantFloatInfo *>(x)
#define CAST_CONSTANT_DOUBLE_INFO(x) INSTANCE_CAST<ConstantDoubleInfo *>(x)
#define CAST_CONSTANT_STRING_INFO(x) INSTANCE_CAST<ConstantStringInfo *>(x)
#define CAST_CONSTANT_UTF_8_INFO(x) INSTANCE_CAST<ConstantUTF8Info *>(x)
#define CAST_CONSTANT_CLASS_INFO(x) INSTANCE_CAST<ConstantClassInfo *>(x)
#define CAST_CONSTANT_1_INDEX_INFO(x) INSTANCE_CAST<Constant1IndexInfo *>(x)
#define CAST_CONSTANT_CLASS_NAME_TYPE_INDEX_INFO(x) INSTANCE_CAST<ConstantClassNameTypeIndexInfo *>(x)
#define CAST_CONSTANT_INVOKE_DYNAMIC_INFO(x) INSTANCE_CAST<ConstantInvokeDynamicInfo *>(x)
#define CAST_CONSTANT_NAME_AND_TYPE_INFO(x) INSTANCE_CAST<ConstantNameAndTypeInfo *>(x)
#define CAST_CONSTANT_METHOD_HANDLE_INFO(x) INSTANCE_CAST<ConstantMethodHandleInfo *>(x)
#define CAST_CONSTANT_METHOD_TYPE_INFO(x) INSTANCE_CAST<ConstantMethodTypeInfo *>(x)

#define CAST_BYTE_STREAM_ATTRIBUTE(x) INSTANCE_CAST<ByteStreamAttribute *>(x)
#define CAST_CODE_ATTRIBUTE(x) INSTANCE_CAST<CodeAttribute *>(x)
#define CAST_SIGNATURE_ATTRIBUTE(x) INSTANCE_CAST<SignatureAttribute *>(x)
#define CAST_SOURCE_FILE_ATTRIBUTE(x) INSTANCE_CAST<SourceFileAttribute *>(x)
#define CAST_BOOT_STRAP_METHODS_ATTRIBUTE(x) INSTANCE_CAST<BootstrapMethodsAttribute *>(x)
#define CAST_ENCLOSING_METHOD_ATTRIBUTE(x) INSTANCE_CAST<EnclosingMethodAttribute *>(x)
#define CAST_CONSTANT_VALUE_ATTRIBUTE(x) INSTANCE_CAST<ConstantValueAttribute *>(x)
#define CAST_LINE_NUMBER_ATTRIBUTE(x) INSTANCE_CAST<LineNumberTableAttribute *>(x)
#define CAST_INNER_CLASSES_ATTRIBUTE(x) INSTANCE_CAST<InnerClassesAttribute *>(x)
#define CAST_EXCEPTIONS_ATTRIBUTE(x) INSTANCE_CAST<ExceptionsAttribute *>(x)

#define CAST_CLASS(x) static_cast<Class *>(x)
#define CAST_CLASS_MEMBER(x) static_cast<ClassMember *>(x)
#define CAST_METHOD(x) static_cast<Method *>(x)
#define CAST_FIELD(x) static_cast<Field *>(x)
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
#define CAST_MIRROR_OOP(x) static_cast<MirOop *>(x)
#define CAST_VM_THREAD_OOP(x) static_cast<VMThread *>(x)

#define GET_MIRROR_CLASS(x) CAST_MIRROR_OOP(x)->getMirrorClass()
#define GET_MIRROR_INSTANCE_CLASS(x) CAST_INSTANCE_CLASS(CAST_MIRROR_OOP(x)->getMirrorClass())

#define FOR_FROM(f, x) for (auto i = std::remove_const_t<decltype(x)>(f); i < (x); ++i)
#define FOR_FROM_ZERO(x) FOR_FROM(0, (x))
#define FOR_FROM_ONE(x)  FOR_FROM(1, (x))

#define FOR_VECTOR(x, iname) for (const auto & iname : (x))
#define FOR_VECTOR_ITEM(x) FOR_VECTOR(x, item)

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
