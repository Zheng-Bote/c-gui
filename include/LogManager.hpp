/**
 * SPDX-FileComment: LogManager for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file LogManager.hpp
 * @brief Manages topic-based logging with spdlog.
 * @version 1.0.0
 * @date 2026-05-14
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_LOG_MANAGER_HPP
#define C_GUI_LOG_MANAGER_HPP

#include <string>
#include <memory>
#include <map>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <functional>
#include "common.hpp"

namespace cgui {

/**
 * @class LogManager
 * @brief Singleton/Manager for topic-specific spdlog loggers.
 */
class CGUI_API LogManager {
public:
    using LogCallback = std::function<void(const std::string&)>;

    static LogManager& get_instance();

    /**
     * @brief Initialize global logging settings.
     * @param log_path Base directory for log files.
     * @param log_level Global log level (string: trace, debug, info, etc.).
     */
    void initialize(const std::string& log_path, const std::string& log_level);

    /**
     * @brief Set a callback for all log messages.
     * @param callback The function to call with the formatted message.
     */
    void set_callback(LogCallback callback);

    /**
     * @brief Get or create a logger for a specific topic.
     * @param topic The topic name.
     * @return spdlog logger pointer.
     */
    std::shared_ptr<spdlog::logger> get_logger(const std::string& topic);

    /**
     * @brief Get or create a signed audit logger for a specific topic.
     * @param topic The topic name.
     */
    std::shared_ptr<spdlog::logger> get_audit_logger(const std::string& topic);

    /**
     * @brief Get the system-wide core logger.
     */
    std::shared_ptr<spdlog::logger> get_core_logger();

    /**
     * @brief Set the Ed25519 private key for log signing.
     * @param hex_key Private key as hex string.
     */
    void set_signing_key(const std::string& hex_key);

private:
    LogManager();
    ~LogManager();

    struct Impl;
#pragma warning(push)
#pragma warning(disable: 4251)
    std::unique_ptr<Impl> m_impl;
#pragma warning(pop)
};

} // namespace cgui

#endif // C_GUI_LOG_MANAGER_HPP
