
add_library(utils
    empty.cpp
)
target_include_directories(utils PUBLIC .. .)
target_link_libraries(utils picojson)

add_compile_definitions(PICOJSON_USE_INT64)

if (EMSCRIPTEN)
else()
    add_subdirectory(test)
endif(EMSCRIPTEN)
