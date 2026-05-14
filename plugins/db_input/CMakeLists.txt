# SPDX-FileComment: CMakeLists.txt for DB Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/db_input/CMakeLists.txt
# @brief PostgreSQL input plugin build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

add_library(db_input SHARED DbInputPlugin.cpp DbInputPlugin.hpp)

target_include_directories(db_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find dependencies
find_package(nlohmann_json REQUIRED)
find_package(libpqxx REQUIRED)

target_link_libraries(db_input PRIVATE 
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
)

set_target_properties(db_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
