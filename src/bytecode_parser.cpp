#include "bytecode_parser.hpp"
#include "class_member.hpp"

namespace RexVM {

    ByteCodeParser::ByteCodeParser(const Method &method) {
        reader.init(method.code.get(), method.codeLength);
    }

    void ByteCodeParser::parse() {
        u1 currentByteCode{};
        while (!reader.eof()) {
            currentByteCode = reader.readU1();
            
        }
    }


}