# SPDX-FileComment: Top-level CMakeLists.txt for the c-gui project
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file CMakeLists.txt
# @brief Main build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

cmake_minimum_required(VERSION 3.28)

project(c-gui
    VERSION 1.0.0
    DESCRIPTION "C++23 desktop application for data validation and upload"
    HOMEPAGE_URL "https://github.com/Zheng-Bote/c-gui"
    LANGUAGES CXX
)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Options ---
option(BUILD_TESTING "Build tests" ON)

# --- Conan Integration ---
# When using Conan v2 with CMakeToolchain, we just use find_package()
# We expect Conan to generate the necessary config files.

# --- Global Compiler Flags ---
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# --- Configure Variables ---
set(EXECUTABLE_NAME "${PROJECT_NAME}")
set(PROG_LONGNAME "${PROJECT_DESCRIPTION}")
set(PROG_CREATED "2025")
set(PROG_COPYRIGHT "ZHENG Robert")
set(PROG_LICENSE "Apache-2.0")
set(PROG_AUTHOR "ZHENG Robert")
set(PROG_ORGANIZATION_NAME "ZHENG Robert")
set(PROG_ORGANIZATION_DOMAIN "net.hase-zheng")
set(CMAKE_QTV "N/A")

# --- Subdirectories ---
add_subdirectory(configure)
add_subdirectory(src)
add_subdirectory(plugins)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
