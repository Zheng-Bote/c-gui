/**
 * SPDX-FileComment: AppController implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file AppController.cpp
 * @brief Implementation of application orchestration logic.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "AppController.hpp"
#include "MainWindow.hpp"
#include "LogManager.hpp"
#include "AuthManager.hpp"
#include <wx/wx.h>
#include <wx/app.h>
#include <thread>
#include <format>

namespace cgui {

AppController::AppController()
    : m_config_manager(std::make_unique<ConfigManager>()),
      m_plugin_loader(std::make_unique<PluginLoader>()),
      m_topic_registry(std::make_unique<TopicRegistry>()),
      m_schema_manager(std::make_unique<SchemaManager>()),
      m_validator(std::make_unique<Validator>()),
      m_uploader(std::make_unique<Uploader>()) {}

std::expected<void, std::string> AppController::init(const std::filesystem::path& ini_path, const std::string& password) {
    auto result = m_config_manager->load_encrypted_ini(ini_path, password);
    if (!result) {
        return result;
    }

    const auto& config = m_config_manager->get_config();

    // Initialize Logging
    std::string log_path = "./logs";
    std::string log_level = "info";
    if (config.contains("logging")) {
        if (config["logging"].contains("log_path")) log_path = config["logging"]["log_path"].get<std::string>();
        if (config["logging"].contains("log_level")) log_level = config["logging"]["log_level"].get<std::string>();
    }
    LogManager::get_instance().initialize(log_path, log_level);
    
    auto logger = LogManager::get_instance().get_core_logger();
    logger->info("Application initialized. Config loaded from {}", ini_path.string());

    // Register topics from config
    if (config.contains("topics")) {
        for (auto it = config["topics"].begin(); it != config["topics"].end(); ++it) {
            TopicMeta meta;
            meta.topic = it.key();
            meta.schema_path = it.value().at("schema_path").get<std::string>();
            meta.upload_endpoint = it.value().at("upload_endpoint").get<std::string>();
            m_topic_registry->register_topic(meta);
        }
    }

    return {};
}

std::vector<std::string> AppController::get_topics() const {
    return m_topic_registry->get_all_topics();
}

void AppController::select_topic(const std::string& topic) {
    m_current_topic = topic;
    update_log(std::format("Selected topic: {}", topic));
    LogManager::get_instance().get_core_logger()->info("Topic selected: {}", topic);
}

std::expected<void, std::string> AppController::load_csv(const std::filesystem::path& path) {
    if (m_current_topic.empty()) {
        return std::unexpected("No topic selected");
    }

    auto logger = LogManager::get_instance().get_logger(m_current_topic);
    logger->info("Loading CSV file: {}", path.string());

    const auto& config = m_config_manager->get_config();
    if (!config.contains("plugins") || !config["plugins"].contains("csv")) {
        logger->error("CSV plugin not configured in INI");
        return std::unexpected("CSV plugin not configured in INI");
    }

    std::filesystem::path plugin_path = config["plugins"]["csv"].get<std::string>();
    auto load_res = m_plugin_loader->load(plugin_path);
    if (!load_res) {
        logger->error("Failed to load plugin {}: {}", plugin_path.string(), load_res.error());
        return std::unexpected(std::format("Failed to load plugin {}: {}", plugin_path.string(), load_res.error()));
    }

    auto* plugin = m_plugin_loader->get_plugin();
    nlohmann::json plugin_config;
    plugin_config["file_path"] = path.string();
    plugin_config["topic"] = m_current_topic;

    if (!plugin->initialize(plugin_config)) {
        logger->error("Failed to initialize plugin with file: {}", path.string());
        return std::unexpected("Failed to initialize plugin with provided file");
    }

    // Load first batch for preview and full data
    auto records = plugin->fetch_batch(1000000); // For now, load everything
    plugin->shutdown();

    if (records.empty()) {
        logger->warn("No records found in file: {}", path.string());
        return std::unexpected("No records found in file");
    }

    m_current_data = nlohmann::json::array();
    for (const auto& r : records) {
        m_current_data.push_back(r);
    }

    bool show_preview = true;
    if (config.contains("gui") && config["gui"].contains("show_preview")) {
        std::string val = config["gui"]["show_preview"].get<std::string>();
        show_preview = (val == "true" || val == "1" || val == "yes");
    }

    m_preview_data = nlohmann::json::array();
    if (show_preview) {
        for (size_t i = 0; i < std::min(records.size(), size_t(100)); ++i) {
            m_preview_data.push_back(records[i]);
        }
    }

    update_log(std::format("Loaded {} records from {}", m_current_data.size(), path.filename().string()));
    logger->info("Successfully loaded {} records from {}", m_current_data.size(), path.string());
    
    if (m_window) {
        if (show_preview) {
            m_window->update_preview();
        } else {
            m_window->clear_preview();
            update_log("Data preview is disabled in configuration.");
        }
        m_window->update_buttons(true, false);
    }

    return {};
}

void AppController::start_validation() {
    if (m_current_topic.empty() || m_current_data.empty()) return;

    auto logger = LogManager::get_instance().get_logger(m_current_topic);

    auto meta = m_topic_registry->get_topic_meta(m_current_topic);
    if (!meta) {
        update_log("Error: Topic metadata not found");
        logger->error("Topic metadata not found for validation");
        return;
    }

    auto schema = m_schema_manager->get_schema(meta->schema_path);
    if (!schema) {
        update_log(std::format("Error: Could not load schema from {}", meta->schema_path.string()));
        logger->error("Could not load schema from {}", meta->schema_path.string());
        return;
    }

    update_log("Starting validation...");
    logger->info("Starting validation for {} records", m_current_data.size());
    update_progress(0);

    std::thread([this, schema, logger]() {
        std::vector<ValidationError> all_errors;
        bool all_valid = true;
        size_t total = m_current_data.size();

        for (size_t i = 0; i < total; ++i) {
            std::vector<ValidationError> errors;
            if (!Validator::validate(m_current_data[i], *schema, errors)) {
                all_valid = false;
                for (auto& err : errors) {
                    err.path = std::format("[Record {}] {}", i, err.path);
                    all_errors.push_back(err);
                }
            }

            if (i % 100 == 0 || i == total - 1) {
                int prog = static_cast<int>((static_cast<double>(i + 1) / total) * 100);
                logger->info("Validation progress: {}% ({} of {})", prog, i + 1, total);
                if (m_window) {
                    m_window->CallAfter([this, prog]() { update_progress(prog); });
                }
            }
            
            // Limit errors to not overwhelm UI
            if (all_errors.size() > 1000) {
                all_errors.push_back({"", "Too many errors, truncating..."});
                logger->warn("Validation exceeded 1000 errors, truncating further error details.");
                break;
            }
        }

        if (m_window) {
            m_window->CallAfter([this, all_valid, all_errors]() {
                on_validation_complete(all_valid, all_errors);
            });
        }
    }).detach();
}

void AppController::start_upload() {
    if (m_current_topic.empty() || m_current_data.empty()) return;

    auto logger = LogManager::get_instance().get_logger(m_current_topic);

    auto meta = m_topic_registry->get_topic_meta(m_current_topic);
    if (!meta) {
        logger->error("Topic metadata not found for upload");
        return;
    }

    update_log("Starting upload process...");
    logger->info("Starting upload process for topic {} ({} records)", m_current_topic, m_current_data.size());
    update_progress(0);

    const auto& config = m_config_manager->get_config();

    std::thread([this, meta, config, logger]() {
        std::string token;

        // Check for SaaS credentials to perform dynamic auth
        if (config.contains("auth") && config["auth"].contains("saas_base_url") && 
            config["auth"].contains("saas_login") && config["auth"].contains("saas_password")) {

            logger->info("Performing dynamic SaaS authentication...");
            auto auth_res = AuthManager::authenticate(
                config["auth"]["saas_base_url"].get<std::string>(),
                config["auth"]["saas_login"].get<std::string>(),
                config["auth"]["saas_password"].get<std::string>()
            );

            if (auth_res) {
                token = auth_res->access_token;
                logger->info("Authentication successful.");
            } else {
                logger->error("Authentication failed: {}", auth_res.error());
                if (m_window) {
                    m_window->CallAfter([this, err = auth_res.error()]() {
                        update_log(std::format("Authentication failed: {}", err));
                        update_progress(100);
                    });
                }
                return;
            }
        } else if (config.contains("auth") && config["auth"].contains("bearer_token")) {
            token = config["auth"]["bearer_token"].get<std::string>();
            logger->info("Using static bearer token from configuration.");
        }

        nlohmann::json upload_payload = m_current_data;
        if (m_current_topic == "HR") {
            upload_payload = nlohmann::json::object();
            upload_payload["options"] = {
                {"updateExistingRecords", "true"},
                {"insertBaseTables", "true"},
                {"forceLookupTableUpdate", "true"},
                {"disableSegUpdate", "false"},
                {"autoCreatePortalUser", "true"},
                {"mergeRecordsWithMatchingSsn", "false"},
                {"dateFormat", "dd.mm.yyyy"}
            };
            upload_payload["records"] = m_current_data;
            logger->info("Formatted HR payload with 'options' and 'records'.");
        }

        m_uploader->upload_async(meta->upload_endpoint, upload_payload, [this, logger](auto result) {
            if (m_window) {
                m_window->CallAfter([this, result, logger]() {
                    on_upload_complete(result);
                });
            }
        }, token);
    }).detach();
}

void AppController::update_log(const std::string& message) {
    if (m_window) {
        m_window->update_log(wxString::FromUTF8(message));
    }
}

void AppController::update_progress(int progress) {
    if (m_window) {
        m_window->update_progress(progress);
    }
}

void AppController::on_validation_complete(bool success, const std::vector<ValidationError>& errors) {
    auto logger = LogManager::get_instance().get_logger(m_current_topic);
    if (success) {
        update_log("Validation successful!");
        logger->info("Validation completed successfully");
        if (m_window) m_window->update_buttons(true, true);
    } else {
        update_log(std::format("Validation failed with {} errors:", errors.size()));
        logger->error("Validation failed with {} errors", errors.size());
        for (const auto& err : errors) {
            update_log(std::format("  {} : {}", err.path, err.message));
        }
        if (m_window) m_window->update_buttons(true, false);
    }
    update_progress(100);
}

void AppController::on_upload_complete(std::expected<void, std::string> result) {
    auto logger = LogManager::get_instance().get_logger(m_current_topic);
    if (result) {
        update_log("Upload successful!");
        logger->info("Upload completed successfully");
    } else {
        update_log(std::format("Upload failed: {}", result.error()));
        logger->error("Upload failed: {}", result.error());
    }
    update_progress(100);
}

} // namespace cgui
