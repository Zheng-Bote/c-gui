/**
 * SPDX-FileComment: Uploader for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file Uploader.hpp
 * @brief Handles HTTP uploads using libcurl.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_UPLOADER_HPP
#define C_GUI_UPLOADER_HPP

#include <string>
#include <functional>
#include <expected>
#include <thread>
#include <nlohmann/json.hpp>

namespace cgui {

/**
 * @brief Handles synchronous and asynchronous uploads.
 */
class Uploader {
public:
    Uploader();
    ~Uploader();

    /**
     * @brief Synchronous upload.
     * @param url The target URL.
     * @param payload The JSON payload.
     * @param bearer_token Optional bearer token for authentication.
     * @return Success or error message.
     */
    [[nodiscard]] std::expected<void, std::string> upload_sync(const std::string& url, const nlohmann::json& payload, const std::string& bearer_token = "");

    /**
     * @brief Asynchronous upload.
     * @param url The target URL.
     * @param payload The JSON payload.
     * @param callback Callback function called when upload completes.
     * @param bearer_token Optional bearer token for authentication.
     */
    void upload_async(const std::string& url, const nlohmann::json& payload, 
                      std::function<void(std::expected<void, std::string>)> callback,
                      const std::string& bearer_token = "");

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace cgui

#endif // C_GUI_UPLOADER_HPP
