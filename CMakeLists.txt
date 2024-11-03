cmake_minimum_required(VERSION 3.30)
project(WSCpp 
    VERSION 1.0.0
    DESCRIPTION "WebSocket Client GUI Application in C++"
    LANGUAGES CXX
)

# Initialize CMake modules
include(cmake/utils.cmake)
include(cmake/dependencies.cmake)
include(cmake/platform.cmake)
include(cmake/sources.cmake)
setup_console_colors()

# Project configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

if(APPLE)
    enable_language(OBJCXX)
endif()

# Configure dependencies
configure_openssl()
configure_poco()
configure_glfw()

# Add third-party dependencies
add_subdirectory(thirdparty/poco)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/freetype)

# Create executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${IMGUI_SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Configure the target
configure_compiler_options(${PROJECT_NAME})
configure_target_includes(${PROJECT_NAME})
configure_platform_specific_linking(${PROJECT_NAME})
configure_assets(${PROJECT_NAME})