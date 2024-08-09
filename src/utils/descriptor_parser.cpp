#include "descriptor_parser.hpp"
#include "string_utils.hpp"

namespace RexVM {

    //从一个descriptor中拿到第一个元素 返回它的 字符长度 Slot长度 以及类型
    //type 0:基本类型 1: 普通引用类型 2: 数组引用类型
    std::tuple<size_t, u1, u1> getSingleDescriptor(const cview descriptor) {
        auto firstChar = descriptor[0];
        const auto size = descriptor.size();
        size_t descSize{};
        u1 slotSize{};
        u1 type{};
        switch (firstChar) {
            case 'B': case 'C': case 'S':
            case 'F': case 'I': case 'Z': case 'V':
                descSize = 1;
                slotSize = 1;
                type = 0;
                break;

            case 'J': case 'D':
                descSize = 1;
                slotSize = 2;
                type = 0;
                break;

            case 'L':
                descSize = descriptor.find(';', 2) + 1; //Ljava/lang/Object; skip Lj
                slotSize = 1;
                type = 1;
                break;

            case '[': {
                decltype(firstChar) arrayElementFirstChar;
                size_t arrayLength = 1;
                for (size_t arrayPos = 1; arrayPos < size; ++arrayPos) {
                    arrayElementFirstChar = descriptor[arrayPos];
                    if (arrayElementFirstChar != '[') {
                        break;
                    } 
                    arrayLength += 1;
                }
                if (arrayElementFirstChar == 'L') {
                    descSize = descriptor.find(';', arrayLength) + 1;
                } else {
                    descSize = arrayLength + 1;
                }
                slotSize = 1;
                type = 2;
                break;
            }

            default:
                panic("error first char");
        }
        return std::make_tuple(descSize, slotSize, type);
    }

    //解析一个普通descriptor 返回其中每个元素的类名 primitive类型也用int, long等实际类名
    std::vector<cstring> parseDescriptor(const cview str, size_t start, size_t end) {
        size_t pos = start;
        std::vector<cstring> types;
        while (pos < end) {
            const auto [descSize, slotSize, type] = getSingleDescriptor(str.substr(pos));
            switch (type) {
                case 0:
                    types.emplace_back(getPrimitiveClassNameByDescriptor(str[pos]));
                    break;
                case 1:
                    types.emplace_back(str.substr(pos + 1, descSize - 2));
                    break;
                case 2:
                    types.emplace_back(str.substr(pos, descSize));
                    break;
                default:
                    panic("error type");
            }
            pos += descSize;
        }
        return types;
    }

    std::vector<cstring> parseDescriptor(const cview str) {
        return parseDescriptor(str, 0, str.size());
    }

    //解析一个函数的descriptor 返回参数和返回值的类名
    std::tuple<std::vector<cstring>, cstring> parseMethodDescriptor(const cview str) {
        const auto paramBegin = str.find('(');
        const auto paramEnd = str.find(')');
        const auto returnType = parseDescriptor(str, paramEnd + 1, str.size()).at(0);
        return std::make_tuple(parseDescriptor(str, paramBegin + 1, paramEnd), cstring(returnType));
    }

    //解析一个函数descriptor 计算参数部分要占几个Slot size
    size_t getMethodParamSlotSizeFromDescriptor(const cview descriptor, bool isStatic) {
        size_t paramSlotSize = isStatic ? 0 : 1;
        const auto paramBegin = descriptor.find('(');
        const auto paramEnd = descriptor.find(')');
        size_t pos = paramBegin + 1;
        while (pos < paramEnd) {
            const auto [descSize, slotSize, type] = getSingleDescriptor(descriptor.substr(pos));
            //const auto desc = descriptor.substr(pos, descSize);
            paramSlotSize += slotSize;
            pos += descSize;
        }

        return paramSlotSize;
    }
    
}