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

add_library(hr_db-pg_input SHARED DbInputPlugin.cpp DbInputPlugin.hpp)

target_include_directories(hr_db-pg_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find dependencies
find_package(nlohmann_json REQUIRED)
find_package(libpqxx REQUIRED)
find_package(PostgreSQL REQUIRED)

target_link_libraries(hr_db-pg_input PRIVATE 
    nlohmann_json::nlohmann_json
    libpqxx::pqxx
    PostgreSQL::PostgreSQL
)

# Robust GSSAPI/Kerberos linking for Linux
if(UNIX AND NOT APPLE)
    find_library(LIB_GSSAPI gssapi_krb5)
    find_library(LIB_KRB5 krb5)
    find_library(LIB_K5CRYPTO k5crypto)
    find_library(LIB_COM_ERR com_err)

    if(LIB_GSSAPI AND LIB_KRB5)
        # We use --no-as-needed to ensure the linker doesn't discard these libraries
        # even if they aren't directly referenced in the plugin code (but needed by libpq).
        target_link_libraries(hr_db-pg_input PRIVATE 
            "-Wl,--no-as-needed"
            ${LIB_GSSAPI}
            ${LIB_KRB5}
            ${LIB_K5CRYPTO}
            ${LIB_COM_ERR}
            "-Wl,--as-needed"
        )
        message(STATUS "PostgreSQL plugin: Forced GSSAPI/Kerberos linkage.")
    endif()
endif()

set_target_properties(hr_db-pg_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
