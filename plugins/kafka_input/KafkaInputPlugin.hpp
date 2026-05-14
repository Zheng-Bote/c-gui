/**
 * SPDX-FileComment: Kafka Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file KafkaInputPlugin.hpp
 * @brief Implementation of Kafka input plugin (Stub).
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_KAFKA_INPUT_PLUGIN_HPP
#define CGUI_KAFKA_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <string>
#include <vector>

namespace cgui {

class KafkaInputPlugin : public IPlugin {
public:
    KafkaInputPlugin() = default;
    ~KafkaInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override;
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_topic;
    bool m_initialized = false;
};

} // namespace cgui

#endif // CGUI_KAFKA_INPUT_PLUGIN_HPP
