add_rules("mode.debug", "mode.release")

set_languages("c17")
set_defaultmode("release")

target("VirtuaPPU")
    set_kind("static")
    if is_plat("windows") then
        set_plat("mingw")
        set_toolchains("mingw")
    end
    add_includedirs("include", {public = true})
    add_headerfiles("include/**.h")
    add_files("src/*.c")
    add_defines("USE_OPENMP")
    add_cflags("-fopenmp", {tools = {"gcc", "clang"}})
    add_ldflags("-fopenmp", {tools = {"gcc", "clang"}})
    add_syslinks("gomp", {public = true})
