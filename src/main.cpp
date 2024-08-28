#include "utils/format.hpp"
#include "vm.hpp"
#include <vector>

using namespace RexVM;

void printUsage() {
    cprintln("Usage: rex [-cp <classpath>] <MainClass> [params...]");
}

int parseArgs(int argc, char *argv[], ApplicationParameter &applicationParameter) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    cstring classpath;
    std::vector<cstring> params;

    for (auto i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
            if (i + 1 < argc) {
                applicationParameter.userClassPath = argv[++i];
            }
        } else {
            params.emplace_back(argv[i]);
        }
    }

    applicationParameter.userParams = params;
    return 0;
}

#include "composite_string.hpp"

int main(int argc, char *argv[]) {
    // ApplicationParameter applicationParameter;
    // if (parseArgs(argc, argv, applicationParameter) == 0) {
    //     vmMain(applicationParameter);
    // }

    CompositeString tt = "陈哲是abcdefg爱橙科技！@#";
    cprintln("{}, {}", tt.size(), tt.unicodeSize());

    const auto kk = "abdefg";
    char jj[3];
    jj[0] = 'a';
    jj[1] = 'b';
    jj[2] = 'c';
    cprintln("{}, {}", std::strlen(kk), std::strlen(jj));
    cprintln("{}, {}", typeid(tt).name(), typeid("abd").name());


    return 0;
}