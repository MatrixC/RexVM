#include "class_path.hpp"
#include <filesystem>
#include <sstream>
#include "string_utils.hpp"
#include "../exception.hpp"
#include <ranges>

namespace RexVM {

    std::unique_ptr<std::istream> DirClassPath::getStream(const cstring &filePath) {
        const auto fullPath = path + filePath;
        if (std::filesystem::exists(fullPath)) {
            return std::make_unique<std::ifstream>(fullPath, std::ios::binary);
        }
        return nullptr;
    }

    ZipClassPath::ZipClassPath(const cstring &path) : ClassPath(ClassPathTypeEnum::ZIP, path) {
        if (std::filesystem::exists(path)) {
            isOpened = mz_zip_reader_init_file(&archive, path.c_str(), 0);
        }
    }

    ZipClassPath::~ZipClassPath() {
        if (isOpened) {
            mz_zip_reader_end(&archive);
            isOpened = false;
        }
    }

    std::unique_ptr<std::istream> ZipClassPath::getStream(const cstring &filePath) {

        if (!isOpened) {
            return nullptr;
        }

        const auto fileIndex = mz_zip_reader_locate_file(&archive, filePath.c_str(), nullptr, 0);
        if (fileIndex == -1) {
            return nullptr;
        }

        mz_zip_archive_file_stat fileStat{0};
        if (!mz_zip_reader_file_stat(&archive, fileIndex, &fileStat)) {
            return nullptr;
        }


        const auto uncompressedSize = (size_t) fileStat.m_uncomp_size;
        cstring buffer(uncompressedSize, 0);
        if (!mz_zip_reader_extract_to_mem(&archive, fileIndex, (void *) buffer.c_str(), uncompressedSize, 0)) {
            return nullptr;
        }
        return std::make_unique<std::istringstream>(buffer);
    }

    CombineClassPath::CombineClassPath(const cstring &path) : ClassPath(ClassPathTypeEnum::COMBINE, path) {
        const static auto pathSep = ';';
        const static auto dirSep = cstring{std::filesystem::path::preferred_separator};

        //const auto paths = std::ranges::split_view(path, pathSep);

        const auto paths = split_string(path, pathSep);
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
                if (endsWith(childPath, dirSep)) {
                    if (processedPath.find(childPath) == processedPath.end()) {
                        classPaths.push_back(std::make_unique<DirClassPath>(cstring(childPath)));
                        processedPath.insert(childPath);
                    }
                } else {
                    const cstring spath = childPath + dirSep;
                    if (processedPath.find(spath) == processedPath.end()) {
                        classPaths.push_back(std::make_unique<DirClassPath>(spath));
                        processedPath.insert(spath);
                    }
                }
            }
        }
    }

    std::unique_ptr<std::istream> CombineClassPath::getStream(const cstring &filePath) {
        for (auto &&cp: classPaths) {
            auto stream = cp->getStream(filePath);
            if (stream != nullptr) {
                return stream;
            }
        }

        return nullptr;
    }

}