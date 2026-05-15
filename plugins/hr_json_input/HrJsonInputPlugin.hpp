/**
 * SPDX-FileComment: JSON Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrJsonInputPlugin.hpp
 * @brief Implementation of JSON file input plugin.
 * @version 1.0.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_HR_JSON_INPUT_PLUGIN_HPP
#define CGUI_HR_JSON_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <vector>

namespace cgui {

/**
 * @class HrJsonInputPlugin
 * @brief Implementation of JSON file input plugin.
 */
class HrJsonInputPlugin : public IDataPlugin {
public:
    HrJsonInputPlugin() = default;
    ~HrJsonInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "json"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_file_path;
    nlohmann::json m_data;
    bool m_initialized = false;
    size_t m_current_pos = 0;
};

} // namespace cgui

#endif // CGUI_HR_JSON_INPUT_PLUGIN_HPP
