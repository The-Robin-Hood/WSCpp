cmake_minimum_required(VERSION 3.30)
project(WSCpp LANGUAGES CXX)

# Add Objective-C++ language for macOS
if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32 OR APPLE)
    if(OPENSSL_ROOT_DIR)
        message("OpenSSL root dir: ${OPENSSL_ROOT_DIR}")
    else()
        message(FATAL_ERROR "OpenSSL root dir not found. Please set OPENSSL_ROOT_DIR.")
    endif()
endif()

find_package(OpenSSL REQUIRED)

if(OpenSSL_FOUND)
    message(STATUS "Found OpenSSL: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include Dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found!")
endif()

# Function to disable Poco modules
function(disable_poco_modules)
    foreach(module ${ARGV})
        set(ENABLE_${module} OFF CACHE BOOL "" FORCE)
    endforeach()
endfunction()

# Disable all unwanted modules
disable_poco_modules(
    JWT
    DATA
    DATA_MYSQL
    DATA_POSTGRESQL
    DATA_ODBC
    APACHECONNECTOR
    MONGODB
    REDIS
    PROMETHEUS
    PDF
    FUZZING
    SAMPLES
    TESTS
    POCODOC
    XML
    JSON
    ACTIVERECORD
    ACTIVERECORD_COMPILER
    ENCODINGS
    PAGECOMPILER
    PAGECOMPILER_FILE2PAGE
    DATA_SQLITE
)

# Enable required modules
set(ENABLE_UTIL ON CACHE BOOL "" FORCE)
set(ENABLE_NET ON CACHE BOOL "" FORCE)
set(ENABLE_NETSSL ON CACHE BOOL "" FORCE)
set(ENABLE_CRYPTO ON CACHE BOOL "" FORCE)

# Disable GLFW examples, tests, and docs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

add_subdirectory(thirdparty/poco)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/freetype)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/gui.cpp
    src/ws.cpp
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

target_include_directories(${PROJECT_NAME}
    PRIVATE
    {OPENSSL_INCLUDE_DIR}
    thirdparty/poco/include
    thirdparty/imgui/include
    thirdparty/glfw/include
    thirdparty/freetype/include
    thirdparty/others/include
    src/
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
    thirdparty/glfw/src
    thirdparty/freetype/src
)

if(WIN32)
    message("Building for Windows")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        OpenSSL::SSL OpenSSL::Crypto
        Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
        glfw
        opengl32
        freetype
    )
elseif(APPLE)
    message("Building for macOS")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        OpenSSL::SSL OpenSSL::Crypto
        Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
        glfw
        freetype
        "-framework OpenGL"
    )
else()
    message("Building for Linux")
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        OpenSSL::SSL OpenSSL::Crypto
        Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
        glfw
        freetype
        OpenGL::GL
    )
endif()