#include "descriptor_parser.hpp"
#include "string_utils.hpp"

namespace RexVM {

    cview descriptorNextFieldType(const cview str, bool isArray) {
        const auto first = str[0];
        switch(first) {
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'S':
            case 'Z':
            case 'V':
                return str.substr(0, 1);
            case 'L': {
                const auto endIndex = str.find(';');
                if (isArray) {
                    return str.substr(0, endIndex + 1);
                } else {
                    return str.substr(1, endIndex - 1);
                }
            }

            default:
                panic("descriptorNextFieldType error " + cstring(str));
        }
        return {};
    }

    //解析出descriptor中对应的类名, primitive类型也用int, long等实际类名
    std::vector<cstring> parseDescriptor(const cview str) {
        size_t currentIndex = 0;
        const auto size = str.size();
        std::vector<cstring> types;
        while (currentIndex < size) {
            size_t arrayLength = 0;
            if (str[currentIndex] == '[') {
                //is array
                for (size_t i = currentIndex + 1; i < size; ++i) {
                    if (str[i] != '[') {
                        arrayLength = i - currentIndex;
                        break;
                    }
                }
            }
            auto isArray = arrayLength != 0;
            const auto typeFirstChar = str[currentIndex + arrayLength];
            const auto typeView = descriptorNextFieldType(str.substr(currentIndex + arrayLength), isArray);
            const cstring type =
                    arrayLength == 0 ?
                        cstring(typeView) :
                        concat_view(str.substr(currentIndex, arrayLength), typeView);
            if (type.size() == 1 && isBasicType(type[0])) {
                types.emplace_back(getPrimitiveClassNameByDescriptor(type[0]));
            } else {
                types.emplace_back(type);
            }
            currentIndex += type.size();
            if (typeFirstChar == 'L' && !isArray) {
                //skip Instance Class Name 'L' and ':'
                currentIndex += 2;
            }
        }
        return types;
    }

    std::tuple<std::vector<cstring>, cstring> parseMethodDescriptor(const cview str) {
        const auto paramBegin = str.find('(');
        const auto paramEnd = str.find(')');
        const auto param = str.substr(paramBegin + 1, paramEnd - paramBegin - 1);
        const auto returnTypeDesc = str.substr(paramEnd + 1);
        const auto returnType = parseDescriptor(returnTypeDesc).at(0);
        return std::make_tuple(parseDescriptor(param), cstring(returnType));
    }

}