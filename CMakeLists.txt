cmake_minimum_required(VERSION 3.30)
project(003 LANGUAGES CXX)

# Add Objective-C++ language for macOS
if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# poco configuration 
# Disable all modules
set(ENABLE_NETSSL OFF CACHE BOOL "" FORCE)         # Disable NetSSL support
set(ENABLE_CRYPTO OFF CACHE BOOL "" FORCE)          # Disable Crypto support
set(ENABLE_JWT OFF CACHE BOOL "" FORCE)             # Disable JWT support
set(ENABLE_DATA OFF CACHE BOOL "" FORCE)            # Disable Data support
set(ENABLE_DATA_MYSQL OFF CACHE BOOL "" FORCE)      # Disable MySQL Data support
set(ENABLE_DATA_POSTGRESQL OFF CACHE BOOL "" FORCE) # Disable PostgreSQL Data support
set(ENABLE_DATA_ODBC OFF CACHE BOOL "" FORCE)       # Disable ODBC Data support
set(ENABLE_APACHECONNECTOR OFF CACHE BOOL "" FORCE)  # Disable Apache Connector support
set(ENABLE_MONGODB OFF CACHE BOOL "" FORCE)         # Disable MongoDB support
set(ENABLE_REDIS OFF CACHE BOOL "" FORCE)           # Disable Redis support
set(ENABLE_PROMETHEUS OFF CACHE BOOL "" FORCE)      # Disable Prometheus support
set(ENABLE_PDF OFF CACHE BOOL "" FORCE)             # Disable PDF support
set(ENABLE_FUZZING OFF CACHE BOOL "" FORCE)         # Disable Fuzzing support
set(ENABLE_SAMPLES OFF CACHE BOOL "" FORCE)         # Disable Samples
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)           # Disable Tests
set(ENABLE_POCODOC OFF CACHE BOOL "" FORCE)         # Disable Poco Documentation Generator
set(ENABLE_XML OFF CACHE BOOL "" FORCE)             # Disable XML support
set(ENABLE_JSON OFF CACHE BOOL "" FORCE)            # Disable JSON support
set(ACTIVE_RECORD OFF CACHE BOOL "" FORCE)          # Disable Active Record support


# Enable required modules
set(ENABLE_UTIL ON CACHE BOOL "" FORCE)             # Enable Util support
set(ENABLE_NET ON CACHE BOOL "" FORCE)              # Enable Net support
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
    RUNTIME_OUTPUT_DIRECTORY "../bin"
)