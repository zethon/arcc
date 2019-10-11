project(tests)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set(ARCC_FILES
    ../arcc/CommandHistory.cpp
    ../arcc/SimpleArgs.cpp
    ../arcc/utils.cpp
)

set(UTILS_TESTS
    UtilsTests.cpp
)

add_executable(TestUtils
    main.cpp
    ${ARCC_FILES}
    ${UTILS_TESTS}
)

target_link_libraries(TestUtils
    ${CONAN_LIBS}
)

add_test(NAME TestUtils
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestUtils
)
