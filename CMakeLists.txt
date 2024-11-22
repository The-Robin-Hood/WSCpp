cmake_minimum_required(VERSION 3.30)
project(WSCpp
    VERSION 1.0.0
    DESCRIPTION "WebSocket Client GUI Application in C++"
    LANGUAGES CXX
)

# Initialize CMake modules
include(cmake/utils.cmake)
include(cmake/dependencies.cmake)
include(cmake/sources.cmake)

# Set up CMake configurations
setup_cmake_configs()
setup_compiler_options()
setup_console_colors()

# Configure dependencies
configure_openssl()
configure_poco()
configure_glfw()

# Add third-party dependencies
add_subdirectory(thirdparty/poco)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/freetype)
add_subdirectory(thirdparty/spdlog)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_library(WS STATIC ${WS_SOURCES})
target_include_directories(WS PUBLIC ${COMMON_INCLUDES} ${WS_INCLUDES})
target_link_libraries(WS PUBLIC ${COMMON_LIBS} ${WS_LIBS})
configure_target_compiler_options(WS)

add_library(GUI STATIC ${GUI_SOURCES})
target_include_directories(GUI PUBLIC ${GUI_INCLUDES})
target_link_libraries(GUI PUBLIC ${GUI_LIBS})
configure_target_compiler_options(GUI)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${WSCPP_SOURCES})
else()
    add_executable(${PROJECT_NAME} ${WSCPP_SOURCES})
endif()

target_link_libraries(WSCpp PRIVATE GUI WS)
configure_target_compiler_options(WSCpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
configure_assets(${PROJECT_NAME})