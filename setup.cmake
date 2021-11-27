set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# useful for sanitize debugging
if (ASAN)
  message("=== DEBUG ADDRESS SANITIZER ENABLED ===")
  set(CMAKE_BUILD_TYPE "Debug")
  set(ASAN_ARGS "-fno-omit-frame-pointer -fsanitize=address")
  ADD_DEFINITIONS(${ASAN_ARGS})
endif()

if (DEBUGPROFILE)
  message("=== DEBUG PROFILE ENABLED ===")
  ADD_DEFINITIONS(-DDEBUGPROFILE)
endif()

if (DEBUGLOG)
    message("=== DEBUG LOG ENABLED ===")
    ADD_DEFINITIONS(-DDEBUG)
endif(DEBUGLOG)

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
  GIT_TAG        release-1.11.0
)

# google tests do not build cleanly.  Disable some warnings.
set_property(
    DIRECTORY ${googletest_SOURCE_DIR}
    APPEND
    PROPERTY COMPILE_OPTIONS -Wno-undef -Wno-maybe-uninitialized
)

macro(package_add_test TESTNAME)
    # create an exectuable in which the tests will be stored
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main)
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER test)
endmacro()
FetchContent_MakeAvailable(googletest)


set(BOOST_REQUESTED_VERSION 1.71)
find_package(Boost)


