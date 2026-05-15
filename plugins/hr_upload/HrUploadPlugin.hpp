/**
 * SPDX-FileComment: HR Upload Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrUploadPlugin.hpp
 * @brief Implementation of HR-specific upload plugin.
 * @version 1.0.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_HR_UPLOAD_PLUGIN_HPP
#define CGUI_HR_UPLOAD_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>

namespace cgui {

/**
 * @class HrUploadPlugin
 * @brief Handles HR-specific payload formatting and upload.
 */
class HrUploadPlugin : public IUploadPlugin {
public:
    HrUploadPlugin() = default;
    ~HrUploadPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    std::expected<void, std::string> upload(const nlohmann::json& data) override;
    void shutdown() override;

private:
    nlohmann::json m_config;
    std::string m_endpoint;
    std::string m_bearer_token;

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
};

} // namespace cgui

#endif // CGUI_HR_UPLOAD_PLUGIN_HPP
