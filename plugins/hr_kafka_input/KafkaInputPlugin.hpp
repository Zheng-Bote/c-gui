/**
 * SPDX-FileComment: Kafka Input Plugin for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file KafkaInputPlugin.hpp
 * @brief Implementation of Kafka input plugin (Stub).
 * @version 1.2.0
 * @date 2026-05-23
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef CGUI_KAFKA_INPUT_PLUGIN_HPP
#define CGUI_KAFKA_INPUT_PLUGIN_HPP

#include "PluginAPI.hpp"
#include <librdkafka/rdkafkacpp.h>
#include <string>
#include <vector>
#include <memory>

namespace cgui {

/**
 * @class KafkaInputPlugin
 * @brief Implementation of Kafka input plugin using librdkafka.
 */
class KafkaInputPlugin : public IDataPlugin {
public:
    KafkaInputPlugin() = default;
    ~KafkaInputPlugin() override = default;

    bool initialize(const nlohmann::json& config) override;
    [[nodiscard]] std::string get_topic() const override { return "HR"; }
    [[nodiscard]] std::string get_version() const override { return "1.2.0"; }
    [[nodiscard]] std::string get_interface_type() const override { return "kafka"; }
    std::vector<nlohmann::json> fetch_batch(size_t max_records) override;
    void shutdown() override;

private:
    std::string m_kafka_topic;
    bool m_initialized = false;
    
    std::unique_ptr<RdKafka::Conf> m_conf;
    std::unique_ptr<RdKafka::KafkaConsumer> m_consumer;
};

} // namespace cgui

#endif // CGUI_KAFKA_INPUT_PLUGIN_HPP
