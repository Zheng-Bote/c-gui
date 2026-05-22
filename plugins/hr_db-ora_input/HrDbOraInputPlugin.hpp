/**
 * SPDX-FileComment: Oracle DB Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file HrDbOraInputPlugin.hpp
 * @brief Implementation of Oracle DB input plugin.
 * @version 1.2.0
 * @date 2026-05-23
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_HR_DB_ORA_INPUT_PLUGIN_HPP
#define CGUI_HR_DB_ORA_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <soci/soci.h>
#include <string>
#include <vector>
#include <memory>

namespace cgui {

/**
 * @class HrDbOraInputPlugin
 * @brief Implementation of Oracle database input plugin using SOCI.
 */
class HrDbOraInputPlugin : public IDataPlugin {
public:
    HrDbOraInputPlugin() = default;
    ~HrDbOraInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.2.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "db-ora"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_connection_string;
    bool m_initialized = false;
    
    std::unique_ptr<soci::session> m_sql;
};

} // namespace cgui

#endif // CGUI_HR_DB_ORA_INPUT_PLUGIN_HPP
