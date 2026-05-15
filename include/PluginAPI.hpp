/**
 * SPDX-FileComment: Plugin API interface for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file PluginAPI.hpp
 * @brief Definition of the IPlugin interface and C factory functions.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_PLUGIN_API_HPP
#define C_GUI_PLUGIN_API_HPP

#include <string>
#include <vector>
#include <expected>
#include <nlohmann/json.hpp>

namespace cgui {

/**
 * @brief Types of plugins supported by the application.
 */
enum class PluginType {
    DATA,   ///< Ingest plugin (reads data)
    UPLOAD  ///< Output plugin (uploads data)
};

/**
 * @brief Base interface for all plugins.
 */
class IPlugin {
public:
    virtual ~IPlugin() = default;

    /**
     * @brief Initialize the plugin with configuration.
     * @param config JSON configuration object.
     * @return true if initialization was successful, false otherwise.
     */
    virtual bool initialize(const nlohmann::json& config) = 0;

    /**
     * @brief Get the topic this plugin handles.
     * @return The topic name.
     */
    [[nodiscard]] virtual std::string get_topic() const = 0;

    /**
     * @brief Get the version of the plugin.
     * @return Version string (e.g., "1.0.0").
     */
    [[nodiscard]] virtual std::string get_version() const = 0;

    /**
     * @brief Get the type of the plugin.
     * @return PluginType (DATA or UPLOAD).
     */
    [[nodiscard]] virtual PluginType get_type() const = 0;

    /**
     * @brief Shutdown the plugin and release resources.
     */
    virtual void shutdown() = 0;
};

/**
 * @brief Interface for data ingest plugins.
 */
class IDataPlugin : public IPlugin {
public:
    /**
     * @brief Fetch a batch of records.
     * @param max_records Maximum number of records to fetch.
     * @return Vector of JSON objects.
     */
    virtual std::vector<nlohmann::json> fetch_batch(size_t max_records) = 0;

    /**
     * @brief Get the interface type (e.g., "csv", "db-pg").
     * @return The interface type string.
     */
    [[nodiscard]] virtual std::string get_interface_type() const = 0;

    [[nodiscard]] PluginType get_type() const override { return PluginType::DATA; }
};

/**
 * @brief Interface for upload output plugins.
 */
class IUploadPlugin : public IPlugin {
public:
    /**
     * @brief Upload data to the target system.
     * @param data The JSON data to upload.
     * @return Success or error message.
     */
    virtual std::expected<void, std::string> upload(const nlohmann::json& data) = 0;

    [[nodiscard]] PluginType get_type() const override { return PluginType::UPLOAD; }
};

} // namespace cgui

#ifdef _WIN32
#define CGUI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define CGUI_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    /**
     * @brief Factory function to create a plugin instance.
     * @return Pointer to the created plugin instance (must be IDataPlugin* or IUploadPlugin*).
     */
    CGUI_PLUGIN_EXPORT cgui::IPlugin* create_plugin();

    /**
     * @brief Factory function to destroy a plugin instance.
     * @param plugin Pointer to the plugin instance to destroy.
     */
    CGUI_PLUGIN_EXPORT void destroy_plugin(cgui::IPlugin* plugin);
}

#endif // C_GUI_PLUGIN_API_HPP
