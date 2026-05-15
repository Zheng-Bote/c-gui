/**
 * SPDX-FileComment: CSV Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file CsvInputPlugin.cpp
 * @brief Implementation of CSV input plugin.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "CsvInputPlugin.hpp"
#include <sstream>

namespace cgui {

bool CsvInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("topic") || !config.contains("file_path")) {
        return false;
    }

    m_topic = config["topic"].get<std::string>();
    m_file_path = config["file_path"].get<std::string>();

    m_file.open(m_file_path);
    if (!m_file.is_open()) {
        return false;
    }

    std::string header_line;
    if (std::getline(m_file, header_line)) {
        m_headers = parse_csv_line(header_line);
    } else {
        return false;
    }

    m_initialized = true;
    return true;
}

std::vector<nlohmann::json> CsvInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    std::string line;
    while (batch.size() < max_records && std::getline(m_file, line)) {
        auto values = parse_csv_line(line);
        nlohmann::json obj;
        for (size_t i = 0; i < m_headers.size() && i < values.size(); ++i) {
            obj[m_headers[i]] = values[i];
        }
        batch.push_back(obj);
    }

    return batch;
}

void CsvInputPlugin::shutdown() {
    if (m_file.is_open()) {
        m_file.close();
    }
    m_initialized = false;
}

std::vector<std::string> CsvInputPlugin::parse_csv_line(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // Basic CSV parsing, doesn't handle quotes/escaping for simplicity
        result.push_back(item);
    }
    return result;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::CsvInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
