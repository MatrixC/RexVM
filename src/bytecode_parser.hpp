#ifndef BYTE_CODE_PARSER_HPP
#define BYTE_CODE_PARSER_HPP
#include "utils/byte_reader.hpp"

namespace RexVM {

    struct Method;

    struct ByteCodeParser {

        ByteReader reader{};

        explicit ByteCodeParser(const Method &method);

        void parse();

    };

}

#endif