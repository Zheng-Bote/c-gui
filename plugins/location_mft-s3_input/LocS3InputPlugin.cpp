/**
 * SPDX-FileComment: Location S3 Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file LocS3InputPlugin.cpp
 * @brief Implementation of S3 MFT input plugin for the Location topic (Stub).
 * @version 1.0.0
 * @date 2026-05-18
 */

#include "LocS3InputPlugin.hpp"

namespace cgui {

bool LocS3InputPlugin::initialize(const nlohmann::json& config) {
    if (!config.contains("connection_string")) {
        return false;
    }

    m_s3_bucket = config["connection_string"].get<std::string>();
    
    // In a real implementation, you would initialize AWS SDK here.
    m_initialized = true;
    return true;
}

std::vector<nlohmann::json> LocS3InputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized) return batch;

    // Stub: Return some dummy data mimicking Location records
    for (size_t i = 0; i < max_records; ++i) {
        nlohmann::json row;
        row["location_id"] = 4000 + i;
        row["city"] = "S3_Cloud_City_" + std::to_string(i);
        row["region"] = "AWS-MFT";
        row["bucket"] = m_s3_bucket;
        batch.push_back(row);
    }

    return batch;
}

void LocS3InputPlugin::shutdown() {
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::LocS3InputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
