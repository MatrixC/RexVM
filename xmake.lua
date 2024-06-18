add_rules("mode.debug", "mode.release")
add_requires("fmt", "miniz")

set_languages("c99", "cxx23")
set_optimize("none")


target("RexVM")
    set_kind("binary")
    set_toolchains("clang")
    set_toolset("debugger", "lldb")
    add_files(
        "src/*.cpp",
        "src/utils/*.cpp",
        "src/native/*.cpp"
    )
    add_packages("fmt", "miniz")
    set_warnings("all", "error")

    -- set_policy("build.sanitizer.address", true)
    -- set_policy("build.sanitizer.undefined", true)
