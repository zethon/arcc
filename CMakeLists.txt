project(arcc)

cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)

# initialize conan libs
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)

# optional configuration
option(BUILD_ARCC_TESTS "Build unit tests (default OFF)" OFF)
option(BUILD_SESSION_TESTS "Build Sessions Tests (default OFF)" OFF)

# Global definitions
add_definitions(
    -DCURL_STATICLIB
    -DBOOST_TEST_NO_LIB
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
)

# Code Coverage Configuration
# add_library(COVERAGE_CONFIG INTERFACE)

# option(CODE_COVERAGE "Enable coverage reporting" OFF)
# if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#   # Add required flags (GCC & LLVM/Clang)
#   target_compile_options(COVERAGE_CONFIG INTERFACE
#     -O0        # no optimization
#     -g         # generate debug info
#     --coverage # sets all required flags
#   )
#   if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
#     target_link_options(COVERAGE_CONFIG INTERFACE --coverage)
#   else()
#     target_link_libraries(COVERAGE_CONFIG INTERFACE --coverage)
#   endif()
# endif(CODE_COVERAGE)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)

    add_compile_options(
        # remove dead code
        -ffunction-sections
        -fdata-sections

        # enforced errors
        -Werror=format
        -Werror=trigraphs
        -Werror=missing-braces
        -Werror=parentheses
        -Werror=return-type
        -Werror=sign-compare
        -Werror=unused-macros
        -Werror=unused-function
        -Werror=unused-label
        -Werror=unused-value
        -Werror=unused-variable
        -Werror=empty-body
        -Werror=reorder
        -Werror=switch
        -Werror=int-conversion
        -Werror=uninitialized
        -Werror=char-subscripts
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=non-virtual-dtor>

        # disabled errors/warnings
        -Wno-error=date-time
    )

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
    # add_link_options(
    #     --coverage
    # )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)

    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>

        -Wno-\#pragma-messages
        -Wno-unused-local-typedef
        -Werror=tautological-compare
        -Werror=four-char-constants
        -Werror=constant-conversion
        -Werror=bool-conversion
        -Werror=enum-conversion
        -Werror=deprecated-declarations
        -Werror=return-stack-address

        $<$<COMPILE_LANGUAGE:CXX>:-Werror=overloaded-virtual>
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=unused-private-field>
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=mismatched-tags>
    )
endif()

if (MSVC)
    add_definitions(
        -DWIN32
        -D_WIN32
        -D_WINDOWS
        -DNOMINMAX
        -D_SCL_SECURE_NO_WARNINGS
        -D_WIN32_WINNT=0x0600
        -DWIN32_LEAN_AND_MEAN

        # use this to supress the boost generated "warning C4996"
        # on Windows
        -D_SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING
    )

    add_compile_options(
        # treat warnings as errors
        /WX
        /wd4996
    )
endif(MSVC)

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)

add_subdirectory(Simple-Web-Server)
include_directories(Simple-Web-Server)

add_subdirectory(rang)
include_directories(rang/include)

add_subdirectory(arcc)

if (BUILD_ARCC_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_ARCC_TESTS)
