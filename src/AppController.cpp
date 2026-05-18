/**
 * SPDX-FileComment: AppController implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file AppController.cpp
 * @brief Implementation of application orchestration logic.
 * @version 1.1.0
 * @date 2026-05-15
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#include "AppController.hpp"
#include "MainWindow.hpp"
#include "LogManager.hpp"
#include "AuthManager.hpp"
#include "rz_config.hpp"
#include <wx/wx.h>
#include <wx/app.h>
#include <thread>
#include <format>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <lmcons.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

namespace cgui {

namespace {
std::string get_os_user() {
#ifdef _WIN32
    TCHAR name[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserName(name, &size)) {
        std::wstring wname(name);
        return std::string(wname.begin(), wname.end());
    }
#else
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw) {
        return std::string(pw->pw_name);
    }
#endif
    return "unknown_user";
}
} // namespace

#ifdef _WIN32
const std::string kLibExt = ".dll";
#else
const std::string kLibExt = ".so";
#endif

AppController::AppController()
    : m_config_manager(std::make_unique<ConfigManager>()),
      m_plugin_loader(std::make_unique<PluginLoader>()),
      m_topic_registry(std::make_unique<TopicRegistry>()),
      m_schema_manager(std::make_unique<SchemaManager>()),
      m_validator(std::make_unique<Validator>()),
      m_uploader(std::make_unique<Uploader>()) {}

AppController::~AppController() {
    auto logger = LogManager::get_instance().get_core_logger();
    if (logger) {
        logger->info("Application session ending.");
    }
}

std::expected<void, std::string> AppController::init(const std::filesystem::path& ini_path, const std::string& password) {
    try {
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

        LogManager::get_instance().set_callback([this](const std::string& msg) {
            update_log(msg);
        });
        LogManager::get_instance().initialize(log_path, log_level);
        
        auto logger = LogManager::get_instance().get_core_logger();
        logger->info("Application session started. Version: {}", rz::config::VERSION);
        logger->info("OS User: {}", get_os_user());
        logger->info("Config loaded from {}", ini_path.string());

        // Load Plugin Paths
        m_data_plugins_dir = "./plugins/data";
        m_upload_plugins_dir = "./plugins/upload";
        if (config.contains("paths")) {
            if (config["paths"].contains("data_plugins_dir")) m_data_plugins_dir = config["paths"]["data_plugins_dir"].get<std::string>();
            if (config["paths"].contains("upload_plugins_dir")) m_upload_plugins_dir = config["paths"]["upload_plugins_dir"].get<std::string>();
        }
        logger->info("Data plugins directory: {}", m_data_plugins_dir.string());
        logger->info("Upload plugins directory: {}", m_upload_plugins_dir.string());

        // Register topics from config
        if (config.contains("topics")) {
            for (auto it = config["topics"].begin(); it != config["topics"].end(); ++it) {
                if (!it.value().is_object()) continue;

                TopicMeta meta;
                meta.topic = it.key();
                meta.schema_path = it.value().at("schema_path").get<std::string>();
                meta.upload_endpoint = it.value().at("upload_endpoint").get<std::string>();
                
                // Proxy (optional)
                if (it.value().contains("proxy")) {
                    meta.proxy = it.value().at("proxy").get<std::string>();
                }

                // Upload Timeout (optional)
                if (it.value().contains("upload_timeout")) {
                    try {
                        meta.upload_timeout = std::stol(it.value().at("upload_timeout").get<std::string>());
                    } catch (...) {
                        meta.upload_timeout = 60;
                    }
                }
                
                // Default upload plugin name based on topic
                meta.upload_plugin = it.value().contains("upload_plugin") ? it.value().at("upload_plugin").get<std::string>() : meta.topic + "_upload";
                
                // Convert to lowercase for plugin filename (convention)
                std::transform(meta.upload_plugin.begin(), meta.upload_plugin.end(), meta.upload_plugin.begin(), ::tolower);

                m_topic_registry->register_topic(meta);
                logger->info("Registered topic: {} (Upload plugin: {})", meta.topic, meta.upload_plugin);
            }
        }
    } catch (const std::exception& e) {
        auto logger = LogManager::get_instance().get_core_logger();
        if (logger) {
            logger->error("Exception during initialization: {}", e.what());
        }
        return std::unexpected(std::format("Configuration error: {}", e.what()));
    }

    return {};
}

std::filesystem::path AppController::get_plugin_path(const std::string& name, PluginType type) const {
    std::filesystem::path base_dir = (type == PluginType::DATA) ? m_data_plugins_dir : m_upload_plugins_dir;
    return base_dir / (name + kLibExt);
}

std::vector<std::string> AppController::get_topics() const {
    return m_topic_registry->get_all_topics();
}

void AppController::select_topic(const std::string& topic) {
    m_current_topic = topic;
    update_log(std::format("Selected topic: {}", topic));
    LogManager::get_instance().get_core_logger()->info("Topic selected: {}", topic);
}

void AppController::set_date_format(const std::string& format) {
    m_date_format = format;
    update_log(std::format("Date format set to: {}", format));
}

std::string AppController::get_effective_date_format(const std::string& topic) const {
    const auto& config = m_config_manager->get_config();
    if (config.contains("topics") && config["topics"].contains(topic)) {
        const auto& topic_config = config["topics"][topic];
        if (topic_config.contains("options") && topic_config["options"].contains("dateFormat")) {
            return topic_config["options"]["dateFormat"].get<std::string>();
        }
    }
    return m_date_format;
}

std::vector<std::string> AppController::get_available_interfaces(const std::string& topic) const {
    std::vector<std::string> interfaces;
    if (!std::filesystem::exists(m_data_plugins_dir)) return interfaces;

    std::string prefix = topic + "_";
    std::string suffix = "_input" + kLibExt;
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

    for (const auto& entry : std::filesystem::directory_iterator(m_data_plugins_dir)) {
        if (!entry.is_regular_file()) continue;
        
        std::string filename = entry.path().filename().string();
        std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

        if (filename.starts_with(prefix) && filename.ends_with(suffix)) {
            // Extract interface name: <topic>_<interface>_input.<ext>
            std::string interface_name = filename.substr(prefix.length());
            interface_name = interface_name.substr(0, interface_name.length() - suffix.length());
            interfaces.push_back(interface_name);
        }
    }
    return interfaces;
}

std::string AppController::get_default_data_source(const std::string& topic, const std::string& interface_name) const {
    const auto& config = m_config_manager->get_config();
    if (config.contains("topics") && config["topics"].contains(topic)) {
        const auto& topic_config = config["topics"][topic];
        std::string key = "data_" + interface_name;
        if (topic_config.contains(key)) {
            return topic_config[key].get<std::string>();
        }
    }
    return "";
}

std::vector<PluginInfo> AppController::get_all_plugins() const {
    std::vector<PluginInfo> plugins;
    auto logger = LogManager::get_instance().get_core_logger();

    auto scan_dir = [&](const std::filesystem::path& dir, PluginType type) {
        logger->debug("Scanning directory for {} plugins: {}", 
                     (type == PluginType::DATA ? "DATA" : "UPLOAD"), dir.string());
        
        if (!std::filesystem::exists(dir)) {
            logger->warn("Plugin directory does not exist: {}", dir.string());
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != kLibExt) continue;

            logger->debug("Found potential plugin file: {}", entry.path().string());
            PluginLoader loader;
            if (auto res = loader.load(entry.path()); res) {
                auto* plugin = loader.get_plugin();
                
                // Validate that the plugin type matches the directory type
                if (plugin->get_type() != type) {
                    logger->warn("Plugin {} reported type {}, but was found in the {} directory. Skipping.", 
                                 entry.path().string(), 
                                 (plugin->get_type() == PluginType::DATA ? "DATA" : "UPLOAD"),
                                 (type == PluginType::DATA ? "data" : "upload"));
                    continue;
                }

                PluginInfo info;
                info.name = entry.path().stem().string();
                info.topic = plugin->get_topic();
                info.type = (plugin->get_type() == PluginType::DATA) ? "DATA" : "UPLOAD";
                info.version = plugin->get_version();
                info.path = entry.path();

                if (plugin->get_type() == PluginType::DATA) {
                    info.interface_type = static_cast<IDataPlugin*>(plugin)->get_interface_type();
                }
                
                logger->info("Discovered plugin: {} (Topic: {}, Version: {})", info.name, info.topic, info.version);
                plugins.push_back(info);
                plugin->shutdown();
            } else {
                logger->error("Failed to load plugin for metadata scan: {} ({})", entry.path().string(), res.error());
            }
        }
    };

    scan_dir(m_data_plugins_dir, PluginType::DATA);
    scan_dir(m_upload_plugins_dir, PluginType::UPLOAD);

    return plugins;
}

std::expected<void, std::string> AppController::load_data(const std::string& interface_name, const std::string& path_or_conn) {
    if (m_current_topic.empty()) {
        return std::unexpected("No topic selected");
    }

    auto logger = LogManager::get_instance().get_logger(m_current_topic);
    logger->info("Loading data for topic {} using interface {} from: {}", m_current_topic, interface_name, path_or_conn);

    auto meta = m_topic_registry->get_topic_meta(m_current_topic);
    if (!meta) {
        logger->error("Topic metadata not found for: {}", m_current_topic);
        return std::unexpected("Topic metadata not found");
    }

    // Construct plugin name based on convention: <topic>_<interface>_input
    std::string plugin_name = m_current_topic + "_" + interface_name + "_input";
    std::transform(plugin_name.begin(), plugin_name.end(), plugin_name.begin(), ::tolower);

    std::filesystem::path plugin_path = get_plugin_path(plugin_name, PluginType::DATA);
    auto load_res = m_plugin_loader->load(plugin_path);
    if (!load_res) {
        logger->error("Failed to load data plugin {}: {}", plugin_path.string(), load_res.error());
        return std::unexpected(std::format("Failed to load data plugin {}: {}", plugin_path.string(), load_res.error()));
    }

    auto* plugin = m_plugin_loader->get_plugin();
    if (plugin->get_type() != PluginType::DATA) {
        logger->error("Plugin {} is not a data plugin", plugin_path.string());
        return std::unexpected("Loaded plugin is not a data plugin");
    }

    auto* data_plugin = static_cast<IDataPlugin*>(plugin);
    
    // Verify interface type matches
    if (data_plugin->get_interface_type() != interface_name) {
        logger->warn("Plugin reported interface type '{}', but was loaded as '{}'", data_plugin->get_interface_type(), interface_name);
    }

    nlohmann::json plugin_config;
    plugin_config["file_path"] = path_or_conn;
    plugin_config["connection_string"] = path_or_conn; // Support both naming styles
    plugin_config["topic"] = m_current_topic;

    if (!data_plugin->initialize(plugin_config)) {
        logger->error("Failed to initialize data plugin {} with source: {}", plugin_name, path_or_conn);
        return std::unexpected("Failed to initialize data plugin");
    }

    // Load first batch for preview and full data
    auto records = data_plugin->fetch_batch(1000000); // For now, load everything
    data_plugin->shutdown();

    if (records.empty()) {
        logger->warn("No records found by data plugin from: {}", path_or_conn);
        return std::unexpected("No records found");
    }

    m_current_data = nlohmann::json::array();
    for (const auto& r : records) {
        m_current_data.push_back(r);
    }

    const auto& config = m_config_manager->get_config();
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

    update_log(std::format("Loaded {} records from {}", m_current_data.size(), std::filesystem::path(path_or_conn).filename().string()));
    logger->info("Successfully loaded {} records from {}", m_current_data.size(), path_or_conn);
    
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
    logger->info("Initiated by OS User: {}", get_os_user());
    update_progress(0);

    const auto& config = m_config_manager->get_config();

    std::thread([this, meta, config, logger]() {
        std::filesystem::path plugin_path = get_plugin_path(meta->upload_plugin, PluginType::UPLOAD);
        
        // We need a separate loader for the background thread or protect the main one
        PluginLoader loader;
        auto load_res = loader.load(plugin_path);
        if (!load_res) {
            logger->error("Failed to load upload plugin {}: {}", plugin_path.string(), load_res.error());
            if (m_window) {
                m_window->CallAfter([this, plugin_path, err = load_res.error()]() {
                    update_log(std::format("Failed to load upload plugin {}: {}", plugin_path.string(), err));
                    update_progress(100);
                });
            }
            return;
        }

        auto* plugin = loader.get_plugin();
        if (plugin->get_type() != PluginType::UPLOAD) {
            logger->error("Plugin {} is not an upload plugin", plugin_path.string());
            return;
        }

        auto* upload_plugin = static_cast<IUploadPlugin*>(plugin);

        nlohmann::json plugin_config = config; // Give it full config
        plugin_config["topic"] = m_current_topic;
        plugin_config["upload_endpoint"] = meta->upload_endpoint;
        plugin_config["options"] = nlohmann::json::object();
        plugin_config["options"]["dateFormat"] = m_date_format;

        // Merge topic-specific overrides (like 'options')
        if (config.contains("topics") && config["topics"].contains(m_current_topic)) {
            const auto& topic_cfg = config["topics"][m_current_topic];
            for (auto it = topic_cfg.begin(); it != topic_cfg.end(); ++it) {
                if (it.key() == "options" && it.value().is_object()) {
                    // Merge options specifically
                    for (auto opt_it = it.value().begin(); opt_it != it.value().end(); ++opt_it) {
                        plugin_config["options"][opt_it.key()] = opt_it.value();
                    }
                } else {
                    plugin_config[it.key()] = it.value();
                }
            }
        }

        if (!upload_plugin->initialize(plugin_config)) {
            logger->error("Failed to initialize upload plugin {}", plugin_path.string());
            if (m_window) {
                m_window->CallAfter([this]() {
                    update_log("Failed to initialize upload plugin");
                    update_progress(100);
                });
            }
            return;
        }

        auto result = upload_plugin->upload(m_current_data);
        upload_plugin->shutdown();

        if (m_window) {
            m_window->CallAfter([this, result]() {
                on_upload_complete(result);
            });
        }
    }).detach();
}

void AppController::update_log(const std::string& message) {
    if (m_window) {
        m_window->CallAfter([this, message]() {
            if (m_window) {
                m_window->update_log(wxString::FromUTF8(message));
            }
        });
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
