include(FetchContent)

find_package(GTest CONFIG QUIET)

if (NOT GTest_FOUND)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(log_analyzer_tests
    test_log_parser.cpp
    test_log_reader.cpp
    test_log_analyzer.cpp
    test_report_formatter.cpp
    test_command_line.cpp
)
target_link_libraries(log_analyzer_tests
    PRIVATE
        log_analyzer_core
        GTest::gtest_main
)

if (MSVC)
    target_compile_options(log_analyzer_tests PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(log_analyzer_tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

if (ENABLE_SANITIZERS)
    target_compile_options(log_analyzer_tests PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(log_analyzer_tests PRIVATE
        -fsanitize=address,undefined
    )
endif()

include(GoogleTest)
gtest_discover_tests(log_analyzer_tests)

add_test(
    NAME CliHelp
    COMMAND log_analyzer --help
)
set_tests_properties(CliHelp PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage: log_analyzer"
)

add_test(
    NAME CliSampleReport
    COMMAND log_analyzer --input "${CMAKE_CURRENT_SOURCE_DIR}/../samples/sample.log"
)
set_tests_properties(CliSampleReport PROPERTIES
    PASS_REGULAR_EXPRESSION "Total records: 5"
)
