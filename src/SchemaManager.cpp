/**
 * SPDX-FileComment: SchemaManager implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file SchemaManager.cpp
 * @brief Implementation of JSON schema management.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "SchemaManager.hpp"
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/utils/nlohmann_json_utils.hpp>
#include <valijson/schema_parser.hpp>
#include <fstream>

namespace cgui {

std::shared_ptr<valijson::Schema> SchemaManager::get_schema(const std::filesystem::path& path) {
    auto it = m_schemas.find(path);
    if (it != m_schemas.end()) {
        return it->second;
    }

    nlohmann::json schema_json;
    if (!valijson::utils::loadDocument(path.string(), schema_json)) {
        return nullptr;
    }

    auto schema = std::make_shared<valijson::Schema>();
    valijson::SchemaParser parser;
    valijson::adapters::NlohmannJsonAdapter schema_adapter(schema_json);

    try {
        parser.populateSchema(schema_adapter, *schema);
    } catch (const std::exception&) {
        return nullptr;
    }

    m_schemas[path] = schema;
    return schema;
}

} // namespace cgui
