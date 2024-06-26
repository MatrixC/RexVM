#include "config.hpp"
#include "utils/format.hpp"
#include "vm.hpp"
#include <vector>
#include <variant>

using namespace RexVM;

void printUsage() {
    println("Usage: RexVM -cp <classpath> <MainClass> [params...]");
}

int parseArgs(int argc, char *argv[], ApplicationParameter &applicationParameter) {
    if (argc < 4) {
        printUsage();
        return 1;
    }
    
    cstring classpath;
    std::vector<cstring> params;

    for (auto i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-cp") == 0) {
            if (i + 1 < argc) {
                applicationParameter.userClassPath = argv[++i];
            } else {
                printUsage();
                return 1;
            }
        } else {
            params.emplace_back(argv[i]);
        }
    }

    applicationParameter.userParams = params;
    return 0;
}

int main(int argc, char *argv[]) {
    ApplicationParameter applicationParameter;
    if (parseArgs(argc, argv, applicationParameter) == 0) {
        vmMain(applicationParameter);
    }

    return 1;
}
