cmake_minimum_required (VERSION 3.20.3)
project (MyLU)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")


# for clangd and other clang tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  fmt
  UPDATE_DISCONNECTED 0,
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG
)

FetchContent_MakeAvailable(fmt)


FetchContent_Declare(
  googletest
  UPDATE_DISCONNECTED 0
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)

# google tests do not build cleanly.  Disable some warnings.
set_property(
    DIRECTORY ${googletest_SOURCE_DIR}
    APPEND
    PROPERTY COMPILE_OPTIONS -Wno-undef -Wno-maybe-uninitialized
)

FetchContent_MakeAvailable(googletest)

add_subdirectory (olc)
add_subdirectory (model)


