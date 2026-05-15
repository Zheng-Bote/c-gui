/**
 * SPDX-FileComment: AppController for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file AppController.hpp
 * @brief Orchestrates application logic and state.
 * @version 1.1.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_APP_CONTROLLER_HPP
#define C_GUI_APP_CONTROLLER_HPP

#include <memory>
#include <string>
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>
#include <expected>
#include <functional>

#include "ConfigManager.hpp"
#include "PluginLoader.hpp"
#include "TopicRegistry.hpp"
#include "SchemaManager.hpp"
#include "Validator.hpp"
#include "Uploader.hpp"

namespace cgui {

class MainWindow;

/**
 * @struct PluginInfo
 * @brief Metadata about a discovered plugin.
 */
struct PluginInfo {
    std::string name;
    std::string topic;
    std::string type;    // "DATA" or "UPLOAD"
    std::string version;
    std::string interface; // e.g., "csv" (for DATA only)
    std::filesystem::path path;
};

/**
 * @class AppController
 * @brief Main controller for the application.
 */
class AppController {
public:
    AppController();
    ~AppController();

    /**
     * @brief Initialize the controller and load configuration.
     * @param ini_path Path to the encrypted INI file.
     * @param password Password for decryption.
     * @return Success or error message.
     */
    [[nodiscard]] std::expected<void, std::string> init(const std::filesystem::path& ini_path, const std::string& password);

    /**
     * @brief Get the list of available topics.
     * @return Vector of topic names.
     */
    [[nodiscard]] std::vector<std::string> get_topics() const;

    /**
     * @brief Select a topic.
     * @param topic Topic name.
     */
    void select_topic(const std::string& topic);

    /**
     * @brief Get available data interfaces for a topic.
     * @param topic Topic name.
     * @return Vector of interface names (e.g., "csv", "db-pg").
     */
    [[nodiscard]] std::vector<std::string> get_available_interfaces(const std::string& topic) const;

    /**
     * @brief Load data using a specific interface.
     * @param interface_name Name of the interface (e.g., "csv").
     * @param path_or_conn Path to file or connection string.
     * @return Success or error message.
     */
    [[nodiscard]] std::expected<void, std::string> load_data(const std::string& interface_name, const std::string& path_or_conn);

    /**
     * @brief Get the default data source string from the configuration.
     * @param topic Topic name.
     * @param interface_name Interface name.
     * @return Default path or connection string, or empty if not set.
     */
    [[nodiscard]] std::string get_default_data_source(const std::string& topic, const std::string& interface_name) const;

    /**
     * @brief Get a list of all discovered plugins.
     * @return Vector of PluginInfo.
     */
    [[nodiscard]] std::vector<PluginInfo> get_all_plugins() const;

    /**
     * @brief Start validation of loaded data in a background thread.
     */
    void start_validation();

    /**
     * @brief Start upload of validated data in a background thread.
     */
    void start_upload();

    /**
     * @brief Set the main window reference.
     * @param window Pointer to MainWindow.
     */
    void set_window(MainWindow* window) { m_window = window; }

    /**
     * @brief Get the preview data.
     * @return JSON array of records.
     */
    [[nodiscard]] const nlohmann::json& get_preview_data() const { return m_preview_data; }

    /**
     * @brief Get the loaded configuration.
     * @return JSON object containing the configuration.
     */
    [[nodiscard]] const nlohmann::json& get_config() const { return m_config_manager->get_config(); }

private:
    std::unique_ptr<ConfigManager> m_config_manager;
    std::unique_ptr<PluginLoader> m_plugin_loader;
    std::unique_ptr<TopicRegistry> m_topic_registry;
    std::unique_ptr<SchemaManager> m_schema_manager;
    std::unique_ptr<Validator> m_validator;
    std::unique_ptr<Uploader> m_uploader;

    std::filesystem::path m_data_plugins_dir;
    std::filesystem::path m_upload_plugins_dir;

    MainWindow* m_window = nullptr;
    std::string m_current_topic;
    nlohmann::json m_current_data;
    nlohmann::json m_preview_data;

    [[nodiscard]] std::filesystem::path get_plugin_path(const std::string& name, PluginType type) const;

    void update_log(const std::string& message);
    void update_progress(int progress);
    void on_validation_complete(bool success, const std::vector<ValidationError>& errors);
    void on_upload_complete(std::expected<void, std::string> result);
};

} // namespace cgui

#endif // C_GUI_APP_CONTROLLER_HPP
