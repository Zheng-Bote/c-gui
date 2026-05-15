/**
 * SPDX-FileComment: Oracle DB Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrDbOraInputPlugin.cpp
 * @brief Implementation of Oracle DB input plugin (Stub).
 * @version 1.0.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "HrDbOraInputPlugin.hpp"

namespace cgui {

bool HrDbOraInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("connection_string")) {
        return false;
    }

    m_connection_string = config["connection_string"].get<std::string>();
    
    // In a real implementation, you would initialize OCCI or odpi-c here.
    // For now, we simulate a successful connection.
    m_initialized = true;
    return true;
}

std::vector<nlohmann::json> HrDbOraInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    // Stub: Return some dummy data mimicking Oracle row fetch
    for (size_t i = 0; i < max_records; ++i) {
        nlohmann::json row;
        row["employee_id"] = 1000 + i;
        row["first_name"] = "Oracle";
        row["last_name"] = "User_" + std::to_string(i);
        row["email"] = "user" + std::to_string(i) + "@oracle-source.com";
        batch.push_back(row);
    }

    return batch;
}

void HrDbOraInputPlugin::shutdown() {
    // Cleanup Oracle client resources here
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::HrDbOraInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
