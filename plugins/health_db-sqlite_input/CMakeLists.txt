# SPDX-FileComment: CMakeLists.txt for Health SQLite Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0

add_library(health_db-sqlite_input MODULE
    HealthSqliteInputPlugin.cpp
    HealthSqliteInputPlugin.hpp
)

target_include_directories(health_db-sqlite_input PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(health_db-sqlite_input PRIVATE
    nlohmann_json::nlohmann_json
)

set_target_properties(health_db-sqlite_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
