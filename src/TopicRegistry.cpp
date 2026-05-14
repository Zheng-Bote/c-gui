/**
 * SPDX-FileComment: TopicRegistry implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file TopicRegistry.cpp
 * @brief Implementation of topic metadata registry.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "TopicRegistry.hpp"

namespace cgui {

void TopicRegistry::register_topic(const TopicMeta& meta) {
    m_topics[meta.topic] = meta;
}

std::optional<TopicMeta> TopicRegistry::get_topic_meta(const std::string& topic) const {
    auto it = m_topics.find(topic);
    if (it != m_topics.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<std::string> TopicRegistry::get_all_topics() const {
    std::vector<std::string> topics;
    topics.reserve(m_topics.size());
    for (const auto& [topic, _] : m_topics) {
        topics.push_back(topic);
    }
    return topics;
}

} // namespace cgui
