project(arcc)

set(SOURCE_FILES
    arcc.cpp
)

set(HEADER_FILES
    arcc.h
    core.h
    Console.h
)

if (WIN32)
    list(APPEND SOURCE_FILES ConsoleWindows.cpp)
elseif (UNIX)
    list(APPEND SOURCE_FILES ConsolePosix.cpp)
endif()

add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(${PROJECT_NAME}
    ${CURL_LIBRARIES})
