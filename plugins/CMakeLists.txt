# SPDX-FileComment: CMakeLists.txt for plugins
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file plugins/CMakeLists.txt
# @brief Plugins directory build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

# Common plugin configuration
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

add_subdirectory(csv_input)
add_subdirectory(db_input)
add_subdirectory(kafka_input)
