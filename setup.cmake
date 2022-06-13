set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# useful for sanitize debugging
if (ASAN)
  message("=== DEBUG ADDRESS SANITIZER ENABLED ===")
  set(CMAKE_BUILD_TYPE "Debug")
  set(ASAN_ARGS "-fno-omit-frame-pointer -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${ASAN_ARGS}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${ASAN_ARGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${ASAN_ARGS}")
#  ADD_DEFINITIONS(${ASAN_ARGS})
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

include(CTest)
include(FetchContent)

FetchContent_Declare(
  picojson
  UPDATE_DISCONNECTED 0,
  GIT_REPOSITORY https://github.com/cuzdav/picojson.git
  GIT_TAG
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
FetchContent_MakeAvailable(picojson)

add_library(picojson INTERFACE)
target_include_directories(picojson INTERFACE ${picojson_SOURCE_DIR})


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


# Only supports json schama up to revision 7, but I'd like the 2020 or
# later, so for now will write a hand-made schema validator
#FetchContent_Declare(
#  valijson
#  UPDATE_DISCONNECTED 0,
#  GIT_REPOSITORY https://github.com/tristanpenman/valijson.git
#  GIT_TAG 4d603df4333b17e5309c368c5b614bb881a16b35
#)
#FetchContent_MakeAvailable(valijson)


# google tests do not build cleanly.  Disable some warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set (GTEST_COMPILE_OPTS -Wno-undef)
else()
  set (GTEST_COMPILE_OPTS -Wno-undef -Wno-maybe-uninitialized)
endif()

set_property(
    DIRECTORY ${googletest_SOURCE_DIR}
    APPEND
    PROPERTY COMPILE_OPTIONS ${GTEST_COMPILE_OPTS}
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



