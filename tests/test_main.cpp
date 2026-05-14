/**
 * SPDX-FileComment: Test main for Catch2
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file tests/test_main.cpp
 * @brief Test entry point
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Basic test", "[main]") {
    REQUIRE(1 == 1);
}
