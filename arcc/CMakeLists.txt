project(arcc)

set(SOURCE_FILES
    arcc.cpp
    CommandHistory.cpp
    ConsoleApp.cpp
    Listing.cpp
    RedditSession.cpp
    Settings.cpp
    utils.cpp
    SimpleArgs.cpp
    WebClient.cpp
    OAuth2Login.cpp
)

set(HEADER_FILES
    AppBase.h
    CommandHistory.h
    ConsoleApp.h
    core.h
    Listing.h
    RedditSession.h
    Settings.h
    Terminal.h
    utils.h
    SimpleArgs.h
    WebClient.h
    OAuth2Login.h
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

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${CONAN_LIBS}
        ${EXTRA_LIBS}
        simple-web-server
        "$<$<CONFIG:DEBUG>:${COVERAGE_FLAG}>"
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
