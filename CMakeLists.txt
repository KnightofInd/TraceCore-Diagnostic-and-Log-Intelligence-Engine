cmake_minimum_required(VERSION 3.16)

project(cpp_log_analyzer
    VERSION 0.1.0
    DESCRIPTION "Offline C++ diagnostic log analyzer"
    LANGUAGES CXX
)

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(log_analyzer_core
    src/CommandLine.cpp
    src/LogParser.cpp
    src/LogReader.cpp
    src/LogAnalyzer.cpp
    src/ReportFormatter.cpp
    src/LogSeverity.cpp
)
target_include_directories(log_analyzer_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if (MSVC)
    target_compile_options(log_analyzer_core PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(log_analyzer_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

add_executable(log_analyzer
    src/main.cpp
)
target_link_libraries(log_analyzer PRIVATE log_analyzer_core)

if (MSVC)
    target_compile_options(log_analyzer PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(log_analyzer PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

if (ENABLE_SANITIZERS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(log_analyzer_core PUBLIC
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(log_analyzer_core INTERFACE
            -fsanitize=address,undefined
        )
        target_compile_options(log_analyzer PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(log_analyzer PRIVATE
            -fsanitize=address,undefined
        )
    else()
        message(FATAL_ERROR
            "ENABLE_SANITIZERS requires a GCC or Clang compiler")
    endif()
endif()

if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
endif()

add_subdirectory(tests)
