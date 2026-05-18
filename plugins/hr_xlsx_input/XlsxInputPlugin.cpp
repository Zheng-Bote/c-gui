/**
 * SPDX-FileComment: HR XLSX Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file XlsxInputPlugin.cpp
 * @brief Implementation of XLSX input plugin using OpenXLSX.
 * @version 1.0.0
 * @date 2026-05-18
 */

#include "XlsxInputPlugin.hpp"
#include <iostream>

namespace cgui {

bool XlsxInputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("topic") || !config.contains("file_path")) {
        return false;
    }

    m_topic = config["topic"].get<std::string>();
    m_file_path = config["file_path"].get<std::string>();

    try {
        m_doc = std::make_unique<OpenXLSX::XLDocument>();
        m_doc->open(m_file_path);
        
        auto sheet = m_doc->workbook().worksheet(m_doc->workbook().sheetNames()[0]);
        m_max_row = sheet.rowCount();
        
        // Extract headers from first row
        for (uint16_t col = 1; col <= sheet.columnCount(); ++col) {
            auto cell = sheet.cell(1, col);
            if (cell.value().type() != OpenXLSX::XLValueType::Empty) {
                m_headers.push_back(cell.value().get<std::string>());
            } else {
                m_headers.push_back("Column_" + std::to_string(col));
            }
        }
        
        m_current_row = 2; // Data starts at row 2
        m_initialized = true;
    } catch (const std::exception& e) {
        std::cerr << "XlsxInputPlugin: Error opening " << m_file_path << ": " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::vector<nlohmann::json> XlsxInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized || !m_doc) return batch;

    auto sheet = m_doc->workbook().worksheet(m_doc->workbook().sheetNames()[0]);
    
    while (batch.size() < max_records && m_current_row <= m_max_row) {
        nlohmann::json obj;
        bool has_data = false;
        
        for (uint16_t col = 1; col <= m_headers.size(); ++col) {
            auto cell = sheet.cell(m_current_row, col);
            
            if (cell.value().type() != OpenXLSX::XLValueType::Empty) {
                has_data = true;
                auto type = cell.value().type();
                
                if (type == OpenXLSX::XLValueType::String) {
                    obj[m_headers[col-1]] = cell.value().get<std::string>();
                } else if (type == OpenXLSX::XLValueType::Integer) {
                    obj[m_headers[col-1]] = cell.value().get<int64_t>();
                } else if (type == OpenXLSX::XLValueType::Float) {
                    obj[m_headers[col-1]] = cell.value().get<double>();
                } else if (type == OpenXLSX::XLValueType::Boolean) {
                    obj[m_headers[col-1]] = cell.value().get<bool>();
                } else {
                    obj[m_headers[col-1]] = cell.value().get<std::string>();
                }
            } else {
                obj[m_headers[col-1]] = nullptr;
            }
        }
        
        if (has_data) {
            batch.push_back(obj);
        }
        m_current_row++;
    }

    return batch;
}

void XlsxInputPlugin::shutdown() {
    if (m_doc) {
        m_doc->close();
        m_doc.reset();
    }
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::XlsxInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
