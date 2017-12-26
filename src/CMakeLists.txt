project(arcc)

set(SOURCE_FILES
    arcc.cpp
    Reddit.cpp
    utils.cpp
    SimpleArgs.cpp
    WebClient.cpp
)

set(HEADER_FILES
    arcc.h
    core.h
    Reddit.h
    Terminal.h
    utils.h
    SimpleArgs.h
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

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME}
    ${CURL_LIBRARIES}
    ${EXTRA_LIBS}
    ${Boost_LIBRARIES}
    simple-web-server
)
