#include "class_path.hpp"
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <ranges>
#include "string_utils.hpp"
#include "../exception.hpp"
#include "../file_system.hpp"

namespace RexVM {

    cstring ClassPath::getVMClassPath() const {
        return path;
    }

    std::unique_ptr<std::istream> DirClassPath::getStream(cview filePath) {
        const auto fullPath = cformat("{}{}", path, filePath);
        if (std::filesystem::exists(fullPath)) {
            return std::make_unique<std::ifstream>(fullPath, std::ios::binary);
        }
        return nullptr;
    }

    ZipClassPath::ZipClassPath(cview path) : ClassPath(ClassPathTypeEnum::ZIP, path) {
        if (std::filesystem::exists(path)) {
            isOpened = mz_zip_reader_init_file(&archive, path.data(), 0);
        }
    }

    ZipClassPath::~ZipClassPath() {
        if (isOpened) {
            mz_zip_reader_end(&archive);
            isOpened = false;
        }
    }

    std::unique_ptr<std::istream> ZipClassPath::getStream(cview filePath) {

        if (!isOpened) {
            return nullptr;
        }

        const auto fileIndex = mz_zip_reader_locate_file(&archive, filePath.data(), nullptr, 0);
        if (fileIndex == -1) {
            return nullptr;
        }

        mz_zip_archive_file_stat fileStat{0};
        if (!mz_zip_reader_file_stat(&archive, fileIndex, &fileStat)) {
            return nullptr;
        }


        const auto uncompressedSize = CAST_SIZE_T(fileStat.m_uncomp_size);
        cstring buffer(uncompressedSize, 0);
        if (!mz_zip_reader_extract_to_mem(&archive, fileIndex, reinterpret_cast<voidPtr>(buffer.data()), uncompressedSize, 0)) {
            return nullptr;
        }
        return std::make_unique<std::istringstream>(buffer);
    }

    CombineClassPath::CombineClassPath(cview path, cview javaHome) 
        : ClassPath(ClassPathTypeEnum::COMBINE, path), javaHome(javaHome) {
        const auto fileSep = cstring{FILE_SEPARATOR};
        const auto paths = splitString(path, PATH_SEPARATOR);
        for (const auto &pathView: paths) {
            if (pathView.empty()) {
                continue;
            }
            const cstring childPath{pathView};
            if (std::filesystem::is_regular_file(pathView)) {
                if (endsWith(childPath, ".jar") || endsWith(childPath, ".JAR")) {
                    if (processedPath.find(childPath) == processedPath.end()) {
                        classPaths.push_back(std::make_unique<ZipClassPath>(cstring(childPath)));
                        processedPath.insert(childPath);
                    }
                }
            } else if (std::filesystem::is_directory(childPath)) {
                if (endsWith(childPath, fileSep)) {
                    if (processedPath.find(childPath) == processedPath.end()) {
                        classPaths.push_back(std::make_unique<DirClassPath>(cstring(childPath)));
                        processedPath.insert(childPath);
                    }
                } else {
                    const cstring spath = childPath + fileSep;
                    if (processedPath.find(spath) == processedPath.end()) {
                        classPaths.push_back(std::make_unique<DirClassPath>(spath));
                        processedPath.insert(spath);
                    }
                }
            }
        }
    }

    std::unique_ptr<std::istream> CombineClassPath::getStream(cview filePath) {
        for (auto &&cp: classPaths) {
            auto stream = cp->getStream(filePath);
            if (stream != nullptr) {
                return stream;
            }
        }

        return nullptr;
    }

    cstring CombineClassPath::getVMClassPath() const {
        return joinString(processedPath, {PATH_SEPARATOR});
    }

    std::unique_ptr<CombineClassPath> CombineClassPath::getDefaultCombineClassPath(
        cview javaHome,
        cview userClassPath
    ) {
        std::vector<cstring> pathList;
        pathList.emplace_back(".");
        pathList.emplace_back(buildRtPath(javaHome));
        pathList.emplace_back(buildCharsetsPath(javaHome));
        const auto userEnvClassPath = std::getenv("CLASSPATH");
        if (userEnvClassPath != nullptr) {
            const auto envClassPath = cstring(userEnvClassPath);
            const auto cps = splitString(envClassPath, PATH_SEPARATOR);
            for (const auto &item: cps) {
                pathList.emplace_back(item);
            }
        }
        if (!userClassPath.empty()) {
            const auto cps = splitString(userClassPath, PATH_SEPARATOR);
            for (const auto &item: cps) {
                pathList.emplace_back(item);
            }
        }

        return std::make_unique<CombineClassPath>(joinString(pathList, cstring{PATH_SEPARATOR}), javaHome);
    }
}