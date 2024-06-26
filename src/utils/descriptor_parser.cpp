#include "descriptor_parser.hpp"
#include "string_utils.hpp"

namespace RexVM {

    cview descriptorNextFieldType(const cview str) {
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
                return str.substr(1, endIndex - 1);
            }

            default:
                panic("descriptorNextFieldType error " + cstring(str));
        }
        return {};
    }

    std::vector<cstring> parseDescriptor(const cview str) {
        size_t currentIndex = 0;
        const auto size = str.size();
        std::vector<cstring> types;
        while (currentIndex < size) {
            auto arrayLength = 0;
            if (str[currentIndex] == '[') {
                for (size_t i = currentIndex + 1; i < size; ++i) {
                    if (str[i] != '[') {
                        arrayLength = i - currentIndex;
                        break;
                    }
                }
            }
            const auto typeFirstChar = str[currentIndex + arrayLength];
            const auto typeView = descriptorNextFieldType(str.substr(currentIndex + arrayLength));
            const cstring type = arrayLength == 0 ? cstring(typeView) : concat_view(str.substr(currentIndex, arrayLength), typeView);
            types.emplace_back(type);
            currentIndex += type.size();
            if (typeFirstChar == 'L') {
                currentIndex += 2;  //'L' and ';'
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