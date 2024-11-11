cmake_minimum_required(VERSION 3.30)
project(PocoWS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set output director for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

function(configure_openssl)
    if(WIN32 OR APPLE)
        if(NOT OPENSSL_ROOT_DIR)
            message("OpenSSL root dir not found. Please set OPENSSL_ROOT_DIR.")
        else()
            message("OpenSSL root dir: ${OPENSSL_ROOT_DIR}")
        endif()
    endif()

    find_package(OpenSSL REQUIRED)

    if(OpenSSL_FOUND)
        message("OpenSSL version: ${OPENSSL_VERSION}")
    else()
        message("OpenSSL not found!")
    endif()
endfunction()

function(configure_poco)
    function(disable_poco_modules)
        foreach(module ${ARGV})
            set(ENABLE_${module} OFF CACHE BOOL "" FORCE)
        endforeach()
    endfunction()

    disable_poco_modules(
        JWT DATA DATA_MYSQL DATA_POSTGRESQL DATA_ODBC
        APACHECONNECTOR MONGODB REDIS PROMETHEUS PDF
        FUZZING SAMPLES TESTS POCODOC XML JSON
        ACTIVERECORD ACTIVERECORD_COMPILER ENCODINGS
        PAGECOMPILER PAGECOMPILER_FILE2PAGE DATA_SQLITE
    )

    set(REQUIRED_MODULES "UTIL;NET;NETSSL;CRYPTO")

    foreach(MODULE ${REQUIRED_MODULES})
        set(ENABLE_${MODULE} ON CACHE BOOL "" FORCE)
    endforeach()
endfunction()


configure_openssl()
configure_poco()
add_subdirectory(poco)

add_executable(PocoWS main.cpp ws.cpp)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MP)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${OPENSSL_INCLUDE_DIR}
    poco/include
    include/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenSSL::SSL OpenSSL::Crypto
    Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
)