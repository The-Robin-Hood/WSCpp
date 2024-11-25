set(WSCPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
set(WS_SOURCES src/websocket/ws.cpp)
set(GUI_SOURCES src/gui/gui.cpp src/gui/ui.cpp
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

set(COMMON_INCLUDES
    thirdparty/spdlog/include
    src/utils/
)

set(WS_INCLUDES
    ${OPENSSL_INCLUDE_DIR}
    src/websocket/
    thirdparty/poco/include
)

set(GUI_INCLUDES
    src/gui/
    src/utils/
    thirdparty/imgui/include
    thirdparty/glfw/include
    thirdparty/freetype/include
    thirdparty/others/include
)

set(COMMON_LIBS spdlog::spdlog)
set(WS_LIBS Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
    set(GUI_LIBS WS glfw freetype opengl32)
elseif(APPLE)
    set(GUI_LIBS WS glfw freetype "-framework OpenGL")
else()
    set(GUI_LIBS WS glfw freetype OpenGL::GL)
endif()

function(setup_compiler_options)
    if(MSVC)
        add_compile_options(/MP)
    endif()
endfunction()

function(configure_target_compiler_options TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /W4 /MP)
    else()
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -pedantic)
    endif()
endfunction()
