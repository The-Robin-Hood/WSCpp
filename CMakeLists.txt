cmake_minimum_required(VERSION 3.30)
project(WSCpp LANGUAGES CXX)

# Add Objective-C++ language for macOS
if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Function to disable Poco modules
function(disable_poco_modules)
    foreach(module ${ARGV})
        set(ENABLE_${module} OFF CACHE BOOL "" FORCE)
    endforeach()
endfunction()

# Disable all unwanted modules
disable_poco_modules(
    NETSSL
    CRYPTO
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

add_subdirectory(thirdParty/poco)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    thirdParty/poco/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
    thirdParty/poco/lib
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Poco::Net Poco::Foundation
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "../dist"
)