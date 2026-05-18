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

    /**
     * @brief Configure SSL settings.
     * @param verify_ssl Whether to verify SSL certificates.
     * @param ca_path Path to a CA bundle file.
     */
    void configure_ssl(bool verify_ssl, const std::string& ca_path = "");

    /**
     * @brief Set the proxy configuration.
     * @param proxy_string Proxy string (e.g., "user:pass@proxy:8080").
     */
    void set_proxy(const std::string& proxy_string);

    /**
     * @brief Set the upload timeout.
     * @param timeout_seconds Timeout in seconds.
     */
    void set_timeout(long timeout_seconds);

private:
    bool m_verify_ssl = true;
    std::string m_ssl_ca_path;
    std::string m_proxy;
    long m_timeout = 60;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace cgui

#endif // C_GUI_UPLOADER_HPP
