/**
 * SPDX-FileComment: DB Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file DbInputPlugin.cpp
 * @brief Implementation of PostgreSQL input plugin.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "DbInputPlugin.hpp"
#include <iostream>

namespace cgui {

bool DbInputPlugin::initialize(const nlohmann::json& config) {
    try {
        if (!config.contains("topic") || !config.contains("connection_string") || !config.contains("query")) {
            return false;
        }

        m_topic = config["topic"].get<std::string>();
        m_connection_string = config["connection_string"].get<std::string>();
        m_query = config["query"].get<std::string>();

        m_connection = std::make_unique<pqxx::connection>(m_connection_string);
        if (!m_connection->is_open()) {
            return false;
        }

        m_initialized = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "DbInputPlugin initialization failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<nlohmann::json> DbInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized || !m_connection) return batch;

    try {
        pqxx::work txn(*m_connection);
        // Note: In a real implementation, you'd handle pagination or streaming.
        // For this minimal plugin, we just execute the query and limit results.
        std::string limited_query = m_query + " LIMIT " + std::to_string(max_records);
        pqxx::result res = txn.exec(limited_query);

        for (const auto& row : res) {
            nlohmann::json obj;
            for (const auto& col : row) {
                obj[col.name()] = col.c_str();
            }
            batch.push_back(obj);
        }

        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "DbInputPlugin fetch_batch failed: " << e.what() << std::endl;
    }

    return batch;
}

void DbInputPlugin::shutdown() {
    if (m_connection) {
        m_connection->close();
        m_connection.reset();
    }
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::DbInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
