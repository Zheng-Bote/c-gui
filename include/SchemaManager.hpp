/**
 * SPDX-FileComment: SchemaManager for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file SchemaManager.hpp
 * @brief Loads and caches JSON schemas.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_SCHEMA_MANAGER_HPP
#define C_GUI_SCHEMA_MANAGER_HPP

#include <string>
#include <map>
#include <filesystem>
#include <memory>
#include <valijson/schema.hpp>

namespace cgui {

/**
 * @brief Manages JSON schemas using valijson.
 */
class SchemaManager {
public:
    SchemaManager() = default;
    ~SchemaManager() = default;

    /**
     * @brief Get a schema by path. Loads and caches it if not already present.
     * @param path Path to the schema file.
     * @return Shared pointer to the schema, or nullptr if loading failed.
     */
    std::shared_ptr<valijson::Schema> get_schema(const std::filesystem::path& path);

private:
    std::map<std::filesystem::path, std::shared_ptr<valijson::Schema>> m_schemas;
};

} // namespace cgui

#endif // C_GUI_SCHEMA_MANAGER_HPP
