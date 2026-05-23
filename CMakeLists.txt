# SPDX-FileComment: Top-level CMakeLists.txt for the c-gui project
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file CMakeLists.txt
# @brief Main build configuration
# @version 0.6.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

cmake_minimum_required(VERSION 3.28)

project(c-gui
    VERSION 0.6.0
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

# --- Output Directories ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --- RPATH Configuration (Linux) ---
if(UNIX AND NOT APPLE)
    set(CMAKE_SKIP_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # Force RPATH instead of RUNPATH so transitive dependencies can be found
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")
endif()

# --- Dependencies via FetchContent ---
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)
include(FetchContent)

FetchContent_Declare(
  gh_update_checker
  GIT_REPOSITORY https://github.com/Zheng-Bote/gh-update-checker.git
  GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(gh_update_checker)
if(NOT TARGET gh_update_checker::gh_update_checker)
    add_library(gh_update_checker::gh_update_checker ALIAS gh_update_checker)
endif()

# --- Conan Integration ---
# When using Conan v2 with CMakeToolchain, we just use find_package()
# We expect Conan to generate the necessary config files.

# --- Global Compiler Flags ---
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()



# --- Dependencies ---
find_package(wxWidgets REQUIRED COMPONENTS core base net xml)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(libsodium REQUIRED)
find_package(valijson REQUIRED)
find_package(spdlog REQUIRED)
find_package(OpenXLSX REQUIRED)
find_package(soci REQUIRED COMPONENTS core oracle)
find_package(RdKafka REQUIRED)
find_package(wasmtime REQUIRED)

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
