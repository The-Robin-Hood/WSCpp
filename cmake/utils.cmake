function(setup_console_colors)
    if(WIN32)
        set(Green       "")
        set(Yellow      "")
        set(Red         "")
        set(ColourReset "")
    else()
        string(ASCII 27 Esc)
        set(Green       "${Esc}[32m" PARENT_SCOPE)
        set(Yellow      "${Esc}[33m" PARENT_SCOPE)
        set(Red         "${Esc}[31m" PARENT_SCOPE)
        set(ColourReset "${Esc}[0m" PARENT_SCOPE)
    endif()
endfunction()

#==================================

function(log_status MESSAGE)
    message(STATUS "${Green}${MESSAGE}${ColourReset}")
endfunction()

function(log_warning MESSAGE)
    message(WARNING "${Yellow}${MESSAGE}${ColourReset}")
endfunction()

function(log_error MESSAGE)
    message(FATAL_ERROR "${Red}${MESSAGE}${ColourReset}")
endfunction()

#==================================

function(configure_assets TARGET)
    set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    set(ASSETS_DEST_DIR "$<TARGET_FILE_DIR:${TARGET}>/assets")
    
    add_custom_command(
        TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR}
        ${ASSETS_DEST_DIR}
        COMMENT "Copying assets to build directory"
    )
    log_status("Configured asset copying from ${ASSETS_DIR}")
endfunction()