# SPDX-FileComment: CMakeLists.txt for tests
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file tests/CMakeLists.txt
# @brief Tests directory build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

find_package(Catch2 3 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(valijson REQUIRED)

# --- Dummy Plugin ---
add_library(dummy_plugin MODULE dummy_plugin.cpp)
target_include_directories(dummy_plugin PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(dummy_plugin PRIVATE nlohmann_json::nlohmann_json)

# --- Unit Tests ---
add_executable(unit_tests
    test_main.cpp
    test_validator.cpp
    test_plugin_loader.cpp
    test_config_manager.cpp
    test_uploader.cpp
)

target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    cgui_core
)

# Add a define for the plugin path to use in tests
target_compile_definitions(unit_tests PRIVATE
    DUMMY_PLUGIN_PATH="$<TARGET_FILE:dummy_plugin>"
)

include(Catch)
catch_discover_tests(unit_tests)
