add_rules("mode.debug", "mode.release")
add_requires("fmt")

set_languages("c99", "cxx23")
set_optimize("none")

target("RexVM")
    set_kind("binary")
    set_toolchains("clang")
    set_toolset("debugger", "lldb")
    add_includedirs("src/third_party/miniz")
    add_files(
        "src/*.cpp",
        "src/utils/*.cpp",
        "src/native/*.cpp",

        "src/third_party/miniz/miniz.c",
        "src/third_party/miniz/miniz_zip.c",
        "src/third_party/miniz/miniz_tinfl.c",
        "src/third_party/miniz/miniz_tdef.c"
    )
    add_packages("fmt")
    set_warnings("all", "error")

    -- set_policy("build.sanitizer.address", true)
    -- set_policy("build.sanitizer.undefined", true)
