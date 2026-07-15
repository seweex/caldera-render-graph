
find_package(Doxygen)

if(DOXYGEN_FOUND)
    message(STATUS "[Doxygen] Run docs generation target configuration")

    set(CALDERA_DOXYGEN_CSS_DIR "${CMAKE_SOURCE_DIR}/external/doxygen-awesome-css" CACHE INTERNAL "")

    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive external/doxygen-awesome-css
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cfg
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )

    add_custom_target(caldera-docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "[Doxygen] Generating docs..."
        VERBATIM
    )

    message(STATUS "[Doxygen] Docs generation target created successfully")
else()
    message(WARNING "[Doxygen] Doxygen not found! Docs generation skipped")
endif()