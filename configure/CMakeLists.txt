# SPDX-FileComment: CMakeLists.txt for configuration
# SPDX-FileType: SOURCE
# SPDX-FileContributor: ZHENG Robert
# SPDX-FileCopyrightText: 2025 ZHENG Robert
# SPDX-License-Identifier: Apache-2.0
#
# @file configure/CMakeLists.txt
# @brief Configuration directory build configuration
# @version 1.0.0
# @date 2025-02-13
#
# @author ZHENG Robert (robert@hase-zheng.net)
# @copyright Copyright (c) 2025 ZHENG Robert
# @license Apache-2.0

configure_file(
    "rz_config.hpp.in" "${PROJECT_SOURCE_DIR}/include/rz_config.hpp"
    ESCAPE_QUOTES @ONLY)
