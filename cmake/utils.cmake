macro(setup_cmake_configs)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    endforeach(OUTPUTCONFIG)

    message(STATUS "Configured CMake output directories")
    message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    if(APPLE)
        enable_language(OBJCXX)
    endif()
endmacro()

function(setup_console_colors)
    if(WIN32)
        set(Green "")
        set(Yellow "")
        set(Red "")
        set(ColourReset "")
    else()
        string(ASCII 27 Esc)
        set(Green "${Esc}[32m" PARENT_SCOPE)
        set(Yellow "${Esc}[33m" PARENT_SCOPE)
        set(Red "${Esc}[31m" PARENT_SCOPE)
        set(ColourReset "${Esc}[0m" PARENT_SCOPE)
    endif()
endfunction()

function(log_status MESSAGE)
    message(STATUS "${Green}${MESSAGE}${ColourReset}")
endfunction()

function(log_warning MESSAGE)
    message(WARNING "${Yellow}${MESSAGE}${ColourReset}")
endfunction()

function(log_error MESSAGE)
    message(FATAL_ERROR "${Red}${MESSAGE}${ColourReset}")
endfunction()

function(configure_assets TARGET)
    set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    set(ASSETS_DEST_DIR "$<TARGET_FILE_DIR:${TARGET}>/assets")

    if(WIN32)
        file(GLOB OPENSSL_DLLS "${OPENSSL_ROOT_DIR}/*.dll")
        log_status("OpenSSL DLLs to copy: ${OPENSSL_DLLS}")
        add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${OPENSSL_DLLS}
            $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "Copying OpenSSL DLLs to build directory"
        )
    endif()

    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR}
        ${ASSETS_DEST_DIR}
        COMMENT "Copying assets to build directory"
    )
    log_status("Configured asset copying from ${ASSETS_DIR}")
endfunction()