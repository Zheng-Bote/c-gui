/**
 * SPDX-FileComment: JSON Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrJsonInputPlugin.cpp
 * @brief Implementation of JSON file input plugin.
 * @version 1.0.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "HrJsonInputPlugin.hpp"
#include <fstream>

namespace cgui {

bool HrJsonInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("file_path")) {
        return false;
    }

    m_file_path = config["file_path"].get<std::string>();

    try {
        std::ifstream file(m_file_path);
        if (!file.is_open()) {
            return false;
        }

        file >> m_data;
        if (!m_data.is_array()) {
            return false;
        }

        m_current_pos = 0;
        m_initialized = true;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<nlohmann::json> HrJsonInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    while (batch.size() < max_records && m_current_pos < m_data.size()) {
        batch.push_back(m_data[m_current_pos++]);
    }

    return batch;
}

void HrJsonInputPlugin::shutdown() {
    m_data.clear();
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::HrJsonInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
