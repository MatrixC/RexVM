add_rules("mode.debug", "mode.release", "mode.check")
set_languages("c11", "cxx20")
option("llvm-jit")
    set_default(false)
    set_description("llvm-jit")
    local llvm_path = os.getenv("LLVM_PATH")
    add_includedirs(llvm_path .. "/include")
    add_linkdirs(llvm_path .. "/lib")
    add_links("LLVM-17")
    add_defines("LLVM_JIT")
option_end()

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
    set_toolchains("clang")
    add_includedirs(
        "third_party/miniz",
        "third_party/fmt/include",
        "third_party/emhash"
    )
    add_files(
        "src/*.cpp",
        "src/jit/*.cpp",
        "src/utils/*.cpp",
        "src/native/*.cpp",
        "src/native/core/*.cpp",
        "src/native/misc/*.cpp",
        "src/native/sun/*.cpp",
        "src/native/rex/*.cpp",
    

        "third_party/miniz/miniz.c"
    )

    -- llvm
--     add_includedirs("/opt/homebrew/Cellar/llvm/18.1.6/include")
--     add_linkdirs("/opt/homebrew/Cellar/llvm/18.1.6/lib")
--     add_links("LLVM-18")

    add_includedirs("/usr/lib/llvm-17/include")
    add_linkdirs("/usr/lib/llvm-17/lib")
    add_links("LLVM-17")

target_end()
