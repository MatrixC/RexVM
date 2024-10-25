#ifndef CLASS_PATH_HPP
#define CLASS_PATH_HPP

#include <memory>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <miniz.h>
#include "../config.hpp"

namespace RexVM {

    enum class ClassPathTypeEnum {
        DIR,
        ZIP,
        COMBINE,
    };

    struct ClassPath {
        const ClassPathTypeEnum type;
        const cstring path;

        explicit ClassPath(const ClassPathTypeEnum type, const cview path) : type(type), path(path) {
        }

        virtual ~ClassPath() = default;

        virtual std::unique_ptr<std::istream> getStream(cview filePath) = 0;
        [[nodiscard]] virtual cstring getVMClassPath() const;

        bool operator==(const ClassPath &other) const {
            return path == other.path;
        }
    };

    struct DirClassPath : ClassPath {
        explicit DirClassPath(cview path) : ClassPath(ClassPathTypeEnum::DIR, path) {
        }

        std::unique_ptr<std::istream> getStream(cview filePath) override;

    };

    struct ZipClassPath : ClassPath {
        bool isOpened{false};
        mz_zip_archive archive{0};

        explicit ZipClassPath(cview path);

        ~ZipClassPath() override;

        std::unique_ptr<std::istream> getStream(cview filePath) override;
    };

    struct CombineClassPath : ClassPath {
        cview javaHome{};
        std::vector<std::unique_ptr<ClassPath>> classPaths;
        std::unordered_set<cstring> processedPath;

        explicit CombineClassPath(cview path, cview javaHome = {});
        [[nodiscard]] cstring getVMClassPath() const override;

        std::unique_ptr<std::istream> getStream(cview filePath) override;

        static std::unique_ptr<CombineClassPath> getDefaultCombineClassPath(cview javaHome, cview userClassPath);
    };



}

#endif