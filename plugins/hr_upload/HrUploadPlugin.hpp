/**
 * SPDX-FileComment: HR Upload Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrUploadPlugin.hpp
 * @brief Implementation of HR-specific upload plugin with Cority Auth flow.
 * @version 1.3.0
 * @date 2026-05-17
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_HR_UPLOAD_PLUGIN_HPP
#define CGUI_HR_UPLOAD_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <chrono>
#include <optional>

namespace cgui {

/**
 * @class HrUploadPlugin
 * @brief Handles HR-specific payload formatting and complex Cority authentication flow.
 */
class HrUploadPlugin : public IUploadPlugin {
public:
    HrUploadPlugin() = default;
    ~HrUploadPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.3.0"; }
    std::expected<void, std::string> upload(const nlohmann::json& data) override;
    void shutdown() override;

private:
    nlohmann::json m_config;
    std::string m_base_url;
    std::string m_login;
    std::string m_password;
    
    // Auth state
    std::string m_refresh_token;
    std::string m_access_token;
    std::chrono::system_clock::time_point m_access_expiry;

    // SSL Settings
    bool m_verify_ssl = true;
    std::string m_ssl_ca_path;
    std::string m_proxy;
    long m_upload_timeout = 60;

    /**
     * @brief Sets common CURL options including SSL settings.
     */
    void setup_curl_common(void* curl_handle);

    /**
     * @brief Ensures a valid access token is available.
     */
    std::expected<void, std::string> ensure_authenticated();

    /**
     * @brief Step 1: POST /api/refreshtoken
     */
    std::expected<void, std::string> perform_refresh();

    /**
     * @brief Step 2: GET /api/token/
     */
    std::expected<void, std::string> fetch_access_token();

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace cgui

#endif // CGUI_HR_UPLOAD_PLUGIN_HPP
