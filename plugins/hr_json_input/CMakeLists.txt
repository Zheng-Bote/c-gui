# SPDX-FileComment: CMakeLists.txt for HR JSON Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/hr_json_input/CMakeLists.txt
# @brief JSON input plugin build configuration
# @version 1.0.0
# @date 2026-05-15
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2026 ZHENG Robert
# @license Apache-2.0

add_library(hr_json_input SHARED HrJsonInputPlugin.cpp HrJsonInputPlugin.hpp)

target_include_directories(hr_json_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)
target_link_libraries(hr_json_input PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(hr_json_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
