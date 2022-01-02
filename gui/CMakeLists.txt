add_executable(illum
    Illum.cpp
    illum_main.cpp
)
target_include_directories(illum PUBLIC .)
target_link_libraries(illum levels solver model fmt common olc)

add_custom_command(TARGET illum PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E
                   copy ${CMAKE_SOURCE_DIR}/gui/resources/* $<TARGET_FILE_DIR:illum>/..)

if (EMSCRIPTEN)
    set_target_properties(illum PROPERTIES
       LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s USE_LIBPNG=1 \
                   -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2 \
                   --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/resources/hint.png@hint.png \
                   --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/resources/restart.png@restart.png \
                   --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/resources/RetroMenu.png@RetroMenu.png \
                   --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/resources/undo.png@undo.png \
                   "
    )
endif(EMSCRIPTEN)



