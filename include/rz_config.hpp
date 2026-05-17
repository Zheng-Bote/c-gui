/**
 * SPDX-FileComment: Project Configuration template
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file rz_config.hpp.in
 * @brief Configuration template for CMake.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#pragma once

#include <string_view>
#include <cstdint>

namespace rz {
namespace config {
constexpr std::string_view PROJECT_NAME = "c-gui";
constexpr std::string_view PROG_LONGNAME = "C++23 desktop application for data validation and upload";
constexpr std::string_view PROJECT_DESCRIPTION = "C++23 desktop application for data validation and upload";

constexpr std::string_view EXECUTABLE_NAME = "c-gui";

constexpr std::string_view VERSION = "0.3.0";
constexpr std::int32_t PROJECT_VERSION_MAJOR { 0 };
constexpr std::int32_t PROJECT_VERSION_MINOR { 3 };
constexpr std::int32_t PROJECT_VERSION_PATCH { 0 };

constexpr std::string_view PROJECT_HOMEPAGE_URL = "https://github.com/Zheng-Bote/c-gui";
constexpr std::string_view AUTHOR = "ZHENG Robert";

constexpr std::string_view CREATED_YEAR = "2025";
constexpr std::string_view COPYRIGHT = "ZHENG Robert";
constexpr std::string_view LICENSE = "Apache-2.0";

constexpr std::string_view ORGANIZATION = "ZHENG Robert";
constexpr std::string_view PROJECT_DOMAIN = "net.hase-zheng";

constexpr std::string_view CMAKE_CXX_STANDARD = "c++23";
constexpr std::string_view CMAKE_CXX_COMPILER =
    "MSVC 19.44.35227.0";
constexpr std::string_view QT_VERSION_BUILD = "N/A";
} // namespace config
} // namespace rz
