# WinSplit Revolution Test Suite
# Uses established testing frameworks

cmake_minimum_required(VERSION 3.14)
project(WinSplitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only build on Windows
if(NOT WIN32)
    message(FATAL_ERROR "WinSplit tests only run on Windows")
endif()

# ============================================================
# Google Test Integration (Industry Standard)
# ============================================================
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ============================================================
# Catch2 Alternative (simpler, header-only)
# ============================================================
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# ============================================================
# Common Windows Libraries
# ============================================================
set(WIN_LIBS
    user32.lib
    kernel32.lib
    dwmapi.lib
    psapi.lib
    shlwapi.lib
    shell32.lib
    wininet.lib
    ws2_32.lib
    shcore.lib
)

# ============================================================
# Security Tests (using Google Test)
# ============================================================
add_executable(security_tests
    security/gtest_hook_spoofing.cpp
    security/gtest_dll_security.cpp
    security/gtest_xml_injection.cpp
    security/gtest_http_updates.cpp
)
target_link_libraries(security_tests PRIVATE GTest::gtest_main ${WIN_LIBS})
target_include_directories(security_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================
# Functional Tests (using Catch2)
# ============================================================
add_executable(functional_tests
    functional/catch2_window_positioning.cpp
    functional/catch2_multimonitor.cpp
    functional/catch2_dpi_awareness.cpp
)
target_link_libraries(functional_tests PRIVATE Catch2::Catch2WithMain ${WIN_LIBS})
target_include_directories(functional_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================
# Stress Tests
# ============================================================
add_executable(stress_tests
    stress/test_rapid_hotkeys.cpp
    stress/test_memory_leaks.cpp
)
target_link_libraries(stress_tests PRIVATE GTest::gtest_main ${WIN_LIBS})
target_include_directories(stress_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ============================================================
# Test Utilities
# ============================================================
add_executable(mock_window tools/mock_window.cpp)
target_link_libraries(mock_window PRIVATE user32.lib kernel32.lib)

add_executable(message_spoofer tools/message_spoofer.cpp)
target_link_libraries(message_spoofer PRIVATE user32.lib kernel32.lib)

# ============================================================
# CTest Integration
# ============================================================
include(GoogleTest)
gtest_discover_tests(security_tests)
gtest_discover_tests(stress_tests)

include(Catch)
catch_discover_tests(functional_tests)

# ============================================================
# Custom Targets
# ============================================================
add_custom_target(run_security_tests
    COMMAND $<TARGET_FILE:security_tests>
    DEPENDS security_tests
    COMMENT "Running security tests..."
)

add_custom_target(run_functional_tests
    COMMAND $<TARGET_FILE:functional_tests>
    DEPENDS functional_tests
    COMMENT "Running functional tests..."
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS security_tests functional_tests stress_tests
    COMMENT "Running all tests..."
)
