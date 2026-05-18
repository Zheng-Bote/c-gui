/**
 * SPDX-FileComment: Health SQLite Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HealthSqliteInputPlugin.hpp
 * @brief Implementation of SQLite input plugin for the Health topic.
 * @version 1.0.0
 * @date 2026-05-18
 */

#ifndef CGUI_HEALTH_SQLITE_INPUT_PLUGIN_HPP
#define CGUI_HEALTH_SQLITE_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <vector>

namespace cgui {

class HealthSqliteInputPlugin : public IDataPlugin {
public:
    HealthSqliteInputPlugin() = default;
    ~HealthSqliteInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "Health"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "db-sqlite"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_db_path;
    bool m_initialized = false;
};

} // namespace cgui

#endif // CGUI_HEALTH_SQLITE_INPUT_PLUGIN_HPP
