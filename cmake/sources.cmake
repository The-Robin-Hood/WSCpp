set(IMGUI_SOURCES
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_stdlib.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_impl_glfw.cpp
    thirdparty/imgui/imgui_impl_opengl3.cpp
    thirdparty/imgui/imgui_freetype.cpp
    thirdparty/imgui/imgui_custom.cpp
)

set(PROJECT_SOURCES
    src/main.cpp
    src/gui.cpp
    src/ws.cpp
)

function(configure_target_includes TARGET)
    target_include_directories(${TARGET}
        PRIVATE
        ${OPENSSL_INCLUDE_DIR}
        thirdparty/poco/include
        thirdparty/imgui/include
        thirdparty/glfw/include
        thirdparty/freetype/include
        thirdparty/others/include
        src/
    )
endfunction()

#==================================

function(configure_compiler_options TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /W4 /MP)
        log_status("Configured MSVC compiler options")
    else()
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -pedantic)
        log_status("Configured GCC/Clang compiler options")
    endif()
endfunction()
