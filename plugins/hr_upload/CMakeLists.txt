# SPDX-FileComment: CMakeLists.txt for HR Upload Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/hr_upload/CMakeLists.txt
# @brief HR upload plugin build configuration
# @version 1.0.0
# @date 2026-05-15
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2026 ZHENG Robert
# @license Apache-2.0

add_library(hr_upload SHARED HrUploadPlugin.cpp HrUploadPlugin.hpp)

target_include_directories(hr_upload PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find nlohmann_json and CURL
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

target_link_libraries(hr_upload PRIVATE nlohmann_json::nlohmann_json CURL::libcurl)

set_target_properties(hr_upload PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
