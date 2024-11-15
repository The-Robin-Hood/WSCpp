function(configure_platform_specific_linking TARGET)
    if(WIN32)
        log_status("Configuring Windows-specific linking")
        target_link_libraries(${TARGET} PRIVATE
            OpenSSL::SSL OpenSSL::Crypto
            Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
            glfw opengl32 freetype spdlog::spdlog
        )

        file(GLOB OPENSSL_DLLS "${OPENSSL_ROOT_DIR}/*.dll")
        log_status("OpenSSL DLLs to copy: ${OPENSSL_DLLS}")
        add_custom_command(
            TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${OPENSSL_DLLS}
            $<TARGET_FILE_DIR:${TARGET}>
            COMMENT "Copying OpenSSL DLLs to build directory"
        )
    elseif(APPLE)
        log_status("Configuring macOS-specific linking")
        target_link_libraries(${TARGET} PRIVATE
            OpenSSL::SSL OpenSSL::Crypto
            Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
            glfw freetype "-framework OpenGL" spdlog::spdlog
        )
    else()
        log_status("Configuring Linux-specific linking")
        find_package(OpenGL REQUIRED)
        target_link_libraries(${TARGET} PRIVATE
            OpenSSL::SSL OpenSSL::Crypto
            Poco::Foundation Poco::Net Poco::NetSSL Poco::Crypto
            glfw freetype OpenGL::GL spdlog::spdlog
        )
    endif()
endfunction()