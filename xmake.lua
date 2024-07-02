add_rules("mode.debug", "mode.release")

set_languages("c99", "cxx20")
set_optimize("none")

target("RexVM")
    set_kind("binary")
    set_toolchains("clang")
    -- set_toolset("debugger", "lldb")
    add_includedirs(
        "src/third_party/miniz",
        "src/third_party/fmt/include"
    )
    add_files(
        "src/*.cpp",
        "src/utils/*.cpp",
        "src/native/*.cpp",
        "src/native/core/*.cpp",

        "src/third_party/miniz/miniz.c"
    )
    -- set_warnings("all", "error")

    -- set_policy("build.sanitizer.address", true)
    set_policy("build.sanitizer.undefined", true)
