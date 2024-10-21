#include <vector>
#include "utils/format.hpp"
#include "vm.hpp"

void printUsage() {
    RexVM::cprintln("Usage: rex [-cp <classpath>] <MainClass> [params...]");
}

int parseArgs(int argc, char *argv[], RexVM::ApplicationParameter &applicationParameter) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    RexVM::cstring classpath;
    std::vector<RexVM::cstring> params;

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

int main(int argc, char *argv[]) {
    RexVM::ApplicationParameter applicationParameter;
    if (parseArgs(argc, argv, applicationParameter) == 0) {
        vmMain(applicationParameter);
    }
    
    return 0;
}