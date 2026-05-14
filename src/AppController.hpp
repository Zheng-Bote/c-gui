/**
 * SPDX-FileComment: AppController for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file AppController.hpp
 * @brief Orchestrates application logic and state.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
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
 * @class AppController
 * @brief Main controller for the application.
 */
class AppController {
public:
    AppController();
    ~AppController() = default;

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
     * @brief Load data from a CSV file using a plugin.
     * @param path Path to the CSV file.
     * @return Success or error message.
     */
    [[nodiscard]] std::expected<void, std::string> load_csv(const std::filesystem::path& path);

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

private:
    std::unique_ptr<ConfigManager> m_config_manager;
    std::unique_ptr<PluginLoader> m_plugin_loader;
    std::unique_ptr<TopicRegistry> m_topic_registry;
    std::unique_ptr<SchemaManager> m_schema_manager;
    std::unique_ptr<Validator> m_validator;
    std::unique_ptr<Uploader> m_uploader;

    MainWindow* m_window = nullptr;
    std::string m_current_topic;
    nlohmann::json m_current_data;
    nlohmann::json m_preview_data;

    void update_log(const std::string& message);
    void update_progress(int progress);
    void on_validation_complete(bool success, const std::vector<ValidationError>& errors);
    void on_upload_complete(std::expected<void, std::string> result);
};

} // namespace cgui

#endif // C_GUI_APP_CONTROLLER_HPP
