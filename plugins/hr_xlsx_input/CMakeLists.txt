# SPDX-FileComment: CMakeLists.txt for HR XLSX Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0

find_package(nlohmann_json REQUIRED)
find_package(OpenXLSX REQUIRED)

add_library(hr_xlsx_input MODULE
    XlsxInputPlugin.cpp
    XlsxInputPlugin.hpp
)

target_include_directories(hr_xlsx_input PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(hr_xlsx_input PRIVATE
    nlohmann_json::nlohmann_json
    OpenXLSX::OpenXLSX
)

set_target_properties(hr_xlsx_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
