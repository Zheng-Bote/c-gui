/**
 * SPDX-FileComment: DB Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file DbInputPlugin.hpp
 * @brief Implementation of PostgreSQL input plugin.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_DB_INPUT_PLUGIN_HPP
#define CGUI_DB_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <vector>

namespace cgui {

class DbInputPlugin : public IDataPlugin {
public:
    DbInputPlugin() = default;
    ~DbInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "db-pg"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_topic;
    std::string m_connection_string;
    std::string m_query;
    std::unique_ptr<pqxx::connection> m_connection;
    bool m_initialized = false;
};

} // namespace cgui

#endif // CGUI_DB_INPUT_PLUGIN_HPP
