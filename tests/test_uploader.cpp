/**
 * SPDX-FileComment: Unit tests for Uploader
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file tests/test_uploader.cpp
 * @brief Unit tests for the Uploader class
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include <catch2/catch_test_macros.hpp>
#include "Uploader.hpp"
#include <chrono>
#include <thread>

using namespace cgui;

TEST_CASE("Uploader handles invalid URL", "[Uploader]") {
    Uploader uploader;
    nlohmann::json payload = {{"test", "data"}};
    std::string auth_token = "some_token";

    SECTION("Synchronous upload fails with invalid URL") {
        auto result = uploader.upload_sync("https://invalid.example.com/api", payload, auth_token);
        REQUIRE(!result.has_value());
    }

    SECTION("Asynchronous upload returns error via callback") {
        bool callback_called = false;
        std::string error_received;

        uploader.upload_async("https://invalid.example.com/api", payload,
            [&](std::expected<void, std::string> result) {
                callback_called = true;
                if (!result.has_value()) {
                    error_received = result.error();
                }
            }, auth_token);

        // Wait a bit for the thread to finish
        int retries = 0;
        while (!callback_called && retries < 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            retries++;
        }

        REQUIRE(callback_called);
        REQUIRE(!error_received.empty());
    }
}
