# SPDX-FileComment: CMakeLists.txt for CSV Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/csv_input/CMakeLists.txt
# @brief CSV input plugin build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

add_library(hr_csv_input SHARED CsvInputPlugin.cpp CsvInputPlugin.hpp)

target_include_directories(hr_csv_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)
target_link_libraries(hr_csv_input PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(hr_csv_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
