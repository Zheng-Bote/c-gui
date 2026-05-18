/**
 * SPDX-FileComment: Department Oracle DB Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file DeptOraInputPlugin.cpp
 * @brief Implementation of Oracle DB input plugin for the Department topic (Stub).
 * @version 1.0.0
 * @date 2026-05-18
 */

#include "DeptOraInputPlugin.hpp"

namespace cgui {

bool DeptOraInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("connection_string")) {
        return false;
    }

    m_connection_string = config["connection_string"].get<std::string>();
    
    // In a real implementation, you would initialize Oracle client here.
    m_initialized = true;
    return true;
}

std::vector<nlohmann::json> DeptOraInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    // Stub: Return some dummy data mimicking Department records
    for (size_t i = 0; i < max_records; ++i) {
        nlohmann::json row;
        row["dept_id"] = 2000 + i;
        row["dept_name"] = "Oracle_Department_" + std::to_string(i);
        row["manager_id"] = 5000 + (i % 5);
        row["location_id"] = 1700;
        batch.push_back(row);
    }

    return batch;
}

void DeptOraInputPlugin::shutdown() {
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::DeptOraInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
