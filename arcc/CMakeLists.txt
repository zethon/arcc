project(arcc)

set(SOURCE_FILES
    arcc.cpp
    CommandHistory.cpp
    ConsoleApp.cpp
    CursesApp.cpp
    Listing.cpp
    OAuth2Login.cpp
    Reddit.cpp
    Settings.cpp
    SimpleArgs.cpp
    utils.cpp
    WebClient.cpp
)

set(HEADER_FILES
    CommandHistory.h
    ConsoleApp.h
    core.h
    CursesApp.h
    Listing.h
    OAuth2Login.h
    Reddit.h
    Terminal.h
    utils.h
    WebClient.h
)

if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:library")
    list(APPEND SOURCE_FILES TerminalWindows.cpp)
elseif (UNIX)
    list(APPEND SOURCE_FILES TerminalPosix.cpp)
endif()

if (APPLE)
    set(EXTRA_LIBS ${APP_SERVICES_LIBRARY} ${CORE_SERVICES_LIBRARY})
endif()

add_executable(arcc
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(arcc
    PUBLIC
        ${CONAN_LIBS}
        ${EXTRA_LIBS}
        simple-web-server
        "$<$<CONFIG:DEBUG>:${COVERAGE_FLAG}>"
)

target_compile_definitions(arcc
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
