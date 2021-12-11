add_library(olc STATIC
    olcPixelGameEngine.cpp
    olcRetroMenu.cpp
)
target_include_directories(olc PUBLIC .)

set(THREADS_PREFER_PTHREAD_FLAD ON)
find_package(Threads REQUIRED)

find_package(OpenGL REQUIRED)

if (UNIX)
    find_package(X11 REQUIRED)
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
endif (UNIX)

if (WIN32)
    include_directories(${WinSDK})
endif (WIN32)

target_link_libraries(olc
    Threads::Threads
    OpenGL::OpenGL
    OpenGL::GL
    OpenGL::GLX
)

if (UNIX) 
    target_link_libraries(olc
        ${X11_LIBRARIES}
        PNG::PNG
    )
endif (UNIX)
if (WIN32)
    target_link_libraries(olc
        ${WinSDK}
    )
endif (WIN32)
