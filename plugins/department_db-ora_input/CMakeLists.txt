# SPDX-FileComment: CMakeLists.txt for Department Oracle Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0

add_library(department_db-ora_input MODULE
    DeptOraInputPlugin.cpp
    DeptOraInputPlugin.hpp
)

target_include_directories(department_db-ora_input PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(department_db-ora_input PRIVATE
    nlohmann_json::nlohmann_json
)

set_target_properties(department_db-ora_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
