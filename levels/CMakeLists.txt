add_library(levels
    BasicWallLayout.cpp
    LevelCreator.cpp
)
target_include_directories(levels PUBLIC .)

target_link_libraries(levels model)


#add_subdirectory(test)
