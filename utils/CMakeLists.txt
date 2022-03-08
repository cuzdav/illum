
add_library(utils
    jsonschema.cpp
)
target_include_directories(utils PUBLIC .. .)
target_link_libraries(utils valijson)

add_compile_definitions(PICOJSON_USE_INT64)
