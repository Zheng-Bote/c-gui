/**
 * SPDX-FileComment: Department Oracle DB Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file DeptOraInputPlugin.hpp
 * @brief Implementation of Oracle DB input plugin for the Department topic.
 * @version 1.0.0
 * @date 2026-05-18
 */

#ifndef CGUI_DEPT_ORA_INPUT_PLUGIN_HPP
#define CGUI_DEPT_ORA_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <vector>

namespace cgui {

class DeptOraInputPlugin : public IDataPlugin {
public:
    DeptOraInputPlugin() = default;
    ~DeptOraInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "Department"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "db-ora"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_connection_string;
    bool m_initialized = false;
};

} // namespace cgui

#endif // CGUI_DEPT_ORA_INPUT_PLUGIN_HPP
