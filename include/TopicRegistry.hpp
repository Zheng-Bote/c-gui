/**
 * SPDX-FileComment: TopicRegistry for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file TopicRegistry.hpp
 * @brief Maps topics to schemas and endpoints.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_TOPIC_REGISTRY_HPP
#define C_GUI_TOPIC_REGISTRY_HPP

#include <string>
#include <map>
#include <filesystem>
#include <optional>
#include <vector>

namespace cgui {

struct TopicMeta {
    std::string topic;
    std::filesystem::path schema_path;
    std::string upload_endpoint;
    std::string upload_plugin;  ///< Name of the upload plugin (e.g., "hr_upload")
};

/**
 * @brief Registry for topic metadata.
 */
class TopicRegistry {
public:
    void register_topic(const TopicMeta& meta);
    [[nodiscard]] std::optional<TopicMeta> get_topic_meta(const std::string& topic) const;
    [[nodiscard]] std::vector<std::string> get_all_topics() const;

private:
    std::map<std::string, TopicMeta> m_topics;
};

} // namespace cgui

#endif // C_GUI_TOPIC_REGISTRY_HPP
