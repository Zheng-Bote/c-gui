# SPDX-FileComment: Top-level CMakeLists.txt for the c-gui project
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file CMakeLists.txt
# @brief Main build configuration
# @version 0.2.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

cmake_minimum_required(VERSION 3.28)

project(c-gui
    VERSION 0.1.0
    DESCRIPTION "C++23 desktop application for data validation and upload"
    HOMEPAGE_URL "https://github.com/Zheng-Bote/c-gui"
    LANGUAGES CXX
)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# --- Options ---
option(BUILD_TESTING "Build tests" ON)

# --- Dependencies via FetchContent ---
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)
include(FetchContent)

FetchContent_Declare(
  gh_update_checker
  GIT_REPOSITORY https://github.com/Zheng-Bote/gh-update-checker.git
  GIT_TAG        v1.0.9
)
FetchContent_MakeAvailable(gh_update_checker)

# --- Conan Integration ---
# When using Conan v2 with CMakeToolchain, we just use find_package()
# We expect Conan to generate the necessary config files.

# --- Global Compiler Flags ---
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()



# --- Subdirectories ---
add_subdirectory(configure)
add_subdirectory(src)
add_subdirectory(plugins)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/${PROJECT_NAME}_libs.txt" 
     CONTENT "$<TARGET_PROPERTY:${PROJECT_NAME},LINK_LIBRARIES>")
