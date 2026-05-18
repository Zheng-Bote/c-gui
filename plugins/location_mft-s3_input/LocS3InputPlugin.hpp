/**
 * SPDX-FileComment: Location S3 Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file LocS3InputPlugin.hpp
 * @brief Implementation of S3 MFT input plugin for the Location topic.
 * @version 1.0.0
 * @date 2026-05-18
 */

#ifndef CGUI_LOC_S3_INPUT_PLUGIN_HPP
#define CGUI_LOC_S3_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <vector>

namespace cgui {

class LocS3InputPlugin : public IDataPlugin {
public:
    LocS3InputPlugin() = default;
    ~LocS3InputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "Location"; }
    [[nodiscard]] std::string get_version() const override { return "1.0.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "mft-s3"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_s3_bucket;
    bool m_initialized = false;
};

} // namespace cgui

#endif // CGUI_LOC_S3_INPUT_PLUGIN_HPP
