/**
 * SPDX-FileComment: HR XLSX Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file XlsxInputPlugin.hpp
 * @brief Implementation of XLSX input plugin using OpenXLSX.
 * @version 1.0.0
 * @date 2026-05-18
 */

#ifndef CGUI_XLSX_INPUT_PLUGIN_HPP
#define CGUI_XLSX_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <OpenXLSX/OpenXLSX.hpp>
#include <string>
#include <vector>
#include <memory>

namespace cgui {

class XlsxInputPlugin : public IDataPlugin {
public:
    XlsxInputPlugin() = default;
    ~XlsxInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "xlsx"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_topic;
    std::string m_file_path;
    bool m_initialized = false;
    
    std::unique_ptr<OpenXLSX::XLDocument> m_doc;
    std::vector<std::string> m_headers;
    uint32_t m_current_row = 2; // Assuming row 1 is headers
    uint32_t m_max_row = 0;
};

} // namespace cgui

#endif // CGUI_XLSX_INPUT_PLUGIN_HPP
