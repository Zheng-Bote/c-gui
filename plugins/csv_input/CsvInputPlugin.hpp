/**
 * SPDX-FileComment: CSV Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file CsvInputPlugin.hpp
 * @brief Implementation of CSV input plugin.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_CSV_INPUT_PLUGIN_HPP
#define CGUI_CSV_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <fstream>
#include <string>
#include <vector>

namespace cgui {

class CsvInputPlugin : public IPlugin {
public:
    CsvInputPlugin() = default;
    ~CsvInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override;
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_topic;
    std::string m_file_path;
    std::ifstream m_file;
    std::vector<std::string> m_headers;
    bool m_initialized = false;

    std::vector<std::string> parse_csv_line(const std::string& line);
};

} // namespace cgui

#endif // CGUI_CSV_INPUT_PLUGIN_HPP
