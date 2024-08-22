add_rules("mode.debug", "mode.release", "mode.check")
set_languages("c11", "cxx20")

if is_mode("debug") or is_mode("check") then
    add_defines("DEBUG")
    set_optimize("none")
else -- release
    set_warnings("all", "error")
    set_optimize("fastest")
end

if is_mode("check") then
    set_policy("build.sanitizer.address", true)
    set_policy("build.sanitizer.undefined", true)
end

if is_plat("windows") then
    add_cxflags("/utf-8")
elseif is_plat("macosx") then
    set_toolchains("clang")
end

target("rex")
    set_kind("binary")
    add_includedirs(
        "third_party/miniz",
        "third_party/fmt/include"
    )
    add_files(
        "src/*.cpp",
        "src/utils/*.cpp",
        "src/native/*.cpp",
        "src/native/core/*.cpp",
        "src/native/misc/*.cpp",
        "src/native/sun/*.cpp",
        "src/native/rex/*.cpp",

        "third_party/miniz/miniz.c"
    )
target_end()
