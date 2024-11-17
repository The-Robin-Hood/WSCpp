function(configure_openssl)
    if(WIN32)
        log_status("Configuring Pre-built OpenSSL library for Windows.")
        log_status("If you want to use a different version, set the OPENSSL_ROOT_DIR variable.")
        set(OPENSSL_ROOT_DIR "thirdparty/openssl")
    endif()

    find_package(OpenSSL REQUIRED QUIET)

    if(OpenSSL_FOUND)
        log_status("OpenSSL version: ${OPENSSL_VERSION}")
    else()
        log_error("OpenSSL not found! Please install it or set the OPENSSL_ROOT_DIR variable.")
    endif()
endfunction()

function(configure_poco)
    log_status("Configuring Poco library")
    set(BUILD_SHARED_LIBS NO CACHE BOOL "" FORCE)
    set(POCO_MT ON CACHE BOOL "" FORCE)
    add_definitions(-DPOCO_NO_AUTOMATIC_LIBS)

    function(disable_poco_modules)
        foreach(module ${ARGV})
            set(ENABLE_${module} OFF CACHE BOOL "" FORCE)
        endforeach()
    endfunction()

    log_status("Disabling unwanted Poco modules")
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

function(configure_glfw)
    log_status("Configuring GLFW library")
    set(GLFW_LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
endfunction()
