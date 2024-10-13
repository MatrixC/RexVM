add_rules("mode.debug", "mode.release", "mode.check")
set_languages("c11", "cxx20")
option("llvm-jit")
    set_default(true)
    set_showmenu(true)
    set_description("llvm-jit")
    local llvm_path = os.getenv("LLVM_PATH")
    local llvm_version = os.getenv("LLVM_VERSION")
    local include_dir = llvm_path .. "/include"
    local lib_dir = llvm_path .. "/lib"
    add_includedirs(include_dir)
    add_linkdirs(lib_dir)
    add_links(llvm_version)
    --add_cxxflags("-fno-exceptions")
    add_cxxflags("-funwind-tables")
    add_cxxflags("-D_GNU_SOURCE")
    add_cxxflags("-D__STDC_CONSTANT_MACROS")
    add_cxxflags("-D__STDC_FORMAT_MACROS")
    add_cxxflags("-D__STDC_LIMIT_MACROS")
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
    add_options("llvm-jit")
    set_toolchains("clang")
    add_includedirs(
        "third_party/miniz",
        "third_party/fmt/include",
        "third_party/emhash"
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

if get_config("llvm-jit") then
    add_files("src/jit/*.cpp")
end

target_end()
