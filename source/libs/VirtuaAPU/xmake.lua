add_rules("mode.debug", "mode.release")

set_languages("c17")
set_optimize("fastest")
set_defaultmode("release")

target("VirtuaAPU")
    set_kind("static")
    if is_plat("windows") then
        set_plat("mingw")
        set_toolchains("mingw")
    end
    add_includedirs("include", {public = true})
    add_headerfiles("include/**.h")
    add_files("src/*.c")
