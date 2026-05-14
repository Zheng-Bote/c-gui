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

namespace cgui {

/**
 * @class LogManager
 * @brief Singleton/Manager for topic-specific spdlog loggers.
 */
class LogManager {
public:
    static LogManager& get_instance();

    /**
     * @brief Initialize global logging settings.
     * @param log_path Base directory for log files.
     * @param log_level Global log level (string: trace, debug, info, etc.).
     */
    void initialize(const std::string& log_path, const std::string& log_level);

    /**
     * @brief Get or create a logger for a specific topic.
     * @param topic The topic name.
     * @return spdlog logger pointer.
     */
    std::shared_ptr<spdlog::logger> get_logger(const std::string& topic);

    /**
     * @brief Get the system-wide core logger.
     */
    std::shared_ptr<spdlog::logger> get_core_logger();

private:
    LogManager() = default;
    ~LogManager() = default;

    std::string m_log_path = "./logs";
    spdlog::level::level_enum m_level = spdlog::level::info;
    std::map<std::string, std::shared_ptr<spdlog::logger>> m_loggers;

    spdlog::level::level_enum parse_level(const std::string& level_str);
    std::string get_current_date_str();
};

} // namespace cgui

#endif // C_GUI_LOG_MANAGER_HPP
