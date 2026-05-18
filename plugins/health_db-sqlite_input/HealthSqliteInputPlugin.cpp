/**
 * SPDX-FileComment: Health SQLite Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HealthSqliteInputPlugin.cpp
 * @brief Implementation of SQLite input plugin for the Health topic (Stub).
 * @version 1.0.0
 * @date 2026-05-18
 */

#include "HealthSqliteInputPlugin.hpp"

namespace cgui {

bool HealthSqliteInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("file_path")) {
        return false;
    }

    m_db_path = config["file_path"].get<std::string>();
    
    // In a real implementation, you would open the SQLite database here.
    m_initialized = true;
    return true;
}

std::vector<nlohmann::json> HealthSqliteInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    // Stub: Return some dummy data mimicking Health records
    for (size_t i = 0; i < max_records; ++i) {
        nlohmann::json row;
        row["patient_id"] = 3000 + i;
        row["status"] = "Healthy";
        row["last_checkup"] = "2026-05-01";
        row["notes"] = "SQLite source example";
        batch.push_back(row);
    }

    return batch;
}

void HealthSqliteInputPlugin::shutdown() {
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::HealthSqliteInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
