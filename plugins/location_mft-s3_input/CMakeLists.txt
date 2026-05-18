# SPDX-FileComment: CMakeLists.txt for Location S3 Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0

add_library(location_mft-s3_input MODULE
    LocS3InputPlugin.cpp
    LocS3InputPlugin.hpp
)

target_include_directories(location_mft-s3_input PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(location_mft-s3_input PRIVATE
    nlohmann_json::nlohmann_json
)

set_target_properties(location_mft-s3_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
