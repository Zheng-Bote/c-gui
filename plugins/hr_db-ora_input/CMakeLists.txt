# SPDX-FileComment: CMakeLists.txt for HR Oracle Input Plugin
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2026 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/hr_db-ora_input/CMakeLists.txt
# @brief Oracle input plugin build configuration
# @version 1.0.0
# @date 2026-05-15
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2026 ZHENG Robert
# @license Apache-2.0

add_library(hr_db-ora_input SHARED HrDbOraInputPlugin.cpp HrDbOraInputPlugin.hpp)

target_include_directories(hr_db-ora_input PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find nlohmann_json, soci and spdlog
find_package(nlohmann_json REQUIRED)
find_package(soci REQUIRED COMPONENTS core oracle)
find_package(spdlog REQUIRED)

target_include_directories(hr_db-ora_input PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${SOCI_INCLUDE_DIRS}
)

target_link_libraries(hr_db-ora_input PRIVATE 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    ${SOCI_LIBRARIES}
)

# Fallback: link specifically if variables are empty (some Conan generators)
if(NOT SOCI_LIBRARIES)
    target_link_libraries(hr_db-ora_input PRIVATE 
        soci::soci_core
        soci::soci_oracle
    )
endif()

set_target_properties(hr_db-ora_input PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX}
)
