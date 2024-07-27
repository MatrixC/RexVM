add_rules("mode.debug", "mode.release")

set_languages("c11", "cxx20")
set_optimize("none")

target("rex")
    set_kind("binary")
    set_toolchains("clang")
    add_includedirs(
        "third_party/miniz",
        "third_party/fmt/include"
    )
    add_files(
        "src/*.cpp",
        "src/utils/*.cpp",
        "src/native/*.cpp",
        "src/native/core/*.cpp",
        "src/native/sun/*.cpp",

        "third_party/miniz/miniz.c"
    )
    set_warnings("all", "error")

    -- set_policy("build.sanitizer.address", true)
    set_policy("build.sanitizer.undefined", true)
