#ifndef CLASS_PATH_HPP
#define CLASS_PATH_HPP

#include <memory>
#include <fstream>
#include <utility>
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

        explicit ClassPath(ClassPathTypeEnum type, cstring path) : type(type), path(std::move(path)) {
        }

        virtual ~ClassPath() = default;

        virtual std::unique_ptr<std::istream> getStream(const cstring &filePath) = 0;
        virtual cstring getVMClassPath() const;

        bool operator==(const ClassPath &other) const {
            return path == other.path;
        }
    };

    struct DirClassPath : ClassPath {
        explicit DirClassPath(const cstring &path) : ClassPath(ClassPathTypeEnum::DIR, path) {
        }

        std::unique_ptr<std::istream> getStream(const cstring &filePath) override;

    };

    struct ZipClassPath : ClassPath {
        bool isOpened{false};
        mz_zip_archive archive{0};

        explicit ZipClassPath(const cstring &path);

        ~ZipClassPath() override;

        std::unique_ptr<std::istream> getStream(const cstring &filePath) override;
    };

    struct CombineClassPath : ClassPath {
        cstring javaHome{};
        std::vector<std::unique_ptr<ClassPath>> classPaths;
        std::unordered_set<cstring> processedPath;

        explicit CombineClassPath(const cstring &path, const cstring &javaHome = {});
        cstring getVMClassPath() const override;

        std::unique_ptr<std::istream> getStream(const cstring &filePath) override;

        static std::unique_ptr<CombineClassPath> getDefaultCombineClassPath(const cstring &javaHome, const cstring &userClassPath);
    };



}

#endif