cmake_minimum_required(VERSION 3.20)

project(arcc)
set(CMAKE_CXX_STANDARD 20)

# initialize conan libs
include(CMake/conan.cmake)
conan_cmake_configure(
    REQUIRES
        boost/1.84.0
        libcurl/8.6.0
        fmt/9.1.0
        nlohmann_json/3.11.2
    GENERATORS
        cmake_find_package
        cmake_paths
    OPTIONS
        boost:shared=False
        boost:without_test=False
        boost:without_thread=False
        boost:without_filesystem=False
        boost:without_system=False
        boost:without_stacktrace=False
        boost:without_program_options=False
        boost:without_math=True
        boost:without_wave=True
        boost:without_container=False
        boost:without_contract=True
        boost:without_exception=False
        boost:without_graph=True
        boost:without_iostreams=True
        boost:without_locale=True
        boost:without_log=True
        boost:without_random=True
        boost:without_regex=True
        boost:without_mpi=True
        boost:without_serialization=False
        boost:without_coroutine=True
        boost:without_fiber=True
        boost:without_context=True
        boost:without_timer=True
        boost:without_chrono=False
        boost:without_date_time=False
        boost:without_atomic=False
        boost:without_graph_parallel=True
        boost:without_python=True
        boost:without_type_erasure=True
    )

conan_cmake_autodetect(settings)

conan_cmake_install(
    PATH_OR_REFERENCE  .
    BUILD  missing
    REMOTE conancenter
    SETTINGS ${settings}
)

# optional configuration
option(BUILD_ARCC_TESTS "Build unit tests (default OFF)" OFF)
option(BUILD_SESSION_TESTS "Build Sessions Tests (default OFF)" OFF)
option(BUILD_CODE_COVERAGE "Enable coverage reporting" OFF)

# Global definitions
add_definitions(
    -DCURL_STATICLIB
    -DBOOST_TEST_NO_LIB
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    -DFMT_HEADER_ONLY
)

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

    if (BUILD_CODE_COVERAGE)
        add_compile_options(
            "$<$<CONFIG:DEBUG>:--coverage>"
        )

        # travis-ci still uses an old cmake which does not support
        # add_link_options, so we set a variable here and then
        # append it to each build target manually
        set(COVERAGE_FLAG "--coverage")
    endif(BUILD_CODE_COVERAGE)
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
        # -Werror=deprecated-declarations
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
        /wd4099
    )
endif(MSVC)

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)



include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
find_package(CURL 7.88.1 REQUIRED)
find_package(fmt 9.1.0 REQUIRED)
find_package(nlohmann_json 3.11.2 REQUIRED)

include_directories(${nlohmann_json_INCLUDE_DIRS})
include_directories(${fmt_fmt_INCLUDE_DIRS})

add_subdirectory(Simple-Web-Server)
include_directories(Simple-Web-Server)

add_subdirectory(rang)
include_directories(rang/include)

add_subdirectory(arcc)

if (BUILD_ARCC_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_ARCC_TESTS)
