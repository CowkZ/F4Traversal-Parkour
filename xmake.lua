-- CommonLibF4 (git submodule em lib/commonlibf4, veja setup.sh)
includes("lib/commonlibf4")

-- constantes do projeto
set_project("F4Traversal")
set_version("0.1.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")
set_encodings("utf-8")

add_rules("mode.debug", "mode.releasedbg")

target("F4Traversal")
    -- "name" aparece no log e nos metadados do plugin; renomeie aqui quando quiser
    add_rules("commonlibf4.plugin", {
        name = "F4Traversal",
        author = "local",
        description = "Base de debug para port de parkour (Fallout 4)"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
