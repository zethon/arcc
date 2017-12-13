cmake_minimum_required(VERSION 3.0)

project(arcc)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MACOSX_RPATH 1)

if (UNIX)
    if (APPLE)
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -ffunction-sections -fdata-sections -ftemplate-depth-256 -Wno-inconsistent-missing-override")
         set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -Wl,-dead_strip")
    endif (APPLE)
endif (UNIX)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Werror=sign-compare
        -Werror=unused-macros
        -Werror=unused-function
        -Werror=unused-label
        -Werror=unused-value
        -Werror=unused-variable
        -Werror=empty-body
        -Werror=uninitialized
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Werror=bool-conversion
        -Werror=enum-conversion
        -Werror=tautological-compare
    )
endif()

if (MSVC)
	add_definitions(
        -D_SCL_SECURE_NO_WARNINGS
    )
endif()

find_package(Boost REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS}) 

add_subdirectory(src)
