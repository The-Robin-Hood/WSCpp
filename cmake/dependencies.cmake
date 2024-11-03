function(configure_openssl)
    if(WIN32 OR APPLE)
        if(NOT OPENSSL_ROOT_DIR)
            log_error("OpenSSL root dir not found. Please set OPENSSL_ROOT_DIR.")
        else()
            log_status("OpenSSL root dir: ${OPENSSL_ROOT_DIR}")
        endif()
    endif()

    find_package(OpenSSL REQUIRED)
    if(OpenSSL_FOUND)
        log_status("OpenSSL version: ${OPENSSL_VERSION}")
    else()
        log_error("OpenSSL not found!")
    endif()
endfunction()

#==================================

function(configure_poco)
    log_status("Configuring Poco library")
    
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
        log_status("Enabled Poco module: ${MODULE}")
    endforeach()
endfunction()

#==================================

function(configure_glfw)
    log_status("Configuring GLFW library")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
endfunction()

#==================================