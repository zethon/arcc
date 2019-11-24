project(tests)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set(ARCC_FILES
    ../arcc/CommandHistory.cpp
    ../arcc/Settings.cpp
    ../arcc/SimpleArgs.cpp
    ../arcc/utils.cpp
)

set(UTILS_FILES
    TestUtils.cpp
)

add_executable(TestUtils
    main.cpp
    ${ARCC_FILES}
    ${UTILS_FILES}
)

target_link_libraries(TestUtils
    PUBLIC
        ${CONAN_LIBS}
        "$<$<CONFIG:DEBUG>:${COVERAGE_FLAG}>"
)

add_test(NAME TestUtils
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestUtils
)

if (BUILD_SESSION_TESTS)
    add_definitions(-D_SESSION_FILE="${CMAKE_SOURCE_DIR}/tests/session.dat")

    set(SESSION_FILES
        TestSession.cpp
        ../arcc/ConsoleApp.cpp
        ../arcc/Listing.cpp
        ../arcc/Reddit.cpp
        ../arcc/WebClient.cpp
        ../arcc/OAuth2Login.cpp
    )

    if (WIN32)
        list(APPEND SESSION_FILES ../arcc/TerminalWindows.cpp)
    elseif (UNIX)
        list(APPEND SESSION_FILES ../arcc/TerminalPosix.cpp)
    endif()

    add_executable(TestSession
        main.cpp
        ${ARCC_FILES}
        ${SESSION_FILES}
    )

    target_link_libraries(TestSession
        ${CONAN_LIBS}
    )

    add_test(NAME TestSession
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestSession
    )
endif (BUILD_SESSION_TESTS)
