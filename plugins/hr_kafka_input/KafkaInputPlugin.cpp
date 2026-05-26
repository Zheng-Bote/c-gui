/**
 * SPDX-FileComment: Kafka Input Plugin implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file KafkaInputPlugin.cpp
 * @brief Implementation of Kafka input plugin using librdkafka.
 * @version 1.2.0
 * @date 2026-05-23
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "KafkaInputPlugin.hpp"
#include "LogManager.hpp"
#include <iostream>
#include <chrono>
#include <map>
#include <sstream>
#include <format>

namespace cgui {

/**
 * @brief Helper to parse connection string parts.
 */
static std::map<std::string, std::string> parse_kafka_params(const std::string& conn_str) {
    std::map<std::string, std::string> result;
    std::stringstream ss(conn_str);
    std::string segment;

    while (std::getline(ss, segment, ',')) {
        size_t colon_pos = segment.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = segment.substr(0, colon_pos);
            std::string value = segment.substr(colon_pos + 1);
            
            auto trim = [](std::string& s) {
                if (s.empty()) return;
                s.erase(0, s.find_first_not_of(" "));
                if (s.empty()) return;
                s.erase(s.find_last_not_of(" ") + 1);
            };
            trim(key);
            trim(value);
            
            result[key] = value;
        }
    }
    return result;
}

bool KafkaInputPlugin::initialize(const nlohmann::json& config) {
    std::string errstr;
    std::string topic_name = config.contains("topic") ? config["topic"].get<std::string>() : "Core";
    auto logger = LogManager::get_instance().get_logger(topic_name);

    if (!config.contains("connection_string")) {
        logger->error("Kafka Plugin: Missing connection_string in configuration");
        return false;
    }

    std::string conn_str = config["connection_string"].get<std::string>();
    auto params = parse_kafka_params(conn_str);

    if (params.count("broker") == 0 || params.count("topic") == 0 || 
        params.count("api_key") == 0 || params.count("api_secret") == 0) {
        logger->error("Kafka Plugin: Missing required parameters in connection string (broker, topic, api_key, api_secret)");
        return false;
    }

    m_kafka_topic = params["topic"];
    std::string brokers = params["broker"];
    std::string api_key = params["api_key"];
    std::string api_secret = params["api_secret"];
    std::string base_group_id = params.count("group_id") ? params["group_id"] : "cgui.default.group.";

    // Requirement: group.id should be extended with <username>
    std::string username = "unknown_user";
    if (config.contains("os_user")) {
        username = config["os_user"].get<std::string>();
    }
    std::string full_group_id = base_group_id + username;

    m_conf.reset(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

    auto set_conf = [&](const std::string& key, const std::string& val) {
        if (m_conf->set(key, val, errstr) != RdKafka::Conf::CONF_OK) {
            logger->error("Kafka Plugin: Failed to set {}: {}", key, errstr);
            return false;
        }
        return true;
    };

    if (!set_conf("bootstrap.servers", brokers)) return false;
    
    // Support custom security protocol and mechanism if provided in params, otherwise default to SASL_SSL/PLAIN
    std::string protocol = params.count("security_protocol") ? params["security_protocol"] : "SASL_SSL";
    std::string mechanism = params.count("sasl_mechanism") ? params["sasl_mechanism"] : "PLAIN";

    if (!set_conf("security.protocol", protocol)) return false;
    if (protocol.find("SASL") != std::string::npos) {
        if (!set_conf("sasl.mechanism", mechanism)) return false;
        if (!set_conf("sasl.username", api_key)) return false;
        if (!set_conf("sasl.password", api_secret)) return false;
    }
    
    if (!set_conf("group.id", full_group_id)) return false;
    if (!set_conf("auto.offset.reset", "earliest")) return false;

    // SSL CA handling
    if (config.contains("ssl_ca_path") && !config["ssl_ca_path"].is_null()) {
        std::string ca_path = config["ssl_ca_path"].get<std::string>();
        if (!ca_path.empty()) {
            if (!set_conf("ssl.ca.location", ca_path)) return false;
        }
    }

    m_consumer.reset(RdKafka::KafkaConsumer::create(m_conf.get(), errstr));
    if (!m_consumer) {
        logger->error("Kafka Plugin: Failed to create consumer: {}", errstr);
        return false;
    }

    RdKafka::ErrorCode err = m_consumer->subscribe({m_kafka_topic});
    if (err != RdKafka::ERR_NO_ERROR) {
        logger->error("Kafka Plugin: Failed to subscribe to {}: {}", m_kafka_topic, RdKafka::err2str(err));
        return false;
    }

    m_initialized = true;
    logger->info("Kafka Plugin: Initialized for user '{}' on topic {} (group: {})", username, m_kafka_topic, full_group_id);
    return true;
}

std::vector<nlohmann::json> KafkaInputPlugin::fetch_batch(size_t max_records) {
    std::vector<nlohmann::json> batch;
    if (!m_initialized || !m_consumer) return batch;

    for (size_t i = 0; i < max_records; ++i) {
        std::unique_ptr<RdKafka::Message> msg(m_consumer->consume(1000)); // 1s timeout

        if (msg->err() == RdKafka::ERR__TIMED_OUT) {
            break; // No more messages for now
        }

        if (msg->err() != RdKafka::ERR_NO_ERROR) {
            std::cerr << "Kafka Plugin: Consume error: " << msg->errstr() << "\n";
            continue;
        }

        nlohmann::json data;
        std::string payload(static_cast<const char*>(msg->payload()), msg->len());

        try {
            data = nlohmann::json::parse(payload);
        } catch (...) {
            data["raw_value"] = payload;
        }

        // Add metadata
        if (msg->key()) {
            data["key"] = *(msg->key());
        }

        RdKafka::MessageTimestamp ts = msg->timestamp();
        if (ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
            // Convert ms to ISO string (simplified)
            auto tp = std::chrono::system_clock::from_time_t(ts.timestamp / 1000);
            data["Timestamp"] = std::format("{:%FT%TZ}", tp);
        }

        batch.push_back(data);
    }

    return batch;
}

void KafkaInputPlugin::shutdown() {
    if (m_consumer) {
        m_consumer->close();
    }
    m_initialized = false;
}

} // namespace cgui

extern "C" {
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin() {
        return new cgui::KafkaInputPlugin();
    }

    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin) {
        delete plugin;
    }
}
