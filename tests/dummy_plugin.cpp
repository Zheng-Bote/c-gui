/**
 * SPDX-FileComment: Dummy plugin for testing
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file tests/dummy_plugin.cpp
 * @brief Minimal plugin implementation for unit tests.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "PluginAPI.hpp"
#include <string>

class DummyPlugin : public cgui::IPlugin {
public:
    std::string get_topic() const override { return "test_topic"; }
    bool initialize(const nlohmann::json& config) override { (void)config; return true; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override { (void)max_records; return {}; }
    void shutdown() override {}
};

extern "C" {
    cgui::IPlugin* create_plugin() {
        return new DummyPlugin();
    }

    void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
