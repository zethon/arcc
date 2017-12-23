cmake_minimum_required(VERSION 3.0)

project(arcc)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MACOSX_RPATH 1)

if (UNIX)
    if (APPLE)
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -ffunction-sections -fdata-sections -ftemplate-depth-256 -Wno-inconsistent-missing-override")
         set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -Wl,-dead_strip")

         FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices)
         FIND_LIBRARY(CORE_SERVICES_LIBRARY CoreFoundation)
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

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

if (MSVC)
	add_definitions(
        -D_SCL_SECURE_NO_WARNINGS
        -DWIN32_LEAN_AND_MEAN
        -DCURL_STATICLIB
        -DNOMINMAX
    )

    list(APPEND CURL_LIBRARIES
        "Crypt32.lib"
        "Ws2_32.lib"
    )

    SET(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:library")
endif()

find_package(Boost COMPONENTS filesystem system thread REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS}) 

add_subdirectory(Simple-Web-Server)
include_directories(Simple-Web-Server)

# disable the unit tests for json project
set(JSON_BuildTests OFF CACHE BOOL "Build the unit tests")
add_subdirectory(json)
include_directories(json/src)

# disable the example projects for cxxopts
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "Set to ON to build examples")
add_subdirectory(cxxopts)
include_directories(cxxopts/include)

add_subdirectory(rang)
include_directories(rang/include)

add_subdirectory(src)
